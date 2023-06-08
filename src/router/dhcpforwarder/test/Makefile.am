# Copyright (C) 2002, 2008, 2014
#               Enrico Scholz <enrico.scholz@ensc.de>
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; version 3 of the License.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program. If not, see http://www.gnu.org/licenses/.

check_PROGRAMS		=  writeTest parseTest modifyTest ipCRC

writeTest_SOURCES	=  writeTest.c ../src/output.c

parseTest_SOURCES	=  parseTest.c ../src/output.c ../src/parser.c

modifyTest_SOURCES	=  modifyTest.c \
			   ../src/output.c ../src/parser.c \
                           ../src/cfg.c ../src/recvfromflags.c

ipCRC_SOURCES		=  ipCRC.c \
			   ../src/output.c ../src/parser.c \
                           ../src/cfg.c ../src/recvfromflags.c

GCOV_FLAGS		=  -fprofile-arcs -ftest-coverage

AM_CPPFLAGS		=  -I $(top_srcdir)/src -DCFG_FILENAME='"$(cfg_filename)"'
AM_CFLAGS		=  $(GCOV_FLAGS)

test_DTA		=  data/write.out				\
			   data/parse-test1.in data/parse-test1.out	\
			   data/parse-test2.in data/parse-test2.out

TESTS			=  cmpout.writeTest				\
			   cmpout.parseTest.test1 cmpout.parseTest.test2 \
			   modifyTest ipCRC

CLEANFILES		=  cmpout.*

EXTRA_DIST		=  cmpout-script compile-test $(test_DTA)

cmpout.%:		cmpout-script
			-rm -f $@
			ln -s $< $@

include $(top_srcdir)/m4/diet.am
