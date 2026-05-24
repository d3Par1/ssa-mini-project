# Makefile для minishell (варіант 1)
# Той же стиль, що в pipeline/Makefile.

CC      := gcc
CFLAGS  := -Wall -Wextra -O2 -std=c11
TARGET  := minishell
SRC     := minishell.c

.PHONY: all clean run

all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) -o $@ $<

# Швидкий smoke-test: запустити оболонку з однією командою через echo
run: $(TARGET)
	echo "ls -la" | ./$(TARGET)

clean:
	rm -f $(TARGET)
