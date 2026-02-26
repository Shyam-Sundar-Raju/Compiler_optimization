#ifndef COMPILER_CFG_H
#define COMPILER_CFG_H

#include "engine.h"

#define MAX_BLOCKS 256
#define MAX_BLOCK_INSTS 64
#define MAX_PREDECESSORS 16
#define MAX_SUCCESSORS 16

typedef struct BasicBlock {
    int id;
    int start_index;
    int end_index;
    int inst_count;
    GimpleInst *insts[MAX_BLOCK_INSTS];
    
    int pred_count;
    struct BasicBlock *predecessors[MAX_PREDECESSORS];
    
    int succ_count;
    struct BasicBlock *successors[MAX_SUCCESSORS];
    
    int is_reachable;
    int dominator_id;
    int loop_depth;
} BasicBlock;

typedef struct {
    BasicBlock blocks[MAX_BLOCKS];
    int block_count;
    int entry_block_id;
    int exit_block_id;
} ControlFlowGraph;

typedef struct {
    char name[32];
    int defined_at_index;
} Definition;

typedef struct {
    Definition defs[512];
    int def_count;
} ReachingDefs;

typedef struct {
    char name[32];
    int live_at_exit;
} LiveVariable;

typedef struct {
    LiveVariable vars[512];
    int var_count;
} LivenessSet;

ControlFlowGraph build_cfg(FunctionIR *ir);
void compute_liveness(ControlFlowGraph *cfg, LivenessSet *liveness_result);
void compute_dominators(ControlFlowGraph *cfg);
void detect_loops(ControlFlowGraph *cfg);
void free_cfg(ControlFlowGraph *cfg);

#endif
