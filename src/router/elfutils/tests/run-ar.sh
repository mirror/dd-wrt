#! /usr/bin/env bash
# Copyright (C) 2017 Red Hat, Inc.
# This file is part of elfutils.
#
# This file is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 3 of the License, or
# (at your option) any later version.
#
# elfutils is distributed in the hope that it will be useful, but
# WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.

. $srcdir/test-subr.sh

tempfiles objects.list test.ar

echo Make a sorted list of the just build src .o files.
(cd ${abs_top_builddir}/src; ls *.o | sort) > objects.list
cat objects.list

echo Create a new ar file with the .o files.
testrun ${abs_top_builddir}/src/ar -r test.ar \
	$(echo ${abs_top_builddir}/src/*.o | sort)

echo List the ar file contents.
testrun_compare ${abs_top_builddir}/src/ar -t test.ar < objects.list

echo Delete all objects again.
testrun ${abs_top_builddir}/src/ar -d test.ar $(cat objects.list)

echo Check new ar file is now empty
testrun_compare ${abs_top_builddir}/src/ar -t test.ar << EOF
EOF

tempfiles bin* dup* long* inner* outer*

echo Compile files for nested archives.

cat > dup.c <<'EOF'
int dup_func(void){return 1;}
EOF
cat > bin_outer.c <<'EOF'
int outer_func(void){return 1;}
EOF
cat > long_name_long_name_outer.c <<'EOF'
int long_name_long_name_func(void){return 1;}
EOF
cat > bin_inner1.c <<'EOF'
int inner1_func(void){return 1;}
EOF
cat > long_name_long_name_inner1.c <<'EOF'
int long_name_long_name_func(void){return 1;}
EOF
cat > bin_inner2.c <<'EOF'
int inner2_func(void){return 1;}
EOF
cat > long_name_long_name_inner2.c <<'EOF'
int long_name_long_name_func(void){return 1;}
EOF

# Compile the source files.
for src in *.c; do
  obj=$(echo "$src" | sed 's/\.c$/.o/')
  gcc -O0 -fno-pic -fno-pie -fno-plt -c "$src" -o "$obj"
done

echo Create nested archives.
testrun ${abs_top_builddir}/src/ar -r inner2.ar bin_inner2.o
testrun ${abs_top_builddir}/src/ar -r inner2.ar dup.o
testrun ${abs_top_builddir}/src/ar -r inner2.ar long_name_long_name_inner2.o

# inner1.ar contains inner2.ar
testrun ${abs_top_builddir}/src/ar -r inner1.ar inner2.ar
testrun ${abs_top_builddir}/src/ar -r inner1.ar bin_inner1.o
testrun ${abs_top_builddir}/src/ar -r inner1.ar dup.o
testrun ${abs_top_builddir}/src/ar -r inner1.ar long_name_long_name_inner1.o

# outer.ar contains inner1.ar
testrun ${abs_top_builddir}/src/ar -r outer.ar inner1.ar
testrun ${abs_top_builddir}/src/ar -r outer.ar bin_outer.o
testrun ${abs_top_builddir}/src/ar -r outer.ar dup.o
testrun ${abs_top_builddir}/src/ar -r outer.ar long_name_long_name_outer.o

echo Check symbol and header names of the nested archives.

testrun_compare ${abs_builddir}/ar-extract-ar outer.ar <<'EOF'
== symbol names ==
outer_func
dup_func
long_name_long_name_func

== headers ==
/ /               
// //              
inner1.ar inner1.ar/      
    == symbol names ==
    inner1_func
    dup_func
    long_name_long_name_func

    == headers ==
    / /               
    // //              
    inner2.ar inner2.ar/      
        == symbol names ==
        inner2_func
        dup_func
        long_name_long_name_func

        == headers ==
        / /               
        // //              
        bin_inner2.o bin_inner2.o/   
        dup.o dup.o/          
        long_name_long_name_inner2.o /0              

    bin_inner1.o bin_inner1.o/   
    dup.o dup.o/          
    long_name_long_name_inner1.o /0              

bin_outer.o bin_outer.o/    
dup.o dup.o/          
long_name_long_name_outer.o /0              

EOF

exit 0
