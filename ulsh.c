#include <readline/readline.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include "builtins/builtin.h"
#include "prompt.h"

void handle_sigint(int sig) {
  // do nothing.
  // but is it proper?
}

int execute(int argc, char *argv[]) {
  BUILTIN_HANDLER handler = builtin_find_entry(argv[0]);
  if (handler != NULL) {
    return handler(argc, (const char **)argv);
  }

  pid_t pid;
  if ((pid = vfork()) == -1) {
    perror("Cannot vfork");
  }
  if (pid == 0) {
    execvp(argv[0], argv);
    // we can't do perror here because of the vfork
    _exit(255); // and exit without flushing stdio

    /* 
     * bash do stat(2) before doing any fork/vfork/clone syscalls
     * to detect a potential command not found. If we do nothing
     * special here we simply cannot do anything with a execve error.
     * and the return value carries information too limited, the main
     * process can only receive a non-zero return value but determine
     * the source. another way to do message transport is use a pipe
     * or at least a fd, vfork makes this impossible too.
     * vfork maybe too limited to be used here.
     * maybe it's time to have a look at clone(2)
     */
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

    execute(cargc, cargv);

    free(cmd);
  }
}