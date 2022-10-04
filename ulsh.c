#include <readline/readline.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include "prompt.h"

void handle_sigint(int sig) {
  // do nothing.
}

int main() {
  signal(SIGINT, handle_sigint);
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

    pid_t pid;
    if ((pid = vfork()) == -1) {
      perror("Cannot vfork");
    }
    if (pid == 0) {
      execvp(cargv[0], cargv);
      // we can't do perror here because of the vfork
      exit(255);
    }

    int wstatus;
    if (waitpid(pid, &wstatus, WUNTRACED) == -1) {
      perror("Cannot wait for child");
    }

    free(cmd);
  }
}