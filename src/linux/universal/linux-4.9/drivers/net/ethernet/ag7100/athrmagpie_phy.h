
/*
 * will include all the phy emulation register set that is needed
 * by magpie
 * most of the time host need to set only speed, duplex mode
 * All of these can be set through MII control register
 *
 */
/* configuration registers */
#define ATHR_MAGPIE_CFG_REG_BASE 0x0000

#define ATHR_MAGPIE_REG_MAC_CFG1    (ATHR_MAGPIE_CFG_REG_BASE + 0x00)   /*MAC config 1*/
#define ATHR_MAGPIE_REG_MAC_CFG2    (ATHR_MAGPIE_CFG_REG_BASE + 0x04)   /*MAC config 2*/
#define ATHR_MAGPIE_REG_IPG_IFG     (ATHR_MAGPIE_CFG_REG_BASE + 0x08)   /*Inter-packet-gap*/
#define ATHR_MAGPIE_REG_HALF_DPLX   (ATHR_MAGPIE_CFG_REG_BASE + 0x0c)   /*Half duplex*/
#define ATHR_MAGPIE_REG_MAX_FRAME   (ATHR_MAGPIE_CFG_REG_BASE + 0x10)   /*Max frame length*/
#define ATHR_MAGPIE_REG_MII_CFG     (ATHR_MAGPIE_CFG_REG_BASE + 0x20)   /*MII mgmt config*/
#define ATHR_MAGPIE_REG_MII_CMD     (ATHR_MAGPIE_CFG_REG_BASE + 0x24)   /*MII mgmt command*/
#define ATHR_MAGPIE_REG_MII_ADDR    (ATHR_MAGPIE_CFG_REG_BASE + 0x28)   /*MII mgmt address*/
#define ATHR_MAGPIE_REG_MII_CTRL    (ATHR_MAGPIE_CFG_REG_BASE + 0x2c)   /*MII mgmt control*/
#define ATHR_MAGPIE_REG_MII_STAT    (ATHR_MAGPIE_CFG_REG_BASE + 0x30)   /*MII mgmt status*/
#define ATHR_MAGPIE_REG_MII_PSTAT   (ATHR_MAGPIE_CFG_REG_BASE + 0x34)   /*MII mgmt Phy status/ind*/
#define ATHR_MAGPIE_REG_IF_CTRL     (ATHR_MAGPIE_CFG_REG_BASE + 0x38)   /*Interface control*/
#define ATHR_MAGPIE_REG_IF_STAT     (ATHR_MAGPIE_CFG_REG_BASE + 0x3c)   /*Interface status*/
#define ATHR_MAGPIE_REG_MAC_ADDR1   (ATHR_MAGPIE_CFG_REG_BASE + 0x40)   /*MAC address 1*/
#define ATHR_MAGPIE_REG_MAC_ADDR2   (ATHR_MAGPIE_CFG_REG_BASE + 0x44)   /*MAC address 2*/
#define ATHR_MAGPIE_REG_FIFO_CFG0   (ATHR_MAGPIE_CFG_REG_BASE + 0x48)   /*FIFO config reg0*/
#define ATHR_MAGPIE_REG_FIFO_CFG1   (ATHR_MAGPIE_CFG_REG_BASE + 0x4c)   /*FIFO config reg1*/
#define ATHR_MAGPIE_REG_FIFO_CFG2   (ATHR_MAGPIE_CFG_REG_BASE + 0x50)   /*FIFO config reg2*/
#define ATHR_MAGPIE_REG_FIFO_CFG3   (ATHR_MAGPIE_CFG_REG_BASE + 0x54)   /*FIFO config reg3*/
#define ATHR_MAGPIE_REG_FIFO_CFG4   (ATHR_MAGPIE_CFG_REG_BASE + 0x58)   /*FIFO config reg4*/
#define ATHR_MAGPIE_REG_FIFO_CFG5   (ATHR_MAGPIE_CFG_REG_BASE + 0x5c)   /*FIFO config reg5*/
#define ATHR_MAGPIE_REG_FIFO_RAM0   (ATHR_MAGPIE_CFG_REG_BASE + 0x60)   /*FIFO RAM access reg0*/
#define ATHR_MAGPIE_REG_FIFO_RAM1   (ATHR_MAGPIE_CFG_REG_BASE + 0x64)   /*FIFO RAM access reg1*/
#define ATHR_MAGPIE_REG_FIFO_RAM2   (ATHR_MAGPIE_CFG_REG_BASE + 0x68)   /*FIFO RAM access reg2*/
#define ATHR_MAGPIE_REG_FIFO_RAM3   (ATHR_MAGPIE_CFG_REG_BASE + 0x6c)   /*FIFO RAM access reg3*/
#define ATHR_MAGPIE_REG_FIFO_RAM4   (ATHR_MAGPIE_CFG_REG_BASE + 0x70)   /*FIFO RAM access reg4*/
#define ATHR_MAGPIE_REG_FIFO_RAM5   (ATHR_MAGPIE_CFG_REG_BASE + 0x74)   /*FIFO RAM access reg5*/
#define ATHR_MAGPIE_REG_FIFO_RAM6   (ATHR_MAGPIE_CFG_REG_BASE + 0x78)   /*FIFO RAM access reg6*/
#define ATHR_MAGPIE_REG_FIFO_RAM7   (ATHR_MAGPIE_CFG_REG_BASE + 0x7c)   /*FIFO RAM access reg7*/

/* control file specifications */
#define ATHR_MAGPIE_CTRL_REG_BASE 0x8000  /* real offset 0x54100 on magpie */

#define ATHR_MAGPIE_MII_CTRL      (ATHR_MAGPIE_CTRL_REG_BASE+0x0)
#define ATHR_MAGPIE_MII_CTRL_SPEED_1000 (2<<2)
#define ATHR_MAGPIE_MII_CTRL_SPEED_100  (1<<2)
#define ATHR_MAGPIE_MII_CTRL_SPEED_10   (0<<2)

#define ATHR_MAGPIE_MII_CTRL_MODE_GMII  0x0
#define ATHR_MAGPIE_MII_CTRL_MODE_MII   0x1
#define ATHR_MAGPIE_MII_CTRL_MODE_RGMII 0x2
#define ATHR_MAGPIE_MII_CTRL_MODE_RMII  0x3
