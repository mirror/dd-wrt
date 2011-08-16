/*
** Copyright (C) 2010 Sourcefire, Inc.
** Author: Michael R. Altizer <maltizer@sourcefire.com>
**
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU General Public License Version 2 as
** published by the Free Software Foundation.  You may not use, modify or
** distribute this program under any other version of the GNU General
** Public License.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with this program; if not, write to the Free Software
** Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
*/

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdio.h>
#include <stdint.h>
#include <sys/types.h>
#include "sfbpf-int.h"

struct entry {
    uint32_t code;
    const char *name;
};

static struct entry classes[] = {
    { BPF_LD  , "LD  "},
    { BPF_LDX , "LDX "},
    { BPF_ST  , "ST  "},
    { BPF_STX , "STX "},
    { BPF_ALU , "ALU "},
    { BPF_JMP , "JMP "},
    { BPF_RET , "RET "},
    { BPF_MISC, "MISC"},
    { 0,         NULL }
};

static struct entry ldx_sizes[] = {
    { BPF_W, "W"},
    { BPF_H, "H"},
    { BPF_B, "B"},
    { 0,   NULL }
};

static struct entry ldx_modes[] = {
    { BPF_IMM, "IMM"},
    { BPF_ABS, "ABS"},
    { BPF_IND, "IND"},
    { BPF_MEM, "MEM"},
    { BPF_LEN, "LEN"},
    { BPF_MSH, "MSH"},
    { 0,       NULL }
};

static struct entry alu_ops[] = {
    { BPF_ADD, "ADD"},
    { BPF_SUB, "SUB"},
    { BPF_MUL, "MUL"},
    { BPF_DIV, "DIV"},
    { BPF_OR , "OR "},
    { BPF_AND, "AND"},
    { BPF_LSH, "LSH"},
    { BPF_RSH, "RSH"},
    { BPF_NEG, "NEG"},
    { 0,       NULL }
};

static struct entry jmp_ops[] = {
    { BPF_JA  , "JA  "},
    { BPF_JEQ , "JEQ "},
    { BPF_JGT , "JGT "},
    { BPF_JGE , "JGE "},
    { BPF_JSET, "JSET"},
    { 0,         NULL }
};

static struct entry srcs[] = {
    { BPF_K, "K"},
    { BPF_X, "X"},
    { 0,   NULL }
};

static struct entry rvals[] = {
    { BPF_K, "K"},
    { BPF_X, "X"},
    { BPF_A, "A"},
    { 0,   NULL }
};

static struct entry misc_ops[] = {
    { BPF_TAX, "TAX"},
    { BPF_TXA, "TXA"},
    { 0,       NULL }
};

static const char *get_code_name(struct entry *table, uint32_t code)
{
    int i;

    for (i = 0; table[i].name; i++)
    {
        if (code == table[i].code)
            return table[i].name;
    }
    return NULL;
}

static int get_size(uint32_t code)
{
    code = BPF_SIZE(code);
    if (code == BPF_W)
        return 4;
    if (code == BPF_H)
        return 2;
    if (code == BPF_B)
        return 1;
    return 0;
}

static void print_ld(struct bpf_insn *inst)
{
    printf("LD   A <- ");
    switch (BPF_MODE(inst->code))
    {
        case BPF_ABS:
            printf("P[%d:%d]", inst->k, get_size(inst->code));
            break;
        case BPF_IND:
            printf("P[X+%d:%d]", inst->k, get_size(inst->code));
            break;
        case BPF_LEN:
            printf("len");
            break;
        case BPF_IMM:
            printf("%d", inst->k);
            break;
        case BPF_MEM:
            printf("M[%d]", inst->k);
            break;
        default:
            printf("???");
    }
    printf("\n");
}

static void print_ldx(struct bpf_insn *inst)
{
    printf("LDX  X <- ");
    switch (BPF_MODE(inst->code))
    {
        case BPF_LEN:
            printf("len");
            break;
        case BPF_IMM:
            printf("%d", inst->k);
            break;
        case BPF_MEM:
            printf("M[%d]", inst->k);
            break;
        case BPF_MSH:
            printf("4*(P[%d:1]&0xf)", inst->k);
            break;
    }
    printf("\n");
}

static void print_alu(struct bpf_insn *inst)
{
    const char *op;

    switch (BPF_OP(inst->code))
    {
        case BPF_ADD:
            op = "+";
            break;
        case BPF_SUB:
            op = "-";
            break;
        case BPF_MUL:
            op = "*";
            break;
        case BPF_DIV:
            op = "/";
            break;
        case BPF_OR:
            op = "|";
            break;
        case BPF_AND:
            op = "&";
            break;
        case BPF_LSH:
            op = "<<";
            break;
        case BPF_RSH:
            op = ">>";
            break;
        case BPF_NEG:
            printf("ALU  -A\n");
            return;
        default:
            op = "???";
            break;
    }
    printf("ALU  A <- A %s \n", op);
    switch(BPF_SRC(inst->code))
    {
        case BPF_K:
            printf("%d", inst->k);
            break;
        case BPF_X:
            printf("X");
            break;
        default:
            printf("???");
    }
    printf("\n");
}

static void print_jmp(struct bpf_insn *inst, int line)
{
    const char *op;

    switch(BPF_OP(inst->code))
    {
        case BPF_JA:
            printf("JMP  L%d\n", line + 1 + inst->k);
            return;
        case BPF_JEQ:
            op = "==";
            break;
        case BPF_JGT:
            op = ">";
            break;
        case BPF_JGE:
            op = ">=";
            break;
        case BPF_JSET:
            op = "&";
            break;
        default:
            op = "???";
            break;
    }
    printf("JMP  (A %s ", op);
    switch(BPF_SRC(inst->code))
    {
        case BPF_K:
            printf("%d", inst->k);
            break;
        case BPF_X:
            printf("X");
            break;
        default:
            printf("???");
            break;
    }
    printf(") ? L%d : L%d\n", line + 1 + inst->jt, line + 1 + inst->jf);
}

static void print_ret(struct bpf_insn *inst)
{
    printf("RET  accept ");
    switch (BPF_RVAL(inst->code))
    {
        case BPF_K:
            printf("%d", inst->k);
            break;
        case BPF_X:
            printf("X");
            break;
        case BPF_A:
            printf("A");
            break;
        default:
            printf("???");
            break;
    }
    printf(" bytes\n");
}

static void print_misc(struct bpf_insn *inst)
{
    printf("MISC ");
    switch (BPF_MISCOP(inst->code))
    {
        case BPF_TAX:
            printf("X <- A");
            break;
        case BPF_TXA:
            printf("A <- X");
            break;
        default:
            printf("???");
            break;
    }
    printf("\n");
}

static void pretty_print_instruction(struct bpf_insn *inst, int line)
{
    switch(BPF_CLASS(inst->code))
    {
        case BPF_LD:
            print_ld(inst);
            break;
        case BPF_LDX:
            print_ldx(inst);
            break;
        case BPF_ST:
            printf("ST   M[%d] <- A\n", inst->k);
            break;
        case BPF_STX:
            printf("STX  M[%d] <- X\n", inst->k);
            break;
        case BPF_ALU:
            print_alu(inst);
            break;
        case BPF_JMP:
            print_jmp(inst, line);
            break;
        case BPF_RET:
            print_ret(inst);
            break;
        case BPF_MISC:
            print_misc(inst);
            break;
    }
}

static void print_instruction(struct bpf_insn *inst)
{
    printf("%s ", get_code_name(classes, BPF_CLASS(inst->code)));
    switch(BPF_CLASS(inst->code))
    {
        case BPF_LD:
        case BPF_LDX:
            printf("size=%s mode=%s\n", get_code_name(ldx_sizes, BPF_SIZE(inst->code)), get_code_name(ldx_modes, BPF_MODE(inst->code)));
            break;
        case BPF_ALU:
            printf("op=%s src=%s\n", get_code_name(alu_ops, BPF_OP(inst->code)), get_code_name(srcs, BPF_SRC(inst->code)));
            break;
        case BPF_JMP:
            printf("op=%s src=%s\n", get_code_name(jmp_ops, BPF_OP(inst->code)), get_code_name(srcs, BPF_SRC(inst->code)));
            break;
        case BPF_RET:
            printf("rval=%s\n", get_code_name(rvals, BPF_RVAL(inst->code)));
            break;
        case BPF_MISC:
            printf("op=%s\n", get_code_name(misc_ops, BPF_MISCOP(inst->code)));
            break;
    }
    printf("    jt=%u jf=%u k=%u\n", inst->jt, inst->jf, inst->k);
}

SO_PUBLIC void sfbpf_print(struct bpf_program *fp, int verbose)
{
    struct bpf_insn *bi;
    unsigned int i;

    printf("Printing BPF:\n");
    for (i = 0; i < fp->bf_len; i++)
    {
        bi = fp->bf_insns + i;
        printf("%3d: ", i);
        if (verbose)
            print_instruction(bi);
        else
            pretty_print_instruction(bi, i);
    }
}

