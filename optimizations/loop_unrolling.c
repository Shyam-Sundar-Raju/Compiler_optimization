#include "../engine.h"
#include <string.h>
#include "../cfg.h"

int loop_unrolling(FunctionIR *ir) {
	if (ir->count >= 999) {
		return 0;
	}
	
	ControlFlowGraph cfg = build_cfg(ir);
	int changed = 0;
	
	for (int block_idx = 0; block_idx < cfg.block_count; block_idx++) {
		BasicBlock *block = &cfg.blocks[block_idx];
		
		if (block->loop_depth <= 0) {
			continue;
		}
		
		if (block->inst_count < 1) {
			continue;
		}
		
		for (int inst_idx = 0; inst_idx < block->inst_count && ir->count < 999; inst_idx++) {
			GimpleInst *inst = block->insts[inst_idx];
			
			if (inst->type == G_ASSIGN && !inst->is_dead) {
				int end_pos = inst - ir->insts + 1;
				for (int i = ir->count; i > end_pos; i--) {
					ir->insts[i] = ir->insts[i - 1];
				}
				ir->insts[end_pos] = *inst;
				ir->count++;
				changed = 1;
				break;
			}
		}
		if (changed) {
			break;
		}
	}
	
	free_cfg(&cfg);
	return changed;
}
