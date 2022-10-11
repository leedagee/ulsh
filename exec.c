#include "exec.h"

#include <errno.h>
#include <spawn.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include "builtins/builtin.h"

int last_return_value;

extern char **environ;

pid_t execute(int flags, int argc, char *argv[], int fd_in, int fd_out,
              pid_t pgrp) {
  BUILTIN_HANDLER handler = builtin_find_entry(argv[0]);
  pid_t pid;
  if (handler != NULL) {
    if (flags & EXECUTE_MUST_FORK) {
      switch (pid = fork()) {
        case -1:
          perror("Cannot fork");
          break;
        case 0:
          if (flags & EXECUTE_DUP_STDIN) dup2(fd_in, 0);
          if (flags & EXECUTE_DUP_STDOUT) dup2(fd_out, 1);
          setpgid(0, pgrp);
          exit(handler(argc, argv));
          break;
      }
    } else {
      last_return_value = handler(argc, argv);
      return 0;
    }
  } else {
    posix_spawn_file_actions_t file_actions;
    posix_spawnattr_t attr;
    if (posix_spawn_file_actions_init(&file_actions) ||
        ((flags & EXECUTE_DUP_STDIN) &&
         posix_spawn_file_actions_adddup2(&file_actions, fd_in, 0)) ||
        ((flags & EXECUTE_DUP_STDOUT) &&
         posix_spawn_file_actions_adddup2(&file_actions, fd_out, 1)))
      perror("Cannot set posix_spawn file_actions");
    if (posix_spawnattr_init(&attr) || posix_spawnattr_setpgroup(&attr, pgrp) ||
        posix_spawnattr_setflags(&attr, POSIX_SPAWN_SETPGROUP))
      perror("Cannot set posix_spawn attrs");
    if (posix_spawnp(&pid, argv[0], &file_actions, &attr, argv, environ))
      perror("Cannot perform posix_spawn");
    if (posix_spawnattr_destroy(&attr) ||
        posix_spawn_file_actions_destroy(&file_actions))
      perror("Cannot destroy posix_spawn arguments");
  }

  if (flags & EXECUTE_DUP_STDIN) close(fd_in);
  if (flags & EXECUTE_DUP_STDOUT) close(fd_out);

  return pid;
}
