#include <readline/readline.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "builtins/builtin.h"
#include "prompt.h"
#include "parser.h"

void handle_sigint(int sig) {
  // do nothing.
  // but is it proper?
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

    parse_command(cmd, -1);

    free(cmd);
  }
}