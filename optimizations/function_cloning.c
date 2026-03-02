#include "../engine.h"
#include <stdio.h>
#include <string.h>

int function_cloning(FunctionIR *ir) {
	char seen_functions[128][32];
	int seen_count = 0;
	int changed = 0;

	for (int index = 0; index < ir->count; index++) {
		GimpleInst *instruction = &ir->insts[index];
		if (instruction->is_dead || instruction->type != G_CALL || instruction->arg1[0] == '\0') {
			continue;
		}

		int seen_index = -1;
		for (int i = 0; i < seen_count; i++) {
			if (strcmp(seen_functions[i], instruction->arg1) == 0) {
				seen_index = i;
				break;
			}
		}

		if (seen_index >= 0) {
			char cloned_name[32];
			size_t name_len = strlen(instruction->arg1);
			if (name_len > sizeof(cloned_name) - 7) {
				name_len = sizeof(cloned_name) - 7;
			}
			memcpy(cloned_name, instruction->arg1, name_len);
			memcpy(cloned_name + name_len, "_clone", 7);
			strncpy(instruction->arg1, cloned_name, sizeof(instruction->arg1) - 1);
			instruction->arg1[sizeof(instruction->arg1) - 1] = '\0';
			changed = 1;
		} else if (seen_count < 128) {
			strncpy(seen_functions[seen_count], instruction->arg1, sizeof(seen_functions[seen_count]) - 1);
			seen_functions[seen_count][sizeof(seen_functions[seen_count]) - 1] = '\0';
			seen_count++;
		}
	}

	return changed;
}
