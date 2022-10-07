#pragma once

#include <sys/types.h>

#define EXECUTE_MUST_FORK 0x0001
#define EXECUTE_DUP_STDIN 0x0002
#define EXECUTE_DUP_STDOUT 0x0004
#define EXECUTE_NO_WAIT 0x0008

int execute(int flags, int argc, char *argv[], pid_t *pid_store, int fd_in,
            int fd_out);

extern int last_return_value;
