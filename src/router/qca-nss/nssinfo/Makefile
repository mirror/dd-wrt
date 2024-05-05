MKDIR = mkdir -p $(@D)
SRCPATH = src
OBJPATH = obj
SRCDIR = ./

BINARY = $(OBJPATH)/nssinfo
SOURCES = $(wildcard $(SRCDIR)/src/*.c)
HEADERS = $(wildcard $(SRCDIR)/include/*.h)
OBJECTS = $(SOURCES:$(SRCDIR)/src/%.c=$(OBJPATH)/%.o)

INCLUDE += -I../lib/include
EXTRA_CFLAGS=-Wall -UENABLE_DEBUG -I$(TOP)/libnl/include  -I$(TOP)/qca-nss/qca-nss-clients/exports $(KERNEL_FLAGS) $(MIPS16_OPT) -I$(TOP)/ncurses/include -I$(TOP)/qca-nss/qca-nss-clients/netlink/include -D_GNU_SOURCE -DNEED_PRINTF $(LTO)
LDFLAGS = $(LDLTO) -L$(TOP)/libnl/lib/.libs -L$(TOP)/ncurses/lib -lnl-nss -lnl-3 -lnl-genl-3 -lncurses
LDLIBS  = -L../lib/obj

all: release

release: $(BINARY)

$(OBJPATH)/%.o: $(SRCPATH)/%.c $(HEADERS)
	$(MKDIR)
	@echo [CC] $@
	$(CC) -c $(CFLAGS) $(EXTRA_CFLAGS) $(INCLUDE) -o $@ $<

$(BINARY): $(OBJECTS)
	@echo $(BINARY)
	@echo [LD] $@
	$(CC) -o $@ $^ $(LDFLAGS) $(LDLIBS)
clean:
	@echo [Clean]
	@rm -f $(OBJECTS)
	@rm -f $(BINARY)
	@rmdir $(OBJPATH)

.PHONY: clean
