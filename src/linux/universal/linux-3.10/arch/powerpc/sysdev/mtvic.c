#include <linux/module.h>
#include <linux/irq.h>
#include <asm/irq.h>
#include <asm/mpic.h>
#include <asm/mtvic.h>

struct irq_domain *virq_host;
EXPORT_SYMBOL(virq_host);

unsigned get_virq_nr(unsigned hwirq)
{
	return irq_create_mapping(virq_host, hwirq);
}
EXPORT_SYMBOL(get_virq_nr);

void mtvic_mask_irq(struct irq_data *irq)
{
}

void mtvic_unmask_irq(struct irq_data *irq)
{
}

static struct irq_chip softirq_chip = {
	.irq_mask	= mtvic_mask_irq,
	.irq_unmask	= mtvic_unmask_irq,
};

static int mtvic_map(struct irq_domain *h, unsigned int virq, irq_hw_number_t hw)
{
	irq_set_chip_and_handler(virq, &softirq_chip, handle_simple_irq);
	return 0;
}

static struct irq_domain_ops mtvic_ops = {
	.map = mtvic_map,
};

void __init mtvic_init(int def)
{
	static unsigned virqs;


	virq_host = irq_domain_add_linear(NULL , 32, &mtvic_ops, NULL);

	virq_host->host_data = &virqs;

	if (def)
		irq_set_default_host(virq_host);
}

unsigned mtvic_get_irq(void)
{
	static unsigned i = 0;
	unsigned *irqs = virq_host->host_data;

	if (!irqs)
		return NO_IRQ;

	for (i = (i + 1) & 31; *irqs; i = (i + 1) & 31) {
		if (*irqs & (1 << i)) {
			atomic_sub(1 << i, (atomic_t *) irqs);
			return irq_linear_revmap(virq_host, i);
		}
	}
	return NO_IRQ;
}

unsigned rb_get_irq(void)
{
	unsigned irq = mtvic_get_irq();
	if (irq != NO_IRQ) return irq;

	return mpic_get_irq();
}
