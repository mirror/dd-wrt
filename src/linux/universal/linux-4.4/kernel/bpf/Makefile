obj-y := core.o
CFLAGS_core.o += $(call cc-disable-warning, override-init)

obj-$(CONFIG_BPF_SYSCALL) += syscall.o verifier.o inode.o helpers.o
obj-$(CONFIG_BPF_SYSCALL) += hashtab.o arraymap.o

# various version of gcc have a LTO bug where the &&labels used in the
# BPF interpreter can cause linker errors when spread incorrectly over
# partitions. Disable LTO for BPF for now
CFLAGS_core.o += $(DISABLE_LTO)
CFLAGS_syscall.o = $(DISABLE_LTO)
