#include "utils.h"
#include <string.h>
#include <ctype.h>

void trim_whitespace(char *str) {
    char *start = str;
    while (isspace(*start)) start++;
    char *end = str + strlen(str) - 1;
    while (end > start && isspace(*end)) end--;
    *(end+1) = '\0';
    memmove(str, start, end - start + 2);
}

int is_background(char **args, int *argc) {
    if (*argc > 0 && strcmp(args[*argc - 1], "&") == 0) {
        args[*argc - 1] = NULL;
        (*argc)--;
        return 1;
    }
    return 0;
}
