#include <errno.h>
#include <libgen.h>
#include <linux/limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

// will close both in and out
void copy(FILE *in, FILE *out) {
  char buf[BUFSIZ];
  while (!feof(in)) {
    size_t len = fread(buf, 1, BUFSIZ, in);
    if (len != BUFSIZ && ferror(in) && errno) {
      perror("Cannot read source file");
      return;
    }
    size_t wlen = fwrite(buf, 1, len, out);
    if (wlen != len && ferror(out) && errno) {
      perror("Cannot write target file");
      return;
    }
  }
  return;
}

int main(int argc, char *argv[]) {
  if (argc <= 2) {
    fprintf(stderr, "Usage: cp <source1> [source2] ... <target>\n");
    return 1;
  }

  int is_target_directory = 0;
  struct stat st;
  if (stat(argv[argc - 1], &st) == -1) {
    if (errno != ENOENT) {
      perror("Cannot stat target");
      return 1;
    }
  } else if ((st.st_mode & S_IFMT) == S_IFDIR) {
    is_target_directory = 1;
  } else if (argc > 3) {
    fprintf(stderr, "Target is not a directory.\n");
    return 1;
  }

  for (int i = 1; i + 1 < argc; i++) {
    if (stat(argv[i], &st) == -1) {
      fprintf(stderr, "Cannot stat source %s: %s\n", argv[i], strerror(errno));
      continue;
    }
    if ((st.st_mode & S_IFMT) != S_IFREG) {
      fprintf(stderr, "%s is not a regular file, which is not supported.\n",
              argv[i]);
      continue;
    }
    if (access(argv[i], R_OK)) {
      fprintf(stderr, "Cannot read file %s: %s\n", argv[i], strerror(errno));
      continue;
    }
    char *out = (char *)argv[argc - 1];
    if (is_target_directory) {
      char *bn = basename((char *)argv[i]);
      char out_file[PATH_MAX];
      sprintf(out_file, "%s/%s", argv[argc - 1], bn);
      out = out_file;
    }

    FILE *fin = fopen(argv[i], "r");
    if (fin == NULL) {
      perror("Failed to open source file");
      continue;
    }
    FILE *fout = fopen(out, "w+");
    if (fout == NULL) {
      perror("Failed to open target file");
      fclose(fin);
      continue;
    }
    copy(fin, fout);
    fclose(fin);
    fclose(fout);
    chmod(out, st.st_mode);
  }
}
