#pragma once

#include <stdint.h>
#include <sys/types.h>
#include <signal.h>

#define JOB_STATUS(x) (x &)

struct procstat {
  pid_t pid;
  int wstatus;
  struct procstat* next;
};

struct job_t {
  struct job_t *next;
  uint16_t flags;
  pid_t pgid;
  struct procstat *procstats;
};

extern struct job_t *jobs;
extern int got_sigchld;
extern int fd_chld;

void add_job(int pgid);
void delete_job(struct job_t **job);
struct job_t *find_job(int pgid);

void reap_children();
