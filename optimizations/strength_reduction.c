#include "../engine.h"
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int sr_is_number(const char *text) {
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

static int sr_log2_if_power_of_two(int value) {
	if (value <= 0 || (value & (value - 1)) != 0) {
		return -1;
	}

	int shift = 0;
	while (value > 1) {
		value >>= 1;
		shift++;
	}
	return shift;
}

int strength_reduction(FunctionIR *ir) {
	int changed = 0;

	for (int index = 0; index < ir->count; index++) {
		GimpleInst *instruction = &ir->insts[index];
		if (instruction->is_dead || instruction->type != G_ASSIGN || instruction->op[0] == '\0') {
			continue;
		}

		if (strcmp(instruction->op, "*") == 0) {
			if (sr_is_number(instruction->arg2)) {
				int factor = atoi(instruction->arg2);
				int shift = sr_log2_if_power_of_two(factor);
				if (shift >= 0) {
					strcpy(instruction->op, "<<");
					snprintf(instruction->arg2, sizeof(instruction->arg2), "%d", shift);
					changed = 1;
				}
			} else if (sr_is_number(instruction->arg1)) {
				int factor = atoi(instruction->arg1);
				int shift = sr_log2_if_power_of_two(factor);
				if (shift >= 0) {
					char variable_operand[32];
					strcpy(variable_operand, instruction->arg2);
					strcpy(instruction->arg1, variable_operand);
					strcpy(instruction->op, "<<");
					snprintf(instruction->arg2, sizeof(instruction->arg2), "%d", shift);
					changed = 1;
				}
			}
		} else if (strcmp(instruction->op, "/") == 0 && sr_is_number(instruction->arg2)) {
			int divisor = atoi(instruction->arg2);
			int shift = sr_log2_if_power_of_two(divisor);
			if (shift >= 0) {
				strcpy(instruction->op, ">>");
				snprintf(instruction->arg2, sizeof(instruction->arg2), "%d", shift);
				changed = 1;
			}
		}
	}

	return changed;
}
