#include "prompt.h"

#include <malloc.h>
#include <pwd.h>
#include <stdio.h>
#include <sys/time.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

char *getprompt() {
  char buf[256];
  size_t avail = PROMPT_LENGTH;
  char *prompt = malloc(PROMPT_LENGTH);
  char *cursor = prompt;
  size_t consumed;

  if (gethostname(buf, sizeof(buf)) == -1) {
    perror("Cannot get hostname");
    return "> ";
  }

  uid_t uid = getuid();
  struct passwd *pw = getpwuid(uid);
  if (pw == NULL) {
    perror("Cannot get passwd entry");
    return "> ";
  }
  consumed = snprintf(cursor, avail, "%s@%s ", pw->pw_name, buf);
  cursor += consumed;
  avail -= consumed;

  if (getcwd(buf, sizeof(buf)) == NULL) {
    perror("Cannot get current working directory");
    return "> ";
  }
  consumed = snprintf(cursor, avail, "%s ", buf);
  cursor += consumed;
  avail -= consumed;

  time_t raw_time;
  time(&raw_time);
  struct tm *bd_time = localtime(&raw_time);
  snprintf(cursor, avail, "%02d:%02d:%02d\n> ", bd_time->tm_hour,
           bd_time->tm_min, bd_time->tm_sec);
  cursor += consumed;
  avail -= consumed;

  return prompt;
}