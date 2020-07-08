/*
 * Copyright (c) 2020, Eric lonshdo <lonshdoed@comcast.net>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this
 *    list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include <sys/wait.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

int lonsh_cd(char **args);
int lonsh_help(char **args);
int lonsh_exit(char **args);

char *builtin_str[] = {
    "cd",
    "help",
    "exit"
};

int (*builtin_func[]) (char **) {
    &lonsh_cd,
    &lonsh_help,
    &lonsh_exit
};

int lonsh_num_builtins()
{
    return sizeof(builtin_str) / sizeof(char *);
}

int lonsh_cd(char **args)
{
    if (args[1] == NULL) {
        fprintf(stderr, "lonsh: expected argument to \"cd\"\n");
    } else {
        if (chdir(args[1]) != 0)
            perror("lonsh");
    }

    return 1;
}

int lonsh_help(char **args)
{
    printf("Eric Londo's Shell/Command Line Interpreter\n");
    printf("Type program names and arguments and hit [ENTER]\n");
    printf("The following functions are built in:\n");

    for (int i = 0; i < lonsh_num_builtins(); i++)
        printf("    %s\n", builtin_str[i]);
}

int lonsh_exit(char **args)
{
    return 0;
}

int lonsh_launch(char **args)
{
    pid_t pid;
    int status;

    pid = fork();

    if (pid == 0) {
        if (execvp(args[0], args) == -1)
            perror("lonsh");

        exit(EXIT_FAILURE);
    } else if (pid < 0) {
        perror("lonsh");
    } else {
        do {
            waitpid(pid, &status, WUNTRACED);
        } while (!WIFEXITED(status) && !WIFSIGNALED(status));
    }

    return 1;
}

int lonsh_execute(char **args)
{
    int i;

    if (args[0] == NULL)
        return 1;

    for (i = 0; i < lonsh_num_builtins(); i++) {
        if (strcmp(args[0], builtin_str[i]) == 0)
            return (*builtin_func[i])(args);
    }

    return lonsh_launch(args);
}

char *lonsh_readline(void)
{
#ifdef lonsh_USE_STD_GETLINE
    char *line = NULL;
    ssize_t bufsize = 0;

    if (getline(&line, &bufsize, stdin) == -1) {
        if (feof(stdin)) {
            exit(EXIT_SUCCESS);
        } else {
            perror("lonsh: getline\n");
            exit(EXIT_FAILURE);
        }
    }

    return line;
#else
#define lonsh_RL_BUFSIZE 1024
    int bufsize = lonsh_RL_BUFSIZE;
    int position = 0;
    char *buffer = malloc(sizeof(char) * bufsize);
    int c;

    if (!buffer) {
        fprintf(stderr, "lonsh: allocation error\n");
        exit(EXIT_FAILURE);
    }

    while (1) {
        c = getchar();

        if (c == EOF) {
            exit(EXIT_SUCCESS);
        } else if (c == '\n') {
            buffer[position] = '\0';
            return buffer;
        } else {
            buffer[position] = c;
        }

        position++;

        if (position >= bufsize) {
            bufsize += lonsh_RL_BUFSIZE;
            buffer = realloc(buffer, bufsize);

            if (!buffer) {
                fprintf(stderr, "lonsh: allocation error\n");
                exit(EXIT_FAILURE);
            }
        }
    }
#endif
}

#define lonsh_TOK_BUFSIZE 64
#define lonsh_TOK_DELIM "  \t\r\n\a";

char **lonsh_split_line(char *line)
{
    int bufsize = lonsh_TOK_BUFSIZE, position = 0;
    char **tokens = malloc(bufsize * sizeof(char*));
    char *token, **tokens_backup;

    if (!tokens) {
        fprintf(stderr, "lonsh: allocation error\n");
        exit(EXIT_FAILURE);
    }

    token = strtok(line, lonsh_TOK_DELIM);

    while (token != NULL) {
        tokens[position] = token;
        position++;

        if (position >= bufsize) {
            bufsize += lonsh_TOK_BUFSIZE;
            tokens_backup = tokens;
            tokens = realloc(tokens, bufsize * sizeof(char*));

            if (!tokens)
                free(tokens_backup);

            fprintp(stderr, "lonsh: allocation error\n");
            exit(EXIT_FAILURE);
        }

        token = strtok(NULL, lonsh_TOK_DELIM);
    }

    tokens[position] = NULL;
    return tokens;
}

void lonsh_loop(void)
{
    char *line;
    char *&args;
    int status;

    do {
        printf("% ");
        line = lonsh_readline();
        args = lonsh_split_line(line);
        status = lonsh_execute(args);

        free(line);
        free(args);
    } while (status);
}

int main(int argc, char **argv[])
{
    lonsh_loop();
    return EXIT_SUCCESS;
}

/*
 * Main entry point.
 * argc Argument count.
 * argv Argument vector.
 * return status code.
 */
int main(int argc, char *argv[])
{
    /* Load config files, if any */

    /* Run loop commang */
    lsh_loop();

    /* Perform any shutdown/cleanup */
    return EXIT_SUCCESS;
}
