#include "exec.h"

#include <errno.h>
#include <fcntl.h>
#include <spawn.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include "builtins/builtin.h"
#include "jobs.h"

int last_return_value;

extern char **environ;

pid_t execute(int flags, int argc, char *argv[], int fd_in, int fd_out,
              pid_t pgrp) {
  BUILTIN_HANDLER handler = builtin_find_entry(argv[0]);
  pid_t pid;
  if (handler != NULL) {
    if (flags & EXECUTE_MUST_FORK) {
      switch (pid = fork()) {
        case -1:
          perror("Cannot fork");
          break;
        case 0:
          if (flags & EXECUTE_DUP_STDIN) dup2(fd_in, 0);
          if (flags & EXECUTE_DUP_STDOUT) dup2(fd_out, 1);
          setpgid(0, pgrp);
          if (flags & EXECUTE_FOREGROUND && pgrp == 0) {
            tcsetpgrp(255, getpid());
          }
          exit(handler(argc, argv));
          break;
      }
    } else {
      last_return_value = handler(argc, argv);
      return 0;
    }
  } else {
    if ((pid = fork()) == 0) {
      if (flags & EXECUTE_DUP_STDIN) dup2(fd_in, 0);
      if (flags & EXECUTE_DUP_STDOUT) dup2(fd_out, 1);
      setpgid(0, pgrp);
      if ((flags & EXECUTE_FOREGROUND) && pgrp == 0) {
        tcsetpgrp(255, getpid());
      }
      execvp(argv[0], argv);
      perror("Failed to exec");
      exit(1);
    }
    if (pid == -1) {
      perror("Cannot fork");
    }
  }

  if (flags & EXECUTE_DUP_STDIN) close(fd_in);
  if (flags & EXECUTE_DUP_STDOUT) close(fd_out);

  return pid;
}

int run_parsed(struct parse_result_t *res, struct procstat **proc, pid_t pgrp,
               int fd_in) {
  int fd_out = -1;
  if (res->f_in) {
    if (fd_in) close(fd_in);
    fd_in = open(res->f_in, O_RDONLY);
    if (fd_in == -1) {
      fprintf(stderr, "Cannot open file %s as read: %s\n", res->f_in,
              strerror(errno));
      return -1;
    }
  }
  int pipefd[2] = {-1, -1};
  if (res->flags & PARSE_RESULT_PIPE) {
    if (pipe(pipefd) == -1) perror("Cannot create pipe pairs");
    fd_out = pipefd[1];
  }
  if (res->f_out) {
    if (fd_out) close(fd_out);
    fd_out = open(
        res->f_out,
        O_WRONLY | O_CREAT | (res->flags & PARSE_RESULT_APPEND) ? O_APPEND : 0);
    if (fd_out == -1) {
      fprintf(stderr, "Cannot open file %s as write: %s\n", res->f_in,
              strerror(errno));
      return -1;
    }
  }

  int options = ((res->flags & PARSE_RESULT_PIPE) ||
                 (res->flags & PARSE_RESULT_BACKGROUND))
                    ? EXECUTE_MUST_FORK
                    : 0;

  if (!(res->flags & PARSE_RESULT_BACKGROUND)) options |= EXECUTE_FOREGROUND;

  if (fd_in != -1) options |= EXECUTE_DUP_STDIN;
  if (fd_out != -1) options |= EXECUTE_DUP_STDOUT;

  pid_t pid;
  pid = execute(options, res->argc, res->argv, fd_in, fd_out, pgrp);

  if (pid == 0)
    return 1;

  struct procstat *newproc = malloc(sizeof(struct procstat));
  newproc->next = NULL;
  newproc->pid = pid;
  newproc->retval = -1;
  newproc->status = PROCSTAT_RUNNING;

  if (proc != NULL) *proc = newproc;
  if (pgrp == 0) {
    pgrp = pid;
    if (res->flags & PARSE_RESULT_BACKGROUND) {
      struct job_t *job = add_job(pid);
      job->procstats = newproc;
    }
  }

  if (res->next != NULL) {
    run_parsed(res->next, &newproc->next, pgrp, pipefd[0]);
  }
}
