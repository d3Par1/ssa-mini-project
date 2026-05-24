/*
 * minishell.c — Мінімальна оболонка командного рядка (варіант 1)
 *
 * Мініпроєкт з системного програмування в Linux
 * Курс: АСПЗ, ТВ-43, 2025/26 сем. 2
 * Автор: Аніщенко Артем
 *
 * Реалізує інтерактивний REPL (Read–Eval–Print Loop):
 *   1. Виводить запрошення "$ ".
 *   2. Читає рядок від користувача (fgets).
 *   3. Вбудована команда "exit" — завершує оболонку.
 *   4. Будь-яка інша команда → fork() + execvp() + waitpid().
 *   5. Виводить exit-код завершеного процесу.
 *   6. Усі системні виклики перевіряються; помилки — через perror().
 *
 * Збірка: gcc -Wall -Wextra -O2 -std=c11 -o minishell minishell.c
 * Запуск: ./minishell
 */

#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <ctype.h>
#include <errno.h>

#define MAX_ARGS    64
#define LINE_SIZE  512

/* Обрізати ведучі та кінцеві пробіли/переноси рядка in-place.
 * Повертає вказівник на першу непробільну літеру (може вказувати
 * всередину buf, не є окремою алокацією). */
static char *trim(char *s)
{
    while (*s && isspace((unsigned char)*s))
        s++;
    char *end = s + strlen(s);
    while (end > s && isspace((unsigned char)*(end - 1)))
        end--;
    *end = '\0';
    return s;
}

/* Розбити рядок cmd на argv за пробілами/табами.
 * Модифікує cmd in-place (вставляє '\0' між словами).
 * argv[] заповнюється вказівниками на слова; argv[n] = NULL.
 * Повертає кількість аргументів (0, якщо рядок порожній). */
static int parse_args(char *cmd, char **argv, int max_argv)
{
    int n = 0;
    char *save = NULL;
    for (char *tok = strtok_r(cmd, " \t", &save);
         tok != NULL && n < max_argv - 1;
         tok = strtok_r(NULL, " \t", &save))
    {
        argv[n++] = tok;
    }
    argv[n] = NULL;
    return n;
}

int main(void)
{
    char line[LINE_SIZE];

    /* REPL: нескінченний цикл до "exit" або EOF (Ctrl-D). */
    while (1) {
        /* Вивести запрошення; fflush — щоб рядок з'явився одразу
         * (stdout може бути буферизований). */
        fputs("$ ", stdout);
        fflush(stdout);

        /* Читати рядок. fgets повертає NULL при EOF або помилці. */
        if (fgets(line, sizeof(line), stdin) == NULL) {
            /* EOF (Ctrl-D) — завершуємо оболонку штатно. */
            putchar('\n');
            break;
        }

        /* Прибрати пробіли та '\n'. */
        char *cmd = trim(line);

        /* Порожній рядок — просто показати запрошення знову. */
        if (*cmd == '\0')
            continue;

        /* Вбудована команда: exit [код] */
        if (strncmp(cmd, "exit", 4) == 0 &&
            (cmd[4] == '\0' || isspace((unsigned char)cmd[4])))
        {
            /* Необов'язковий числовий аргумент після "exit". */
            int code = 0;
            char *arg = trim(cmd + 4);
            if (*arg != '\0') {
                char *endptr;
                long val = strtol(arg, &endptr, 10);
                if (*endptr == '\0')
                    code = (int)(val & 0xFF);
                else
                    fprintf(stderr,
                            "minishell: exit: %s: числовий аргумент необхідний\n",
                            arg);
            }
            exit(code);
        }

        /* Зовнішня команда: розбити на argv і запустити через fork/exec. */
        char *argv[MAX_ARGS];
        int argc = parse_args(cmd, argv, MAX_ARGS);
        if (argc == 0)
            continue;

        pid_t pid = fork();
        if (pid < 0) {
            /* fork() не вдався — ресурси вичерпано або інша помилка. */
            perror("minishell: fork");
            continue;
        }

        if (pid == 0) {
            /* === ДОЧІРНІЙ ПРОЦЕС === */
            execvp(argv[0], argv);

            /* execvp повертає лише при помилці. */
            fprintf(stderr, "minishell: %s: %s\n",
                    argv[0], strerror(errno));
            /* Код 127 — стандарт POSIX: "команду не знайдено". */
            _exit(127);
        }

        /* === БАТЬКІВСЬКИЙ ПРОЦЕС === */
        int status;
        if (waitpid(pid, &status, 0) == -1) {
            perror("minishell: waitpid");
            continue;
        }

        /* Відзвітувати про завершення дочірнього процесу. */
        if (WIFEXITED(status)) {
            printf("[exit code: %d]\n", WEXITSTATUS(status));
        } else if (WIFSIGNALED(status)) {
            printf("[killed by signal: %d (%s)]\n",
                   WTERMSIG(status), strsignal(WTERMSIG(status)));
        }
    }

    return 0;
}
