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
#include <linux/kernel.h>
#include <linux/slab.h>   /* kmalloc() */
#include <linux/module.h>
#include <linux/init.h>
#include <linux/sched.h>
#include <asm/semaphore.h>
#include <asm/uaccess.h>
#include <asm/io.h> /* ioremap */
#include <linux/ioport.h>

#include <linux/proc_fs.h>
#include <linux/cdev.h>

#include <linux/delay.h>

#include <mach/hardware.h>

#include <asm/arch/ws_tools.h>


// Local debug, set to 0 for no debug print or something else for debug print.
#if 0
#define print_deb printk
#else
#define print_deb(...)
#endif


/******************************************************************************
   Board type.
 ******************************************************************************/
#if defined CONFIG_MACH_WAVESAT_DWW34
static int gWavesatBoardType = WAVESAT_DWW34;
#elif defined CONFIG_MACH_WAVESAT_VENUS
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
#if defined (CONFIG_MACH_WAVESAT_VENUS) || defined(CONFIG_MACH_WAVESAT_DWW34)

#if defined (CONFIG_WAVESAT_DEFINE_OFDM_MAC_ADDRESS)
/**
 * @brief Set the ofdm mac addr in the eeprom.
 */
void eeprom_set_ofdm_mac_addr(
   char * pOfdmAddr
   )
{
   ws_eeprom_write(&pOfdmAddr[0], EEPROM_OFDM_OFF, EEPROM_OFDM_LEN);
   return;
}
/**
 * @brief Get the ofdm mac addr from eeprom.
 */
int eeprom_get_ofdm_mac_addr(
   uint8_t *buf
   )
{
   ws_eeprom_read( buf, EEPROM_OFDM_OFF, EEPROM_OFDM_LEN);
   return 0;
}

#endif // #if defined (CONFIG_WAVESAT_DEFINE_OFDM_MAC_ADDRESS)

#if CONFIG_WAVESAT_DEFINE_NPE_MAC_ADDRESS
/**
 * @brief Get the npe mac address from eeprom.
 */
int eeprom_get_npe_esa(
   uint8_t index,
   uint8_t *mac
   )
{
   if ( index == 0 )
   {
      ws_eeprom_read(&mac[0], EEPROM_ETHA_OFF, EEPROM_ETHA_LEN);
   }
   else
   {
      return -1;
   }
   return 0;
}
/**
 * @brief Save the npe mac address in eeprom.
 */
int eeprom_set_npe_esa (
   int index,
   uint8_t *mac
   )
{
   if ( index == 0 )
   {
      ws_eeprom_write(&mac[0], EEPROM_ETHA_OFF, EEPROM_ETHA_LEN);
   }
   return 0;
}
#endif // #if CONFIG_WAVESAT_DEFINE_NPE_MAC_ADDRESS

#endif // #if defined(CONFIG_MACH_WAVESAT_VENUS || CONFIG_MACH_WAVESAT_DWW34)

#if defined(CONFIG_MACH_AVILA)

void avila_set_ofdm_mac_addr(
   char * pOfdmAddr
   )
{
   return;
}

static int read_cmd_line_mac_addr(
   const char *name,
   uint8_t *buf
   )
{
   static char  npe_mac[6]= {0x00,0x00,0x00,0x00,0x00,0x00};
   extern char *saved_command_line;
   int i;
   int val;
   int retVal = 0;

   for ( i=0;i<strlen(saved_command_line);i++ )
   {
      if ( strncmp(saved_command_line+i,name,strlen(name))==0 )
      {
         //printk("Index = %i  %s\n", i,&saved_command_line[i] );
         sscanf(&saved_command_line[i+ 5],"0x%x",&val);   npe_mac[0]= (uint8_t)val;
         sscanf(&saved_command_line[i+10],"0x%x",&val);   npe_mac[1]= (uint8_t)val;
         sscanf(&saved_command_line[i+15],"0x%x",&val);   npe_mac[2]= (uint8_t)val;
         sscanf(&saved_command_line[i+20],"0x%x",&val);   npe_mac[3]= (uint8_t)val;
         sscanf(&saved_command_line[i+25],"0x%x",&val);   npe_mac[4]= (uint8_t)val;
         sscanf(&saved_command_line[i+30],"0x%x",&val);   npe_mac[5]= (uint8_t)val;
         print_deb("%s address: %02X:%02X:%02X:%02X:%02X:%02X \n", name, npe_mac[0],npe_mac[1],npe_mac[2],npe_mac[3],npe_mac[4],npe_mac[5]);
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
      retVal = -1;
   }
   return retVal;
}

static int avila_get_ofdm_mac_addr(
   uint8_t *buf
   )
{
   return read_cmd_line_mac_addr( "OFDM", buf );
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
   if ( index )
   {
      return read_cmd_line_mac_addr( "NPE1", buf );
   }
   else
   {
      return read_cmd_line_mac_addr( "NPE0", buf );
   }
   return 0;
}

#endif // #if defined(CONFIG_MACH_AVILA)

/******************************************************************************
   OFDM AND NPE Export functions.
 ******************************************************************************/
#if defined (CONFIG_WAVESAT_DEFINE_OFDM_MAC_ADDRESS)

static char g_ofdm_mac_address[7];
/*
   Return the WiMAX OFDM mac address.
   Use a local global variable (i.e. not exported) to store the mac address.
   This speed-up the access time after the first call.
   This variable is read from the eeprom only one time at the first call. After the first call
   modification in the mac address storage will not available before next reboot.
*/
char* ws_get_ofdm_mac_addr(
   void
   )
{
   static int   firstTime = 1;

   // Use of local global variable to store the mac address.
   // This speed-up the access time after the first call.
   if ( firstTime )
   {
      firstTime = 0;
      g_ofdm_mac_address[6] = 0;
      #if defined (CONFIG_MACH_WAVESAT_DWW34) || defined(CONFIG_MACH_WAVESAT_VENUS)
      eeprom_get_ofdm_mac_addr( &g_ofdm_mac_address[0] );
      #else  // CONFIG_MACH_AVILA
      avila_get_ofdm_mac_addr( &g_ofdm_mac_address[0] );
      #endif
   }
   return &g_ofdm_mac_address[0];
}

void ws_set_ofdm_mac_addr(
   char *mac
   )
{
   #if defined (CONFIG_MACH_WAVESAT_DWW34) || defined(CONFIG_MACH_WAVESAT_VENUS)
   eeprom_set_ofdm_mac_addr( &mac[0] );
   #else  // CONFIG_MACH_AVILA
   avila_set_ofdm_mac_addr( &mac[0] );
   #endif
   return;
}


EXPORT_SYMBOL(ws_get_ofdm_mac_addr);
EXPORT_SYMBOL(ws_set_ofdm_mac_addr);

#endif // #if defined (CONFIG_WAVESAT_DEFINE_OFDM_MAC_ADDRESS)

struct pci_dev;

#if defined (CONFIG_MACH_WAVESAT_VENUS)
extern int venus_map_irq(struct pci_dev *dev, u8 slot, u8 pin);
int wavesat_pci_irq_map(struct pci_dev *dev, u8 slot, u8 pin)
{
   return venus_map_irq(dev,slot,pin);
}
#elif defined(CONFIG_MACH_WAVESAT_DWW34)
extern int dww34_map_irq(struct pci_dev *dev, u8 slot, u8 pin);
inline int wavesat_pci_irq_map(struct pci_dev *dev, u8 slot, u8 pin)
{
   return dww34_map_irq(dev,slot,pin);
}
#elif defined (CONFIG_MACH_AVILA)
extern int avila_map_irq(struct pci_dev *dev, u8 slot, u8 pin);
inline int wavesat_pci_irq_map(struct pci_dev *dev, u8 slot, u8 pin)
{
   return avila_map_irq(dev,slot,pin);
}
#endif //#if defined (CONFIG_MACH_WAVESAT_DWW34)

EXPORT_SYMBOL(wavesat_pci_irq_map);

#if defined (CONFIG_WAVESAT_DEFINE_NPE_MAC_ADDRESS)

int ws_get_npe_esa (
   uint8_t *mac
   )
{
   print_deb( "%s, %s, %d\n", __FILE__, __FUNCTION__, __LINE__ );
   #if defined (CONFIG_MACH_WAVESAT_DWW34) || defined(CONFIG_MACH_WAVESAT_VENUS)
   print_deb( "%s, %s, %d\n", __FILE__, __FUNCTION__, __LINE__ );
   eeprom_get_npe_esa( 0, &mac[0] );
   #else  // CONFIG_MACH_AVILA
   print_deb( "%s, %s, %d\n", __FILE__, __FUNCTION__, __LINE__ );
   avila_get_npe_esa( 0, &mac[0] );
   #endif
   // don't use broadcast address
   if (mac[0] == 0xff && mac[1] == 0xff && mac[2] == 0xff &&
       mac[3] == 0xff && mac[4] == 0xff && mac[5] == 0xff)
   {
       return 0;
   }
   print_deb( "%s, %s, %d\n", __FILE__, __FUNCTION__, __LINE__ );
   return 1;
}

int ws_set_npe_esa (
   uint8_t *mac
   )
{
   print_deb( "%s, %s, %d\n", __FILE__, __FUNCTION__, __LINE__ );
   #if defined (CONFIG_MACH_WAVESAT_DWW34) || defined(CONFIG_MACH_WAVESAT_VENUS)
   print_deb( "%s, %s, %d\n", __FILE__, __FUNCTION__, __LINE__ );
   eeprom_set_npe_esa( 0, &mac[0] );
   #else  // CONFIG_MACH_AVILA
   print_deb( "%s, %s, %d\n", __FILE__, __FUNCTION__, __LINE__ );
   avila_set_npe_esa( 0, &mac[0] );
   #endif
   return 0;
}

EXPORT_SYMBOL(ws_get_npe_esa);
EXPORT_SYMBOL(ws_set_npe_esa);

#endif // #if defined (CONFIG_WAVESAT_DEFINE_OFDM_MAC_ADDRESS)

#if defined (CONFIG_WAVESAT_DEFINE_NPE_MAC_ADDRESS)

int ws_get_ixp_mac_address (
   uint8_t index,
   uint8_t *mac
   )
{
   print_deb( "%s, %s, %d\n", __FILE__, __FUNCTION__, __LINE__ );
   #if defined (CONFIG_MACH_WAVESAT_DWW34) || defined(CONFIG_MACH_WAVESAT_VENUS)
   print_deb( "%s, %s, %d\n", __FILE__, __FUNCTION__, __LINE__ );
   eeprom_get_npe_esa( index, &mac[0] );
   #else  // CONFIG_MACH_AVILA
   print_deb( "%s, %s, %d\n", __FILE__, __FUNCTION__, __LINE__ );
   avila_get_npe_esa( index, &mac[0] );
   #endif
   // don't use broadcast address
   if (mac[0] == 0xff && mac[1] == 0xff && mac[2] == 0xff &&
       mac[3] == 0xff && mac[4] == 0xff && mac[5] == 0xff)
   {
       return 0;
   }
   print_deb( "%s, %s, %d\n", __FILE__, __FUNCTION__, __LINE__ );
   return 1;
}

int ws_set_ixp_mac_address (
   uint8_t index,
   uint8_t *mac
   )
{
   print_deb( "%s, %s, %d\n", __FILE__, __FUNCTION__, __LINE__ );
   #if defined (CONFIG_MACH_WAVESAT_DWW34) || defined(CONFIG_MACH_WAVESAT_VENUS)
   print_deb( "%s, %s, %d\n", __FILE__, __FUNCTION__, __LINE__ );
   eeprom_set_npe_esa( index, &mac[0] );
   #else  // CONFIG_MACH_AVILA
   print_deb( "%s, %s, %d\n", __FILE__, __FUNCTION__, __LINE__ );
   avila_set_npe_esa( index, &mac[0] );
   #endif
   return 0;
}

EXPORT_SYMBOL(ws_get_ixp_mac_address);
EXPORT_SYMBOL(ws_set_ixp_mac_address);

#endif // #if CONFIG_WAVESAT_DEFINE_NPE_MAC_ADDRESS


MODULE_AUTHOR("Wavesat 2007");
MODULE_DESCRIPTION("Wavesat tools for WiMAX hardware compatible.");



