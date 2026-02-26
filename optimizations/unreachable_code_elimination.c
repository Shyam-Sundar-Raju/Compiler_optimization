#include "../engine.h"
#include <string.h>
#include "../cfg.h"

int unreachable_code_elimination(FunctionIR *ir) {
	ControlFlowGraph cfg = build_cfg(ir);
	int changed = 0;
	
	for (int block_idx = 0; block_idx < cfg.block_count; block_idx++) {
		BasicBlock *block = &cfg.blocks[block_idx];
		
		if (block->is_reachable == 0) {
			for (int inst_idx = 0; inst_idx < block->inst_count; inst_idx++) {
				if (!block->insts[inst_idx]->is_dead) {
					block->insts[inst_idx]->is_dead = 1;
					changed = 1;
				}
			}
		}
	}
	
	for (int index = 0; index < ir->count; index++) {
		GimpleInst *instruction = &ir->insts[index];

		if (instruction->is_dead) {
			continue;
		}

		if (instruction->type == G_GOTO) {
			int is_false_branch = 0;
			if (index > 0) {
				GimpleInst *prev = &ir->insts[index - 1];
				if (prev->type == G_COND) {
					is_false_branch = 1;
				}
			}
			
			if (!is_false_branch) {
				for (int i = index + 1; i < ir->count; i++) {
					GimpleInst *following = &ir->insts[i];
					if (following->is_dead) {
						continue;
					}
					if (following->type == G_LABEL) {
						break;
					}
					following->is_dead = 1;
					changed = 1;
				}
			}
		}
	}

	free_cfg(&cfg);
	return changed;
}
