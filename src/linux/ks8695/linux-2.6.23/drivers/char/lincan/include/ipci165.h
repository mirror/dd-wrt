/* ipci165.h
 * Linux CAN-bus device driver for IXXAT iPC-I 165 (PCI) compatible HW.
 * Written for new CAN driver version by Radim Kalas
 * email:kalas@unicontrols.cz
 * This software is released under the GPL-License.
 * Version lincan-0.3  17 Jun 2004
 */ 

#include "../include/can_sysdep.h"
#include "../include/kthread.h"

/* PCI device identification */
#define IPCI165_VENDOR_ID    PCI_VENDOR_ID_PLX
#define IPCI165_DEVICE_ID    PCI_DEVICE_ID_PLX_9050

#define IPCI165_SUBSYSTEM_ID 0x1067   /* subsystem ID for IXXAT iPC-I 165 card */
#define CP350_SUBSYSTEM_ID   0x1089   /* subsystem ID for PEP/Kontron CP350 card */


/*****************************************************************************
 * Control Registers Memory specific defines                                 *
 *****************************************************************************/
#define CRM_SIZE             0x80     /* region size */

#define CRM_UCR              0x50     /* user control register */
#define CRM_ICR              0x4C     /* interrupt control register */

/*****************************************************************************
 * DP-RAM (PCI Memory Window 2) specific defines                             *
 *****************************************************************************/
#define DPRAM_SIZE                 8*1024   /* DP-RAM size - PCI Memory Window 2 */

#define OF_CMD_BUFFER                   0   /* Offset of Command Buffer \
                                               (normal mode) */

#define OF_CH1_TX_QUEUE              0x50   /* offset to TX queue for channel 1 */
#define OF_CH2_TX_QUEUE             0x438   /* offset to TX queue for channel 2 */
#define OF_CH1_RX_QUEUE             0x820   /* offset to RX queue for channel 1 */
#define OF_CH2_RX_QUEUE             0xc08   /* offset to RX queue for channel 2 */

#define OF_STATUS_BUFFER            0xff0   /* offset to status buffer */

#define BCI_QUEUE_SIZE                 50   /* Size of TX and RX message queues \
                                               (number of messages) */
#define BCI_MSG_SIZE                   20   /* Size of one message */

/*****************************************************************************
 * Bootstrap loader specific defines                                         *
 *****************************************************************************/

/* identification strings offsets & lengths */
#define BOARD_NAME_OFS               0x00   /* Name (string) */
#define BOARD_NAME_LEN                  7   /* String length */

#define HW_VERSION_OFS               0x20   /* Version (string) */
#define HW_VERSION_LEN                  6   /* String length */

#define MODE_OFS                    0x102   /* Mode (string) */
#define MODE_LEN                       14   /* String length */

#define TYPE_LEN                       11   /* Type length */

/* FW loader command buffer offsets */
#define OF_LD_SYNC                  0x100   /* Synchronization flag */
#define OF_LD_CMD                   0x101   /* Command code */
#define OF_LD_NUM                   0x102   /* Size of FW block */
#define OF_LD_ADDRESS               0x104   /* Start Address of FW block */
#define OF_LD_DATA                  0x108   /* Data of FW block */

/* FW commands */
#define LD_CMD_DOWNLOAD                 1   /* Download FW block */
#define LD_CMD_START_FW                 2   /* Start FW */

/*****************************************************************************
 * BCI specific defines                                                      *
 *****************************************************************************/

/* BCI command buffer offsets */
#define OF_BCI_SYNC                  0x00   /* Synchronization flag */
#define OF_BCI_NUM                   0x01   /* Command size */
#define OF_BCI_CMD                   0x04   /* Command code */
#define OF_BCI_DATA                  0x05   /* Command data */
#define BCI_CMD_MAX_LEN                76   /* Max command length */

/* BCI commands */
#define CMD_ID                          1   /* Get identification string */
#define CMD_VERSION                     2   /* Get version number string */
#define CMD_TEST                        3   /* Test the command buffer, invert data bytes */
#define CMD_INIT_CAN                    4   /* Initialization of the CAN controller */
#define CMD_START_CAN                   6   /* Start the CAN controller */
#define CMD_STOP_CAN                    7   /* Stop the CAN controller */
#define CMD_RESET_CAN                   8   /* Reset the CAN controller */
#define CMD_SET_EXT_FILTER_MASK         9   /* Set global filter mask */
#define CMD_CONFIG_RX_QUEUE            11   /* Config receive queue mode */
#define CMD_GET_BOARD_INFO             12   /* Get board information */
#define CMD_START_TIMER                13   /* Start cyclic timer */
#define CMD_STOP_TIMER                 14   /* Stop cyclic timer */
#define CMD_SET_ACC_MASK               15   /* Set acceptance mask */

/* CMD_BOARD_INFO offsets */
#define BOARD_INFO_SIZE                26   /* Size */
#define OF_BOARD_INFO_VER               1   /* Version in BCD, UINT16 */
#define OF_BOARD_INFO_CHIPS             3   /* Num of chips, UINT16 */
#define OF_BOARD_INFO_CHIP1_TYPE        5   /* Type of first chip, UINT8[10] */
#define OF_BOARD_INFO_CHIP2_TYPE       15   /* Type of second chip, UINT8[10] */

/* baud rates BTR0, BTR1 for SJA1000 */
#define BCI_10KB                0x67,0x2F
#define BCI_20KB                0x53,0x2F
#define BCI_50KB                0x47,0x2F
#define BCI_100KB               0x43,0x2F
#define BCI_125KB               0x03,0x1C
#define BCI_250KB               0x01,0x1C
#define BCI_500KB               0x00,0x1C
#define BCI_1000KB              0x00,0x14

#define CAN_FRAME_MIN_BIT_LEN          47   /* Min no of bits in CAN standard data frame */
#define CAN_FRAME_MAX_BIT_LEN         111   /* Max no of bits in CAN standard data frame */

/* BCI_CONFIG_RX Queue Modes */
#define BCI_POLL_MODE                   0
#define BCI_LATENCY_MODE                1
#define BCI_THROUGHPUT_MODE             2

/* BCI queue info */
struct bci_queue_t {
  int         idx;                         /* points to the active record in buffer */
  can_ioptr_t addr;                        /* start address of the message queue    */
};

/* ipci165 chip data */
struct ipci165_chip_t {
  struct bci_queue_t rx_queue;             /* RX queue info */
  struct bci_queue_t tx_queue;             /* TX queue info */
  kthread_t  kthread;                      /* kernel thread info  */
  long       flags;                        /* flag for syncing with kernel thread */
};

#define CHIP_FLAG_BUS_OFF               1  /* bus-off signal to kthread */
#define CHIP_FLAG_RESET                 2  /* chip is being reseted */


/* RX & TX Queue message structure */
#define BCI_MSG_STATUS                  0  /* status       (U8) */
#define BCI_MSG_NUM                     1  /* size         (U8) */
#define BCI_MSG_TIMESTAMP               2  /* timestamp    (U32)*/
#define BCI_MSG_TYPE                    6  /* message type (U8) */

/* CAN message */
#define BCI_MSG_FRAME                   7  /* frame info   (U8) */
#define BCI_MSG_ID                      8  /* ID 11/29b    (U16/U32) */
#define BCI_MSG_STD_DATA               10  /* message data */
#define BCI_MSG_EXT_DATA               12  /* message data */

/* status message */
#define BCI_MSG_CAN_STATUS              8  /* status info  (U16) */

#define BCI_MSG_STATUS_FREE             0  /* message buffer is free */
#define BCI_MSG_STATUS_FULL             1  /* message buffer is used */

#define BCI_MSG_TYPE_CAN                0  /* message containing CAN frame */
#define BCI_MSG_TYPE_STATUS             1  /* message containing status info */

#define BCI_MSG_FRAME_RTR            0x40  /* RTR flag          */
#define BCI_MSG_FRAME_EXT            0x80  /* Extended Frame    */

#define BCI_TIMESTAMP_RES             125  /* Time stamp resolution in usec */

/* status buffer */
#define OF_CAN1_STATUS    (OF_STATUS_BUFFER + 0)
#define OF_CAN1_LOAD      (OF_STATUS_BUFFER + 2)
#define OF_CAN2_STATUS    (OF_STATUS_BUFFER + 4)
#define OF_CAN2_LOAD      (OF_STATUS_BUFFER + 6)
#define OF_CPU_LOAD       (OF_STATUS_BUFFER + 8)
#define OF_LIFE_COUNTER   (OF_STATUS_BUFFER + 10)

#define BCI_CAN_STATUS_INIT           0x0001 /* ctrl is in init mode */
#define BCI_CAN_STATUS_WARNING_LEVEL  0x0002 /* warning level has been reached */
#define BCI_CAN_STATUS_BUS_OFF        0x0004 /* ctrl disconnected from net */
#define BCI_CAN_STATUS_DATA_OVERRUN   0x0008 /* overrun of can message happend */
#define BCI_CAN_STATUS_RX             0x0010 /* ctrl is recieving */
#define BCI_CAN_STATUS_TX             0x0020 /* ctrl is sending */
#define BCI_CAN_STATUS_QUEUE_OVERRUN  0x0100 /* queue overrun */
