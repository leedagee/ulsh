#include "exec.h"

#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>

#include "builtins/builtin.h"

int last_return_value;

int executer_execvp(int argc, char *argv[]) {
  execvp(argv[0], argv);
  perror("Cannot execute new process");
  // we can use perror here of course
  return 255;
}

int execute(int flags, int argc, char *argv[], pid_t *pid_store, int fd_in,
            int fd_out) {
  if (strlen(argv[0]) == 0) {
    return last_return_value;
  }
  BUILTIN_HANDLER handler = builtin_find_entry(argv[0]);
  if (handler != NULL) {
    if (!(flags & EXECUTE_MUST_FORK)) {
      return handler(argc, argv);
    }
  } else {
    handler = (BUILTIN_HANDLER)executer_execvp;
  }

  pid_t pid;
  if ((pid = fork()) == -1) {
    perror("Cannot fork");
  }
  if (pid == 0) {
    if (flags & EXECUTE_DUP_STDIN) {
      if (dup2(fd_in, 0) == -1) {
        perror("Cannot set stdin");
      }
      if (close(fd_in) == -1) {
        perror("Cannot close old fd");
      }
    }
    if (flags & EXECUTE_DUP_STDOUT) {
      if (dup2(fd_out, 1) == -1) {
        perror("Cannot set stdout");
      }
      if (close(fd_out) == -1) {
        perror("Cannot close old fd");
      }
    }

    exit(handler(argc, argv));

    /*
     * bash do stat(2) before doing any fork/vfork/clone syscalls
     * to detect a potential command not found. If we do nothing
     * special here we simply cannot do anything with a execve error.
     * and the return value carries information too limited, the main
     * process can only receive a non-zero return value but determine
     * the source. another way to do message transport is use a pipe
     * or at least a fd, vfork makes this impossible too.
     * but not all errors can be easily detected, for example the
     * absence of a interpreter, or operation block by some selinux
     * rules for example.
     * vfork maybe too limited to be used here.
     * maybe it's time to have a look at clone(2)
     */
  }

  if (flags & EXECUTE_DUP_STDIN) close(fd_in);
  if (flags & EXECUTE_DUP_STDOUT) close(fd_out);

  if (pid_store != NULL) {
    *pid_store = pid;
  }

  if (flags & EXECUTE_NO_WAIT) {
    return 0;
  }

  int wstatus;
  if (waitpid(pid, &wstatus, WUNTRACED) == -1) {
    perror("Cannot wait for child");
  }

  return last_return_value = WEXITSTATUS(wstatus);
}
