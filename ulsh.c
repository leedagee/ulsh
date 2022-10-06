#include <readline/readline.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <fcntl.h>

#include "builtins/builtin.h"
#include "prompt.h"

#define EXECUTE_MUST_FORK     0x0001
#define EXECUTE_DUP_STDIN     0x0002
#define EXECUTE_DUP_STDOUT    0x0004
#define EXECUTE_NO_WAIT       0x0008

int last_return_value;

void handle_sigint(int sig) {
  // do nothing.
  // but is it proper?
}

int executer_execvp(int argc, char *argv[]) {
  execvp(argv[0], argv); 
  perror("Cannot execute new process");
  // we can use perror here of course
  return 255;
}

int execute(int flags, int argc, char *argv[], pid_t *pid_store, int fd_in, int fd_out) {
  if (strlen(argv[0]) == 0) {
    return last_return_value;
  }
  BUILTIN_HANDLER handler = builtin_find_entry(argv[0]);
  if (handler != NULL) {
    if (!(flags & EXECUTE_MUST_FORK)) {
      return handler(argc, (const char **)argv);
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
      if(dup2(fd_in, 0) == -1) {
        perror("Cannot set stdin");
      }
      if(fcntl(0, F_SETFD, O_CLOEXEC) == -1) {
        perror("Cannot set CLOEXEC on stdin");
      }
    }
    if (flags & EXECUTE_DUP_STDOUT) {
      if(dup2(fd_out, 1) == -1) {
        perror("Cannot set stdout");
      }
      if(fcntl(1, F_SETFD, O_CLOEXEC) == -1) {
        perror("Cannot set CLOEXEC on stdout");
      }
    }

    exit(handler(argc, (const char **)argv));

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

  return WEXITSTATUS(wstatus);
}

int main() {
  signal(SIGINT, handle_sigint);
  builtin_init();
  for (;;) {
    char *prompt = getprompt();
    char *cmd = readline(prompt);
    free(prompt);
    if (cmd == NULL) {
      putchar('\n');
      exit(0);
    }

    char *cargv[128] = {cmd};
    int cargc = 1;
    for (char *s = cmd, *c = cmd; *c; c++) {
      if (*c == ' ') {
        *c = '\0';
        cargv[cargc++] = c + 1;
      }
    }

    last_return_value = execute(0, cargc, cargv, NULL, 0, 0);

    free(cmd);
  }
}