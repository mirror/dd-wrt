
# Copyright (C) 2018-2023 Ruilin Peng (Nick) <pymumu@gmail.com>.
#
# smartdns is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# smartdns is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.

BIN=smartdns 
OBJS_LIB=$(patsubst %.c,%.o,$(wildcard lib/*.c))
OBJS_MAIN=$(patsubst %.c,%.o,$(wildcard *.c))
OBJS=$(OBJS_MAIN) $(OBJS_LIB)

# cflags
ifndef CFLAGS
CFLAGS =-O2 -g -Wall -Wstrict-prototypes -fno-omit-frame-pointer -Wstrict-aliasing -funwind-tables -Wmissing-prototypes -Wshadow -Wextra -Wno-unused-parameter -Wno-implicit-fallthrough
endif
override CFLAGS +=-Iinclude $(LTO)
override CFLAGS += -DBASE_FILE_NAME='"$(notdir $<)"'
ifdef VER
override CFLAGS += -DSMARTDNS_VERION='"$(VER)"'
endif

CXXFLAGS=-O2 -g -Wall -std=c++11 
override CXXFLAGS +=-Iinclude

# ldflags
override LDFLAGS += $(LDLTO)
ifeq ($(STATIC), yes)
override LDFLAGS += -Wl,--whole-archive -lpthread -Wl,--no-whole-archive -ldl -static
else
override LDFLAGS += -lpthread 
endif
ifeq ($(SSL), yes)
override LDFLAGS += -lssl -lcrypto
override CFLAGS += -DHAVE_OPENSSL -DNEED_PRINTF
endif


.PHONY: all clean

all: $(BIN)
 
$(BIN) : $(OBJS)
	$(CC) $(OBJS) -o $@ $(LDFLAGS)

clang-tidy:
	clang-tidy -p=. $(OBJS_MAIN:.o=.c) -- $(CFLAGS)

clean:
	$(RM) $(OBJS) $(BIN)
