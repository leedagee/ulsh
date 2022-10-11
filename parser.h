#pragma once

#include <sys/types.h>

struct parse_result_t {
  char *buf;
  char **argv;
  int argc;
  char *f_in;
  char *f_out;
  struct parse_result_t *next;
};

int parse_command(const char *cmd, int pipe_in, int flags, pid_t first);
