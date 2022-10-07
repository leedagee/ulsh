#include <errno.h>
#include <getopt.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

int main(int argc, const char *argv[]) {
  int opt;
  int do_symlink = 0;
  while ((opt = getopt(argc, (char *const *)argv, "s")) != -1) {
    switch (opt) {
      case 's':
        do_symlink = 1;
        break;
    }
  }

  if (optind + 2 != argc) {
    fprintf(stderr, "ln takes exactly two arguments.\n");
    return 1;
  }

  if (do_symlink) {
    if (symlink(argv[optind], argv[optind + 1]) == -1) {
      perror("Cannot create symlink");
      return 1;
    }
  } else {
    if (link(argv[optind], argv[optind + 1]) == -1) {
      perror("Cannot create hard link");
      return 1;
    }
  }
}
