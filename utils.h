#ifndef UTILS_H
#define UTILS_H

#define MAX_ARGS 100
#define MAX_CMD 1024

void trim_whitespace(char *str);
int is_background(char **args, int *argc);

#endif
