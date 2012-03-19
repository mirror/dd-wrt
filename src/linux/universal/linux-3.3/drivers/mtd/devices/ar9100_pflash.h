#ifndef _AR9100_PFLASH_H 
#define _AR9100_PFLASH_H

#define CFG_FLASH_WORD_SIZE             uint16_t
#define CFG_FLASH_ADDR0                 (0x5555)
#define CFG_FLASH_ADDR1                 (0x2AAA)
#define FLASHWORD(_x)                   ((CFG_FLASH_WORD_SIZE)(_x))
#define AR9100_PFLASH_CTRLR             0xbe000000
#define AR9100_FLASH_NAME               "ar9100-nor0"


#define FLASH_Read_ID                   FLASHWORD( 0x90 )
#define FLASH_Read_ID_Exit              FLASHWORD( 0xF0 )
#define FLASH_Reset                     FLASHWORD( 0xFF )
#define FLASH_Program                   FLASHWORD( 0xA0 )
#define FLASH_Block_Erase               FLASHWORD( 0x30 )

#define FLASH_Data                      FLASHWORD( 0x80 )       /*  Data complement */
#define FLASH_Busy                      FLASHWORD( 0x40 )       /*  "Toggle" bit    */
#define FLASH_Err                       FLASHWORD( 0x20 )
#define FLASH_Sector_Erase_Timer        FLASHWORD( 0x08 )

#define FLASH_Setup_Code1               FLASHWORD( 0xAA )
#define FLASH_Setup_Code2               FLASHWORD( 0x55 )
#define FLASH_Setup_Erase               FLASHWORD( 0x80 )
#define FLASH_Jedec_Query               FLASHWORD( 0x90 )
#define FLASH_Soft_Exit                 FLASHWORD( 0xF0 )

typedef volatile CFG_FLASH_WORD_SIZE * f_ptr; 

/* primitives */

void 
ar9100_write(uint32_t offset,CFG_FLASH_WORD_SIZE val) {
       
        f_ptr ROM =  (f_ptr) AR9100_PFLASH_CTRLR;
	ROM[offset] = val;
}

CFG_FLASH_WORD_SIZE
ar9100_read(uint32_t offset) {

        f_ptr ROM =  (f_ptr) AR9100_PFLASH_CTRLR;
   	CFG_FLASH_WORD_SIZE val;
   	val = ROM[offset];
   	return val;
}

#endif /*_AR9100_LL_H*/
