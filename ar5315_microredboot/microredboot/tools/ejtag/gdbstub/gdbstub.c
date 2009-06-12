#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <features.h>
#include <netpacket/packet.h>
#include <net/ethernet.h>

#include <asm/semaphore.h>
#include <sys/ioctl.h> 
#include <sys/types.h>
#include <sys/stat.h>
#include <ejtag.h>

#include <signal.h>
#include <fcntl.h>
#include <assert.h>

#include "ejtag_primitives.h"
#include "remote-mips-ejtag.h"
#include "tm.h"
#include <strings.h>

#ifdef VERBOSE
static int verbose = 1;
#else
static int verbose = 0;
#endif

#define MAX_HW_BREAKPOINTS 4
#define DSREG_BASE         0xff300000
#define IBC_BASE           (DSREG_BASE + 0x1118)
#define IBC_SEP            0x100
#define IBS                (DSREG_BASE + 0x1000)
#define IBA_BASE           (DSREG_BASE + 0x1100)
#define IBA_SEP            0x100
#define IBM_BASE           (DSREG_BASE + 0x1108)
#define IBM_SEP            0x100

#define IBC_ENABLED        1

#define MAX_HW_WATCHPOINTS 2
#define DBC_BASE           (DSREG_BASE + 0x2118)
#define DBC_SEP            0x100
#define DBS                (DSREG_BASE + 0x2000)
#define DBA_BASE           (DSREG_BASE + 0x2100)
#define DBA_SEP            0x100
#define DBM_BASE           (DSREG_BASE + 0x2108)
#define DBM_SEP            0x100
#define DBV_BASE           (DSREG_BASE + 0x2120)
#define DBV_SEP            0x100

#define DBC_ENABLED        1

unsigned long pp_base = PP_BASE;

extern int jtagbrk;
extern unsigned int big_probe_mem[];
int display = 1 ;
unsigned int c = 'q'  ;
unsigned use_dma=0;

char *regnames[]=MIPS_REGISTER_NAMES;

static struct _mips_stuff {
    unsigned long version, implementation ;
    unsigned long cp0_debug, cp0_depc ;
    unsigned  open : 1 ;
    unsigned  single_stepping :1 ;
    unsigned  data_changed :1 ;
    unsigned  code_changed :1 ;
    unsigned long impl, dev_id ;
    int fd ;
    struct bp* pbp;
} mips_data ;


struct bp {
    struct bp* next;
    unsigned long addr ;
    unsigned long insn ;
};


/*
 * Semantics:
 *   1) we're in charge of storing the dislodged instr at the
 *      address a breakpoint is inserted. when the breakpoint
 *      is removed we put the instr back.
 *   2) all register values with addresses are 64-bit and require
 *      the leading 0xffffffff........ instead of zeroes.
 *   3) all the step instructions (next, step, stepi) work by
 *      issuing a 's' command and reacting to the increased $pc.
 *      next and step will issue enough 's' commands to increment
 *      the $pc till the next higher-level language command.
 */

static const char hexchars[]="0123456789abcdef";

#define BUFMAX 4096
static char remcomInBuffer[BUFMAX];
static char remcomOutBuffer[BUFMAX];

#ifdef REG_SIZE_64
#define REG_PREFIX "ffffffff"
#else
#define REG_PREFIX
#endif

/* socket to use once connected */
static int gConnectedSock;

void dumpregs()
{
    int i ;
    int reg ;

  
    fflush(stdout);
    for(i=0;i<=PC_REGNUM;i++) {
        if ( i && !(i%4) ) printf("\n");
        reg=mips_ejtag_getreg(i);      // sw      $i,0($3)
        printf("%5s (%3d): %08x ", regnames[i], i, reg);
    }
    printf("\n");
    for(i=FIRST_EMBED_REGNUM;i<=LAST_EMBED_REGNUM;i++){
        if ( i-FIRST_EMBED_REGNUM && !((i-FIRST_EMBED_REGNUM)%4) ) printf("\n");
        reg=mips_ejtag_getreg(i);      // sw      $i,0($3)
        printf("%5s (%3d): %08x ", regnames[i], i, reg);
    }
    printf("\n");
}

void putDebugChar(char c)
{
    send(gConnectedSock, &c,1,0);
}

int getDebugChar()
{
    char c;
    recv(gConnectedSock, &c,1,0);
    return c;
}


/* Convert ch from a hex digit to an int */
static int
hex (unsigned char ch)
{
    if (ch >= 'a' && ch <= 'f')
        return ch-'a'+10;
    if (ch >= '0' && ch <= '9')
        return ch-'0';
    if (ch >= 'A' && ch <= 'F')
        return ch-'A'+10;
    return -1;
}

/* scan for the sequence $<data>#<checksum>     */
unsigned char *
getpacket (void)
{
    unsigned char *buffer = (unsigned char *)&remcomInBuffer[0];
    unsigned char checksum;
    unsigned char xmitcsum;
    int count;
    char ch;

    while (1)
    {
        /* wait around for the start character, ignore all other characters */
        while ((ch = getDebugChar ()) != '$')
            ;

    retry:
        checksum = 0;
        xmitcsum = -1;
        count = 0;

        /* now, read until a # or end of buffer is found */
        while (count < BUFMAX)
        {
            ch = getDebugChar ();
            if (ch == '$')
                goto retry;
            if (ch == '#')
                break;
            checksum = checksum + ch;
            buffer[count] = ch;
            count = count + 1;
        }
        buffer[count] = 0;

        if (ch == '#')
        {
			if (verbose)
	            printf("getpacket: %s\n",buffer);

            ch = getDebugChar ();
            xmitcsum = hex (ch) << 4;
            ch = getDebugChar ();
            xmitcsum += hex (ch);

            if (checksum != xmitcsum)
            {
                putDebugChar ('-');	/* failed checksum */
            }
            else
            {
                putDebugChar ('+');	/* successful transfer */

                /* if a sequence char is present, reply the sequence ID */
                if (buffer[2] == ':')
                {
                    putDebugChar (buffer[0]);
                    putDebugChar (buffer[1]);

                    return &buffer[3];
                }

                return &buffer[0];
            }
        }
    }
}

/* send the packet in buffer.  */
static void
putpacket (unsigned char *buffer)
{
    unsigned char checksum;
    int count;
    unsigned char ch;

	if (verbose)
	    printf("putpacket: %s\n",buffer);

    /*  $<packet info>#<checksum>. */
    do
    {
        putDebugChar('$');
        checksum = 0;
        count = 0;

        while (ch = buffer[count])
        {
            putDebugChar(ch);
            checksum += ch;
            count += 1;
        }

        putDebugChar('#');
        putDebugChar(hexchars[checksum >> 4]);
        putDebugChar(hexchars[checksum & 0xf]);

    }
    while (getDebugChar() != '+');
}


void handleQuery(char *p)
{
    if(strcmp(p,"C") == 0){ /* QC<pid> */
        /* default to thread 0 as curent */
        strcpy(remcomOutBuffer,"QC0000");        
    }
    else{
        strcpy(remcomOutBuffer,"");
        printf("Unrecognized query!\n");
    }
}

void getRegisters()
{
    /* All as 64-bit encoded as ascii-hex, one after the other. 
     * Register order is:
     *    32 general-purpose; 
     *    sr; lo; hi; bad; cause; pc; 
     *    32 floating-point registers; 
     *    fsr; fir; fp; 
     */
    
    int i;
    unsigned long reg;
    char regstr[256];

    /* XXX: skip floating-point for now */

    remcomOutBuffer[0]='\0';
    for(i=0;i<=PC_REGNUM;i++) {
        reg=mips_ejtag_getreg(i);      // sw      $i,0($3)
        sprintf(regstr,REG_PREFIX "%08x",reg);
        strcat(remcomOutBuffer,regstr);
    }
}

char *findchar(char *str,char c)
{
    do{
        if(str[0]==c) return str;
    }while((strlen(str++)>0));
    return 0;
}

void readMemory(char *cmd)
{
    char *p;
    unsigned long addr, data;
    int bytes, numBytesReturn;
    char num[16];

    p = findchar(cmd,',');
    *p = '\0';
    addr=strtoul(cmd,NULL,16);
    bytes=strtol(p+1,0,16);
	if (verbose) printf("addr: %08x, bytes: %d\n",addr,bytes);

    remcomOutBuffer[0]='\0';
#if 0
    /* XXX: HACK! */
    if(bytes==8){
        strcat(remcomOutBuffer, "ffffffff");
        if ( use_dma )
            data=mips_ejtag_read_w(addr&(~3));
        else
            data=mips_ejtag_procRead_w(addr&(~3));

        sprintf(num,"%08x",data);
        printf("   %s\n",num);
        strcat(remcomOutBuffer,num);
        return;
    }
#endif

    while(bytes>0){
        /* ALWAYS read 4B aligned then fixup later.
         * XXX: make somewhat bad assumption that non-4B aligned chunks always
         *      come in <4B requests.
         */
        if ( use_dma )
            data=mips_ejtag_read_w(addr&(~3));
        else
            data=mips_ejtag_procRead_w(addr&(~3));
        numBytesReturn = 4;
        /* non-default in switch is non-4B left-over at end of request */
        switch (bytes) {
        case 1:
            data >>= (3 - (addr&3)) * 8;
            sprintf(num,"%02x",data&0xff);
            break;
        case 2:
            data >>= (2 - (addr&3)) * 8;
            sprintf(num,"%04x",data&0xffff);
            break;
        default:
            /* take care of non-4B alignment at beginning */
            numBytesReturn = 4 - (bytes % 4);
            switch (numBytesReturn) {
            case 1:
                sprintf(num,"%02x",data&0xff);
                if (verbose) printf("   %s\n",num);
                break;
            case 2:
                sprintf(num,"%04x",data&0xffff);
                if (verbose) printf("   %s\n",num);
                break;
            case 3:
                sprintf(num,"%06x",data&0xffffff);
                if (verbose) printf("   %s\n",num);
                break;
            default:
                sprintf(num,"%08x",data);
                if (verbose) printf("   %s\n",num);
            }
        }
        strcat(remcomOutBuffer,num);
        bytes-=numBytesReturn;
        addr+=4;
    }
}


void setRegister(char *cmd)
{
    char *p;
    unsigned long regnum, data;

    p = findchar(cmd,'=');
    *p = '\0';
    regnum=strtoul(cmd,NULL,16);
    data=strtoul(p+1,0,16);
    if (verbose) printf("Setting Reg: %02x to %08x\n", regnum, data);
    mips_ejtag_setreg(regnum, data);
    strcpy(remcomOutBuffer,"OK");
}


void writeMemory(char *cmd)
{
    char *p,*q,cdata[8];
    unsigned long addr, data;
    int bytes;
    char num[8];

    p = findchar(cmd,',');
    *p = '\0';
    p++;
    q = findchar(p,':');
    *(q++) = '\0';
    addr=strtoul(cmd,NULL,16);
    bytes=strtol(p,0,16);

    /* q holds data start */
    while(bytes>0){
        strncpy(cdata,q,8);
        data = strtoul(cdata,0,16);
        if (verbose) printf("writing %08x to %08x\n",data,addr);
        if ( use_dma )
            mips_ejtag_write_w(addr,data);
        else
            mips_ejtag_procWrite_w(addr,data);
        q+=8;
        bytes-=4;
        addr+=4;
    }  
    strcpy(remcomOutBuffer,"OK");
}


void formContinueResp()
{
    unsigned long reg;
    char regstr[256];

    /* 0x25 is $pc */
    strcpy(remcomOutBuffer,"T0525:");
    reg=mips_ejtag_getreg(0x25);
    sprintf(regstr,REG_PREFIX "%08x",reg);
    strcat(remcomOutBuffer,regstr);

    /* 0x1d is $sp */
    strcat(remcomOutBuffer,";1d:");
    reg=mips_ejtag_getreg(0x1d);
    sprintf(regstr,REG_PREFIX "%08x",reg);
    strcat(remcomOutBuffer,regstr);

    /* 0x1e is $fp */
    strcat(remcomOutBuffer,";1e:");
    reg=mips_ejtag_getreg(0x1e);
    sprintf(regstr,REG_PREFIX "%08x",reg);
    strcat(remcomOutBuffer,regstr);

    strcat(remcomOutBuffer,";");
}

struct bp* locate_bp(int addr)
{
    struct bp* pbp;

    pbp=mips_data.pbp;
    while(pbp!=0&&pbp->addr!=addr)
        pbp=pbp->next;
    return pbp;
}

void insertHwBp(int addr)
{
    int i;
    unsigned long ibc;

    /* find free slot */
    for(i=0; i<MAX_HW_BREAKPOINTS; i++){
        ibc = mips_ejtag_procRead_w(IBC_BASE + i*IBC_SEP);
        if( (ibc & IBC_ENABLED) == 0 )
            break;
    }
    if(i == MAX_HW_BREAKPOINTS){
        printf("ATTEMPT TO INSERT TOO MANY HW BREAKPOINTS!\n");
        return;
    }
    mips_ejtag_procWrite_w(IBA_BASE + i*IBA_SEP, addr);
    mips_ejtag_procWrite_w(IBM_BASE + i*IBM_SEP, 0);
    mips_ejtag_procWrite_w(IBC_BASE + i*IBC_SEP, ibc|IBC_ENABLED);
}

void removeHwBp(int addr)
{
    int i;
    unsigned long ibc;

    /* find slot */
    for(i=0; i<MAX_HW_BREAKPOINTS; i++){
        ibc = mips_ejtag_procRead_w(IBC_BASE + i*IBC_SEP);
        if( (ibc & IBC_ENABLED) && 
            (mips_ejtag_procRead_w(IBA_BASE + i*IBA_SEP) == addr) )
            break;
    }
    if(i == MAX_HW_BREAKPOINTS){
        printf("FAILED TO REMOVE HW BREAKPOINT!\n");
        return;
    }
    mips_ejtag_procWrite_w(IBC_BASE + i*IBC_SEP, ibc&(~IBC_ENABLED));
}


void insertHwWp(int addr)
{
   int i;
   unsigned long dbc;

   /* find free slot */
   for(i=0; i<MAX_HW_WATCHPOINTS; i++){
	   dbc = mips_ejtag_procRead_w(DBC_BASE + i*DBC_SEP);
	   if( (dbc & DBC_ENABLED) == 0 )
		   break;
   }
   if(i == MAX_HW_WATCHPOINTS){
	   printf("ATTEMPT TO INSERT TOO MANY HW WATCHPOINTS!\n");
	   return;
   }
   mips_ejtag_procWrite_w(DBA_BASE + i*DBA_SEP, addr);
   mips_ejtag_procWrite_w(DBM_BASE + i*DBM_SEP, 0);
   mips_ejtag_procWrite_w(DBV_BASE + i*DBV_SEP, 0);
   mips_ejtag_procWrite_w(DBC_BASE + i*DBC_SEP, DBC_ENABLED);
   //    mips_ejtag_procWrite_w(DBC_BASE + i*DBC_SEP, dbc|DBC_ENABLED);
}

void removeHwWp(int addr)
{
    int i;
    unsigned long dbc;

    /* find slot */
    for(i=0; i<MAX_HW_WATCHPOINTS; i++){
        dbc = mips_ejtag_procRead_w(DBC_BASE + i*DBC_SEP);
        if( (dbc & IBC_ENABLED) && 
            (mips_ejtag_procRead_w(DBA_BASE + i*DBA_SEP) == addr) )
            break;
    }
    if(i == MAX_HW_WATCHPOINTS){
        printf("FAILED TO REMOVE HW BREAKPOINT!\n");
        return;
    }
    mips_ejtag_procWrite_w(DBC_BASE + i*DBC_SEP, dbc&(~DBC_ENABLED));
}

void insertSwBp(int addr)
{
    struct bp* pbp;

    pbp=locate_bp(addr);
    if ( pbp ) {
        fprintf(stderr,"addr %08x already has a breakpoint",addr);
    } else {
        // allocate space for breakpoint
        pbp=malloc(sizeof(struct bp));
        assert(pbp);
        // update bp data structure
        pbp->addr=addr;
        pbp->insn=mips_ejtag_procRead_w(addr);
        // write breakpoint instruction into memory
        mips_ejtag_procWrite_w(addr,SDBBP(0));

        // flush the caches
        mips_ejtag_flushDCacheLine(addr);
        mips_ejtag_flushICacheLine(addr);

        // add bp to list
        pbp->next=mips_data.pbp;
        mips_data.pbp=pbp;
    }
}

void removeSwBp(int addr)
{
    struct bp* pbp, *pbp_prev=NULL;

    pbp=mips_data.pbp;
    while(pbp!=0&&pbp->addr!=addr) {
        pbp_prev=pbp; 
        pbp=pbp->next;
    }
    if ( !pbp ) {
        fprintf(stderr,"addr %08x has no breakpoint",addr);
    } else {
        if(pbp_prev)
            pbp_prev->next=pbp->next;
        else
            mips_data.pbp=NULL;
        mips_ejtag_procWrite_w(pbp->addr,pbp->insn);

        // flush the caches
        if (verbose) printf("Flushing caches at addr: %08x\n",addr);
        mips_ejtag_flushDCacheLine(addr);
        mips_ejtag_flushICacheLine(addr);

        free(pbp);
    }
}

unsigned long disableInt()
{
    unsigned long r;
    r = mips_ejtag_getreg(PS_REGNUM);
    mips_ejtag_setreg(PS_REGNUM, r&0xfffffffe);
    return r;
}

void restoreInt(unsigned long prev)
{
    unsigned long r;

    if(prev&1){
        r = mips_ejtag_getreg(PS_REGNUM);
        mips_ejtag_setreg(PS_REGNUM, r|1);
    }
}

/* killall -s TRAP gdbstub */
void createBreak(int sig)
{
    if(sig == SIGTRAP){
        /* XXX: need mutex between here and mips_ejtag_wait() */
        printf("Generating break\n");
        mips_ejtag_jtagbrk();   
    }
    else{
        printf("Received unrecognized signal.\n");
    }
}

int main(int argc, char *argv[]) 
{
    int tcpSock,i,numRead,len,accSock;
    struct sockaddr_in addr,servAddr;
    unsigned char buf[4096];
    unsigned long ip;

    unsigned char addrIncr = 0x3c; // temporary hack

    if(argc==2){
        pp_base = strtoul(argv[1],0,0);
        printf("set parport base to: %08x\n",pp_base);
    }

    if(ioperm(pp_base, 4 /* num addr */, 1 /* perm */)){
        printf("No permission for parport!\n");
        return 1;
    }

    // signal handler to generate break
    signal(SIGTRAP, createBreak);

    /* init tcp connection */
    tcpSock = socket(PF_INET, SOCK_STREAM, 0);  // tcp socket
    if (tcpSock == -1) {
        perror("Socket Open");
        return -1;
    }
    addr.sin_family = AF_INET;
    addr.sin_port = htons(8370);
    ip = (127 << 24) | (0 << 16) | (0 << 8) | 1; // localhost
    addr.sin_addr.s_addr = htonl(ip);
    fprintf (stderr,"Binding to 127.0.0.1\n");
    if (bind(tcpSock, (struct sockaddr *) &addr, sizeof(addr))) {
        perror("Bind");
        close(tcpSock);
        return -1;
    }
    if (listen(tcpSock,1024000)) {
        perror("Listen");
        close(tcpSock);
        return -1;
    }
    if ((accSock = accept(tcpSock, (struct sockaddr *) &servAddr, 
                          (socklen_t *) &len)) == -1) {
        perror("Accept");
        close(tcpSock);
        return -1;
    }
    ip = ntohl(servAddr.sin_addr.s_addr);
    fprintf (stderr, "Connected with %d.%d.%d.%d\n",
             (ip >> 24) & 0xff, (ip >> 16) & 0xff,
             (ip >> 8) & 0xff, ip & 0xff);
    gConnectedSock = accSock;

    /* main loop with gdb */
    while (1) {
        char *ptr,*q;
        unsigned long vaddr;
        unsigned int reason;
        unsigned long srval;
        remcomOutBuffer[0] = 0;
        int isSwBp = 0;
        
        ptr = (char *)getpacket();
        switch (*ptr++){
        case 'H':
            /* Set thread. Actually followed by a char indicating
             * thread set for different operations, but for now we
             * won't support threads, so always return OK
             */
            strcpy(remcomOutBuffer,"OK");
            break;
        case 'q':
            handleQuery(ptr);
            break;
        case 'g':
            /* get registers. */
            getRegisters();
            break;
        case 'G':
            /* set registers. */
            /* XXX: implement me! */
            printf("*** set registers not implemented yet! *** \n");
            break;
        case 'P':
            /* program single register */
            setRegister(ptr);            
            break;
        case 'm':
            /* read memory */
            readMemory(ptr);
            break;
        case 'M':
            /* write memory */
            writeMemory(ptr);
            break;
#if 0  /* vCont for threading support of s,S,c,C. skip for now. */
        case 'v':
            if(strcmp(ptr,"Cont?") == 0){
                strcpy(remcomOutBuffer,"vCont;s;S;c;C;");
            }
            break;
#endif
        case 'c': case 'C':
            /* continue */
            if( strlen(ptr) !=0 )
                printf("XXX: continue from specified addr not supported!\n");

            /* clear any potential timer int */
            //mips_ejtag_setcp0reg(CP0_COMPARE1, mips_ejtag_getcp0reg(CP0_COMPARE1));

            mips_ejtag_release();
            printf("waiting for breakpoint...");
            fflush(stdout);
            /* only respond once stopped again (for now) */
            mips_ejtag_wait(&reason);
            printf("   break taken\n");
            formContinueResp();
            break;
        case 's': case 'S':
            /* step */
            if( strlen(ptr) !=0 )
                printf("XXX: continue from specified addr not supported!\n");
            srval = disableInt();
            mips_ejtag_setSingleStep(1); /* enter SS mode */
            mips_ejtag_release();

            jtagbrk = 1;

            /* NOTE: though the spec claims SYNC on entrance to debug mode
             *       is required in some implementations, it has been found
             *       to put the CPU into confused state.
             *       For reference:
             *         mips_ejtag_pracc(MIPS_SYNC);
             *         mips_ejtag_pracc(SSNOP);
             *         mips_ejtag_pracc(NOP);
             */
            
            /* print the count on the way in */
            if (verbose) printf("******* Count: %08x *******\n",mips_ejtag_getcp0reg(CP0_COUNT1));

            mips_ejtag_setSingleStep(0); /* leave SS mode */
            /* only respond once stopped again (for now) */
            formContinueResp();
            restoreInt(srval);
            break;
        case 'z':
            /* remove breakpoint (cheating the protocol a bit) */
            if(ptr[0] == '0'){
                /* SW break */
                isSwBp = 1;
            }
            ptr = findchar(ptr,',');
            ptr++;
            q = findchar(ptr,',');
            *q = '\0';
            vaddr = strtoul(ptr,NULL,16);

            /* XXX: hardcode to HW bp for quick test! */
            //isSwBp = 0;

            if(isSwBp){
                if (verbose) printf("Removing SW breakpoint at %08x.\n",vaddr);
                removeSwBp(vaddr);
            }
            else{
                if (verbose) printf("Removing HW breakpoint at %08x.\n",vaddr);
                removeHwBp(vaddr);
            }
            strcpy(remcomOutBuffer,"OK");
            break;
        case 'Z':
            /* insert breakpoint */
            if(ptr[0] == '0'){
                /* SW break */
                isSwBp = 1;
            }
            ptr = findchar(ptr,',');
            ptr++;
            q = findchar(ptr,',');
            *q = '\0';
            vaddr = strtoul(ptr,NULL,16);

            /* XXX: hardcode to HW bp for quick test! */
            //isSwBp = 0;

            if(isSwBp){
                if (verbose) printf("Inserting SW breakpoint at %08x.\n",vaddr);
                insertSwBp(vaddr);
            }
            else{
                if (verbose) printf("Inserting HW breakpoint at %08x.\n",vaddr);
                insertHwBp(vaddr);
            }
            strcpy(remcomOutBuffer,"OK");
            break;
        case 'a':
            /* remove watchpoint (cheating the protocol a bit) */
            ptr = findchar(ptr,',');
            ptr++;
            q = findchar(ptr,',');
            *q = '\0';
            vaddr = strtoul(ptr,NULL,16);

			printf("Removing HW watchpoint at %08x.\n",vaddr);
			removeHwWp(vaddr);
            strcpy(remcomOutBuffer,"OK");
            break;
        case 'A':
            /* insert watchpoint */
            ptr = findchar(ptr,',');
            ptr++;
            q = findchar(ptr,',');
            *q = '\0';
            vaddr = strtoul(ptr,NULL,16);

			printf("Inserting HW watchpoint at %08x.\n",vaddr);
			insertHwWp(vaddr);
            strcpy(remcomOutBuffer,"OK");
            break;

        case '?':
            /* last signal. Always respond with mips SIGSTOP (23).
             * Implies we come up stopped!
             */
            //strcpy(remcomOutBuffer,"S17"); /* value in hex */
            strcpy(remcomOutBuffer,"S05"); /* value in hex */
            break;
        default:
            printf("**** Received unrecognized command!!! ****\n");
        }
        putpacket((unsigned char *)remcomOutBuffer);
    }
}



#define read_cycle_counter(x) \
        __asm__(".byte 0x0f,0x31" \
                        :"=a" (((unsigned long *) &x)[0]), \
                        "=d" (((unsigned long *) &x)[1]));

#define diff_cycle_counter(time_low, time_high, s, e) \
        __asm__("subl %2,%0\n\t" \
                        "sbbl %3,%1" \
                        :"=r" (time_low), "=r" (time_high) \
                        :"m" (*(0+(long *)&s)), \
                        "m" (*(1+(long *)&s)), \
                        "0" (*(0+(long *)&e)), \
                        "1" (*(1+(long *)&e)));
        

int profile_main(int argc, char *argv[]) 
{
	unsigned long reg;
	int           delay;
	int           num;
	unsigned long long start;
	unsigned long long end;
	unsigned long time_high, time_low;

	if (argc<3) {
		fprintf(stderr, "Usage: %s <number of samples> <sample period>\n", argv[0]);
		exit(1);
	}

	if(ioperm(pp_base, 4 /* num addr */, 1 /* perm */)){
		printf("No permission for parport!\n");
		return 1;
	}

	num   = atoi(argv[1]);
	delay = atoi(argv[2]);

	for ( ; (num > 0); num--) {

		mips_ejtag_jtagbrk();   
		reg = mips_ejtag_getreg(PC_REGNUM);
		mips_ejtag_release();   
		printf("0x%x\n", reg);
		fflush(stdout);
		if (delay)
			usleep(delay);
	}
}



int bogus_main(int argc, char *argv[]) 
{
	char *prog_name;
	
	prog_name = rindex(argv[0], '/');
	if (prog_name == NULL) {
		prog_name = argv[0];
	} else {
		prog_name++;
	}

	
	if (strcmp(prog_name, "gdbstub") == 0) {
		//gdb_main(argc, argv);
	} else {
		profile_main(argc, argv);
	}
}
