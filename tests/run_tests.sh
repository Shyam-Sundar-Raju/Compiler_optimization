#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_DIR="$(cd "$SCRIPT_DIR/.." && pwd)"
TEST_DIR="$PROJECT_DIR/tests"
OUTPUT_DIR="$TEST_DIR/optimization_outputs"
COMPILER_BIN="$PROJECT_DIR/main"

if [[ ! -x "$COMPILER_BIN" ]]; then
    echo "Error: compiler binary not found or not executable at $COMPILER_BIN"
    echo "Build first, for example: gcc main.c -o main"
    exit 1
fi

mkdir -p "$OUTPUT_DIR"

opt_names=(
    "algebraic_simplification"
    "constant_propagation"
    "copy_propagation"
    "constant_folding"
    "common_subexpression_elimination"
    "strength_reduction"
    "function_inlining"
    "function_cloning"
    "loop_invariant_code_motion"
    "induction_variable_elimination"
    "loop_fusion"
    "loop_peeling"
    "loop_unrolling"
    "dead_code_elimination"
    "unreachable_code_elimination"
)

for i in $(seq 1 ${#opt_names[@]}); do
    technique_name="${opt_names[$((i - 1))]}"
    test_file="$TEST_DIR/${technique_name}.c"
    output_file="$OUTPUT_DIR/${technique_name}.txt"
    opt_id="$i"

    if [[ ! -f "$test_file" ]]; then
        echo "Missing file: $test_file" > "$output_file"
        continue
    fi

    {
        echo "Command: $COMPILER_BIN $test_file $opt_id"
        echo "----------------------------------------"
        "$COMPILER_BIN" "$test_file" "$opt_id"
    } > "$output_file" 2>&1

    echo "Saved: $output_file"
done

echo "All done. Outputs are in: $OUTPUT_DIR"