#include "../engine.h"
#include <ctype.h>
#include <string.h>

typedef struct {
	char name[32];
	char value[32];
} ConstantBinding;

static int cprop_is_number(const char *text) {
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

static int cprop_find(ConstantBinding *bindings, int count, const char *name) {
	for (int index = 0; index < count; index++) {
		if (strcmp(bindings[index].name, name) == 0) {
			return index;
		}
	}
	return -1;
}

static void cprop_kill(ConstantBinding *bindings, int *count, const char *name) {
	int found_index = cprop_find(bindings, *count, name);
	if (found_index < 0) {
		return;
	}

	for (int index = found_index; index + 1 < *count; index++) {
		bindings[index] = bindings[index + 1];
	}
	(*count)--;
}

static void cprop_set(ConstantBinding *bindings, int *count, const char *name, const char *value) {
	int found_index = cprop_find(bindings, *count, name);
	if (found_index >= 0) {
		strcpy(bindings[found_index].value, value);
		return;
	}

	if (*count >= 512) {
		return;
	}

	strcpy(bindings[*count].name, name);
	strcpy(bindings[*count].value, value);
	(*count)++;
}

static int cprop_is_block_barrier(const GimpleInst *instruction) {
	return instruction->type == G_LABEL
		|| instruction->type == G_GOTO
		|| instruction->type == G_COND
		|| instruction->type == G_CALL;
}

// NEW HELPER: Find or add a variable to the counting array
static int get_assign_count(char names[][32], int counts[], int *unique_vars, const char *name) {
    for (int i = 0; i < *unique_vars; i++) {
        if (strcmp(names[i], name) == 0) return counts[i];
    }
    return 0; // Not found
}

static void increment_assign_count(char names[][32], int counts[], int *unique_vars, const char *name) {
    for (int i = 0; i < *unique_vars; i++) {
        if (strcmp(names[i], name) == 0) {
            counts[i]++;
            return;
        }
    }
    if (*unique_vars < 512) {
        strcpy(names[*unique_vars], name);
        counts[*unique_vars] = 1;
        (*unique_vars)++;
    }
}

int constant_propagation(FunctionIR *ir) {
    ConstantBinding bindings[512];
    int binding_count = 0;
    int changed = 0;
    
    // Arrays for our pre-pass
    char var_names[512][32];
    int var_counts[512] = {0};
    int unique_vars = 0;

    // --- NEW PRE-PASS: Count how many times every variable is assigned ---
    for (int index = 0; index < ir->count; index++) {
        GimpleInst *instruction = &ir->insts[index];
        if (!instruction->is_dead && instruction->type == G_ASSIGN) {
            increment_assign_count(var_names, var_counts, &unique_vars, instruction->dest);
        }
    }
    // ---------------------------------------------------------------------

    for (int index = 0; index < ir->count; index++) {
        GimpleInst *instruction = &ir->insts[index];
        if (instruction->is_dead) {
            continue;
        }

        if (cprop_is_block_barrier(instruction)) {
            // SAFE DELETION: Iterate backwards so shifting doesn't skip elements!
            for (int i = binding_count - 1; i >= 0; i--) {
                int count = get_assign_count(var_names, var_counts, &unique_vars, bindings[i].name);
                if (count > 1) {
                    cprop_kill(bindings, &binding_count, bindings[i].name);
                }
            }
            continue; 
        }

        if (instruction->type != G_ASSIGN) {
            continue;
        }

        int binding_index = cprop_find(bindings, binding_count, instruction->arg1);
        if (binding_index >= 0) {
            strcpy(instruction->arg1, bindings[binding_index].value);
            changed = 1;
        }

        if (instruction->op[0] != '\0') {
            binding_index = cprop_find(bindings, binding_count, instruction->arg2);
            if (binding_index >= 0) {
                strcpy(instruction->arg2, bindings[binding_index].value);
                changed = 1;
            }
        }

        cprop_kill(bindings, &binding_count, instruction->dest);
        if (instruction->op[0] == '\0' && cprop_is_number(instruction->arg1)) {
            cprop_set(bindings, &binding_count, instruction->dest, instruction->arg1);
        }
    }

    return changed;
}