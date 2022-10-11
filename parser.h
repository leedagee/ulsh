#pragma once

#include <sys/types.h>

#define PARSE_RESULT_BACKGROUND   0x001
#define PARSE_RESULT_PIPE         0x002
#define PARSE_RESULT_APPEND       0x004

struct parse_result_t {
  char *buf;
  char *f_in;
  char *f_out;
  struct parse_result_t *next;
  int argc;
  int flags;
  char *argv[4];
};

ssize_t parse_command(const char *cmd, struct parse_result_t **comp);
void free_parse_result(struct parse_result_t *);
