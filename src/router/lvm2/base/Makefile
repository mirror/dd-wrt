# Copyright (C) 2018 Red Hat, Inc. All rights reserved.
#
# This file is part of the device-mapper userspace tools.
#
# This copyrighted material is made available to anyone wishing to use,
# modify, copy, or redistribute it subject to the terms and conditions
# of the GNU Lesser General Public License v.2.1.
#
# You should have received a copy of the GNU Lesser General Public License
# along with this program; if not, write to the Free Software Foundation,
# Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA

# Uncomment this to build the simple radix tree.  You'll need to make clean too.
# Comment to build the advanced radix tree.
#base/data-struct/radix-tree.o: CFLAGS += -DSIMPLE_RADIX_TREE

# NOTE: this Makefile only works as 'include' for toplevel Makefile
#       which defined all top_* variables

BASE_SOURCE=\
	base/data-struct/hash.c \
	base/data-struct/list.c \
	base/data-struct/radix-tree.c

BASE_TARGET = base/libbase.a
BASE_DEPENDS = $(BASE_SOURCE:%.c=%.d)
BASE_OBJECTS = $(BASE_SOURCE:%.c=%.o)
CLEAN_TARGETS += $(BASE_DEPENDS) $(BASE_OBJECTS) \
	$(BASE_SOURCE:%.c=%.gcda) \
	$(BASE_SOURCE:%.c=%.gcno) \
	$(BASE_TARGET)

$(BASE_TARGET): $(BASE_OBJECTS)
	@echo "    [AR] $@"
	$(Q) $(RM) $@
	$(Q) $(AR) rsv $@ $(BASE_OBJECTS) > /dev/null

ifeq ("$(DEPENDS)","yes")
-include $(BASE_DEPENDS)
endif
