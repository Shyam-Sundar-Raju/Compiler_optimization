#include "../engine.h"
#include <ctype.h>
#include <string.h>
#include "../cfg.h"

static int licm_is_constant(const char *text) {
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

static int licm_is_defined_in_loop(const char defs[][32], int def_count, const char *name) {
	if (name[0] == '\0' || licm_is_constant(name)) {
		return 0;
	}
	for (int index = 0; index < def_count; index++) {
		if (strcmp(defs[index], name) == 0) {
			return 1;
		}
	}
	return 0;
}

static void licm_insert_instruction(FunctionIR *ir, int position, const GimpleInst *instruction) {
	if (ir->count >= 999 || position < 0 || position > ir->count) {
		return;
	}
	for (int index = ir->count; index > position; index--) {
		ir->insts[index] = ir->insts[index - 1];
	}
	ir->insts[position] = *instruction;
	ir->count++;
}

int loop_invariant_code_motion(FunctionIR *ir) {
	ControlFlowGraph cfg = build_cfg(ir);
	int changed = 0;
	char loop_defs[256][32];
	int loop_def_count = 0;

	for (int block_idx = 0; block_idx < cfg.block_count; block_idx++) {
		BasicBlock *block = &cfg.blocks[block_idx];
		if (block->loop_depth <= 0) {
			continue;
		}
		for (int inst_idx = 0; inst_idx < block->inst_count; inst_idx++) {
			GimpleInst *inst = block->insts[inst_idx];
			if (inst->is_dead || inst->dest[0] == '\0') {
				continue;
			}
			int exists = 0;
			for (int i = 0; i < loop_def_count; i++) {
				if (strcmp(loop_defs[i], inst->dest) == 0) {
					exists = 1;
					break;
				}
			}
			if (!exists && loop_def_count < 256) {
				strcpy(loop_defs[loop_def_count++], inst->dest);
			}
		}
	}
	
	for (int block_idx = 0; block_idx < cfg.block_count; block_idx++) {
		BasicBlock *block = &cfg.blocks[block_idx];
		
		if (block->loop_depth <= 0) {
			continue;
		}
		
		for (int inst_idx = 0; inst_idx < block->inst_count; inst_idx++) {
			GimpleInst *inst = block->insts[inst_idx];
			
			if (inst->is_dead || inst->type != G_ASSIGN || inst->op[0] == '\0') {
				continue;
			}
			
			int arg1_loop_defined = licm_is_defined_in_loop(loop_defs, loop_def_count, inst->arg1);
			int arg2_loop_defined = licm_is_defined_in_loop(loop_defs, loop_def_count, inst->arg2);
			int is_invariant = !arg1_loop_defined && !arg2_loop_defined;
			
			if (is_invariant) {
				int safe_to_hoist = 1;
				for (int i = inst_idx + 1; i < block->inst_count; i++) {
					GimpleInst *later = block->insts[i];
					if (strcmp(later->dest, inst->dest) == 0) {
						safe_to_hoist = 0;
						break;
					}
				}
				
				if (safe_to_hoist) {
					GimpleInst hoisted = *inst;
					int original_index = (int)(inst - ir->insts);
					int insert_pos = block->start_index;
					licm_insert_instruction(ir, insert_pos, &hoisted);
					if (insert_pos <= original_index) {
						original_index++;
					}
					if (original_index >= 0 && original_index < ir->count) {
						ir->insts[original_index].is_dead = 1;
					}
					changed = 1;
					free_cfg(&cfg);
					return changed;
				}
			}
		}
	}
	
	free_cfg(&cfg);
	return changed;
}
