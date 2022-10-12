#include "jobops.h"

#include <stdio.h>
#include "../jobs.h"

BUILTIN_DECLARE(jobs) {
  struct job_t *head = jobs_head;
  while (head) {
    printf("%d\n", head->pgid);
    head = head->next;
  }
  return 0;
}
