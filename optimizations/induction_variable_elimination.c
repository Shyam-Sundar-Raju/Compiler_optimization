#include "../engine.h"
#include <ctype.h>
#include <string.h>
#include "../cfg.h"

static int ive_is_number(const char *text) {
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

static int ive_find(char used[][32], int used_count, const char *name) {
	for (int index = 0; index < used_count; index++) {
		if (strcmp(used[index], name) == 0) {
			return index;
		}
	}
	return -1;
}

static void ive_add(char used[][32], int *used_count, const char *name) {
	if (name[0] == '\0' || ive_is_number(name) || ive_find(used, *used_count, name) >= 0 || *used_count >= 512) {
		return;
	}
	strcpy(used[*used_count], name);
	(*used_count)++;
}

int induction_variable_elimination(FunctionIR *ir) {
	ControlFlowGraph cfg = build_cfg(ir);
	char used[512][32];
	int used_count = 0;
	int changed = 0;

	for (int block_idx = 0; block_idx < cfg.block_count; block_idx++) {
		BasicBlock *block = &cfg.blocks[block_idx];
		
		if (block->loop_depth <= 0) {
			continue;
		}
		
		for (int inst_idx = block->inst_count - 1; inst_idx >= 0; inst_idx--) {
			GimpleInst *instruction = block->insts[inst_idx];
			
			if (instruction->is_dead) {
				continue;
			}

			if (instruction->type == G_ASSIGN
				&& strcmp(instruction->dest, instruction->arg1) == 0
				&& instruction->op[0] != '\0'
				&& ive_is_number(instruction->arg2)) {
				int live = ive_find(used, used_count, instruction->dest) >= 0;
				if (!live) {
					instruction->is_dead = 1;
					changed = 1;
					continue;
				}
			}

			if (instruction->type == G_ASSIGN) {
				ive_add(used, &used_count, instruction->arg1);
				if (instruction->op[0] != '\0') {
					ive_add(used, &used_count, instruction->arg2);
				}
			} else if (instruction->type == G_COND || instruction->type == G_CALL || instruction->type == G_RETURN) {
				ive_add(used, &used_count, instruction->arg1);
				if (instruction->op[0] != '\0' || instruction->type == G_CALL) {
					ive_add(used, &used_count, instruction->arg2);
				}
			}
		}
	}

	free_cfg(&cfg);
	return changed;
}
