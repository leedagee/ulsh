#include "jobs.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

struct job_t *jobs;

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

void sigchld_handler(int sig) {}
