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

static void handle_line(char *cmd) {
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
    struct procstat *proc = NULL;
    siginfo_t si;
    si.si_pid = 0;
    int r = run_parsed(res, &proc, 0, -1);
    if (r == 1) goto finalize;
    if (proc == NULL) goto finalize;
    pid_t pid = proc->pid;
    if (!(res->flags & PARSE_RESULT_BACKGROUND)) {
      for (;;) {
        fd_set fds;
        FD_ZERO(&fds);
        FD_SET(fd_chld, &fds);
        int r = select(FD_SETSIZE, &fds, NULL, NULL, NULL);
        if (r == -1) {
          perror("Cannot wait on fds");
          goto finalize;
        }
        if (FD_ISSET(fd_chld, &fds)) {
          reap_children();
        }
        while (waitid(P_PGID, pid, &si, WEXITED | WSTOPPED | WNOHANG) != -1)
          if (si.si_code == CLD_STOPPED) {
            pid_t pgid = getpgid(si.si_pid);
            struct job_t **job = find_job(pgid), *_job;
            if (job == NULL) {
              job = &_job;
              *job = add_job(pgid);
            }
            struct procstat *now = NULL;
            for (struct procstat *p = (*job)->procstats; p; p = p->next) {
              if (p == NULL) break;
              if (p->pid == si.si_pid) {
                now = p;
              }
            }
            if (now == NULL) now = malloc(sizeof(struct procstat));
            now->pid = si.si_pid;
            now->status = PROCSTAT_STOPPED;
            now->retval = -1;
            now->next = (*job)->procstats;
            (*job)->procstats = now;
            goto finalize;
          }
        if (errno == ECHILD) break;
      }
    }

  finalize:
    if (!(res->flags & PARSE_RESULT_BACKGROUND) && r != 1) delete_proc(proc);
    tcsetpgrp(255, getpid());
    if (res != NULL) free_parse_result(res);
    cur += s;
  }
  free(cmd);

  char *prompt = getprompt();
  puts(prompt);
  free(prompt);
}

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
  if (tcsetpgrp(255, getpid()) == -1) {
    perror("Cannot set foreground process group");
  }

  builtin_init();
  rl_callback_handler_install("> ", handle_line);

  fd_set fds;

  sigset_t mask;
  sigemptyset(&mask);
  sigaddset(&mask, SIGCHLD);
  sigprocmask(SIG_BLOCK, &mask, NULL);
  fd_chld = signalfd(-1, &mask, SFD_CLOEXEC | SFD_NONBLOCK);

  for (;;) {
    FD_ZERO(&fds);
    FD_SET(0, &fds);
    FD_SET(fd_chld, &fds);
    int r = select(FD_SETSIZE, &fds, NULL, NULL, NULL);
    if (r == -1) {
      perror("Cannot wait on fds");
    } else if (FD_ISSET(fileno(rl_instream), &fds)) {
      rl_callback_read_char();
    } else if (FD_ISSET(fd_chld, &fds)) {
      reap_children();
    }
  }

  tcsetpgrp(255, old_fg);

  close(255);
}
