#define CFG_MAX_FLASH_SECT		1024	/* max number of sectors on one chip */

#define CFG_FLASH_BASE			0xbf000000

#define ATH_SPI_CMD_SECTOR_ERASE	0xd8
#define CFG_DEFAULT_FLASH_SECTOR_SIZE	(64*1024)
#define CFG_DEFAULT_FLASH_SIZE		0x01000000	/* Total flash size */
#define FLASH_M25P64    0x00F2

#define FLASH_SR3_HOLD_RESET_OFS 	7
#define FLASH_SR2_QE_OFS 		1

#define ATH_SPI_PAGE_SIZE	256

/* Basic SPI FLASH commands */
#define SPI_FLASH_CMD_WRSR		0x01
#define SPI_FLASH_CMD_PP		0x02
#define SPI_FLASH_CMD_READ		0x03
#define SPI_FLASH_CMD_WRDI		0x04
#define SPI_FLASH_CMD_RDSR		0x05
#define SPI_FLASH_CMD_WREN		0x06
/* 4-byte commands */
#define SPI_FLASH_CMD_4READ		0x13
#define SPI_FLASH_CMD_4PP		0x12
#define SPI_FLASH_CMD_4SE		0xDC

#define SPI_FLASH_CMD_EN4B		0xB7
#define SPI_FLASH_CMD_EX4B		0xE9

/* SPI FLASH erase related commands */
#define SPI_FLASH_CMD_ES_4KB	0x20
#define SPI_FLASH_CMD_ES_32KB	0x52
#define SPI_FLASH_CMD_ES_64KB	0xD8
#define SPI_FLASH_CMD_ES_ALL	0xC7

/* Other SPI FLASH commands */
#define SPI_FLASH_CMD_JEDEC		0x9F
#define SPI_FLASH_CMD_SFDP		0x5A

/* SFDP related defines */
#define SPI_FLASH_SFDP_SIGN		0x50444653

/* Register access commands */
#define SPI_FLASH_CMD_RSR1		0x5
#define SPI_FLASH_CMD_WSR1		0x1
#define SPI_FLASH_CMD_RSR2		0x35
#define SPI_FLASH_CMD_WSR2		0x31
#define SPI_FLASH_CMD_RSR3		0x15
#define SPI_FLASH_CMD_WSR3		0x11
#define SPI_FLASH_CMD_REAR		0xC8
#define SPI_FLASH_CMD_WEAR		0xC5

#define MXIC_ENSO            0xb1
#define MXIC_EXSO            0xc1

#define MXIC_JEDEC_ID        0xc2
#define ATMEL_JEDEC_ID        0x1f
#define SST_JEDEC_ID        0x20
#define INTEL_JEDEC_ID        0x89
#define WINB_JEDEC_ID        0xef

#define qca_soc_reg_read(_addr)			*(volatile unsigned int *)(KSEG1ADDR(_addr))
#define qca_soc_reg_write(_addr, _val)	((*(volatile unsigned int *)KSEG1ADDR(_addr)) = (_val))


#define qca_soc_reg_write_flush(_addr, _val) do {     \
         qca_soc_reg_write(_addr, _val);     \
         qca_soc_reg_read(_addr);       \
}while(0);

#define QCA_FLASH_BASE_REG			0x1F000000
#define QCA_SPI_FUNC_SEL_REG				QCA_FLASH_BASE_REG + 0x00
#define QCA_SPI_CTRL_REG					QCA_FLASH_BASE_REG + 0x04
#define QCA_SPI_IO_CTRL_REG					QCA_FLASH_BASE_REG + 0x08
#define QCA_SPI_READ_DATA_REG				QCA_FLASH_BASE_REG + 0x0C
#define QCA_SPI_SHIFT_DATAOUT_REG			QCA_FLASH_BASE_REG + 0x10
#define QCA_SPI_SHIFT_CNT_REG				QCA_FLASH_BASE_REG + 0x14
#define QCA_SPI_SHIFT_DATAIN_REG			QCA_FLASH_BASE_REG + 0x18

/*
 * SPI serial flash registers BIT fields
 */

/* SPI_FUNC_SELECT register (SPI function select) */
#define QCA_SPI_FUNC_SEL_FUNC_SEL_SHIFT		0
#define QCA_SPI_FUNC_SEL_FUNC_SEL_MASK		BIT(QCA_SPI_FUNC_SEL_FUNC_SEL_SHIFT)

/* SPI_CONTROL register (SPI control) */
#define QCA_SPI_CTRL_CLK_DIV_SHIFT			0
#define QCA_SPI_CTRL_CLK_DIV_MASK			BITS(QCA_SPI_CTRL_CLK_DIV_SHIFT, 6)
#define QCA_SPI_CTRL_REMAP_DIS_SHIFT		6
#define QCA_SPI_CTRL_REMAP_DIS_MASK			BIT(QCA_SPI_CTRL_REMAP_DIS_SHIFT)
#define QCA_SPI_CTRL_SPI_RELOCATE_SHIFT		7
#define QCA_SPI_CTRL_SPI_RELOCATE_MASK		BIT(QCA_SPI_CTRL_SPI_RELOCATE_SHIFT)
#define QCA_SPI_CTRL_TSHSL_CNT_SHIFT		8
#define QCA_SPI_CTRL_TSHSL_CNT_MASK			BITS(QCA_SPI_CTRL_TSHSL_CNT_SHIFT, 6)

/* SPI_IO_CONTROL register (SPI I/O control) */
#define QCA_SPI_IO_CTRL_IO_DO_SHIFT			0
#define QCA_SPI_IO_CTRL_IO_DO_MASK			BIT(QCA_SPI_IO_CTRL_IO_DO_SHIFT)
#define QCA_SPI_IO_CTRL_IO_CLK_SHIFT		8
#define QCA_SPI_IO_CTRL_IO_CLK_MASK			BIT(QCA_SPI_IO_CTRL_IO_CLK_SHIFT)
#define QCA_SPI_IO_CTRL_IO_CS0_SHIFT		16
#define QCA_SPI_IO_CTRL_IO_CS0_MASK			BIT(QCA_SPI_IO_CTRL_IO_CS0_SHIFT)
#define QCA_SPI_IO_CTRL_IO_CS1_SHIFT		17
#define QCA_SPI_IO_CTRL_IO_CS1_MASK			BIT(QCA_SPI_IO_CTRL_IO_CS1_SHIFT)
#define QCA_SPI_IO_CTRL_IO_CS2_SHIFT		18
#define QCA_SPI_IO_CTRL_IO_CS2_MASK			BIT(QCA_SPI_IO_CTRL_IO_CS2_SHIFT)

/* SPI_SHIFT_CNT_ADDR register (SPI content to shift out or in) */
#define QCA_SPI_SHIFT_CNT_BITS_CNT_SHIFT		0
#define QCA_SPI_SHIFT_CNT_BITS_CNT_MASK			BITS(QCA_SPI_SHIFT_CNT_BITS_CNT_SHIFT, 7)
#define QCA_SPI_SHIFT_CNT_TERMINATE_SHIFT		26
#define QCA_SPI_SHIFT_CNT_TERMINATE_MASK		BIT(QCA_SPI_SHIFT_CNT_TERMINATE_SHIFT)
#define QCA_SPI_SHIFT_CNT_CLKOUT_INIT_SHIFT		27
#define QCA_SPI_SHIFT_CNT_CLKOUT_INIT_MASK		BIT(QCA_SPI_SHIFT_CNT_CLKOUT_INIT_SHIFT)
#define QCA_SPI_SHIFT_CNT_CHNL_CS0_SHIFT		28
#define QCA_SPI_SHIFT_CNT_CHNL_CS0_MASK			BIT(QCA_SPI_SHIFT_CNT_CHNL_CS0_SHIFT)
#define QCA_SPI_SHIFT_CNT_CHNL_CS1_SHIFT		29
#define QCA_SPI_SHIFT_CNT_CHNL_CS1_MASK			BIT(QCA_SPI_SHIFT_CNT_CHNL_CS1_SHIFT)
#define QCA_SPI_SHIFT_CNT_CHNL_CS2_SHIFT		30
#define QCA_SPI_SHIFT_CNT_CHNL_CS2_MASK			BIT(QCA_SPI_SHIFT_CNT_CHNL_CS2_SHIFT)
#define QCA_SPI_SHIFT_CNT_SHIFT_EN_SHIFT		31
#define QCA_SPI_SHIFT_CNT_SHIFT_EN_MASK			BIT(QCA_SPI_SHIFT_CNT_SHIFT_EN_SHIFT)

typedef struct {
	u32 size;		/* total bank size in bytes             */
	u16 sector_count;	/* number of erase units                */
	u32 flash_id;		/* combined device & manufacturer code  */
	u32 sector_size;
	u32 bank;
	u16 page_size;
	u8 use_4byte_addr;
	u8 need_4byte_enable_op;
	u8 erase_cmd;
	u8 read_cmd;
	u8 page_program_cmd;
} flash_info_t;

u32 qca_sf_sect_erase(flash_info_t * info, u32 address);
int qca_sf_read(flash_info_t * info, u32 bank, u32 address, u32 length, u8 *data);
int qca_sf_write_buf(flash_info_t * info, u32 bank, u32 address, u32 length, const u8 *buf);
unsigned long flash_get_geom(flash_info_t * flash_info);
