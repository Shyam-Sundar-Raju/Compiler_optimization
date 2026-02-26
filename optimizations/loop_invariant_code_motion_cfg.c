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

int loop_invariant_code_motion(FunctionIR *ir) {
    ControlFlowGraph cfg = build_cfg(ir);
    int changed = 0;
    
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
            
            int is_invariant = licm_is_constant(inst->arg1) && licm_is_constant(inst->arg2);
            
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
                    changed = 1;
                }
            }
        }
    }
    
    free_cfg(&cfg);
    return changed;
}
