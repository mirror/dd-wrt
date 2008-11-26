# This flags will be used only by the Marvell arch files compilation.

# General definitions
CPU_ARCH    = ARM
CHIP        = 88FXX81
CHIP_DEFINE = 88F1181
VENDOR      = Marvell
ENDIAN      = LE

# Main directory structure
SRC_PATH	   = ..
BOARD_DIR          = $(SRC_PATH)/Board
COMMON_DIR         = $(SRC_PATH)/Common
SOC_DIR     	   = $(SRC_PATH)/Soc
OSSERVICES_DIR     = $(SRC_PATH)/osServices
LSP_DIR	           = $(SRC_PATH)/LSP

PLAT_DIR	   = $(SRC_PATH)/mv88f1181

# Board components
BOARD_ENV_DIR      = $(BOARD_DIR)/boardEnv
BOARD_ENV_PLAT_DIR = $(BOARD_DIR)/boardEnv/DB_$(CHIP)
BOARD_DRAM_DIR     = $(BOARD_DIR)/dram
BOARD_ETHPHY_DIR   = $(BOARD_DIR)/ethPhy
BOARD_FLASH_DIR    = $(BOARD_DIR)/flash
BOARD_PCI_DIR      = $(BOARD_DIR)/pci
BOARD_RTC_DIR      = $(BOARD_DIR)/rtc

# Controller components
SOC_CPU_DIR      = $(SOC_DIR)/cpu
SOC_CPU_PLAT_DIR = $(SOC_DIR)/cpu/MV_$(CHIP)
SOC_AHB_TO_MBUS_DIR = $(SOC_DIR)/ahbtombus
SOC_CNTMR_DIR     = $(SOC_DIR)/cntmr
SOC_CPUIF_DIR     = $(SOC_DIR)/cpuIf
SOC_ENV_DIR       = $(SOC_DIR)/ctrlEnv
SOC_ENV_CHIP_DIR  = $(SOC_ENV_DIR)/MV_$(CHIP)
SOC_DEVICE_DIR    = $(SOC_DIR)/device
SOC_DRAM_DIR      = $(SOC_DIR)/dram
SOC_DRAM_ARCH_DIR = $(SOC_DRAM_DIR)/Arch$(CPU_ARCH)
SOC_GPP_DIR       = $(SOC_DIR)/gpp
SOC_IDMA_DIR      = $(SOC_DIR)/idma
SOC_PCI_DIR       = $(SOC_DIR)/pci
SOC_PEX_DIR       = $(SOC_DIR)/pex
SOC_TWSI_DIR      = $(SOC_DIR)/twsi
SOC_TDM_DIR       = $(SOC_DIR)/tdm
SOC_TWSI_ARCH_DIR = $(SOC_TWSI_DIR)/Arch$(CPU_ARCH)
SOC_ETH_DIR       = $(SOC_DIR)/eth
SOC_UART_DIR      = $(SOC_DIR)/uart
SOC_PCIIF_DIR     = $(SOC_DIR)/pciIf

# OS services
OSSERV_LINUX       = $(OSSERVICES_DIR)/linux
OSSERV_ARCH_DIR    = $(OSSERVICES_DIR)/linux/Arch$(CPU_ARCH)

# Internal definitions
MV_DEFINE = -DMV_LINUX -DMV_CPU_$(ENDIAN) -DMV_$(CHIP_DEFINE) -DMV_$(CPU_ARCH)

# Internal include path
SRC_PATH_I	   = $(TOPDIR)/arch/arm/mach-mv88fxx81/mv88f1181
BOARD_PATH         = -I$(SRC_PATH_I)/$(BOARD_DIR) -I$(SRC_PATH_I)/$(BOARD_ENV_DIR) \
                     -I$(SRC_PATH_I)/$(BOARD_DRAM_DIR) -I$(SRC_PATH_I)/$(BOARD_ETHPHY_DIR) -I$(SRC_PATH_I)/$(BOARD_FLASH_DIR) \
                     -I$(SRC_PATH_I)/$(BOARD_PCI_DIR) -I$(SRC_PATH_I)/$(BOARD_RTC_DIR) -I$(SRC_PATH_I)/$(BOARD_ENV_PLAT_DIR)
                     
COMMON_PATH        = -I$(SRC_PATH_I)/$(COMMON_DIR) 
SOC_PATH    	   = -I$(SRC_PATH_I)/$(SOC_DIR) -I$(SRC_PATH_I)/$(SOC_UART_DIR) -I$(SRC_PATH_I)/$(SOC_CNTMR_DIR)     \
                     -I$(SRC_PATH_I)/$(SOC_CPUIF_DIR) -I$(SRC_PATH_I)/$(SOC_ENV_DIR) -I$(SRC_PATH_I)/$(SOC_DEVICE_DIR)       \
                     -I$(SRC_PATH_I)/$(SOC_DRAM_DIR) -I$(SRC_PATH_I)/$(SOC_DRAM_ARCH_DIR) -I$(SRC_PATH_I)/$(SOC_GPP_DIR)   \
                     -I$(SRC_PATH_I)/$(SOC_IDMA_DIR) -I$(SRC_PATH_I)/$(SOC_PCI_DIR) -I$(SRC_PATH_I)/$(SOC_ENV_CHIP_DIR)    \
                     -I$(SRC_PATH_I)/$(SOC_ETH_DIR) -I$(SRC_PATH_I)/$(SOC_TWSI_DIR) -I$(SRC_PATH_I)/$(SOC_AHB_TO_MBUS_DIR) \
		     -I$(SRC_PATH_I)/$(SOC_CPU_DIR) -I$(SRC_PATH_I)/$(SOC_CPU_PLAT_DIR) -I$(SRC_PATH_I)/$(SOC_TDM_DIR) 				   \
                     -I$(SRC_PATH_I)/$(SOC_TWSI_ARCH_DIR) -I$(SRC_PATH_I)/$(SOC_PEX_DIR) -I$(SRC_PATH_I)/$(SOC_PCIIF_DIR)
OSSERVICES_PATH    = -I$(SRC_PATH_I)/$(OSSERVICES_DIR) -I$(SRC_PATH_I)/$(OSSERV_LINUX) -I$(SRC_PATH_I)/$(OSSERV_ARCH_DIR) 
LSP_PATH           = -I$(SRC_PATH_I)/$(LSP_DIR)
PLAT_PATH	   = -I$(SRC_PATH_I)/$(PLAT_DIR)

EXTRA_INCLUDE  = $(MV_DEFINE) $(OSSERVICES_PATH) $(BOARD_PATH) $(COMMON_PATH) $(SOC_PATH)  \
            	 $(LSP_PATH) $(PLAT_PATH)

EXTRA_CFLAGS += $(EXTRA_INCLUDE) $(EXTRA_MV_DEFINE)

