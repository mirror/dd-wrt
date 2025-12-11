#! /bin/sh
# Copyright (C) 2025 Mark J. Wielaard <mark@klomp.org>
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

# Using GCC 15.0.1 20250310 (experimental)
#
# echo "void foo () { }" | g++ -g -c -x c++ -std=c++20 - -o cpp.o
# echo "int main () { }" | gcc -g -c -x c -std=c23 - -o c.o
# gcc -g  -o testfile-lnames cpp.o c.o
#
# Note this version outputs an older DW_AT_language you need to
# see the DW_AT_language_version to see the real std used.

testfiles testfile-lnames

testrun_compare ${abs_top_builddir}/src/readelf --debug-dump=info testfile-lnames <<\EOF

DWARF section [28] '.debug_info' at offset 0x3314:
 [Offset]
 Compilation unit at offset 0:
 Version: 5, Abbreviation section offset: 0, Address size: 8, Offset size: 4
 Unit type: compile (1)
 [     c]  compile_unit         abbrev: 1
           producer             (strp) "GNU C++20 15.0.1 20250310 (experimental) -mtune=generic -march=x86-64 -g -std=c++20"
           language             (data1) C_plus_plus_14 (33)
           language_name        (data1) C_plus_plus (4)
           language_version     (data4) 202002
           name                 (line_strp) "<stdin>"
           comp_dir             (line_strp) "/tmp"
           low_pc               (addr) 0x0000000000401106 <_Z3foov>
           high_pc              (data8) 7 (0x000000000040110d <main>)
           stmt_list            (sec_offset) 0
 [    33]    subprogram           abbrev: 2
             external             (flag_present) yes
             name                 (string) "foo"
             decl_file            (data1) <stdin> (1)
             decl_line            (data1) 1
             decl_column          (data1) 6
             linkage_name         (strp) "_Z3foov"
             low_pc               (addr) 0x0000000000401106 <_Z3foov>
             high_pc              (data8) 7 (0x000000000040110d <main>)
             frame_base           (exprloc) 
              [ 0] call_frame_cfa
             call_all_calls       (flag_present) yes
 Compilation unit at offset 82:
 Version: 5, Abbreviation section offset: 51, Address size: 8, Offset size: 4
 Unit type: compile (1)
 [    5e]  compile_unit         abbrev: 1
           producer             (strp) "GNU C23 15.0.1 20250310 (experimental) -mtune=generic -march=x86-64 -g -std=c23"
           language             (data1) C11 (29)
           language_name        (data1) C (3)
           language_version     (data4) 202311
           name                 (line_strp) "<stdin>"
           comp_dir             (line_strp) "/tmp"
           low_pc               (addr) 0x000000000040110d <main>
           high_pc              (data8) 11 (0x0000000000401118 <_fini>)
           stmt_list            (sec_offset) 76
 [    85]    subprogram           abbrev: 2
             external             (flag_present) yes
             name                 (strp) "main"
             decl_file            (data1) <stdin> (1)
             decl_line            (data1) 1
             decl_column          (data1) 5
             prototyped           (flag_present) yes
             type                 (ref4) [    a3]
             low_pc               (addr) 0x000000000040110d <main>
             high_pc              (data8) 11 (0x0000000000401118 <_fini>)
             frame_base           (exprloc) 
              [ 0] call_frame_cfa
             call_all_calls       (flag_present) yes
 [    a3]    base_type            abbrev: 3
             byte_size            (data1) 4
             encoding             (data1) signed (5)
             name                 (string) "int"
EOF

exit 0
