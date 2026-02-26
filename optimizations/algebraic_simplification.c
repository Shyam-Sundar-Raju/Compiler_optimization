#include "../engine.h"
#include <ctype.h>
#include <string.h>

static int algsimp_is_zero(const char *value) {
	return strcmp(value, "0") == 0;
}

static int algsimp_is_one(const char *value) {
	return strcmp(value, "1") == 0;
}

int algebraic_simplification(FunctionIR *ir) {
	int changed = 0;

	for (int index = 0; index < ir->count; index++) {
		GimpleInst *instruction = &ir->insts[index];
		if (instruction->is_dead || instruction->type != G_ASSIGN || instruction->op[0] == '\0') {
			continue;
		}

		if (strcmp(instruction->op, "+") == 0) {
			if (algsimp_is_zero(instruction->arg2)) {
				instruction->op[0] = '\0';
				instruction->arg2[0] = '\0';
				changed = 1;
			} else if (algsimp_is_zero(instruction->arg1)) {
				strcpy(instruction->arg1, instruction->arg2);
				instruction->op[0] = '\0';
				instruction->arg2[0] = '\0';
				changed = 1;
			}
		} else if (strcmp(instruction->op, "-") == 0) {
			if (algsimp_is_zero(instruction->arg2)) {
				instruction->op[0] = '\0';
				instruction->arg2[0] = '\0';
				changed = 1;
			}
		} else if (strcmp(instruction->op, "*") == 0) {
			if (algsimp_is_one(instruction->arg2)) {
				instruction->op[0] = '\0';
				instruction->arg2[0] = '\0';
				changed = 1;
			} else if (algsimp_is_one(instruction->arg1)) {
				strcpy(instruction->arg1, instruction->arg2);
				instruction->op[0] = '\0';
				instruction->arg2[0] = '\0';
				changed = 1;
			} else if (algsimp_is_zero(instruction->arg1) || algsimp_is_zero(instruction->arg2)) {
				strcpy(instruction->arg1, "0");
				instruction->op[0] = '\0';
				instruction->arg2[0] = '\0';
				changed = 1;
			}
		} else if (strcmp(instruction->op, "/") == 0) {
			if (algsimp_is_one(instruction->arg2)) {
				instruction->op[0] = '\0';
				instruction->arg2[0] = '\0';
				changed = 1;
			}
		}
	}

	return changed;
}
