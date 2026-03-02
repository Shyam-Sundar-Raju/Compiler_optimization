#include "../engine.h"
#include <string.h>
#include "../cfg.h"

static int lf_find_label(FunctionIR *ir, const char *label) {
	for (int index = 0; index < ir->count; index++) {
		if (ir->insts[index].type == G_LABEL && strcmp(ir->insts[index].dest, label) == 0) {
			return index;
		}
	}
	return -1;
}

static int lf_insert_instruction(FunctionIR *ir, int pos, const GimpleInst *inst) {
	if (ir->count >= 999 || pos < 0 || pos > ir->count) {
		return 0;
	}
	for (int index = ir->count; index > pos; index--) {
		ir->insts[index] = ir->insts[index - 1];
	}
	ir->insts[pos] = *inst;
	ir->count++;
	return 1;
}

int loop_fusion(FunctionIR *ir) {
	for (int cond1_idx = 0; cond1_idx + 1 < ir->count; cond1_idx++) {
		GimpleInst *cond1 = &ir->insts[cond1_idx];
		if (cond1->type != G_COND) {
			continue;
		}
		GimpleInst *false_goto1 = &ir->insts[cond1_idx + 1];
		if (false_goto1->type != G_GOTO) {
			continue;
		}

		char body1_label[32];
		char mid_label[32];
		char induction_var[32];
		char bound[32];
		char relop[8];
		strcpy(body1_label, cond1->dest);
		strcpy(mid_label, false_goto1->dest);
		strcpy(induction_var, cond1->arg1);
		strcpy(bound, cond1->arg2);
		strcpy(relop, cond1->op);

		int body1_label_idx = lf_find_label(ir, body1_label);
		int mid_label_idx = lf_find_label(ir, mid_label);
		if (body1_label_idx < 0 || mid_label_idx < 0 || body1_label_idx >= cond1_idx || mid_label_idx <= cond1_idx) {
			continue;
		}
		if (mid_label_idx + 2 >= ir->count) {
			continue;
		}

		GimpleInst *reset = &ir->insts[mid_label_idx + 1];
		GimpleInst *goto_cond2 = &ir->insts[mid_label_idx + 2];
		if (reset->type != G_ASSIGN || strcmp(reset->dest, induction_var) != 0 || strcmp(reset->arg1, "0") != 0 || reset->op[0] != '\0') {
			continue;
		}
		if (goto_cond2->type != G_GOTO) {
			continue;
		}

		int cond2_label_idx = lf_find_label(ir, goto_cond2->dest);
		if (cond2_label_idx < 0 || cond2_label_idx + 2 >= ir->count) {
			continue;
		}
		GimpleInst *cond2 = &ir->insts[cond2_label_idx + 1];
		GimpleInst *false_goto2 = &ir->insts[cond2_label_idx + 2];
		if (cond2->type != G_COND || false_goto2->type != G_GOTO) {
			continue;
		}
		if (strcmp(cond2->arg1, induction_var) != 0 || strcmp(cond2->arg2, bound) != 0 || strcmp(cond2->op, relop) != 0) {
			continue;
		}

		char body2_label[32];
		char exit2_label[32];
		strcpy(body2_label, cond2->dest);
		strcpy(exit2_label, false_goto2->dest);

		int body2_label_idx = lf_find_label(ir, body2_label);
		if (body2_label_idx < 0 || body2_label_idx >= cond2_label_idx) {
			continue;
		}

		int increment1_idx = -1;
		for (int idx = body1_label_idx + 1; idx < cond1_idx; idx++) {
			GimpleInst *inst = &ir->insts[idx];
			if (inst->type == G_ASSIGN && strcmp(inst->dest, induction_var) == 0 && strcmp(inst->arg1, induction_var) == 0 && strcmp(inst->op, "+") == 0 && strcmp(inst->arg2, "1") == 0) {
				increment1_idx = idx;
				break;
			}
		}
		if (increment1_idx < 0) {
			continue;
		}

		GimpleInst copied_body[64];
		int copied_count = 0;
		for (int idx = body2_label_idx + 1; idx < cond2_label_idx; idx++) {
			GimpleInst *inst = &ir->insts[idx];
			if (inst->is_dead || inst->type != G_ASSIGN) {
				continue;
			}
			if (strcmp(inst->dest, induction_var) == 0 && strcmp(inst->arg1, induction_var) == 0 && strcmp(inst->op, "+") == 0 && strcmp(inst->arg2, "1") == 0) {
				continue;
			}
			if (copied_count < 64) {
				copied_body[copied_count++] = *inst;
			}
		}

		for (int i = 0; i < copied_count; i++) {
			if (lf_insert_instruction(ir, increment1_idx, &copied_body[i])) {
				increment1_idx++;
				cond1_idx++;
			}
		}

		false_goto1 = &ir->insts[cond1_idx + 1];
		strcpy(false_goto1->dest, exit2_label);

		int in_second_loop_region = 0;
		for (int idx = 0; idx < ir->count; idx++) {
			GimpleInst *inst = &ir->insts[idx];
			if (inst->type == G_LABEL && strcmp(inst->dest, mid_label) == 0) {
				in_second_loop_region = 1;
			}
			if (inst->type == G_LABEL && strcmp(inst->dest, exit2_label) == 0) {
				in_second_loop_region = 0;
				continue;
			}
			if (in_second_loop_region) {
				inst->is_dead = 1;
			}
		}

		return 1;
	}

	return 0;
}
