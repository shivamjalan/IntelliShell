// parser.c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "parser.h"
#include "utils.h"

// Ensure these sizes match your usage
#define MAX_TOKENS 128
#define MAX_TOKEN_LEN 512

// Insert spaces around special symbols so tokenization is simpler
void preprocess_input(char *input) {
    char buffer[4096];
    int j = 0;
    for (int i = 0; input[i] != '\0' && j < (int)sizeof(buffer) - 2; i++) {
        // Handle ">>" as special two-char operator
        if (input[i] == '>' && input[i+1] == '>') {
            if (j > 0 && buffer[j-1] != ' ') buffer[j++] = ' ';
            buffer[j++] = '>';
            buffer[j++] = '>';
            i++; // skip second '>'
            if (input[i+1] != ' ' && input[i+1] != '\0') buffer[j++] = ' ';
            continue;
        }

        if (input[i] == '>' || input[i] == '<' || input[i] == '|' || input[i] == '&') {
            if (j > 0 && buffer[j-1] != ' ') buffer[j++] = ' ';
            buffer[j++] = input[i];
            if (input[i+1] != ' ' && input[i+1] != '\0') buffer[j++] = ' ';
        } else {
            buffer[j++] = input[i];
        }
    }
    buffer[j] = '\0';
    strcpy(input, buffer);
}

// Tokenize input preserving quoted strings. Returns malloc'd array of char* (need to free strings).
// argc_out returns the token count (excluding final NULL).
char **tokenize(const char *in, int *argc_out) {
    char *s = strdup(in);
    char *ptr = s;
    char *tokens[MAX_TOKENS];
    int argc = 0;

    while (*ptr && argc < MAX_TOKENS - 1) {
        while (isspace((unsigned char)*ptr)) ptr++;
        if (*ptr == '\0') break;

        if (*ptr == '"' || *ptr == '\'') {
            char quote = *ptr++;
            char *start = ptr;
            while (*ptr && *ptr != quote) ptr++;
            int len = ptr - start;
            tokens[argc++] = strndup(start, len);
            if (*ptr == quote) ptr++;
        } else {
            char *start = ptr;
            while (*ptr && !isspace((unsigned char)*ptr)) ptr++;
            int len = ptr - start;
            tokens[argc++] = strndup(start, len);
        }
    }

    // Build a malloc'd array
    char **arr = malloc((argc + 1) * sizeof(char *));
    for (int i = 0; i < argc; ++i) arr[i] = tokens[i];
    arr[argc] = NULL;
    *argc_out = argc;
    free(s);
    return arr;
}

// The function main.c expects
int parse_input(char *input, char **args, char **input_file, char **output_file,
                int *append, int *pipe_index, int *background) {
    // initialize outputs
    *input_file = NULL;
    *output_file = NULL;
    *append = 0;
    *pipe_index = -1;
    *background = 0;

    preprocess_input(input);

    int tokc = 0;
    char **tokens = tokenize(input, &tokc);
    if (!tokens) return 0;

    int argi = 0;
    for (int i = 0; i < tokc; ++i) {
        if (strcmp(tokens[i], "<") == 0) {
            if (i + 1 < tokc) {
                *input_file = strdup(tokens[i+1]);
                i++;
            }
        } else if (strcmp(tokens[i], ">>") == 0) {
            if (i + 1 < tokc) {
                *output_file = strdup(tokens[i+1]);
                *append = 1;
                i++;
            }
        } else if (strcmp(tokens[i], ">") == 0) {
            if (i + 1 < tokc) {
                *output_file = strdup(tokens[i+1]);
                *append = 0;
                i++;
            }
        } else if (strcmp(tokens[i], "|") == 0) {
            // mark pipe index as where the next argv element will begin
            *pipe_index = argi;
            // place a NULL in args to separate for execvp usage later
            args[argi++] = NULL;
        } else if (strcmp(tokens[i], "&") == 0) {
            *background = 1;
        } else {
            // normal argument
            args[argi++] = strdup(tokens[i]);
        }
    }

    args[argi] = NULL;

    // free token array (strings already moved/copied into args or freed below)
    for (int i = 0; i < tokc; ++i) free(tokens[i]);
    free(tokens);

    return argi; // argc (count of non-NULL entries in args up to first NULL)
}
