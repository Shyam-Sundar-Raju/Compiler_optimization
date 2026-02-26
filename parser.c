#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "engine.h"

#define MAX_LINE 256

static void trim_in_place(char *text) {
    size_t len = strlen(text);
    while (len > 0 && isspace((unsigned char)text[len - 1])) {
        text[--len] = '\0';
    }

    size_t start = 0;
    while (text[start] != '\0' && isspace((unsigned char)text[start])) {
        start++;
    }

    if (start > 0) {
        memmove(text, text + start, strlen(text + start) + 1);
    }

    len = strlen(text);
    while (len > 0 && (text[len - 1] == ';' || text[len - 1] == ',')) {
        text[--len] = '\0';
    }
}

static void normalize_label_token(char *text) {
    trim_in_place(text);
    if (strncmp(text, "<<", 2) == 0) {
        memmove(text, text + 2, strlen(text + 2) + 1);
    } else if (text[0] == '<') {
        memmove(text, text + 1, strlen(text + 1) + 1);
    }

    size_t len = strlen(text);
    while (len > 0 && (text[len - 1] == '>' || text[len - 1] == ':')) {
        text[--len] = '\0';
    }
}

static void map_expr_to_op(const char *expr, char *op_out, size_t op_out_size) {
    if (strcmp(expr, "plus_expr") == 0) snprintf(op_out, op_out_size, "+");
    else if (strcmp(expr, "minus_expr") == 0) snprintf(op_out, op_out_size, "-");
    else if (strcmp(expr, "mult_expr") == 0) snprintf(op_out, op_out_size, "*");
    else if (strcmp(expr, "trunc_div_expr") == 0 || strcmp(expr, "exact_div_expr") == 0) snprintf(op_out, op_out_size, "/");
    else if (strcmp(expr, "lshift_expr") == 0) snprintf(op_out, op_out_size, "<<");
    else if (strcmp(expr, "rshift_expr") == 0) snprintf(op_out, op_out_size, ">>");
    else if (strcmp(expr, "le_expr") == 0) snprintf(op_out, op_out_size, "<=");
    else if (strcmp(expr, "lt_expr") == 0) snprintf(op_out, op_out_size, "<");
    else if (strcmp(expr, "ge_expr") == 0) snprintf(op_out, op_out_size, ">=");
    else if (strcmp(expr, "gt_expr") == 0) snprintf(op_out, op_out_size, ">");
    else if (strcmp(expr, "eq_expr") == 0) snprintf(op_out, op_out_size, "==");
    else if (strcmp(expr, "ne_expr") == 0) snprintf(op_out, op_out_size, "!=");
    else op_out[0] = '\0';
}

static int split_gimple_payload(char *payload, char tokens[][64], int max_tokens) {
    int token_count = 0;
    char *cursor = payload;

    while (cursor != NULL && *cursor != '\0' && token_count < max_tokens) {
        char *next = strchr(cursor, ',');
        if (next != NULL) {
            *next = '\0';
        }

        snprintf(tokens[token_count], 64, "%s", cursor);
        trim_in_place(tokens[token_count]);
        token_count++;

        cursor = (next == NULL) ? NULL : (next + 1);
    }

    return token_count;
}

static void copy_field(char *dst, size_t dst_size, const char *src) {
    strncpy(dst, src, dst_size - 1);
    dst[dst_size - 1] = '\0';
}

FunctionIR parse_gimple_file(const char *filename) {
    FILE *fp = fopen(filename, "r");
    if (!fp) {
        perror("Failed to open GIMPLE file");
        exit(1);
    }

    FunctionIR ir;
    ir.insts = (GimpleInst *)calloc(1000, sizeof(GimpleInst));
    ir.count = 0;

    char line[MAX_LINE];
    while (fgets(line, sizeof(line), fp)) {
        if (ir.count >= 1000) {
            break;
        }

        trim_in_place(line);
        if (line[0] == '\0') {
            continue;
        }

        GimpleInst *ins = &ir.insts[ir.count];
        ins->is_dead = 0;
        ins->dest[0] = '\0';
        ins->arg1[0] = '\0';
        ins->arg2[0] = '\0';
        ins->op[0] = '\0';

        if (strncmp(line, "gimple_goto", 11) == 0) {
            char *start = strchr(line, '<');
            char *end = strrchr(line, '>');
            if (start == NULL || end == NULL || end <= start) {
                continue;
            }

            char payload[64];
            size_t payload_len = (size_t)(end - start - 1);
            if (payload_len >= sizeof(payload)) {
                payload_len = sizeof(payload) - 1;
            }

            memcpy(payload, start + 1, payload_len);
            payload[payload_len] = '\0';
            normalize_label_token(payload);

            ins->type = G_GOTO;
            copy_field(ins->dest, sizeof(ins->dest), payload);
            ir.count++;
        } else if (strncmp(line, "gimple_label", 11) == 0) {
            char *start = strchr(line, '<');
            char *end = strrchr(line, '>');
            if (start == NULL || end == NULL || end <= start) {
                continue;
            }

            char payload[64];
            size_t payload_len = (size_t)(end - start - 1);
            if (payload_len >= sizeof(payload)) {
                payload_len = sizeof(payload) - 1;
            }

            memcpy(payload, start + 1, payload_len);
            payload[payload_len] = '\0';
            normalize_label_token(payload);

            ins->type = G_LABEL;
            copy_field(ins->dest, sizeof(ins->dest), payload);
            ir.count++;
        } else if (strncmp(line, "gimple_assign", 13) == 0) {
            char *start = strchr(line, '<');
            char *end = strrchr(line, '>');
            if (start == NULL || end == NULL || end <= start) {
                continue;
            }

            char payload[256];
            size_t payload_len = (size_t)(end - start - 1);
            if (payload_len >= sizeof(payload)) {
                payload_len = sizeof(payload) - 1;
            }

            memcpy(payload, start + 1, payload_len);
            payload[payload_len] = '\0';

            char tokens[6][64] = {{0}};
            int token_count = split_gimple_payload(payload, tokens, 6);
            if (token_count < 3) {
                continue;
            }

            ins->type = G_ASSIGN;
            copy_field(ins->dest, sizeof(ins->dest), tokens[1]);
            copy_field(ins->arg1, sizeof(ins->arg1), tokens[2]);

            if (token_count >= 4 && strcmp(tokens[3], "NULL") != 0) {
                map_expr_to_op(tokens[0], ins->op, sizeof(ins->op));
                copy_field(ins->arg2, sizeof(ins->arg2), tokens[3]);
            } else {
                ins->op[0] = '\0';
                ins->arg2[0] = '\0';
            }
            ir.count++;
        } else if (strncmp(line, "gimple_cond", 11) == 0) {
            char *start = strchr(line, '<');
            char *end = strrchr(line, '>');
            if (start == NULL || end == NULL || end <= start) {
                continue;
            }

            char payload[256];
            size_t payload_len = (size_t)(end - start - 1);
            if (payload_len >= sizeof(payload)) {
                payload_len = sizeof(payload) - 1;
            }

            memcpy(payload, start + 1, payload_len);
            payload[payload_len] = '\0';

            char tokens[6][64] = {{0}};
            int token_count = split_gimple_payload(payload, tokens, 6);
            if (token_count < 5) {
                continue;
            }

            // 1. Generate the True edge (Conditional Goto)
            ins->type = G_COND;
            map_expr_to_op(tokens[0], ins->op, sizeof(ins->op));
            copy_field(ins->arg1, sizeof(ins->arg1), tokens[1]);
            copy_field(ins->arg2, sizeof(ins->arg2), tokens[2]);
            normalize_label_token(tokens[3]);
            copy_field(ins->dest, sizeof(ins->dest), tokens[3]);
            ir.count++;

            // 2. Generate the False edge (Unconditional Goto)
            GimpleInst *false_ins = &ir.insts[ir.count];
            false_ins->is_dead = 0;
            false_ins->type = G_GOTO;
            false_ins->op[0] = '\0';
            false_ins->arg1[0] = '\0';
            false_ins->arg2[0] = '\0';
            normalize_label_token(tokens[4]);
            copy_field(false_ins->dest, sizeof(false_ins->dest), tokens[4]);
            ir.count++;
        } else if (strncmp(line, "gimple_call", 11) == 0) {
            char *start = strchr(line, '<');
            char *end = strrchr(line, '>');
            if (start == NULL || end == NULL || end <= start) {
                continue;
            }

            char payload[256];
            size_t payload_len = (size_t)(end - start - 1);
            if (payload_len >= sizeof(payload)) {
                payload_len = sizeof(payload) - 1;
            }

            memcpy(payload, start + 1, payload_len);
            payload[payload_len] = '\0';

            char tokens[6][64] = {{0}};
            int token_count = split_gimple_payload(payload, tokens, 6);
            if (token_count < 1) {
                continue;
            }

            ins->type = G_CALL;
            copy_field(ins->arg1, sizeof(ins->arg1), tokens[0]);
            if (token_count > 1 && strcmp(tokens[1], "NULL") != 0) {
                copy_field(ins->dest, sizeof(ins->dest), tokens[1]);
            }
            ir.count++;
        }
    }

    fclose(fp);
    return ir;
}