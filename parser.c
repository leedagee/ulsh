#include "parser.h"

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
ssize_t parse_command(const char *cmd, struct parse_result_t **comp) {
  *comp = NULL;
  char *buf = malloc(ARG_MAX), *m = buf;
  char *cargv[128], *f_in = NULL, *f_out = NULL;
  int cargc = 0,     // command argc and argv
      in_quote = 0,  // 0 for normal, 1 for in a "" quote
      in_argv = 0,   // 0 for parsing unused chars, 1 for copying into buffer
                     // 2 for waiting for copying input file name, 3 for output
      out_append = 0,
      cont = 1;  // continue reading
  const char *c;
  for (c = cmd; *c && cont; c++, m++) {
    if (in_quote) {
      switch (*c) {
        case '"':
          in_quote = 0;
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
        case '|':
        case '&':
        case ';':
          m--;
          c--;
          cont = 0;
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
  if (in_argv == 1) {
    *m = '\0';
  } else if (in_argv) {
    fprintf(stderr, "Syntax error: unknown redirection\n");
    free(buf);
    return -1;
  }
  if (in_quote) {
    fprintf(stderr, "Syntax error: quote not closed\n");
    free(buf);
    return -1;
  }
  cargv[cargc] = NULL;

  struct parse_result_t *res = malloc(sizeof(struct parse_result_t));
  memset(res, 0, sizeof(*res));

  res->buf = buf;
  res->f_in = f_in;
  res->f_out = f_out;
  res->argc = cargc;
  for (int i = 0; i <= cargc; i++) {
    res->argv[i] = cargv[i];
  }
  if (out_append) {
    res->flags |= PARSE_RESULT_APPEND;
  }

  ssize_t len = c - cmd + 1, nlen = 0;

  if (cargc == 0) {
    return len;
  }

  *comp = res;

  switch (*c) {
    case '|':
      nlen = parse_command(c + 1, &res->next);
      if (nlen == -1) return -1;
      res->flags |=
          (res->next->flags & PARSE_RESULT_BACKGROUND) | PARSE_RESULT_PIPE;
      return len + nlen;
    case '&':
      res->flags |= PARSE_RESULT_BACKGROUND;
      return len;
    case ';':
    case '\0':
      return len;
    default:
      fprintf(stderr, "parse failed on '%c' with unknown reason\n", *c);
      return -1;
  }
}

void free_parse_result(struct parse_result_t *res) {
  if (res->next) {
    free_parse_result(res->next);
  }
  free(res->buf);
  free(res);
}
