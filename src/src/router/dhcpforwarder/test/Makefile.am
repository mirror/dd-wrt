## $Id: Makefile.am,v 1.3 2002/08/29 15:40:16 ensc Exp $

# Copyright (C) 2002 Enrico Scholz <enrico.scholz@informatik.tu-chemnitz.de>
#  
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; version 2 of the License.
#  
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#  
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
#  



check_PROGRAMS		=  writeTest parseTest

writeTest_SOURCES	=  writeTest.c
writeTest_LDADD		=  ../src/output.o

parseTest_SOURCES	=  parseTest.c
parseTest_LDADD		=  ../src/output.o ../src/parser.o 

AM_CPPFLAGS		=  -I $(top_srcdir)/src

test_DTA		=  data/write.out				\
			   data/parse-test1.in data/parse-test1.out	\
			   data/parse-test2.in data/parse-test2.out

TESTS			=  cmpout.writeTest				\
			   cmpout.parseTest.test1 cmpout.parseTest.test2

CLEANFILES		=  cmpout.*

EXTRA_DIST		=  cmpout-script compile-test $(test_DTA)

cmpout.%:		cmpout-script
			-rm -f $@
			ln -s $< $@

include $(top_srcdir)/m4/diet.am
