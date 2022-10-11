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
#include "parser.h"
#include "prompt.h"

void handle_ignored(int sig) {}

int main(int argc, char *argv[]) {
  int fd_tty = open("/dev/tty", O_RDWR);
  if (fd_tty == -1) perror("Cannot set fd 255 to the console");
  dup2(fd_tty, 255);
  if (fd_tty == -1) perror("Cannot set fd 255 to the console");
  close(fd_tty);

  signal(SIGTTOU, SIG_IGN);
  signal(SIGTTIN, SIG_IGN);

  setpgid(0, 0);

  pid_t old_fg = tcgetpgrp(255);
  if(tcsetpgrp(255, getpid()) == -1) {
    perror("Cannot set foreground process group");
  }

  builtin_init();
  for (;;) {
    char *prompt = getprompt();
    char *cmd = readline(prompt);
    add_history(cmd);
    free(prompt);

    if (cmd == NULL) {
      putchar('\n');
      free(cmd);
      exit(0);
    }

    pid_t ret = parse_command(cmd, -1, 0, 0);
    if (ret == -1) {
      fprintf(stderr, "Cannot parse command\n");
      free(cmd);
      continue;
    } else if (ret == 0) {
      continue;
    }
    tcsetpgrp(255, ret);

    siginfo_t siginfo;
    errno = 0;
    for(;;) {
      if (waitid(P_PGID, ret, &siginfo, WEXITED) == -1) {
        if (errno != ECHILD) {
          perror("Cannot wait for child processes");
        }
        break;
      }
      printf("siginfo.si_errno=%d, si_code=%d, s_signo=%d, si_pid=%d\n",  siginfo.si_errno, siginfo.si_code, siginfo.si_signo, siginfo.si_pid);
    }
    tcsetpgrp(255, getpid());

    free(cmd);
  }

  tcsetpgrp(255, old_fg);

  close(255);
}
