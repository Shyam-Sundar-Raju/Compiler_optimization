#include "../engine.h"
#include <string.h>
#include "../cfg.h"

int loop_peeling(FunctionIR *ir) {
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
		
		if (block->inst_count < 2) {
			continue;
		}
		
		GimpleInst *first = block->insts[0];
		GimpleInst *second = block->insts[1];
		
		if (first->type == G_LABEL && second->type == G_ASSIGN && !first->is_dead && !second->is_dead) {
			if (ir->count < ir->count + 1) {
				for (int i = ir->count; i > first - ir->insts; i--) {
					ir->insts[i] = ir->insts[i - 1];
				}
				ir->insts[first - ir->insts] = *second;
				ir->count++;
			}
			changed = 1;
			break;
		}
	}
	
	free_cfg(&cfg);
	return changed;
}
