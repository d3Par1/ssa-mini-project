/*
 * pipeline.c — Виконавець конвеєра команд (варіант 11)
 *
 * Мініпроєкт з системного програмування в Linux
 * Курс: АСПЗ, ТВ-43, 2025/26 сем. 2
 * Автор: Степаненко Назар Юрійович
 *
 * Парсить рядок виду "cmd1 a1 | cmd2 a2 | cmd3" та запускає N процесів,
 * з'єднаних (N-1) ядерними пайпами. Реалізує аналог shell-конвеєра.
 *
 * Збірка: gcc -Wall -Wextra -O2 -std=c11 -o pipeline pipeline.c
 * Запуск: ./pipeline "ls /usr/bin | grep gcc | wc -l"
 *
 * Код завершення:
 *   = exit-код останньої команди у конвеєрі (як у bash без pipefail).
 *   = 2 у разі неправильних аргументів CLI.
 *   = 1 у разі системної помилки (fork, pipe, malloc).
 */

#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <ctype.h>
#include <errno.h>

#define MAX_COMMANDS 16
#define MAX_ARGS     32

/* Розділити рядок на токени за роздільником.
 * Працює in-place (модифікує str), пише вказівники у out[].
 * Обрізає leading/trailing whitespace кожного токена.
 * Повертає кількість заповнених елементів (out[n] = NULL термінатор). */
static int tokenize(char *str, const char *delim, char **out, int max_out)
{
    int n = 0;
    char *save = NULL;
    for (char *tok = strtok_r(str, delim, &save);
         tok != NULL && n < max_out - 1;
         tok = strtok_r(NULL, delim, &save)) {

        while (*tok && isspace((unsigned char)*tok))
            tok++;
        char *end = tok + strlen(tok);
        while (end > tok && isspace((unsigned char)*(end - 1)))
            end--;
        *end = '\0';

        if (*tok)
            out[n++] = tok;
    }
    out[n] = NULL;
    return n;
}

/* Закрити обидва дескриптори всіх пайпів у масиві.
 * Викликається і в кожній дитині (одразу після dup2), і в батьку
 * (одразу після останнього fork). Це КРИТИЧНА процедура — див. розділ
 * "Пастка з дескрипторами" у README.md. */
static void close_all_pipes(int pipes[][2], int npipes)
{
    for (int i = 0; i < npipes; i++) {
        close(pipes[i][0]);
        close(pipes[i][1]);
    }
}

int main(int argc, char *argv[])
{
    if (argc != 2) {
        fprintf(stderr,
                "Використання: %s \"cmd1 arg1 | cmd2 arg2 | ...\"\n"
                "Приклад:      %s \"ls /usr/bin | grep gcc | wc -l\"\n",
                argv[0], argv[0]);
        return 2;
    }

    /* strdup, бо strtok_r модифікує рядок in-place. argv[1] належить ядру
     * (через execve), модифікувати його ми не маємо права. */
    char *line = strdup(argv[1]);
    if (!line) {
        perror("strdup");
        return 1;
    }

    /* Етап 1: розділити вхід за '|' на окремі команди. */
    char *cmd_strs[MAX_COMMANDS];
    int ncmd = tokenize(line, "|", cmd_strs, MAX_COMMANDS);
    if (ncmd == 0) {
        fprintf(stderr, "pipeline: пустий вхід\n");
        free(line);
        return 2;
    }
    if (ncmd >= MAX_COMMANDS - 1) {
        fprintf(stderr, "pipeline: забагато команд (максимум %d)\n",
                MAX_COMMANDS - 1);
        free(line);
        return 2;
    }

    /* Етап 2: для кожної команди розбити на argv. */
    char **argvs[MAX_COMMANDS] = {0};
    for (int i = 0; i < ncmd; i++) {
        argvs[i] = calloc(MAX_ARGS, sizeof(char *));
        if (!argvs[i]) {
            perror("calloc");
            free(line);
            return 1;
        }
        tokenize(cmd_strs[i], " \t", argvs[i], MAX_ARGS);
        if (argvs[i][0] == NULL) {
            fprintf(stderr,
                    "pipeline: пуста команда у позиції %d\n", i + 1);
            free(argvs[i]);
            free(line);
            return 2;
        }
    }

    /* Етап 3: створити (ncmd - 1) пайпів. Кожен пайп з'єднує сусідні
     * процеси: pipes[i][1] (write-end) → pipes[i][0] (read-end). */
    int pipes[MAX_COMMANDS - 1][2];
    int npipes = ncmd - 1;
    for (int i = 0; i < npipes; i++) {
        if (pipe(pipes[i]) == -1) {
            perror("pipe");
            return 1;
        }
    }

    /* Етап 4: форк-цикл. Кожна дитина:
     *  - dup2 потрібні pipe-end'и на STDIN/STDOUT,
     *  - закриває ВСІ pipe-fd (свої дублікати після dup2 + чужі),
     *  - execvp. */
    pid_t pids[MAX_COMMANDS];
    for (int i = 0; i < ncmd; i++) {
        pid_t pid = fork();
        if (pid < 0) {
            perror("fork");
            return 1;
        }
        if (pid == 0) {
            /* === ДИТЯЧИЙ ПРОЦЕС i === */
            if (i > 0) {
                if (dup2(pipes[i - 1][0], STDIN_FILENO) == -1) {
                    perror("dup2 stdin");
                    _exit(127);
                }
            }
            if (i < ncmd - 1) {
                if (dup2(pipes[i][1], STDOUT_FILENO) == -1) {
                    perror("dup2 stdout");
                    _exit(127);
                }
            }
            close_all_pipes(pipes, npipes);

            execvp(argvs[i][0], argvs[i]);

            /* execvp повертає -1 лише при помилці (зазвичай ENOENT). */
            fprintf(stderr, "pipeline: %s: %s\n",
                    argvs[i][0], strerror(errno));
            _exit(127);
        }
        pids[i] = pid;
    }

    /* === БАТЬКІВСЬКИЙ ПРОЦЕС === */
    /* Закрити всі pipe-fd у батька. Без цього останні writer'и не
     * отримають EOF на read-стороні: ядро рахує кількість fd, які
     * вказують на write-end, і відправляє EOF лише коли вона стає 0. */
    close_all_pipes(pipes, npipes);

    /* Етап 5: дочекатися кожної дитини, відрепортити коди завершення. */
    int last_status = 0;
    for (int i = 0; i < ncmd; i++) {
        int status;
        if (waitpid(pids[i], &status, 0) == -1) {
            perror("waitpid");
            continue;
        }
        if (WIFEXITED(status)) {
            int ec = WEXITSTATUS(status);
            fprintf(stderr,
                    "[pid=%d] %s -> exit=%d\n",
                    pids[i], argvs[i][0], ec);
            if (i == ncmd - 1)
                last_status = ec;
        } else if (WIFSIGNALED(status)) {
            int sig = WTERMSIG(status);
            fprintf(stderr,
                    "[pid=%d] %s -> вбито сигналом %d (%s)\n",
                    pids[i], argvs[i][0], sig, strsignal(sig));
            if (i == ncmd - 1)
                last_status = 128 + sig;
        }
    }

    /* Прибирання. */
    for (int i = 0; i < ncmd; i++)
        free(argvs[i]);
    free(line);

    return last_status;
}
