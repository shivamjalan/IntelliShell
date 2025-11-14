// ai_helper.c
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "ai_helper.h"

// ---------------- Known Commands ----------------
const char *known_cmds[] = {
    "ls", "cat", "echo", "grep", "pwd", "cd", "mkdir", "rmdir",
    "cp", "mv", "rm", "touch", "head", "tail", "chmod", "chown",
    "sleep", "clear", "man", "exit", "whoami", "date", "uptime",
    "ps", "kill", "history", "find", "diff", "more", "less", NULL
};

// ---------------- Levenshtein Distance ----------------
int levenshtein(const char *s1, const char *s2) {
    int len1 = strlen(s1), len2 = strlen(s2);
    int **dp = malloc((len1 + 1) * sizeof(int *));
    for (int i = 0; i <= len1; i++)
        dp[i] = malloc((len2 + 1) * sizeof(int));

    for (int i = 0; i <= len1; i++) dp[i][0] = i;
    for (int j = 0; j <= len2; j++) dp[0][j] = j;

    for (int i = 1; i <= len1; i++) {
        for (int j = 1; j <= len2; j++) {
            int cost = (s1[i - 1] == s2[j - 1]) ? 0 : 1;
            int del = dp[i - 1][j] + 1;
            int ins = dp[i][j - 1] + 1;
            int sub = dp[i - 1][j - 1] + cost;

            int min = del < ins ? del : ins;
            dp[i][j] = (min < sub) ? min : sub;
        }
    }

    int result = dp[len1][len2];
    for (int i = 0; i <= len1; i++) free(dp[i]);
    free(dp);
    return result;
}

// ---------------- Suggest Command ----------------
void suggest_command(const char *cmd) {
    int best_dist = 999;
    const char *best_match = NULL;

    for (int i = 0; known_cmds[i] != NULL; i++) {
        int dist = levenshtein(cmd, known_cmds[i]);
        if (dist < best_dist) {
            best_dist = dist;
            best_match = known_cmds[i];
        }
    }

    if (best_dist <= 2 && best_match) {
        printf("🤖 AI Suggestion: Did you mean '%s' ?\n", best_match);
    } else {
        printf("🤖 AI Suggestion: No similar command found.\n");
    }
}
