#ifndef __LINUX_ATH_UDC_H
#define __LINUX_ATH_UDC_H

#include "../gadget/ath_defs.h"

#define ATH_USB_PORTSCX_PORT_RESET			(0x00000100)
#define ATH_USB_PORTSCX_PORT_HIGH_SPEED			(0x00000200)
#define ATH_USB_PORTSCX_PORT_SUSPEND			(0x00000080)

/* Command Register Bit Masks */
#define ATH_USB_CMD_RUN_STOP				(0x00000001)
#define ATH_USB_CMD_CTRL_RESET				(0x00000002)
#define ATH_USB_CMD_SETUP_TRIPWIRE_SET			(0x00002000)
#define ATH_USB_CMD_SETUP_TRIPWIRE_CLEAR		~(0x00002000)
#define ATH_USB_CMD_ATDTW_TRIPWIRE_SET			(0x00004000)
#define ATH_USB_CMD_ATDTW_TRIPWIRE_CLEAR		~(0x00004000)

#define ATH_USB_INTR_INT_EN				(0x00000001)
#define ATH_USB_INTR_ERR_INT_EN				(0x00000002)
#define ATH_USB_INTR_PORT_CHANGE_DETECT_EN		(0x00000004)
#define ATH_USB_INTR_RESET_EN				(0x00000040)
#define ATH_USB_INTR_SOF_UFRAME_EN			(0x00000080)
#define ATH_USB_INTR_DEVICE_SUSPEND			(0x00000100)

#define ATH_USB_ADDRESS_BIT_SHIFT			(25)
#define ATH_USB_DEVADDR_USBADRA				(24)

/* EPCTRLX[0] Bit Masks */
#define ATH_USB_EPCTRL_RX_EP_TYPE_SHIFT			(2)
#define ATH_USB_EPCTRL_TX_EP_TYPE_SHIFT			(18)
#define ATH_USB_EP_QUEUE_HEAD_NEXT_TERMINATE		(0x00000001)
#define ATH_USB_EPCTRL_RX_EP_STALL			(0x00000001)
#define ATH_USB_EPCTRL_TX_EP_STALL			(0x00010000)
#define ATH_USB_EPCTRL_TX_DATA_TOGGLE_RST		(0x00400000)
#define ATH_USB_EPCTRL_RX_DATA_TOGGLE_RST		(0x00000040)
#define ATH_USB_EPCTRL_RX_ENABLE			(0x00000080)
#define ATH_USB_EPCTRL_TX_ENABLE			(0x00800000)

/* USB_STS Interrupt Status Register Masks */
#define ATH_USB_EHCI_STS_SOF				(0x00000080)
#define ATH_USB_EHCI_STS_RESET				(0x00000040)
#define ATH_USB_EHCI_STS_PORT_CHANGE			(0x00000004)
#define ATH_USB_EHCI_STS_ERR				(0x00000002)
#define ATH_USB_EHCI_STS_INT				(0x00000001)
#define ATH_USB_EHCI_STS_SUSPEND			(0x00000100)
#define ATH_USB_EHCI_STS_HC_HALTED			(0x00001000)

/* EndPoint Queue Bit Usage */
#define ATH_USB_EP_QUEUE_HEAD_MULT_POS			(30)
#define ATH_USB_TD_NEXT_TERMINATE			(0x00000001)
#define ATH_USB_EP_QUEUE_HEAD_ZERO_LEN_TER_SEL		(0x20000000)
#define ATH_USB_EP_QUEUE_HEAD_IOS			(0x00008000)
#define ATH_USB_EP_MAX_PACK_LEN_MASK			(0x07FF0000)

/* dTD Bit Masks */
#define ATH_USB_TD_STATUS_ACTIVE			(0x00000080)
#define ATH_USB_TD_IOC					(0x00008000)
#define ATH_USB_TD_RESERVED_FIELDS			(0x00007F00)
#define ATH_USB_TD_STATUS_HALTED			(0x00000040)
#define ATH_USB_TD_ADDR_MASK				(0xFFFFFFE0)

#define ATH_USB_TD_ERROR_MASK				(0x68)
#define ATH_USB_TD_LENGTH_BIT_POS			(16)
#define ATH_USB_STD_REQ_TYPE				(0x60)
#define ATH_USB_EP_MAX_LENGTH_TRANSFER			(0x4000)

/* End Point Queue Head chipIdea Sec-7.1 */
struct ep_qhead {
	__le32	maxPacketLen,
		curr_dtd,
		next_dtd,
		size_ioc_int_status,
		buff[5],
		resv;
	__u8	setup_buff[8];
	__u32	resv1[4];
};

/* End Point Transfer Desc chipIdea Sec-7.2 */
struct ep_dtd {
	/* Hardware access fields */
	__le32	next_dtd,
		size_ioc_status,
		buff[5];

	/* Software access fields */
	dma_addr_t dtd_dma;
	struct ep_dtd *next;
	struct list_head tr_list;
};

#endif /* __LINUX_ATH_USB_UDC_H */
