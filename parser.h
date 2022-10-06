#pragma once

#include <sys/types.h>

int parse_command(const char *cmd, int pipe_in, int flags, pid_t pid_store[]);