#include "builtins.h"
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

int handle_builtin(char **args) {
    if (!args[0]) return 0;

    if (strcmp(args[0], "cd") == 0) {
        if (!args[1]) chdir(getenv("HOME"));
        else if (chdir(args[1]) != 0) perror("cd failed");
        return 1;
    }

    if (strcmp(args[0], "exit") == 0) {
        exit(0);
    }

    return 0; // not a builtin
}
