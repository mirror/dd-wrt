#ifndef __EJTAG_GDBSTUBS__
#define __EJTAG_GDBSTUBS__


/* gdb stub routines */
char getDebugChar(int sock);
void putDebugChar(int sock ,char c);
void getpacket(int sock,char *buffer);
void putpacket(int sock, char *buffer);
int hex(unsigned char ch);
int hexToInt(char**ptr, int *intValue);
unsigned char *mem2hex(char *mem, char *buf, int count, int may_fault);
static const char hexchars[]="0123456789abcdef";

/* ejtag/gdb stub merging */
int ejtag_hexToInt(int, int *intValue);

void handle_exception (int fd, int sock, unsigned regs, char *output_buffer);
void parse_gdb_cmds (int fd, int sock, unsigned regs, char *output_buffer, char*input_buffer);
unsigned char *ejtag_mem2hex(int fd, unsigned int mem, char *buf, int count, int may_fault);
int ejtag_hex2mem(int fd,char *buf, unsigned int mem, int count, int may_fault);

#endif
