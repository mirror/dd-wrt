# Copyright (c) 2014, 2019, 2021 The Linux Foundation. All rights reserved.
#
# Permission to use, copy, modify, and/or distribute this software for any
# purpose with or without fee is hereby granted, provided that the above
# copyright notice and this permission notice appear in all copies.
#
#
# THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
# WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY
# AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT
# INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM
# LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE
# OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
# PERFORMANCE OF THIS SOFTWARE.

ifndef CFLAGS
CFLAGS = -MMD -O2 -Wall -g
endif



EXTRA_CFLAGS += -I$(PRJ_PATH)/cli/include/ -I$(PRJ_PATH)/cli/include/dal $(QCA_NSS_MACSEC_INCLUDE)
EXTRA_CFLAGS += -Werror -Wall

%.o:%.c
	$(CC) $(CFLAGS) $(EXTRA_CFLAGS) -c -o $@ $<

CLI_US_OBJ := shell_main.o \
           shell_entity.o \
           cli.o \
	   array.o \
	   cli_cmd.o \
	   cli_cmd_basic.o \
	   cli_lib.o \
	   vty.o \
	   fal/cli_cmd_fal.o


all : $(CLI_US_OBJ)
	$(CC) $(LDFLAGS) -o cli   -lpthread  -L$(PRJ_PATH)/lib/fal -fPIC $^  -lfal
	#cp cli $(IMAGE_PATH)/clish
	-rm -rf *.o  *.d

clean :
	-rm -rf *.o *.d fal/*.o fal/*.d fal/.cli*
