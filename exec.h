#pragma once

#include <sys/types.h>

#define EXECUTE_MUST_FORK 0x0001
#define EXECUTE_DUP_STDIN 0x0002
#define EXECUTE_DUP_STDOUT 0x0004

pid_t execute(int flags, int argc, char *argv[], int fd_in, int fd_out,
              pid_t pgrp);

extern int last_return_value;
