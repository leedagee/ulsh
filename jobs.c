#include "jobs.h"

#include <asm-generic/errno.h>
#include <assert.h>
#include <bits/types/siginfo_t.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/signalfd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include "prompt.h"
#include "readline.h"

struct job_t *jobs_head;
pid_t foreground;
int sigchld_pipes[2];

struct job_t *add_job(int pgid) {
  struct job_t *job = malloc(sizeof(struct job_t));
  job->pgid = pgid;
  job->flags = 0;
  job->next = jobs_head;
  job->procstats = NULL;
  jobs_head = job;
  return job;
}

void delete_job(struct job_t **job) {
  struct job_t *tmp = (*job)->next;
  delete_proc((*job)->procstats);
  free((*job)->cmd);
  free(*job);
  *job = tmp;
}

void add_proc(struct job_t *job, struct procstat *proc) {
  struct procstat *tmp = job->procstats;
  job->procstats = proc;
  proc->next = tmp;
}

void delete_proc(struct procstat *proc) {
  if (proc == NULL) return;
  if (proc->next != NULL) delete_proc(proc->next);
  free(proc);
}

struct job_t **find_job(int pgid) {
  struct job_t **c = &jobs_head;
  while (*c != NULL) {
    if ((*c)->pgid == pgid) return c;
    c = &((*c)->next);
  }
  return NULL;
}

void find_procstat(pid_t pid, struct job_t **job, struct procstat **proc) {
  for (struct job_t *j = jobs_head; j; j = j->next) {
    for (struct procstat *p = j->procstats; p; p = p->next) {
      if (p->pid == pid) {
        *job = j;
        *proc = p;
        return;
      }
    }
  }
}

void on_child_stopped(pid_t pid) {
  fprintf(stderr, "Child %d stopped.\n", pid);
  struct job_t *job;
  struct procstat *p;
  find_procstat(pid, &job, &p);
  p->status = PROCSTAT_STOPPED;
  pid_t pgrp = getpgid(pid);
  if (foreground == pgrp) {
    tcsetpgrp(255, getpid());
    foreground = getpid();
  }
}

void on_child_running(pid_t pid) {
  struct job_t *job;
  struct procstat *p;
  find_procstat(pid, &job, &p);
  p->status = PROCSTAT_RUNNING;
}

void on_child_exited(pid_t pid) {
  struct job_t *job;
  struct procstat *p;
  siginfo_t si;
  find_procstat(pid, &job, &p);
  p->status = PROCSTAT_EXITED;
  int pgst = 0;
  for (struct procstat *i = job->procstats; i; i = i->next) {
    pgst |= i->status;
  }
  if (!(pgst & (PROCSTAT_RUNNING | PROCSTAT_STOPPED))) {
    // fprintf(stderr, "Process group %d dead. RIP.\n", job->pgid);
    //  procs in group r all dead, reap the whole group
    errno = 0;
    while (waitid(P_PGID, job->pgid, &si, WNOHANG | WEXITED) != -1)
      ;
    if (errno != ECHILD) perror("Cannot reap children");
    if (foreground == job->pgid) {
      tcsetpgrp(255, getpid());
      foreground = getpid();
    }
    delete_job(find_job(job->pgid));
  }
}

void reap_children() {
  siginfo_t si;
  char c;
  while (read(sigchld_pipes[0], &c, 1) == 1) assert(c == '.');
  assert(errno == EAGAIN || errno == EWOULDBLOCK);
  errno = 0;
  // signalfd is still unreliable, use waitid to receive more
  for (;;) {
    int r = waitid(P_ALL, 0, &si, WEXITED | WCONTINUED | WSTOPPED | WNOHANG);
    if (r == -1) break;
    if (si.si_pid == 0) return;
    fprintf(stderr, "%d has changed its state to %d.\n", si.si_pid, si.si_code);
    switch (si.si_code) {
      case CLD_CONTINUED:
        on_child_running(si.si_pid);
        break;
      case CLD_STOPPED:
        on_child_stopped(si.si_pid);
        break;
      case CLD_DUMPED:
      case CLD_KILLED:
      case CLD_EXITED:
        on_child_exited(si.si_pid);
        break;
    }
  }
  if (errno != ECHILD) perror("Cannot receive info for stopped children");
}
