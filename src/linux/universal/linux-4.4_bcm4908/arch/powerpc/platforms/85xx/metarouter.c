#include <linux/interrupt.h>
#include <asm/time.h>
#include <asm/machdep.h>
#include <asm/of_platform.h>
#include <asm/mtvic.h>
#include <asm/rb_aux.h>
#include <asm/vm.h>

#define BUF_SIZE	256
#define BUF_COUNT	4

hypercall(vm_create_queue, 4, unsigned id, unsigned irq,
	  unsigned tx, unsigned rx);
hypercall(vm_release_queue, 5, unsigned id);
hypercall(vm_running, 6, void);

hypercall(vm_setup_irqs, 14, unsigned *irqs, unsigned count);
hypercall(vm_yield, 16, void);

EXPORT_SYMBOL(vm_yield);

static volatile struct vdma_descr tx_chain[BUF_COUNT];
static volatile struct vdma_descr rx_chain[BUF_COUNT];
static unsigned char tx_buffers[BUF_COUNT][BUF_SIZE];
static unsigned char rx_buffers[BUF_COUNT][BUF_SIZE];

static unsigned cur_tx;
static unsigned cur_rx;

static int send_message(const unsigned char *buf, int len)
{
	unsigned long flags;

	local_irq_save(flags);

	/* drop some data if full buffer */
	while (tx_chain[cur_tx].size & DONE)
		hc_yield();

	len = min_t(int, len, BUF_SIZE);
	memcpy(tx_buffers[cur_tx], buf, len);
	tx_chain[cur_tx].size = len | DONE;

	cur_tx = (cur_tx + 1) % BUF_COUNT;

	local_irq_restore(flags);

	return len;
}

static int recv_message(char *buf, int len)
{
	unsigned long flags;

	local_irq_save(flags);

	if (!(rx_chain[cur_rx].size & DONE)) {
		local_irq_restore(flags);
		return 0;
	}
	
	len = min_t(int, len, rx_chain[cur_rx].size & ~DONE);
	memcpy(buf, rx_buffers[cur_rx], len);

	rx_chain[cur_rx].size = BUF_SIZE;
	cur_rx = (cur_rx + 1) % BUF_COUNT;

	local_irq_restore(flags);

	return len;
}

static irqreturn_t ctrl_interrupt(int irq, void *dev_id)
{
	struct task_struct *init;
	char buf[256];
	int len;

	len = recv_message(buf, sizeof(buf));
	if (len <= 0)
		return IRQ_HANDLED;

	if (strncmp(buf, "restart", len) == 0) {
		printk("RESTART\n");
		init = find_task_by_pid_ns(1, &init_pid_ns);
		if (init)
			send_sig(SIGINT, init, 1);
	} else if (strncmp(buf, "halt", len) == 0) {
	    printk("HALT\n");
		init = find_task_by_pid_ns(1, &init_pid_ns);
		if (init)
			send_sig(SIGWINCH, init, 1);
	}

	return IRQ_HANDLED;
}

static void rbvm_restart(char *command)
{
	char msg[] = "restart";

	send_message(msg, sizeof(msg));

	local_irq_disable();
	while (1);
}

static void rbvm_power_off(void)
{
	char msg[] = "halt";

	send_message(msg, sizeof(msg));
}

static int __init rbvm_probe(void)
{
	char *model;

	model = of_get_flat_dt_prop(of_get_flat_dt_root(), "model", NULL);

	if (!model)
		return 0;

	return strcmp(model, "RB MetaROUTER") == 0;
}

static void __init rbvm_pic_init(void)
{
	extern struct irq_host *virq_host;

	mtvic_init(1);
	vm_setup_irqs(virq_host->host_data, 32);
}

static void rb_idle(void) {
	local_irq_enable();
	vm_yield();
	local_irq_disable();
}

static void __init rbvm_setup_arch(void)
{
	unsigned i;

	for (i = 0; i < BUF_COUNT; ++i) {
		rx_chain[i].addr = (unsigned) rx_buffers[i];
		rx_chain[i].size = BUF_SIZE;
		rx_chain[i].next = (unsigned) &rx_chain[i + 1];
		
		tx_chain[i].addr = (unsigned) tx_buffers[i];
		tx_chain[i].size = 0;
		tx_chain[i].next = (unsigned) &tx_chain[i + 1];
	}
	rx_chain[BUF_COUNT - 1].next = (unsigned) &rx_chain[0];
	tx_chain[BUF_COUNT - 1].next = (unsigned) &tx_chain[0];

	vm_create_queue(0, 0, (unsigned) &tx_chain[0],
			(unsigned) &rx_chain[0]);

	ppc_md.power_save = rb_idle;
}

int __init init_ctrl_interrupt(void)
{
	if (request_irq(get_virq_nr(0), ctrl_interrupt,
			0, "ctrl", (void *) 1));

	return 0;
}
machine_device_initcall(rbvm, init_ctrl_interrupt);

define_machine(rbvm) {
	.name			= "RB MetaROUTER",
	.probe			= rbvm_probe,
	.setup_arch		= rbvm_setup_arch,
	.show_cpuinfo		= rb_show_cpuinfo,
	.get_irq		= mtvic_get_irq,
	.init_IRQ		= rbvm_pic_init,
	.calibrate_decr		= generic_calibrate_decr,
	.restart		= rbvm_restart,
	.power_off		= rbvm_power_off,
	.halt			= rbvm_power_off,
};
