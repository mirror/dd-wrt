#include <asm/time.h>
#include <asm/machdep.h>
#include <asm/mpic.h>
#include <asm/mtvic.h>
#include <linux/of.h>
#include <linux/of_platform.h>
#include <sysdev/fsl_pci.h>
#include <asm/rb_aux.h>
#include <asm/vm.h>

hypercall(hv_yield, 16, void);

#ifdef MT_DEBUG
void rb1000_putc(char c)
{

        while (!(*(volatile unsigned char *) 0xe0004505 & 0x20));

	*(char *) 0xe0004500 = c;
}

void rb1000_puts(char *str)
{
        while (*str) {
	        if (*str == '\n') rb1000_putc('\r');
		rb1000_putc(*str);
		++str;
	}
}

void rb1000_printk(const char *fmt, ...) {
	va_list args;
	char buf[256];

	va_start(args, fmt);
	vsnprintf(buf, sizeof(buf), fmt, args);
	va_end(args);

	rb1000_puts(buf);
}

void rb1000_init(void)
{
#if 0
        if (inited) return;

        settlbcam(3, 0xf0000000, 0xe0000000, 0x10000, _PAGE_IO, 0);
	inited = 1;
#endif
}
#endif

struct of_prop {
    const char *name;
    unsigned size;
    unsigned values[2];
    struct property prop;
};

struct of_prop crypto_properties[] = {
    { "reg", 8, { 0x30000, 0x10000 } },
    { "interrupts", 8,  { 0x1d, 2 } },
    { "interrupt-parent", 4, { 0x700 } },
    { "fsl,num-channels", 4, { 4 } },
    { "fsl,channel-fifo-len", 4, { 24 } },
    { "fsl,exec-units-mask", 4, { 0xfe } },
    { "fsl,descriptor-types-mask", 4, { 0x12b0ebf } },
    { NULL, 0, { 0 } }
};

static void add_of_property(struct device_node *np, struct property *pp,
			    const char *name, unsigned size, void *value)
{
	memset(pp, 0, sizeof(struct property));
	pp->name = (char *) name;
	pp->length = size;
	pp->value = value;

	prom_add_property(np, pp);
}

static void add_crypto_of_node(void)
{
	static struct device_node crypto_node;
	static struct property comp_prop;

	struct device_node *np;
	struct device_node *sp;
	struct of_prop *p;

	sp = of_find_node_by_type(NULL, "serial");
	if (!sp)
		return;

	np = &crypto_node;
	memset(np, 0, sizeof(struct device_node));
	np->full_name = "crypto@30000";
	kref_init(&np->kref);
	
	add_of_property(np, &comp_prop, "compatible", 11, "fsl,sec2.0");

	for (p = crypto_properties; p->name; ++p) {
		add_of_property(np, &p->prop, p->name, p->size, p->values);
	}

	np->parent = sp->parent;
	of_attach_node(np);
}

static void __init rb1000_pic_init(void)
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
			  1, 0, " OpenPIC ");
	
	for (i = 0; i < 31; ++i) {
		if (i == 11 || i == 12) {
		    /* Ext IRQ4 and IRQ5 is mapped to 11 & 12 respectively */
			mpic_assign_isu(mpic, i,
					r.start + 0x10000 + (i - 11 + 4) * 0x20);
		} else if (i == 30) {
			mpic_assign_isu(mpic, i, r.start + 0x10000 + 7 * 0x20);
		} else {
			mpic_assign_isu(mpic, i, r.start + 0x10200 + i * 0x20);
		}
	}
	mpic_assign_isu(mpic, 31, r.start + 0x1120);

	of_node_put(np);
	mpic_init(mpic);
}

static void rb_idle(void) {
	extern void e500_idle(void);
	int err;

	local_irq_enable();
	err = hv_yield();
	local_irq_disable();
	if (err < 0) e500_idle();
}

static void __init rb1000_setup_arch(void)
{
	mtspr(SPRN_HID0, 1 << 14); /* set TBEN */
	mb();

	/* matches MPC8548E, MPC8547E, MPC8545E */
	if ((mfspr(SPRN_SVR) >> 16) == 0x8039)
		add_crypto_of_node();

	ppc_md.power_save = rb_idle;
}

static int __init rb1000_probe(void)
{
	char *model;

	model = of_get_flat_dt_prop(of_get_flat_dt_root(), "model", NULL);

	if (!model)
		return 0;

	return strcmp(model, "RB1000") == 0;
}

define_machine(rb1000) {
	.name			= "RB1000",
	.probe			= rb1000_probe,
	.setup_arch		= rb1000_setup_arch,
	.init_IRQ		= rb1000_pic_init,
	.show_cpuinfo		= rb_show_cpuinfo,
	.get_irq		= rb_get_irq,
	.restart		= rb_restart,
	.calibrate_decr		= generic_calibrate_decr,
	.pcibios_fixup_bus	= fsl_pcibios_fixup_bus,
};
