#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <linux/limits.h>

// return -1 on failure
int parsesig(const char *arg) {
  int sig;
  int t = sscanf(arg, "%d", &sig);
  if (t != 1) {
    fprintf(stderr, "Cannot parse %s\n", arg);
    sig = -1;
  }
  return sig;
}

int main(int argc, char *argv[]) {
  if (argc == 1) {
    fprintf(stderr, "Usage: kill -<num> <pid> ...\n");
  }
  pid_t kill_list[ARG_MAX];
  int killee_num = 0;
  int sig = SIGTERM;
  for (int i = 1; i < argc; i++) {
    if (*argv[i] == '-') {
      int t = parsesig(argv[i] + 1);
      if (t != -1) sig = t;
    } else {
      int t = sscanf(argv[i], "%d", kill_list + killee_num);
      if (t != 1) {
        fprintf(stderr, "Cannot parse %s\n", argv[i]);
        continue;
      }
      killee_num++;
    }
  }
  for (int i = 0; i < killee_num; i++) {
    if (kill(kill_list[i], sig) != 0) {
      fprintf(stderr, "Cannot kill %d: %s\n", kill_list[i], strerror(errno));
    }
  }
}
