#include "../engine.h"
#include <string.h>

typedef struct {
	char arg1[32];
	char op[8];
	char arg2[32];
	char value[32];
} CSEntry;

static int cse_match_expr(const GimpleInst *instruction, const CSEntry *entry) {
	if (strcmp(instruction->op, entry->op) != 0) {
		return 0;
	}

	if (strcmp(instruction->arg1, entry->arg1) == 0 && strcmp(instruction->arg2, entry->arg2) == 0) {
		return 1;
	}

	if ((strcmp(instruction->op, "+") == 0 || strcmp(instruction->op, "*") == 0)
		&& strcmp(instruction->arg1, entry->arg2) == 0
		&& strcmp(instruction->arg2, entry->arg1) == 0) {
		return 1;
	}

	return 0;
}

static void cse_invalidate(CSEntry *entries, int *entry_count, const char *redefined) {
	int write_index = 0;
	for (int read_index = 0; read_index < *entry_count; read_index++) {
		CSEntry current = entries[read_index];
		if (strcmp(current.value, redefined) == 0
			|| strcmp(current.arg1, redefined) == 0
			|| strcmp(current.arg2, redefined) == 0) {
			continue;
		}
		entries[write_index++] = current;
	}
	*entry_count = write_index;
}

static int cse_is_block_barrier(const GimpleInst *instruction) {
	return instruction->type == G_LABEL
		|| instruction->type == G_GOTO
		|| instruction->type == G_COND
		|| instruction->type == G_CALL;
}

int common_subexpression_elimination(FunctionIR *ir) {
	CSEntry entries[512];
	int entry_count = 0;
	int changed = 0;

	for (int index = 0; index < ir->count; index++) {
		GimpleInst *instruction = &ir->insts[index];
		if (instruction->is_dead) {
			continue;
		}

		if (cse_is_block_barrier(instruction)) {
			entry_count = 0;
			continue;
		}

		if (instruction->type != G_ASSIGN) {
			continue;
		}

		int matched = 0;
		if (instruction->op[0] != '\0') {
			for (int entry_index = 0; entry_index < entry_count; entry_index++) {
				if (!cse_match_expr(instruction, &entries[entry_index])) {
					continue;
				}

				strcpy(instruction->arg1, entries[entry_index].value);
				instruction->op[0] = '\0';
				instruction->arg2[0] = '\0';
				changed = 1;
				matched = 1;
				break;
			}
		}

		cse_invalidate(entries, &entry_count, instruction->dest);

		if (!matched && instruction->op[0] != '\0' && entry_count < 512) {
			strcpy(entries[entry_count].arg1, instruction->arg1);
			strcpy(entries[entry_count].op, instruction->op);
			strcpy(entries[entry_count].arg2, instruction->arg2);
			strcpy(entries[entry_count].value, instruction->dest);
			entry_count++;
		}
	}

	return changed;
}
