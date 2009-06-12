/* Generated automatically by gengenrtl from rtl.def.  */

#ifndef GCC_GENRTL_H
#define GCC_GENRTL_H

extern rtx gen_rtx_fmt_s	PARAMS ((RTX_CODE, enum machine_mode mode,
				       const char *arg0));
extern rtx gen_rtx_fmt_ee	PARAMS ((RTX_CODE, enum machine_mode mode,
				       rtx arg0, rtx arg1));
extern rtx gen_rtx_fmt_ue	PARAMS ((RTX_CODE, enum machine_mode mode,
				       rtx arg0, rtx arg1));
extern rtx gen_rtx_fmt_iss	PARAMS ((RTX_CODE, enum machine_mode mode,
				       int arg0, const char *arg1,
				       const char *arg2));
extern rtx gen_rtx_fmt_is	PARAMS ((RTX_CODE, enum machine_mode mode,
				       int arg0, const char *arg1));
extern rtx gen_rtx_fmt_i	PARAMS ((RTX_CODE, enum machine_mode mode,
				       int arg0));
extern rtx gen_rtx_fmt_isE	PARAMS ((RTX_CODE, enum machine_mode mode,
				       int arg0, const char *arg1,
				       rtvec arg2));
extern rtx gen_rtx_fmt_iE	PARAMS ((RTX_CODE, enum machine_mode mode,
				       int arg0, rtvec arg1));
extern rtx gen_rtx_fmt_Ess	PARAMS ((RTX_CODE, enum machine_mode mode,
				       rtvec arg0, const char *arg1,
				       const char *arg2));
extern rtx gen_rtx_fmt_sEss	PARAMS ((RTX_CODE, enum machine_mode mode,
				       const char *arg0, rtvec arg1,
				       const char *arg2, const char *arg3));
extern rtx gen_rtx_fmt_eE	PARAMS ((RTX_CODE, enum machine_mode mode,
				       rtx arg0, rtvec arg1));
extern rtx gen_rtx_fmt_E	PARAMS ((RTX_CODE, enum machine_mode mode,
				       rtvec arg0));
extern rtx gen_rtx_fmt_e	PARAMS ((RTX_CODE, enum machine_mode mode,
				       rtx arg0));
extern rtx gen_rtx_fmt_ss	PARAMS ((RTX_CODE, enum machine_mode mode,
				       const char *arg0, const char *arg1));
extern rtx gen_rtx_fmt_sies	PARAMS ((RTX_CODE, enum machine_mode mode,
				       const char *arg0, int arg1,
				       rtx arg2, const char *arg3));
extern rtx gen_rtx_fmt_sse	PARAMS ((RTX_CODE, enum machine_mode mode,
				       const char *arg0, const char *arg1,
				       rtx arg2));
extern rtx gen_rtx_fmt_sE	PARAMS ((RTX_CODE, enum machine_mode mode,
				       const char *arg0, rtvec arg1));
extern rtx gen_rtx_fmt_iuuBteiee	PARAMS ((RTX_CODE, enum machine_mode mode,
				       int arg0, rtx arg1, rtx arg2,
				       struct basic_block_def *arg3,
				       union tree_node *arg4, rtx arg5,
				       int arg6, rtx arg7, rtx arg8));
extern rtx gen_rtx_fmt_iuuBteiee0	PARAMS ((RTX_CODE, enum machine_mode mode,
				       int arg0, rtx arg1, rtx arg2,
				       struct basic_block_def *arg3,
				       union tree_node *arg4, rtx arg5,
				       int arg6, rtx arg7, rtx arg8));
extern rtx gen_rtx_fmt_iuuBteieee	PARAMS ((RTX_CODE, enum machine_mode mode,
				       int arg0, rtx arg1, rtx arg2,
				       struct basic_block_def *arg3,
				       union tree_node *arg4, rtx arg5,
				       int arg6, rtx arg7, rtx arg8,
				       rtx arg9));
extern rtx gen_rtx_fmt_iuu000000	PARAMS ((RTX_CODE, enum machine_mode mode,
				       int arg0, rtx arg1, rtx arg2));
extern rtx gen_rtx_fmt_iuuB00is	PARAMS ((RTX_CODE, enum machine_mode mode,
				       int arg0, rtx arg1, rtx arg2,
				       struct basic_block_def *arg3,
				       int arg4, const char *arg5));
extern rtx gen_rtx_fmt_ssiEEsi	PARAMS ((RTX_CODE, enum machine_mode mode,
				       const char *arg0, const char *arg1,
				       int arg2, rtvec arg3, rtvec arg4,
				       const char *arg5, int arg6));
extern rtx gen_rtx_fmt_Ei	PARAMS ((RTX_CODE, enum machine_mode mode,
				       rtvec arg0, int arg1));
extern rtx gen_rtx_fmt_eEee0	PARAMS ((RTX_CODE, enum machine_mode mode,
				       rtx arg0, rtvec arg1, rtx arg2,
				       rtx arg3));
extern rtx gen_rtx_fmt_eee	PARAMS ((RTX_CODE, enum machine_mode mode,
				       rtx arg0, rtx arg1, rtx arg2));
extern rtx gen_rtx_fmt_	PARAMS ((RTX_CODE, enum machine_mode mode));
extern rtx gen_rtx_fmt_w	PARAMS ((RTX_CODE, enum machine_mode mode,
				       HOST_WIDE_INT arg0));
extern rtx gen_rtx_fmt_wwww	PARAMS ((RTX_CODE, enum machine_mode mode,
				       HOST_WIDE_INT arg0,
				       HOST_WIDE_INT arg1,
				       HOST_WIDE_INT arg2,
				       HOST_WIDE_INT arg3));
extern rtx gen_rtx_fmt_0	PARAMS ((RTX_CODE, enum machine_mode mode));
extern rtx gen_rtx_fmt_i0	PARAMS ((RTX_CODE, enum machine_mode mode,
				       int arg0));
extern rtx gen_rtx_fmt_ei	PARAMS ((RTX_CODE, enum machine_mode mode,
				       rtx arg0, int arg1));
extern rtx gen_rtx_fmt_e0	PARAMS ((RTX_CODE, enum machine_mode mode,
				       rtx arg0));
extern rtx gen_rtx_fmt_u00	PARAMS ((RTX_CODE, enum machine_mode mode,
				       rtx arg0));
extern rtx gen_rtx_fmt_eit	PARAMS ((RTX_CODE, enum machine_mode mode,
				       rtx arg0, int arg1,
				       union tree_node *arg2));
extern rtx gen_rtx_fmt_eeeee	PARAMS ((RTX_CODE, enum machine_mode mode,
				       rtx arg0, rtx arg1, rtx arg2,
				       rtx arg3, rtx arg4));
extern rtx gen_rtx_fmt_Ee	PARAMS ((RTX_CODE, enum machine_mode mode,
				       rtvec arg0, rtx arg1));
extern rtx gen_rtx_fmt_uuEiiiiiibbii	PARAMS ((RTX_CODE, enum machine_mode mode,
				       rtx arg0, rtx arg1, rtvec arg2,
				       int arg3, int arg4, int arg5,
				       int arg6, int arg7, int arg8,
				       struct bitmap_head_def *arg9,
				       struct bitmap_head_def *arg10,
				       int arg11, int arg12));
extern rtx gen_rtx_fmt_iiiiiiiitt	PARAMS ((RTX_CODE, enum machine_mode mode,
				       int arg0, int arg1, int arg2,
				       int arg3, int arg4, int arg5,
				       int arg6, int arg7,
				       union tree_node *arg8,
				       union tree_node *arg9));
extern rtx gen_rtx_fmt_eti	PARAMS ((RTX_CODE, enum machine_mode mode,
				       rtx arg0, union tree_node *arg1,
				       int arg2));
extern rtx gen_rtx_fmt_bi	PARAMS ((RTX_CODE, enum machine_mode mode,
				       struct bitmap_head_def *arg0,
				       int arg1));
extern rtx gen_rtx_fmt_uuuu	PARAMS ((RTX_CODE, enum machine_mode mode,
				       rtx arg0, rtx arg1, rtx arg2,
				       rtx arg3));

#define gen_rtx_INCLUDE(MODE, ARG0) \
  gen_rtx_fmt_s (INCLUDE, (MODE), (ARG0))
#define gen_rtx_EXPR_LIST(MODE, ARG0, ARG1) \
  gen_rtx_fmt_ee (EXPR_LIST, (MODE), (ARG0), (ARG1))
#define gen_rtx_INSN_LIST(MODE, ARG0, ARG1) \
  gen_rtx_fmt_ue (INSN_LIST, (MODE), (ARG0), (ARG1))
#define gen_rtx_MATCH_OPERAND(MODE, ARG0, ARG1, ARG2) \
  gen_rtx_fmt_iss (MATCH_OPERAND, (MODE), (ARG0), (ARG1), (ARG2))
#define gen_rtx_MATCH_SCRATCH(MODE, ARG0, ARG1) \
  gen_rtx_fmt_is (MATCH_SCRATCH, (MODE), (ARG0), (ARG1))
#define gen_rtx_MATCH_DUP(MODE, ARG0) \
  gen_rtx_fmt_i (MATCH_DUP, (MODE), (ARG0))
#define gen_rtx_MATCH_OPERATOR(MODE, ARG0, ARG1, ARG2) \
  gen_rtx_fmt_isE (MATCH_OPERATOR, (MODE), (ARG0), (ARG1), (ARG2))
#define gen_rtx_MATCH_PARALLEL(MODE, ARG0, ARG1, ARG2) \
  gen_rtx_fmt_isE (MATCH_PARALLEL, (MODE), (ARG0), (ARG1), (ARG2))
#define gen_rtx_MATCH_OP_DUP(MODE, ARG0, ARG1) \
  gen_rtx_fmt_iE (MATCH_OP_DUP, (MODE), (ARG0), (ARG1))
#define gen_rtx_MATCH_PAR_DUP(MODE, ARG0, ARG1) \
  gen_rtx_fmt_iE (MATCH_PAR_DUP, (MODE), (ARG0), (ARG1))
#define gen_rtx_MATCH_INSN(MODE, ARG0, ARG1) \
  gen_rtx_fmt_is (MATCH_INSN, (MODE), (ARG0), (ARG1))
#define gen_rtx_DEFINE_COMBINE(MODE, ARG0, ARG1, ARG2) \
  gen_rtx_fmt_Ess (DEFINE_COMBINE, (MODE), (ARG0), (ARG1), (ARG2))
#define gen_rtx_DEFINE_EXPAND(MODE, ARG0, ARG1, ARG2, ARG3) \
  gen_rtx_fmt_sEss (DEFINE_EXPAND, (MODE), (ARG0), (ARG1), (ARG2), (ARG3))
#define gen_rtx_DEFINE_DELAY(MODE, ARG0, ARG1) \
  gen_rtx_fmt_eE (DEFINE_DELAY, (MODE), (ARG0), (ARG1))
#define gen_rtx_DEFINE_COND_EXEC(MODE, ARG0, ARG1, ARG2) \
  gen_rtx_fmt_Ess (DEFINE_COND_EXEC, (MODE), (ARG0), (ARG1), (ARG2))
#define gen_rtx_SEQUENCE(MODE, ARG0) \
  gen_rtx_fmt_E (SEQUENCE, (MODE), (ARG0))
#define gen_rtx_ADDRESS(MODE, ARG0) \
  gen_rtx_fmt_e (ADDRESS, (MODE), (ARG0))
#define gen_rtx_EXCLUSION_SET(MODE, ARG0, ARG1) \
  gen_rtx_fmt_ss (EXCLUSION_SET, (MODE), (ARG0), (ARG1))
#define gen_rtx_PRESENCE_SET(MODE, ARG0, ARG1) \
  gen_rtx_fmt_ss (PRESENCE_SET, (MODE), (ARG0), (ARG1))
#define gen_rtx_ABSENCE_SET(MODE, ARG0, ARG1) \
  gen_rtx_fmt_ss (ABSENCE_SET, (MODE), (ARG0), (ARG1))
#define gen_rtx_DEFINE_AUTOMATON(MODE, ARG0) \
  gen_rtx_fmt_s (DEFINE_AUTOMATON, (MODE), (ARG0))
#define gen_rtx_AUTOMATA_OPTION(MODE, ARG0) \
  gen_rtx_fmt_s (AUTOMATA_OPTION, (MODE), (ARG0))
#define gen_rtx_DEFINE_RESERVATION(MODE, ARG0, ARG1) \
  gen_rtx_fmt_ss (DEFINE_RESERVATION, (MODE), (ARG0), (ARG1))
#define gen_rtx_DEFINE_INSN_RESERVATION(MODE, ARG0, ARG1, ARG2, ARG3) \
  gen_rtx_fmt_sies (DEFINE_INSN_RESERVATION, (MODE), (ARG0), (ARG1), (ARG2), (ARG3))
#define gen_rtx_DEFINE_ATTR(MODE, ARG0, ARG1, ARG2) \
  gen_rtx_fmt_sse (DEFINE_ATTR, (MODE), (ARG0), (ARG1), (ARG2))
#define gen_rtx_ATTR(MODE, ARG0) \
  gen_rtx_fmt_s (ATTR, (MODE), (ARG0))
#define gen_rtx_SET_ATTR(MODE, ARG0, ARG1) \
  gen_rtx_fmt_ss (SET_ATTR, (MODE), (ARG0), (ARG1))
#define gen_rtx_SET_ATTR_ALTERNATIVE(MODE, ARG0, ARG1) \
  gen_rtx_fmt_sE (SET_ATTR_ALTERNATIVE, (MODE), (ARG0), (ARG1))
#define gen_rtx_EQ_ATTR(MODE, ARG0, ARG1) \
  gen_rtx_fmt_ss (EQ_ATTR, (MODE), (ARG0), (ARG1))
#define gen_rtx_ATTR_FLAG(MODE, ARG0) \
  gen_rtx_fmt_s (ATTR_FLAG, (MODE), (ARG0))
#define gen_rtx_INSN(MODE, ARG0, ARG1, ARG2, ARG3, ARG4, ARG5, ARG6, ARG7, ARG8) \
  gen_rtx_fmt_iuuBteiee (INSN, (MODE), (ARG0), (ARG1), (ARG2), (ARG3), (ARG4), (ARG5), (ARG6), (ARG7), (ARG8))
#define gen_rtx_JUMP_INSN(MODE, ARG0, ARG1, ARG2, ARG3, ARG4, ARG5, ARG6, ARG7, ARG8) \
  gen_rtx_fmt_iuuBteiee0 (JUMP_INSN, (MODE), (ARG0), (ARG1), (ARG2), (ARG3), (ARG4), (ARG5), (ARG6), (ARG7), (ARG8))
#define gen_rtx_CALL_INSN(MODE, ARG0, ARG1, ARG2, ARG3, ARG4, ARG5, ARG6, ARG7, ARG8, ARG9) \
  gen_rtx_fmt_iuuBteieee (CALL_INSN, (MODE), (ARG0), (ARG1), (ARG2), (ARG3), (ARG4), (ARG5), (ARG6), (ARG7), (ARG8), (ARG9))
#define gen_rtx_BARRIER(MODE, ARG0, ARG1, ARG2) \
  gen_rtx_fmt_iuu000000 (BARRIER, (MODE), (ARG0), (ARG1), (ARG2))
#define gen_rtx_CODE_LABEL(MODE, ARG0, ARG1, ARG2, ARG3, ARG4, ARG5) \
  gen_rtx_fmt_iuuB00is (CODE_LABEL, (MODE), (ARG0), (ARG1), (ARG2), (ARG3), (ARG4), (ARG5))
#define gen_rtx_COND_EXEC(MODE, ARG0, ARG1) \
  gen_rtx_fmt_ee (COND_EXEC, (MODE), (ARG0), (ARG1))
#define gen_rtx_PARALLEL(MODE, ARG0) \
  gen_rtx_fmt_E (PARALLEL, (MODE), (ARG0))
#define gen_rtx_ASM_INPUT(MODE, ARG0) \
  gen_rtx_fmt_s (ASM_INPUT, (MODE), (ARG0))
#define gen_rtx_ASM_OPERANDS(MODE, ARG0, ARG1, ARG2, ARG3, ARG4, ARG5, ARG6) \
  gen_rtx_fmt_ssiEEsi (ASM_OPERANDS, (MODE), (ARG0), (ARG1), (ARG2), (ARG3), (ARG4), (ARG5), (ARG6))
#define gen_rtx_UNSPEC(MODE, ARG0, ARG1) \
  gen_rtx_fmt_Ei (UNSPEC, (MODE), (ARG0), (ARG1))
#define gen_rtx_UNSPEC_VOLATILE(MODE, ARG0, ARG1) \
  gen_rtx_fmt_Ei (UNSPEC_VOLATILE, (MODE), (ARG0), (ARG1))
#define gen_rtx_ADDR_VEC(MODE, ARG0) \
  gen_rtx_fmt_E (ADDR_VEC, (MODE), (ARG0))
#define gen_rtx_ADDR_DIFF_VEC(MODE, ARG0, ARG1, ARG2, ARG3) \
  gen_rtx_fmt_eEee0 (ADDR_DIFF_VEC, (MODE), (ARG0), (ARG1), (ARG2), (ARG3))
#define gen_rtx_PREFETCH(MODE, ARG0, ARG1, ARG2) \
  gen_rtx_fmt_eee (PREFETCH, (MODE), (ARG0), (ARG1), (ARG2))
#define gen_rtx_SET(MODE, ARG0, ARG1) \
  gen_rtx_fmt_ee (SET, (MODE), (ARG0), (ARG1))
#define gen_rtx_USE(MODE, ARG0) \
  gen_rtx_fmt_e (USE, (MODE), (ARG0))
#define gen_rtx_CLOBBER(MODE, ARG0) \
  gen_rtx_fmt_e (CLOBBER, (MODE), (ARG0))
#define gen_rtx_CALL(MODE, ARG0, ARG1) \
  gen_rtx_fmt_ee (CALL, (MODE), (ARG0), (ARG1))
#define gen_rtx_RETURN(MODE) \
  gen_rtx_fmt_ (RETURN, (MODE))
#define gen_rtx_TRAP_IF(MODE, ARG0, ARG1) \
  gen_rtx_fmt_ee (TRAP_IF, (MODE), (ARG0), (ARG1))
#define gen_rtx_RESX(MODE, ARG0) \
  gen_rtx_fmt_i (RESX, (MODE), (ARG0))
#define gen_rtx_raw_CONST_INT(MODE, ARG0) \
  gen_rtx_fmt_w (CONST_INT, (MODE), (ARG0))
#define gen_rtx_raw_CONST_VECTOR(MODE, ARG0) \
  gen_rtx_fmt_E (CONST_VECTOR, (MODE), (ARG0))
#define gen_rtx_CONST_STRING(MODE, ARG0) \
  gen_rtx_fmt_s (CONST_STRING, (MODE), (ARG0))
#define gen_rtx_CONST(MODE, ARG0) \
  gen_rtx_fmt_e (CONST, (MODE), (ARG0))
#define gen_rtx_PC(MODE) \
  gen_rtx_fmt_ (PC, (MODE))
#define gen_rtx_VALUE(MODE) \
  gen_rtx_fmt_0 (VALUE, (MODE))
#define gen_rtx_raw_REG(MODE, ARG0) \
  gen_rtx_fmt_i0 (REG, (MODE), (ARG0))
#define gen_rtx_SCRATCH(MODE) \
  gen_rtx_fmt_0 (SCRATCH, (MODE))
#define gen_rtx_raw_SUBREG(MODE, ARG0, ARG1) \
  gen_rtx_fmt_ei (SUBREG, (MODE), (ARG0), (ARG1))
#define gen_rtx_STRICT_LOW_PART(MODE, ARG0) \
  gen_rtx_fmt_e (STRICT_LOW_PART, (MODE), (ARG0))
#define gen_rtx_CONCAT(MODE, ARG0, ARG1) \
  gen_rtx_fmt_ee (CONCAT, (MODE), (ARG0), (ARG1))
#define gen_rtx_raw_MEM(MODE, ARG0) \
  gen_rtx_fmt_e0 (MEM, (MODE), (ARG0))
#define gen_rtx_LABEL_REF(MODE, ARG0) \
  gen_rtx_fmt_u00 (LABEL_REF, (MODE), (ARG0))
#define gen_rtx_SYMBOL_REF(MODE, ARG0) \
  gen_rtx_fmt_s (SYMBOL_REF, (MODE), (ARG0))
#define gen_rtx_CC0(MODE) \
  gen_rtx_fmt_ (CC0, (MODE))
#define gen_rtx_ADDRESSOF(MODE, ARG0, ARG1, ARG2) \
  gen_rtx_fmt_eit (ADDRESSOF, (MODE), (ARG0), (ARG1), (ARG2))
#define gen_rtx_QUEUED(MODE, ARG0, ARG1, ARG2, ARG3, ARG4) \
  gen_rtx_fmt_eeeee (QUEUED, (MODE), (ARG0), (ARG1), (ARG2), (ARG3), (ARG4))
#define gen_rtx_IF_THEN_ELSE(MODE, ARG0, ARG1, ARG2) \
  gen_rtx_fmt_eee (IF_THEN_ELSE, (MODE), (ARG0), (ARG1), (ARG2))
#define gen_rtx_COND(MODE, ARG0, ARG1) \
  gen_rtx_fmt_Ee (COND, (MODE), (ARG0), (ARG1))
#define gen_rtx_COMPARE(MODE, ARG0, ARG1) \
  gen_rtx_fmt_ee (COMPARE, (MODE), (ARG0), (ARG1))
#define gen_rtx_PLUS(MODE, ARG0, ARG1) \
  gen_rtx_fmt_ee (PLUS, (MODE), (ARG0), (ARG1))
#define gen_rtx_MINUS(MODE, ARG0, ARG1) \
  gen_rtx_fmt_ee (MINUS, (MODE), (ARG0), (ARG1))
#define gen_rtx_NEG(MODE, ARG0) \
  gen_rtx_fmt_e (NEG, (MODE), (ARG0))
#define gen_rtx_MULT(MODE, ARG0, ARG1) \
  gen_rtx_fmt_ee (MULT, (MODE), (ARG0), (ARG1))
#define gen_rtx_DIV(MODE, ARG0, ARG1) \
  gen_rtx_fmt_ee (DIV, (MODE), (ARG0), (ARG1))
#define gen_rtx_MOD(MODE, ARG0, ARG1) \
  gen_rtx_fmt_ee (MOD, (MODE), (ARG0), (ARG1))
#define gen_rtx_UDIV(MODE, ARG0, ARG1) \
  gen_rtx_fmt_ee (UDIV, (MODE), (ARG0), (ARG1))
#define gen_rtx_UMOD(MODE, ARG0, ARG1) \
  gen_rtx_fmt_ee (UMOD, (MODE), (ARG0), (ARG1))
#define gen_rtx_AND(MODE, ARG0, ARG1) \
  gen_rtx_fmt_ee (AND, (MODE), (ARG0), (ARG1))
#define gen_rtx_IOR(MODE, ARG0, ARG1) \
  gen_rtx_fmt_ee (IOR, (MODE), (ARG0), (ARG1))
#define gen_rtx_XOR(MODE, ARG0, ARG1) \
  gen_rtx_fmt_ee (XOR, (MODE), (ARG0), (ARG1))
#define gen_rtx_NOT(MODE, ARG0) \
  gen_rtx_fmt_e (NOT, (MODE), (ARG0))
#define gen_rtx_ASHIFT(MODE, ARG0, ARG1) \
  gen_rtx_fmt_ee (ASHIFT, (MODE), (ARG0), (ARG1))
#define gen_rtx_ROTATE(MODE, ARG0, ARG1) \
  gen_rtx_fmt_ee (ROTATE, (MODE), (ARG0), (ARG1))
#define gen_rtx_ASHIFTRT(MODE, ARG0, ARG1) \
  gen_rtx_fmt_ee (ASHIFTRT, (MODE), (ARG0), (ARG1))
#define gen_rtx_LSHIFTRT(MODE, ARG0, ARG1) \
  gen_rtx_fmt_ee (LSHIFTRT, (MODE), (ARG0), (ARG1))
#define gen_rtx_ROTATERT(MODE, ARG0, ARG1) \
  gen_rtx_fmt_ee (ROTATERT, (MODE), (ARG0), (ARG1))
#define gen_rtx_SMIN(MODE, ARG0, ARG1) \
  gen_rtx_fmt_ee (SMIN, (MODE), (ARG0), (ARG1))
#define gen_rtx_SMAX(MODE, ARG0, ARG1) \
  gen_rtx_fmt_ee (SMAX, (MODE), (ARG0), (ARG1))
#define gen_rtx_UMIN(MODE, ARG0, ARG1) \
  gen_rtx_fmt_ee (UMIN, (MODE), (ARG0), (ARG1))
#define gen_rtx_UMAX(MODE, ARG0, ARG1) \
  gen_rtx_fmt_ee (UMAX, (MODE), (ARG0), (ARG1))
#define gen_rtx_PRE_DEC(MODE, ARG0) \
  gen_rtx_fmt_e (PRE_DEC, (MODE), (ARG0))
#define gen_rtx_PRE_INC(MODE, ARG0) \
  gen_rtx_fmt_e (PRE_INC, (MODE), (ARG0))
#define gen_rtx_POST_DEC(MODE, ARG0) \
  gen_rtx_fmt_e (POST_DEC, (MODE), (ARG0))
#define gen_rtx_POST_INC(MODE, ARG0) \
  gen_rtx_fmt_e (POST_INC, (MODE), (ARG0))
#define gen_rtx_PRE_MODIFY(MODE, ARG0, ARG1) \
  gen_rtx_fmt_ee (PRE_MODIFY, (MODE), (ARG0), (ARG1))
#define gen_rtx_POST_MODIFY(MODE, ARG0, ARG1) \
  gen_rtx_fmt_ee (POST_MODIFY, (MODE), (ARG0), (ARG1))
#define gen_rtx_NE(MODE, ARG0, ARG1) \
  gen_rtx_fmt_ee (NE, (MODE), (ARG0), (ARG1))
#define gen_rtx_EQ(MODE, ARG0, ARG1) \
  gen_rtx_fmt_ee (EQ, (MODE), (ARG0), (ARG1))
#define gen_rtx_GE(MODE, ARG0, ARG1) \
  gen_rtx_fmt_ee (GE, (MODE), (ARG0), (ARG1))
#define gen_rtx_GT(MODE, ARG0, ARG1) \
  gen_rtx_fmt_ee (GT, (MODE), (ARG0), (ARG1))
#define gen_rtx_LE(MODE, ARG0, ARG1) \
  gen_rtx_fmt_ee (LE, (MODE), (ARG0), (ARG1))
#define gen_rtx_LT(MODE, ARG0, ARG1) \
  gen_rtx_fmt_ee (LT, (MODE), (ARG0), (ARG1))
#define gen_rtx_GEU(MODE, ARG0, ARG1) \
  gen_rtx_fmt_ee (GEU, (MODE), (ARG0), (ARG1))
#define gen_rtx_GTU(MODE, ARG0, ARG1) \
  gen_rtx_fmt_ee (GTU, (MODE), (ARG0), (ARG1))
#define gen_rtx_LEU(MODE, ARG0, ARG1) \
  gen_rtx_fmt_ee (LEU, (MODE), (ARG0), (ARG1))
#define gen_rtx_LTU(MODE, ARG0, ARG1) \
  gen_rtx_fmt_ee (LTU, (MODE), (ARG0), (ARG1))
#define gen_rtx_UNORDERED(MODE, ARG0, ARG1) \
  gen_rtx_fmt_ee (UNORDERED, (MODE), (ARG0), (ARG1))
#define gen_rtx_ORDERED(MODE, ARG0, ARG1) \
  gen_rtx_fmt_ee (ORDERED, (MODE), (ARG0), (ARG1))
#define gen_rtx_UNEQ(MODE, ARG0, ARG1) \
  gen_rtx_fmt_ee (UNEQ, (MODE), (ARG0), (ARG1))
#define gen_rtx_UNGE(MODE, ARG0, ARG1) \
  gen_rtx_fmt_ee (UNGE, (MODE), (ARG0), (ARG1))
#define gen_rtx_UNGT(MODE, ARG0, ARG1) \
  gen_rtx_fmt_ee (UNGT, (MODE), (ARG0), (ARG1))
#define gen_rtx_UNLE(MODE, ARG0, ARG1) \
  gen_rtx_fmt_ee (UNLE, (MODE), (ARG0), (ARG1))
#define gen_rtx_UNLT(MODE, ARG0, ARG1) \
  gen_rtx_fmt_ee (UNLT, (MODE), (ARG0), (ARG1))
#define gen_rtx_LTGT(MODE, ARG0, ARG1) \
  gen_rtx_fmt_ee (LTGT, (MODE), (ARG0), (ARG1))
#define gen_rtx_SIGN_EXTEND(MODE, ARG0) \
  gen_rtx_fmt_e (SIGN_EXTEND, (MODE), (ARG0))
#define gen_rtx_ZERO_EXTEND(MODE, ARG0) \
  gen_rtx_fmt_e (ZERO_EXTEND, (MODE), (ARG0))
#define gen_rtx_TRUNCATE(MODE, ARG0) \
  gen_rtx_fmt_e (TRUNCATE, (MODE), (ARG0))
#define gen_rtx_FLOAT_EXTEND(MODE, ARG0) \
  gen_rtx_fmt_e (FLOAT_EXTEND, (MODE), (ARG0))
#define gen_rtx_FLOAT_TRUNCATE(MODE, ARG0) \
  gen_rtx_fmt_e (FLOAT_TRUNCATE, (MODE), (ARG0))
#define gen_rtx_FLOAT(MODE, ARG0) \
  gen_rtx_fmt_e (FLOAT, (MODE), (ARG0))
#define gen_rtx_FIX(MODE, ARG0) \
  gen_rtx_fmt_e (FIX, (MODE), (ARG0))
#define gen_rtx_UNSIGNED_FLOAT(MODE, ARG0) \
  gen_rtx_fmt_e (UNSIGNED_FLOAT, (MODE), (ARG0))
#define gen_rtx_UNSIGNED_FIX(MODE, ARG0) \
  gen_rtx_fmt_e (UNSIGNED_FIX, (MODE), (ARG0))
#define gen_rtx_ABS(MODE, ARG0) \
  gen_rtx_fmt_e (ABS, (MODE), (ARG0))
#define gen_rtx_SQRT(MODE, ARG0) \
  gen_rtx_fmt_e (SQRT, (MODE), (ARG0))
#define gen_rtx_FFS(MODE, ARG0) \
  gen_rtx_fmt_e (FFS, (MODE), (ARG0))
#define gen_rtx_SIGN_EXTRACT(MODE, ARG0, ARG1, ARG2) \
  gen_rtx_fmt_eee (SIGN_EXTRACT, (MODE), (ARG0), (ARG1), (ARG2))
#define gen_rtx_ZERO_EXTRACT(MODE, ARG0, ARG1, ARG2) \
  gen_rtx_fmt_eee (ZERO_EXTRACT, (MODE), (ARG0), (ARG1), (ARG2))
#define gen_rtx_HIGH(MODE, ARG0) \
  gen_rtx_fmt_e (HIGH, (MODE), (ARG0))
#define gen_rtx_LO_SUM(MODE, ARG0, ARG1) \
  gen_rtx_fmt_ee (LO_SUM, (MODE), (ARG0), (ARG1))
#define gen_rtx_RANGE_INFO(MODE, ARG0, ARG1, ARG2, ARG3, ARG4, ARG5, ARG6, ARG7, ARG8, ARG9, ARG10, ARG11, ARG12) \
  gen_rtx_fmt_uuEiiiiiibbii (RANGE_INFO, (MODE), (ARG0), (ARG1), (ARG2), (ARG3), (ARG4), (ARG5), (ARG6), (ARG7), (ARG8), (ARG9), (ARG10), (ARG11), (ARG12))
#define gen_rtx_RANGE_REG(MODE, ARG0, ARG1, ARG2, ARG3, ARG4, ARG5, ARG6, ARG7, ARG8, ARG9) \
  gen_rtx_fmt_iiiiiiiitt (RANGE_REG, (MODE), (ARG0), (ARG1), (ARG2), (ARG3), (ARG4), (ARG5), (ARG6), (ARG7), (ARG8), (ARG9))
#define gen_rtx_RANGE_VAR(MODE, ARG0, ARG1, ARG2) \
  gen_rtx_fmt_eti (RANGE_VAR, (MODE), (ARG0), (ARG1), (ARG2))
#define gen_rtx_RANGE_LIVE(MODE, ARG0, ARG1) \
  gen_rtx_fmt_bi (RANGE_LIVE, (MODE), (ARG0), (ARG1))
#define gen_rtx_CONSTANT_P_RTX(MODE, ARG0) \
  gen_rtx_fmt_e (CONSTANT_P_RTX, (MODE), (ARG0))
#define gen_rtx_CALL_PLACEHOLDER(MODE, ARG0, ARG1, ARG2, ARG3) \
  gen_rtx_fmt_uuuu (CALL_PLACEHOLDER, (MODE), (ARG0), (ARG1), (ARG2), (ARG3))
#define gen_rtx_VEC_MERGE(MODE, ARG0, ARG1, ARG2) \
  gen_rtx_fmt_eee (VEC_MERGE, (MODE), (ARG0), (ARG1), (ARG2))
#define gen_rtx_VEC_SELECT(MODE, ARG0, ARG1) \
  gen_rtx_fmt_ee (VEC_SELECT, (MODE), (ARG0), (ARG1))
#define gen_rtx_VEC_CONCAT(MODE, ARG0, ARG1) \
  gen_rtx_fmt_ee (VEC_CONCAT, (MODE), (ARG0), (ARG1))
#define gen_rtx_VEC_DUPLICATE(MODE, ARG0) \
  gen_rtx_fmt_e (VEC_DUPLICATE, (MODE), (ARG0))
#define gen_rtx_SS_PLUS(MODE, ARG0, ARG1) \
  gen_rtx_fmt_ee (SS_PLUS, (MODE), (ARG0), (ARG1))
#define gen_rtx_US_PLUS(MODE, ARG0, ARG1) \
  gen_rtx_fmt_ee (US_PLUS, (MODE), (ARG0), (ARG1))
#define gen_rtx_SS_MINUS(MODE, ARG0, ARG1) \
  gen_rtx_fmt_ee (SS_MINUS, (MODE), (ARG0), (ARG1))
#define gen_rtx_US_MINUS(MODE, ARG0, ARG1) \
  gen_rtx_fmt_ee (US_MINUS, (MODE), (ARG0), (ARG1))
#define gen_rtx_SS_TRUNCATE(MODE, ARG0) \
  gen_rtx_fmt_e (SS_TRUNCATE, (MODE), (ARG0))
#define gen_rtx_US_TRUNCATE(MODE, ARG0) \
  gen_rtx_fmt_e (US_TRUNCATE, (MODE), (ARG0))
#define gen_rtx_PHI(MODE, ARG0) \
  gen_rtx_fmt_E (PHI, (MODE), (ARG0))

#endif /* GCC_GENRTL_H */
