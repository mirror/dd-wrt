/* main.h
 * Header file for the Linux CAN-bus driver.
 * Written by Arnaud Westenberg email:arnaud@wanadoo.nl
 * Rewritten for new CAN queues by Pavel Pisa - OCERA team member
 * email:pisa@cmp.felk.cvut.cz
 * This software is released under the GPL-License.
 * Version lincan-0.3  17 Jun 2004
 */

#include "./can.h"
#include "./constants.h"
#include "./can_sysdep.h"
#include "./can_queue.h"
#if defined(CONFIG_ARCH_KS8695_VSOPENRISC)
#include <linux/vsopenrisc.h>
#include <asm/arch/regs-irq.h>
#endif

#ifdef CAN_DEBUG
	#define DEBUGMSG(fmt,args...) can_printk(KERN_ERR "lincan (debug): " fmt,\
	##args)
#else
	#define DEBUGMSG(fmt,args...)
#endif

#define CANMSG(fmt,args...) can_printk(KERN_ERR "lincan: " fmt,##args)

#if defined(CONFIG_ARCH_KS8695_VSOPENRISC)
/* VS OpenRISC stuff */
#define CONFIG_LINCAN_HW "vscan"
#define CONFIG_LINCAN_IO (VSOPENRISC_VA_EXTIO0_BASE + 0x7000)
#define CONFIG_LINCAN_IRQ KS8695_INTEPLD_CAN

#define CAN_MAX_TRANSMIT_TIMEOUT 1000
#define CAN_WAIT_FOR_TBS 10
#define CAN_USE_LEDS
//#define VSCAN_CHECK_WRITTEN_FRAME /* read ID and TXDATA registers aufter filling them with a frame */
#define ALIGNMENT_FACTOR 4 
#elif
#define ALIGNMENT_FACTOR 1 
#endif



extern can_spinlock_t canuser_manipulation_lock;

/**
 * struct canhardware_t - structure representing pointers to all CAN boards
 * @nr_boards: number of present boards
 * @rtr_queue: RTR - remote transmission request queue (expect some changes there)
 * @rtr_lock: locking for RTR queue
 * @candevice: array of pointers to CAN devices/boards
 */
struct canhardware_t {
	int nr_boards;
	struct rtr_id *rtr_queue;
	can_spinlock_t rtr_lock;
	struct candevice_t *candevice[MAX_HW_CARDS];
};

/**
 * struct candevice_t - CAN device/board structure
 * @hwname: text string with board type
 * @candev_idx: board index in canhardware_t.candevice[]
 * @io_addr: IO/physical MEM address
 * @res_addr: optional reset register port
 * @dev_base_addr: CPU translated IO/virtual MEM address
 * @flags: board flags: %PROGRAMMABLE_IRQ .. interrupt number
 *	can be programmed into board
 * @nr_all_chips: number of chips present on the board
 * @nr_82527_chips: number of Intel 8257 chips 
 * @nr_sja1000_chips: number of Philips SJA100 chips
 * @chip: array of pointers to the chip structures
 * @hwspecops: pointer to board specific operations
 * @hosthardware_p: pointer to the root hardware structure
 * @sysdevptr: union reserved for pointer to bus specific
 *	device structure (case @pcidev is used for PCI devices)
 *
 * The structure represent configuration and state of associated board.
 * The driver infrastructure prepares this structure and calls
 * board type specific board_register() function. The board support provided
 * register function fills right function pointers in @hwspecops structure.
 * Then driver setup calls functions init_hw_data(), init_chip_data(),
 * init_chip_data(), init_obj_data() and program_irq(). Function init_hw_data()
 * and init_chip_data() have to specify number and types of connected chips
 * or objects respectively.
 * The use of @nr_all_chips is preferred over use of fields @nr_82527_chips
 * and @nr_sja1000_chips in the board non-specific functions.
 * The @io_addr and @dev_base_addr is filled from module parameters
 * to the same value. The request_io function can fix-up @dev_base_addr
 * field if virtual address is different than bus address.
 */
struct candevice_t {
	char *hwname;			/* text board type */
	int candev_idx;			/* board index in canhardware_t.candevice[] */
	unsigned long io_addr;		/* IO/physical MEM address */
	unsigned long res_addr;		/* optional reset register port */
	can_ioptr_t dev_base_addr;	/* CPU translated IO/virtual MEM address */
	can_ioptr_t aux_base_addr;	/* CPU translated IO/virtual MEM address */
	unsigned int flags;
	int nr_all_chips;
	int nr_82527_chips;
	int nr_sja1000_chips;
	can_spinlock_t device_lock;
	struct canchip_t *chip[MAX_HW_CHIPS];

	struct hwspecops_t *hwspecops;

	struct canhardware_t *hosthardware_p;
	
	union {
		void *anydev;
	    #ifdef CAN_ENABLE_PCI_SUPPORT
		struct pci_dev *pcidev;
	    #endif /*CAN_ENABLE_PCI_SUPPORT*/
	} sysdevptr;

};

/**
 * struct canchip_t - CAN chip state and type information
 * @chip_type: text string describing chip type
 * @chip_idx: index of the chip in candevice_t.chip[] array
 * @chip_irq: chip interrupt number if any
 * @chip_base_addr: chip base address in the CPU IO or virtual memory space
 * @flags: chip flags: %CHIP_CONFIGURED .. chip is configured,
 *	%CHIP_SEGMENTED .. access to the chip is segmented (mainly for i82527 chips)
 * @clock: chip base clock frequency in Hz
 * @baudrate: selected chip baudrate in Hz
 * @write_register: write chip register function copy
 * @read_register: read chip register function copy
 * @chip_data: pointer for optional chip specific data extension
 * @sja_cdr_reg: SJA specific register -
 *	holds hardware specific options for the Clock Divider
 *	register. Options defined in the sja1000.h file:
 *	%CDR_CLKOUT_MASK, %CDR_CLK_OFF, %CDR_RXINPEN, %CDR_CBP, %CDR_PELICAN
 * @sja_ocr_reg: SJA specific register -
 *	hold hardware specific options for the Output Control
 *	register. Options defined in the sja1000.h file:
 *	%OCR_MODE_BIPHASE, %OCR_MODE_TEST, %OCR_MODE_NORMAL, %OCR_MODE_CLOCK,
 *	%OCR_TX0_LH, %OCR_TX1_ZZ.
 * @int_cpu_reg: Intel specific register -
 *	holds hardware specific options for the CPU Interface
 *	register. Options defined in the i82527.h file:
 *	%iCPU_CEN, %iCPU_MUX, %iCPU_SLP, %iCPU_PWD, %iCPU_DMC, %iCPU_DSC, %iCPU_RST.
 * @int_clk_reg: Intel specific register -
 *	holds hardware specific options for the Clock Out
 *	register. Options defined in the i82527.h file:
 *	%iCLK_CD0, %iCLK_CD1, %iCLK_CD2, %iCLK_CD3, %iCLK_SL0, %iCLK_SL1.
 * @int_bus_reg: Intel specific register -
 *	holds hardware specific options for the Bus Configuration
 *	register. Options defined in the i82527.h file:
 *	%iBUS_DR0, %iBUS_DR1, %iBUS_DT1, %iBUS_POL, %iBUS_CBY.
 * @msgobj: array of pointers to individual communication objects
 * @chipspecops: pointer to the set of chip specific object filled by init_chip_data() function
 * @hostdevice: pointer to chip hosting board
 * @max_objects: maximal number of communication objects connected to this chip
 * @chip_lock: reserved for synchronization of the chip supporting routines
 *	(not used in the current driver version)
 * @worker_thread: chip worker thread ID (RT-Linux specific field)
 * @pend_flags: holds information about pending interrupt and tx_wake() operations
 *	(RT-Linux specific field). Masks values:
 *	%MSGOBJ_TX_REQUEST .. some of the message objects requires tx_wake() call, 
 *	%MSGOBJ_IRQ_REQUEST .. chip interrupt processing required
 *	%MSGOBJ_WORKER_WAKE .. marks, that worker thread should be waked
 *		for some of above reasons
 *
 * The fields @write_register and @read_register are copied from
 * corresponding fields from @hwspecops structure
 * (chip->hostdevice->hwspecops->write_register and 
 * chip->hostdevice->hwspecops->read_register)
 * to speedup can_write_reg() and can_read_reg() functions.
 */
struct canchip_t {
	char *chip_type;
	int chip_idx;	/* chip index in candevice_t.chip[] */
	int chip_irq;
	can_ioptr_t chip_base_addr;
	unsigned int flags;
	long clock; /* Chip clock in Hz */
	long baudrate;

	void (*write_register)(unsigned data, can_ioptr_t address);
	unsigned (*read_register)(can_ioptr_t address);
	
	void *chip_data;
	
	unsigned short sja_cdr_reg; /* sja1000 only! */
	unsigned short sja_ocr_reg; /* sja1000 only! */
	unsigned short int_cpu_reg; /* intel 82527 only! */
	unsigned short int_clk_reg; /* intel 82527 only! */
	unsigned short int_bus_reg; /* intel 82527 only! */

	struct msgobj_t *msgobj[MAX_MSGOBJS];

	struct chipspecops_t *chipspecops;

	struct candevice_t *hostdevice;
	
	int max_objects;	/* 1 for sja1000, 15 for i82527 */

	can_spinlock_t chip_lock;

    #ifdef CAN_WITH_RTL
	pthread_t worker_thread;
	unsigned long pend_flags;
    #endif /*CAN_WITH_RTL*/
	int canStatus;
#if defined(CONFIG_ARCH_KS8695_VSOPENRISC)
	can_ioptr_t epld_addr;
	unsigned char old_epld_val;
#ifdef CAN_USE_LEDS
	char canStatusLED;
#endif /* CAN_USE_LEDS */
#endif /* defined(CONFIG_ARCH_KS8695_VSOPENRISC) */
};

/**
 * struct msgobj_t - structure holding communication object state
 * @obj_base_addr: 
 * @minor: associated device minor number
 * @object: object number in canchip_t structure +1
 * @flags: message object flags
 * @ret: field holding status of the last Tx operation
 * @qends: pointer to message object corresponding ends structure
 * @tx_qedge: edge corresponding to transmitted message
 * @tx_slot: slot holding transmitted message, slot is taken from
 *	canque_test_outslot() call and is freed by canque_free_outslot()
 *	or rescheduled canque_again_outslot()
 * @tx_retry_cnt: transmission attempt counter
 * @tx_timeout: can be used by chip driver to check for the transmission timeout
 * @rx_msg: temporary storage to hold received messages before
 *	calling to canque_filter_msg2edges()
 * @hostchip: pointer to the &canchip_t structure this object belongs to
 * @obj_used: counter of users (associated file structures for Linux
 *	userspace clients) of this object
 * @obj_users: list of user structures of type &canuser_t.
 * @obj_flags: message object specific flags. Masks values:
 *	%MSGOBJ_TX_REQUEST .. the message object requests TX activation
 *	%MSGOBJ_TX_LOCK .. some IRQ routine or callback on some CPU 
 *		is running inside TX activation processing code
 * @rx_preconfig_id: place to store RX message identifier for some chip types
 *		 that reuse same object for TX
 */
struct msgobj_t {
	can_ioptr_t obj_base_addr;
	unsigned int minor;	/* associated device minor number  */
	unsigned int object;	/* object number in canchip_t +1 for debug printk */
	unsigned long obj_flags; 
	int ret;

	struct canque_ends_t *qends;

	struct canque_edge_t *tx_qedge;
	struct canque_slot_t *tx_slot;
	int tx_retry_cnt;
	struct timer_list tx_timeout;

	struct canmsg_t rx_msg;

	struct canchip_t *hostchip;
 
	unsigned long rx_preconfig_id;

	atomic_t obj_used;
	struct list_head obj_users;
};

#define CAN_USER_MAGIC 0x05402033

/**
 * struct canuser_t - structure holding CAN user/client state
 * @flags: used to distinguish Linux/RT-Linux type
 * @peers: for connection into list of object users
 * @qends: pointer to the ends structure corresponding for this user
 * @msgobj: communication object the user is connected to
 * @rx_edge0: default receive queue for filter IOCTL
 * @userinfo: stores user context specific information.
 *	The field @fileinfo.file holds pointer to open device file state structure
 *	for the Linux user-space client applications
 * @magic: magic number to check consistency when pointer is retrieved
 *	from file private field
 */
struct canuser_t {
	unsigned long flags;
	struct list_head peers;
	struct canque_ends_t *qends;
	struct msgobj_t *msgobj;
	struct canque_edge_t *rx_edge0;	/* simplifies IOCTL */
        union {
		struct {
			struct file *file;  /* back ptr to file */
		} fileinfo;
	    #ifdef CAN_WITH_RTL
		struct {
			struct rtl_file *file;
		} rtlinfo;
	    #endif /*CAN_WITH_RTL*/
	} userinfo;
	int magic;
};

/**
 * struct hwspecops_t - hardware/board specific operations
 * @request_io: reserve io or memory range for can board
 * @release_io: free reserved io memory range
 * @reset: hardware reset routine
 * @init_hw_data: called to initialize &candevice_t structure, mainly 
 *	@res_add, @nr_all_chips, @nr_82527_chips, @nr_sja1000_chips
 *	and @flags fields
 * @init_chip_data: called initialize each &canchip_t structure, mainly
 *	@chip_type, @chip_base_addr, @clock and chip specific registers.
 *	It is responsible to setup &canchip_t->@chipspecops functions
 *	for non-standard chip types (type other than "i82527", "sja1000" or "sja1000p")
 * @init_obj_data: called initialize each &msgobj_t structure,
 *	mainly @obj_base_addr field.
 * @program_irq: program interrupt generation hardware of the board
 *	if flag %PROGRAMMABLE_IRQ is present for specified device/board 
 * @write_register: low level write register routine
 * @read_register: low level read register routine
 */
struct hwspecops_t {
	int (*request_io)(struct candevice_t *candev);
	int (*release_io)(struct candevice_t *candev);
	int (*reset)(struct candevice_t *candev);
	int (*init_hw_data)(struct candevice_t *candev);
	int (*init_chip_data)(struct candevice_t *candev, int chipnr);
	int (*init_obj_data)(struct canchip_t *chip, int objnr);
	int (*program_irq)(struct candevice_t *candev);
	void (*write_register)(unsigned data, can_ioptr_t address);
	unsigned (*read_register)(can_ioptr_t address);
};

/**
 * struct chipspecops_t - can controller chip specific operations
 * @chip_config: CAN chip configuration
 * @baud_rate: set communication parameters
 * @standard_mask: setup of mask for message filtering
 * @extended_mask: setup of extended mask for message filtering
 * @message15_mask: set mask of i82527 message object 15
 * @clear_objects: clears state of all message object residing in chip
 * @config_irqs: tunes chip hardware interrupt delivery
 * @pre_read_config: prepares message object for message reception
 * @pre_write_config: prepares message object for message transmission
 * @send_msg: initiate message transmission
 * @remote_request: configures message object and asks for RTR message
 * @check_tx_stat: checks state of transmission engine
 * @wakeup_tx: wakeup TX processing
 * @filtch_rq: optional routine for propagation of outgoing edges filters to HW
 * @enable_configuration: enable chip configuration mode
 * @disable_configuration: disable chip configuration mode
 * @set_btregs: configures bitrate registers
 * @attach_to_chip: attaches to the chip, setups registers and possibly state informations
 * @release_chip: called before chip structure removal if %CHIP_ATTACHED is set
 * @start_chip: starts chip message processing
 * @stop_chip: stops chip message processing
 * @irq_handler: interrupt service routine
 * @irq_accept: optional fast irq accept routine responsible for blocking further interrupts
 */
struct chipspecops_t {
	int (*chip_config)(struct canchip_t *chip);
	int (*baud_rate)(struct canchip_t *chip, int rate, int clock, int sjw,
						int sampl_pt, int flags);
	int (*standard_mask)(struct canchip_t *chip, unsigned short code, 
							unsigned short mask);
	int (*extended_mask)(struct canchip_t *chip, unsigned long code, 
							unsigned long mask);
	int (*message15_mask)(struct canchip_t *chip, unsigned long code, 
							unsigned long mask);
	int (*clear_objects)(struct canchip_t *chip);
	int (*config_irqs)(struct canchip_t *chip, short irqs);
	int (*pre_read_config)(struct canchip_t *chip, struct msgobj_t *obj);
	int (*pre_write_config)(struct canchip_t *chip, struct msgobj_t *obj,
							struct canmsg_t *msg);
	int (*send_msg)(struct canchip_t *chip, struct msgobj_t *obj,
							struct canmsg_t *msg);
	int (*remote_request)(struct canchip_t *chip, struct msgobj_t *obj);
	int (*check_tx_stat)(struct canchip_t *chip);
	int (*wakeup_tx)(struct canchip_t *chip, struct msgobj_t *obj);
	int (*filtch_rq)(struct canchip_t *chip, struct msgobj_t *obj);
	int (*enable_configuration)(struct canchip_t *chip);
	int (*disable_configuration)(struct canchip_t *chip);
	int (*set_btregs)(struct canchip_t *chip, unsigned short btr0, 
							unsigned short btr1);
	int (*attach_to_chip)(struct canchip_t *chip);
	int (*release_chip)(struct canchip_t *chip);
	int (*start_chip)(struct canchip_t *chip);
	int (*stop_chip)(struct canchip_t *chip);
	int (*irq_handler)(int irq, struct canchip_t *chip);
	int (*irq_accept)(int irq, struct canchip_t *chip);
};

struct mem_addr {
	void *address;
	struct mem_addr *next;
	size_t size;
};

/* Structure for the RTR queue */
struct rtr_id {
	unsigned long id;
	struct canmsg_t *rtr_message;
	wait_queue_head_t rtr_wq;
	struct rtr_id *next;
};

extern int major;
extern int minor[MAX_TOT_CHIPS];
extern int extended;
extern int baudrate[MAX_TOT_CHIPS];
extern int irq[MAX_IRQ];
extern char *hw[MAX_HW_CARDS];
extern unsigned long io[MAX_HW_CARDS];
extern long clockfreq[MAX_HW_CARDS];
extern int processlocal;

extern struct canhardware_t *hardware_p;
extern struct canchip_t *chips_p[MAX_TOT_CHIPS];
extern struct msgobj_t *objects_p[MAX_TOT_MSGOBJS];

extern struct mem_addr *mem_head;


#if defined(CONFIG_OC_LINCAN_PORTIO_ONLY)
extern inline void can_write_reg(const struct canchip_t *chip, unsigned char data, unsigned reg_offs)
{
	can_outb(data, chip->chip_base_addr+reg_offs);
}
extern inline unsigned can_read_reg(const struct canchip_t *chip, unsigned reg_offs)
{
	return can_inb(chip->chip_base_addr+reg_offs);
}
extern inline void canobj_write_reg(const struct canchip_t *chip, const struct msgobj_t *obj,
				unsigned char data, unsigned reg_offs)
{
	can_outb(data, obj->obj_base_addr+reg_offs);
}
extern inline unsigned canobj_read_reg(const struct canchip_t *chip, const struct msgobj_t *obj,
				unsigned reg_offs)
{
	return can_inb(obj->obj_base_addr+reg_offs);
}

#elif defined(CONFIG_OC_LINCAN_MEMIO_ONLY)
extern inline void can_write_reg(const struct canchip_t *chip, unsigned char data, unsigned reg_offs)
{
	can_writeb(data, chip->chip_base_addr+reg_offs);
}
extern inline unsigned can_read_reg(const struct canchip_t *chip, unsigned reg_offs)
{
	return can_readb(chip->chip_base_addr+reg_offs);
}
extern inline void canobj_write_reg(const struct canchip_t *chip, const struct msgobj_t *obj,
				unsigned char data, unsigned reg_offs)
{
	can_writeb(data, obj->obj_base_addr+reg_offs);
}
extern inline unsigned canobj_read_reg(const struct canchip_t *chip, const struct msgobj_t *obj,
				unsigned reg_offs)
{
	return can_readb(obj->obj_base_addr+reg_offs);
}

#else /*CONFIG_OC_LINCAN_DYNAMICIO*/
#ifndef CONFIG_OC_LINCAN_DYNAMICIO
#define CONFIG_OC_LINCAN_DYNAMICIO
#endif
/* Inline function to write to the hardware registers. The argument reg_offs is 
 * relative to the memory map of the chip and not the absolute memory reg_offs.
 */
extern inline void can_write_reg(const struct canchip_t *chip, unsigned char data, unsigned reg_offs)
{
	can_ioptr_t address_to_write;
	address_to_write = chip->chip_base_addr+(reg_offs*ALIGNMENT_FACTOR);
	chip->write_register(data, address_to_write);
}

extern inline unsigned can_read_reg(const struct canchip_t *chip, unsigned reg_offs)
{
	can_ioptr_t address_to_read;
	address_to_read = chip->chip_base_addr+(reg_offs*ALIGNMENT_FACTOR);
	return chip->read_register(address_to_read);
}

extern inline void canobj_write_reg(const struct canchip_t *chip, const struct msgobj_t *obj,
				unsigned char data, unsigned reg_offs)
{
	can_ioptr_t address_to_write;
	address_to_write = obj->obj_base_addr+(reg_offs*ALIGNMENT_FACTOR);
	chip->write_register(data, address_to_write);
}

extern inline unsigned canobj_read_reg(const struct canchip_t *chip, const struct msgobj_t *obj,
				unsigned reg_offs)
{
	can_ioptr_t address_to_read;
	address_to_read = obj->obj_base_addr+(reg_offs*ALIGNMENT_FACTOR);
	return chip->read_register(address_to_read);
}

#endif /*CONFIG_OC_LINCAN_DYNAMICIO*/

int can_base_addr_fixup(struct candevice_t *candev, can_ioptr_t new_base);
int can_request_io_region(unsigned long start, unsigned long n, const char *name);
void can_release_io_region(unsigned long start, unsigned long n);
int can_request_mem_region(unsigned long start, unsigned long n, const char *name);
void can_release_mem_region(unsigned long start, unsigned long n);

struct boardtype_t {
	const char *boardtype;
	int (*board_register)(struct hwspecops_t *hwspecops);
	int irqnum;
};

const struct boardtype_t* boardtype_find(const char *str);

int can_check_dev_taken(void *anydev);

#if defined(can_gettimeofday) && defined(CAN_MSG_VERSION_2) && 1
static inline
void can_filltimestamp(canmsg_tstamp_t *ptimestamp)
{
	can_gettimeofday(ptimestamp);
}
#else /* No timestamp support, set field to zero */
static inline
void can_filltimestamp(canmsg_tstamp_t *ptimestamp)
{
    #ifdef CAN_MSG_VERSION_2
	ptimestamp->tv_sec = 0;
	ptimestamp->tv_usec = 0;
    #else /* CAN_MSG_VERSION_2 */
	*ptimestamp = 0;
    #endif /* CAN_MSG_VERSION_2 */

}
#endif /* End of timestamp source selection */

#ifdef CAN_WITH_RTL
extern int can_rtl_priority;
#endif /*CAN_WITH_RTL*/
