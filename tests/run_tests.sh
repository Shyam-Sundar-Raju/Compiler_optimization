#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_DIR="$(cd "$SCRIPT_DIR/.." && pwd)"
TEST_DIR="$PROJECT_DIR/tests"
OUTPUT_DIR="$TEST_DIR/outputs"
COMPILER_BIN="$PROJECT_DIR/main"

if [[ ! -x "$COMPILER_BIN" ]]; then
    echo "Error: compiler binary not found or not executable at $COMPILER_BIN"
    echo "Build first, for example: gcc main.c -o main"
    exit 1
fi

mkdir -p "$OUTPUT_DIR"

for i in $(seq 1 15); do
    test_file="$TEST_DIR/test${i}.c"
    output_file="$OUTPUT_DIR/output${i}.txt"

    if [[ ! -f "$test_file" ]]; then
        echo "Missing file: $test_file" > "$output_file"
        continue
    fi

    {
        echo "Command: $COMPILER_BIN $test_file"
        echo "----------------------------------------"
        "$COMPILER_BIN" "$test_file"
    } > "$output_file" 2>&1

    echo "Saved: $output_file"
done

echo "All done. Outputs are in: $OUTPUT_DIR"