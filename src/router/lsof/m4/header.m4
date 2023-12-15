# header.m4 -- Generate headers from kernel source

# Copyright 2002 Purdue Research Foundation, West Lafayette,
# Indiana 47907.  All rights reserved.
# 
# Written by Victor A. Abell
# 
# This software is not subject to any license of the American
# Telephone and Telegraph Company or the Regents of the
# University of California.
# 
# Permission is granted to anyone to use this software for
# any purpose on any computer system, and to alter it and
# redistribute it freely, subject to the following
# restrictions:
# 
# 1. Neither the authors nor Purdue University are responsible
#    for any consequences of the use of this software.
# 
# 2. The origin of this software must not be misrepresented,
#    either by explicit claim or by omission.  Credit to the
#    authors and Purdue University must appear in documentation
#    and sources.
# 
# 3. Altered versions must be plainly marked as such, and must
#    not be misrepresented as being the original software.
# 
# 4. This notice may not be removed or altered.

# HEADER_GENERATE([HEADER_NAME, KERNEL_SOURCE, PATTERN_BEGIN, PATTERN_END, MACRO])
# Create HEADER_NAME header from KERNEL_SOURCE,
# copying the code between lines matching PATTERN_BEGIN and PATTERN_END.
# The header is guarded from including twice by defining MACRO.
AC_DEFUN([HEADER_GENERATE], [
	# Generate HEADER_NAME($1) from KERNEL_SOURCE($2)
	rm -rf $1
	AS_IF([test -r $2], [
		# Find the line number of PATTERN_BEGIN($3)
		# Note extra quoting [[]] for M4
		LSOF_TMP1=$(grep -n $3 $2 | sed 's/\([[0-9]]*\):.*$/\1/')
		AS_IF([test "X$LSOF_TMP1" != "X"], [
			LSOF_TMP2=0
			# Find the end of PATTERN_END($4)
			for i in $(grep -n $4 $2 | sed 's/\([[0-9]]*\):.*/\1/') # {
			do
				AS_IF([test $LSOF_TMP2 -eq 0 -a $i -gt $LSOF_TMP1], [
					LSOF_TMP2=$i
				])
			done
			AS_IF([test $LSOF_TMP2 -eq 0], [
				LSOF_TMP1=""
			], [
				cat > $1 << EOF
/*
 * $1 -- created by lsof configure script on
EOF
				printf " * " >> $1
				date >> $1
				cat >> $1 << EOF
 */

#if	!defined($5)
#define	$5

EOF
				ed -s $2 >> $1 << EOF
${LSOF_TMP1},${LSOF_TMP2}p
EOF
				AS_IF([test $? -ne 0], [
					AC_MSG_ERROR([can't extract source from $2])
				], [
					cat >> $1 << EOF

#endif	/* defined($5) */
EOF
				])
			])
		])
	], [
		AC_MSG_ERROR([can't read $2])
	])
]))
