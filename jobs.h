#pragma once

#include <signal.h>
#include <stdint.h>
#include <sys/types.h>

#define JOB_STATUS(x) (x &)

#define PROCSTAT_RUNNING 1
#define PROCSTAT_STOPPED 2
#define PROCSTAT_EXITED 4

struct procstat {
  pid_t pid;
  int status;
  int retval;
  struct procstat *next;
};

struct job_t {
  struct job_t *next;
  uint16_t flags;
  pid_t pgid;
  struct procstat *procstats;
};

extern struct job_t *jobs_head;
extern pid_t foreground;
extern int sigchld_pipes[2];

struct job_t *add_job(int pgid);
void delete_job(struct job_t **job);
struct job_t **find_job(int pgid);
void delete_proc(struct procstat *proc);
void add_proc(struct job_t *job, struct procstat *proc);
void reap_children();
