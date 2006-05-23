/*
 * JFFS2-BBC: armlib compressor plugin
 *
 * $Id: 301-jffs-compression,v 1.1 2005/03/26 10:33:31 wbx Exp $
 *
 * Copyright (C) 2004, Ferenc Havasi & Tamas Gergely
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 */

#include "jffs2_bbc_framework.h"

#ifdef __KERNEL__
#include <linux/string.h>
#else
#include <string.h>
#endif

//ORIGIN: include/DataStructures/TypeDefs.h

/*******************************************************************************
* FILE:     TypeDefs.h
* AUTHOR:   Tamás Gergely
* MODIFIED: $Id: 301-jffs-compression,v 1.1 2005/03/26 10:33:31 wbx Exp $
*******************************************************************************/

#ifndef TYPEDEFS_H
#define TYPEDEFS_H

#pragma pack(4)

#ifndef bool
#define bool  char
#define true  1
#define false 0
#endif

#ifndef u8
#define u8  unsigned char
#endif
#ifndef u16
#define u16 unsigned short
#endif
#ifndef u32
#define u32 unsigned long
#endif
#ifndef s8
#define s8  signed char
#endif
#ifndef s16
#define s16 signed short
#endif
#ifndef s32
#define s32 signed long
#endif

typedef struct
{
	u32 capacity;
	u32 size;
	u32 alloc_size;
	void *ptr;
} vector;

#define VECTOR_P_END(vct)   ((void*)(((char*)((vct)->ptr)) + (vct)->size))
#define VECTOR_S_END(vct)   ((void*)(((char*)((vct).ptr)) + (vct).size))

static void vector_clear(vector *);
#ifdef JFFS2_BBC_ARMLIB_MODELGEN
static void vector_reset(vector *);
static void vector_clr_ptr(vector *);
static void vector_add_u8(vector *, u8);
static void vector_add_u16(vector *, u16);
static void vector_add_u32(vector *, u32);
static void vector_add_s8(vector *, s8);
static void vector_add_s16(vector *, s16);
static void vector_add_s32(vector *, s32);
static void vector_add_ptr(vector *, void *);
static void vector_concat(vector *, vector *);
#endif

#endif

//ORIGIN: include/DataStructures/DataTypes.h

/*******************************************************************************
* FILE:     DataTypes.h
* AUTHOR:   Tamás Gergely
* MODIFIED: $Id: 301-jffs-compression,v 1.1 2005/03/26 10:33:31 wbx Exp $
*******************************************************************************/

#ifndef DATATYPES_H
#define DATATYPES_H

//#include "DataStructures/TypeDefs.h"

typedef u16 THUMB_DataType;
typedef u32 ARM_DataType;
typedef u8 TokenType;
typedef u8 PredictorType;
typedef u8 *ProbDist;

typedef vector RawData;
typedef vector RawBlocks;
typedef vector TokenStream;
typedef vector TokenBlocks;
typedef vector LatType;

#define THUMB_DATA_LENGTH    16
#define ARM_DATA_LENGTH      32
#define TOKEN_LENGTH          8
#define TOKEN_MAXVALUE     0xff
#define PREDICTOR_LENGTH      8
#define PREDICTOR_MAXVALUE 0xff

#endif

//ORIGIN: include/DataStructures/BitVector.h

/*******************************************************************************
* FILE:     BitVector.h
* AUTHOR:   Tamás Gergely
* MODIFIED: $Id: 301-jffs-compression,v 1.1 2005/03/26 10:33:31 wbx Exp $
*******************************************************************************/

#ifndef BITVECTOR_H
#define BITVECTOR_H

//#include "DataStructures/TypeDefs.h"

typedef vector BitBlocks;

#pragma pack(4)

typedef struct
{
	u32 freebits;
	u32 capacity;
	u32 size;
	u8 *base;
	u8 *ptr;
} BitVector;

#ifdef JFFS2_BBC_ARMLIB_MODELGEN
static void bitblocks_clear(BitBlocks *);
static void bitvector_clear(BitVector *);
static void bitvector_W_reset(BitVector *);
static void bitvector_W_add0(BitVector *);
static void bitvector_W_add1(BitVector *);
static void bitvector_W_concat_b(BitVector *, BitVector *);
static void bitvector_W_concat_v(BitVector *, vector *);
static void bitvector_W_flush(BitVector *);
static void bitvector_R_reset(BitVector *);
static u8 bitvector_R_get1(BitVector *);
static u8 bitvector_R_get8(BitVector *);
#endif

#define BITVECTOR_P_END(bv)    ((void*)(((bv)->base)+((bv)->size)))
#define BITVECTOR_S_END(bv)    ((void*)( ((bv).base)+ ((bv).size)))
#define BITVECTOR_SKIP(bv,num) ((bv)->ptr) += (num)

#endif

//ORIGIN: include/DataStructures/DecisionTree.h

/*******************************************************************************
* FILE:     DecisionTree.h
* AUTHOR:   Tamás Gergely
* MODIFIED: $Id: 301-jffs-compression,v 1.1 2005/03/26 10:33:31 wbx Exp $
*******************************************************************************/

#ifndef DECISIONTREE_H
#define DECISIONTREE_H

//#include "DataStructures/DataTypes.h"

#pragma pack(4)

#define TREENODETYPE_NULLNODE         0
#define TREENODETYPE_NODE_BINARY_EQ   1
#define TREENODETYPE_LEAF_P           2
#define TREENODETYPE_LEAF_C           3
#define TREENODETYPE_NODE_BINARY_LT   5
#define TREENODETYPE_IS_NODE(n)        (((n) == TREENODETYPE_NODE_BINARY_EQ) || \
                                        ((n) == TREENODETYPE_NODE_BINARY_LT))
#define TREENODETYPE_IS_NODE_BINARY(n) (((n) == TREENODETYPE_NODE_BINARY_EQ) || \
                                        ((n) == TREENODETYPE_NODE_BINARY_LT))

#define TREENODETYPE_IS_LEAF(n)        (((n) == TREENODETYPE_LEAF_P) || \
                                        ((n) == TREENODETYPE_LEAF_C))


#define TREE_SUBTREE_RELATION_LEFT_EQ  !=
#define TREE_SUBTREE_RELATION_RIGHT_EQ ==
#define TREE_SUBTREE_RELATION_LEFT_LT  <
#define TREE_SUBTREE_RELATION_RIGHT_LT >=

#define GET_NODE_PTR_TYPE(n) (((TreeNodeDummy*)(n))->type)

typedef struct
{
	u8 type;
} TreeNodeDummy;

typedef struct
{
	u8 type;		// [TREENODETYPE_NODE_BINARY]
	u8 attribute;
	PredictorType value;
	void *left;
	void *right;
} TreeNodeBinary;

typedef struct
{
	u8 type;		// [TREENODETYPE_LEAF_P]
	u16 pairs;
	PredictorType *probabilities;
} TreeLeafP;

typedef struct
{
	u8 type;		// [TREENODETYPE_LEAF_C]
	PredictorType predicted_class;
} TreeLeafC;

typedef struct
{
	u32 high;
	u32 low;
	u32 max;
} ProbabilityType;


typedef struct
{
	void *root;
	u16 number_of_classes;
	u16 number_of_predictors;
	PredictorType *predictor_max_values;
} DecisionTree;

#ifdef JFFS2_BBC_ARMLIB_MODELGEN
static void decisiontree_delete(DecisionTree *);
static void decisiontree_get_probability_for_token(void *, PredictorType *, TokenType, ProbabilityType *);
static TokenType decisiontree_get_token_for_range(void *, PredictorType *, u32, u32, ProbabilityType *);
#endif

#endif

//ORIGIN: include/DataStructures/PredictorTable.h

/*******************************************************************************
* FILE:     PredictorTable.h
* AUTHOR:   Tamás Gergely
* MODIFIED: $Id: 301-jffs-compression,v 1.1 2005/03/26 10:33:31 wbx Exp $
*******************************************************************************/

#ifndef PREDICTORTABLE_H
#define PREDICTORTABLE_H

//#include "DataStructures/TypeDefs.h"
//#include "DataStructures/DataTypes.h"
////#include "DataStructures/Filter.h"
////#include "DataStructures/Converter.h"
////#include "DataStructures/Manipulator.h"

#define NUMBER_OF_PREDICTORS_ARM 17

#ifndef __KERNEL__
#define NUMBER_OF_PREDICTORS_TXT 2
#else
#undef  TXT_TOKENS
#endif // __KERNEL__

#ifdef  TXT_TOKENS
#define NUMBER_OF_PREDICTORS    NUMBER_OF_PREDICTORS_TXT
#define predictortable_reset    predictortable_resetTXT
#define predictortable_update   predictortable_updateTXT
#define predictortable_minvalue predictortable_minvalueTXT
#define predictortable_maxvalue predictortable_maxvalueTXT
#else
#define NUMBER_OF_PREDICTORS    NUMBER_OF_PREDICTORS_ARM
#define predictortable_reset    predictortable_resetARM
#define predictortable_update   predictortable_updateARM
#define predictortable_minvalue predictortable_minvalueARM
#define predictortable_maxvalue predictortable_maxvalueARM
#endif


#pragma pack(4)

typedef struct
{
	PredictorType *predictors;
} PredictorTable;

#ifdef JFFS2_BBC_ARMLIB_MODELGEN
static void predictortable_clear(PredictorTable *);
static void predictortable_free(PredictorTable *);
static void predictortable_resetARM(PredictorTable *);
static void predictortable_updateARM(PredictorTable *, TokenType);
static PredictorType predictortable_minvalueARM(PredictorTable *, u32);
static PredictorType predictortable_maxvalueARM(PredictorTable *, u32);
#endif

#ifndef __KERNEL__
/*
static void predictortable_resetTXT(PredictorTable *);
static void predictortable_updateTXT(PredictorTable *, TokenType);
static PredictorType predictortable_minvalueTXT(PredictorTable *, u32);
static PredictorType predictortable_maxvalueTXT(PredictorTable *, u32);
*/
#endif // __KERNEL__

#endif

//ORIGIN: include/DataStructures/ipack_model.h

/*******************************************************************************
* FILE:     ipack_model.h
* AUTHOR:   Tamás Gergely
* MODIFIED: $Id: 301-jffs-compression,v 1.1 2005/03/26 10:33:31 wbx Exp $
*******************************************************************************/

#ifndef IPACK_MODEL_H
#define IPACK_MODEL_H

//#include "DataStructures/DataTypes.h"
//#include "DataStructures/DecisionTree.h"
//#include "DataStructures/PredictorTable.h"

#define PROBABILITY_SHIFT 12
#define PROBABILITY_MAX   0x00001000l

#define NUMBER_OF_TOKENS_ARM                  16
#define NUMBER_OF_TOKENS_PER_INSTRUCTION_ARM   8

#ifndef __KERNEL__
#define NUMBER_OF_TOKENS_TXT                 256
#define NUMBER_OF_TOKENS_PER_INSTRUCTION_TXT   4
#else
#undef TXT_TOKENS
#endif // __KERNEL__

#ifdef TXT_TOKENS
#define NUMBER_OF_TOKENS                 NUMBER_OF_TOKENS_TXT
#define NUMBER_OF_TOKENS_PER_INSTRUCTION NUMBER_OF_TOKENS_PER_INSTRUCTION_TXT
#else
#define NUMBER_OF_TOKENS                 NUMBER_OF_TOKENS_ARM
#define NUMBER_OF_TOKENS_PER_INSTRUCTION NUMBER_OF_TOKENS_PER_INSTRUCTION_ARM
#endif

#pragma pack(4)

/*
        Data structure of an internal node of the tree
*/
typedef struct
{
	PredictorType *attribute_ptr;
	u32 value;		// PredictorType
	void *right_child_ptr;
} ipack_treenodeBin;
/*
        Data structure of a leaf with probabilities
*/
typedef struct
{
	u16 probabilities[0];	// PredictorType[0]
} ipack_treeleafP;
/*
        Data structure of a leaf with class prediction
*/
typedef struct
{
	PredictorType predicted_class;	// PredictorType
} ipack_treeleafC;
/*
        Possible data structures of a tree node
*/
typedef union
{
	ipack_treenodeBin nodeBin;
	ipack_treeleafP leafP;
	ipack_treeleafC leafC;
} ipack_node_data;
/*
        Tree node
*/
typedef struct
{
	u32 type;		// u8
	ipack_node_data data;	// ipack_node_data
} ipack_nodetype;
/*
        Nullnode
*/
typedef struct
{
	u32 type;
	u16 probabilities[NUMBER_OF_TOKENS];
} ipack_nullnode;
/*
        Model for ipack project
*/
typedef struct
{
	char ID[4];		// char[4]
	char block_sign[4];	// only the first 2 are used!
	void *tree_root_ptr;	// void*
	void *tree_code;	// generated ARM code
	PredictorType *predictors_ptr;	// PredictorType*
	ipack_nullnode nullnode;
} ipack_model_type;

typedef struct
{
	u32 high;
	u32 low;
} ipack_probability_type;


static void ipack_model_get_probability_for_token(ipack_nodetype *, TokenType, ipack_probability_type *);
static TokenType ipack_model_get_token_for_range(ipack_nodetype *, u32, u32, ipack_probability_type *);
/*void      ipack_model_predictortable_reset     (PredictorType*);
void      ipack_model_predictortable_update    (PredictorType*, TokenType);*/

#ifndef __KERNEL__
/*static void ipack_model_printinfo(ipack_model_type *);
static void ipack_dumpmodel(void *);*/
#endif

#endif

//ORIGIN: include/Builders/PredictorGenerator.h

/*******************************************************************************
* FILE:     PredictorGenerator.h
* AUTHOR:   Tamás Gergely
* MODIFIED: $Id: 301-jffs-compression,v 1.1 2005/03/26 10:33:31 wbx Exp $
*******************************************************************************/

#ifndef PREDICTORGENERATOR_H
#define PREDICTORGENERATOR_H

//#include "DataStructures.h"

#ifdef JFFS2_BBC_ARMLIB_MODELGEN
static PredictorTable *predictorgenerator_generate(void);
#endif

#endif

//ORIGIN: include/Builders/Coder.h

/*******************************************************************************
* FILE:     Coder.h
* AUTHOR:   Tamás Gergely
* MODIFIED: $Id: 301-jffs-compression,v 1.1 2005/03/26 10:33:31 wbx Exp $
*******************************************************************************/

#ifndef CODER_H
#define CODER_H

#define CODER_VALUEBITS 16
#define CODER_VALUEMAX  0x00010000l
#define CODER_VALUE3RD  0x0000c000l
#define CODER_VALUEHLF  0x00008000l
#define CODER_VALUE1ST  0x00004000l

#endif

//ORIGIN: DataStructures/src/TypeDefs.c

/*******************************************************************************
* FILE:     TypeDefs.c
* AUTHOR:   Tamás Gergely
* MODIFIED: $Id: 301-jffs-compression,v 1.1 2005/03/26 10:33:31 wbx Exp $
*******************************************************************************/

//#include "ipack_common.h"
//#include "DataStructures/TypeDefs.h"
#ifndef __KERNEL__
#include <memory.h>
#endif

#define VECTOR_ALLOC_SIZE 0x00001000

static void vector_clear(vector * vct)
{
	if (vct->ptr)
		jffs2_bbc_free(vct->ptr);
	vct->capacity = 0;
	vct->size = 0;
	vct->ptr = 0;
}

#ifdef JFFS2_BBC_ARMLIB_MODELGEN
static void vector_extend(vector * vct)
{
	void *tmp;
	vct->capacity += vct->alloc_size;
	tmp = jffs2_bbc_malloc(vct->capacity);
	if (vct->ptr) {
		memcpy(tmp, vct->ptr, vct->size);
		jffs2_bbc_free(vct->ptr);
	}
	vct->ptr = tmp;
}

static void vector_reset(vector * vct)
{
	vct->capacity = 0;
	vct->size = 0;
	vct->alloc_size = VECTOR_ALLOC_SIZE;
	vct->ptr = 0;
}

static void vector_clr_ptr(vector * vct)
{
	void **it;
	void *end_it;
	for (it = vct->ptr, end_it = (((char *) (vct->ptr)) + vct->size); it != end_it; it++) {
		vector_clear(*it);
		jffs2_bbc_free(*it);
	}
	if (vct->ptr)
		jffs2_bbc_free(vct->ptr);
	vct->capacity = 0;
	vct->size = 0;
	vct->ptr = 0;
}

static void vector_add_u8(vector * vct, u8 val)
{
	if ((vct->size) + sizeof(u8) > (vct->capacity)) {
		vector_extend(vct);
	}
	*(u8 *) ((char *) (vct->ptr) + (vct->size)) = val;
	vct->size += sizeof(u8);
};

static void vector_add_u16(vector * vct, u16 val)
{
	if ((vct->size) + sizeof(u16) > (vct->capacity)) {
		vector_extend(vct);
	}
	*(u16 *) ((char *) (vct->ptr) + (vct->size)) = val;
	vct->size += sizeof(u16);
};

static void vector_add_u32(vector * vct, u32 val)
{
	if ((vct->size) + sizeof(u32) > (vct->capacity)) {
		vector_extend(vct);
	}
	*(u32 *) ((char *) (vct->ptr) + (vct->size)) = val;
	vct->size += sizeof(u32);
};

static void vector_add_s8(vector * vct, s8 val)
{
	if ((vct->size) + sizeof(s8) > (vct->capacity)) {
		vector_extend(vct);
	}
	*(s8 *) ((char *) (vct->ptr) + (vct->size)) = val;
	vct->size += sizeof(s8);
};

static void vector_add_s16(vector * vct, s16 val)
{
	if ((vct->size) + sizeof(s16) > (vct->capacity)) {
		vector_extend(vct);
	}
	*(s16 *) ((char *) (vct->ptr) + (vct->size)) = val;
	vct->size += sizeof(s16);
};

static void vector_add_s32(vector * vct, s32 val)
{
	if ((vct->size) + sizeof(s32) > (vct->capacity)) {
		vector_extend(vct);
	}
	*(s32 *) ((char *) (vct->ptr) + (vct->size)) = val;
	vct->size += sizeof(s32);
};

static void vector_add_ptr(vector * vct, void *ptr)
{
	if ((vct->size) + sizeof(void *) > (vct->capacity)) {
		vector_extend(vct);
	}
	*(void **) ((char *) (vct->ptr) + (vct->size)) = ptr;
	vct->size += sizeof(void *);
}

static void vector_concat(vector * lhs, vector * rhs)
{
	void *tmp;
	if (!(rhs->size)) {
		return;
	}
	tmp = lhs->ptr;
	lhs->capacity = (lhs->size) + (rhs->size);
	lhs->ptr = jffs2_bbc_malloc(lhs->capacity);
	if (tmp) {
		memcpy(lhs->ptr, tmp, lhs->size);
		jffs2_bbc_free(tmp);
	}
	memcpy((((u8 *) lhs->ptr) + lhs->size), rhs->ptr, rhs->size);
	lhs->size += rhs->size;
}

#endif

//ORIGIN: DataStructures/src/BitVector.c

/*******************************************************************************
* FILE:     BitVector.c
* AUTHOR:   Tamás Gergely
* MODIFIED: $Id: 301-jffs-compression,v 1.1 2005/03/26 10:33:31 wbx Exp $
*******************************************************************************/

//#include "ipack_common.h"
//#include "DataStructures/BitVector.h"
#ifndef __KERNEL__
#include <memory.h>
#endif

#define VECTOR_ALLOC_SIZE 0x00001000

#ifdef JFFS2_BBC_ARMLIB_MODELGEN

static void bitblocks_clear(BitBlocks * this)
{
	BitVector **it;
	void *end_it;
	for (it = this->ptr, end_it = VECTOR_P_END(this); it != end_it; it++) {
		bitvector_clear(*it);
		jffs2_bbc_free(*it);
	}
	jffs2_bbc_free(this->ptr);
	this->ptr = 0;
}

static void bitvector_clear(BitVector * this)
{
	if (this->base) {
		jffs2_bbc_free(this->base);
	}
	this->freebits = 0;
	this->capacity = 0;
	this->size = 0;
	this->base = 0;
	this->ptr = 0;
}

static void bitvector_W_reset(BitVector * this)
{
	this->freebits = 0;
	this->capacity = 0;
	this->size = 0;
	this->base = 0;
	this->ptr = 0;
}

static void bitvector_W_add0(BitVector * this)
{
	if (!(this->freebits)) {
		if (this->size == this->capacity) {
			void *tmp = this->base;
			this->capacity += VECTOR_ALLOC_SIZE;
			this->base = jffs2_bbc_malloc(this->capacity);
			this->ptr = ((u8 *) (this->base)) + this->size;
			memcpy(this->base, tmp, this->size);
			jffs2_bbc_free(tmp);
		}
		else {
			this->ptr++;
		}
		this->size++;
		this->freebits = 7;
		*(this->ptr) = 0x00;
	}
	else {
		this->freebits--;
		(*(this->ptr)) <<= 1;
	}
}

static void bitvector_W_add1(BitVector * this)
{
	if (!(this->freebits)) {
		if (this->size == this->capacity) {
			void *tmp = this->base;
			this->capacity += VECTOR_ALLOC_SIZE;
			this->base = jffs2_bbc_malloc(this->capacity);
			this->ptr = ((u8 *) (this->base)) + this->size;
			memcpy(this->base, tmp, this->size);
			jffs2_bbc_free(tmp);
		}
		else {
			this->ptr++;
		}
		this->size++;
		this->freebits = 7;
		*(this->ptr) = 0x01;
	}
	else {
		this->freebits--;
		(*(this->ptr)) <<= 1;
		(*(this->ptr)) |= 0x01;
	}
}

static void bitvector_W_concat_b(BitVector * lhs, BitVector * rhs)
{
	void *tmp;
	if (!(rhs->size)) {
		return;
	}
	tmp = lhs->base;
	lhs->capacity = ((((lhs->size) + (rhs->size) - 1) / VECTOR_ALLOC_SIZE) + 1) * VECTOR_ALLOC_SIZE;
	lhs->base = jffs2_bbc_malloc(lhs->capacity);
	if (tmp) {
		memcpy(lhs->base, tmp, lhs->size);
		jffs2_bbc_free(tmp);
	}
	memcpy((((u8 *) (lhs->base)) + lhs->size), rhs->base, rhs->size);
	lhs->freebits = 0;
	lhs->size += rhs->size;
	lhs->ptr = ((u8 *) (lhs->base)) + lhs->size;
}

static void bitvector_W_concat_v(BitVector * lhs, vector * rhs)
{
	void *tmp;
	if (!(rhs->size)) {
		return;
	}
	tmp = lhs->base;
	lhs->capacity = ((((lhs->size) + (rhs->size) - 1) / VECTOR_ALLOC_SIZE) + 1) * VECTOR_ALLOC_SIZE;
	lhs->base = jffs2_bbc_malloc(lhs->capacity);
	if (tmp) {
		memcpy(lhs->base, tmp, lhs->size);
		jffs2_bbc_free(tmp);
	}
	memcpy((((u8 *) (lhs->base)) + lhs->size), rhs->ptr, rhs->size);
	lhs->freebits = 0;
	lhs->size += rhs->size;
	lhs->ptr = ((u8 *) (lhs->base)) + lhs->size;
}

static void bitvector_W_flush(BitVector * this)
{
	(*(this->ptr)) <<= this->freebits;
	this->freebits = 0;
}

static void bitvector_R_reset(BitVector * this)
{
	this->freebits = 7;
	this->ptr = this->base;
}

static u8 bitvector_R_get1(BitVector * this)
{
	u8 tmp = ((*(this->ptr)) >> this->freebits) & 0x01;
	if (!(this->freebits)) {
		this->freebits = 7;
		this->ptr++;
	}
	else {
		this->freebits--;
	}
	return tmp;
}

static u8 bitvector_R_get8(BitVector * this)
{
	u8 tmp = (*(this->ptr));
	this->ptr++;
	return tmp;
}

#endif

//ORIGIN: DataStructures/src/DecisionTree.c

/*******************************************************************************
* FILE:     DecisionTree.c
* AUTHOR:   Tamás Gergely
* MODIFIED: $Id: 301-jffs-compression,v 1.1 2005/03/26 10:33:31 wbx Exp $
*******************************************************************************/

//#include "ipack_common.h"
//#include "DataStructures/DecisionTree.h"

static void decisiontree_delete_node(void *root)
{
	u8 tmp = GET_NODE_PTR_TYPE(root);
	if (TREENODETYPE_IS_NODE_BINARY(tmp)) {
		decisiontree_delete_node(((TreeNodeBinary *) root)->left);
		decisiontree_delete_node(((TreeNodeBinary *) root)->right);
	}
	else if ((tmp) == TREENODETYPE_LEAF_P) {
		if (((TreeLeafP *) root)->probabilities) {
			jffs2_bbc_free(((TreeLeafP *) root)->probabilities);
		}
	}
	else if ((tmp) == TREENODETYPE_LEAF_C) {
	}
	jffs2_bbc_free(root);
}

#ifdef JFFS2_BBC_ARMLIB_MODELGEN

static void decisiontree_delete(DecisionTree * dt)
{
	decisiontree_delete_node(dt->root);
	jffs2_bbc_free(dt->predictor_max_values);
}

static void decisiontree_get_probability_for_token(void *root, PredictorType * preds, TokenType token, ProbabilityType * prob)
{
	void *tmp = root;
	while (TREENODETYPE_IS_NODE(((TreeNodeBinary *) tmp)->type)) {
		if (((TreeNodeBinary *) tmp)->type == TREENODETYPE_NODE_BINARY_EQ) {
			if (preds[((TreeNodeBinary *) tmp)->attribute] TREE_SUBTREE_RELATION_LEFT_EQ((TreeNodeBinary *) tmp)->value) {
				tmp = ((TreeNodeBinary *) tmp)->left;
			}
			else {
				tmp = ((TreeNodeBinary *) tmp)->right;
			}
		}
		else if (((TreeNodeBinary *) tmp)->type == TREENODETYPE_NODE_BINARY_LT) {
			if (preds[((TreeNodeBinary *) tmp)->attribute] TREE_SUBTREE_RELATION_LEFT_LT((TreeNodeBinary *) tmp)->value) {
				tmp = ((TreeNodeBinary *) tmp)->left;
			}
			else {
				tmp = ((TreeNodeBinary *) tmp)->right;
			}
		}
	}
	prob->high = 0;
	prob->low = 0;
	prob->max = 0;
	if (((TreeNodeBinary *) tmp)->type == TREENODETYPE_LEAF_P) {
		u32 i;
		u32 lngth = ((TreeLeafP *) tmp)->pairs << 1;
		for (i = 0; i < lngth;) {
			TokenType at = ((TreeLeafP *) tmp)->probabilities[i++];
			TokenType av = ((TreeLeafP *) tmp)->probabilities[i++];
			if (token > at)
				prob->low += av;
			if (token >= at)
				prob->high += av;
			prob->max += av;
		}
	}
	else if (((TreeNodeBinary *) tmp)->type == TREENODETYPE_LEAF_C) {
		if (((TreeLeafC *) tmp)->predicted_class == token) {
			prob->high = TOKEN_MAXVALUE;
			prob->max = TOKEN_MAXVALUE;
		}
	}
}

static TokenType decisiontree_get_token_for_range(void *root, PredictorType * preds, u32 value, u32 range, ProbabilityType * prob)
{
	void *tmp = root;
	TokenType token = 0;
	while (TREENODETYPE_IS_NODE(((TreeNodeBinary *) tmp)->type)) {
		if (((TreeNodeBinary *) tmp)->type == TREENODETYPE_NODE_BINARY_EQ) {
			if (preds[((TreeNodeBinary *) tmp)->attribute] TREE_SUBTREE_RELATION_LEFT_EQ((TreeNodeBinary *) tmp)->value) {
				tmp = ((TreeNodeBinary *) tmp)->left;
			}
			else {
				tmp = ((TreeNodeBinary *) tmp)->right;
			}
		}
		else if (((TreeNodeBinary *) tmp)->type == TREENODETYPE_NODE_BINARY_LT) {
			if (preds[((TreeNodeBinary *) tmp)->attribute] TREE_SUBTREE_RELATION_LEFT_LT((TreeNodeBinary *) tmp)->value) {
				tmp = ((TreeNodeBinary *) tmp)->left;
			}
			else {
				tmp = ((TreeNodeBinary *) tmp)->right;
			}
		}
	}
	prob->high = 0;
	prob->low = 0;
	prob->max = 0;
	if (((TreeNodeBinary *) tmp)->type == TREENODETYPE_LEAF_P) {
		u32 i;
		u32 norm;
		TokenType at = 0;
		TokenType av;
		u32 lngth = ((TreeLeafP *) tmp)->pairs << 1;
		for (i = 0; i < lngth;) {
			i++;
			prob->max += ((TreeLeafP *) tmp)->probabilities[i++];
		}
		norm = (value * prob->max - 1) / range;
		for (i = 0; prob->high <= norm;) {
			at = ((TreeLeafP *) tmp)->probabilities[i++];
			av = ((TreeLeafP *) tmp)->probabilities[i++];
			prob->high += av;
			if (prob->high <= norm)
				prob->low += av;
		}
		token = at;
	}
	else if (((TreeNodeBinary *) tmp)->type == TREENODETYPE_LEAF_C) {
		token = ((TreeLeafC *) tmp)->predicted_class;
		prob->high = TOKEN_MAXVALUE;
		prob->max = TOKEN_MAXVALUE;
	}
	return token;
}
#endif

//ORIGIN: DataStructures/src/PredictorTable.c

/*******************************************************************************
* FILE:     PredictorTable.c
* AUTHOR:   Tamás Gergely
* MODIFIED: $Id: 301-jffs-compression,v 1.1 2005/03/26 10:33:31 wbx Exp $
*******************************************************************************/

//#include "ipack_common.h"
//#include "DataStructures/PredictorTable.h"

#ifdef JFFS2_BBC_ARMLIB_MODELGEN

static void predictortable_clear(PredictorTable * table)
{
	table->predictors = 0;
}

static void predictortable_free(PredictorTable * table)
{
	if (table->predictors) {
		jffs2_bbc_free(table->predictors);
		table->predictors = 0;
	}
}

static void predictortable_resetARM(PredictorTable * table)
{
	register PredictorType *ptr = table->predictors;
	register PredictorType *end = ptr + NUMBER_OF_PREDICTORS_ARM;
	while (ptr < end) {
		*(ptr++) = 0;
	}
}

static void predictortable_updateARM(PredictorTable * table, TokenType token)
{
	register PredictorType *ptr = table->predictors;
	register u32 ndx = ptr[0] + 1;
	ptr[ndx + 8] = ptr[ndx];
	ptr[ndx] = token;
	if (ndx == 8) {
		ptr[0] = 0;
	}
	else {
		++ptr[0];
	}
}

static PredictorType predictortable_minvalueARM(PredictorTable * table, u32 index)
{
	return 0;
}

static PredictorType predictortable_maxvalueARM(PredictorTable * table, u32 index)
{
	if (index == 0) {
		return 7;
	}
	else {
		return 15;
	}
}

#endif

#ifndef __KERNEL__

/*static void predictortable_resetTXT(PredictorTable * table)
{
	register PredictorType *ptr = table->predictors;
	register PredictorType *end = ptr + NUMBER_OF_PREDICTORS_TXT;
	while (ptr < end) {
		*(ptr++) = 0;
	}
}

static void predictortable_updateTXT(PredictorTable * table, TokenType token)
{				//TODO: modify
	register PredictorType *ptr = table->predictors;
//        register u32            ndx;
	ptr[0] = token;
	if ((('a' <= token) && (token <= 'z')) || (('A' <= token) && (token <= 'Z'))) {
		++(ptr[1]);
	}
	else {
		ptr[1] = 0;
	}
}

static PredictorType predictortable_minvalueTXT(PredictorTable * table, u32 index)
{
	return 0;
}

static PredictorType predictortable_maxvalueTXT(PredictorTable * table, u32 index)
{				//TODO: modify
	return 254;
}*/

#endif // __KERNEL__

//ORIGIN: DataStructures/src/ipack_model.c

/*******************************************************************************
* FILE:     ipack_model.c
* AUTHOR:   Tamás Gergely
* MODIFIED: $Id: 301-jffs-compression,v 1.1 2005/03/26 10:33:31 wbx Exp $
*******************************************************************************/

//#include "DataStructures/ipack_model.h"
//#include "measuredef.h"
//#include "ipack_common.h"

#ifdef __MEASURE_TIME_MODEL_GETPROB
#define __MT_P_MAX 256
#define __MT_P_DIV 128
#define __MT_P_MIN 0
#endif

static void ipack_model_get_probability_for_token(ipack_nodetype * tmp, TokenType token, ipack_probability_type * prob)
{
//        register ipack_nodetype* tmp = model->tree_root_ptr;
//        register ipack_nodetype* tmp = root;
	while (TREENODETYPE_IS_NODE(tmp->type)) {
		if (tmp->type == TREENODETYPE_NODE_BINARY_EQ) {
			if (*(tmp->data.nodeBin.attribute_ptr) TREE_SUBTREE_RELATION_LEFT_EQ tmp->data.nodeBin.value) {
				((char *) tmp) += sizeof(tmp->type) + sizeof(ipack_treenodeBin);
			}
			else {
				tmp = tmp->data.nodeBin.right_child_ptr;
			}
		}
		else if (tmp->type == TREENODETYPE_NODE_BINARY_LT) {
			if (*(tmp->data.nodeBin.attribute_ptr) TREE_SUBTREE_RELATION_LEFT_LT tmp->data.nodeBin.value) {
				((char *) tmp) += sizeof(tmp->type) + sizeof(ipack_treenodeBin);
			}
			else {
				tmp = tmp->data.nodeBin.right_child_ptr;
			}
		}
	}
	prob->high = 0;
	prob->low = 0;
//        prob->max  = 0;
	if (tmp->type == TREENODETYPE_LEAF_P) {
		if (token) {
			prob->low = tmp->data.leafP.probabilities[token - 1];
		}
		prob->high = tmp->data.leafP.probabilities[token];
//                prob->max  = tmp->data.leafP.probabilities[15];
	}
	else if (tmp->type == TREENODETYPE_LEAF_C) {
		if (tmp->data.leafC.predicted_class == token) {
			prob->high = TOKEN_MAXVALUE;
//                        prob->max  = TOKEN_MAXVALUE;
		}
	}
}

#ifndef IPACK_ARM_ASM

//return ipack_model_get_token_for_range2(tmp,value,range,prob);

static TokenType ipack_model_get_token_for_range(ipack_nodetype * tmp, u32 value, u32 range, ipack_probability_type * prob)
{
//        register ipack_nodetype* tmp   = model->tree_root_ptr;
//        register ipack_nodetype* tmp   = root;
	register TokenType token = 0;
	while (TREENODETYPE_IS_NODE(tmp->type)) {
		if (tmp->type == TREENODETYPE_NODE_BINARY_EQ) {
			if (*(tmp->data.nodeBin.attribute_ptr) TREE_SUBTREE_RELATION_LEFT_EQ tmp->data.nodeBin.value) {
				((char *) tmp) += sizeof(tmp->type) + sizeof(ipack_treenodeBin);
			}
			else {
				tmp = tmp->data.nodeBin.right_child_ptr;
			}
		}
		else if (tmp->type == TREENODETYPE_NODE_BINARY_LT) {
			if (*(tmp->data.nodeBin.attribute_ptr) TREE_SUBTREE_RELATION_LEFT_LT tmp->data.nodeBin.value) {
				((char *) tmp) += sizeof(tmp->type) + sizeof(ipack_treenodeBin);
			}
			else {
				tmp = tmp->data.nodeBin.right_child_ptr;
			}
		}
	}
	prob->high = 0;
	prob->low = 0;
//        prob->max  = 0;
	if (tmp->type == TREENODETYPE_LEAF_P) {
		u32 i;
		u32 norm;
//                prob->max = tmp->data.leafP.probabilities[15];
/*                norm = (value * prob->max -1)/range;
                for(i = 0; i < 15; ++i) {
                        if(tmp->data.leafP.probabilities[i] > norm) {
                                break;
                        }
                }*/
		norm = ((value << PROBABILITY_SHIFT) - 1);
		for (i = 0; i < NUMBER_OF_TOKENS; ++i) {
			if (range * tmp->data.leafP.probabilities[i] > norm) {
				break;
			}
		}
		token = (TokenType) i;
		prob->high = tmp->data.leafP.probabilities[i];
		if (token) {
			prob->low = tmp->data.leafP.probabilities[token - 1];
		}
	}
	else if (tmp->type == TREENODETYPE_LEAF_C) {
		token = tmp->data.leafC.predicted_class;
		prob->high = PROBABILITY_MAX;
//                prob->max  = PROBABILITY_MAX;
	}
	return token;
}
#endif
/*
void ipack_model_predictortable_reset(PredictorType* ptr)
{
//        register PredictorType* ptr = model->predictors_ptr;
//        register PredictorType* ptr = preds;
        register PredictorType* end = ptr + NUMBER_OF_PREDICTORS;
        while(ptr < end) {
                *(ptr++) = 0;
        }
}

void ipack_model_predictortable_update(PredictorType* ptr, TokenType token)
{
//        register PredictorType* ptr = model->predictors_ptr;
//        register PredictorType* ptr = preds;
        register u32            ndx = ptr[0] + 1;
        ptr[ndx + 8] = ptr[ndx];
        ptr[ndx]     = token;
        if(ndx == 8) {
                ptr[0] = 0;
        } else {
                ++ ptr[0];
        }
}*/
/****************************************************************************/

#ifndef __KERNEL__
static void ipack_model_countpreds(void *ptr, ipack_nodetype * node, double *table, double val)
{
	if ((node->type == TREENODETYPE_NODE_BINARY_EQ) || (node->type == TREENODETYPE_NODE_BINARY_LT)) {
		table[(u32) (node->data.nodeBin.attribute_ptr) - (u32) (ptr)] += val;
		ipack_model_countpreds(ptr, (void *) (((u8 *) (node)) + sizeof(node->type) + sizeof(ipack_treenodeBin)), table, val / 2);
		ipack_model_countpreds(ptr, node->data.nodeBin.right_child_ptr, table, val / 2);
	}
	else {
	}
}

/*static void ipack_model_printinfo(ipack_model_type * model)
{
	double *prcnt = jffs2_bbc_malloc(sizeof(double) * NUMBER_OF_PREDICTORS);
	u32 i;
	for (i = 0; i < NUMBER_OF_PREDICTORS; i++) {
		prcnt[i] = 0.0;
	}
	ipack_model_countpreds(model->predictors_ptr, model->tree_root_ptr, prcnt, 100);
	for (i = 0; i < NUMBER_OF_PREDICTORS; i++) {
		jffs2_bbc_print3("        p[%3d] = %10.6lf\n", (int) i, prcnt[i]);
	}
	jffs2_bbc_free(prcnt);
}*/

static void ipack_dumpnode(unsigned char **ptr, FILE * file, char *prefs)
{
	switch (*((*ptr)++)) {
		u32 i;
		u32 j;
		u32 x;
		u32 y;
	case TREENODETYPE_NODE_BINARY_EQ:
		x = *((*ptr)++);
		y = *((*ptr)++);
		fprintf(file, "%s+->\tBinary node: P[%u] equals %u\n", prefs, (unsigned int)x, (unsigned int)y);
		for (j = 0; j < 4096 && prefs[j]; ++j);
		prefs[j] = '\t';
		prefs[++j] = '|';
		ipack_dumpnode(ptr, file, prefs);
		prefs[j--] = 0;
		ipack_dumpnode(ptr, file, prefs);
		prefs[j] = 0;
		break;
	case TREENODETYPE_NODE_BINARY_LT:
		x = *((*ptr)++);
		y = *((*ptr)++);
		fprintf(file, "%s+->\tBinary node: P[%u] greater than %u\n", prefs, (unsigned int)x, (unsigned int)y);
		for (j = 0; j < 4096 && prefs[j]; ++j);
		prefs[j] = '\t';
		prefs[++j] = '|';
		ipack_dumpnode(ptr, file, prefs);
		prefs[j--] = 0;
		ipack_dumpnode(ptr, file, prefs);
		prefs[j] = 0;
		break;
	case TREENODETYPE_LEAF_P:
		x = *((*ptr)++);
		fprintf(file, "%s+->\tLeaf: %u pairs\n", prefs, (unsigned int)x);
		(*ptr) += (x << 1);
		break;
	case TREENODETYPE_LEAF_C:
		x = *((*ptr)++);
		fprintf(file, "%s+->\tLeaf: class %u\n", prefs, (unsigned int)x);
		break;
	default:
		fprintf(file, "%s+->\tLeaf: nullnode\n", prefs);
	}
}

/*static void ipack_dumpmodel(void *model)
{
	unsigned char *tmp_ptr = model;
	FILE *file;
	char C[4096];
	if ((file = fopen("DUMPED_MODEL", "wa"))) {
		int i;
		for (i = 0; i < 4096; C[i++] = 0);
		tmp_ptr += 8;
		tmp_ptr += sizeof(u32);
		ipack_dumpnode(&tmp_ptr, file, C);
		fclose(file);
	}
}*/

#endif

//ORIGIN: Builders/src/PredictorGenerator.c

/*******************************************************************************
* FILE:     PredictorGenerator.c
* AUTHOR:   Tamás Gergely
* MODIFIED: $Id: 301-jffs-compression,v 1.1 2005/03/26 10:33:31 wbx Exp $
*******************************************************************************/

//#include "ipack_common.h"
//#include "Builders/PredictorGenerator.h"

#ifdef JFFS2_BBC_ARMLIB_MODELGEN
static PredictorTable *predictorgenerator_generate( /*PredictorGeneratorSettings* settings */ )
{
	PredictorTable *ptr = jffs2_bbc_malloc(sizeof(PredictorTable));
	predictortable_clear(ptr);
	ptr->predictors = jffs2_bbc_malloc(NUMBER_OF_PREDICTORS * sizeof(PredictorType));
	return ptr;
}
#endif

//ORIGIN: Builders/src/ipack_armlib_compressor.c

/*******************************************************************************
* FILE:     ipack_armlim_compressor.c
* AUTHOR:   Tamás Gergely
* MODIFIED: $Id: 301-jffs-compression,v 1.1 2005/03/26 10:33:31 wbx Exp $
*******************************************************************************/

//#include "ipack_common.h"
//#include "DataStructures.h"
//#include "Builders/PredictorGenerator.h"
//#include "Builders/Tokenizer.h"
//#include "Builders/Coder.h"

#define EC_NO_ERROR                      0
#define EC_NOT_IPMF_FILE                -1
#define EC_NOT_IPMF_MODEL               -2
#define EC_NOT_HG_BLOCK                 -3
#define EC_WRONG_INPUT_LENGTH         -501
#define EC_CODER_WRONG_PROBABILITY       1
#define EC_CODER_WRONG_RANGE             2
#define EC_BUFFER_OVERFLOW             501
#define EC_BUFFER_UNDERFLOW            502
#define EC_UNKNOWN_TOKEN_TYPE         1001
#define EC_UNKNOWN_FILTER             1002
#define EC_UNKNOWN_CONVERTER          1003
#define EC_UNKNOWN_MANIPULATOR        1004

/*******************************************************************************

        COMPRESSOR INIT FUNCTIONS

*******************************************************************************/

#define ROUND_UP_TO_DWORD(val) ( ( (val) + 3 ) & 0xfffffffc )

#ifndef __KERNEL__
int ipack_glb_endian_X;
#endif

static int ipack_compressor_init_tree(unsigned char **ptr, ipack_model_type * model, ipack_nodetype * node, void *nullnode)
{
	int retval = 0;
	node->type = *((*ptr)++);
	switch (node->type) {
		u32 i;
		u32 j;
		u32 lngth;
		u32 tmpret;
		TokenType at;
		u16 av;
	case TREENODETYPE_NODE_BINARY_EQ:
	case TREENODETYPE_NODE_BINARY_LT:
		node->data.nodeBin.attribute_ptr = (model->predictors_ptr) + (*((*ptr)++));
		node->data.nodeBin.value = *((*ptr)++);
		retval = sizeof(node->data.nodeBin);
		retval += ipack_compressor_init_tree(ptr, model, (void *) ROUND_UP_TO_DWORD(((u32) node) + sizeof(node->type) + sizeof(node->data.nodeBin)), nullnode);
		node->data.nodeBin.right_child_ptr = (void *) ROUND_UP_TO_DWORD(((u32) node) + retval + sizeof(node->type));
		retval += ipack_compressor_init_tree(ptr, model, node->data.nodeBin.right_child_ptr, nullnode);
		break;
	case TREENODETYPE_LEAF_P:
		lngth = *((*ptr)++);
		av = 0;
		for (i = 0, j = 0; i < lngth; ++i) {
			at = *((*ptr)++);
			while (j < at) {
				node->data.leafP.probabilities[j++] = av;
			}
			av += *((*ptr)++);
		}
		while (j < NUMBER_OF_TOKENS) {
			node->data.leafP.probabilities[j++] = av;
		}
		for (i = 0; i < NUMBER_OF_TOKENS; ++i) {
			node->data.leafP.probabilities[i] = ((node->data.leafP.probabilities[i] << PROBABILITY_SHIFT) / node->data.leafP.probabilities[NUMBER_OF_TOKENS - 1]);
		}
		retval = ROUND_UP_TO_DWORD(NUMBER_OF_TOKENS * sizeof(u16));
		break;
	case TREENODETYPE_LEAF_C:
		node->data.leafC.predicted_class = *((*ptr)++);
		retval = sizeof(node->data.leafC);
		retval = ROUND_UP_TO_DWORD(retval);
		break;
	default:
		return 0;
	}
	return retval + sizeof(node->type);
}

#define IPACK_TREE_CONVERT_REPLACE 0
#define IPACK_TREE_CONVERT_KEEP    1

static void *ipack_tree_to_code(ipack_model_type * model, int *code_size);

static int ipack_armlib_convert_tree_to_code(ipack_model_type * model_img, int mode)
{
#ifdef IPACK_TREE_TO_CODE
	int tree_size;

	model_img->tree_code = ipack_tree_to_code(model_img, &tree_size);
	jffs2_bbc_print2("Convertation done. Code size=%d\n", tree_size);
	if (mode == IPACK_TREE_CONVERT_REPLACE) {
		jffs2_bbc_print1("Freeing original tree.\n");
		jffs2_bbc_free(model_img->tree_root_ptr);
		model_img->tree_root_ptr = NULL;
	}
#endif	
	return 0;
}


static int ipack_armlib_compressor_init(void **model)
{
	int retval = EC_NO_ERROR;
	unsigned char *tmp_ptr = *model;
	u32 i;
	ipack_model_type *model_img;
	char tmp_c[2];

	if (*(tmp_ptr++) != 'i') {
		return EC_NOT_IPMF_FILE;
	}
	else if (*(tmp_ptr++) != 'P') {
		return EC_NOT_IPMF_FILE;
	}
	else if (*(tmp_ptr++) != 'M') {
		return EC_NOT_IPMF_FILE;
	}
	else if (*(tmp_ptr++) != 'F') {
		return EC_NOT_IPMF_FILE;
	}
	tmp_c[0] = *(tmp_ptr++);
	tmp_c[1] = *(tmp_ptr++);
	tmp_ptr += 2;

	//model_img = jffs2_bbc_malloc(*((u32*)tmp_ptr));
	model_img = jffs2_bbc_malloc(sizeof(ipack_model_type) + ROUND_UP_TO_DWORD(NUMBER_OF_PREDICTORS));
	model_img->tree_root_ptr = jffs2_bbc_malloc(*((u32 *) tmp_ptr));	//it is smaller a little but, but...

	tmp_ptr += sizeof(u32);

	model_img->ID[0] = 'i';
	model_img->ID[1] = 'P';
	model_img->ID[2] = 'M';
	model_img->ID[3] = 'F';

	model_img->block_sign[0] = tmp_c[0];
	model_img->block_sign[1] = tmp_c[1];

	model_img->nullnode.type = TREENODETYPE_LEAF_P;
	for (i = 0; i < NUMBER_OF_TOKENS; ++i) {
		model_img->nullnode.probabilities[i] = 0;
	}
	model_img->predictors_ptr = (void *) (((u32) model_img) + sizeof(ipack_model_type));
	//model_img->tree_root_ptr  = (void*)ROUND_UP_TO_DWORD(((u32)(model_img->predictors_ptr)) + NUMBER_OF_PREDICTORS);//ALIGN

	ipack_compressor_init_tree(&tmp_ptr, model_img, model_img->tree_root_ptr, &(model_img->nullnode));

#ifdef IPACK_TREE_TO_CODE
#ifdef IPACK_AUTO_TREE_TO_CODE
	jffs2_bbc_print1("Automatically converting tree to ARM code...\n");
	ipack_armlib_convert_tree_to_code(model_img, IPACK_TREE_CONVERT_REPLACE);
#else
	model_img->tree_code = NULL;
#endif
#else
	model_img->tree_code = NULL;
#endif

	jffs2_bbc_free(*model);
	*model = model_img;
	return retval;
}

/*******************************************************************************

        COMPRESSOR DEINIT FUNCTIONS

*******************************************************************************/


/* Descructor of compressor (model will be freed with jffs2_bbc_free() after it)*/
static void ipack_armlib_compressor_deinit(void)
{
}

/*******************************************************************************

        COMPRESS FUNCTIONS

*******************************************************************************/

static int writebits0(unsigned char **dest, u8 * freebits, u32 * opposite, unsigned char *end)
{
	if (!(*freebits)) {
		++(*dest);
		*freebits = 7;
		**dest = 0x00;
	}
	else {
		--(*freebits);
		(**dest) <<= 1;
	}
	if ((*dest == end) && !(*freebits)) {
		return EC_BUFFER_OVERFLOW;
	}
	while (*opposite) {
		--(*opposite);
		if (!(*freebits)) {
			++(*dest);
			*freebits = 7;
			**dest = 0x01;
		}
		else {
			--(*freebits);
			(**dest) <<= 1;
			(**dest) |= 0x01;
		}
		if ((*dest == end) && !(*freebits)) {
			return EC_BUFFER_OVERFLOW;
		}
	}
	return 0;
}

static int writebits1(unsigned char **dest, u8 * freebits, u32 * opposite, unsigned char *end)
{
	if (!(*freebits)) {
		++(*dest);
		*freebits = 7;
		**dest = 0x01;
	}
	else {
		--(*freebits);
		(**dest) <<= 1;
		(**dest) |= 0x01;
	}
	if ((*dest == end) && !(*freebits)) {
		return EC_BUFFER_OVERFLOW;
	}
	while (*opposite) {
		--(*opposite);
		if (!(*freebits)) {
			++(*dest);
			*freebits = 7;
			**dest = 0x00;
		}
		else {
			--(*freebits);
			(**dest) <<= 1;
		}
		if ((*dest == end) && !(*freebits)) {
			return EC_BUFFER_OVERFLOW;
		}
	}
	return 0;
}




/* Compress block
 *   *dstlen bytes are allocated.
 *   if it is not enough write *sourcelen over to the processed amount of data
 *   returns non zero if fails
 */
static int ipack_armlib_compress(void *model, unsigned char *input, unsigned char *output, unsigned long *sourcelen, unsigned long *dstlen)
{
	register u32 coder_high = CODER_VALUEMAX - 1;
	register u32 coder_low = 0;
	u32 coder_opbits = 0;
	u8 bitvector_freebits = 8;
	unsigned char *bitvector_ptr = output;
	unsigned char *bitvector_end = output + (*dstlen - 1);
	ARM_DataType *tmpp;
	TokenStream tmpv;
	TokenType *it;
	void *end_it;

	ipack_nodetype *treeroot = ((ipack_model_type *) model)->tree_root_ptr;
	PredictorType *predctrs = ((ipack_model_type *) model)->predictors_ptr;

#ifdef IPACK_TREE_TO_CODE
	void (*treefunc) (ipack_nodetype *, TokenType, ipack_probability_type *);

	treefunc = ((ipack_model_type *) model)->tree_code;
	if (treefunc != NULL)
		treefunc += 4;
#endif

	if ((*sourcelen % 4) != 0) {
		return EC_WRONG_INPUT_LENGTH;
	}
	if (*dstlen <= 4) {
		return EC_BUFFER_OVERFLOW;
	}

	if (((ipack_model_type *) model)->ID[0] != 'i') {
		return EC_NOT_IPMF_MODEL;
	}
	else if (((ipack_model_type *) model)->ID[1] != 'P') {
		return EC_NOT_IPMF_MODEL;
	}
	else if (((ipack_model_type *) model)->ID[2] != 'M') {
		return EC_NOT_IPMF_MODEL;
	}
	else if (((ipack_model_type *) model)->ID[3] != 'F') {
		return EC_NOT_IPMF_MODEL;
	}
#ifdef TXT_TOKENS
	tmpv.capacity = (*sourcelen);
#else
	tmpv.capacity = (*sourcelen) << 1;
#endif
	tmpv.size = tmpv.capacity;
	tmpv.ptr = jffs2_bbc_malloc(tmpv.size);
	it = tmpv.ptr;

#ifndef __KERNEL__
	if (ipack_glb_endian_X) {
		for (tmpp = (void *) input; (u32) tmpp < (u32) (input + *sourcelen); ++tmpp) {
#ifdef TXT_TOKENS
			*(it++) = (u8) ((*tmpp & 0xff000000) >> 24);
			*(it++) = (u8) ((*tmpp & 0x00ff0000) >> 16);
			*(it++) = (u8) ((*tmpp & 0x0000ff00) >> 8);
			*(it++) = (u8) ((*tmpp & 0x000000ff));
#else
			*(it++) = (u8) ((*tmpp & 0x0000f000) >> 12);
			*(it++) = (u8) ((*tmpp & 0x0000000f));
			*(it++) = (u8) ((*tmpp & 0xf0000000) >> 28);
			*(it++) = (u8) ((*tmpp & 0x000f0000) >> 16);
			*(it++) = (u8) ((*tmpp & 0x00000f00) >> 8);
			*(it++) = (u8) ((*tmpp & 0x00f00000) >> 20);
			*(it++) = (u8) ((*tmpp & 0x0f000000) >> 24);
			*(it++) = (u8) ((*tmpp & 0x000000f0) >> 4);
#endif //TXT_TOKENS
		}
	}
	else {
#endif
		for (tmpp = (void *) input; (u32) tmpp < (u32) (input + *sourcelen); ++tmpp) {
#ifdef TXT_TOKENS
			*(it++) = (u8) ((*tmpp & 0x000000ff));
			*(it++) = (u8) ((*tmpp & 0x0000ff00) >> 8);
			*(it++) = (u8) ((*tmpp & 0x00ff0000) >> 16);
			*(it++) = (u8) ((*tmpp & 0xff000000) >> 24);
#else
			*(it++) = (u8) ((*tmpp & 0x00f00000) >> 20);
			*(it++) = (u8) ((*tmpp & 0x0f000000) >> 24);
			*(it++) = (u8) ((*tmpp & 0x000000f0) >> 4);
			*(it++) = (u8) ((*tmpp & 0x00000f00) >> 8);
			*(it++) = (u8) ((*tmpp & 0x000f0000) >> 16);
			*(it++) = (u8) ((*tmpp & 0x0000f000) >> 12);
			*(it++) = (u8) ((*tmpp & 0x0000000f));
			*(it++) = (u8) ((*tmpp & 0xf0000000) >> 28);
#endif //TXT_TOKENS
		}
#ifndef __KERNEL__
	}
#endif
/*
        ENCODE
*/
	{			//predictor reset
		register PredictorType *ptr = predctrs;
		register PredictorType *end = ptr + NUMBER_OF_PREDICTORS;
		while (ptr < end) {
			*(ptr++) = 0;
		}
	}

	//*(bitvector_ptr++) = 'H';
	//*(bitvector_ptr++) = 'G';
	*(bitvector_ptr++) = ((ipack_model_type *) model)->block_sign[0];
	*(bitvector_ptr++) = ((ipack_model_type *) model)->block_sign[1];

	*(bitvector_ptr++) = (unsigned char) (((*sourcelen) >> 8) & 0xff);
	*(bitvector_ptr++) = (unsigned char) ((*sourcelen) & 0xff);
	for (it = tmpv.ptr, end_it = VECTOR_S_END(tmpv); it != end_it; ++it) {
		ipack_probability_type prob;
		u32 range;

#ifdef IPACK_TREE_TO_CODE
		if (treefunc != NULL)
			(*treefunc) (treeroot, *it, &prob);
		else
			ipack_model_get_probability_for_token(treeroot, *it, &prob);
#else
		ipack_model_get_probability_for_token(treeroot, *it, &prob);
#endif

		if (prob.high == prob.low) {
			vector_clear(&tmpv);
			return EC_CODER_WRONG_PROBABILITY;
		}
		range = coder_high - coder_low + 1;
		coder_high = coder_low + ((range * prob.high) >> PROBABILITY_SHIFT) - 1;
		coder_low += ((range * prob.low) >> PROBABILITY_SHIFT);
		for (;;) {
			if (coder_high < CODER_VALUEHLF) {
				if (writebits0(&bitvector_ptr, &bitvector_freebits, &coder_opbits, bitvector_end)) {
					vector_clear(&tmpv);
					return EC_BUFFER_OVERFLOW;
				}
			}
			else if (coder_low >= CODER_VALUEHLF) {
				if (writebits1(&bitvector_ptr, &bitvector_freebits, &coder_opbits, bitvector_end)) {
					vector_clear(&tmpv);
					return EC_BUFFER_OVERFLOW;
				}
				coder_high -= CODER_VALUEHLF;
				coder_low -= CODER_VALUEHLF;
			}
			else if ((CODER_VALUE1ST <= coder_low) && (coder_high < CODER_VALUE3RD)) {
				++coder_opbits;
				coder_high -= CODER_VALUE1ST;
				coder_low -= CODER_VALUE1ST;
			}
			else {
				break;
			}
			coder_high <<= 1;
			++coder_high;
			coder_low <<= 1;
			if (coder_high < coder_low) {
				vector_clear(&tmpv);
				return EC_CODER_WRONG_RANGE;
			}
		}
		{
#ifdef TXT_TOKENS
//                        register u32 ndx;
			predctrs[0] = *it;
			if ((('a' <= *it) && (*it <= 'z')) || (('A' <= *it) && (*it <= 'Z'))) {
				++(predctrs[1]);
			}
			else {
				predctrs[1] = 0;
			}
#else
			register u32 ndx = predctrs[0] + 1;
			predctrs[ndx + 8] = predctrs[ndx];
			predctrs[ndx] = *it;
			if (ndx == 8) {
				predctrs[0] = 0;
			}
			else {
				++predctrs[0];
			}
#endif
		}

	}
	vector_clear(&tmpv);
	++coder_opbits;
	if (coder_low < CODER_VALUE1ST) {
		if (writebits0(&bitvector_ptr, &bitvector_freebits, &coder_opbits, bitvector_end)) {
			return EC_BUFFER_OVERFLOW;
		}
	}
	else {
		if (writebits1(&bitvector_ptr, &bitvector_freebits, &coder_opbits, bitvector_end)) {
			return EC_BUFFER_OVERFLOW;
		}
	}
	(*(bitvector_ptr)) <<= bitvector_freebits;
	*dstlen = ((u32) bitvector_ptr - (u32) output + 1);
	return EC_NO_ERROR;
}

/*******************************************************************************

        DECOMPRESS FUNCTIONS

*******************************************************************************/

typedef struct
{
	u32 high;
	u32 low;
	u32 value;
	u32 overread;
} ipack_decompressor_values;

typedef struct
{
	u8 freebits;
	unsigned char *ptr;
	unsigned char *end;
} ipack_decompressor_bitvector;

static u8 ipack_bitvector_R_get1(ipack_decompressor_bitvector * bv)
{
	u8 tmp;
	if (bv->ptr == bv->end) {
		bv->freebits = 0;
		return 0;
	}
	tmp = (*(bv->ptr) >> bv->freebits) & 0x01;
	if (!(bv->freebits)) {
		bv->freebits = 7;
		++(bv->ptr);
	}
	else {
		--(bv->freebits);
	}
	return tmp;
}

/* Decompress block 
 *   returns non zero if fails
 */
static int ipack_armlib_decompress(void *model, unsigned char *input, unsigned char *output, unsigned long sourcelen, unsigned long dstlen)
{
	ARM_DataType *data;
	register u32 coder_high = CODER_VALUEMAX - 1;
	register u32 coder_low = 0;
	register u32 coder_value = 0;
	u32 coder_overread = 0;
	ipack_decompressor_bitvector bitvector;
	u32 lngth;
	u32 i;
	u32 cntbytes;
	TokenType tkns[8];
	TokenType *tptr;

	ipack_nodetype *treeroot = ((ipack_model_type *) model)->tree_root_ptr;
	PredictorType *predctrs = ((ipack_model_type *) model)->predictors_ptr;

#ifdef IPACK_TREE_TO_CODE
	TokenType(*treefunc) (ipack_nodetype *, u32, u32, ipack_probability_type *);

	treefunc = ((ipack_model_type *) model)->tree_code;
#endif


	if (((ipack_model_type *) model)->ID[0] != 'i') {
		return EC_NOT_IPMF_MODEL;
	}
	else if (((ipack_model_type *) model)->ID[1] != 'P') {
		return EC_NOT_IPMF_MODEL;
	}
	else if (((ipack_model_type *) model)->ID[2] != 'M') {
		return EC_NOT_IPMF_MODEL;
	}
	else if (((ipack_model_type *) model)->ID[3] != 'F') {
		return EC_NOT_IPMF_MODEL;
	}

	bitvector.freebits = 7;
	bitvector.ptr = input;
	bitvector.end = input + sourcelen;

	/*if(*(bitvector.ptr++) != 'H') {
	   return EC_NOT_HG_BLOCK;
	   } else if(*(bitvector.ptr++) != 'G') {
	   return EC_NOT_HG_BLOCK;
	   } */
	bitvector.ptr++;
	bitvector.ptr++;

	data = (void *) output;
	cntbytes = *(bitvector.ptr++);
	cntbytes <<= 8;
	cntbytes += *(bitvector.ptr++);

	{			//predictor reset
		register PredictorType *ptr = predctrs;
		register PredictorType *end = ptr + NUMBER_OF_PREDICTORS;
		while (ptr < end) {
			*(ptr++) = 0;
		}
	}
	for (i = 0; i < CODER_VALUEBITS; ++i) {
		coder_value <<= 1;
		coder_value += ipack_bitvector_R_get1(&bitvector);
	}
	lngth = dstlen >> 2;
	if (lngth > (cntbytes >> 2)) {
		lngth = cntbytes >> 2;
	}
	for (i = 0; (i < lngth); ++i) {
		TokenType itoken;
		u32 j;
		tptr = tkns;
		for (j = 0; j < NUMBER_OF_TOKENS_PER_INSTRUCTION; ++j) {
			ipack_probability_type prob;
			u32 range = coder_high - coder_low + 1;

#ifdef IPACK_TREE_TO_CODE
			if (treefunc != NULL)
				itoken = (*treefunc) (treeroot, coder_value - coder_low + 1, range, &prob);
			else
#endif
				itoken = ipack_model_get_token_for_range(treeroot, coder_value - coder_low + 1, range, &prob);


			if (prob.high == prob.low) {
				return EC_CODER_WRONG_PROBABILITY;
			}
			coder_high = coder_low + ((range * prob.high) >> PROBABILITY_SHIFT) - 1;
			coder_low += ((range * prob.low) >> PROBABILITY_SHIFT);
			for (;;) {
				if (coder_high < CODER_VALUEHLF) {
				}
				else if (CODER_VALUEHLF <= coder_low) {
					coder_value -= CODER_VALUEHLF;
					coder_high -= CODER_VALUEHLF;
					coder_low -= CODER_VALUEHLF;
				}
				else if ((CODER_VALUE1ST <= coder_low) && (coder_high < CODER_VALUE3RD)) {
					coder_value -= CODER_VALUE1ST;
					coder_high -= CODER_VALUE1ST;
					coder_low -= CODER_VALUE1ST;
				}
				else {
					break;
				}
				coder_low <<= 1;
				coder_high <<= 1;
				++(coder_high);
				coder_value <<= 1;
				if (bitvector.ptr == bitvector.end) {
					bitvector.freebits = 0;
				}
				coder_value += ((*(bitvector.ptr) >> bitvector.freebits) & 0x01);
				if (bitvector.freebits) {
					--bitvector.freebits;
				}
				else {
					bitvector.freebits = 7;
					++bitvector.ptr;
				}
				if (coder_high < coder_low) {
					return EC_CODER_WRONG_RANGE;
				}
				if ((bitvector.ptr == bitvector.end) && !(bitvector.freebits)) {
					if ((coder_overread++) > CODER_VALUEBITS) {
						return EC_BUFFER_UNDERFLOW;
					}
				}
			}
			{
#ifdef TXT_TOKENS
//                                register u32 ndx;
				predctrs[0] = itoken;
				if ((('a' <= itoken) && (itoken <= 'z')) || (('A' <= itoken) && (itoken <= 'Z'))) {
					++(predctrs[1]);
				}
				else {
					predctrs[1] = 0;
				}

#else
				register u32 ndx = predctrs[0] + 1;
				predctrs[ndx + 8] = predctrs[ndx];
				predctrs[ndx] = itoken;
				if (ndx == 8) {
					predctrs[0] = 0;
				}
				else {
					++predctrs[0];
				}
#endif
			}

			(*(tptr++)) = itoken;
		}
		tptr = tkns;
#ifndef __KERNEL__
		if (ipack_glb_endian_X) {
#ifdef TXT_TOKENS
			(*data) = ((*tptr) << 24);
			++tptr;
			(*data) |= ((*tptr) << 16);
			++tptr;
			(*data) |= ((*tptr) << 8);
			++tptr;
			(*data) |= (*tptr);
			++data;
#else
			(*data) = (((*tptr) & 0xf) << 12);
			++tptr;
			(*data) |= ((*tptr) & 0xf);
			++tptr;
			(*data) |= (((*tptr) & 0xf) << 28);
			++tptr;
			(*data) |= (((*tptr) & 0xf) << 16);
			++tptr;
			(*data) |= (((*tptr) & 0xf) << 8);
			++tptr;
			(*data) |= (((*tptr) & 0xf) << 20);
			++tptr;
			(*data) |= (((*tptr) & 0xf) << 24);
			++tptr;
			(*data) |= (((*tptr) & 0xf) << 4);
			++data;
#endif
		}
		else {
#endif
#ifdef TXT_TOKENS
			(*data) = (*tptr);
			++tptr;
			(*data) |= ((*tptr) << 8);
			++tptr;
			(*data) |= ((*tptr) << 16);
			++tptr;
			(*data) |= ((*tptr) << 24);
			++data;
#else
			(*data) = (((*tptr) & 0xf) << 20);
			++tptr;
			(*data) |= (((*tptr) & 0xf) << 24);
			++tptr;
			(*data) |= (((*tptr) & 0xf) << 4);
			++tptr;
			(*data) |= (((*tptr) & 0xf) << 8);
			++tptr;
			(*data) |= (((*tptr) & 0xf) << 16);
			++tptr;
			(*data) |= (((*tptr) & 0xf) << 12);
			++tptr;
			(*data) |= ((*tptr) & 0xf);
			++tptr;
			(*data) |= (((*tptr) & 0xf) << 28);
			++data;
#endif
#ifndef __KERNEL__
		}
#endif
	}
	return EC_NO_ERROR;
}

static int ipack_armlib_estimate(void *model, unsigned char *input, unsigned long sourcelen, unsigned long *dstlen, unsigned long *readtime, unsigned long *writetime)
{
	int i, tmp, tmp2, max, maxi;
	int cnt_cond[] = { 0, 0, 0, 0 };
	int cnt_inst[] = { 0, 0, 0, 0 };

	// TODO: make a more precise estimation!!!
	*readtime  = JFFS2_BBC_ZLIB_READ_TIME  * 6;
	*writetime = JFFS2_BBC_ZLIB_WRITE_TIME * 2;

	if (sourcelen % 4 != 0) {
		*dstlen = sourcelen;
		return 0;
	}
	for (i = 0; i < sourcelen; i++, input++) {
		tmp2 = tmp = *input;
		tmp = ((tmp) & 0xf0) >> 4;
		tmp2 = tmp2 & 0xf;
		if (tmp == 14)
			cnt_cond[i % 4]++;
		if ((tmp2 == 2) || (tmp2 == 3))
			cnt_inst[i % 4]++;
	}
	maxi = -1;
	max = -1;
	for (i = 0; i < 4; i++)
		if (max < cnt_cond[i]) {
			max = cnt_cond[i];
			maxi = i;
		}
	/*jffs2_bbc_print("armlib_EST: %d/%d : %d/%d %d/%d %d/%d %d/%d",
	   cnt_cond[maxi],cnt_inst[maxi],
	   cnt_cond[0],cnt_inst[0],
	   cnt_cond[1],cnt_inst[1],
	   cnt_cond[2],cnt_inst[2],
	   cnt_cond[3],cnt_inst[3]); */

	if (cnt_cond[maxi] < (sourcelen >> 4)) {
		*dstlen = sourcelen;
	}
	else {
		*dstlen = sourcelen / 3;
	}	
	
	return 0;
}

static char *ipack_armlib_proc_info(void);
static int ipack_armlib_proc_command(char *command);
static void ipack_armlib_destroy_model(void **model);

struct jffs2_bbc_compressor_type jffs2_bbc_armlib = {
	"armlib",
	0x464d5069,
	{0, 0, 0, 0},
	NULL,			// init
	ipack_armlib_compressor_init,	// init_model
	ipack_armlib_destroy_model,	// destroy_model
	ipack_armlib_compressor_deinit,	// deinit
	ipack_armlib_compress,
	ipack_armlib_estimate,
	ipack_armlib_decompress,
	ipack_armlib_proc_info,
	ipack_armlib_proc_command
};


static char *ipack_armlib_proc_info()
{
#ifdef IPACK_TREE_TO_CODE
#ifdef IPACK_AUTO_TREE_TO_CODE
	return "automatic tree to code conversion";
#else
	return "manual tree to code conversion possibility";
#endif
#else
	return "tree in memory version";
#endif
}

static int ipack_armlib_proc_command(char *command)
{
	struct jffs2_bbc_model_list_node *model;
	ipack_model_type *armlib_model;

	if ((*command == 'g') || (*command == 'G')) {
		jffs2_bbc_print1("Converting tree(s) to ARM code... (keeping original)\n");
		model = jffs2_bbc_armlib.models;
		if (model == NULL)
			jffs2_bbc_print1("no model found!\n");
		while (model != NULL) {
			armlib_model = model->model;
			if (armlib_model == NULL) {
				jffs2_bbc_print1("Error: NULL model!\n");
			}
			else {
				ipack_armlib_convert_tree_to_code(armlib_model, IPACK_TREE_CONVERT_KEEP);
			}
			model = model->next_compr_model;
		}
	}
	else if ((*command == 'r') || (*command == 'R')) {
		jffs2_bbc_print1("Converting tree(s) to ARM code... (deleting original)\n");
		model = jffs2_bbc_armlib.models;
		if (model == NULL)
			jffs2_bbc_print1("no model found!\n");
		while (model != NULL) {
			armlib_model = model->model;
			if (armlib_model == NULL) {
				jffs2_bbc_print1("Error: NULL model!\n");
			}
			else {
				//armlib_model->tree_code = ipack_tree_to_code(armlib_model, &tree_size);
				//jffs2_bbc_print("Convertation done. Code size=%d\n",tree_size);
				ipack_armlib_convert_tree_to_code(armlib_model, IPACK_TREE_CONVERT_REPLACE);
			}
			model = model->next_compr_model;
		}
	}
	else if ((*command == 'c') || (*command == 'C')) {
		jffs2_bbc_print1("Deleting ARM representation of the tree(s)...\n");
		model = jffs2_bbc_armlib.models;
		if (model == NULL)
			jffs2_bbc_print1("no model found!\n");
		while (model != NULL) {
			armlib_model = model->model;
			if (armlib_model == NULL) {
				jffs2_bbc_print1("Error: NULL model!\n");
			}
			else {
				if (armlib_model->tree_code == NULL) {
					jffs2_bbc_print1("already deleted.\n");
				}
				else {
					if (armlib_model->tree_root_ptr == NULL) {
						jffs2_bbc_print1("cannot delete this ARM tree - original tree has deleted\n");
					}
					else {
						jffs2_bbc_print1("deleting...");
						jffs2_bbc_free(armlib_model->tree_code);
						armlib_model->tree_code = NULL;
						jffs2_bbc_print1("done.\n");
					}
				}
			}
			model = model->next_compr_model;
		}
	}
	else if (*command == '?') {
		jffs2_bbc_print1("ARMLIB commands:\n");
		jffs2_bbc_print1("  g: convert TREEs to ARM code and keep the original\n");
		jffs2_bbc_print1("  r: convert TREEs to ARM code and remove the original\n");
		jffs2_bbc_print1("  c: delete the original TREEs - if there is any\n");
	}
	else {
		jffs2_bbc_print1("Unknown command.\n");
	}
	return 0;
}

static void ipack_armlib_destroy_model(void **model)
{
	ipack_model_type *model_img;

	model_img = *model;
	if (model_img == NULL) {
		jffs2_bbc_print1("jffs2.bbc: armlib: NULL model at destoying model!\n");
		return;
	}
	if (model_img->tree_code != NULL) {
		//jffs2_bbc_print1("jffs2.bbc: armlib: debug: freeing code...\n");
		jffs2_bbc_free(model_img->tree_code);
		model_img->tree_code = NULL;
	}
	if (model_img->tree_root_ptr != NULL) {
		//jffs2_bbc_print1("jffs2.bbc: armlib: debug: freeing tree...\n");
		jffs2_bbc_free(model_img->tree_root_ptr);
		model_img->tree_root_ptr = NULL;
	}

	jffs2_bbc_free(model_img);
	*model = NULL;
}

struct jffs2_bbc_compressor_type *jffs2_bbc_armlib_init(int mode)
{
	if (jffs2_bbc_register_compressor(&jffs2_bbc_armlib) == 0)
		return &jffs2_bbc_armlib;
	else
		return NULL;
}

void jffs2_bbc_armlib_deinit(void)
{
	jffs2_bbc_unregister_compressor(&jffs2_bbc_armlib);
}

/*END OF ARMLIB*/
