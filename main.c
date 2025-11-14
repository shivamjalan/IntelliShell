#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include "parser.h"
#include "executor.h"

void sigint_handler(int sig) {
    // Ignore Ctrl+C inside the shell
    printf("\nType 'exit' to quit.\n");
}

int main() {
    signal(SIGINT, sigint_handler);

    char input[1024];
    char *args[100];
    int argc;

    printf("Welcome to SmartShellX 🤖\n");
    printf("Type 'help' for commands, or 'exit' to quit.\n\n");

    while (1) {
        printf("mysh> ");
        fflush(stdout);

        if (!fgets(input, sizeof(input), stdin))
            break;

        // remove newline
        input[strcspn(input, "\n")] = 0;

        if (strlen(input) == 0)
            continue;

        // parse input
        char *input_file = NULL, *output_file = NULL;
        int append = 0, pipe_index = -1, background = 0;

        argc = parse_input(input, args, &input_file, &output_file, &append, &pipe_index, &background);

        if (argc == 0)
            continue;

        if (strcmp(args[0], "exit") == 0)
            break;

        execute_command(args, argc, input_file, output_file, append, pipe_index, background);
    }

    printf("Goodbye 👋\n");
    return 0;
}
