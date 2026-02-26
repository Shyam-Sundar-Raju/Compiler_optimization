#include "../engine.h"
#include <string.h>
#include "../cfg.h"

int loop_fusion(FunctionIR *ir) {
	ControlFlowGraph cfg = build_cfg(ir);
	int changed = 0;
	
	for (int block_idx = 0; block_idx < cfg.block_count - 1; block_idx++) {
		BasicBlock *block_a = &cfg.blocks[block_idx];
		BasicBlock *block_b = &cfg.blocks[block_idx + 1];
		
		if (block_a->loop_depth <= 0 || block_b->loop_depth <= 0) {
			continue;
		}
		
		if (block_a->loop_depth != block_b->loop_depth) {
			continue;
		}
		
		if (block_a->inst_count > 0 && block_b->inst_count > 0) {
			GimpleInst *last_a = block_a->insts[block_a->inst_count - 1];
			GimpleInst *first_b = block_b->insts[0];
			
			if (last_a->type == G_GOTO && first_b->type == G_LABEL) {
				if (strcmp(last_a->dest, first_b->dest) == 0) {
					changed = 1;
				}
			}
		}
	}
	
	free_cfg(&cfg);
	return changed;
}
