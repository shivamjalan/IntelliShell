#ifndef EXECUTOR_H
#define EXECUTOR_H

void execute_command(char **args, int argc, char *input_file, char *output_file, int append, int pipe_index, int background);

#endif
