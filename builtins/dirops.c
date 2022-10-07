#include "dirops.h"
#include "builtin.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <linux/limits.h>

BUILTIN_DECLARE(pwd) {
  char buf[PATH_MAX];
  getcwd(buf, PATH_MAX);
  puts(buf);
  return 0;
}

BUILTIN_DECLARE(cd) {
  const char* target;
  if (argc == 1) {
    target = getenv("HOME");
    if (target == NULL) target = "/";
  } else {
    target = argv[1];
  }
  if (chdir(target) == -1) {
    perror("Cannot change directory");
  }
  return -1;
}
