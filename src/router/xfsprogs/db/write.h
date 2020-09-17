// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2000-2001,2005 Silicon Graphics, Inc.
 * All Rights Reserved.
 */

struct field;

extern void	write_init(void);
extern void	write_block(const field_t *fields, int argc, char **argv);
extern void	write_struct(const field_t *fields, int argc, char **argv);
extern void	write_string(const field_t *fields, int argc, char **argv);
