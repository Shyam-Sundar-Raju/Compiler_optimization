#ifndef COMPILER_ENGINE_H
#define COMPILER_ENGINE_H

typedef enum { G_ASSIGN, G_COND, G_GOTO, G_CALL, G_LABEL, G_RETURN } OpType;

typedef struct {
    OpType type;
    char dest[32];   // e.g., "_1"
    char arg1[32];   // e.g., "x"
    char op[8];     // e.g., "+", "<<", "=="
    char arg2[32];   // e.g., "5"
    int is_dead;     // Flag for DCE pass
} GimpleInst;

typedef struct {
    GimpleInst *insts;
    int count;
} FunctionIR;

FunctionIR parse_gimple_file(const char *filename);
int constant_folding(FunctionIR *ir);
int algebraic_simplification(FunctionIR *ir);
int common_subexpression_elimination(FunctionIR *ir);
int constant_propagation(FunctionIR *ir);
int copy_propagation(FunctionIR *ir);
int dead_code_elimination(FunctionIR *ir);
int function_cloning(FunctionIR *ir);
int function_inlining(FunctionIR *ir);
int induction_variable_elimination(FunctionIR *ir);
int loop_fusion(FunctionIR *ir);
int loop_invariant_code_motion(FunctionIR *ir);
int loop_peeling(FunctionIR *ir);
int loop_unrolling(FunctionIR *ir);
int strength_reduction(FunctionIR *ir);
int unreachable_code_elimination(FunctionIR *ir);

#endif