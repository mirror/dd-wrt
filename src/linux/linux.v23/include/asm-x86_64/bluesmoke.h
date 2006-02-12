#ifndef _bluesmoke_h
#define _bluesmoke_h

/* Notifier to allow an external module to read machine check events */

extern struct notifier_block *mc_notifier_list;
struct notifier_mc_err {
	unsigned int cpunum;
	unsigned int banknum;
	u64 mci_ctl;
	u64 mci_status;
	u64 mci_addr;
	u64 mci_misc;
};

#endif 
