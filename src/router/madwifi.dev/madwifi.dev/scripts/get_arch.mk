# Calculate ARCH based on the included Linux .config file.  This should
# cover all architectures supported by Linux 2.4 and 2.6 whether they
# are supported by MadWifi or not.  Note that more specific entries must
# follow less specific ones, e.g. CONFIG_X86_64 overrides CONFIG_X86.

ARCH-$(CONFIG_ALPHA) = alpha
ARCH-$(CONFIG_ARM) = arm
ARCH-$(CONFIG_ARM26) = arm26
ARCH-$(CONFIG_CRIS) = cris
ARCH-$(CONFIG_FRV) = frv
ARCH-$(CONFIG_H8300) = h8300
ARCH-$(CONFIG_X86) = i386
ARCH-$(CONFIG_IA64) = ia64
ARCH-$(CONFIG_M32R) = m32r
ARCH-$(CONFIG_M68K) = m68k
ARCH-$(CONFIG_M68KNOMMU) = m68knommu
ARCH-$(CONFIG_MIPS) = mips
ARCH-$(CONFIG_MIPS64) = mips64
ARCH-$(CONFIG_PARISC) = parisc
ARCH-$(CONFIG_PPC) = ppc
ARCH-$(CONFIG_PPC64) = ppc64
ARCH-$(CONFIG_PPC_MERGE) = powerpc
ARCH-$(CONFIG_ARCH-S390) = s390
ARCH-$(CONFIG_ARCH-S390X) = s390x
ARCH-$(CONFIG_SUPERH) = sh
ARCH-$(CONFIG_CPU_SH5) = sh64
ARCH-$(CONFIG_SUPERH64) = sh64
ARCH-$(CONFIG_SPARC32) = sparc
ARCH-$(CONFIG_SPARC64) = sparc64
ARCH-$(CONFIG_UML) = um
ARCH-$(CONFIG_V850) = v850
ARCH-$(CONFIG_X86_64) = x86_64
ARCH-$(CONFIG_XTENSA) = xtensa

ifeq (,$(ARCH-y))
$(Cannot determine ARCH)
endif

# Don't allow ARCH to be overridden by a different value.
ifeq (,$(ARCH))
ARCH = $(ARCH-y)
else
ifneq ($(ARCH),$(ARCH-y))
$(error ARCH mismatch: supplied "$(ARCH)", determined "$(ARCH-y)")
endif
endif
