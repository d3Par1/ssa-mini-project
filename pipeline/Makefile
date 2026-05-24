# Makefile для pipeline (варіант 11)
# Тиh же стиль, що в LR1-LR11.

CC      := gcc
CFLAGS  := -Wall -Wextra -O2 -std=c11
TARGET  := pipeline
SRC     := pipeline.c

.PHONY: all clean run demo

all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) -o $@ $<

# Швидкий smoke-test
run: $(TARGET)
	./$(TARGET) "ls | wc -l"

# Запуск усіх демо-сценаріїв; вивід пишеться у demo/output_*.txt
demo: $(TARGET)
	bash demo/run_demos.sh

clean:
	rm -f $(TARGET) demo/output_*.txt
