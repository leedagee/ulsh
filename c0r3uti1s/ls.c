#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#define LS_OPTION_LONG 0x0001
#define LS_OPTION_SINGLEENT 0x0002
#define LS_OPTION_ALL 0x0004

void usage() {
  puts("Usage: ls [-l] <file1> <file2> ...");
  exit(1);
}

void printlong(struct stat *st, const char *filename) {
  printf("%6o\t% 4d\t% 4d\t% 10ld\t%s\n", st->st_mode, st->st_uid, st->st_gid,
         st->st_size, filename);
}

void printdirent(int parent_fd, struct dirent *ent, int options) {
  if (!(options & LS_OPTION_ALL) && *ent->d_name == '.') {
    return;
  }

  if (options & LS_OPTION_LONG) {
    struct stat st;
    if (fstatat(parent_fd, ent->d_name, &st, 0) == -1) {
      perror("Cannot stat file");
      return;
    }
    printlong(&st, ent->d_name);
  } else {
    puts(ent->d_name);
  }
}

void list(const char *target, int options) {
  struct stat st;
  if (stat(target, &st) == -1) {
    perror("Failed to stat target file");
    return;
  }

  if ((st.st_mode & S_IFMT) == S_IFDIR) {
    if ((options & LS_OPTION_SINGLEENT) == 0) {
      printf("\n%s:\n", target);
    }
    int fd = open(target, O_RDONLY);
    if (fd == -1) {
      fprintf(stderr, "Cannot open directory %s: %s\n", target,
              strerror(errno));
      return;
    }
    DIR *dirp = fdopendir(fd);
    struct dirent *ent;
    while ((ent = readdir(dirp)) != NULL) {
      printdirent(fd, ent, options);
    }
    closedir(dirp);
    close(fd);
  } else if (options & LS_OPTION_LONG) {
    printlong(&st, target);
  } else {
    puts(target);
  }
}

int main(int argc, char *argv[]) {
  int options = 0;
  int c;
  while ((c = getopt(argc, (char *const *)argv, "la")) != -1) {
    switch (c) {
      case 'l':
        options |= LS_OPTION_LONG;
        break;
      case 'a':
        options |= LS_OPTION_ALL;
        break;
      default:
        usage();
    }
  }

  if (optind + 1 > argc) {
    options |= LS_OPTION_SINGLEENT;
  }
  if (optind == argc) {
    list(".", options);
    return 0;
  }
  for (; optind < argc; optind++) {
    list(argv[optind], options);
  }
  return 0;
}
