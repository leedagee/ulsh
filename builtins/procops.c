#include "procops.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

extern char **environ;

BUILTIN_DECLARE(exit) {
  if (argc == 1) {
    exit(0);
    return 1;
  }
  assert(argc == 2);
  int return_code;
  if (sscanf(argv[1], "%d", &return_code) != 1) {
    fprintf(stderr, "Cannot convert %s to a interger.\n", argv[1]);
  }
  exit(return_code);
  return 1;
}

BUILTIN_DECLARE(export) {
  if (argc == 1) {
    // output current environment variables
    for (char **env = environ; *env; env++) {
      puts(*env);
    }
  } else {
    char buf[1024];
    strncpy(buf, argv[1], 1024);
    for (char *c = buf; *c; c++) {
      if (*c == '=') {
        *c = '\0';
        if (setenv(buf, c + 1, 1) != 0) {
          perror("Cannot set environment variable");
          return 1;
        }
        return 0;
      }
    }
    fprintf(stderr, "Cannot parse environment variable.\n");
  }
  return 1;
}

BUILTIN_DECLARE(exec) {
  if (execvp(argv[1], (char *const *)(argv + 1)) == -1) {
    perror("Cannot exec");
  }
  return 1;
}