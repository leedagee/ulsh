#include <errno.h>
#include <fcntl.h>
#include <libgen.h>
#include <linux/fs.h>
#include <linux/limits.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <unistd.h>

int renameat2(int olddirfd, const char *oldpath, int newdirfd,
              const char *newpath, unsigned int flags) {
  return syscall(SYS_renameat2, olddirfd, oldpath, newdirfd, newpath, flags);
}

int main(int argc, char *argv[]) {
  if (argc <= 2) {
    fprintf(stderr, "Usage: mv <file1> [file2] ... <target>\n");
    return 1;
  }

  /*
    An strange fact is that the mv(1) in coreutils
    try renameat2 without checking the target, but with
    a NOREPLACE flag and when it gets EEXIST, a stat on
    target is performed. (strace observed, not source
    code digging)
    INTERESTING!
    let's implement it in this way, with a regression on
    the portablity.
  */

  for (int i = 1; i + 1 < argc; i++) {
    int ret = renameat2(AT_FDCWD, argv[i], AT_FDCWD, argv[argc - 1],
                        RENAME_NOREPLACE);
    if (ret == 0) continue;

    if (errno != EEXIST) {
      perror("Cannot rename file");
      continue;
    }

    struct stat st;
    if (stat(argv[argc - 1], &st) == -1) {
      perror("Cannot stat target file");
      continue;
    }

    char *target = argv[argc - 1];
    if ((st.st_mode & S_IFMT) == S_IFDIR) {
      char buf[PATH_MAX];
      sprintf(buf, "%s/%s", argv[argc - 1], basename(argv[i]));
      target = buf;
    }

    if (rename(argv[i], target) == -1) {
      perror("Cannot rename file");
    }
  }
}
