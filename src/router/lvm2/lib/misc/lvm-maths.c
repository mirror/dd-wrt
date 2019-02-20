/*
 * Copyright (C) 2016 Red Hat, Inc. All rights reserved.
 *
 * This file is part of LVM2.
 *
 * This copyrighted material is made available to anyone wishing to use,
 * modify, copy, or redistribute it subject to the terms and conditions
 * of the GNU Lesser General Public License v.2.1.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

#include "lib/misc/lib.h"

/* Greatest common divisor */
unsigned long gcd(unsigned long n1, unsigned long n2)
{
	unsigned long remainder;

	do {
		remainder = n1 % n2;
		n1 = n2;
		n2 = remainder;
	} while (n2);

	return n1;
}

/* Least common multiple */
unsigned long lcm(unsigned long n1, unsigned long n2)
{
	if (!n1 || !n2)
		return 0;

	return (n1 * n2) / gcd(n1, n2);
}
