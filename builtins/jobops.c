#include "jobops.h"

#include <stdio.h>
#include <unistd.h>
#include "../jobs.h"

BUILTIN_DECLARE(jobs) {
  struct job_t *head = jobs_head;
  while (head) {
    printf("%d\n", head->pgid);
    head = head->next;
  }
  return 0;
}

BUILTIN_DECLARE(fg) {
  struct job_t *job = jobs_head;
  tcsetpgrp(255, job->pgid);
  foreground = job->pgid;
  killpg(job->pgid, SIGCONT);
}

BUILTIN_DECLARE(bg) {
  struct job_t *job = jobs_head;
  killpg(job->pgid, SIGCONT);
}
