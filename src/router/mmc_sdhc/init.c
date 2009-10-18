/*==============================================================================
 * init.c - routines responsible for initializing SD/MM card.
 * 
 * Methods in this file handle the initization of the media card.
 *
 * Initialization must typcially be done at a reduced frequency (under 400khz)
 * so be careful to call only methods that honour the maximum frequency setting.
 * (i.e. dont' call methods that are optimized...).
 *
 * Conventions:
 *
 *    -All methods begin with "init_".
 *
 * Revisions:
 *
 *    -Fix integer overflow that occured when calculating the size of an SD
 *     or MMC HC card that was > 4Gb.
 *============================================================================*/

// Structure for tracking card information
struct card_info {
    unsigned char      state;      	// State of card
#define CARD_STATE_DET   (1<<0) 	// b'00000001' - Card detected
#define CARD_STATE_INI   (1<<1) 	// b'00000010' - Card initialized
    unsigned char      type;       	// high nybble - card type - low nybble - capacity
#define CARD_TYPE_MM     (1<<5) 	// b'0010xxxx' - MM Card
#define CARD_TYPE_SD     (1<<4) 	// b'0001xxxx' - SD Card
#define CARD_TYPE_HC     (1<<0) 	// b'xxxx0001' - High Capacity card if bit set
    unsigned char      version;    	// card version - based on card type
#define CARD_VERSD_10    0x00;  	// SD card - 1.00-1.01
#define CARD_VERSD_11    0x01;  	//         - 1.10
#define CARD_VERSD_20    0x02   	//         - 2.00
#define CARD_VERMM_10    0x00   	// MM card - 1.0-1.2
#define CARD_VERMM_14    0x01   	//         - 1.4
#define CARD_VERMM_20    0x02   	//         - 2.0-2.2
#define CARD_VERMM_30    0x03   	//         - 3.1-3.3
#define CARD_VERMM_40    0x04   	//         - 4.0-4.1
    unsigned int       blocks;       	// Number of 512 byte blocks on card - use to calculate size
    unsigned int       volt;		// Voltage range bits from OCR
    unsigned char      manid;		// Manufacturer ID
    unsigned char      appid[2];	// OEM/Application ID
    unsigned char      name[6];		// Product Name
    unsigned char      rev;		// Product Revision      
    unsigned int       year;		// Product Date - year
    unsigned char      month; 	 	// Product Date - month
    unsigned int       serial;     	// Product serial number - can use to detect card change
};

// Function prototypes
static int init_card_sd(struct card_info *);
static int init_card_mm(struct card_info *);
static int init_card(struct card_info **);

//=================================================================================================

/*-----------------------------------------------------------------------------
 * Run through initialization sequence for a MM card.
 *
 * Pre-condition: 
 *
 *    Card power on clock cycles and CMD0 complete - card in SPI mode.
 *
 * Returns:
 *
 *    0 - MM card successfully initialized
 *    1 - Unexpected error encountered
 *
 * Notes:
 * 
 *-----------------------------------------------------------------------------*/
static int init_card_mm(struct card_info *c) {
    unsigned char r = 0;	// response
    unsigned char ocr[4];	// Operating Conditions Register
    unsigned char cid[16];	// Card Identification Register
    unsigned char csd[16];	// Card Specific Data Register
    unsigned char ext_csd[512];	// Extended Card Specific Data Register
    unsigned char cmd;
    int i;
    (void)c;						// Supress compiler warning

    // Send CMD58 to read the Operating Conditions Register (OCR).
    // Check that 3.3 volts is in the supported voltage range.
    // Set bits 30:29 to 1,0 to indicate host supports high capacity card.
    // Hi capacity bits can confuse older cards, retry without on error.
    LOG_DEBUG(DBG_INIT, "init MM: CMD58 - OCR voltage range check...\n");
    r = spi_mmc_cmd_r(58, 0x40000000, 0xff, ocr, 4 );
    if (r != R1_IDLE_STATE) {
	// An error occured - retry without high capacity support
	r = spi_mmc_cmd_r(58, 0x00000000, 0xff, ocr, 4 );
    }
    if ((r & ~R1_IDLE_STATE) > 0) {
	// Any bit other than idle state set - something wrong!
	cmd = 58; goto err_proto;
    } else if ((ocr[1] & 0x10) == 0) {
	// ocr indicates card does not support 3.3 volts
	LOG_ERR("init MM: CMD58 - card does not support 3.3 volts\n");
	return -EACCES;
    }

    // Send CMD1 to start MM card initialization. 
    // Continue to poll until idle state bit is clear.
    LOG_DEBUG(DBG_INIT, "init MM: CMD1 - card initialization...\n");
    for (i = 0; i < 100000; i++) {
	r = spi_mmc_cmd_r(1, 0x00000000, 0xff, NULL, 0);
	if (r != R1_IDLE_STATE) break;
    }
    if (r == R1_IDLE_STATE) {
	// Initialization timed out if busy bit still set.
	LOG_ERR("init MM: CMD1 - card initialize timeout\n");
	return -ETIMEDOUT;
    } else if (r != 0x00 ) {
	cmd = 1; goto err_proto;
    }

    // Reread the OCR again after busy clear so we can check capacity bits 30:29
    // If set to 1,0 it's a high capacity (MMHC), 1,1 or 0,0 standard capacity (MMSC).
    // Module uses byte addressing for standard cap, block addressing for high cap.
    // Same as previous cmd58 - retry without high capacity bits on error...
    LOG_DEBUG(DBG_INIT, "init MM: CMD58 - Card Capacity Status check...\n");
    r = spi_mmc_cmd_r(58, 0x40000000, 0xff, ocr, 4 );
    if (r != 0x00) {
	r = spi_mmc_cmd_r(58, 0x00000000, 0xff, ocr, 4 );
    }
    if (r != 0x00) {
	cmd = 58; goto err_proto;
    }

    // Read in the Card Identification Register (use single block read).
    LOG_DEBUG(DBG_INIT, "init MM: CMD10 - Card Identification Register read...\n");
    r = spi_mmc_read_blk(10, 0x00000000, 0xff, cid, sizeof(cid));
    if (r != 0x00) {
	cmd = 10; goto err_proto;
    }

    // Read in the Card Specific Data Register (use single block read).
    // Note that the csd structure is different for High Capacity Cards
    // The first 2 bits indicate the version...
    LOG_DEBUG(DBG_INIT, "init MM: CMD9 - Card Specific Data read...\n");
    r = spi_mmc_read_blk(9, 0x00000000, 0xff, csd, sizeof(csd));
    if (r != 0x00) {
	cmd = 9; goto err_proto;
    }

    // If a high density card, read in the extended csd (use single block read).
    // We need a value out of it to compute the card size
    if ( ((ocr[0] & 0x60) >> 5) == 2) {
	LOG_DEBUG(DBG_INIT, "init MM: CMD8 - Extended CSD read...\n");
	r = spi_mmc_read_blk(8, 0x00000000, 0xff, ext_csd, sizeof(ext_csd));
	if (r != 0x00) {
	   cmd = 8; goto err_proto;
	}
    }

    //-----------------------------------------------------------------------
    // Card initialized - extract card info and initialize card structure.
    //-----------------------------------------------------------------------

    // The formula for calulating size varies between standard & high capacity
    //
    // Standard capacity (OCR bit 30:29 = 1,1 or 0,0) - use data in CSD as follows:
    //
    //    C_Size      - 12 bits - [73:62]
    //    C_Size_Mult -  3 bits - [49:47]
    //    Read_Bl_Len -  4 bits - [83:80] 
    //
    //    Capacity (bytes) = (C_Size + 1) * ( 2 ^ (C_Size_Mult + 2)) * (2 ^ Read_Bl_Len)
    //
    // High capacity (OCR bit 30:29 = 1,0) - use data in the extended csd
    //
    //    Sec_Count   - 32 bits - [bytes 215:212]
    //
    //    Capacity (bytes) = Sec_Count * 512
    //
    unsigned int Blocks;
    if ( ((ocr[0] & 0x60) >> 5) != 2) {
	
	// Standard Capacity
	unsigned int C_Size      = ((csd[6] & 0x03) << 10) | (csd[7] << 2) | ((csd[8] >>6) & 0x03);
	unsigned int C_Size_Mult = ((csd[9] & 0x03) << 1) | ((csd[10] >> 7) & 0x01);
	unsigned int Read_Bl_Len = csd[5] & 0x0f;
	Blocks = ((C_Size + 1) * (1 << (C_Size_Mult + 2)) * (1 << Read_Bl_Len)) / 512;

    } else {
	
	// High Capacity - Use Sec_Count from extended CSD
	Blocks = ((ext_csd[296] << 24) | (ext_csd[297] << 16) | (ext_csd[298] << 8) | csd[299]);
    }

    // Fill in card structure
    c->state |= (CARD_STATE_DET & CARD_STATE_INI); 		// State - detected/initialized
    c->type |= CARD_TYPE_MM;					// MM card
    if (((ocr[0] & 0x60) >> 5) == 2) c->type |= CARD_TYPE_HC;	// std/high capacity
    c->version = (csd[0] & 0x3c) >> 2;				// Version
    c->blocks = Blocks;						// Size in 512 byte blocks
    c->volt = (ocr[1] << 16) + (ocr[2] << 8) + ocr[3];		// Voltage range from OCR
    c->manid = cid[0];		 				// Manufacturer ID
    c->appid[0] = cid[2];					// OEM/Application ID
    memcpy(c->name, cid+3, 6);					// Product Name
    c->rev = cid[9];						// Product Revision
    c->year = 1997 + (cid[14] & 0x0f);				// Product Date - year
    c->month= cid[14] >> 4;					// Product Date - year
    c->serial = (cid[10] << 24) + (cid[11] << 16) + (cid[12] << 8) + cid[13]; // Serial Number

    LOG_INFO("MM card detected & initialized\n");

    return 0;

    /* ----- Error handling ----- */
err_proto:
    // Using goto for protocol errors reduces module size substantially
    LOG_ERR("init MM: %s%d - protocol error - r1=%02x\n", (cmd & 0x80) ? "ACMD" : "CMD", (cmd & ~0x80),  r);
    return -EPROTO;
}


/*-----------------------------------------------------------------------------
 * Run through initialization sequence for an SD card.
 *
 * Pre-condition: 
 *
 *    Card power on clock cycles and CMD0 complete - card in SPI mode.
 *
 * Returns:
 *
 *    0 - SD card successfully initialized
 *    1 - Card present, but not an SD card
 *   <0 - Error occured (if a negative value).
 *
 * Notes:
 *
 *-----------------------------------------------------------------------------*/
static int init_card_sd(struct card_info *c) {
    unsigned char r = 0;	// response
    unsigned char cmd8 = 0;	// flag - set to 1 if cmd8 worked
    unsigned char ocr[4];	// Operating Conditions Register
    unsigned char cic[4];	// Card Interface Condition
    unsigned char cid[16];	// Card Identification Register
    unsigned char csd[16];	// Card Specific Data Register
    unsigned char scr[8];	// SD card configuration register
    unsigned char cmd;
    int i;

    // Start with CMD8 (SEND_IF_COND) to support v2 and high capacity cards.
    // Correct CRC required for CMD8 even though in spi mode.
    // Returns Card Interface Condition (r7) response.
    // If accepted - v2.00 SD card. If illegal cmd - v1 SD or MM card.
    LOG_DEBUG(DBG_INIT, "init SD: CMD8 - CIC voltage range check...\n");
    r = spi_mmc_cmd_r(8, 0x000001AA, 0x87, cic, 4 );  // CRC is checked for CMD8!
    if ((r & ~R1_IDLE_STATE) == 0) {
	// Cmd accepted - v2.00 SD card - ensure voltage range accepted
	if ( cic[2] != 0x01 ) {
	    LOG_ERR("init SD: CMD8 - voltage range rejected\n");
	    return -EACCES;
	} else if ( cic[3] != 0xAA ) {
	    LOG_ERR("init SD: CMD8 - check pattern failure\n");
	    return -EPROTO;
	}
	cmd8 = 1;
    } else if ((r & ~(R1_IDLE_STATE | R1_ILL_CMD)) > 0) {
	// Anything other than idle state/illegal command set - something wrong!
	cmd = 8; goto err_proto;
    } else {
	LOG_DEBUG(DBG_INIT, "init SD: CMD8 - illegal command - not v2.0 SD card...\n");
    }

    // Send CMD58 to read the Operating Conditions Register (OCR).
    // Check that 3.3 volts is in the supported voltage range.
    // Required for v1.0 cards which don't support voltage check with CMD8.
    LOG_DEBUG(DBG_INIT, "init SD: CMD58 - OCR voltage range check...\n");
    r = spi_mmc_cmd_r(58, 0x00000000, 0xff, ocr, 4 );
    if (r != R1_IDLE_STATE) {
	// Any bit other than idle state set - something wrong!
	cmd = 58; goto err_proto;
    } else if ((ocr[1] & 0x10) == 0) {
	// ocr indicates card does not support 3.3 volts
	LOG_ERR("init: CMD58 - card does not support 3.3 volts\n");
	return -EACCES;
    }

    // Send ACMD41 to start SD card initialization. 
    // If CMD8 was successful, set HCS bit in arg to indicate host supports high capacity card.
    // Continue to poll until idle state bit is clear.
    LOG_DEBUG(DBG_INIT, "init SD: ACMD41 - card initialization...\n");
    for (i = 0; i < 100000; i++) {
	r = spi_mmc_cmd_r(55, 0x00000000, 0xff, NULL, 0);
	if (r != R1_IDLE_STATE) break;
	r = spi_mmc_cmd_r(41, cmd8 ? 0x70000000 : 0x00000000, 0xff, NULL, 0);
	if (r != R1_IDLE_STATE) break;
    }
    if ((r & R1_ILL_CMD) > 0) {
	// This isn't an SD card if ACMD41 is illegal
	LOG_DEBUG(DBG_INIT, "init SD: ACMD41 - illegal command - not an SD card...\n");
        return 1;
    } else if (r == R1_IDLE_STATE) {
	// Initialization timed out if busy bit still set.
	LOG_ERR("init SD: ACMD41 - card initialize timeout\n");
	return -ETIMEDOUT;
    } else if (r != 0x00 ) {
	// Any other bits set indicates an error.
	cmd = 41 | 0x80; goto err_proto;
    }

    // If this is a version 2.0 card, we must now get the OCR again because
    // the Card Capacity Status (CCS) bit isn't valid until after card initialization.
    // If CCS bit set - v2.00 high capacity (SDHC), clear - v2.00 standard capacity (SDSC).
    // Module uses byte addressing for standard cap, block addressing for high cap.
    if (cmd8) {
	LOG_DEBUG(DBG_INIT, "init SD: CMD58 - Card Capacity Status check...\n");
	r = spi_mmc_cmd_r(58, 0x00000000, 0xff, ocr, 4 );
	if (r != 0x00) {
	    cmd = 58; goto err_proto;
	}
    }

    // Read in the Card Identification Register (use single block read).
    LOG_DEBUG(DBG_INIT, "init SD: CMD10 - Card Identification Register read...\n");
    r = spi_mmc_read_blk(10, 0x00000000, 0xff, cid, sizeof(cid));
    if (r != 0x00) {
	cmd = 10; goto err_proto;
    }

    // Read in the Card Specific Data Register (use single block read).
    // Note that the csd structure is different for High Capacity Cards
    // The first 2 bits indicate the version...
    LOG_DEBUG(DBG_INIT, "init SD: CMD9 - Card Specific Data read...\n");
    r = spi_mmc_read_blk(9, 0x00000000, 0xff, csd, sizeof(csd));
    if (r != 0x00) {
	cmd = 9; goto err_proto;
    }

    // Read in the SD config Register (use single block read).
    LOG_DEBUG(DBG_INIT, "init SD: ACMD51 - SD Config Register read...\n");
    r = spi_mmc_cmd_r(55, 0x00000000, 0xff, NULL, 0);
    r = spi_mmc_read_blk(51, 0x00000000, 0xff, scr, sizeof(scr));
    if (r != 0x00) {
	cmd = 9 | 0x80; goto err_proto;
    }

    //-----------------------------------------------------------------------
    // Card initialized - extract card info and initialize card structure.
    //-----------------------------------------------------------------------

    // The formula for calulating size varies between CSD type 1.0 and 2.0
    //
    // CSD type 1 - version 1.x cards, and version 2.x standard capacity
    //
    //    C_Size      - 12 bits - [73:62]
    //    C_Size_Mult -  3 bits - [49:47]
    //    Read_Bl_Len -  4 bits - [83:80] 
    //
    //    Capacity (bytes) = (C_Size + 1) * ( 2 ^ (C_Size_Mult + 2)) * (2 ^ Read_Bl_Len)
    //
    // CSD type 2 - version 2.x high capacity
    //
    //    C_Size      - 22 bits - [69:48]
    //
    //    Capacity (bytes) = (C_Size + 1) * 1024 * 512
    //
    unsigned int Blocks;
    if ( (csd[0] & 0xc0) == 0) {
	
	//Type 1 csd
	unsigned int C_Size      = ((csd[6] & 0x03) << 10) | (csd[7] << 2) | ((csd[8] >>6) & 0x03);
	unsigned int C_Size_Mult = ((csd[9] & 0x03) << 1) | ((csd[10] >> 7) & 0x01);
	unsigned int Read_Bl_Len = csd[5] & 0x0f;
	Blocks = ((C_Size + 1) * (1 << (C_Size_Mult + 2)) * (1 << Read_Bl_Len)) / 512;

    } else {
	
	//Type 2 csd
	Blocks = (((csd[7] & 0x3F) << 16) | (csd[8] << 8) | csd[9]) * 1024;
    }

    // Fill in card structure
    c->state |= (CARD_STATE_DET & CARD_STATE_INI); 		// State - detected/initialized
    c->type |= CARD_TYPE_SD;					// SD card
    if (ocr[0] & 0x40) c->type |= CARD_TYPE_HC;			// std/high capacity
    c->version = scr[0] & 0x0f;					// Version
    c->blocks = Blocks;						// Size in 512 byte blocks
    c->volt = (ocr[1] << 16) + (ocr[2] << 8) + ocr[3];		// Voltage range from OCR
    c->manid = cid[0];		 				// Manufacturer ID
    memcpy(c->name, cid+3, 5);					// Product Name
    memcpy(c->appid, cid+1, 2);					// OEM/Application ID
    c->rev = cid[8];						// Product Revision
    c->year = 2000 + ((cid[13] & 0x0f) << 4) + (cid[14] >> 4);	// Product Date - year
    c->month = cid[14] &0x0f;					// Product Date - month
    c->serial = (cid[9] << 24) + (cid[10] << 16) + (cid[11] << 8) + cid[12]; // Serial Number

    LOG_INFO("SD Card detected & initialized\n");

    return 0;

    /* ----- Error handling ----- */
err_proto:
    // Using goto for protocol errors reduces module size substantially
    LOG_ERR("init SD: %s%d - protocol error - r1=%02x\n", (cmd & 0x80) ? "ACMD" : "CMD", (cmd & ~0x80),  r);
    return -EPROTO;
}


/*-----------------------------------------------------------------------------
 * Initialize card
 *
 * Pre-condition: 
 *
 *    none
 *
 * Returns:
 *
 *    0  - Card detected and successfully initialized
 *    >0 - Initialization failed.
 *
 * Notes:
 *
 *   Caller is responsible for freeing the card structure
 *
 *-----------------------------------------------------------------------------*/
static int init_card(struct card_info **ret) {
    int r = 0;
    int rcode = 0;
    short i;
    struct card_info *c;
    unsigned long flags;

    // Save processor interrupt priority level and disable interrupts
    // while initializing the card
    save_flags(flags);
    cli();

    // Create a card structure to track card information and zero it
    c = kmalloc(sizeof(struct card_info), GFP_KERNEL); 
    if (!c) {
	LOG_ERR("init: memory allocation failure");
	rcode = -ENOMEM;
	goto err;
    }
    memset(c, 0, sizeof(struct card_info));

    // Clock frequency below 400KHz until card is fully initialized. 
    // 380KHz allows for  margin of error in the spi delay routines.
    spi_freq_max(380);

    // After insertion - card must be sent a minimum of 74 clock cycles 
    // with CS de-asserted before it will accept a command. We send it
    // WAY more, because, since we can't power cycle the card, we might
    // be trying to recover one left in a funny state from previous use/errors.
    LOG_DEBUG(DBG_INIT, "init: power on/insertion initialization...\n");
    spi_cs_dea();
    for (i = 0; i < 10000; i++) spi_io(0xff);

    // Send a CMD0 (reset) with CS asserted (low) to enter spi mode/idle state
    LOG_DEBUG(DBG_INIT, "init: CMD0 - reset...\n");
    r = spi_mmc_cmd_r(0, 0x00000000, 0x95, NULL, 0);
    if (r != 0x01 ) {
	LOG_ERR("init: No card detected\n");
	rcode = -ENODEV;
	goto err;
    }
    c->state |= CARD_STATE_DET;		// Set card detected flag bit

    // At this point we know a card is present. Attempt SD card initialization first.
    rcode = init_card_sd(c);
    if (r < 0) goto err;			// SD card init failed

    // Not an SD card so attempt MM card initialization.
    if (rcode == 1) rcode = init_card_mm(c);
    if (r < 0) goto err;			// MM card init failed

    // Card fully initialized - full steam ahead with the clock speed.
    spi_freq_max(0);

    // All cards support 512 byte blocks for read/write - use it!
    LOG_DEBUG(DBG_INIT, "init - CMD16 - set 512 byte blocksize...\n");
    r = spi_mmc_cmd_r(16, 512, 0xff, NULL, 0);
    if (r != 0x00 ) {
	LOG_ERR("init: CMD16 - set blocksize failed - r1=%02x\n", r);
	rcode = -EPROTO;
	goto err;
    }

    restore_flags(flags);
    *ret = c;
    return rcode;
 
    /* ----- Error handling ----- */
err:
    if (c) kfree(c);
    restore_flags(flags);
    *ret = NULL;
    return rcode;
}

