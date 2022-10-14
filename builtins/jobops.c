#include "jobops.h"

#include <stdio.h>
#include <unistd.h>

#include "../exec.h"
#include "../jobs.h"

BUILTIN_DECLARE(jobs) {
  struct job_t *head = jobs_head;
  while (head) {
    int pgst = 0;
    char *status_str;
    for (struct procstat *proc = head->procstats; proc; proc = proc->next) {
      pgst |= proc->status;
    }
    if (pgst & PROCSTAT_RUNNING)
      status_str = "Running";
    else if (pgst & PROCSTAT_STOPPED)
      status_str = "Stopped";
    else
      status_str = "Done";  // impossible here, a job that already done is
                            // deleted from the list
    printf("% 6d %9s %s\n", head->pgid, status_str, head->cmd);
    head = head->next;
  }
  return 0;
}

BUILTIN_DECLARE(fg) {
  struct job_t *job = jobs_head;
  if (argc == 2) {
    int pgid;
    sscanf(argv[1], "%d", &pgid);
    while (job != NULL) {
      if (job->pgid == pgid) break;
      job = job->next;
    }
  }
  if (job == NULL) {
    fprintf(stderr, "Cannot find target job.\n");
    return 1;
  }
  tcsetpgrp(255, job->pgid);
  foreground = job->pgid;
  killpg(job->pgid, SIGCONT);
  return last_return_value;
}

BUILTIN_DECLARE(bg) {
  struct job_t *job = jobs_head;
  if (argc == 2) {
    int pgid;
    sscanf(argv[1], "%d", &pgid);
    while (job != NULL) {
      if (job->pgid == pgid) break;
      job = job->next;
    }
  }
  if (job == NULL) {
    fprintf(stderr, "Cannot find target job.\n");
    return 1;
  }
  killpg(job->pgid, SIGCONT);
  return 0;
}
