#pragma once
#include <stddef.h>

#define PROMPT_LENGTH 1024

// returns a prompt string.
// the returned string is allocated by malloc(3)
// the caller should free(3) it
char* getprompt();