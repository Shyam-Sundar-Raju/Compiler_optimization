#include "../engine.h"
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int is_integer_literal(const char *text) {
    int index = 0;
    if (text[0] == '-') {
        index = 1;
    }
    if (text[index] == '\0') {
        return 0;
    }
    for (; text[index] != '\0'; index++) {
        if (!isdigit((unsigned char)text[index])) {
            return 0;
        }
    }
    return 1;
}

static int eval_condition(const char *op, int lhs, int rhs, int *result) {
    if (strcmp(op, "==") == 0) *result = (lhs == rhs);
    else if (strcmp(op, "!=") == 0) *result = (lhs != rhs);
    else if (strcmp(op, "<") == 0) *result = (lhs < rhs);
    else if (strcmp(op, "<=") == 0) *result = (lhs <= rhs);
    else if (strcmp(op, ">") == 0) *result = (lhs > rhs);
    else if (strcmp(op, ">=") == 0) *result = (lhs >= rhs);
    else return 0;
    return 1;
}

int constant_folding(FunctionIR *ir) {
    int changed = 0;
    for (int i = 0; i < ir->count; i++) {
        GimpleInst *ins = &ir->insts[i];
        
        // Logic: If both args are integer literals, compute now
        if (ins->type == G_ASSIGN && is_integer_literal(ins->arg1) && is_integer_literal(ins->arg2)) {
            int a = atoi(ins->arg1);
            int b = atoi(ins->arg2);
            int res = 0;
            int evaluated = 1; // Flag to track if we actually folded something

            if (strcmp(ins->op, "+") == 0) res = a + b;
            else if (strcmp(ins->op, "-") == 0) res = a - b;
            else if (strcmp(ins->op, "*") == 0) res = a * b;
            else if (strcmp(ins->op, "/") == 0 && b != 0) res = a / b; // Prevent div by zero
            else evaluated = 0; // Unknown operator, don't fold

            if (evaluated) {
                // Update instruction to a simple assignment
                sprintf(ins->arg1, "%d", res);
                strcpy(ins->op, "");
                strcpy(ins->arg2, "");
                changed = 1;
            }
        }

        if (ins->type == G_COND && is_integer_literal(ins->arg1) && is_integer_literal(ins->arg2)) {
            int lhs = atoi(ins->arg1);
            int rhs = atoi(ins->arg2);
            int cond_value = 0;

            if (eval_condition(ins->op, lhs, rhs, &cond_value)) {
                if (cond_value) {
                    ins->type = G_GOTO;
                    ins->arg1[0] = '\0';
                    ins->arg2[0] = '\0';
                    ins->op[0] = '\0';
                } else {
                    ins->is_dead = 1;
                }
                changed = 1;
            }
        }
    }
    return changed;
}