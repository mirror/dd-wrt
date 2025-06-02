
# Copyright (C) 2018-2025 Ruilin Peng (Nick) <pymumu@gmail.com>.
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
SMARTDNS_LIB=libsmartdns.a
SMARTDNS_TEST_LIB=libsmartdns-test.a
# libs without main.o
OBJS=$(filter-out main.o, $(patsubst %.c,%.o,$(wildcard *.c)) $(patsubst %.c,%.o,$(wildcard */*.c)))
# libs without lib/%.o
LINT_OBJS=$(filter-out lib/%.o, $(OBJS))
TEST_OBJS=$(patsubst %.o,%_test.o,$(OBJS))
MAIN_OBJ = main.o


# cflags
ifndef CFLAGS
CFLAGS =-O2 -g -Wall -Wstrict-prototypes -fno-omit-frame-pointer -Wstrict-aliasing -funwind-tables -Wmissing-prototypes -Wshadow -Wextra -Wno-unused-parameter -Wno-implicit-fallthrough
endif
override CFLAGS += -Iinclude $(LTO)
override CFLAGS += -DBASE_FILE_NAME='"$(notdir $<)"'
ifdef VER
override CFLAGS += -DSMARTDNS_VERION='"$(VER)"'
endif

HASH := \#
HAS_UNWIND := $(shell printf '$(HASH)include <unwind.h>\nvoid main() { _Unwind_Backtrace(0, 0);}' | $(CC) -x c - -o /dev/null >/dev/null 2>&1 && echo -n 1 || echo -n 0)
ifeq ($(HAS_UNWIND), 1)
override CFLAGS += -DHAVE_UNWIND_BACKTRACE
endif

CXXFLAGS+= -Wall -std=c++11 
override CXXFLAGS +=-Iinclude

USE_ATOMIC := $(shell printf '$(HASH)include <stdint.h>\nvoid main() { void *ptr; uint64_t value=0;__atomic_fetch_add_8(ptr, &value, 1);}' | $(CC) -x c - -o /dev/null >/dev/null 2>&1 && echo -n 1 || echo -n 0)
ifeq ($(USE_ATOMIC), 0)
 override CFLAGS += -DUSE_ATOMIC
 override LDFLAGS += -latomic 
endif

# ldflags
override LDFLAGS += $(LDLTO)
ifeq ($(STATIC), yes)
override CFLAGS += -DBUILD_STATIC
override LDFLAGS += -Wl,--whole-archive -lpthread -Wl,--no-whole-archive -ldl -lm -static -rdynamic
else
override LDFLAGS += -lpthread 
endif
ifeq ($(SSL), yes)
override LDFLAGS += -lssl -lcrypto -rdynamic
override CFLAGS += -DHAVE_OPENSSL -DNEED_PRINTF
endif
ifeq ($(WOLFSSL), yes)
override LDFLAGS += -lwolfssl -rdynamic
override CFLAGS += -DHAVE_OPENSSL -DHAVE_WOLFSSL -DOPENSSL_ALL -DNEED_PRINTF
endif


.PHONY: all clean

all: $(BIN)
 
%_test.o: %.c
	$(CC) -DTEST $(CFLAGS) -c $< -o $@

$(BIN) : $(MAIN_OBJ) $(SMARTDNS_LIB)
	$(CC) $^ -o $@ $(LDFLAGS)

$(SMARTDNS_TEST_LIB): $(TEST_OBJS)
	$(AR) $(LTOPLUGIN) rcs $@ $^

$(SMARTDNS_LIB): $(OBJS)
	$(AR) $(LTOPLUGIN) rcs $@ $^

clang-tidy:
	clang-tidy -p=. $(LINT_OBJS:.o=.c) -- $(CFLAGS)

clean:
	$(RM) $(OBJS) $(BIN) $(SMARTDNS_LIB) $(MAIN_OBJ) $(SMARTDNS_TEST_LIB) $(TEST_OBJS)
