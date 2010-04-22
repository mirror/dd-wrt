#ifndef _AT_COMMON_H_
#define _AT_COMMON_H_

#include "kcompat.h"
#include "at_osdep.h"

#define ATHEROS_ETHERNET_DEVICE(device_id) {\
	PCI_DEVICE(0x1969, device_id)}

#define DEV_ID_ATL1E    	0x1026
#define DEV_ID_ATL1C    	0x1067   /* TODO change */
#define DEV_ID_ATL2C    	0x1066
#define DEV_ID_ATL1C_2_0 	0x1063
#define DEV_ID_ATL2C_2_0	0x1062
#define DEV_ID_ATL2C_B		0x2060
#define DEV_ID_ATL2C_B_2	0x2062
#define DEV_ID_ATL1D		0x1073

#define L2CB_V10		0xc0
#define L2CB_V11		0xc1

#define AT_MAX_NIC 32

#define OPTION_UNSET    -1
#define OPTION_DISABLED 0
#define OPTION_ENABLED  1

#define INT_MOD_DEFAULT_CNT             50 // 200us
#define INT_MOD_MAX_CNT                 65000
#define INT_MOD_MIN_CNT                 50

#define SPEED_0             0xffff
#define SPEED_10            10
#define SPEED_100           100
#define SPEED_1000          1000
#define HALF_DUPLEX         1
#define FULL_DUPLEX         2

#define MEDIA_TYPE_AUTO_SENSOR		0
#define MEDIA_TYPE_1000M_FULL		1
#define MEDIA_TYPE_100M_FULL		2	
#define MEDIA_TYPE_100M_HALF		3
#define MEDIA_TYPE_10M_FULL		4
#define MEDIA_TYPE_10M_HALF		5

#define ADVERTISE_10_HALF               0x0001
#define ADVERTISE_10_FULL               0x0002
#define ADVERTISE_100_HALF              0x0004
#define ADVERTISE_100_FULL              0x0008
#define ADVERTISE_1000_HALF             0x0010 /* Not used, just FYI */
#define ADVERTISE_1000_FULL             0x0020

#define AUTONEG_ADVERTISE_SPEED_DEFAULT 0x002F /* Everything but 1000-Half */
#define AUTONEG_ADVERTISE_10_100_ALL    0x000F /* All 10/100 speeds*/
#define AUTONEG_ADVERTISE_10_ALL        0x0003 /* 10Mbps Full & Half speeds*/


 /* All parameters are treated the same, as an integer array of values.
 * This macro just reduces the need to repeat the same declaration code
 * over and over (plus this helps to avoid typo bugs).
 */
#define AT_PARAM_INIT { [0 ... AT_MAX_NIC] = OPTION_UNSET }
#ifndef module_param_array
/* Module Parameters are always initialized to -1, so that the driver
 * can tell the difference between no user specified value or the
 * user asking for the default value.
 * The true default values are loaded in when atl1e_check_options is called.
 *
 * This is a GCC extension to ANSI C.
 * See the item "Labeled Elements in Initializers" in the section
 * "Extensions to the C Language Family" of the GCC documentation.
 */

#define AT_PARAM(X, desc) \
    const int __devinitdata X[AT_MAX_NIC+1] = AT_PARAM_INIT; \
    MODULE_PARM(X, "1-" __MODULE_STRING(AT_MAX_NIC) "i"); \
    MODULE_PARM_DESC(X, desc);
#else
#define AT_PARAM(X, desc) \
    int __devinitdata X[AT_MAX_NIC+1] = AT_PARAM_INIT; \
    int num_##X = 0; \
    module_param_array_named(X, X, int, &num_##X, 0); \
    MODULE_PARM_DESC(X, desc);
#endif


extern int cards_found;
extern int atl1c_probe(struct pci_dev *pdev, const struct pci_device_id *ent);
extern int atl1e_probe(struct pci_dev *pdev, const struct pci_device_id *ent);

extern void __devexit atl1c_remove(struct pci_dev *pdev);
extern void __devexit atl1e_remove(struct pci_dev *pdev);

#ifdef CONFIG_PM
extern int atl1c_suspend(struct pci_dev *pdev, pm_message_t state);
extern int atl1e_suspend(struct pci_dev *pdev, pm_message_t state);

extern int atl1c_resume(struct pci_dev *pdev);
extern int atl1e_resume(struct pci_dev *pdev);
#endif

#ifdef CONFIG_AT_PCI_ERS
extern pci_ers_result_t atl1e_io_error_detected(struct pci_dev *pdev,
		pci_channel_state_t state);
extern pci_ers_result_t atl1c_io_error_detected(struct pci_dev *pdev,
		pci_channel_state_t state);

extern pci_ers_result_t atl1e_io_slot_reset(struct pci_dev *pdev);
extern pci_ers_result_t atl1c_io_slot_reset(struct pci_dev *pdev);

extern void atl1e_io_resume(struct pci_dev *pdev);
extern void atl1c_io_resume(struct pci_dev *pdev);
#endif

extern void atl1c_shutdown(struct pci_dev *pdev);
extern void atl1e_shutdown(struct pci_dev *pdev);
#endif /* __AT_COMMON_H__ */
