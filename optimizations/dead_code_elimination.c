#include "../engine.h"
#include <ctype.h>
#include <string.h>

static int dce_is_number(const char *text) {
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

static int dce_find(char used[][32], int used_count, const char *name) {
	for (int index = 0; index < used_count; index++) {
		if (strcmp(used[index], name) == 0) {
			return index;
		}
	}
	return -1;
}

static void dce_add(char used[][32], int *used_count, const char *name) {
	if (name[0] == '\0' || dce_is_number(name) || dce_find(used, *used_count, name) >= 0 || *used_count >= 512) {
		return;
	}

	strcpy(used[*used_count], name);
	(*used_count)++;
}

static void dce_remove(char used[][32], int *used_count, const char *name) {
	int found_index = dce_find(used, *used_count, name);
	if (found_index < 0) {
		return;
	}

	for (int index = found_index; index + 1 < *used_count; index++) {
		strcpy(used[index], used[index + 1]);
	}
	(*used_count)--;
}

int dead_code_elimination(FunctionIR *ir) {
	char used[512][32];
	int used_count = 0;
	int changed = 0;

	for (int index = ir->count - 1; index >= 0; index--) {
		GimpleInst *instruction = &ir->insts[index];
		if (instruction->is_dead) {
			continue;
		}

		if (instruction->type == G_ASSIGN) {
			int side_effect_assignment =
				strchr(instruction->dest, '[') != NULL ||
				strchr(instruction->dest, ']') != NULL ||
				strchr(instruction->dest, '*') != NULL;

			int removable_temp = instruction->dest[0] == '_';
			int live_dest = dce_find(used, used_count, instruction->dest) >= 0;
			if (!live_dest && removable_temp && !side_effect_assignment) {
				instruction->is_dead = 1;
				changed = 1;
				continue;
			}

			dce_remove(used, &used_count, instruction->dest);
			dce_add(used, &used_count, instruction->arg1);
			if (instruction->op[0] != '\0') {
				dce_add(used, &used_count, instruction->arg2);
			}
		} else if (instruction->type == G_GOTO || instruction->type == G_COND) {
			dce_add(used, &used_count, instruction->arg1);
			dce_add(used, &used_count, instruction->arg2);
		}
	}

	return changed;
}
