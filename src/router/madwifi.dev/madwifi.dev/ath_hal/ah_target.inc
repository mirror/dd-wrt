# Determine HAL target based on the kernel architecture

# Make sure ARCH is defined
ifeq ($(ARCH),)
$(error ARCH must be defined)
endif

# Determine endianess.  Note that it's not indicated for some CPUs at
# all, so this value is only valid for certain processors.
ifeq ($(filter-out arm%,$(ARCH)),)
ENDIAN = le
endif

ifeq ($(filter-out mips%,$(ARCH)),)
ENDIAN = be
endif

ifneq ($(CONFIG_CPU_BIG_ENDIAN)$(CONFIG_BIG_ENDIAN),)
ENDIAN = be
endif

ifneq ($(CONFIG_CPU_LITTLE_ENDIAN)$(CONFIG_LITTLE_ENDIAN),)
ENDIAN = le
endif

# Determine the target (i.e. which HAL to use).
# The default is $(ARCH)-elf
TARGET-$(CONFIG_CPU_32v4) = armv4-$(ENDIAN)-elf
TARGET-$(CONFIG_CPU_MIPS32_R1) = mips1-$(ENDIAN)-elf
TARGET-$(CONFIG_CPU_MIPS32_R2) = mips-$(ENDIAN)-elf
TARGET-$(CONFIG_CPU_R4X00) = mipsisa32-$(ENDIAN)-elf
TARGET-$(CONFIG_CPU_TX49XX) = mipsisa32-$(ENDIAN)-elf
TARGET-$(CONFIG_PPC32) = powerpc-be-elf
TARGET-$(CONFIG_CPU_SH4) = sh4-le-elf
TARGET-$(CONFIG_SPARC32) = sparc-be-elf
TARGET-$(CONFIG_SPARC64) = sparc64-be-elf
TARGET-$(CONFIG_CPU_SA110) = xscale-$(ENDIAN)-elf
TARGET-$(CONFIG_CPU_SA1100) = xscale-$(ENDIAN)-elf
TARGET-$(CONFIG_CPU_XSCALE) = xscale-$(ENDIAN)-elf

TARGET = $(if $(TARGET-y),$(TARGET-y),$(ARCH)-elf)
