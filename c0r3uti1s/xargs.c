#include <getopt.h>
#include <linux/limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#define BUFFER_SIZE 2

int isdelimiter(char delimiter, char c) {
  if (delimiter) return c == delimiter;
  return c == ' ' || c == '\n';
}

int main(int argc, const char *argv[]) {
  char delimiter = '\0';
  int opt;
  while ((opt = getopt(argc, (char *const *)argv, "d:")) != -1) {
    switch (opt) {
      case 'd':
        delimiter = *optarg;
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
// 0x4cd9ec
  for (int i = 0; i < cargc; i++) {
    cargv[i] = (char *)argv[i + optind];
    // pre-copy arguments to cargv
  }
  cargv[cargc] = cbuf;
  cargv[cargc + 1] = NULL;

  for (ssize_t len = 0; !feof(stdin); ) {
    cur = buf;
    len = fread(buf, 1, ARG_MAX, stdin);
    for (;;) {
      for (; cur - buf < len && !isdelimiter(delimiter, *cur); cur++, ccur++) {
        *ccur = *cur;
      }
      // end of buffer
      if (cur >= buf + len && !feof(stdin)) break;
      cur++;

      // delimiter
      if (cbuf == ccur) continue;  // ignore zero-length item
      *ccur = '\0';

      //fprintf(stderr, ">>> %s\n", cbuf);

      pid_t pid;
      if ((pid = vfork()) == 0) {
        execvp(cargv[0], cargv);
        _exit(1);
      }

      if (pid == -1) {
        perror("Cannot vfork");
        exit(3);
      }
      int wstatus;
      if (waitpid(pid, &wstatus, WUNTRACED) == -1) {
        perror("Error waiting for child exit");
      }

      ccur = cbuf;

      if (feof(stdin)) break;
    }
  }

  return 0;
}
