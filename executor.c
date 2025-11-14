// executor.c
#include "executor.h"
#include "builtins.h"
#include "ai_helper.h"   // <- AI helper header
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <errno.h>

// ---------- Helper: safe PATH iteration (returns best match name) ----------
static void find_best_match_in_path(const char *wrong_cmd, char *best_match, int *best_score) {
    char *path = getenv("PATH");
    if (!path) return;

    char *path_copy = strdup(path);            // DO NOT strtok() getenv() directly
    if (!path_copy) return;

    char *token = strtok(path_copy, ":");
    while (token != NULL) {
        DIR *dir = opendir(token);
        if (dir) {
            struct dirent *entry;
            while ((entry = readdir(dir)) != NULL) {
                // compute simple similarity (you may use levenshtein in ai_helper instead)
                int score = 0;
                const char *a = wrong_cmd;
                const char *b = entry->d_name;
                int la = strlen(a), lb = strlen(b);
                int limit = la < lb ? la : lb;
                for (int i = 0; i < limit; ++i) if (a[i] == b[i]) ++score;
                // boost short exact prefixes
                if (strncmp(a, b, limit) == 0) score += 5;

                if (score > *best_score) {
                    *best_score = score;
                    strncpy(best_match, entry->d_name, 127);
                    best_match[127] = '\0';
                }
            }
            closedir(dir);
        }
        token = strtok(NULL, ":");
    }
    free(path_copy);
}

// ---------- Execute Command ----------
void execute_command(char **args, int argc, char *input_file, char *output_file, int append, int pipe_index, int background) {
    if (args == NULL || args[0] == NULL) return;

    if (handle_builtin(args)) return;

    // Handle single pipe case (your existing approach)
    if (pipe_index != -1) {
        int fd[2];
        if (pipe(fd) == -1) { perror("pipe"); return; }

        pid_t pid1 = fork();
        if (pid1 == 0) {
            // child 1: left side -> write end
            dup2(fd[1], STDOUT_FILENO);
            close(fd[0]); close(fd[1]);
            // restore default signals if needed
            execvp(args[0], args);
            // exec failed in child: exit with 127 so parent can suggest
            perror("exec failed");
            _exit(127);
        }

        pid_t pid2 = fork();
        if (pid2 == 0) {
            // child 2: right side -> read end
            dup2(fd[0], STDIN_FILENO);
            close(fd[0]); close(fd[1]);
            execvp(args[pipe_index + 1], &args[pipe_index + 1]);
            perror("exec failed");
            _exit(127);
        }

        // parent
        close(fd[0]); close(fd[1]);
        int status1=0, status2=0;
        waitpid(pid1, &status1, 0);
        waitpid(pid2, &status2, 0);

        // If left child failed with 127, suggest
        if (WIFEXITED(status1) && WEXITSTATUS(status1) == 127) {
            suggest_command(args[0]);
        }
        // If right child failed with 127, suggest for right command
        if (WIFEXITED(status2) && WEXITSTATUS(status2) == 127) {
            suggest_command(args[pipe_index + 1]);
        }
        return;
    }

    // No pipe: single command
    pid_t pid = fork();
    if (pid == 0) {
        // Child: set up redirections then exec
        if (input_file) {
            int fd = open(input_file, O_RDONLY);
            if (fd < 0) { perror("input file"); _exit(1); }
            dup2(fd, STDIN_FILENO);
            close(fd);
        }

        if (output_file) {
            int fd;
            if (append) fd = open(output_file, O_WRONLY | O_CREAT | O_APPEND, 0644);
            else fd = open(output_file, O_WRONLY | O_CREAT | O_TRUNC, 0644);
            if (fd < 0) { perror("output file"); _exit(1); }
            dup2(fd, STDOUT_FILENO);
            close(fd);
        }

        // Exec: if fails, exit with 127
        execvp(args[0], args);
        perror("exec failed");
        _exit(127);
    } else if (pid > 0) {
        int status = 0;
        if (!background) {
            waitpid(pid, &status, 0);
            // if child exited with 127 => command not found -> suggest
            if (WIFEXITED(status) && WEXITSTATUS(status) == 127) {
                suggest_command(args[0]);
            }
        } else {
            // background: don't block; optionally record job
            printf("[Background pid %d] %s\n", pid, args[0]);
            // optionally reap using SIGCHLD handler elsewhere
        }
    } else {
        perror("fork failed");
    }
}
