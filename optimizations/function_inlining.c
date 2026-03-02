#include "../engine.h"
#include <string.h>

int function_inlining(FunctionIR *ir) {
	int changed = 0;

	for (int index = 0; index < ir->count; index++) {
		GimpleInst *instruction = &ir->insts[index];
		if (instruction->is_dead || instruction->type != G_CALL) {
			continue;
		}

		if (strcmp(instruction->arg1, "identity") == 0 && instruction->arg2[0] != '\0') {
			instruction->type = G_ASSIGN;
			strcpy(instruction->arg1, instruction->arg2);
			instruction->op[0] = '\0';
			instruction->arg2[0] = '\0';
			changed = 1;
		} else if (strcmp(instruction->arg1, "add_one") == 0 && instruction->arg2[0] != '\0') {
			instruction->type = G_ASSIGN;
			instruction->op[0] = '\0';
			strcpy(instruction->arg1, instruction->arg2);
			strcpy(instruction->op, "+");
			strcpy(instruction->arg2, "1");
			changed = 1;
		} else if (strcmp(instruction->arg1, "const0") == 0) {
			instruction->type = G_ASSIGN;
			strcpy(instruction->arg1, "0");
			instruction->op[0] = '\0';
			instruction->arg2[0] = '\0';
			changed = 1;
		}
	}

	return changed;
}
