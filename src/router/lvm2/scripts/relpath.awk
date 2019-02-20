#!/usr/bin/awk -f
#
# Copyright (C) 2010 Red Hat, Inc. All rights reserved.
#
# This file is part of LVM2.
#
# This copyrighted material is made available to anyone wishing to use,
# modify, copy, or redistribute it subject to the terms and conditions
# of the GNU General Public License v.2.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software Foundation,
# Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA

# relpath.awk: Script is used to calculate relative path
#	       between two real absolute paths.
#
# echo /a/b/c/d  /a/b/e/f | relpath.awk
# -> ../../e/f/

{
	length_from = split($1, from, "/");
	length_to = split($2, to, "/") ;
	l = 1;
	while (l <= length_from && l <= length_to && from[l] == to[l])
		l++;
	for (i = l; i <= length_from && length(from[i]); i++) {
		if (i > l)
			p = sprintf("%s/", p);
		p = sprintf("%s..", p);
	}
	for (i = l; i <= length_to && length(to[i]); i++) {
		if (length(p) > 0)
			p = sprintf("%s/", p);
		p = sprintf("%s%s", p, to[i]);
	}
	if (length(p))
		p = sprintf("%s/", p);
	print p
}
