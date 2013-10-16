#include <asm/time.h>
#include <asm/machdep.h>
#include <asm/mpic.h>
#include <asm/mtvic.h>
#include <linux/of_platform.h>
#include <sysdev/fsl_pci.h>
#include <asm/rb_aux.h>

static void __init rb800_setup_arch(void)
{
	mtspr(SPRN_HID0, 1 << 14); /* set TBEN */
	mb();

#ifdef CONFIG_PCI
	rb_init_pci();
#endif
}

static int __init rb800_probe(void)
{
	char *model;

	model = of_get_flat_dt_prop(of_get_flat_dt_root(), "model", NULL);

	if (!model)
		return 0;

	return strcmp(model, "RB800") == 0;
}

define_machine(rb800) {
	.name			= "RB800",
	.probe			= rb800_probe,
	.setup_arch		= rb800_setup_arch,
	.init_IRQ		= rb_pic_init,
	.get_irq		= rb_get_irq,
	.show_cpuinfo		= rb_show_cpuinfo,
	.restart		= rb_restart,
	.calibrate_decr		= generic_calibrate_decr,
	.pcibios_fixup_bus	= fsl_pcibios_fixup_bus,
};
