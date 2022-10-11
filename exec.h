#pragma once

#include <sys/types.h>

#include "parser.h"

#define EXECUTE_MUST_FORK 0x0001
#define EXECUTE_DUP_STDIN 0x0002
#define EXECUTE_DUP_STDOUT 0x0004

pid_t execute(int flags, int argc, char *argv[], int fd_in, int fd_out,
              pid_t pgrp);

int run_parsed(struct parse_result_t *res, pid_t *pid, pid_t pgrp, int fd_in);

extern int last_return_value;
