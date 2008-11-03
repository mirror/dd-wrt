#ifndef RA_MAC_H
#define RA_MAC_H

void ra2880start(END_DEVICE *ei_local);
void ra2880stop(END_DEVICE *ei_local);
void ra2880dmaRest(MAC_INFO *macinfo);

void ra2880MacAddressSet(MAC_INFO *macinfo, unsigned char p[6]);
void ra2880Mac2AddressSet(MAC_INFO *macinfo, unsigned char p[6]);
void hard_init(struct net_device *dev);

void ra2880EnableInterrupt(void);

void dump_qos(void);
void dump_reg(void);
void dump_cp0(void);

int debug_proc_init(void);
void debug_proc_exit(void);

unsigned long readreg(unsigned long addr);
void writereg(unsigned long value, unsigned long addr);

void enable_auto_negotiate(void);
int wait_linkup(void);
int wait_an_completed(void);

void rt2880_gmac_hard_reset(void);

extern u32 mii_mgr_read( unsigned int , unsigned int, unsigned int *);
extern u32 mii_mgr_write( unsigned int, unsigned int, unsigned int);
#endif
