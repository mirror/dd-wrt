#ifndef __EJTAGD_H__
#define __EJTAGD_H__

#define EJTAG_TCP_PORT 8888
#define BUFMAX 1024
#define PROBEMEMSIZE 0x1000      // 4 kilobytes
#define PROBEMEMBASE 0xFF000200
#define CONTEXTBASE  0x00000000

#ifndef _LANGUAGE_ASSEMBLY

unsigned int ejtag_targetExec(int fd, unsigned ctrl,unsigned int stopaddr);
unsigned int ejtag_targetMemAcc(int fd,unsigned ctrl);
unsigned int ejtag_returnWord(int fd, unsigned ctrl, unsigned int word);
unsigned int ejtag_forwardWord(int fd, unsigned ctrl, unsigned int *word);
unsigned int  ejtag_jump(int fd, unsigned ctrl, unsigned int addr);
void ejtag_flushCacheAll(int fd);
void ejtag_softReset(int fd);
void ejtag_init(int fd);
void ejtag_setSingleStep(int fd, unsigned regs, int dss);
void ejtag_singleStep(int fd,unsigned regs);
void ejtag_breakPoint(int fd);
void ejtag_dataAddrLoad(int fd);
void ejtag_dataAddrStore(int fd);
void ejtag_instrAddr(int fd);
void ejtag_busJtagBrk(int fd);
void ejtag_error(int fd);
void print_ejtag_ctrl(unsigned ctrl);
void ejtag_getRegisters(int fd);
void ejtag_restoreRegisters(int fd);

void catch_segv(int sig_num); /* signal handler */

 /* operational flags */
extern int debug_mode ;
extern int dsu_enabled ;
extern int single_stepping ;
extern int bigendian ;

#endif 

#endif
