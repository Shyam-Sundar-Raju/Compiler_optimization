#include "../engine.h"
#include <ctype.h>
#include <string.h>

typedef struct {
	char name[32];
	char source[32];
} CopyBinding;

static int copyprop_is_number(const char *text) {
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

static int copyprop_find(CopyBinding *bindings, int count, const char *name) {
	for (int index = 0; index < count; index++) {
		if (strcmp(bindings[index].name, name) == 0) {
			return index;
		}
	}
	return -1;
}

static void copyprop_set(CopyBinding *bindings, int *count, const char *name, const char *source) {
	int found_index = copyprop_find(bindings, *count, name);
	if (found_index >= 0) {
		strcpy(bindings[found_index].source, source);
		return;
	}

	if (*count >= 512) {
		return;
	}

	strcpy(bindings[*count].name, name);
	strcpy(bindings[*count].source, source);
	(*count)++;
}

static void copyprop_resolve_operand(char *operand, CopyBinding *bindings, int count, int *changed) {
	int guard = 0;
	while (guard < 16) {
		int found_index = copyprop_find(bindings, count, operand);
		if (found_index < 0) {
			return;
		}

		if (strcmp(operand, bindings[found_index].source) == 0) {
			return;
		}

		strcpy(operand, bindings[found_index].source);
		*changed = 1;
		guard++;
	}
}

static int copyprop_is_block_barrier(const GimpleInst *instruction) {
	return instruction->type == G_LABEL
		|| instruction->type == G_GOTO
		|| instruction->type == G_COND
		|| instruction->type == G_CALL;
}

int copy_propagation(FunctionIR *ir) {
	CopyBinding bindings[512];
	int binding_count = 0;
	int changed = 0;

	for (int index = 0; index < ir->count; index++) {
		GimpleInst *instruction = &ir->insts[index];
		if (instruction->is_dead) {
			continue;
		}

		if (copyprop_is_block_barrier(instruction)) {
			binding_count = 0;
			continue;
		}

		if (instruction->type != G_ASSIGN) {
			continue;
		}

		copyprop_resolve_operand(instruction->arg1, bindings, binding_count, &changed);
		if (instruction->op[0] != '\0') {
			copyprop_resolve_operand(instruction->arg2, bindings, binding_count, &changed);
		}

        for (int b = 0; b < binding_count; ) {
            if (strcmp(bindings[b].name, instruction->dest) == 0 ||
                strcmp(bindings[b].source, instruction->dest) == 0) {
                for (int k = b; k + 1 < binding_count; k++) {
                    bindings[k] = bindings[k + 1];
                }
                binding_count--;
            } else {
                b++;
            }
        }
		if (instruction->op[0] == '\0' && !copyprop_is_number(instruction->arg1)) {
			copyprop_set(bindings, &binding_count, instruction->dest, instruction->arg1);
		}
	}

	return changed;
}
