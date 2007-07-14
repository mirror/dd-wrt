#ifndef NPE_REGS_H
#define NPE_REGS_H

/* Execution Address  */
#define IX_NPEDL_REG_OFFSET_EXAD             0x00
/* Execution Data */
#define IX_NPEDL_REG_OFFSET_EXDATA           0x04
/* Execution Control */
#define IX_NPEDL_REG_OFFSET_EXCTL            0x08
/* Execution Count */
#define IX_NPEDL_REG_OFFSET_EXCT             0x0C
/* Action Point 0 */
#define IX_NPEDL_REG_OFFSET_AP0              0x10
/* Action Point 1 */
#define IX_NPEDL_REG_OFFSET_AP1              0x14
/* Action Point 2 */
#define IX_NPEDL_REG_OFFSET_AP2              0x18
/* Action Point 3 */
#define IX_NPEDL_REG_OFFSET_AP3              0x1C
/* Watchpoint FIFO */
#define IX_NPEDL_REG_OFFSET_WFIFO            0x20
/* Watch Count */
#define IX_NPEDL_REG_OFFSET_WC               0x24
/* Profile Count */
#define IX_NPEDL_REG_OFFSET_PROFCT           0x28

/* Messaging Status */
#define IX_NPEDL_REG_OFFSET_STAT	     0x2C
/* Messaging Control */
#define IX_NPEDL_REG_OFFSET_CTL	             0x30
/* Mailbox Status */
#define IX_NPEDL_REG_OFFSET_MBST	     0x34
/* messaging in/out FIFO */
#define IX_NPEDL_REG_OFFSET_FIFO	     0x38


#define IX_NPEDL_MASK_ECS_DBG_REG_2_IF       0x00100000
#define IX_NPEDL_MASK_ECS_DBG_REG_2_IE       0x00080000
#define IX_NPEDL_MASK_ECS_REG_0_ACTIVE       0x80000000

#define IX_NPEDL_EXCTL_CMD_NPE_STEP          0x01
#define IX_NPEDL_EXCTL_CMD_NPE_START         0x02
#define IX_NPEDL_EXCTL_CMD_NPE_STOP          0x03
#define IX_NPEDL_EXCTL_CMD_NPE_CLR_PIPE      0x04
#define IX_NPEDL_EXCTL_CMD_CLR_PROFILE_CNT   0x0C
#define IX_NPEDL_EXCTL_CMD_RD_INS_MEM        0x10
#define IX_NPEDL_EXCTL_CMD_WR_INS_MEM        0x11
#define IX_NPEDL_EXCTL_CMD_RD_DATA_MEM       0x12
#define IX_NPEDL_EXCTL_CMD_WR_DATA_MEM       0x13
#define IX_NPEDL_EXCTL_CMD_RD_ECS_REG        0x14
#define IX_NPEDL_EXCTL_CMD_WR_ECS_REG        0x15

#define IX_NPEDL_EXCTL_STATUS_RUN            0x80000000
#define IX_NPEDL_EXCTL_STATUS_STOP           0x40000000
#define IX_NPEDL_EXCTL_STATUS_CLEAR          0x20000000

#define IX_NPEDL_MASK_WFIFO_VALID            0x80000000
#define IX_NPEDL_MASK_STAT_OFNE              0x00010000
#define IX_NPEDL_MASK_STAT_IFNE              0x00080000

#define IX_NPEDL_ECS_DBG_CTXT_REG_0          0x0C
#define IX_NPEDL_ECS_PRI_1_CTXT_REG_0        0x04
#define IX_NPEDL_ECS_PRI_2_CTXT_REG_0        0x08

/* NPE control register bit definitions */
#define IX_NPEMH_NPE_CTL_OFE   (1 << 16) /**< OutFifoEnable */
#define IX_NPEMH_NPE_CTL_IFE   (1 << 17) /**< InFifoEnable */
#define IX_NPEMH_NPE_CTL_OFEWE (1 << 24) /**< OutFifoEnableWriteEnable */
#define IX_NPEMH_NPE_CTL_IFEWE (1 << 25) /**< InFifoEnableWriteEnable */

/* NPE status register bit definitions */
#define IX_NPEMH_NPE_STAT_OFNE  (1 << 16) /**< OutFifoNotEmpty */
#define IX_NPEMH_NPE_STAT_IFNF  (1 << 17) /**< InFifoNotFull */
#define IX_NPEMH_NPE_STAT_OFNF  (1 << 18) /**< OutFifoNotFull */
#define IX_NPEMH_NPE_STAT_IFNE  (1 << 19) /**< InFifoNotEmpty */
#define IX_NPEMH_NPE_STAT_MBINT (1 << 20) /**< Mailbox interrupt */
#define IX_NPEMH_NPE_STAT_IFINT (1 << 21) /**< InFifo interrupt */
#define IX_NPEMH_NPE_STAT_OFINT (1 << 22) /**< OutFifo interrupt */
#define IX_NPEMH_NPE_STAT_WFINT (1 << 23) /**< WatchFifo interrupt */

#endif

