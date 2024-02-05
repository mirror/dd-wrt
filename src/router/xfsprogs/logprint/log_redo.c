// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2000-2003,2005 Silicon Graphics, Inc.
 * Copyright (c) 2016 Oracle, Inc.
 * All Rights Reserved.
 */
#include "libxfs.h"
#include "libxlog.h"

#include "logprint.h"

/* Extent Free Items */

static int
xfs_efi_copy_format(
	char			  *buf,
	uint			  len,
	struct xfs_efi_log_format *dst_efi_fmt,
	int			  continued)
{
	uint i;
	uint nextents = ((xfs_efi_log_format_t *)buf)->efi_nextents;
	uint dst_len = xfs_efi_log_format_sizeof(nextents);
	uint len32 = xfs_efi_log_format32_sizeof(nextents);
	uint len64 = xfs_efi_log_format64_sizeof(nextents);

	if (len == dst_len || continued) {
		memcpy((char *)dst_efi_fmt, buf, len);
		return 0;
	} else if (len == len32) {
		xfs_efi_log_format_32_t *src_efi_fmt_32 = (xfs_efi_log_format_32_t *)buf;

		dst_efi_fmt->efi_type	 = src_efi_fmt_32->efi_type;
		dst_efi_fmt->efi_size	 = src_efi_fmt_32->efi_size;
		dst_efi_fmt->efi_nextents = src_efi_fmt_32->efi_nextents;
		dst_efi_fmt->efi_id	   = src_efi_fmt_32->efi_id;
		for (i = 0; i < dst_efi_fmt->efi_nextents; i++) {
			dst_efi_fmt->efi_extents[i].ext_start =
				src_efi_fmt_32->efi_extents[i].ext_start;
			dst_efi_fmt->efi_extents[i].ext_len =
				src_efi_fmt_32->efi_extents[i].ext_len;
		}
		return 0;
	} else if (len == len64) {
		xfs_efi_log_format_64_t *src_efi_fmt_64 = (xfs_efi_log_format_64_t *)buf;

		dst_efi_fmt->efi_type	 = src_efi_fmt_64->efi_type;
		dst_efi_fmt->efi_size	 = src_efi_fmt_64->efi_size;
		dst_efi_fmt->efi_nextents = src_efi_fmt_64->efi_nextents;
		dst_efi_fmt->efi_id	   = src_efi_fmt_64->efi_id;
		for (i = 0; i < dst_efi_fmt->efi_nextents; i++) {
			dst_efi_fmt->efi_extents[i].ext_start =
				src_efi_fmt_64->efi_extents[i].ext_start;
			dst_efi_fmt->efi_extents[i].ext_len =
				src_efi_fmt_64->efi_extents[i].ext_len;
		}
		return 0;
	}
	fprintf(stderr, _("%s: bad size of efi format: %u; expected %u or %u; nextents = %u\n"),
		progname, len, len32, len64, nextents);
	return 1;
}

int
xlog_print_trans_efi(
	char			**ptr,
	uint			src_len,
	int			continued)
{
	xfs_efi_log_format_t	*src_f, *f = NULL;
	uint			dst_len;
	xfs_extent_t		*ex;
	int			i;
	int			error = 0;
	int			core_size = offsetof(xfs_efi_log_format_t, efi_extents);

	/*
	 * memmove to ensure 8-byte alignment for the long longs in
	 * xfs_efi_log_format_t structure
	 */
	if ((src_f = (xfs_efi_log_format_t *)malloc(src_len)) == NULL) {
		fprintf(stderr, _("%s: xlog_print_trans_efi: malloc failed\n"), progname);
		exit(1);
	}
	memmove((char*)src_f, *ptr, src_len);
	*ptr += src_len;

	/* convert to native format */
	dst_len = xfs_efi_log_format_sizeof(src_f->efi_nextents);

	if (continued && src_len < core_size) {
		printf(_("EFI: Not enough data to decode further\n"));
		error = 1;
		goto error;
	}

	if ((f = (xfs_efi_log_format_t *)malloc(dst_len)) == NULL) {
		fprintf(stderr, _("%s: xlog_print_trans_efi: malloc failed\n"), progname);
		exit(1);
	}
	if (xfs_efi_copy_format((char*)src_f, src_len, f, continued)) {
		error = 1;
		goto error;
	}

	printf(_("EFI:  #regs: %d	num_extents: %d  id: 0x%llx\n"),
		f->efi_size, f->efi_nextents, (unsigned long long)f->efi_id);

	if (continued) {
		printf(_("EFI free extent data skipped (CONTINUE set, no space)\n"));
		goto error;
	}

	ex = f->efi_extents;
	for (i=0; i < f->efi_nextents; i++) {
		printf("(s: 0x%llx, l: %d) ",
			(unsigned long long)ex->ext_start, ex->ext_len);
		if (i % 4 == 3) printf("\n");
		ex++;
	}
	if (i % 4 != 0)
		printf("\n");
error:
	free(src_f);
	free(f);
	return error;
}	/* xlog_print_trans_efi */

void
xlog_recover_print_efi(
	struct xlog_recover_item *item)
{
	xfs_efi_log_format_t	*f, *src_f;
	xfs_extent_t		*ex;
	int			i;
	uint			src_len, dst_len;

	src_f = (xfs_efi_log_format_t *)item->ri_buf[0].i_addr;
	src_len = item->ri_buf[0].i_len;
	/*
	 * An xfs_efi_log_format structure contains a variable length array
	 * as the last field.
	 * Each element is of size xfs_extent_32_t or xfs_extent_64_t.
	 * Need to convert to native format.
	 */
	dst_len = sizeof(xfs_efi_log_format_t) +
		(src_f->efi_nextents) * sizeof(xfs_extent_t);
	if ((f = (xfs_efi_log_format_t *)malloc(dst_len)) == NULL) {
		fprintf(stderr, _("%s: xlog_recover_print_efi: malloc failed\n"),
			progname);
		exit(1);
	}
	if (xfs_efi_copy_format((char*)src_f, src_len, f, 0)) {
		free(f);
		return;
	}

	printf(_("	EFI:  #regs:%d	num_extents:%d  id:0x%llx\n"),
		   f->efi_size, f->efi_nextents, (unsigned long long)f->efi_id);
	ex = f->efi_extents;
	printf("	");
	for (i=0; i< f->efi_nextents; i++) {
		printf("(s: 0x%llx, l: %d) ",
			(unsigned long long)ex->ext_start, ex->ext_len);
		if (i % 4 == 3)
			printf("\n");
		ex++;
	}
	if (i % 4 != 0)
		printf("\n");
	free(f);
}

int
xlog_print_trans_efd(char **ptr, uint len)
{
	xfs_efd_log_format_t *f;
	xfs_efd_log_format_t lbuf;
	/* size without extents at end */
	uint core_size = sizeof(xfs_efd_log_format_t);

	/*
	 * memmove to ensure 8-byte alignment for the long longs in
	 * xfs_efd_log_format_t structure
	 */
	memmove(&lbuf, *ptr, min(core_size, len));
	f = &lbuf;
	*ptr += len;
	if (len >= core_size) {
		printf(_("EFD:  #regs: %d	num_extents: %d  id: 0x%llx\n"),
			f->efd_size, f->efd_nextents,
			(unsigned long long)f->efd_efi_id);

		/* don't print extents as they are not used */

		return 0;
	} else {
		printf(_("EFD: Not enough data to decode further\n"));
	return 1;
	}
}	/* xlog_print_trans_efd */

void
xlog_recover_print_efd(
	struct xlog_recover_item *item)
{
	xfs_efd_log_format_t	*f;

	f = (xfs_efd_log_format_t *)item->ri_buf[0].i_addr;
	/*
	 * An xfs_efd_log_format structure contains a variable length array
	 * as the last field.
	 * Each element is of size xfs_extent_32_t or xfs_extent_64_t.
	 * However, the extents are never used and won't be printed.
	 */
	printf(_("	EFD:  #regs: %d	num_extents: %d  id: 0x%llx\n"),
		f->efd_size, f->efd_nextents,
		(unsigned long long)f->efd_efi_id);
}

/* Reverse Mapping Update Items */

static int
xfs_rui_copy_format(
	char			  *buf,
	uint			  len,
	struct xfs_rui_log_format *dst_fmt,
	int			  continued)
{
	uint nextents = ((struct xfs_rui_log_format *)buf)->rui_nextents;
	uint dst_len = xfs_rui_log_format_sizeof(nextents);

	if (len == dst_len || continued) {
		memcpy((char *)dst_fmt, buf, len);
		return 0;
	}
	fprintf(stderr, _("%s: bad size of RUI format: %u; expected %u; nextents = %u\n"),
		progname, len, dst_len, nextents);
	return 1;
}

int
xlog_print_trans_rui(
	char			**ptr,
	uint			src_len,
	int			continued)
{
	struct xfs_rui_log_format	*src_f, *f = NULL;
	uint			dst_len;
	uint			nextents;
	struct xfs_map_extent	*ex;
	int			i;
	int			error = 0;
	int			core_size;

	core_size = offsetof(struct xfs_rui_log_format, rui_extents);

	/*
	 * memmove to ensure 8-byte alignment for the long longs in
	 * struct xfs_rui_log_format structure
	 */
	src_f = malloc(src_len);
	if (src_f == NULL) {
		fprintf(stderr, _("%s: %s: malloc failed\n"),
			progname, __func__);
		exit(1);
	}
	memmove((char*)src_f, *ptr, src_len);
	*ptr += src_len;

	/* convert to native format */
	nextents = src_f->rui_nextents;
	dst_len = xfs_rui_log_format_sizeof(nextents);

	if (continued && src_len < core_size) {
		printf(_("RUI: Not enough data to decode further\n"));
		error = 1;
		goto error;
	}

	f = malloc(dst_len);
	if (f == NULL) {
		fprintf(stderr, _("%s: %s: malloc failed\n"),
			progname, __func__);
		exit(1);
	}
	if (xfs_rui_copy_format((char *)src_f, src_len, f, continued)) {
		error = 1;
		goto error;
	}

	printf(_("RUI:  #regs: %d	num_extents: %d  id: 0x%llx\n"),
		f->rui_size, f->rui_nextents, (unsigned long long)f->rui_id);

	if (continued) {
		printf(_("RUI extent data skipped (CONTINUE set, no space)\n"));
		goto error;
	}

	ex = f->rui_extents;
	for (i=0; i < f->rui_nextents; i++) {
		printf("(s: 0x%llx, l: %d, own: %lld, off: %llu, f: 0x%x) ",
			(unsigned long long)ex->me_startblock, ex->me_len,
			(long long)ex->me_owner,
			(unsigned long long)ex->me_startoff, ex->me_flags);
		printf("\n");
		ex++;
	}
error:
	free(src_f);
	free(f);
	return error;
}

void
xlog_recover_print_rui(
	struct xlog_recover_item	*item)
{
	char				*src_f;
	uint				src_len;

	src_f = item->ri_buf[0].i_addr;
	src_len = item->ri_buf[0].i_len;

	xlog_print_trans_rui(&src_f, src_len, 0);
}

int
xlog_print_trans_rud(
	char				**ptr,
	uint				len)
{
	struct xfs_rud_log_format	*f;
	struct xfs_rud_log_format	lbuf;

	/* size without extents at end */
	uint core_size = sizeof(struct xfs_rud_log_format);

	/*
	 * memmove to ensure 8-byte alignment for the long longs in
	 * xfs_efd_log_format_t structure
	 */
	memmove(&lbuf, *ptr, min(core_size, len));
	f = &lbuf;
	*ptr += len;
	if (len >= core_size) {
		printf(_("RUD:  #regs: %d	                 id: 0x%llx\n"),
			f->rud_size,
			(unsigned long long)f->rud_rui_id);

		/* don't print extents as they are not used */

		return 0;
	} else {
		printf(_("RUD: Not enough data to decode further\n"));
		return 1;
	}
}

void
xlog_recover_print_rud(
	struct xlog_recover_item	*item)
{
	char				*f;

	f = item->ri_buf[0].i_addr;
	xlog_print_trans_rud(&f, sizeof(struct xfs_rud_log_format));
}

/* Reference Count Update Items */

static int
xfs_cui_copy_format(
	struct xfs_cui_log_format *cui,
	uint			  len,
	struct xfs_cui_log_format *dst_fmt,
	int			  continued)
{
	uint nextents;
	uint dst_len;

	nextents = cui->cui_nextents;
	dst_len = xfs_cui_log_format_sizeof(nextents);

	if (len == dst_len || continued) {
		memcpy(dst_fmt, cui, len);
		return 0;
	}
	fprintf(stderr, _("%s: bad size of CUI format: %u; expected %u; nextents = %u\n"),
		progname, len, dst_len, nextents);
	return 1;
}

int
xlog_print_trans_cui(
	char			**ptr,
	uint			src_len,
	int			continued)
{
	struct xfs_cui_log_format	*src_f, *f = NULL;
	uint			dst_len;
	uint			nextents;
	struct xfs_phys_extent	*ex;
	int			i;
	int			error = 0;
	int			core_size;

	core_size = offsetof(struct xfs_cui_log_format, cui_extents);

	src_f = malloc(src_len);
	if (src_f == NULL) {
		fprintf(stderr, _("%s: %s: malloc failed\n"),
			progname, __func__);
		exit(1);
	}
	memcpy(src_f, *ptr, src_len);
	*ptr += src_len;

	/* convert to native format */
	nextents = src_f->cui_nextents;
	dst_len = xfs_cui_log_format_sizeof(nextents);

	if (continued && src_len < core_size) {
		printf(_("CUI: Not enough data to decode further\n"));
		error = 1;
		goto error;
	}

	f = malloc(dst_len);
	if (f == NULL) {
		fprintf(stderr, _("%s: %s: malloc failed\n"),
			progname, __func__);
		exit(1);
	}
	if (xfs_cui_copy_format(src_f, src_len, f, continued)) {
		error = 1;
		goto error;
	}

	printf(_("CUI:  #regs: %d	num_extents: %d  id: 0x%llx\n"),
		f->cui_size, f->cui_nextents, (unsigned long long)f->cui_id);

	if (continued) {
		printf(_("CUI extent data skipped (CONTINUE set, no space)\n"));
		goto error;
	}

	ex = f->cui_extents;
	for (i=0; i < f->cui_nextents; i++) {
		printf("(s: 0x%llx, l: %d, f: 0x%x) ",
			(unsigned long long)ex->pe_startblock, ex->pe_len,
			ex->pe_flags);
		printf("\n");
		ex++;
	}
error:
	free(src_f);
	free(f);
	return error;
}

void
xlog_recover_print_cui(
	struct xlog_recover_item	*item)
{
	char				*src_f;
	uint				src_len;

	src_f = item->ri_buf[0].i_addr;
	src_len = item->ri_buf[0].i_len;

	xlog_print_trans_cui(&src_f, src_len, 0);
}

int
xlog_print_trans_cud(
	char				**ptr,
	uint				len)
{
	struct xfs_cud_log_format	*f;
	struct xfs_cud_log_format	lbuf;

	/* size without extents at end */
	uint core_size = sizeof(struct xfs_cud_log_format);

	memcpy(&lbuf, *ptr, min(core_size, len));
	f = &lbuf;
	*ptr += len;
	if (len >= core_size) {
		printf(_("CUD:  #regs: %d	                 id: 0x%llx\n"),
			f->cud_size,
			(unsigned long long)f->cud_cui_id);

		/* don't print extents as they are not used */

		return 0;
	} else {
		printf(_("CUD: Not enough data to decode further\n"));
		return 1;
	}
}

void
xlog_recover_print_cud(
	struct xlog_recover_item	*item)
{
	char				*f;

	f = item->ri_buf[0].i_addr;
	xlog_print_trans_cud(&f, sizeof(struct xfs_cud_log_format));
}

/* Block Mapping Update Items */

static int
xfs_bui_copy_format(
	struct xfs_bui_log_format *bui,
	uint			  len,
	struct xfs_bui_log_format *dst_fmt,
	int			  continued)
{
	uint nextents;
	uint dst_len;

	nextents = bui->bui_nextents;
	dst_len = xfs_bui_log_format_sizeof(nextents);

	if (len == dst_len || continued) {
		memcpy(dst_fmt, bui, len);
		return 0;
	}
	fprintf(stderr, _("%s: bad size of BUI format: %u; expected %u; nextents = %u\n"),
		progname, len, dst_len, nextents);
	return 1;
}

int
xlog_print_trans_bui(
	char			**ptr,
	uint			src_len,
	int			continued)
{
	struct xfs_bui_log_format	*src_f, *f = NULL;
	uint			dst_len;
	uint			nextents;
	struct xfs_map_extent	*ex;
	int			i;
	int			error = 0;
	int			core_size;

	core_size = offsetof(struct xfs_bui_log_format, bui_extents);

	src_f = malloc(src_len);
	if (src_f == NULL) {
		fprintf(stderr, _("%s: %s: malloc failed\n"),
			progname, __func__);
		exit(1);
	}
	memcpy(src_f, *ptr, src_len);
	*ptr += src_len;

	/* convert to native format */
	nextents = src_f->bui_nextents;
	dst_len = xfs_bui_log_format_sizeof(nextents);

	if (continued && src_len < core_size) {
		printf(_("BUI: Not enough data to decode further\n"));
		error = 1;
		goto error;
	}

	f = malloc(dst_len);
	if (f == NULL) {
		fprintf(stderr, _("%s: %s: malloc failed\n"),
			progname, __func__);
		exit(1);
	}
	if (xfs_bui_copy_format(src_f, src_len, f, continued)) {
		error = 1;
		goto error;
	}

	printf(_("BUI:  #regs: %d	num_extents: %d  id: 0x%llx\n"),
		f->bui_size, f->bui_nextents, (unsigned long long)f->bui_id);

	if (continued) {
		printf(_("BUI extent data skipped (CONTINUE set, no space)\n"));
		goto error;
	}

	ex = f->bui_extents;
	for (i=0; i < f->bui_nextents; i++) {
		printf("(s: 0x%llx, l: %d, own: %lld, off: %llu, f: 0x%x) ",
			(unsigned long long)ex->me_startblock, ex->me_len,
			(long long)ex->me_owner,
			(unsigned long long)ex->me_startoff, ex->me_flags);
		printf("\n");
		ex++;
	}
error:
	free(src_f);
	free(f);
	return error;
}

void
xlog_recover_print_bui(
	struct xlog_recover_item	*item)
{
	char				*src_f;
	uint				src_len;

	src_f = item->ri_buf[0].i_addr;
	src_len = item->ri_buf[0].i_len;

	xlog_print_trans_bui(&src_f, src_len, 0);
}

int
xlog_print_trans_bud(
	char				**ptr,
	uint				len)
{
	struct xfs_bud_log_format	*f;
	struct xfs_bud_log_format	lbuf;

	/* size without extents at end */
	uint core_size = sizeof(struct xfs_bud_log_format);

	memcpy(&lbuf, *ptr, min(core_size, len));
	f = &lbuf;
	*ptr += len;
	if (len >= core_size) {
		printf(_("BUD:  #regs: %d	                 id: 0x%llx\n"),
			f->bud_size,
			(unsigned long long)f->bud_bui_id);

		/* don't print extents as they are not used */

		return 0;
	} else {
		printf(_("BUD: Not enough data to decode further\n"));
		return 1;
	}
}

void
xlog_recover_print_bud(
	struct xlog_recover_item	*item)
{
	char				*f;

	f = item->ri_buf[0].i_addr;
	xlog_print_trans_bud(&f, sizeof(struct xfs_bud_log_format));
}

/* Attr Items */

static int
xfs_attri_copy_log_format(
	char				*buf,
	uint				len,
	struct xfs_attri_log_format	*dst_attri_fmt)
{
	uint dst_len = sizeof(struct xfs_attri_log_format);

	if (len == dst_len) {
		memcpy((char *)dst_attri_fmt, buf, len);
		return 0;
	}

	fprintf(stderr, _("%s: bad size of attri format: %u; expected %u\n"),
		progname, len, dst_len);
	return 1;
}

int
xlog_print_trans_attri(
	char				**ptr,
	uint				src_len,
	int				*i)
{
	struct xfs_attri_log_format	*src_f = NULL;
	xlog_op_header_t		*head = NULL;
	uint				dst_len;
	int				error = 0;

	dst_len = sizeof(struct xfs_attri_log_format);
	if (src_len != dst_len) {
		fprintf(stderr, _("%s: bad size of attri format: %u; expected %u\n"),
				progname, src_len, dst_len);
		return 1;
	}

	/*
	 * memmove to ensure 8-byte alignment for the long longs in
	 * xfs_attri_log_format_t structure
	 */
	src_f = malloc(src_len);
	if (!src_f) {
		fprintf(stderr, _("%s: xlog_print_trans_attri: malloc failed\n"),
				progname);
		exit(1);
	}
	memmove((char*)src_f, *ptr, src_len);
	*ptr += src_len;

	printf(_("ATTRI:  #regs: %d	name_len: %d, value_len: %d  id: 0x%llx\n"),
		src_f->alfi_size, src_f->alfi_name_len, src_f->alfi_value_len,
				(unsigned long long)src_f->alfi_id);

	if (src_f->alfi_name_len > 0) {
		printf(_("\n"));
		(*i)++;
		head = (xlog_op_header_t *)*ptr;
		xlog_print_op_header(head, *i, ptr);
		error = xlog_print_trans_attri_name(ptr, be32_to_cpu(head->oh_len));
		if (error)
			goto error;
	}

	if (src_f->alfi_value_len > 0) {
		printf(_("\n"));
		(*i)++;
		head = (xlog_op_header_t *)*ptr;
		xlog_print_op_header(head, *i, ptr);
		error = xlog_print_trans_attri_value(ptr, be32_to_cpu(head->oh_len),
				src_f->alfi_value_len);
	}
error:
	free(src_f);

	return error;
}	/* xlog_print_trans_attri */

int
xlog_print_trans_attri_name(
	char				**ptr,
	uint				src_len)
{
	printf(_("ATTRI:  name len:%u\n"), src_len);
	print_or_dump(*ptr, src_len);

	*ptr += src_len;

	return 0;
}	/* xlog_print_trans_attri */

int
xlog_print_trans_attri_value(
	char				**ptr,
	uint				src_len,
	int				value_len)
{
	int len = min(value_len, src_len);

	printf(_("ATTRI:  value len:%u\n"), value_len);
	print_or_dump(*ptr, len);

	*ptr += src_len;

	return 0;
}	/* xlog_print_trans_attri_value */

void
xlog_recover_print_attri(
	struct xlog_recover_item	*item)
{
	struct xfs_attri_log_format	*f, *src_f = NULL;
	uint				src_len, dst_len;

	int				region = 0;

	src_f = (struct xfs_attri_log_format *)item->ri_buf[0].i_addr;
	src_len = item->ri_buf[region].i_len;

	/*
	 * An xfs_attri_log_format structure contains a attribute name and
	 * variable length value  as the last field.
	 */
	dst_len = sizeof(struct xfs_attri_log_format);

	if ((f = ((struct xfs_attri_log_format *)malloc(dst_len))) == NULL) {
		fprintf(stderr, _("%s: xlog_recover_print_attri: malloc failed\n"),
			progname);
		exit(1);
	}
	if (xfs_attri_copy_log_format((char*)src_f, src_len, f))
		goto out;

	printf(_("ATTRI:  #regs: %d	name_len: %d, value_len: %d  id: 0x%llx\n"),
		f->alfi_size, f->alfi_name_len, f->alfi_value_len, (unsigned long long)f->alfi_id);

	if (f->alfi_name_len > 0) {
		region++;
		printf(_("ATTRI:  name len:%u\n"), f->alfi_name_len);
		print_or_dump((char *)item->ri_buf[region].i_addr,
			       f->alfi_name_len);
	}

	if (f->alfi_value_len > 0) {
		int len = f->alfi_value_len;

		if (len > MAX_ATTR_VAL_PRINT)
			len = MAX_ATTR_VAL_PRINT;

		region++;
		printf(_("ATTRI:  value len:%u\n"), f->alfi_value_len);
		print_or_dump((char *)item->ri_buf[region].i_addr, len);
	}

out:
	free(f);

}

int
xlog_print_trans_attrd(char **ptr, uint len)
{
	struct xfs_attrd_log_format *f;
	struct xfs_attrd_log_format lbuf;
	uint core_size = sizeof(struct xfs_attrd_log_format);

	memcpy(&lbuf, *ptr, MIN(core_size, len));
	f = &lbuf;
	*ptr += len;
	if (len >= core_size) {
		printf(_("ATTRD:  #regs: %d	id: 0x%llx\n"),
			f->alfd_size,
			(unsigned long long)f->alfd_alf_id);
		return 0;
	} else {
		printf(_("ATTRD: Not enough data to decode further\n"));
		return 1;
	}
}	/* xlog_print_trans_attrd */

void
xlog_recover_print_attrd(
	struct xlog_recover_item		*item)
{
	struct xfs_attrd_log_format	*f;

	f = (struct xfs_attrd_log_format *)item->ri_buf[0].i_addr;

	printf(_("	ATTRD:  #regs: %d	id: 0x%llx\n"),
		f->alfd_size,
		(unsigned long long)f->alfd_alf_id);
}
