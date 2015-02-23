/**
 * 
 * @file wavesat_tools.c
 * 
 * @brief Wavesat tools provide support to WiMAX hardware
 *        platform.
 * 
 * @author Benoit Masse.
 * 
 */    

#include <linux/autoconf.h>
#include <mach/hardware.h>

MODULE_AUTHOR("Wavesat 2007");
MODULE_DESCRIPTION("Wavesat tools for WiMAX hardware compatible.");

/**
 * @brief Save the npe mac address in eeprom.
 */
//extern int ws_set_npe_esa ( uint8_t *mac );
/**
 * @brief Get the npe mac address from eeprom.
 */
//extern int ws_get_npe_esa( uint8_t *buf);
/**
 * @brief Set the ofdm mac addr in the eeprom.
 */
//extern void ws_set_ofdm_mac_addr( char * pOfdmAddr );
/**
 * @brief Get the ofdm mac addr from eeprom.
 */
//extern char * ws_get_ofdm_mac_addr(void);

#if defined(WAVESAT_I2C_EEPROM)
// Eeprom access and configuration.
/**
 * @brief Function to read data from Eeprom.
 *
 */ 
extern int ws_eeprom_read(
        unsigned char *buf,  /**< Pointer to the data to write in Eeprom. */
        unsigned int addr,   /**< Address offset where to write data in Eeprom. */
        int size );          /**< Number of data to write in to the Eeprom. */
/**
 * @brief Function to write into Eeprom.
 *
 */ 
extern int ws_eeprom_write(
        unsigned char *buf,  /**< Pointer to the data to write in Eeprom. */
        unsigned int addr,   /**< Address offset where to write data in Eeprom. */
        int size );          /**< Number of data to write in to the Eeprom. */
#endif // #if defined(WAVESAT_I2C_EEPROM)

#if defined (CONFIG_WAVESAT_DEFINE_OFDM_MAC_ADDRESS)

static char g_ofdm_mac_address[7];

char* ws_get_ofdm_mac_addr(
   void
   )
{
   static int   firstTime = 1;

   if ( firstTime )
   {
      firstTime = 0;
      g_ofdm_mac_address[6] = 0;
      #if defined CONFIG_MACH_DWW34 | defined CONFIG_ARCH_VENUS
      eeprom_get_ofdm_mac_addr( &g_ofdm_mac_address[0] );
      #else  // CONFIG_MACH_AVILA
      avila_get_ofdm_mac_addr_( &g_ofdm_mac_address[0] );
      #endif
   }
   return &g_ofdm_mac_address[0];
}

void ws_set_ofdm_mac_addr( 
   char * pOfdmAddr 
   )
{
   #if defined CONFIG_MACH_DWW34 | defined CONFIG_ARCH_VENUS
   eeprom_set_ofdm_mac_addr( pOfdmAddr );
   #else  // CONFIG_MACH_AVILA
   avila_set_ofdm_mac_addr_( pOfdmAddr );
   #endif
   return;
}

EXPORT_SYMBOL(ws_get_ofdm_mac_addr);
EXPORT_SYMBOL(ws_set_ofdm_mac_addr);

#endif // #if defined (CONFIG_WAVESAT_DEFINE_OFDM_MAC_ADDRESS)

#if defined (CONFIG_WAVESAT_DEFINE_NPE_MAC_ADDRESS)

int get_npe_esa( int index, uint8_t *buf )
{

   #if defined CONFIG_MACH_DWW34 | defined CONFIG_ARCH_VENUS
   eeprom_get_npe_esa( index, buf );
   #else  // CONFIG_MACH_AVILA
   avila_get_npe_esa( index, &npe_mac[0] );
   #endif
   memcpy( buf, &npe_mac[0], sizeof(npe_mac) );
   // don't use broadcast address
   if (buf[0] == 0xff && buf[1] == 0xff && buf[2] == 0xff &&
       buf[3] == 0xff && buf[4] == 0xff && buf[5] == 0xff)
   {
       return 0;
   }
   return 1;
}

int ws_get_npe_esa( 
   uint8_t *buf
   )
{
   return get_npe_esa( 0, buf);
}

int ws_set_npe_esa ( 
   uint8_t *mac 
   )
{
   #if defined CONFIG_MACH_DWW34 | defined CONFIG_ARCH_VENUS
   eeprom_set_npe_esa( index, buf );
   #else  // CONFIG_MACH_AVILA
   avila_set_npe_esa( index, &npe_mac[0] );
   #endif
}

EXPORT_SYMBOL(ws_get_npe_esa);
EXPORT_SYMBOL(ws_set_npe_esa);

#endif // #if CONFIG_WAVESAT_DEFINE_NPE_MAC_ADDRESS

/******************************************************************************
   Board type.
 ******************************************************************************/
#if defined CONFIG_MACH_DWW34
static int gWavesatBoardType = WAVESAT_DWW34;
#elif defined CONFIG_ARCH_VENUS
static int gWavesatBoardType = WAVESAT_VENUS;
#elif defined CONFIG_MACH_AVILA
static int gWavesatBoardType = WAVESAT_AVILA;
#else
#error "Wavesat compatible board must be defined."
#endif
int WavesatBoardTypeGet(
   void
   )
{
   return gWavesatBoardType;
}


/******************************************************************************
   OFDM AND NPE Support Definitions.
 ******************************************************************************/
#if defined(CONFIG_MACH_WAVESAT_VENUS || CONFIG_MACH_WAVESAT_DWW34)

#if defined (CONFIG_WAVESAT_DEFINE_OFDM_MAC_ADDRESS)
/**
 * @brief Set the ofdm mac addr in the eeprom.
 */
void eeprom_set_ofdm_mac_addr( 
   char * pOfdmAddr 
   )
{
   int i;
   for ( i = 0; i < EEPROM_OFDM_LEN; i++ )
   {
      ws_eeprom_write(EEPROM_OFDM_OFF + i, pOfdmAddr[i], 1);
      mdelay(100);
   }
   return;

}
/**
 * @brief Get the ofdm mac addr from eeprom.
 */
int eeprom_get_ofdm_mac_addr(
   uint8_t *buf
   )
{
   ws_eeprom_read(EEPROM_OFDM_OFF, buf, EEPROM_OFDM_LEN);
   return 0;
}

#endif #if defined (CONFIG_WAVESAT_DEFINE_OFDM_MAC_ADDRESS)

#if CONFIG_WAVESAT_DEFINE_NPE_MAC_ADDRESS
/**
 * @brief Get the npe mac address from eeprom.
 */
int eeprom_get_npe_esa(
   uint8_t index,
   uint8_t *buf
   )
{
   if ( index == 1 )
   {
      ws_eeprom_read(EEPROM_ETHB_OFF, buf, EEPROM_ETHB_LEN);
   }
   else 
   {
      ws_eeprom_read(EEPROM_ETHA_OFF, buf, EEPROM_ETHA_LEN);
   }
   return 0;
}
/**
 * @brief Save the npe mac address in eeprom.
 */
int eeprom_set_npe_esa ( 
   uint8_t *mac 
   )
{
   int i;
   for ( i = 0; i < EEPROM_ETHA_LEN; i++ )
   {
      print_deb("mac[%d] = 0x%x\n", i, mac[i]);
      ws_eeprom_write(EEPROM_ETHA_OFF + i, mac[i]);
      mdelay(100);
   }
   return 0;
}
#endif // #if CONFIG_WAVESAT_DEFINE_NPE_MAC_ADDRESS

#endif // #if defined(CONFIG_MACH_WAVESAT_VENUS || CONFIG_MACH_WAVESAT_DWW34)

#if defined(CONFIG_ARCH_WAVESAT_AVILA)

void avila_set_ofdm_mac_addr( 
   char * pOfdmAddr 
   )
{
   return;
}

static int avila_get_ofdm_mac_addr(
   uint8_t *buf
   )
{
   static char  npe_mac[6]= {0x00,0x00,0x00,0x00,0x00,0x00};
   extern char saved_command_line[];
   int i;
   int val;

   for ( i=0;i<strlen(saved_command_line);i++ )
   {
      if ( strncmp(saved_command_line+i,"OFDM",4)==0 )
      {
         //printk("Index = %i  %s\n", i,&saved_command_line[i] );
         sscanf(&saved_command_line[i+ 5],"0x%x",&val);   npe_mac[0]= (uint8_t)val;
         sscanf(&saved_command_line[i+10],"0x%x",&val);   npe_mac[1]= (uint8_t)val;
         sscanf(&saved_command_line[i+15],"0x%x",&val);   npe_mac[2]= (uint8_t)val;
         sscanf(&saved_command_line[i+20],"0x%x",&val);   npe_mac[3]= (uint8_t)val;
         sscanf(&saved_command_line[i+25],"0x%x",&val);   npe_mac[4]= (uint8_t)val;
         sscanf(&saved_command_line[i+30],"0x%x",&val);   npe_mac[5]= (uint8_t)val;
         print_deb("OFDM address: %02X:%02X:%02X:%02X:%02X:%02X \n",npe_mac[0],npe_mac[1],npe_mac[2],npe_mac[3],npe_mac[4],npe_mac[5]);
         break;
      }
   }

   if ( i != strlen(saved_command_line) )
   {
      // We found mac address.
      memcpy(buf,npe_mac,6);
      retVal = 0;
   }
   else
   {
      // We didn't find mac address.
      print_deb("No OFDM MAC address on bootline\n");
      retVal = -1
   }
   return retVal;
}

void avila_set_npe_esa( 
   int index, 
   uint8_t *buf
   )
{
   return;
}

int avila_get_npe_esa( 
   int index, 
   uint8_t *buf
   )
{

   char  npe_mac[6]= {0x00,0x00,0x00,0x00,0x00,0x00};
   extern char saved_command_line[];
   int i;
   int val;
   int retVal = -1;

   for ( i=0;i<strlen(saved_command_line);i++ )
   {
      int found = 0;
      if ( index )
      {
         if ( strncmp(saved_command_line+i,"NPE1",4)==0 )
         {
            found = 1;
         }
      }
      else
         if ( strncmp(saved_command_line+i,"NPE0",4)==0 )
         {
            found = 1;
         }
      if ( found )
      {
         //printk("Index = %i  %s\n", i,&saved_command_line[i] );
         sscanf(&saved_command_line[i+ 5],"0x%x",&val);   npe_mac[0]=val;
         sscanf(&saved_command_line[i+10],"0x%x",&val);   npe_mac[1]=val;
         sscanf(&saved_command_line[i+15],"0x%x",&val);   npe_mac[2]=val;
         sscanf(&saved_command_line[i+20],"0x%x",&val);   npe_mac[3]=val;
         sscanf(&saved_command_line[i+25],"0x%x",&val);   npe_mac[4]=val;
         sscanf(&saved_command_line[i+30],"0x%x",&val);   npe_mac[5]=val;
         print_deb("NPE%d address: %02X:%02X:%02X:%02X:%02X:%02X \n", index, npe_mac[0],npe_mac[1],npe_mac[2],npe_mac[3],npe_mac[4],npe_mac[5]);
         break;
      }
   }
   if ( i != strlen(saved_command_line) )
   {
      // We found a mac address.
      memcpy(buf,npe_mac,6);
      retVal = 0;
   }
   else
   {
      // We didn't find a mac address.
      print_deb("No NPE%d MAC address on command line\n", index);
      retVal = -1
   }
  return retVal;
}
#endif // #if defined(CONFIG_ARCH_WAVESAT_AVILA)
/******************************************************************************
   
 ******************************************************************************/


#endif // #if defined CONFIG_MACH_DWW34 | defined CONFIG_ARCH_VENUS


