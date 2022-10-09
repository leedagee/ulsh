#include <getopt.h>
#include <linux/limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <assert.h>
#include <errno.h>

#define BUFFER_SIZE 2

void sigchld_handler(int sig) {
  assert(sig == SIGCHLD);
  int wstatus;
  wait(&wstatus);
}

int isdelimiter(char delimiter, char c) {
  if (delimiter) return c == delimiter;
  return c == ' ' || c == '\n';
}

void execute(int parallel, char *cargv[]) {
  pid_t pid;
  if ((pid = vfork()) == 0) {
    execvp(cargv[0], cargv); // maybe unsafe
    _exit(1);
  }

  if (pid == -1) {
    perror("Cannot vfork");
    exit(3);
  }
  int wstatus;
  if (!parallel && waitpid(pid, &wstatus, WUNTRACED) == -1) {
    perror("Error waiting for child exit");
  }
}

int main(int argc, const char *argv[]) {
  signal(SIGCHLD, sigchld_handler);
  char delimiter = '\0';
  int opt, parallel = 0;
  while ((opt = getopt(argc, (char *const *)argv, "pd:")) != -1) {
    switch (opt) {
      case 'd':
        delimiter = *optarg;
        break;
      case 'p':
        parallel = 1;
        break;
      case '?':
        fprintf(stderr, "Unknown option: %c\n", opt);
        return 1;
    }
  }

  if (optind == argc) {
    fprintf(stderr, "No commands specified.\n");
    return 2;
  }

  char buf[ARG_MAX];  // file read buffer
  char *cur = buf;    // file read cursor
  int cargc = argc - optind;
  char *cargv[128], cbuf[ARG_MAX], *ccur = cbuf;
  // child execution arguments, cargc+1 is the acturally argc for the child
  for (int i = 0; i < cargc; i++) {
    cargv[i] = (char *)argv[i + optind];
    // pre-copy arguments to cargv
  }
  cargv[cargc] = cbuf;
  cargv[cargc + 1] = NULL;

  for (ssize_t len = -1; len;) {
    cur = buf;
    len = read(0, buf, ARG_MAX);
    for (;;) {
      for (; cur - buf < len && !isdelimiter(delimiter, *cur); cur++, ccur++) {
        *ccur = *cur;
      }
      // end of buffer
      if (cur >= buf + len) break;
      cur++;

      // delimiter
      if (cbuf == ccur) continue;  // ignore zero-length item
      *ccur = '\0';

      execute(parallel, cargv);

      ccur = cbuf;
    }
  }

  errno = 0;
  while(errno != ECHILD) {
    waitpid(-1, NULL, WUNTRACED);
  }

  return 0;
}
