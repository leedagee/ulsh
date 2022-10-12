#include "jobs.h"

#include <asm-generic/errno.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/signalfd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

struct job_t *jobs;
int got_sigchld = 0, fd_chld;

void add_job(int pgid) {
  struct job_t *job = malloc(sizeof(struct job_t));
  job->pgid = pgid;
  job->flags = 0;
  job->next = jobs;
  jobs = job;
}

void delete_job(struct job_t **job) {
  struct job_t *tmp = (*job)->next;
  free(*job);
  *job = tmp;
}

struct job_t *find_job(int pgid) {
  struct job_t *c = jobs;
  while (c != NULL) {
    if (c->pgid == pgid) return c;
    c = c->next;
  }
  return NULL;
}

void reap_children() {
  struct signalfd_siginfo ssi;
  while(read(fd_chld, &ssi, sizeof(ssi)) == sizeof(ssi)) {
    printf("%d\n", ssi.ssi_pid);
  }
  if (errno != EAGAIN && errno != EWOULDBLOCK) {
    perror("Cannot read signalfd");
  }
}
