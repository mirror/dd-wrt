#ifndef __LINUX_AR9130_UDC_H
#define __LINUX_AR9130_UDC_H

#include "../gadget/ar9130_defs.h"

#define AR9130_PORTSCX_PORT_RESET                  (0x00000100)
#define AR9130_PORTSCX_PORT_HIGH_SPEED             (0x00000200)
#define AR9130_PORTSCX_PORT_SUSPEND                (0x00000080)

/* Command Register Bit Masks */
#define AR9130_CMD_RUN_STOP                        (0x00000001)
#define AR9130_CMD_CTRL_RESET                      (0x00000002)
#define AR9130_CMD_SETUP_TRIPWIRE_SET              (0x00002000)
#define AR9130_CMD_SETUP_TRIPWIRE_CLEAR           ~(0x00002000)
#define AR9130_CMD_ATDTW_TRIPWIRE_SET              (0x00004000)
#define AR9130_CMD_ATDTW_TRIPWIRE_CLEAR           ~(0x00004000)

#define AR9130_INTR_INT_EN                         (0x00000001)
#define AR9130_INTR_ERR_INT_EN                     (0x00000002)
#define AR9130_INTR_PORT_CHANGE_DETECT_EN          (0x00000004)
#define AR9130_INTR_RESET_EN                       (0x00000040)
#define AR9130_INTR_SOF_UFRAME_EN                  (0x00000080)
#define AR9130_INTR_DEVICE_SUSPEND                 (0x00000100)

#define AR9130_ADDRESS_BIT_SHIFT                   (25)
#define AR9130_DEVADDR_USBADRA                     (24)

/* EPCTRLX[0] Bit Masks */
#define AR9130_EPCTRL_RX_EP_TYPE_SHIFT             (2)
#define AR9130_EPCTRL_TX_EP_TYPE_SHIFT             (18)
#define AR9130_EP_QUEUE_HEAD_NEXT_TERMINATE        (0x00000001)
#define AR9130_EPCTRL_RX_EP_STALL                  (0x00000001)
#define AR9130_EPCTRL_TX_EP_STALL                  (0x00010000)
#define AR9130_EPCTRL_TX_DATA_TOGGLE_RST           (0x00400000)
#define AR9130_EPCTRL_RX_DATA_TOGGLE_RST           (0x00000040)
#define AR9130_EPCTRL_RX_ENABLE                    (0x00000080)
#define AR9130_EPCTRL_TX_ENABLE                    (0x00800000)

/* USB_STS Interrupt Status Register Masks */
#define AR9130_EHCI_STS_SOF                        (0x00000080)
#define AR9130_EHCI_STS_RESET                      (0x00000040)
#define AR9130_EHCI_STS_PORT_CHANGE                (0x00000004)
#define AR9130_EHCI_STS_ERR                        (0x00000002)
#define AR9130_EHCI_STS_INT                        (0x00000001)
#define AR9130_EHCI_STS_SUSPEND                    (0x00000100)
#define AR9130_EHCI_STS_HC_HALTED                  (0x00001000)

/* EndPoint Queue Bit Usage */
#define AR9130_EP_QUEUE_HEAD_MULT_POS              (30)
#define AR9130_TD_NEXT_TERMINATE                   (0x00000001)
#define AR9130_EP_QUEUE_HEAD_ZERO_LEN_TER_SEL      (0x20000000)
#define AR9130_EP_QUEUE_HEAD_IOS                   (0x00008000)
#define AR9130_EP_MAX_PACK_LEN_MASK                (0x07FF0000)

/* dTD Bit Masks */
#define AR9130_TD_STATUS_ACTIVE                    (0x00000080)
#define AR9130_TD_IOC                              (0x00008000)
#define AR9130_TD_RESERVED_FIELDS                  (0x00007F00)
#define AR9130_TD_STATUS_HALTED                    (0x00000040)
#define AR9130_TD_ADDR_MASK                        (0xFFFFFFE0)

#define AR9130_TD_ERROR_MASK                       (0x68)
#define AR9130_TD_LENGTH_BIT_POS                   (16)
#define AR9130_STD_REQ_TYPE                        (0x60)
#define AR9130_EP_MAX_LENGTH_TRANSFER              (0x4000)

/* End Point Queue Head chipIdea Sec-7.1 */
struct ep_qhead {
    __le32  maxPacketLen;
    __le32  curr_dtd;
    __le32  next_dtd;
    __le32  size_ioc_int_status;
    __le32  buff[5];
    __le32  resv;
    __u8    setup_buff[8];
    __u32   resv1[4];
};

/* End Point Transfer Desc chipIdea Sec-7.2 */
struct ep_dtd {
    /* Hardware access fields */
    __le32  next_dtd;
    __le32  size_ioc_status;
    __le32  buff[5];

    /* Software access fields */
    dma_addr_t dtd_dma;
    struct list_head tr_list;
};

#endif /* __LINUX_AR9130_UDC_H */
