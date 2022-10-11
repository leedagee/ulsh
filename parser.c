#include <errno.h>
#include <fcntl.h>
#include <linux/limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "exec.h"

// pipe_in = 0 for no redirection
// returns the pgid/first pid of new process
int parse_command(const char *cmd, int pipe_in, int flags, pid_t pgrp) {
  char *buf = malloc(ARG_MAX), *m = buf;
  char *cargv[128], *f_in = NULL, *f_out = NULL;
  int cargc = 0,     // command argc and argv
      in_quote = 0,  // 0 for normal, 1 for in a "" quote
      in_argv = 0,   // 0 for parsing unused chars, 1 for copying into buffer
                     // 2 for waiting for copying input file name, 3 for output
      out_append = 0;
  const char *c;
  for (c = cmd; *c && *c != '|' && *c != '&'; c++, m++) {
    if (in_quote) {
      switch (*c) {
        case '"':
          in_quote ^= 1;
          m--;
          break;
        case '\\':
          if (*(c + 1) == '\\' || *(c + 1) == '"') {
            c++;
          }
          // fall through
        default:
          if (!in_argv) {
            in_argv = 1;
            cargv[cargc++] = m;
          }
          *m = *c;
      }
    } else {
      switch (*c) {
        case ' ':
          // if not in a quote, terminate this argv
          *m = '\0';
          if (in_argv == 1) in_argv = 0;
          break;
        case '<':
          *m = '\0';
          in_argv = 2;
          break;
        case '>':
          *m = '\0';
          in_argv = 3;
          if (*(c + 1) == '>') {
            out_append = 1;
            c++;  // skip one char
          } else
            out_append = 0;
          break;
        case '"':
          in_quote = 1;
          if (!in_argv) {
            in_argv = 1;
            cargv[cargc++] = m;
          }
          m--;
          break;
        default:
          if (!in_argv) {
            in_argv = 1;
            cargv[cargc++] = m;
          } else if (in_argv == 2) {
            f_in = m;
            in_argv = 1;
          } else if (in_argv == 3) {
            f_out = m;
            in_argv = 1;
          }
          *m = *c;
      }
    }
  }
  if (in_argv) {
    *m = '\0';
  }
  cargv[cargc] = NULL;

  if (cargc == 0) {
    free(buf);
    return 0;
  }

  int pipefd[2] = {-1, -1};
  if (*c == '|') {
    pipe(pipefd);
    fcntl(pipefd[0], F_SETFD, FD_CLOEXEC);
    fcntl(pipefd[1], F_SETFD, FD_CLOEXEC);
    flags |= EXECUTE_MUST_FORK;
  }

  int fd_out = pipefd[1], fd_in = pipe_in;
  if (f_out) {
    if (fd_out != -1) close(fd_out);
    fd_out =
        open(f_out, O_CREAT | O_WRONLY | (out_append ? O_APPEND : 0), 0666);
    if (fd_out == -1) {
      fprintf(stderr, "Cannot open file %s for writing: %s\n", f_out,
              strerror(errno));
      return -1;
    }
    fcntl(fd_out, F_SETFD, FD_CLOEXEC);
  }
  
  if (f_in) {
    if (fd_in != -1) close(fd_in);
    fd_in = open(f_in, O_RDONLY);
    if (fd_in == -1) {
      fprintf(stderr, "Cannot open file %s for reading: %s\n", f_in,
              strerror(errno));
      free(buf);
      return -1;
    }
    fcntl(fd_in, F_SETFD, FD_CLOEXEC);
  }
  if (fd_in != -1) flags |= EXECUTE_DUP_STDIN;
  if (fd_out != -1) flags |= EXECUTE_DUP_STDOUT;
#ifdef DEBUG_PARSER
  fprintf(stderr,
          "m-buf=%ld\ncargs=%d, in_quote=%d, in_argv=%d, out_append=%d\n",
          m - buf, cargc, in_quote, in_argv, out_append);
  fprintf(stderr, "in=\"%s\", out=\"%s\"\n", f_in, f_out);
  for (int i = 0; i < cargc; i++) {
    fprintf(stderr, "\"%s\" ", cargv[i]);
  }
  fputc('\n', stderr);
#endif
  pid_t pid = execute(flags, cargc, cargv, fd_in, fd_out, pgrp);
  free(buf);
  if (*c == '|')
    if(parse_command(c + 1, pipefd[0], flags + 1, pid) == 0)
      return 0;
  return pid;
}
