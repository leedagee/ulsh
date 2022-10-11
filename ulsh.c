#include <errno.h>
#include <fcntl.h>
#include <readline/history.h>
#include <readline/readline.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include "builtins/builtin.h"
#include "exec.h"
#include "jobs.h"
#include "parser.h"
#include "prompt.h"

int main(int argc, char *argv[]) {
  int fd_tty = open("/dev/tty", O_RDWR);
  if (fd_tty == -1) perror("Cannot set fd 255 to the console");
  dup2(fd_tty, 255);
  if (fd_tty == -1) perror("Cannot set fd 255 to the console");
  close(fd_tty);

  signal(SIGTTOU, SIG_IGN);
  signal(SIGTTIN, SIG_IGN);
  signal(SIGCHLD, sigchld_handler);

  setpgid(0, 0);

  pid_t old_fg = tcgetpgrp(255);
  if (tcsetpgrp(255, getpid()) == -1) {
    perror("Cannot set foreground process group");
  }

  builtin_init();
  for (;;) {
    char *prompt = getprompt();
    char *inp = readline(prompt), *cmd = inp;
    free(prompt);

    if (cmd == NULL) {
      putchar('\n');
      free(cmd);
      exit(0);
    }

    char *cur = cmd, *end = cmd + strlen(cmd);
    add_history(cmd);

    while (cur < end) {
      struct parse_result_t *res;
      ssize_t s = parse_command(cur, &res);
      if (s == -1) break;
      pid_t pid;
      siginfo_t si;
      si.si_pid = 0;
      run_parsed(res, &pid, 0, -1);
      tcsetpgrp(255, pid);
      if (!(res->flags & PARSE_RESULT_BACKGROUND)) {
        errno = 0;
        while (errno != ECHILD) {
          waitid(P_PGID, pid, &si, WEXITED | WSTOPPED);
          if (si.si_code == CLD_STOPPED) {
            add_job(pid);
            tcsetpgrp(255, 0);
            break;
          }
        }
      }
      if (res != NULL) free_parse_result(res);
      tcsetpgrp(255, getpid());
      cur += s;
    }

    free(cmd);
  }

  tcsetpgrp(255, old_fg);

  close(255);
}
