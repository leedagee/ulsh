#include <errno.h>
#include <fcntl.h>
#include <readline/history.h>
#include <readline/readline.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/select.h>
#include <sys/signalfd.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include "builtins/builtin.h"
#include "exec.h"
#include "jobs.h"
#include "parser.h"
#include "prompt.h"

void sigchld_handler(int sig, siginfo_t *si, void *data) {
  write(sigchld_pipes[1], ".", 1);
#ifdef DEBUG_CHLD
  write(2, "ON SIGCHLD!\n", 12);
#endif
}

static void handle_line(char *cmd) {
  if (cmd == NULL) {
    putchar('\n');
    exit(0);
  }

  char *cur = cmd, *end = cmd + strlen(cmd);
  add_history(cmd);

  while (cur < end) {
    struct parse_result_t *res;
    ssize_t len = parse_command(cur, &res);
    if (res == NULL) {
      cur += len;
      continue;
    }
    char *comm = malloc(len + 1);
    strncpy(comm, cur, len);
    if (len == -1) break;
    struct procstat *proc = NULL;
    if (run_parsed(res, &proc, 0, -1, comm) == EXECUTE_RESULT_BUILTIN)
      free(comm);
    if (res != NULL) free_parse_result(res);
    cur += len;

    while (foreground != getpid()) {
      fd_set fds;
      FD_ZERO(&fds);
      FD_SET(sigchld_pipes[0], &fds);
      errno = 0;
      int r = select(FD_SETSIZE, &fds, NULL, NULL, NULL);
      if (errno == EINTR) continue;
      if (r == -1) perror("Cannot wait on fds");
      if (FD_ISSET(sigchld_pipes[0], &fds)) {
        reap_children();
      }
    }
  }
  free(cmd);
}

int main(int argc, char *argv[]) {
  int fd_tty = open("/dev/tty", O_RDWR);
  if (fd_tty == -1) perror("Cannot set fd 255 to the console");
  dup2(fd_tty, 255);
  if (fd_tty == -1) perror("Cannot set fd 255 to the console");
  close(fd_tty);

  pipe(sigchld_pipes);
  int old_flags = fcntl(sigchld_pipes[0], F_GETFL);
  fcntl(sigchld_pipes[0], F_SETFL, O_NONBLOCK | old_flags);

  signal(SIGTTOU, SIG_IGN);
  signal(SIGTTIN, SIG_IGN);
  signal(SIGTSTP, SIG_IGN);
  signal(SIGQUIT, SIG_IGN);

  struct sigaction sa;
  sa.sa_flags = SA_RESTART | SA_SIGINFO;
  sa.sa_sigaction = sigchld_handler;
  sigemptyset(&sa.sa_mask);
  sigaddset(&sa.sa_mask, SIGCHLD);
  sigaction(SIGCHLD, &sa, NULL);

  setpgid(0, 0);

  pid_t old_fg = tcgetpgrp(255);
  if (tcsetpgrp(255, getpid()) == -1) {
    perror("Cannot set foreground process group");
  }
  foreground = getpid();

  builtin_init();

  char *prompt = getprompt();
  rl_callback_handler_install(prompt, handle_line);

  fd_set fds;

  for (;;) {
    FD_ZERO(&fds);
    FD_SET(0, &fds);
    FD_SET(sigchld_pipes[0], &fds);
    int r = select(FD_SETSIZE, &fds, NULL, NULL, NULL);
    if (r == -1) {
      if (errno == EINTR) continue;
      perror("Cannot wait on fds");
    } else if (FD_ISSET(fileno(rl_instream), &fds)) {
      free(prompt);
      rl_set_prompt(prompt = getprompt());
      rl_callback_read_char();
    } else if (FD_ISSET(sigchld_pipes[0], &fds)) {
      reap_children();
    }
  }

  tcsetpgrp(255, old_fg);

  close(255);
}
