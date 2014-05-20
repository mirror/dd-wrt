#include <linux/module.h>
#include <linux/io.h>
#include <linux/interrupt.h>
#include <linux/seq_file.h>
#include <linux/of.h>
#include <mm/mmu_decl.h>
#include <asm/machdep.h>
#include <linux/of_platform.h>
#include <asm/mpic.h>
#include <asm/mtvic.h>

#define GPIO(x) (0x80000000 >> (x))
static unsigned *gpio_data = NULL;
static unsigned *picr = NULL;

const unsigned *beep1;
const unsigned *beep2;

#define GT0_BASE_COUNT (picr + (0x1110 / 4))

static void ioremap_from_node(const unsigned *property, unsigned **ptr) {
    struct resource res;
    struct device_node *nd;
    
    nd = of_find_node_by_phandle(property[0]);
    if (!nd || of_address_to_resource(nd, 0, &res)) return;
    of_node_put(nd);

    *ptr = ioremap_nocache(res.start, res.end - res.start + 1);
}

irqreturn_t beeper_irq(int irq, void *ptr)
{
	static int toggle = 1;
	if (toggle) {
	    out_be32(gpio_data, in_be32(gpio_data) & ~GPIO(beep1[0]));
	    out_be32(gpio_data, in_be32(gpio_data) & ~GPIO(beep2[0]));
	}
	else {
	    out_be32(gpio_data, in_be32(gpio_data) | GPIO(beep1[0]));
	    out_be32(gpio_data, in_be32(gpio_data) | GPIO(beep2[0]));
	}
	toggle ^= 1; 
	return IRQ_HANDLED;
}

void consume(int x) { }

extern unsigned long ppc_tb_freq;
static unsigned long ppc_tb_freq_kHz;

static void __init rb_beeper_init(void)
{
	struct device_node *beeper;
	unsigned interrupt;
	const unsigned *int_p;
	const unsigned *gpio;
	
	beeper = of_find_node_by_name(NULL, "beeper");
	if (!beeper)
		return;

	beep1 = of_get_property(beeper, "beep1", NULL);
	beep2 = of_get_property(beeper, "beep2", NULL);
	gpio  = of_get_property(beeper, "gpio", NULL);
	int_p = of_get_property(beeper, "interrupt-parent", NULL);
	
	ioremap_from_node(gpio, &gpio_data);
	ioremap_from_node(int_p, &picr);
	
	ppc_tb_freq_kHz = (ppc_tb_freq / 1000);
	
	interrupt = irq_of_parse_and_map(beeper, 0);
	if (interrupt != NO_IRQ)
		consume(request_irq(interrupt, beeper_irq, 
				    IRQF_TRIGGER_RISING,
				    "beeper", NULL));
}

void rbppc_beep(unsigned freq) {
	if (!ppc_tb_freq_kHz)
		return;

	out_be32(GT0_BASE_COUNT,
		 freq ? (500 * ppc_tb_freq_kHz) / freq : 0x80000000);
}
EXPORT_SYMBOL(rbppc_beep);

static struct resource rb_led_resources[2] = {
	[0] = {
		.flags		= IORESOURCE_IO,
	},
	[1] = {
		.name		= "user-led",
	},
};

static const unsigned rb_uled[2] = { 0x400, 0x1c };

static int __init rb_leds_init(void)
{
	struct device_node *np;
	const unsigned *uled = rb_uled;

	np = of_find_node_by_name(NULL, "led");
	if (np) {
		uled = of_get_property(np, "user_led", NULL);
		of_node_put(np);
		if (!uled) {
			printk("rbppc led error: "
			       "user_led property is missing\n");
			return -1;
		}
	}

	rb_led_resources[1].start = uled[1];
	rb_led_resources[1].end = uled[1];

	np = of_find_node_by_phandle(uled[0]);
	if (!np) {
		printk("rbppc led error: no gpio<%x> node found\n", *uled);
		return -1;
	}
	if (of_address_to_resource(np, 0, &rb_led_resources[0])) {
		of_node_put(np);
		printk("rbppc led error: no reg property in gpio found\n");
		return -1;
	}
	of_node_put(np);

	platform_device_register_simple("rbppc-led", 0,
					rb_led_resources, 2);
	return 0;
}

static struct of_device_id __initdata of_bus_ids[] = {
	{ .type = "soc", },
	{},
};

static int __init rb_declare_of_platform_devices(void)
{
	struct device_node *np;
	unsigned idx;

	np = of_find_node_by_name(NULL, "nand");
	if (np) of_platform_device_create(np, "nand", NULL);

	np = of_find_node_by_name(NULL, "nand_fcm");
	if (np) of_platform_device_create(np, "nand_fcm", NULL);

	idx = 0;
	for_each_node_by_type(np, "rb,cf") {
		char dev_name[12];
		snprintf(dev_name, sizeof(dev_name), "cf.%u", idx);
		of_platform_device_create(np, dev_name, NULL);
		++idx;		
	}

	rb_beeper_init();
	rb_leds_init();

	of_platform_bus_probe(NULL, of_bus_ids, NULL);

	return 0;
}

device_initcall(rb_declare_of_platform_devices);

void __init rb_pic_init(void)
{
	struct device_node *np;
	struct resource r;
	struct mpic *mpic;
	void *gcr;
	unsigned i;

	mtvic_init(0);

	np = of_find_node_by_type(NULL, "open-pic");

	if (!np)
		return;

	if (of_address_to_resource(np, 0, &r)) {
		printk(KERN_ERR "mpic error: no region specified\n");
		of_node_put(np);
		return;
	}

	gcr = ioremap(r.start + 0x1020, 4);
	out_be32(gcr, in_be32(gcr) | (1 << 29));
	iounmap(gcr);

	mpic = mpic_alloc(np, r.start, MPIC_BIG_ENDIAN,
			  4, 0, " OpenPIC ");
	for (i = 0; i < 80; i += 4) {
		mpic_assign_isu(mpic, i / 4, r.start + 0x10000 + i * 0x20);
	}
	mpic_assign_isu(mpic, 80 / 4, r.start + 0x1120);

	of_node_put(np);
	mpic_init(mpic);
}

void rb_show_cpuinfo(struct seq_file *m)
{
	seq_printf(m, "Vendor\t\t: Mikrotik\n");
	seq_printf(m, "Machine\t\t: %s\n", ppc_md.name);
	seq_printf(m, "Memory\t\t: %u MB\n", total_memory / (1024 * 1024));
}

void rb_restart(char *cmd)
{
	local_irq_disable();

	mtmsr(mfmsr() | 0x00000200);
	mtspr(0x134, mfspr(0x134) | 0x70000000);
}
