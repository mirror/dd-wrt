#ifndef __ASM_ARM_ARCH_FLASH_H
#define __ASM_ARM_ARCH_FLASH_H

#define FLASH_START                                     SL2312_FLASH_BASE
#define SFLASH_SIZE                      		0x00400000
#define SPAGE_SIZE                       		0x200
#define BLOCK_ERASE                     		0x50
#define BUFFER1_READ                    		0x54
#define BUFFER2_READ                    		0x56
#define PAGE_ERASE                      		0x81
#define MAIN_MEMORY_PAGE_READ           		0x52
#define MAIN_MEMORY_PROGRAM_BUFFER1     		0x82
#define MAIN_MEMORY_PROGRAM_BUFFER2     		0x85
#define BUFFER1_TO_MAIN_MEMORY          		0x83
#define BUFFER2_TO_MAIN_MEMORY          		0x86
#define MAIN_MEMORY_TO_BUFFER1          		0x53
#define MAIN_MEMORY_TO_BUFFER2          		0x55
#define BUFFER1_WRITE                   		0x84
#define BUFFER2_WRITE                   		0x87
#define AUTO_PAGE_REWRITE_BUFFER1       		0x58
#define AUTO_PAGE_REWRITE_BUFFER2       		0x59
#define READ_STATUS                     		0x57

#define MAIN_MEMORY_PAGE_READ_SPI       		0xD2
#define BUFFER1_READ_SPI                		0xD4
#define BUFFER2_READ_SPI                		0xD6
#define READ_STATUS_SPI                 		0xD7

#define	FLASH_ACCESS_OFFSET	        		0x00000010
#define	FLASH_ADDRESS_OFFSET            		0x00000014
#define	FLASH_WRITE_DATA_OFFSET         		0x00000018
#define	FLASH_READ_DATA_OFFSET          		0x00000018
#define SERIAL_FLASH_CHIP1_EN            0x00010000  // 16th bit = 1
#define SERIAL_FLASH_CHIP0_EN            0x00000000  // 16th bit = 0
#define AT45DB321_PAGE_SHIFT		         0xa
#define AT45DB642_PAGE_SHIFT		         0xb
#define CONTINUOUS_MODE		         0x00008000

#define FLASH_ACCESS_ACTION_OPCODE                      0x0000
#define FLASH_ACCESS_ACTION_OPCODE_DATA                 0x0100
#define FLASH_ACCESS_ACTION_SHIFT_ADDRESS               0x0200
#define FLASH_ACCESS_ACTION_SHIFT_ADDRESS_DATA          0x0300
#define FLASH_ACCESS_ACTION_SHIFT_ADDRESS_X_DATA          0x0400
#define FLASH_ACCESS_ACTION_SHIFT_ADDRESS_2X_DATA         0x0500
#define FLASH_ACCESS_ACTION_SHIFT_ADDRESS_3X_DATA         0x0600
#define FLASH_ACCESS_ACTION_SHIFT_ADDRESS_4X_DATA         0x0700
//#define FLASH_ACCESS_ACTION_SHIFT_ADDRESS_X_DATA        0x0600
//#define FLASH_ACCESS_ACTION_SHIFT_ADDRESS_4X_DATA       0x0700

#define M25P80_PAGE_SIZE  0x100
#define M25P80_SECTOR_SIZE  0x10000


//#define M25P80_BULK_ERASE                                      1
//#define M25P80_SECTOR_ERASE                                    2
//#define M25P80_SECTOR_SIZE                                     0x10000

#define M25P80_WRITE_ENABLE                  		0x06
#define M25P80_WRITE_DISABLE                 		0x04
#define M25P80_READ_STATUS                   		0x05
#define M25P80_WRITE_STATUS              		0x01
#define M25P80_READ                      		0x03
#define M25P80_FAST_READ                 		0x0B
#define M25P80_PAGE_PROGRAM              		0x02
#define M25P80_SECTOR_ERASE              		0xD8
#define M25P80_BULK_ERASE                		0xC7
#define FLASH_ERR_OK							0x0

extern void address_to_page(__u32, __u16 *, __u16 *);
extern void main_memory_page_read(__u8, __u16, __u16, __u8 *);
extern void buffer_to_main_memory(__u8, __u16);
extern void main_memory_to_buffer(__u8, __u16);
extern void main_memory_page_program(__u8, __u16, __u16, __u8);
extern void atmel_flash_read_page(__u32, __u8 *, __u32);
extern void atmel_erase_page(__u8, __u16);
extern void atmel_read_status(__u8, __u8 *);
extern void atmel_flash_program_page(__u32, __u8 *, __u32);
extern void atmel_buffer_write(__u8, __u16, __u8);
extern void flash_delay(void);

extern int m25p80_sector_erase(__u32 address, __u32 schip_en);

#endif
