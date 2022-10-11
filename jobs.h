#pragma once

#include <stdint.h>
#include <sys/types.h>

#define JOB_STATUS(x) (x &)

struct job_t {
  struct job_t *next;
  uint16_t flags;
  pid_t pgid;
};

extern struct job_t *jobs;

void add_job(int pgid);
void delete_job(struct job_t **job);
struct job_t *find_job(int pgid);
void sigchld_handler(int sig);
