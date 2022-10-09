#include <errno.h>
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

void usage() {
  fprintf(stderr, "Usage: ln [-s] <source> <target>\n");
  exit(1);
}

int main(int argc, char *argv[]) {
  int opt;
  int do_symlink = 0;
  while ((opt = getopt(argc, argv, "s")) != -1) {
    switch (opt) {
      case 's':
        do_symlink = 1;
        break;
      default:
        usage();
    }
  }

  if (optind + 2 != argc) {
    usage();
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
