#include "at_common.h"

#define DRV_VERSION		"1.0.1.9"

int cards_found;
static struct pci_driver at_common_driver;
char at_ethernet_driver_name[] = "atheros_eth";
char at_driver_string[] = "Atheros(R) AR8121/AR8113/AR8114/AR8131/AR8132/AR8152 PCI-E Ethernet Network Driver";
char at_copyright[] = "Copyright (c) 2007 - 2009 Atheros Corporation.";
char at_driver_version[] = DRV_VERSION;

static struct pci_device_id at_ethernet_pci_tbl[] = {
	ATHEROS_ETHERNET_DEVICE(DEV_ID_ATL1E),
	ATHEROS_ETHERNET_DEVICE(DEV_ID_ATL1C),
	ATHEROS_ETHERNET_DEVICE(DEV_ID_ATL2C),
	ATHEROS_ETHERNET_DEVICE(DEV_ID_ATL1C_2_0),
	ATHEROS_ETHERNET_DEVICE(DEV_ID_ATL2C_2_0),
	ATHEROS_ETHERNET_DEVICE(DEV_ID_ATL2C_B),
	ATHEROS_ETHERNET_DEVICE(DEV_ID_ATL1D),
	ATHEROS_ETHERNET_DEVICE(DEV_ID_ATL2C_B_2),	
	{0,}
};

MODULE_DEVICE_TABLE(pci, at_ethernet_pci_tbl);
#ifdef CONFIG_PM
static int at_common_suspend(struct pci_dev *pdev, pm_message_t state)
{
	switch (pdev->device) {
	case DEV_ID_ATL1E:
	case DEV_ID_ATL1C:
	case DEV_ID_ATL2C:
		return atl1e_suspend(pdev, state);
		break;
	case DEV_ID_ATL1C_2_0:
	case DEV_ID_ATL2C_2_0:
	case DEV_ID_ATL2C_B:
	case DEV_ID_ATL1D:
	case DEV_ID_ATL2C_B_2:
		return atl1c_suspend(pdev, state);
		break;
	default:
		return -1;
		break;
	}
	return 0;
}

#endif

#ifndef USE_REBOOT_NOTIFIER
static void at_common_shutdown(struct pci_dev *pdev)
{
	switch (pdev->device) {
	case DEV_ID_ATL1E:
	case DEV_ID_ATL1C:
	case DEV_ID_ATL2C:
		return atl1e_shutdown(pdev);
		break;
	case DEV_ID_ATL1C_2_0:
	case DEV_ID_ATL2C_2_0:
	case DEV_ID_ATL2C_B:
	case DEV_ID_ATL1D:
	case DEV_ID_ATL2C_B_2:
		return atl1c_shutdown(pdev);
		break;
	default:
		return;	
	}
}
#else
static int at_notify_reboot(struct notifier_block *nb, 
				unsigned long event, void *p)
{
	struct pci_dev *pdev = NULL;

	switch(event) {
	case SYS_DOWN:
	case SYS_HALT:
	case SYS_POWER_OFF:
		while((pdev = pci_find_device(PCI_ANY_ID, PCI_ANY_ID, pdev))) {
			if(pci_dev_driver(pdev) == &at_common_driver)
				at_common_suspend(pdev, PMSG_SUSPEND);
		}
	}
	return NOTIFY_DONE;
}
struct notifier_block at_notifier_reboot = {
	.notifier_call  = at_notify_reboot,
	.next       = NULL,
	.priority   = 0
};
#endif

#ifdef CONFIG_PM
static int at_common_resume(struct pci_dev *pdev)
{
	switch (pdev->device) {
	case DEV_ID_ATL1E:
	case DEV_ID_ATL1C:
	case DEV_ID_ATL2C:
		return atl1e_resume(pdev);
		break;
	case DEV_ID_ATL1C_2_0:
	case DEV_ID_ATL2C_2_0:
	case DEV_ID_ATL2C_B:
	case DEV_ID_ATL1D:	
	case DEV_ID_ATL2C_B_2:
		return atl1c_resume(pdev);
		break;
	default:
		return -1;
		break;
	}
	return 0;
}
#endif

static void __devexit at_common_remove(struct pci_dev *pdev)
{
	switch (pdev->device) {
	case DEV_ID_ATL1E:
	case DEV_ID_ATL1C:
	case DEV_ID_ATL2C:
		return atl1e_remove(pdev);
		break;
	case DEV_ID_ATL1C_2_0:
	case DEV_ID_ATL2C_2_0:
	case DEV_ID_ATL2C_B:
	case DEV_ID_ATL1D:	
	case DEV_ID_ATL2C_B_2:
		return atl1c_remove(pdev);
		break;
	default:
		break;
	}
}

/*
 * at_common_probe initializes common atheros ethernet adapter
 * identified by a pci_dev structure. The OS initialization,
 * configuring of the adapter private structure, and a hardware
 * reset occur.
 */
static int __devinit at_common_probe(struct pci_dev *pdev,
				const struct pci_device_id *ent)
{
	switch (pdev->device) {
	case DEV_ID_ATL1E:
	case DEV_ID_ATL1C:
	case DEV_ID_ATL2C:
		return atl1e_probe(pdev, ent);
		break;
	
	case DEV_ID_ATL1C_2_0:
	case DEV_ID_ATL2C_2_0:
	case DEV_ID_ATL2C_B:
	case DEV_ID_ATL1D:	
	case DEV_ID_ATL2C_B_2:
		return atl1c_probe(pdev, ent);
		break;
	default:
		return -1;
		break;
	}

	return 0;
}

#ifdef CONFIG_AT_PCI_ERS
static pci_ers_result_t at_common_io_error_detected(struct pci_dev *pdev,
					pci_channel_state_t state)
{
	switch (pdev->device) {
	case DEV_ID_ATL1E:
        case DEV_ID_ATL1C:
        case DEV_ID_ATL2C:
		return atl1e_io_error_detected(pdev, state);
		break;
	
	case DEV_ID_ATL1C_2_0:
	case DEV_ID_ATL2C_2_0:
	case DEV_ID_ATL2C_B:
	case DEV_ID_ATL1D:	
	case DEV_ID_ATL2C_B_2:
                return atl1c_io_error_detected(pdev, state);
                break;
        default:
                return -1;
                break;
	}
}

static pci_ers_result_t at_common_io_slot_reset(struct pci_dev *pdev)
{
	switch (pdev->device) {
        case DEV_ID_ATL1E:
        case DEV_ID_ATL1C:
        case DEV_ID_ATL2C:
                return atl1e_io_slot_reset(pdev);
                break;

        case DEV_ID_ATL1C_2_0:
	case DEV_ID_ATL2C_2_0:
	case DEV_ID_ATL2C_B:
	case DEV_ID_ATL1D:	
	case DEV_ID_ATL2C_B_2:
                return atl1c_io_slot_reset(pdev);
                break;
        default:
                return -1;
                break;
        }
}

static void at_common_io_resume(struct pci_dev *pdev)
{
	switch (pdev->device) {
        case DEV_ID_ATL1E:
        case DEV_ID_ATL1C:
        case DEV_ID_ATL2C:
                return atl1e_io_resume(pdev);
                break;

        case DEV_ID_ATL1C_2_0:
	case DEV_ID_ATL2C_2_0:
	case DEV_ID_ATL2C_B:
	case DEV_ID_ATL1D:	
	case DEV_ID_ATL2C_B_2:
                return atl1c_io_resume(pdev);
                break;
        default:
                return ;
                break;
        }
}

static struct pci_error_handlers at_err_handler = {
	.error_deteach 	= at_common_io_error_detected,
	.slot_reset    	= at_common_io_slot_reset,
	.resume		= at_common_io_resume,
};
#endif

static struct pci_driver at_common_driver = {
	.name		= at_ethernet_driver_name,
	.id_table 	= at_ethernet_pci_tbl,
	.probe 		= at_common_probe,
	.remove		= __devexit_p(at_common_remove),
#ifdef CONFIG_PM
	.suspend	= at_common_suspend,
	.resume		= at_common_resume,
#endif
#ifndef USE_REBOOT_NOTIFIER
	.shutdown 	= at_common_shutdown,
#endif
#ifdef CONFIG_AT_PCI_ERS
	.err_handler	= &at_err_handler
#endif
};

static int __init at_common_init_module(void)
{
	int ret;
	printk(KERN_INFO "%s - version %s\n",
		at_driver_string, at_driver_version);
	printk(KERN_INFO "%s\n", at_copyright);

	ret = pci_register_driver(&at_common_driver);
#ifdef USE_REBOOT_NOTIFIER
	if (ret >= 0) {
		register_reboot_notifier(&at_notifier_reboot);
        }
#endif
	return ret;
}

static void __exit at_common_exit_module(void)
{
#ifdef USE_REBOOT_NOTIFIER
	unregister_reboot_notifier(&at_notifier_reboot);
#endif
	pci_unregister_driver(&at_common_driver);
}

module_init(at_common_init_module);
module_exit(at_common_exit_module);
MODULE_DESCRIPTION("Atheros Gigabit Ethernet driver");
MODULE_AUTHOR("Atheros Corporation, <xiong.huang@atheros.com>, Jie Yang <jie.yang@atheros.com>");
MODULE_LICENSE("GPL");
MODULE_VERSION(DRV_VERSION);
