#include <errno.h>
#include <libgen.h>
#include <linux/limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#define BUFFER_SIZE 16384

// will close both in and out
void copy(FILE *in, FILE *out) {
  char buf[BUFFER_SIZE];
  while (!feof(in)) {
    size_t len = fread(buf, 1, BUFFER_SIZE, in);
    if (len == 0 && errno) {
      perror("Cannot read source file");
      fclose(in);
      fclose(out);
      return;
    }
    char *buf_end = buf + len;
    while (len) {
      len -= fwrite(buf_end - len, 1, len, out);
      if (errno) {
        perror("Cannot write target file");
        fclose(in);
        fclose(out);
        return;
      }
    }
  }
  fclose(in);
  fclose(out);
  return;
}

int main(int argc, const char *argv[]) {
  if (argc <= 2) {
    fprintf(stderr, "cp takes two or more arguments.\n");
    return 1;
  }

  int is_target_directory = 0;
  struct stat st;
  if (stat(argv[argc - 1], &st) == -1) {
    if (errno != ENOENT) {
      perror("Cannot stat target");
      return 1;
    }
  }
  if ((st.st_mode & S_IFMT) == S_IFDIR) {
    is_target_directory = 1;
  }
  if (argc != 3 && (st.st_mode & S_IFMT) != S_IFDIR) {
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
    chmod(out, st.st_mode);
  }
}
