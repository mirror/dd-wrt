SRCS = aes.c checksum.c libzbitmap.c parameters.c sha256.c unicode.c
SRCS += $(wildcard zlib_inflate/*.c)
OBJS = $(SRCS:.c=.o)
DEPS = $(SRCS:.c=.d)

SPARSE_VERSION := $(shell sparse --version 2>/dev/null)

override CFLAGS += -Wall -fno-strict-aliasing -I$(CURDIR)/../include $(COPTS) $(MIPS16_OPT) -ffunction-sections -fdata-sections $(LTO)

libapfs.a: $(OBJS)
	@echo '  Assembling library archive...'
	$(AR) rcs $(LTOPLUGIN) $@ $^

%.o: %.c
	@echo '  Compiling $<...'
	$(CC) $(CFLAGS) -o $@ -MMD -MP -c $<
ifdef SPARSE_VERSION
	@sparse $(CFLAGS) $<
endif

-include $(DEPS)

clean:
	rm -f $(OBJS) $(DEPS) libapfs.a
