#include <readline/readline.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>

#include "builtins/builtin.h"
#include "exec.h"
#include "prompt.h"
#include "parser.h"

void handle_sigint(int sig) {
  // do nothing.
  // but is it proper?
}

int main() {
  signal(SIGINT, handle_sigint);
  builtin_init();
  pid_t childs[128];
  for (;;) {
    memset(childs, 0, sizeof(childs));

    char *prompt = getprompt();
    char *cmd = readline(prompt);
    free(prompt);

    if (cmd == NULL) {
      putchar('\n');
      exit(0);
    }

    int ret = parse_command(cmd, -1, 0, childs);
    if (ret != 0) /* wait for lots of child processes */ {
      int wstatus;
      for(int i=0;childs[i];i++) {
        waitpid(childs[i], &wstatus, WUNTRACED);
      }
      last_return_value = WEXITSTATUS(wstatus);
    }

    free(cmd);
  }
}
