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

ifndef CC
CC=gcc
endif

ifndef CFLAGS
CFLAGS = -MMD -O2 -Wall -g
endif

EXTRA_CFLAGS += -I$(PRJ_PATH)/cli/include $(QCA_NSS_MACSEC_INCLUDE)
EXTRA_CFLAGS += -Werror -Wall

ALL=fal.so

all:$(ALL)

FAL_OBJ :=  nss_macsec_fal_api.o

%.o:%.c
	$(CC) $(CFLAGS) $(EXTRA_CFLAGS) -c -o   $@ $< -fPIC

fal.so: $(FAL_OBJ)
	$(CC) $(LDFLAGS) -o libfal.so -shared -rdynamic -lpthread -fPIC $^
	#cp libfal.so $(IMAGE_PATH)/libsdk_fal.so

clean:
	rm -rf libfal* *.o *.d
