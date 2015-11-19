EXTRA_CFLAGS += -I$(srctree)/arch/arm/mach-qcom/include
obj-y			:= board.o
obj-$(CONFIG_SMP)	+= platsmp.o
obj-$(CONFIG_QCOM_SCM)	+= scm.o scm-boot.o
obj-$(CONFIG_MSM_PCIE)	+= pcie.o pcie_irq.o pcie_phy.o

CFLAGS_scm.o :=$(call as-instr,.arch_extension sec,-DREQUIRES_SEC=1)
