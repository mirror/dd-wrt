/***********************license start***************
 * Copyright (c) 2003-2014  Cavium Inc. (support@cavium.com). All rights
 * reserved.
 *
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *
 *   * Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *
 *   * Redistributions in binary form must reproduce the above
 *     copyright notice, this list of conditions and the following
 *     disclaimer in the documentation and/or other materials provided
 *     with the distribution.

 *   * Neither the name of Cavium Inc. nor the names of
 *     its contributors may be used to endorse or promote products
 *     derived from this software without specific prior written
 *     permission.

 * This Software, including technical data, may be subject to U.S. export  control
 * laws, including the U.S. Export Administration Act and its  associated
 * regulations, and may be subject to export or import  regulations in other
 * countries.

 * TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
 * AND WITH ALL FAULTS AND CAVIUM INC. MAKES NO PROMISES, REPRESENTATIONS OR
 * WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT TO
 * THE SOFTWARE, INCLUDING ITS CONDITION, ITS CONFORMITY TO ANY REPRESENTATION OR
 * DESCRIPTION, OR THE EXISTENCE OF ANY LATENT OR PATENT DEFECTS, AND CAVIUM
 * SPECIFICALLY DISCLAIMS ALL IMPLIED (IF ANY) WARRANTIES OF TITLE,
 * MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE, LACK OF
 * VIRUSES, ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION OR
 * CORRESPONDENCE TO DESCRIPTION. THE ENTIRE  RISK ARISING OUT OF USE OR
 * PERFORMANCE OF THE SOFTWARE LIES WITH YOU.
 ***********************license end**************************************/

/**
 * @file
 *
 * Utility functions for working with the CSR database
 *
 * <hr>$Revision: 96612 $<hr>
 */
#include <ctype.h>
#ifdef CVMX_BUILD_FOR_LINUX_KERNEL
#define PRINTF printk
#include <asm/octeon/cvmx.h>
#include <asm/octeon/cvmx-csr-db.h>
#else
#define PRINTF printf
#include "cvmx.h"
#include "cvmx-csr-db.h"
#endif

#ifndef CVMX_BUILD_FOR_LINUX_KERNEL
#define LEFT_PARENTH			'('
#define RIGHT_PARENTH			')'
#define OCTCSR_MAX_NAME_LEN		128
#define OCTCSR_MAX_NAME_PART_LEN	64
#define OCTCSR_MAX_NINDEX		4
#define OCTCSR_MAX_SUBRANGES		10
#define OCTCSR_INVALID_INDEX		-1
#define OCTCSR_INVALID_BASEADDR		-1
#define OCTCSR_INVALID_INC		0xbadbadbadbadULL

/**
 * Bidimentional array containing the subranges
 * [i][0] is min of index range
 * [i][1] is max of index range
 */
typedef struct {
	int csr_idx[OCTCSR_MAX_SUBRANGES][2];
} csr_idx_range_t;

/**
 * A basename is divided into name parts by ranges. This struct is for
 * storing a name part.
 */
typedef struct {
	char csr_name_part[OCTCSR_MAX_NAME_PART_LEN];
	int len;	/**< The length of the name part stored */
} csr_name_part_t;

typedef enum {
	chunk_type_text,
	chunk_type_range,
} csr_chunk_type_t;

typedef struct {
	csr_chunk_type_t type;
	char str[OCTCSR_MAX_NAME_LEN];
	int csr_idx[OCTCSR_MAX_SUBRANGES][2];
} csr_chunk_t;

typedef struct {
	int identifier;		/**< Proc or PCI id */
	csr_chunk_t chunk[10];
	int maxidx;
	char *pattern;		/**< Pattern with wich to compare or NULL */
	int patlen;		/**< Length of the pattern string */
	int decode;		/**< If != 0 Decode the generated entry. MUST not be remote.
					If 0 - Print the entry */
} csr_chunk_params_t;

/**
 * Figure out which database to use for this chip. The passed
 * identifier can be a processor ID or a PCI ID.
 *
 * @param identifier processor ID or a PCI ID
 *
 * @return index into the csr db
 */
int cvmx_db_get_chipindex(int identifier)
{
	/* First try and see if the identifier is a Processor ID */
	switch (identifier & 0xffff00) {
	case 0x000d0600:	/* CN50XX */
		return 8;
	case 0x000d0400:	/* CN56XX */
		return 7;
	case 0x000d0300:	/* CN58XX */
		return 5;
	case 0x000d0000:	/* CN38XX */
		return 3;
	case 0x000d0100:	/* CN31XX */
		return 1;
	case 0x000d0200:	/* CN3010 */
		return 2;
	case 0x000d0700:	/* CN52XX */
		return 10;
	case 0x000d9300:	/* CN61XX */
		return 11;
	case 0x000d9000:	/* CN63XX */
		return 13;
	case 0x000d9200:	/* CN66XX */
		return 14;
	case 0x000d9100:	/* CN68XX */
		return 16;
	case 0x000d9400:	/* CNF71XX */
		return 17;
	case 0x000d9600:	/* CN70XX */
		return 18;
	case 0x000d9500:	/* CN78XX */
		return 19;
	}

	/* Next try PCI device IDs */
	switch (identifier) {
	case 0x0003177d:	/* CN38XX Pass 1 */
		return 0;
	case 0x0004177d:	/* CN38XX Pass 2 */
		return 0;
	case 0x0005177d:	/* CN38XX Pass 3 */
		return 3;
	case 0x1001177d:	/* Thunder */
		return 3;
	case 0x0020177d:	/* CN31XX Pass 1 */
		return 1;
	case 0x0030177d:	/* CN30XX Pass 1 */
		return 2;
	case 0x0040177d:	/* CN58XX Pass 2 */
		return 5;
	case 0x0050177d:	/* CN56XX Pass 2 */
		return 7;
	case 0x0070177d:	/* CN50XX Pass 1 */
		return 8;
	case 0x0080177d:	/* CN52XX Pass 2 */
		return 10;
	case 0x0093177d:	/* CN61XX Pass 2 */
		return 11;
	case 0x0090177d:	/* CN63XX Pass 2 */
		return 13;
	case 0x0092177d:	/* CN66XX Pass 1 */
		return 14;
	case 0x0091177d:	/* CN68XX Pass 2 */
		return 16;
	case 0x0094177d:	/* CNF71XX Pass 1 */
		return 17;
	case 0x0096177d:	/* CN70XX Pass 1 */
		return 18;
	case 0x0095177d:	/* CN78XX Pass 1 */
		return 19;
	}

	/* Default to Pass 3 if we don't know */
	return 3;
}

/**
 * Convert the basename of a csr to its macro name in the cvmx-xxxx-defs.h.
 * This is done by replacing the '(...)' to 'x', e.g.,
 * "cvmx_gmx(0..1)_rx(0..3)_int_reg" would become "cvmx_gmxx_rxx_int_reg."
 *
 * @param csr_basename
 * @param csr_defname
 * @return 0 for success and -1 for error.
 */
static int __cvmx_csr_db_base2def(char *csr_basename, char *csr_defname)
{
	char *p, *q, *pp;
	int len;

	pp = csr_defname;

	q = p = csr_basename;
	while (p != NULL) {
		p = strchr(q, LEFT_PARENTH);
		if (p) {
			len = p - q;
			strncpy(pp, q, len);
			pp += len;
			*pp = 'x'; pp++;
			q = strchr(q, RIGHT_PARENTH);
			if (!q) {
				cvmx_dprintf(
				    "%s: fatal error no matching ')'\n",
				    __func__);
				return -1;
			}
			q++;
		} else {
			strcpy(pp, q);
		}
	}

	return 0;
}

/**
 * Given a CSR macro name, find the matching entry in the database.
 * @param identifier is the processor ID or a PCI ID.
 * @param csr_defname
 * @return NULL for error.
 *
 * TODO: as the CSR db entries are sorted wrt the basename, a binary search here
 * should be doable.
 */
static __CVMX_CSR_DB_TYPE *__cvmx_csr_dbe_by_defname(int identifier,
    char *csr_defname)
{
	int chip = cvmx_db_get_chipindex(identifier);
	char buf[OCTCSR_MAX_NAME_LEN];
	__CVMX_CSR_DB_TYPE *db, *pdbe;

	if (csr_defname) {
		db = (__CVMX_CSR_DB_TYPE *)__cvmx_csr_db[chip];
		pdbe = &db[0];
		while (pdbe) {
			if (__cvmx_csr_db_base2def(pdbe->basename, buf) == 0) {
				if (strcasecmp(buf, csr_defname) == 0)
					return pdbe;
			} else {
				cvmx_dprintf("%s: base2def error (%s, %s)\n",
				    __func__, pdbe->basename, buf);
				return NULL;
			}
			pdbe++;
		}
	}

	cvmx_dprintf("%s: %s does not have a basename on chip 0x%x\n",
	    __func__, csr_defname, chip);

	return NULL;
}

/**
 * Return an uint64_t for a csr where the bits for all the reserved fields are
 * set to 1.
 *
 * @param identifier is the processor ID or a PCI ID.
 * @param csr_defname is the macro name of a CSR, as defined in the cvmx-xxxx-defs.h.
 *
 * @return (uint64_t)-1 for error.
 */
uint64_t cvmx_csr_db_rsvd_mask(int identifier, char *csr_defname)
{
	__CVMX_CSR_DB_TYPE *pdbe;
	__CVMX_CSR_DB_FIELD_TYPE *pfd;
	int i, chip, max_fieldoff;
	uint64_t rval = 0, fmask;

	pdbe = __cvmx_csr_dbe_by_defname(identifier, csr_defname);
	if (pdbe == NULL) {
		cvmx_dprintf("ERROR: %s cannot find an entry for %s\n",
		    __func__, csr_defname);
		return (uint64_t)-1;
	}

	chip = cvmx_db_get_chipindex(identifier);
	max_fieldoff = pdbe->numfields + pdbe->fieldoff;
	for (i = pdbe->fieldoff; i < max_fieldoff; i++) {
		pfd = (__CVMX_CSR_DB_FIELD_TYPE *)
		    &__cvmx_csr_db_fields[chip][i];
		if (strncmp(pfd->name, "RESERVED", 8) == 0) {
			/* mask for the reserved field */
			fmask = ((((uint64_t)1) << pfd->sizebits) - 1)
			    << pfd->startbit;
			rval |= fmask;
		}
	}

	return rval;
}

/**
 * Resovle the csr_basename into index ranges and name parts.
 *
 * @param csr_basename
 * @param irange points to the csr_idx_range_t arrays to hold the result index
 *        ranges and subranges (min1..max1,min2..max2,min3..max3).
 * @param npart points to the csr_name_part_t array to hold the name parts.
 * @return the number of indices found in the csr_basename. 0 means that the
 * 	  CSR has no ranges and -1 for a match failure.
 *
 * Note:
 * 	Make sure the idx range and name part arrays are big enough. As of o78,
 * 	the max of either #indices or #name-parts is 4. Therefore, the sizes of
 * 	the arrays should be >= 5.
 *
 * 	The function uses the next empty slot in each array to indicate end.
 * 	For the index range array, it is the
 * 		csr_idx[0][0] == OCTCSR_INVALID_INDEX.
 * 	For the name part array, it is csr_name_part[0] == 0.
 */
int __cvmx_csr_basename_resolve(char *csr_basename, csr_idx_range_t *irange,
    csr_name_part_t *npart)
{
	int i, j;
	char *p, *q, c;
	int len = 0, rl;

	/*
	 * Each loop starts with p pointing to the next '(' in the basename.
	 * - record the name part,
	 * - sscanf the min and max of the index pointed to by p,
	 * - find the right parenthesis and set q to point after it, and
	 * - find the next '('.
	 */
	i = 0;
	j = 0;
	q = csr_basename;
	p = strchr(q, LEFT_PARENTH);
	while (p != NULL) {
		len = strcspn(q, "(");
		strncpy(npart[i].csr_name_part, q, len);
		npart[i].csr_name_part[len] = 0;
		npart[i].len = len;
		p++; /* go after the '(' */
		/* Read subranges */
		j = 0;
		do {
			if (sscanf(p, "%d..%d%c%n",  &irange[i].csr_idx[j][0],
			&irange[i].csr_idx[j][1], &c, &rl) != 3) {
				fprintf(stderr, "(%s: sscanf matching failure on string '%s')",
			    __func__, p);
			return -1;
		}
			p += rl;
			j++;
		} while (c != ')');
		irange[i].csr_idx[j][0] = OCTCSR_INVALID_INDEX;
		q = strchr(q, RIGHT_PARENTH) + 1;
		p = strchr(p, LEFT_PARENTH);
		i++;
	}

	if (*q) {
		strcpy(npart[i].csr_name_part, q);
		npart[i].len = strlen(q);
		npart[i + 1].csr_name_part[0] = 0;
	} else {
		npart[i].csr_name_part[0] = 0;
	}

	irange[i].csr_idx[0][0] = OCTCSR_INVALID_INDEX;

	return i;
}

/**
 * Compare a csr name with the basename of a csr db entry.
 *
 * @param csr_name points to the CSR name
 * @param pdbe points to the database entry
 *
 * @returns 1, 0, and -1 when the csr_name is larger than, matches, and less
 * than the basename.
 */
int __cvmx_csr_name_cmp(char *csr_name, __CVMX_CSR_DB_TYPE *pdbe)
{
	char *p, csr_name_fixed[OCTCSR_MAX_NAME_LEN];
	int i, j, idx, nidx, r;
	csr_name_part_t npart[OCTCSR_MAX_NINDEX + 2];
	csr_idx_range_t irange[OCTCSR_MAX_NINDEX + 1];
	int equal;

	nidx = __cvmx_csr_basename_resolve(pdbe->basename, irange, npart);
	assert(nidx != -1); /* This should never happen. 
			     * We should make sure every csr basename resolves.
			     */
	/*
	 * If the csr name does not have the prefix "CVMX_", add it.
	 */
	p = csr_name;
	if (strncasecmp(p, "CVMX_", 5) != 0) {
		strcpy(csr_name_fixed, "CVMX_");
		strcat(csr_name_fixed, p);
		p = csr_name_fixed;
	}

	for (i = 0; i < (nidx + 1); i++) {
		r = strncasecmp(p, npart[i].csr_name_part, npart[i].len);
		if (r != 0)
			return r;
		/* 
		 * The name part matched. Now skip the index.
		 */
		p += strlen(npart[i].csr_name_part);
		if (isdigit((int)*p)) {
			sscanf(p, "%d", &idx);
			if (idx < irange[i].csr_idx[0][0])
				return -1;
			equal = 0;
			for (j = 0; irange[i].csr_idx[j][0] != OCTCSR_INVALID_INDEX; j++)
				if (idx >= irange[i].csr_idx[j][0] && idx <= irange[i].csr_idx[j][1]) {
					while(isdigit((int)*p)) p++;
					equal = 1;
					break;
				}
			if (!equal)
				return 1;
		} else if (*p && !isdigit((int)*p) && irange[i].csr_idx[0][0] != OCTCSR_INVALID_INDEX)
			return 1;
		else if (*p && irange[i].csr_idx[0][0] == OCTCSR_INVALID_INDEX) // If the searched string is longer than the current DB entry inspected - return 1
			return 1;
	}

	return 0;
}

/**
 * Compare a csr name prefix with the basename of a csr db entry.
 *
 * @param csr_name points to the CSR name
 * @param pdbe points to the database entry
 *
 * @returns 0 when csr_name matches its full length with part of or the full
 * expanded basename
 */
int __cvmx_csr_partial_name_cmp(char *csr_name, const __CVMX_CSR_DB_TYPE *pdbe)
{
	char *p, csr_name_fixed[OCTCSR_MAX_NAME_LEN];
	int i, j, idx, nidx, r;
	csr_name_part_t npart[OCTCSR_MAX_NINDEX + 2];
	csr_idx_range_t irange[OCTCSR_MAX_NINDEX + 1];
	int equal;

	nidx = __cvmx_csr_basename_resolve(pdbe->basename, irange, npart);
	assert(nidx != -1); /* This should never happen. 
			     * We should make sure every csr basename resolves.
			     */
	/*
	 * If the csr name does not have the prefix "CVMX_", add it.
	 */
	p = csr_name;
	if (strncasecmp(p, "CVMX_", 5) != 0) {
		strcpy(csr_name_fixed, "CVMX_");
		strcat(csr_name_fixed, p);
		p = csr_name_fixed;
	}

	for (i = 0; i < (nidx + 1); i++) {
		r = strncasecmp(p, npart[i].csr_name_part, npart[i].len);
		if (r != 0) {
			if (0 == strncasecmp(p, npart[i].csr_name_part, strlen(p))) {
				/* If we match the pattern in full - return found */
				return 0;
			} else {
				return r;
			}
		}

		/* 
		 * The name part matched. Now skip the index.
		 */
		p += strlen(npart[i].csr_name_part);

		if (!*p)
			return 0;

		if (isdigit((int)*p)) {
			sscanf(p, "%d", &idx);
			if (idx < irange[i].csr_idx[0][0]) {
				return -1;
			}
			equal = 0;
			for (j = 0; irange[i].csr_idx[j][0] != OCTCSR_INVALID_INDEX; j++)
				if (idx >= irange[i].csr_idx[j][0] && idx <= irange[i].csr_idx[j][1]) {
				while(isdigit((int)*p)) p++;
					equal = 1;
					break;
				}
			if (!equal)
				return 1;
		} else if (!isdigit((int)*p) && irange[i].csr_idx[0][0] != OCTCSR_INVALID_INDEX)
			return *p - '0';
		else if (irange[i].csr_idx[0][0] == OCTCSR_INVALID_INDEX) // If the searched string is longer than the current DB entry inspected - return 1
			return 1;
	}

	return 0;
}

/**
 * Given a chip identifier and a csr_name, returns the database entry.
 *
 * @param identifier is the chip identifier, PCI ID or ProcID.
 * @param csr_name
 *
 * @return the pointer to the matching database entry or NULL for failure
 */
static __CVMX_CSR_DB_TYPE *cvmx_csr_dbe_by_name(int identifier, char *csr_name)
{
	int chip, nmemb, step = 35, check = 5, i, j, cmp, max;
	__CVMX_CSR_DB_TYPE *db, *pdbe;
	chip = cvmx_db_get_chipindex(identifier);

	pdbe = db = (__CVMX_CSR_DB_TYPE *)__cvmx_csr_db[chip];
	nmemb = __cvmx_csr_db_cnt[chip];

	/* First search the DB with step increment */
	for (i = step; i < nmemb; i += step) {
		if (0 == (cmp = __cvmx_csr_name_cmp(csr_name, pdbe + i)))
			return pdbe + i;

		if (cmp < 0) {
			if (i + check >= nmemb) {
				continue;
			}
			/* NOTE check few entries after that so we make sure that we fix the fpa_fpf problem. */
			for (j = check; j > 0; j--) {
				if (0 == __cvmx_csr_name_cmp(csr_name, pdbe + i + j))
					return pdbe + i + j;
	}

			break;
		}
	}

	/* Then go back 2 steps and start sequential search */
	max = i > nmemb ? nmemb : i;
	i -= 2 * step;

	if (i < 0)
		i = 0;

	for ( ; i < max; i++)
		if (0 == __cvmx_csr_name_cmp(csr_name, pdbe + i))
			return pdbe + i;

	return NULL;
}

/**
 * Get the address of a CSR from its name and its matching entry in the CSR DB.
 * @param csr_name is the name of the CSR.
 * @param pdbe is the __cvmx_csr_db[][] entry.
 *
 * @return the address of the CSR, -1ULL if the csr_name does not match 
 *     pdbe->basename, or -2ULL when one index is out of range.
 */
uint64_t __cvmx_csr_get_addr_by_name(char *csr_name, __CVMX_CSR_DB_TYPE *pdbe)
{

	uint64_t csr_get_addrinc(__CVMX_CSR_DB_TYPE *pdbe, int i)
	{
		if (!pdbe || i < 0 || i >= OCTCSR_MAX_NINDEX)
			return -1;
	
		return __csrdb_addrinc[pdbe->incoff][i];
	}

	char *p, csr_name_fixed[OCTCSR_MAX_NAME_LEN];
	int nidx, idx, i, j, is_in_range;
	int indices[OCTCSR_MAX_NINDEX];
	csr_name_part_t npart[OCTCSR_MAX_NINDEX + 2];
	csr_idx_range_t irange[OCTCSR_MAX_NINDEX + 1];

	p = csr_name;
	strcpy(csr_name_fixed, "CVMX_");
	if (strncasecmp(p, "CVMX_", 5) != 0) {
		strcat(csr_name_fixed, p);
		p = csr_name_fixed;
	}

	if (__cvmx_csr_name_cmp(p, pdbe) != 0)
		return -1ULL;

	nidx = __cvmx_csr_basename_resolve(pdbe->basename, irange, npart);
	for (i = 0; i < (nidx + 1); i++) {
		/*
		 * skip the index digits
		 */
		p += strlen(npart[i].csr_name_part);

		if (sscanf(p, "%d", &idx) != 1) {
			i++;
			break;
		}
		while(isdigit((int)*p)) p++;

		/* 
		 * see if the index is in range.
		 */
		j = 0;
		is_in_range = 0;
		while (irange[i].csr_idx[j][0] != OCTCSR_INVALID_INDEX) {
			if (idx >= irange[i].csr_idx[j][0] && idx <= irange[i].csr_idx[j][1]) {
				is_in_range = 1;
			    break;
			}
			j++;
		}

		if (!is_in_range)
			break;

		indices[i] = idx;
	}

	if (i == (nidx + 1)) {	/* index range checking passed */
		uint64_t addr;

		addr = pdbe->baseaddr;
		for (i = 0; i < nidx; i++)
			addr += indices[i] * csr_get_addrinc(pdbe, i);

		return addr;
	}

	return -2ULL;
}

/**
 * Given DB identifer and csr_name, find out the CSR's address, type,
 * basename and CSR width.
 *
 * @return 0 for success, 
 * 	  -1 if the csr_name doesn't match anything in the database, and
 *	  -2 if the match is found but an index is out of range.
 *
 * Note: the function uses binary search over the database. This requires that
 * the database entries are in assending order wrt the CSR basename.
 */
int cvmx_csr_db_get_params(int identifier, char *csr_name, uint64_t *paddr,
    int *ptype, char **pbasename, int *pwidth)
{
	__CVMX_CSR_DB_TYPE *pdbe;
	uint64_t addr;
	
	pdbe = cvmx_csr_dbe_by_name(identifier, csr_name);
	if (pdbe) {
		addr = __cvmx_csr_get_addr_by_name(csr_name, pdbe);
		if (addr == -2ULL)
			return -2;

		if (paddr)
		    *paddr = addr;
		if (ptype)
		    *ptype = pdbe->type;
		if (pbasename)
		    *pbasename = pdbe->basename;
		if (pwidth)
		    *pwidth = pdbe->widthbits;

		return 0;
	}

	return -1;
}

#endif

#ifndef CVMX_BUILD_FOR_LINUX_KERNEL
/**
 * Decode a CSR value into named bitfields. The model can either
 * be specified as a processor id or PCI id.
 *
 * @param identifier Identifer to choose the CSR DB with
 * @param name       CSR name to decode
 * @param pdbe       Pointer to CSR DB entry
 * @param addr       CSR address
 * @param value      Value to decode
 */
void __cvmx_csr_db_decode_by_pdbe(int identifier, char *csr_name,
	__CVMX_CSR_DB_TYPE *pdbe, uint64_t addr, uint64_t value)
{
	__CVMX_CSR_DB_FIELD_TYPE *pfld;
	int field, chip = cvmx_db_get_chipindex(identifier);
	uint64_t v;
		char *ptr;

	if (!pdbe)
		return;

	if (0 == addr) {
		addr = __cvmx_csr_get_addr_by_name(csr_name, pdbe);
		if (addr == -2ULL) {
			fprintf(stderr, "Could not get addres for %s\n", csr_name);
			return;
		}
	}

		ptr = csr_name;
		while (*ptr) {
		*ptr = toupper((int)*ptr);
			ptr++;
		}

		PRINTF("%s(0x%016llx) = 0x%016llx\n", csr_name,
		    (unsigned long long)addr, (unsigned long long)value);

		for (field = pdbe->fieldoff + pdbe->numfields - 1;
		    field >= pdbe->fieldoff; field--) {
			pfld = (__CVMX_CSR_DB_FIELD_TYPE *)
			    &__cvmx_csr_db_fields[chip][field];
			v = (value >> pfld->startbit);
			if (pfld->sizebits < 64)
				v = v & ~((~0x0ull) << pfld->sizebits);
			if (pfld->sizebits == 1)
				PRINTF("  [   %2d] %-20s = %10llu (0x%llx)\n",
				    pfld->startbit, pfld->name,
				    (unsigned long long)v,
				    (unsigned long long)v);
			else
				PRINTF("  [%2d:%2d] %-20s = %10llu (0x%llx)\n",
				    pfld->startbit + pfld->sizebits - 1,
				    pfld->startbit, pfld->name,
				    (unsigned long long)v,
				    (unsigned long long)v);
		}
}

/**
 * Decode a CSR value into named bitfields. The model can either
 * be specified as a processor id or PCI id.
 *
 * @param identifier Identifer to choose the CSR DB with
 * @param name       CSR name to decode
 * @param value      Value to decode
 */
void cvmx_csr_db_decode_by_name(int identifier, char *csr_name,
    uint64_t value)
{
	__CVMX_CSR_DB_TYPE *pdbe;
	uint64_t addr;

	pdbe = cvmx_csr_dbe_by_name(identifier, csr_name);
	if (!pdbe)
		return;
	addr = __cvmx_csr_get_addr_by_name(csr_name, pdbe);
	__cvmx_csr_db_decode_by_pdbe(identifier, csr_name, pdbe, addr, value);
}

/**
 * Print or decode all csrs names beginning with a prefix. The
 * model can either be specified as a processor id or PCI id.
 * For decoding it must not be remote.
 *
 * @param identifier Identifer to choose the CSR DB with
 * @param prefix     Beginning prefix to look for
 * @param decode      If == 0 - print the entry; else decode it.
 */
int cvmx_csr_db_print_decode_by_prefix(int identifier, const char *prefix, int decode) {
	int i;
	char buf[OCTCSR_MAX_NAME_LEN] = {0};
	int chip = cvmx_db_get_chipindex(identifier);
	char *p, csr_name_fixed[OCTCSR_MAX_NAME_LEN];

	if (prefix == NULL)
		return -1;

	p = (char *)prefix;
	if (strncasecmp(p, "CVMX_", 5) != 0) {
		strcpy(csr_name_fixed, "CVMX_");
		strcat(csr_name_fixed, p);
		p = csr_name_fixed;
	}

	for (i = 0; NULL != __cvmx_csr_db[chip][i].basename; i++) {
		if (0 == __cvmx_csr_partial_name_cmp(p, &__cvmx_csr_db[chip][i])) {
			strncpy(buf, __cvmx_csr_db[chip][i].basename, OCTCSR_MAX_NAME_LEN);
			cvmx_csr_db_print_decode_by_basename(identifier, buf, p, decode);
			/* Strip 'cvmx' from the name before printing */
			/*PRINTF("%s\n", __cvmx_csr_db[chip][i].basename+5);*/
		}
	}

	return 0;
}

static int __cvmx_csr_db_decode_by_chunk(csr_chunk_params_t *params) {
	int i;
	char csr_name[OCTCSR_MAX_NAME_LEN] = {0};
	uint64_t csr_addr, value;
	__CVMX_CSR_DB_TYPE *pdbe = NULL;
	
	/* Generate the string from all the chunks */
	for (i = 0; i < (params->maxidx); i++) {
		strcat(csr_name, params->chunk[i].str);
	}

	if (params->pattern) {
		if (((int)strlen(csr_name) < params->patlen) || 
			0 != strncasecmp(csr_name, params->pattern, params->patlen)) {
			return 0;
		}
	}

	if (0 == params->decode) {
		printf("%s\n", csr_name+5);
	} else {
		/* WARNING When passing decode make sure this not remote (PCI) */
		pdbe = cvmx_csr_dbe_by_name(params->identifier, csr_name);
		if (!pdbe)
			return -1;

		csr_addr = __cvmx_csr_get_addr_by_name(csr_name, pdbe);
		if (csr_addr == -2ULL) {
			fprintf(stderr, "Error when getting address for csr name: %s\n", csr_name);
			return -2;
		}
		
		if (pdbe->widthbits == 32)
			value = cvmx_read64_uint32((csr_addr | (1ull<<63)) ^ 4);
		else
			value = cvmx_read_csr(csr_addr | (1ull<<63));
		
		__cvmx_csr_db_decode_by_pdbe(params->identifier, csr_name, pdbe, csr_addr, value);
	}

	return 0;
}

static int __cvmx_csr_db_walk_chunk(csr_chunk_params_t *params, int idx) {
	int i, j, ret = 0;

	if (idx >= params->maxidx) {
		fprintf(stderr, "Error: %s this should not happen\n", __func__);
		return -254;
	}

	if (params->chunk[idx].type == chunk_type_range) {
		for (j = 0; params->chunk[idx].csr_idx[j][0] != OCTCSR_INVALID_INDEX; j++) {
			for (i = params->chunk[idx].csr_idx[j][0]; i <= params->chunk[idx].csr_idx[j][1]; i++) {
				sprintf(params->chunk[idx].str, "%d", i);
				if ((idx+1) == params->maxidx) {
					ret = __cvmx_csr_db_decode_by_chunk(params);
					if (0 != ret)
						return ret;
				} else {
					__cvmx_csr_db_walk_chunk(params, idx+1);
				}
			}
		}
	} else {
		if ((idx+1) == params->maxidx) {
			ret = __cvmx_csr_db_decode_by_chunk(params);
			if (0 != ret)
				return ret;
		} else {
			__cvmx_csr_db_walk_chunk(params, idx+1);
		}
	}
	
	return ret;
}

/**
 * Print CSR given in the form "lmc(0..0)_char_mask(0..4)". This means that
 * this func will pring all the CSRs with names that end on 0 to 4.
 * The model can either be specified as a processor id or PCI id.
 *
 * @param identifier Identifer to choose the CSR DB with
 * @param basename   CSR name as is in CSR DB
 * @param pattern    The pattern that user searches for
 * @param decode      If == 0 - print the entry; else decode it.
 * 
 * @return 0 on success, <0 on error
 */
int cvmx_csr_db_print_decode_by_basename(int identifier, char *basename, char *pattern, int decode) {
	int i, j, cidx = 0, nidx, ret;
	csr_idx_range_t irange[OCTCSR_MAX_NINDEX + 1];
	csr_name_part_t npart[OCTCSR_MAX_NINDEX + 2];
	csr_chunk_params_t params;

	memset ((void*)&params, 0, sizeof(params));

	nidx = __cvmx_csr_basename_resolve(basename, irange, npart);
	assert(nidx != -1); /* This should never happen. 
			     * We should make sure every csr basename resolves.
			     */
	params.identifier = identifier;
	params.decode = decode;
	if (NULL == strchr(pattern, '(')) {
	    params.pattern = pattern;
	    params.patlen = strlen(pattern);
	}

	for (i = 0; npart[i].csr_name_part[0]; i++) {
		params.chunk[cidx].type = chunk_type_text;
		strcpy(params.chunk[cidx].str, npart[i].csr_name_part);
		cidx++;
		if (irange[i].csr_idx[0][0] != OCTCSR_INVALID_INDEX) {
			params.chunk[cidx].type = chunk_type_range;
			sprintf(params.chunk[cidx].str, "%d", irange[i].csr_idx[0][0]);
			for (j = 0; irange[i].csr_idx[j][0] != OCTCSR_INVALID_INDEX; j++) {
				params.chunk[cidx].csr_idx[j][0] = irange[i].csr_idx[j][0];
				params.chunk[cidx].csr_idx[j][1] = irange[i].csr_idx[j][1];
			}
			params.chunk[cidx].csr_idx[j][0] = OCTCSR_INVALID_INDEX;
			cidx++;
		}
	}

	params.maxidx = cidx;
	ret = __cvmx_csr_db_walk_chunk(&params, 0);

	return ret;
}

#endif
