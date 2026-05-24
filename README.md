# Мініпроєкт з системного програмування в Linux

**Курс:** Архітектура системного програмного забезпечення (АСПЗ)
**Група:** ТВ-43
**Рік:** 2025/26, семестр 2

## Учасники й розподіл варіантів

| Студент | GitHub | Варіант | Тема |
|---|---|---|---|
| Степаненко Назар Юрійович | [@d3Par1](https://github.com/d3Par1) | **11** | Реалізація конвеєра команд (`pipe()` + `dup2()` + `fork()` + `exec()`) |
| Аніщенко Артем | [@anishchenko64](https://github.com/anishchenko64) | **1** | Мініоболонка командного рядка (`fork()` + `execvp()` + `waitpid()`) |

Кожен учасник здає індивідуальний звіт відповідно до загальних вимог методички.
Спільний репозиторій вибрано для того, щоб дві частини утворили цілісне демо
(«мінібаш»: інтерпретатор + конвеєр) і працювали з однаковими інструкціями збірки.

## Структура репозиторію

```
SSA-MiniProject/
├── README.md            # цей файл — огляд проєкту
├── Makefile             # збирає обидві частини
├── docs/
│   └── SPEC.md          # спільний дизайн-документ
├── pipeline/            # частина Назара (варіант 11)
│   ├── pipeline.c
│   ├── Makefile
│   ├── README.md        # = звіт
│   ├── demo/            # демо-скрипти та вхідні файли
│   └── screenshots/     # PNG для звіту
└── minishell/           # частина Артема (варіант 1)
    └── README.md        # placeholder, наповнюється Артемом
```

## Швидка збірка

```bash
make all     # збирає pipeline (Назар) та minishell (Артем, якщо є)
make clean   # очищує бінарні файли
```

Або по-частинах:

```bash
make -C pipeline
make -C minishell
```

## Запуск

```bash
./pipeline/pipeline "ls /usr/bin | grep gcc | wc -l"
./minishell/minishell        # інтерактивний REPL
```

## Середовище збірки

Цільова платформа — **Ubuntu 22.04 + gcc 11+, glibc**. Для збірки під Windows
використовуйте WSL2 або Docker із Dockerfile з батьківського каталогу
(`University-Repository/Year-2/Semester-2/SSA/LR/Dockerfile`).

```bash
# Через Docker (з кореня репозиторію):
docker run --rm -v "$(pwd):/src" -w /src ubuntu:22.04 bash -c \
  "apt update && apt install -y build-essential && make all"
```
