MKDIR = @mkdir -p $(@D)
SRCPATH = ./
OBJPATH = obj

BINARY = $(OBJPATH)/libnl-nss.a
SOURCES = $(wildcard $(SRCPATH)/*.c)
OBJECTS = $(SOURCES:$(SRCPATH)/%.c=$(OBJPATH)/%.o)
HEADERS = $(wildcard $(SRCPATH)/*.h)

INCLUDE += -I./include
EXTRA_CFLAGS = -Wall -Werror -fPIC -Wl,-z,relro -Wl,-z,now -UENABLE_DEBUG -I$(TOP)/libnl/include  -I$(TOP)/qca-nss/qca-nss-clients/exports $(KERNEL_FLAGS) $(MIPS16_OPT) -I$(TOP)/ncurses/include -I$(TOP)/qca-nss/qca-nss-clients/netlink/include -D_GNU_SOURCE -DNEED_PRINTF $(LTO)
EXTRA_LDFLAGS = -pie -Wl,-z,relro -Wl,-z,now

all: release

release: $(BINARY)

$(OBJPATH)/%.o: $(SRCPATH)/%.c $(HEADERS)
	$(MKDIR)
	@echo [CC] $@
	@$(CC) -c $(CFLAGS) $(INCLUDE) $(EXTRA_CFLAGS) -o $@ $<

$(BINARY): $(OBJECTS)
	@echo $(BINARY)
	@echo [LD] $@
	$(AR) cr $@ $^
#	@$(CC) -shared -o $@ $^ $(LDFLAGS) $(LDLIBS)

clean:
	@echo [Clean]
	@rm -f $(OBJECTS)
	@rm -f $(BINARY)
	@rm -rf $(OBJPATH)

.PHONY: clean
