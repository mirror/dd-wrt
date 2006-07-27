/* Mjam Tool. SeG */
 
 #include <sys/mman.h>
 #include <stdio.h>
 #include <unistd.h>
 #include <fcntl.h>
 
 #define AR5K_PCICFG 0x4010 
 #define AR5K_PCICFG_EEAE 0x00000001 
 #define AR5K_PCICFG_CLKRUNEN 0x00000004 
 #define AR5K_PCICFG_LED_PEND 0x00000020 
 #define AR5K_PCICFG_LED_ACT 0x00000040 
 #define AR5K_PCICFG_SL_INTEN 0x00000800 
 #define AR5K_PCICFG_BCTL		 0x00001000 
 #define AR5K_PCICFG_SPWR_DN 0x00010000 
 
 /* EEPROM Registers in the MAC */
 #define AR5211_EEPROM_ADDR 0x6000 
 #define AR5211_EEPROM_DATA 0x6004
 #define AR5211_EEPROM_COMD 0x6008
 #define AR5211_EEPROM_COMD_READ 0x0001
 #define AR5211_EEPROM_COMD_WRITE 0x0002
 #define AR5211_EEPROM_COMD_RESET 0x0003
 #define AR5211_EEPROM_STATUS 0x600C
 #define AR5211_EEPROM_STAT_RDERR 0x0001
 #define AR5211_EEPROM_STAT_RDDONE 0x0002
 #define AR5211_EEPROM_STAT_WRERR 0x0003
 #define AR5211_EEPROM_STAT_WRDONE 0x0004
 #define AR5211_EEPROM_CONF 0x6010
 #define AR5K_EEPROM_PROTECT_OFFSET 0x3F
 
 #define VT_WLAN_IN32(a)  (*((volatile unsigned long int *)(mem + (a))))
 #define VT_WLAN_OUT32(v,a) (*((volatile unsigned long int *)(mem + (a))) = (v))
 
 int
 vt_ar5211_eeprom_read( unsigned char *mem,
 		       unsigned long int offset,
 		       unsigned short int *data )
 {
 	int timeout = 10000 ;
 	unsigned long int status ;
 
 	VT_WLAN_OUT32( 0, AR5211_EEPROM_CONF ),
 	usleep( 5 ) ;
 
 	/** enable eeprom read access */
 	VT_WLAN_OUT32( VT_WLAN_IN32(AR5211_EEPROM_COMD)
 		     | AR5211_EEPROM_COMD_RESET, AR5211_EEPROM_COMD) ;
 	usleep( 5 ) ;
 
 	/** set address */
 	VT_WLAN_OUT32( (unsigned char) offset, AR5211_EEPROM_ADDR) ;
 	usleep( 5 ) ;
 
 	VT_WLAN_OUT32( VT_WLAN_IN32(AR5211_EEPROM_COMD)
 		     | AR5211_EEPROM_COMD_READ, AR5211_EEPROM_COMD) ;
 
 	while (timeout > 0) {
 		usleep(1) ;
 		status = VT_WLAN_IN32(AR5211_EEPROM_STATUS) ;
 		if (status & AR5211_EEPROM_STAT_RDDONE) {
 			if (status & AR5211_EEPROM_STAT_RDERR) {
 				(void) fputs( "eeprom read access failed!\n",
 					      stderr ) ;
 				return 1 ;
 			}
 			status = VT_WLAN_IN32(AR5211_EEPROM_DATA) ;
 			*data = status & 0x0000ffff ;
 			return 0 ;
 		}
 		timeout-- ;
 	}
 
 	(void) fputs( "eeprom read timeout!\n", stderr ) ;
 
 	return 1 ;
 }
 
 int
 vt_ar5211_eeprom_write( unsigned char *mem,
 		        unsigned int offset,
 		        unsigned short int new_data )
 {
 	int timeout = 10000 ;
 	unsigned long int status ;
 	unsigned long int pcicfg ;
 	int i ;
 	unsigned short int sdata ;
 
 	/** enable eeprom access */
 	pcicfg = VT_WLAN_IN32( AR5K_PCICFG ) ;
 VT_WLAN_OUT32( ( pcicfg & ~AR5K_PCICFG_SPWR_DN ), AR5K_PCICFG ) ;
 usleep( 500 ) ;
 	VT_WLAN_OUT32( pcicfg | AR5K_PCICFG_EEAE /* | 0x2 */, AR5K_PCICFG) ;
 	usleep( 50 ) ;
 
 	VT_WLAN_OUT32( 0, AR5211_EEPROM_STATUS );
 	usleep( 50 ) ;
 
 	/* VT_WLAN_OUT32( 0x1, AR5211_EEPROM_CONF ) ; */
 	VT_WLAN_OUT32( 0x0, AR5211_EEPROM_CONF ) ;
 	usleep( 50 ) ;
 
 	i = 100 ;
 retry:
 	/** enable eeprom write access */
 	VT_WLAN_OUT32( AR5211_EEPROM_COMD_RESET, AR5211_EEPROM_COMD);
 	usleep( 500 ) ;
 
 	/* Write data */
 	VT_WLAN_OUT32( new_data, AR5211_EEPROM_DATA );
 	usleep( 5 ) ;
 
 	/** set address */
 	VT_WLAN_OUT32( offset, AR5211_EEPROM_ADDR);
 	usleep( 5 ) ;
 
 	VT_WLAN_OUT32( AR5211_EEPROM_COMD_WRITE, AR5211_EEPROM_COMD);
 	usleep( 5 ) ;
 
 	for ( timeout = 10000 ; timeout > 0 ; --timeout ) {
 		status = VT_WLAN_IN32( AR5211_EEPROM_STATUS );
 		if ( status & 0xC ) {
 			if ( status & AR5211_EEPROM_STAT_WRERR ) {
 				fprintf( stderr,
 				 "eeprom write access failed!\n");
 				return 1 ;
 			}
 			VT_WLAN_OUT32( 0, AR5211_EEPROM_STATUS );
 			usleep( 10 ) ;
 			break ;
 		}
 		usleep( 10 ) ;
 		timeout--;
 	}
 	(void) vt_ar5211_eeprom_read( mem, offset, &sdata ) ;
 	if ( ( sdata != new_data ) && i ) {
 		--i ;
 		fprintf( stderr, "Retrying eeprom write!\n");
 		goto retry ;
 	}
 
 	return !i ;
 }
 
 static void
 Usage( char *progname )
 {
 	(void) fprintf( stderr,
 			"Usage: %s physical_address_base new_country_code\n",
 			progname ) ;
 	return ;
 }
 
 int
 main( int argc, char **argv )
 {
 	unsigned long int base_addr ;
 	int fd ;
 	void *membase ;
 	unsigned short int sdata ;
 	unsigned short int new_cc ;
 
 	if ( argc < 3 ) {
 		Usage( argv[0] ) ;
 		return -1 ;
 	}
 
 	base_addr = strtoul( argv[2], NULL, 0 ) ;
 	if ( base_addr > 0xFFFF ) {
 		(void) fputs(
 		      "Error: New domain code must be 16 bits or less\n",
 			      stderr ) ;
 		Usage( argv[0] ) ;
 		return -2 ;
 	}
 	new_cc = (unsigned short int) base_addr ;
 	base_addr = strtoul( argv[1], NULL, 0 ) ;
 #define ATHEROS_PCI_MEM_SIZE 0x10000
 	fd = open( "/dev/mem", O_RDWR ) ;
 	if ( fd < 0 ) {
 		fprintf( stderr, "Open of /dev/mem failed!\n" ) ;
 		return -2 ;
 	}
 	membase = mmap( 0, ATHEROS_PCI_MEM_SIZE, PROT_READ|PROT_WRITE,
 			MAP_SHARED|MAP_FILE, fd, base_addr ) ;
 	if ( membase == (void *) -1 ) {
 		fprintf( stderr,
 			 "Mmap of device at 0x%08X for 0x%X bytes failed!\n",
 			 base_addr, ATHEROS_PCI_MEM_SIZE ) ;
 		return -3 ;
 	}
 
 #if 0
 	(void) vt_ar5211_eeprom_write( (unsigned char *) membase,
 				       AR5K_EEPROM_PROTECT_OFFSET, 0 ) ;
 #endif /* #if 0 */
 
 	if ( vt_ar5211_eeprom_read( (unsigned char *) membase, 0xBF, &sdata ) )
 		fprintf( stderr, "EEPROM read failed\n" ) ;
 
 	printf( "Current value 0x%04X will change to 0x%04X\n", sdata,
 		new_cc ) ;
 
 	if ( vt_ar5211_eeprom_write( (unsigned char *) membase, 0xBF, new_cc ) )
 		fprintf( stderr, "EEPROM write failed\n" ) ;
 
 	if ( vt_ar5211_eeprom_read( (unsigned char *) membase, 0xBF, &sdata ) )
 		fprintf( stderr, "EEPROM read failed\n" ) ;
 
 	if ( sdata != new_cc )
 		fprintf( stderr, "Write & read dont match 0x%04X != 0x%04X\n",
 			 new_cc, sdata ) ;
 
 	return 0 ;
 }
