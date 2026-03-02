#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include "engine.h"
#include "cfg.c"
#include "optimizations/algebraic_simplification.c"
#include "optimizations/common_subexpression_elimination.c"
#include "optimizations/constant_folding.c"
#include "optimizations/constant_propagation.c"
#include "optimizations/copy_propagation.c"
#include "optimizations/dead_code_elimination.c"
#include "optimizations/function_cloning.c"
#include "optimizations/function_inlining.c"
#include "optimizations/induction_variable_elimination.c"
#include "optimizations/loop_fusion.c"
#include "optimizations/loop_invariant_code_motion.c"
#include "optimizations/loop_peeling.c"
#include "optimizations/loop_unrolling.c"
#include "optimizations/strength_reduction.c"
#include "optimizations/unreachable_code_elimination.c"
#include "parser.c"

static int apply_optimization_by_id(FunctionIR *ir, int opt_id) {
    switch (opt_id) {
        case 1: return algebraic_simplification(ir);
        case 2: return constant_propagation(ir);
        case 3: return copy_propagation(ir);
        case 4: return constant_folding(ir);
        case 5: return common_subexpression_elimination(ir);
        case 6: return strength_reduction(ir);
        case 7: return function_inlining(ir);
        case 8: return function_cloning(ir);
        case 9: return loop_invariant_code_motion(ir);
        case 10: return induction_variable_elimination(ir);
        case 11: return loop_fusion(ir);
        case 12: return loop_peeling(ir);
        case 13: return loop_unrolling(ir);
        case 14: return dead_code_elimination(ir);
        case 15: return unreachable_code_elimination(ir);
        default: return 0;
    }
}

void run_pipeline(FunctionIR *ir, int opt_id) {
    int changed = 1;
    int iterations = 0;
    const int max_iterations = 20;

    while (changed && iterations < max_iterations) {
        changed = 0;

        if (opt_id > 0) {
            changed |= apply_optimization_by_id(ir, opt_id);
        } else {
            changed |= algebraic_simplification(ir);
            changed |= constant_propagation(ir);
            changed |= copy_propagation(ir);
            changed |= constant_folding(ir);
            changed |= common_subexpression_elimination(ir);
            changed |= strength_reduction(ir);
            changed |= function_inlining(ir);
            changed |= function_cloning(ir);
            changed |= loop_invariant_code_motion(ir);
            changed |= induction_variable_elimination(ir);
            changed |= loop_fusion(ir);
            changed |= loop_peeling(ir);
            changed |= loop_unrolling(ir);
            changed |= dead_code_elimination(ir);
        }

        iterations++;
    }

    if (opt_id > 0) {
        printf("Optimization #%d completed in %d iterations.\n", opt_id, iterations);
    } else {
        printf("Optimization completed in %d iterations.\n", iterations);
    }
}

void print_optimized_gimple(FunctionIR ir) {
    int printed = 0;
    for (int i = 0; i < ir.count; i++) {
        GimpleInst *ins = &ir.insts[i];
        if (ins->is_dead) continue; // Skip dead instructions

        switch (ins->type) {
            case G_GOTO:
                printf("goto %s;\n", ins->dest);
                printed++;
                break;
            case G_LABEL:
                printf("%s:\n", ins->dest);
                printed++;
                break;
            case G_COND:
                printf("if (%s %s %s) goto %s;\n", ins->arg1, ins->op, ins->arg2, ins->dest);
                printed++;
                break;
            case G_CALL:
                if (strlen(ins->dest) > 0) {
                    if (strlen(ins->arg2) > 0)
                        printf("%s = call %s(%s);\n", ins->dest, ins->arg1, ins->arg2);
                    else
                        printf("%s = call %s;\n", ins->dest, ins->arg1);
                } else {
                    if (strlen(ins->arg2) > 0)
                        printf("call %s(%s);\n", ins->arg1, ins->arg2);
                    else
                        printf("call %s;\n", ins->arg1);
                }
                printed++;
                break;
            case G_ASSIGN:
                if (strlen(ins->op) > 0)
                    printf("%s = %s %s %s;\n", ins->dest, ins->arg1, ins->op, ins->arg2);
                else
                    printf("%s = %s;\n", ins->dest, ins->arg1);
                printed++;
                break;
            // Handle other instruction types as needed
            default:
                break;
        }
    }
    fprintf(stderr, "Printed %d non-dead instructions\n", printed);
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <source-file> [optimization-id]\n", argv[0]);
        fprintf(stderr, "Optimization IDs: 1=algebraic_simplification, 2=constant_propagation, 3=copy_propagation, 4=constant_folding, 5=common_subexpression_elimination, 6=strength_reduction, 7=function_inlining, 8=function_cloning, 9=loop_invariant_code_motion, 10=induction_variable_elimination, 11=loop_fusion, 12=loop_peeling, 13=loop_unrolling, 14=dead_code_elimination, 15=unreachable_code_elimination\n");
        return 1;
    }

    const char *source_file = argv[1];
    int opt_id = 0;

    if (argc >= 3) {
        char *endptr = NULL;
        errno = 0;
        long parsed = strtol(argv[2], &endptr, 10);

        if (errno != 0 || endptr == argv[2] || *endptr != '\0' || parsed < 1 || parsed > 15) {
            fprintf(stderr, "Invalid optimization-id '%s'. Must be an integer from 1 to 15.\n", argv[2]);
            return 1;
        }

        opt_id = (int)parsed;
    }

    // 1. Invoke GCC to dump raw GIMPLE
    const char *gimple_file = "/tmp/compiler_dump.gimple";
    char cmd[768];
    snprintf(cmd, sizeof(cmd), "gcc -fdump-tree-gimple-raw=%s %s -o /tmp/compiler_input.out", gimple_file, source_file);
    if (system(cmd) != 0) {
        fprintf(stderr, "Failed to generate GIMPLE dump for %s\n", source_file);
        return 1;
    }

    // 2. Parse the .gimple file generated by GCC
    FunctionIR ir = parse_gimple_file(gimple_file);
    fprintf(stderr, "Parsed %d instructions from GIMPLE\n", ir.count);
    printf("Original GIMPLE:\n");
    print_optimized_gimple(ir); // Print before optimization for debugging

    // 3. Run the optimization logic
    run_pipeline(&ir, opt_id);

    // 4. Output optimized GIMPLE
    printf("\nOptimized GIMPLE:\n");
    print_optimized_gimple(ir);

    return 0;
}