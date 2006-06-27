/*
 * vrc4171_card.c, NEC VRC4171 Card Controller driver for Socket Services.
 *
 * Copyright (C) 2003  Yoichi Yuasa <yuasa@hh.iij4u.or.jp>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */
#include <linux/init.h>
#include <linux/ioport.h>
#include <linux/irq.h>
#include <linux/module.h>
#include <linux/spinlock.h>
#include <linux/sched.h>
#include <linux/types.h>

#include <asm/io.h>
#include <asm/vr41xx/vrc4171.h>

#include <pcmcia/ss.h>

#include "i82365.h"

MODULE_DESCRIPTION("NEC VRC4171 Card Controllers driver for Socket Services");
MODULE_AUTHOR("Yoichi Yuasa <yuasa@hh.iij4u.or.jp>");
MODULE_LICENSE("GPL");

#define CARD_MAX_SLOTS		2
#define CARD_SLOTA		0
#define CARD_SLOTB		1
#define CARD_SLOTB_OFFSET	0x40

#define CARD_MEM_START		0x10000000
#define CARD_MEM_END		0x13ffffff
#define CARD_MAX_MEM_OFFSET	0x3ffffff
#define CARD_MAX_MEM_SPEED	1000

#define CARD_CONTROLLER_INDEX	0x03e0
#define CARD_CONTROLLER_DATA	0x03e1
#define CARD_CONTROLLER_SIZE	2
 /* Power register */
  #define VPP_GET_VCC		0x01
  #define POWER_ENABLE		0x10
 #define CARD_VOLTAGE_SENSE	0x1f
  #define VCC_3VORXV_CAPABLE	0x00
  #define VCC_XV_ONLY		0x01
  #define VCC_3V_CAPABLE	0x02
  #define VCC_5V_ONLY		0x03
 #define CARD_VOLTAGE_SELECT	0x2f
  #define VCC_3V		0x01
  #define VCC_5V		0x00
  #define VCC_XV		0x02
  #define VCC_STATUS_3V		0x02
  #define VCC_STATUS_5V		0x01
  #define VCC_STATUS_XV		0x03
 #define GLOBAL_CONTROL		0x1e
  #define EXWRBK		0x04
  #define IRQPM_EN		0x08
  #define CLRPMIRQ		0x10

#define IO_MAX_MAPS	2
#define MEM_MAX_MAPS	5

enum {
	SLOTB_PROBE = 0,
	SLOTB_NOPROBE_IO,
	SLOTB_NOPROBE_MEM,
	SLOTB_NOPROBE_ALL
};

typedef struct vrc4171_socket {
	int noprobe;
	void (*handler)(void *, unsigned int);
	void *info;
	socket_cap_t cap;
	spinlock_t event_lock;
	uint16_t events;
	struct socket_info_t *pcmcia_socket;
	struct tq_struct tq_task;
	char name[24];
	int csc_irq;
	int io_irq;
} vrc4171_socket_t;

static vrc4171_socket_t vrc4171_sockets[CARD_MAX_SLOTS];
static int vrc4171_slotb = SLOTB_IS_NONE;
static unsigned int vrc4171_irq;
static uint16_t vrc4171_irq_mask = 0xdeb8;

extern struct socket_info_t *pcmcia_register_socket(int slot,
                                                    struct pccard_operations *vtable,
                                                    int use_bus_pm);
extern void pcmcia_unregister_socket(struct socket_info_t *s);

static inline uint8_t exca_read_byte(int slot, uint8_t index)
{
	if (slot == CARD_SLOTB)
		index += CARD_SLOTB_OFFSET;

	outb(index, CARD_CONTROLLER_INDEX);
	return inb(CARD_CONTROLLER_DATA);
}

static inline uint16_t exca_read_word(int slot, uint8_t index)
{
	uint16_t data;

	if (slot == CARD_SLOTB)
		index += CARD_SLOTB_OFFSET;

	outb(index++, CARD_CONTROLLER_INDEX);
	data = inb(CARD_CONTROLLER_DATA);

	outb(index, CARD_CONTROLLER_INDEX);
	data |= ((uint16_t)inb(CARD_CONTROLLER_DATA)) << 8;

	return data;
}

static inline uint8_t exca_write_byte(int slot, uint8_t index, uint8_t data)
{
	if (slot == CARD_SLOTB)
		index += CARD_SLOTB_OFFSET;

	outb(index, CARD_CONTROLLER_INDEX);
	outb(data, CARD_CONTROLLER_DATA);

	return data;
}

static inline uint16_t exca_write_word(int slot, uint8_t index, uint16_t data)
{
	if (slot == CARD_SLOTB)
		index += CARD_SLOTB_OFFSET;

	outb(index++, CARD_CONTROLLER_INDEX);
	outb(data, CARD_CONTROLLER_DATA);

	outb(index, CARD_CONTROLLER_INDEX);
	outb((uint8_t)(data >> 8), CARD_CONTROLLER_DATA);

	return data;
}

static inline int search_nonuse_irq(void)
{
	int i;

	for (i = 0; i < 16; i++) {
		if (vrc4171_irq_mask & (1 << i)) {
			vrc4171_irq_mask &= ~(1 << i);
			return i;
		}
	}

	return -1;
}

static int pccard_init(unsigned int slot)
{
	vrc4171_socket_t *socket = &vrc4171_sockets[slot];

	socket->cap.features |= SS_CAP_PCCARD | SS_CAP_PAGE_REGS;
	socket->cap.irq_mask = 0;
	socket->cap.pci_irq = vrc4171_irq;
	socket->cap.map_size = 0x1000;
	socket->events = 0;
	spin_lock_init(socket->event_lock);
	socket->csc_irq = search_nonuse_irq();
	socket->io_irq = search_nonuse_irq();

	return 0;
}

static int pccard_suspend(unsigned int slot)
{
	return -EINVAL;
}

static int pccard_register_callback(unsigned int slot,
                                    void (*handler)(void *, unsigned int),
                                    void *info)
{
	vrc4171_socket_t *socket;

	if (slot >= CARD_MAX_SLOTS)
		return -EINVAL;

	socket = &vrc4171_sockets[slot];

	socket->handler = handler;
	socket->info = info;

	if (handler)
		MOD_INC_USE_COUNT;
	else
		MOD_DEC_USE_COUNT;

	return 0;
}

static int pccard_inquire_socket(unsigned int slot, socket_cap_t *cap)
{
	vrc4171_socket_t *socket;

	if (slot >= CARD_MAX_SLOTS || cap == NULL)
		return -EINVAL;

	socket = &vrc4171_sockets[slot];

	*cap = socket->cap;

	return 0;
}

static int pccard_get_status(unsigned int slot, u_int *value)
{
	uint8_t status, sense;
	u_int val = 0;

	if (slot >= CARD_MAX_SLOTS || value == NULL)
		return -EINVAL;

	status = exca_read_byte(slot, I365_STATUS);
	if (exca_read_byte(slot, I365_INTCTL) & I365_PC_IOCARD) {
		if (status & I365_CS_STSCHG)
			val |= SS_STSCHG;
	} else {
		if (!(status & I365_CS_BVD1))
			val |= SS_BATDEAD;
		else if ((status & (I365_CS_BVD1 | I365_CS_BVD2)) == I365_CS_BVD1)
			val |= SS_BATWARN;
	}
	if ((status & I365_CS_DETECT) == I365_CS_DETECT)
		val |= SS_DETECT;
	if (status & I365_CS_WRPROT)
		val |= SS_WRPROT;
	if (status & I365_CS_READY)
		val |= SS_READY;
	if (status & I365_CS_POWERON)
		val |= SS_POWERON;

	sense = exca_read_byte(slot, CARD_VOLTAGE_SENSE);
	switch (sense) {
	case VCC_3VORXV_CAPABLE:
		val |= SS_3VCARD | SS_XVCARD;
		break;
	case VCC_XV_ONLY:
		val |= SS_XVCARD;
		break;
	case VCC_3V_CAPABLE:
		val |= SS_3VCARD;
		break;
	default:
		/* 5V only */
		break;
	}

	*value = val;

	return 0;
}

static inline u_char get_Vcc_value(uint8_t voltage)
{
	switch (voltage) {
	case VCC_STATUS_3V:
		return 33;
	case VCC_STATUS_5V:
		return 50;
	default:
		break;
	}

	return 0;
}

static inline u_char get_Vpp_value(uint8_t power, u_char Vcc)
{
	if ((power & 0x03) == 0x01 || (power & 0x03) == 0x02)
		return Vcc;

	return 0;
}

static int pccard_get_socket(unsigned int slot, socket_state_t *state)
{
	vrc4171_socket_t *socket;
	uint8_t power, voltage, control, cscint;

	if (slot >= CARD_MAX_SLOTS || state == NULL)
		return -EINVAL;

	socket = &vrc4171_sockets[slot];

	power = exca_read_byte(slot, I365_POWER);
	voltage = exca_read_byte(slot, CARD_VOLTAGE_SELECT);

	state->Vcc = get_Vcc_value(voltage);
	state->Vpp = get_Vpp_value(power, state->Vcc);

	state->flags = 0;
	if (power & POWER_ENABLE)
		state->flags |= SS_PWR_AUTO;
	if (power & I365_PWR_OUT)
		state->flags |= SS_OUTPUT_ENA;

	control = exca_read_byte(slot, I365_INTCTL);
	if (control & I365_PC_IOCARD)
		state->flags |= SS_IOCARD;
	if (!(control & I365_PC_RESET))
		state->flags |= SS_RESET;

        cscint = exca_read_byte(slot, I365_CSCINT);
	state->csc_mask = 0;
	if (state->flags & SS_IOCARD) {
		if (cscint & I365_CSC_STSCHG)
			state->flags |= SS_STSCHG;
	} else {
		if (cscint & I365_CSC_BVD1)  
			state->csc_mask |= SS_BATDEAD;
		if (cscint & I365_CSC_BVD2)  
			state->csc_mask |= SS_BATWARN;
	}
	if (cscint & I365_CSC_READY)
		state->csc_mask |= SS_READY;
	if (cscint & I365_CSC_DETECT)
		state->csc_mask |= SS_DETECT;

	return 0;
}

static inline uint8_t set_Vcc_value(u_char Vcc)
{
	switch (Vcc) {
	case 33:
		return VCC_3V;
	case 50:
		return VCC_5V;
	}

	/* Small voltage is chosen for safety. */
	return VCC_3V;
}

static int pccard_set_socket(unsigned int slot, socket_state_t *state)
{
	vrc4171_socket_t *socket;
	uint8_t voltage, power, control, cscint;

	if (slot >= CARD_MAX_SLOTS ||
	    (state->Vpp != state->Vcc && state->Vpp != 0) ||
	    (state->Vcc != 50 && state->Vcc != 33 && state->Vcc != 0))
		return -EINVAL;

	socket = &vrc4171_sockets[slot];

	spin_lock_irq(&socket->event_lock);

	voltage = set_Vcc_value(state->Vcc);
	exca_write_byte(slot, CARD_VOLTAGE_SELECT, voltage);

	power = POWER_ENABLE;
	if (state->Vpp == state->Vcc)
		power |= VPP_GET_VCC;
	if (state->flags & SS_OUTPUT_ENA)
		power |= I365_PWR_OUT;
	exca_write_byte(slot, I365_POWER, power);

	control = 0;
	if (state->io_irq != 0)
		control |= socket->io_irq;
	if (state->flags & SS_IOCARD)
		control |= I365_PC_IOCARD;
	if (state->flags & SS_RESET)
		control	&= ~I365_PC_RESET;
	else
		control |= I365_PC_RESET;
	exca_write_byte(slot, I365_INTCTL, control);

        cscint = 0;
        exca_write_byte(slot, I365_CSCINT, cscint);
	exca_read_byte(slot, I365_CSC);	/* clear CardStatus change */
	if (state->csc_mask != 0)
		cscint |= socket->csc_irq << 8;
	if (state->flags & SS_IOCARD) {
		if (state->csc_mask & SS_STSCHG)
			cscint |= I365_CSC_STSCHG;
	} else {
		if (state->csc_mask & SS_BATDEAD)
			cscint |= I365_CSC_BVD1;
		if (state->csc_mask & SS_BATWARN)
			cscint |= I365_CSC_BVD2;
	}
	if (state->csc_mask & SS_READY)
		cscint |= I365_CSC_READY;
	if (state->csc_mask & SS_DETECT)
		cscint |= I365_CSC_DETECT;
        exca_write_byte(slot, I365_CSCINT, cscint);

	spin_unlock_irq(&socket->event_lock);

	return 0;
}

static int pccard_get_io_map(unsigned int slot, struct pccard_io_map *io)
{
	vrc4171_socket_t *socket;
	uint8_t ioctl, addrwin;
	u_char map;

	if (slot >= CARD_MAX_SLOTS || io == NULL ||
	    io->map >= IO_MAX_MAPS)
		return -EINVAL;

	socket = &vrc4171_sockets[slot];
	map = io->map;

	io->start = exca_read_word(slot, I365_IO(map)+I365_W_START);
	io->stop = exca_read_word(slot, I365_IO(map)+I365_W_STOP);

	ioctl = exca_read_byte(slot, I365_IOCTL);
	if (io->flags & I365_IOCTL_WAIT(map))
		io->speed = 1;
	else
		io->speed = 0;

	io->flags = 0;
	if (ioctl & I365_IOCTL_16BIT(map))
		io->flags |= MAP_16BIT;
	if (ioctl & I365_IOCTL_IOCS16(map))
		io->flags |= MAP_AUTOSZ;
	if (ioctl & I365_IOCTL_0WS(map))
		io->flags |= MAP_0WS;

	addrwin = exca_read_byte(slot, I365_ADDRWIN);
	if (addrwin & I365_ENA_IO(map))
		io->flags |= MAP_ACTIVE;

	return 0;
}

static int pccard_set_io_map(unsigned int slot, struct pccard_io_map *io)
{
	vrc4171_socket_t *socket;
	uint8_t ioctl, addrwin;
	u_char map;

	if (slot >= CARD_MAX_SLOTS ||
	    io == NULL || io->map >= IO_MAX_MAPS ||
	    io->start > 0xffff || io->stop > 0xffff || io->start > io->stop)
		return -EINVAL;

	socket = &vrc4171_sockets[slot];
	map = io->map;

	addrwin = exca_read_byte(slot, I365_ADDRWIN);
	if (addrwin & I365_ENA_IO(map)) {
		addrwin &= ~I365_ENA_IO(map);
		exca_write_byte(slot, I365_ADDRWIN, addrwin);
	}

	exca_write_word(slot, I365_IO(map)+I365_W_START, io->start);
	exca_write_word(slot, I365_IO(map)+I365_W_STOP, io->stop);

	ioctl = 0;
	if (io->speed > 0)
		ioctl |= I365_IOCTL_WAIT(map);
	if (io->flags & MAP_16BIT)
		ioctl |= I365_IOCTL_16BIT(map);
	if (io->flags & MAP_AUTOSZ)
		ioctl |= I365_IOCTL_IOCS16(map);
	if (io->flags & MAP_0WS)
		ioctl |= I365_IOCTL_0WS(map);
	exca_write_byte(slot, I365_IOCTL, ioctl);

	if (io->flags & MAP_ACTIVE) {
		addrwin |= I365_ENA_IO(map);
		exca_write_byte(slot, I365_ADDRWIN, addrwin);
	}

	return 0;
}

static int pccard_get_mem_map(unsigned int slot, struct pccard_mem_map *mem)
{
	vrc4171_socket_t *socket;
	uint8_t addrwin;
	u_long start, stop;
	u_int offset;
	u_char map;

	if (slot >= CARD_MAX_SLOTS || mem == NULL || mem->map >= MEM_MAX_MAPS)
		return -EINVAL;

	socket = &vrc4171_sockets[slot];
	map = mem->map;

	mem->flags = 0;
	mem->speed = 0;

	addrwin = exca_read_byte(slot, I365_ADDRWIN);
	if (addrwin & I365_ENA_MEM(map))
		mem->flags |= MAP_ACTIVE;

	start = exca_read_word(slot, I365_MEM(map)+I365_W_START);
	if (start & I365_MEM_16BIT)
		mem->flags |= MAP_16BIT;
	mem->sys_start = (start & 0x3fffUL) << 12;

	stop = exca_read_word(slot, I365_MEM(map)+I365_W_STOP);
	if (start & I365_MEM_WS0)
		mem->speed += 1;
	if (start & I365_MEM_WS1)
		mem->speed += 2;
	mem->sys_stop = ((stop & 0x3fffUL) << 12) + 0xfffUL;

	offset = exca_read_word(slot, I365_MEM(map)+I365_W_OFF);
	if (offset & I365_MEM_REG)
		mem->flags |= MAP_ATTRIB;
	if (offset & I365_MEM_WRPROT)
		mem->flags |= MAP_WRPROT;
	mem->card_start = (offset & 0x3fffUL) << 12;

	mem->sys_start += CARD_MEM_START;
	mem->sys_stop += CARD_MEM_START;

	return 0;
}

static int pccard_set_mem_map(unsigned int slot, struct pccard_mem_map *mem)
{
	vrc4171_socket_t *socket;
	uint16_t start, stop, offset;
	uint8_t addrwin;
	u_char map;

	if (slot >= CARD_MAX_SLOTS ||
	    mem == NULL || mem->map >= MEM_MAX_MAPS ||
	    mem->sys_start < CARD_MEM_START || mem->sys_start > CARD_MEM_END ||
	    mem->sys_stop < CARD_MEM_START || mem->sys_stop > CARD_MEM_END ||
	    mem->sys_start > mem->sys_stop ||
	    mem->card_start > CARD_MAX_MEM_OFFSET ||
	    mem->speed > CARD_MAX_MEM_SPEED)
		return -EINVAL;

	socket = &vrc4171_sockets[slot];
	map = mem->map;

	addrwin = exca_read_byte(slot, I365_ADDRWIN);
	if (addrwin & I365_ENA_MEM(map)) {
		addrwin &= ~I365_ENA_MEM(map);
		exca_write_byte(slot, I365_ADDRWIN, addrwin);
	}

	start = (mem->sys_start >> 12) & 0x3fff;
	if (mem->flags & MAP_16BIT)
		start |= I365_MEM_16BIT;
	exca_write_word(slot, I365_MEM(map)+I365_W_START, start);

	stop = (mem->sys_stop >> 12) & 0x3fff;
	switch (mem->speed) {
	case 0:
		break;
	case 1:
		stop |= I365_MEM_WS0;
		break;
	case 2:
		stop |= I365_MEM_WS1;
		break;
	default:
		stop |= I365_MEM_WS0 | I365_MEM_WS1;
		break;
	}
	exca_write_word(slot, I365_MEM(map)+I365_W_STOP, stop);

	offset = (mem->card_start >> 12) & 0x3fff;
	if (mem->flags & MAP_ATTRIB)
		offset |= I365_MEM_REG;
	if (mem->flags & MAP_WRPROT)
		offset |= I365_MEM_WRPROT;
	exca_write_word(slot, I365_MEM(map)+I365_W_OFF, offset);

	if (mem->flags & MAP_ACTIVE) {
		addrwin |= I365_ENA_MEM(map);
		exca_write_byte(slot, I365_ADDRWIN, addrwin);
	}

	return 0;
}

static void pccard_proc_setup(unsigned int slot, struct proc_dir_entry *base)
{          
}

static struct pccard_operations vrc4171_pccard_operations = {
	.init			= pccard_init,
	.suspend		= pccard_suspend,
	.register_callback	= pccard_register_callback,
	.inquire_socket		= pccard_inquire_socket,
	.get_status		= pccard_get_status,
	.get_socket		= pccard_get_socket,
	.set_socket		= pccard_set_socket,
	.get_io_map		= pccard_get_io_map,
	.set_io_map		= pccard_set_io_map,
	.get_mem_map		= pccard_get_mem_map,
	.set_mem_map		= pccard_set_mem_map,
	.proc_setup		= pccard_proc_setup,
};

static void pccard_bh(void *data)
{
	vrc4171_socket_t *socket = (vrc4171_socket_t *)data;
	uint16_t events;

	spin_lock_irq(&socket->event_lock);
	events = socket->events;
	socket->events = 0;
	spin_unlock_irq(&socket->event_lock);
 
	if (socket->handler)
		socket->handler(socket->info, events);
}

static inline uint16_t get_events(int slot)
{
	uint16_t events = 0;
	uint8_t status, csc;

	status = exca_read_byte(slot, I365_STATUS);
	csc = exca_read_byte(slot, I365_CSC);

	if (exca_read_byte(slot, I365_INTCTL) & I365_PC_IOCARD) {
		if ((csc & I365_CSC_STSCHG) && (status & I365_CS_STSCHG))
			events |= SS_STSCHG;
	} else {
		if (csc & (I365_CSC_BVD1 | I365_CSC_BVD2)) {
			if (!(status & I365_CS_BVD1))
				events |= SS_BATDEAD;
			else if ((status & (I365_CS_BVD1 | I365_CS_BVD2)) == I365_CS_BVD1)
				events |= SS_BATWARN;
		}
	}
	if ((csc & I365_CSC_READY) && (status & I365_CS_READY))
		events |= SS_READY;
	if ((csc & I365_CSC_DETECT) && ((status & I365_CS_DETECT) == I365_CS_DETECT))
		events |= SS_DETECT;

	return events;
}

static void pccard_status_change(int slot, vrc4171_socket_t *socket)
{
	uint16_t events;

	socket->tq_task.routine = pccard_bh;
	socket->tq_task.data = socket;

	events = get_events(slot);
	if (events) {
		spin_lock(&socket->event_lock);
		socket->events |= events;
		spin_unlock(&socket->event_lock);
		schedule_task(&socket->tq_task);
	}
}

static void pccard_interrupt(int irq, void *dev_id, struct pt_regs *regs)
{
	vrc4171_socket_t *socket;
	uint16_t status;

	status = vrc4171_get_irq_status();
	if (status & IRQ_A) {
		socket = &vrc4171_sockets[CARD_SLOTA];
		if (socket->noprobe == SLOTB_PROBE) {
			if (status & (1 << socket->csc_irq))
				pccard_status_change(CARD_SLOTA, socket);
		}
	}

	if (status & IRQ_B) {
		socket = &vrc4171_sockets[CARD_SLOTB];
		if (socket->noprobe == SLOTB_PROBE) {
			if (status & (1 << socket->csc_irq))
				pccard_status_change(CARD_SLOTB, socket);
		}
	}
}

static inline void reserve_using_irq(int slot)
{
	unsigned int irq;

	irq = exca_read_byte(slot, I365_INTCTL);
	irq &= 0x0f;
	vrc4171_irq_mask &= ~(1 << irq);

	irq = exca_read_byte(slot, I365_CSCINT);
	irq = (irq & 0xf0) >> 4;
	vrc4171_irq_mask &= ~(1 << irq);
}

static int __devinit vrc4171_add_socket(int slot)
{
	vrc4171_socket_t *socket;

	if (slot >= CARD_MAX_SLOTS)
		return -EINVAL;

	socket = &vrc4171_sockets[slot];
	if (socket->noprobe != SLOTB_PROBE) {
		uint8_t addrwin;

		switch (socket->noprobe) {
		case SLOTB_NOPROBE_MEM:
			addrwin = exca_read_byte(slot, I365_ADDRWIN);
			addrwin &= 0x1f;
			exca_write_byte(slot, I365_ADDRWIN, addrwin);
			break;
		case SLOTB_NOPROBE_IO:
			addrwin = exca_read_byte(slot, I365_ADDRWIN);
			addrwin &= 0xc0;
			exca_write_byte(slot, I365_ADDRWIN, addrwin);
			break;
		default:
			break;
		}

		reserve_using_irq(slot);

		return 0;
	}

	sprintf(socket->name, "NEC VRC4171 Card Slot %1c", 'A' + slot);

	socket->pcmcia_socket = pcmcia_register_socket(slot, &vrc4171_pccard_operations, 1);
	if (socket->pcmcia_socket == NULL)
		return -ENOMEM;

	exca_write_byte(slot, I365_ADDRWIN, 0);

	exca_write_byte(slot, GLOBAL_CONTROL, 0);

	return 0;
}

static void vrc4171_remove_socket(int slot)
{
	vrc4171_socket_t *socket;

	if (slot >= CARD_MAX_SLOTS)
		return;

	socket = &vrc4171_sockets[slot];

	if (socket->pcmcia_socket != NULL) {
		pcmcia_unregister_socket(socket->pcmcia_socket);
		socket->pcmcia_socket = NULL;
	}
}

static int __devinit vrc4171_card_setup(char *options)
{
	if (options == NULL || *options == '\0')
		return 0;

	if (strncmp(options, "irq:", 4) == 0) {
		int irq;
		options += 4;
		irq = simple_strtoul(options, &options, 0);
		if (irq >= 0 && irq < NR_IRQS)
			vrc4171_irq = irq;

		if (*options != ',')
			return 0;
		options++;
	}

	if (strncmp(options, "slota:", 6) == 0) {
		options += 6;
		if (*options != '\0') {
			if (strncmp(options, "noprobe", 7) == 0) {
				vrc4171_sockets[CARD_SLOTA].noprobe = 1;
				options += 7;
			}

			if (*options != ',')
				return 0;
			options++;
		} else
			return 0;

	}

	if (strncmp(options, "slotb:", 6) == 0) {
		options += 6;
		if (*options != '\0') {
			if (strncmp(options, "pccard", 6) == 0) {
				vrc4171_slotb = SLOTB_IS_PCCARD;
				options += 6;
			} else if (strncmp(options, "cf", 2) == 0) {
				vrc4171_slotb = SLOTB_IS_CF;
				options += 2;
			} else if (strncmp(options, "flashrom", 8) == 0) {
				vrc4171_slotb = SLOTB_IS_FLASHROM;
				options += 8;
			} else if (strncmp(options, "none", 4) == 0) {
				vrc4171_slotb = SLOTB_IS_NONE;
				options += 4;
			}

			if (*options != ',')
				return 0;
			options++;

			if ( strncmp(options, "memnoprobe", 10) == 0)
				vrc4171_sockets[CARD_SLOTB].noprobe = SLOTB_NOPROBE_MEM;
			if ( strncmp(options, "ionoprobe", 9) == 0)
				vrc4171_sockets[CARD_SLOTB].noprobe = SLOTB_NOPROBE_IO;
			if ( strncmp(options, "noprobe", 7) == 0)
				vrc4171_sockets[CARD_SLOTB].noprobe = SLOTB_NOPROBE_ALL;
		}
	}

	return 0;
}

__setup("vrc4171_card=", vrc4171_card_setup);

static int __devinit vrc4171_card_init(void)
{
	int retval, slot;

	vrc4171_set_multifunction_pin(vrc4171_slotb);

	if (request_region(CARD_CONTROLLER_INDEX, CARD_CONTROLLER_SIZE,
	                       "NEC VRC4171 Card Controller") == NULL)
		return -EBUSY;

	for (slot = 0; slot < CARD_MAX_SLOTS; slot++) {
		if (slot == CARD_SLOTB && vrc4171_slotb == SLOTB_IS_NONE)
			break;

		retval = vrc4171_add_socket(slot);
		if (retval != 0)
			return retval;
	}

	retval = request_irq(vrc4171_irq, pccard_interrupt, SA_SHIRQ,
	                     "NEC VRC4171 Card Controller", vrc4171_sockets);
	if (retval < 0) {
		for (slot = 0; slot < CARD_MAX_SLOTS; slot++)
			vrc4171_remove_socket(slot);

		return retval;
	}

	printk(KERN_INFO "NEC VRC4171 Card Controller, connected to IRQ %d\n", vrc4171_irq);

	return 0;
}

static void __devexit vrc4171_card_exit(void)
{
	int slot;

	for (slot = 0; slot < CARD_MAX_SLOTS; slot++)
		vrc4171_remove_socket(slot);

	release_region(CARD_CONTROLLER_INDEX, CARD_CONTROLLER_SIZE);
}

module_init(vrc4171_card_init);
module_exit(vrc4171_card_exit);
