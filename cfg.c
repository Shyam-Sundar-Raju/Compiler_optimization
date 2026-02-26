#include "cfg.h"
#include <stdlib.h>
#include <string.h>

static int is_block_terminator(GimpleInst *inst) {
    return inst->type == G_GOTO || inst->type == G_COND || inst->type == G_RETURN;
}

ControlFlowGraph build_cfg(FunctionIR *ir) {
    ControlFlowGraph cfg;
    memset(&cfg, 0, sizeof(cfg));
    
    if (ir->count == 0) {
        return cfg;
    }
    
    BasicBlock *current_block = NULL;
    cfg.entry_block_id = 0;
    cfg.exit_block_id = -1;
    
    for (int i = 0; i < ir->count; i++) {
        GimpleInst *inst = &ir->insts[i];
        
        if (inst->type == G_LABEL && (current_block == NULL || i > 0)) {
            if (current_block != NULL && cfg.block_count > 0) {
                cfg.blocks[cfg.block_count - 1].end_index = i - 1;
            }
            
            if (cfg.block_count < MAX_BLOCKS) {
                current_block = &cfg.blocks[cfg.block_count];
                current_block->id = cfg.block_count;
                current_block->start_index = i;
                current_block->end_index = i;
                current_block->inst_count = 0;
                current_block->is_reachable = (cfg.block_count == 0) ? 1 : 0;
                current_block->dominator_id = -1;
                current_block->loop_depth = 0;
                current_block->pred_count = 0;
                current_block->succ_count = 0;
                cfg.block_count++;
            }
        }
        
        if (current_block != NULL && current_block->inst_count < MAX_BLOCK_INSTS) {
            current_block->insts[current_block->inst_count++] = inst;
            current_block->end_index = i;
        }
        
        if (is_block_terminator(inst)) {
            if (current_block != NULL && i + 1 < ir->count && cfg.block_count < MAX_BLOCKS) {
                current_block = NULL;
            }
        }
    }
    
    for (int block_idx = 0; block_idx < cfg.block_count; block_idx++) {
        BasicBlock *block = &cfg.blocks[block_idx];
        if (block->inst_count == 0) {
            continue;
        }
        
        GimpleInst *last = block->insts[block->inst_count - 1];
        
        if (last->type == G_GOTO && last->dest[0] != '\0') {
            for (int j = 0; j < cfg.block_count; j++) {
                BasicBlock *target_block = &cfg.blocks[j];
                if (target_block->inst_count > 0 && target_block->insts[0]->type == G_LABEL) {
                    if (strcmp(target_block->insts[0]->dest, last->dest) == 0) {
                        if (block->succ_count < MAX_SUCCESSORS) {
                            block->successors[block->succ_count++] = target_block;
                        }
                        if (target_block->pred_count < MAX_PREDECESSORS) {
                            target_block->predecessors[target_block->pred_count++] = block;
                        }
                        break;
                    }
                }
            }
        } else if (last->type == G_COND && last->dest[0] != '\0') {
            for (int j = 0; j < cfg.block_count; j++) {
                BasicBlock *target_block = &cfg.blocks[j];
                if (target_block->inst_count > 0 && target_block->insts[0]->type == G_LABEL) {
                    if (strcmp(target_block->insts[0]->dest, last->dest) == 0) {
                        if (block->succ_count < MAX_SUCCESSORS) {
                            block->successors[block->succ_count++] = target_block;
                        }
                        if (target_block->pred_count < MAX_PREDECESSORS) {
                            target_block->predecessors[target_block->pred_count++] = block;
                        }
                        break;
                    }
                }
            }
            
            if (block_idx + 1 < cfg.block_count && block->succ_count < MAX_SUCCESSORS) {
                block->successors[block->succ_count++] = &cfg.blocks[block_idx + 1];
                if (cfg.blocks[block_idx + 1].pred_count < MAX_PREDECESSORS) {
                    cfg.blocks[block_idx + 1].predecessors[cfg.blocks[block_idx + 1].pred_count++] = block;
                }
            }
        } else if (block_idx + 1 < cfg.block_count && last->type != G_RETURN) {
            if (block->succ_count < MAX_SUCCESSORS) {
                block->successors[block->succ_count++] = &cfg.blocks[block_idx + 1];
            }
            if (cfg.blocks[block_idx + 1].pred_count < MAX_PREDECESSORS) {
                cfg.blocks[block_idx + 1].predecessors[cfg.blocks[block_idx + 1].pred_count++] = block;
            }
        }
    }
    
    return cfg;
}

void compute_liveness(ControlFlowGraph *cfg, LivenessSet *liveness_result) {
    memset(liveness_result, 0, sizeof(*liveness_result));
    
    for (int block_idx = 0; block_idx < cfg->block_count; block_idx++) {
        BasicBlock *block = &cfg->blocks[block_idx];
        
        for (int inst_idx = 0; inst_idx < block->inst_count; inst_idx++) {
            GimpleInst *inst = block->insts[inst_idx];
            
            if (inst->is_dead || inst->type != G_ASSIGN) {
                continue;
            }
            
            if (inst->arg1[0] != '\0') {
                int found = 0;
                for (int i = 0; i < liveness_result->var_count; i++) {
                    if (strcmp(liveness_result->vars[i].name, inst->arg1) == 0) {
                        found = 1;
                        break;
                    }
                }
                if (!found && liveness_result->var_count < 512) {
                    strcpy(liveness_result->vars[liveness_result->var_count].name, inst->arg1);
                    liveness_result->vars[liveness_result->var_count].live_at_exit = 1;
                    liveness_result->var_count++;
                }
            }
            
            if (inst->arg2[0] != '\0') {
                int found = 0;
                for (int i = 0; i < liveness_result->var_count; i++) {
                    if (strcmp(liveness_result->vars[i].name, inst->arg2) == 0) {
                        found = 1;
                        break;
                    }
                }
                if (!found && liveness_result->var_count < 512) {
                    strcpy(liveness_result->vars[liveness_result->var_count].name, inst->arg2);
                    liveness_result->vars[liveness_result->var_count].live_at_exit = 1;
                    liveness_result->var_count++;
                }
            }
        }
    }
}

void compute_dominators(ControlFlowGraph *cfg) {
    for (int i = 0; i < cfg->block_count; i++) {
        cfg->blocks[i].dominator_id = (i == 0) ? 0 : -1;
    }
    
    int changed = 1;
    int iterations = 0;
    while (changed && iterations < 20) {
        changed = 0;
        iterations++;
        
        for (int i = 1; i < cfg->block_count; i++) {
            BasicBlock *block = &cfg->blocks[i];
            int new_dom = -1;
            
            for (int p = 0; p < block->pred_count; p++) {
                int pred_dom = block->predecessors[p]->dominator_id;
                if (new_dom == -1) {
                    new_dom = pred_dom;
                } else if (new_dom != pred_dom) {
                    new_dom = 0;
                }
            }
            
            if (new_dom != block->dominator_id && new_dom != -1) {
                block->dominator_id = new_dom;
                changed = 1;
            }
        }
    }
}

void detect_loops(ControlFlowGraph *cfg) {
    for (int i = 0; i < cfg->block_count; i++) {
        cfg->blocks[i].loop_depth = 0;
    }
    
    for (int i = 0; i < cfg->block_count; i++) {
        BasicBlock *block = &cfg->blocks[i];
        
        for (int j = 0; j < block->succ_count; j++) {
            BasicBlock *succ = block->successors[j];
            
            if (succ->id <= block->id) {
                for (int k = succ->id; k <= block->id; k++) {
                    cfg->blocks[k].loop_depth++;
                }
            }
        }
    }
}

void free_cfg(ControlFlowGraph *cfg) {
    (void)cfg;
}
