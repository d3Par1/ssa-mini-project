#!/bin/bash
# run_demos.sh — запускає всі демо-сценарії варіанту 11.
# Вивід кожного сценарію зберігається у demo/output_<N>.txt
# та виводиться на екран (для скріншотів).
#
# Запуск з кореня pipeline/:
#   make demo
# або вручну:
#   bash demo/run_demos.sh

set +e  # не падати при помилці окремого демо
cd "$(dirname "$0")/.."

BIN=./pipeline
if [ ! -x "$BIN" ]; then
    echo "[!] Бінарника $BIN немає. Спершу: make" >&2
    exit 1
fi

run_demo() {
    local n="$1" desc="$2" cmd="$3"
    local out="demo/output_${n}.txt"
    {
        echo "================ Демо $n: $desc ================"
        echo "\$ ./pipeline \"$cmd\""
        echo "------ stdout+stderr ------"
        "$BIN" "$cmd" 2>&1
        local ec=$?
        echo "------ exit code = $ec ------"
        echo
    } | tee "$out"
}

# Підготувати вхідний файл для демо 2
cat > demo/input.txt <<'EOF'
This is a test file for the pipeline mini-project.
The pipeline executor parses commands separated by '|'.
Each pipe character creates a connection between processes.
We use fork(), pipe(), dup2(), execvp(), and waitpid().
The PIPELINE itself is built on top of these system calls.
EOF

run_demo 1 "Простий 2-pipe"          "ls /usr/bin | wc -l"
run_demo 2 "3-pipe з фільтром"        "cat demo/input.txt | grep -i pipeline | wc -l"
run_demo 3 "4-pipe багатоступеневий"  "ps aux | grep root | sort -k1 | head -5"
run_demo 4 "Помилка у першій команді" "cat nonexistent_file.xyz | wc -l"
run_demo 5 "Помилка execvp всередині" "ls | nosuchcommand_xyz | wc -l"
run_demo 6 "Перетворення тексту"      "echo hello world | tr a-z A-Z | rev"
run_demo 7 "Одна команда без pipe"    "ls -la demo"

echo "Демо завершено. Перевірте файли demo/output_*.txt"
