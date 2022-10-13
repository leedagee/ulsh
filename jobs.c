#include "jobs.h"

#include <asm-generic/errno.h>
#include <bits/types/siginfo_t.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/signalfd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

struct job_t *jobs_head;
int got_sigchld = 0, fd_chld;

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

void reap_children() {
  struct signalfd_siginfo ssi;
  siginfo_t si;
  while (read(fd_chld, &ssi, sizeof(ssi)) == sizeof(ssi)) {
    fprintf(stderr, "Received SIGCHLD for %d with %d.\n", ssi.ssi_pid,
            ssi.ssi_code);
    pid_t pgid = getpgid(ssi.ssi_pid);  // also works for a defunct one
    struct job_t **job = find_job(pgid);
    if (job == NULL) continue;
    for (struct procstat *p = (*job)->procstats; p; p = p->next) {
      if (p->pid == ssi.ssi_pid) {
        switch (ssi.ssi_code) {
          case CLD_STOPPED:
            p->status = PROCSTAT_STOPPED;
            break;
          case CLD_EXITED:
            p->status = PROCSTAT_EXITED;
            p->retval = ssi.ssi_status;
            break;
          case CLD_CONTINUED:
            p->status = PROCSTAT_RUNNING;
            break;
        }
        break;
      }
    }
    int pgst = 0;
    for (struct procstat *p = (*job)->procstats; p; p = p->next) {
      pgst |= p->status;
    }
    if (!(pgst & (PROCSTAT_RUNNING | PROCSTAT_STOPPED))) {
      // procs in group r all dead, reap the whole group
      errno = 0;
      while (waitid(P_PGID, pgid, &si, WNOHANG | WEXITED) != -1)
        ;
      if (errno != ECHILD) perror("Cannot reap children");
      delete_job(job);
      fprintf(stderr, "Process group %d dead. RIP.\n", pgid);
    }
  }
  if (errno != EAGAIN && errno != EWOULDBLOCK) {
    perror("Cannot read signalfd");
  }
}

void wait_on_pgrp(pid_t pgrp) {
  siginfo_t si;
  for (;;) {
    fd_set fds;
    FD_ZERO(&fds);
    FD_SET(fd_chld, &fds);
    int r = select(FD_SETSIZE, &fds, NULL, NULL, NULL);
    if (r == -1) {
      perror("Cannot wait on fds");
      return;
    }
    if (FD_ISSET(fd_chld, &fds)) {
      reap_children();
    }
    while (waitid(P_PGID, pgrp, &si, WEXITED | WSTOPPED | WNOHANG) != -1)
      if (si.si_code == CLD_STOPPED) {
        pid_t pgid = getpgid(si.si_pid);
        struct job_t **job = find_job(pgid), *_job;
        if (job == NULL) {
          job = &_job;
          *job = add_job(pgid);
        }
        struct procstat *now = NULL;
        for (struct procstat *p = (*job)->procstats; p; p = p->next) {
          if (p == NULL) break;
          if (p->pid == si.si_pid) {
            now = p;
          }
        }
        if (now == NULL) now = malloc(sizeof(struct procstat));
        now->pid = si.si_pid;
        now->status = PROCSTAT_STOPPED;
        now->retval = -1;
        now->next = (*job)->procstats;
        (*job)->procstats = now;
        return;
      }
    if (errno == ECHILD) return;
  }
}
