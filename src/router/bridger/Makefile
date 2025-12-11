CFLAGS	+= -Wall -Wno-unknown-warning-option -Wno-array-bounds -Wno-format-truncation -Werror -Wno-error=deprecated-declarations --std=gnu99 -DUBUS_SUPPORT -DNEED_PRINTF
CFLAGS	+= $(COPTS) -I$(TOP)/libubox -I$(TOP)/ubus -I$(TOP)/_staging/usr/include -I$(TOP)/libbpf/src -I$(TOP)/libnl-tiny/include -I$(TOP)/kernel_headers/$(KERNELRELEASE)/include
LDFLAGS += -L$(TOP)/libbpf/src -lbpf -L$(TOP)/libnl-tiny -lnl-tiny -L$(TOP)/elfutils/libelf -lelf -L$(TOP)/zlib -lz -L$(TOP)/libubox -lubox -L$(TOP)/ubus -lubus

BPF_KARCH:=mips
BPF_ARCH:=mipsel
BPF_TARGET:=bpfel

BPF_HEADERS_DIR := $(TOP)/bpf-headers

BPF_KERNEL_INCLUDE := \
	-nostdinc -isystem $(TOOLCHAIN_ROOT_DIR)/lib/gcc/*/*/include \
	$(patsubst %,-isystem%,$(TOOLCHAIN_INC_DIRS)) \
	-I$(BPF_HEADERS_DIR)/arch/$(BPF_KARCH)/include \
	-I$(BPF_HEADERS_DIR)/arch/$(BPF_KARCH)/include/asm/mach-generic \
	-I$(BPF_HEADERS_DIR)/arch/$(BPF_KARCH)/include/generated \
	-I$(BPF_HEADERS_DIR)/include \
	-I$(BPF_HEADERS_DIR)/arch/$(BPF_KARCH)/include/uapi \
	-I$(BPF_HEADERS_DIR)/arch/$(BPF_KARCH)/include/generated/uapi \
	-I$(BPF_HEADERS_DIR)/include/uapi \
	-I$(BPF_HEADERS_DIR)/include/generated/uapi \
	-I$(BPF_HEADERS_DIR)/tools/lib \
	-I$(BPF_HEADERS_DIR)/tools/testing/selftests \
	-I$(BPF_HEADERS_DIR)/samples/bpf \
	-include linux/kconfig.h -include asm_goto_workaround.h \
	-include $(BPF_HEADERS_DIR)/include/linux/compiler_attributes.h

BPF_CFLAGS := \
	$(BPF_KERNEL_INCLUDE) -I$(PKG_BUILD_DIR) \
	-D__KERNEL__ -D__BPF_TRACING__ -DCONFIG_GENERIC_CSUM \
	-D__TARGET_ARCH_${BPF_KARCH} \
	-mlittle-endian \
	-fno-stack-protector -Wall \
	-Wno-unused-value -Wno-pointer-sign \
	-Wno-compare-distinct-pointer-types \
	-Wno-gnu-variable-sized-type-not-at-end \
	-Wno-address-of-packed-member -Wno-tautological-compare \
	-Wno-unknown-warning-option \
	-fno-asynchronous-unwind-tables \
	-Wno-uninitialized -Wno-unused-variable \
	-Wno-unused-label \
	-O2 -emit-llvm -Xclang -disable-llvm-passes

BPF_KARCH:=mips
BPF_ARCH:=mipsel
BPF_TARGET:=bpfel

OBJS := main.o nl.o device.o fdb.o bpf.o flow.o ubus.o

all: bridger bridger-bpf.o

clean:
	rm -f *.o bridger

install: all
	install -d $(INSTALLDIR)/usr/sbin
	install -d $(INSTALLDIR)/lib/bpf
	install -D bridger $(INSTALLDIR)/usr/sbin/bridger
	install -D bridger-bpf.o $(INSTALLDIR)/lib/bpf/bridger-bpf.o

bridger: $(OBJS)
	$(CC) -o $@ $^ $(LDFLAGS)

bridger-bpf.o:
	clang -g -target $(BPF_ARCH)-linux-gnu $(BPF_CFLAGS) -c bridger-bpf.c -o $(patsubst %.c,%.bc,bridger-bpf.c)
	opt -O2 -mtriple=$(BPF_TARGET) < $(patsubst %.c,%.bc,bridger-bpf.c) > $(patsubst %.c,%.opt,bridger-bpf.c)
	llvm-dis < $(patsubst %.c,%.opt,bridger-bpf.c) > $(patsubst %.c,%.S,bridger-bpf.c)
	llc -march=$(BPF_TARGET) -mcpu=v3 -filetype=obj -o $(patsubst %.c,%.o,bridger-bpf.c) < $(patsubst %.c,%.S,bridger-bpf.c)
	cp $(patsubst %.c,%.o,bridger-bpf.c) $(patsubst %.c,%.debug.o,bridger-bpf.c)
	llvm-strip --strip-debug $(patsubst %.c,%.o,bridger-bpf.c)
