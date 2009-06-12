/* 
 * original author padraigo
 * synopsis
 *  simple series of calls to verify the operation of the ejtag driver.
 */

#include <ejtag.h>

#include <unistd.h>
#include <signal.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "ejtag_primitives.h"
#include "remote-mips-ejtag.h"
#include "tm.h"
#include <strings.h>

extern int jtagbrk;
typedef unsigned long u32;

struct bp {
    struct bp* next;
    unsigned long addr ;
    unsigned long insn ;
};

void info(void);
void identify_part();
void instr_status();
void issue_reset();
void do_jtagbrk();
void probe_memrd(int data);
void dumpmem();
void dumpregs();
void do_read();
void do_write();
void select_dma(void);
void insert_bp(int addr);
void remove_bp(int addr);
int  download_binary(int addr,int bytes,char *fname);
void mips_ejtag_procWrite_wfast(unsigned long addr, unsigned long data);
void output_debug_reg();

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


int parseIniFile(char *filename);

unsigned long pp_base = PP_BASE;

int main(int argc, char **argv)
{
    unsigned int i ;
    int dev_id, impl;
    int addr,data ;
    char fname[256];

    if(argc==2){
        pp_base = strtoul(argv[1],0,0);
        printf("set parport base to: %08x\n",pp_base);
    }

    if(ioperm(pp_base, 4 /* num addr */, 1 /* perm */)){
        printf("No permission for parport!\n");
        return 1;
    }
    memset(&mips_data,0,sizeof(mips_data));

    ejtag_switch(EJTAG_WRITE_CTRLP,4);
    //ejtag_switch(EJTAG_WRITE_CTRLP,0);
    printf("ejtag> ");
    for(;;)
    {
        c=fgetc(stdin);
	if ( c == '\n') {
		printf("ejtag> ");
	}
        if ( isspace(c) ) continue ;

        switch(c)
        {
        case 'z':
            mips_ejtag_jtagbrk();
            break;
        case 'Z':
            ejtag_switch(EJTAG_WRITE_CTRLP,5);
            //ejtag_switch(EJTAG_WRITE_CTRLP,1);
            sleep(1);
            ejtag_switch(EJTAG_WRITE_CTRLP,4);
            //ejtag_switch(EJTAG_WRITE_CTRLP,0);
            //usleep(40);
            mips_ejtag_jtagbrk();
            break;
        case 'a':
            printf("JTAG addr = 0x%08x\n",mips_ejtag_getaddr());
            break;
        case 'l':
            scanf("%x %x %s", &addr, &data, fname);
            if(download_binary(addr,data,fname))
                printf("Failed to download binary!\n");
            else
                printf("Download complete.\n");
            break;
        case 't': // tr => tap read, tw <%x> => tap write hex data %x
            c=fgetc(stdin);
            if(c=='w'){
                scanf("%x",&data);
                mips_ejtag_pracc_notdbg(data);
            }
            else{
                printf("processor wrote %08x\n",mips_ejtag_pwacc());
            }
            printf("JTAG addr = 0x%08x\n",mips_ejtag_getaddr());
            break;
        case '0':
            ejtag_switch(EJTAG_DEBUG_D3_0,1);
            break;
        case '1':
            ejtag_switch(EJTAG_DEBUG_D3_1,1);
            break;
        case 'I':
            scanf("%s",fname);
            parseIniFile(fname);
            break;
        case 'b':
            scanf("%x", &addr);
            insert_bp(addr);
            break;
        case 'B':
            scanf("%x", &addr);
            remove_bp(addr);
            break;

            // gdb like commands
        case 'g':
            dumpregs();
            break;

        case 'P':
            scanf("%d %x", &addr, &data);
            mips_ejtag_setreg(addr,data);
            break;

        case 'm':
            dumpmem();
            break ;

        case 's':
            mips_ejtag_setSingleStep(mips_data.single_stepping == 0 ? 1 : 0);
            mips_data.single_stepping = mips_data.single_stepping == 0 ? 1 : 0;
            printf("Current SST state = %d\n",mips_data.single_stepping);
            break;

        case 'c':
            mips_ejtag_release();
            if(mips_data.single_stepping){
                jtagbrk=1;
                printf("pc = %08x\n",mips_ejtag_getreg(98)); // 98 is DEPC
            }
            break;

        case 'C':
            mips_ejtag_release();
            printf("waiting for breakpoint...");
            fflush(stdout);
            mips_ejtag_wait(&data);
            printf("   break taken\n");
            break;
            // native commands
        case 'd':
            select_dma();
            break;
        case 'h':
            info();
            break;
        case 'i':
            mips_ejtag_init((u32 *)mips_data.dev_id, (u32 *) &mips_data.impl);
            break;
        case 'p':
            identify_part();
            printf( "PRID %08x\n", mips_ejtag_getcp0reg(CP0_PRID));
            break;
        case 'r':
            do_read();
            break ;
        case 'S':
            instr_status();
            break ;
        case 'w':
            do_write();
            break;
        case 'x':
            exit(1);
            break;
        case 'D':
            output_debug_reg();
            break;
        default:
            printf("wrong selection: %c\n",c);
        };
    }
}


void info(void)
{
    printf("\n\ndo you want to:\n");
    printf("\n\n\tgdb serial commands:\n");
    printf("\tg get registers\n");
    printf("\tP <%%d> <%%x> set register <%%d> to <%%x>\n");
    printf("\tm <%%x> <%%d> dump <%%d> bytes of memory starting at <%%x>\n");
    printf("\ts single step\n");
    printf("\tc continue\n");
    printf("\n\n\tnative commands:\n");
    printf("\td <%%c> <%%c>==y use ejtag dma, ==n use cpu to read/write memory\n");
    printf("\th print this screen\n");
    printf("\ti init - must be called to initialise communication with device\n");
    printf("\tp print part\n");
    printf("\tr <%%c>={w|h|b} a:<%%x> read word|halfword|byte from <%%x>\n");
    printf("\tw <%%c>={w|h|b} a:<%%x> d:<%%x> write d:<%%x> word|halfword|byte to a:<%%x>\n");
    printf("\tS print the control register status\n");
    printf("\tx exit\n");
}

void select_dma(void)
{
    printf("used dma? [y/n]\n");
    do {
        c = getchar();
    }
    while ( !isspace(c) );
    scanf("%c",&c);
    switch(c){
    case 'Y':
    case 'y':
        use_dma=1;
        break;
    case 'N':
    case 'n':
        use_dma=0;
        break;
    default:
        printf("wrong selection %c\n", c);
    }
}

void issue_reset()
{
    unsigned ctrl ;
    ctrl = mips_ejtag_checkstatus();
    ctrl = ctrl | PRRST | PERRST ;
    mips_ejtag_instr(JTAG_CONTROL_IR);
    mips_ejtag_data(ctrl);
    ctrl = ctrl & ~(PRRST|PERRST) ;
    mips_ejtag_data(ctrl);
}

void print_ctrl(int ctrl)
{
    printf ("\nCtrl + : %x\n", ctrl);
    printf ("     |\n");
    printf ("     +-- DCLKEN    : %d\n", (ctrl>> 0)&0x1 );
    printf ("     +-- TOF       : %d\n", (ctrl>> 1)&0x1 );
    printf ("     +-- TIF       : %d\n", (ctrl>> 2)&0x1 );
    printf ("     +-- BrkSt     : %d\n", (ctrl>> 3)&0x1 );
    printf ("     +-- Dinc      : %d\n", (ctrl>> 4)&0x1 );
    printf ("     +-- Dlock     : %d\n", (ctrl>> 5)&0x1 );
    printf ("     +-- Dsz       : %d\n", (ctrl>> 7)&0x3 );
    printf ("     +-- Drwm      : %d\n", (ctrl>> 9)&0x1 );
    printf ("     +-- Derr      : %d\n", (ctrl>>10)&0x1 );
    printf ("     +-- Dstrt     : %d\n", (ctrl>>11)&0x1 );
    printf ("     +-- JtagBrk   : %d\n", (ctrl>>12)&0x1 );
    printf ("     +-- DEV       : %d\n", (ctrl>>14)&0x1 );
    printf ("     +-- ProbEn    : %d\n", (ctrl>>15)&0x1 );
    printf ("     +-- PrRst     : %d\n", (ctrl>>16)&0x1 );
    printf ("     +-- DmaAcc    : %d\n", (ctrl>>17)&0x1 );
    printf ("     +-- PrAcc     : %d\n", (ctrl>>18)&0x1 );
    printf ("     +-- PRnW      : %d\n", (ctrl>>19)&0x1 );
    printf ("     +-- PerRst    : %d\n", (ctrl>>20)&0x1 );
    printf ("     +-- Run       : %d\n", (ctrl>>21)&0x1 );
    printf ("     +-- Doze      : %d\n", (ctrl>>22)&0x1 );
    printf ("     +-- Sync      : %d\n", (ctrl>>23)&0x1 );
    printf ("     +-- DsuRst    : %d\n", (ctrl>>24)&0x1 );
    printf ("     +-- Psz       : %d\n", (ctrl>>25)&0x3 );
    printf ("     +-- JtagInt   : %d\n", (ctrl>>27)&0x1 );
    printf ("     +-- JtagIntEn : %d\n", (ctrl>>28)&0x1 );
}

void instr_status()
{
    int ctrl ;
  
    ctrl = mips_ejtag_checkstatus();

    print_ctrl(ctrl);
}

void output_debug_reg()
{
    int dbgreg;
  
    dbgreg = mips_ejtag_getcp0reg(CP0_DEBUG);;

    printf("\nDEBUG REGISTER: %08x\n",dbgreg);
    printf("   Single-step exc:       %d\n", (dbgreg&CP0_DEBUG_SS_EXC)?1:0);
    printf("   Break-instr exc:       %d\n", (dbgreg&CP0_DEBUG_BP_EXC)?1:0);
    printf("   Data-addr load exc:    %d\n", (dbgreg&CP0_DEBUG_DBL_EXC)?1:0);
    printf("   Data-addr store exc:   %d\n", (dbgreg&CP0_DEBUG_DBS_EXC)?1:0);
    printf("   Inst-addr load exc:    %d\n", (dbgreg&CP0_DEBUG_DIB_EXC)?1:0);
    printf("   Debug intr exc:        %d\n", (dbgreg&CP0_DEBUG_DINT_EXC)?1:0);
    printf("   Single-step enable:    %d\n", (dbgreg&CP0_DEBUG_SST_EN)?1:0);
    printf("   DEXC code (see CAUSE): %d\n", (dbgreg&CP0_DEBUG_EXC_CODE_MASK)>>CP0_DEBUG_EXC_CODE_SHIFT);
    printf("   Data break load impr.  %d\n", (dbgreg&CP0_DEBUG_DDBL)?1:0);
    printf("   Data break store impr. %d\n", (dbgreg&CP0_DEBUG_DDBS)?1:0);
    printf("   Imprec. err exc inhib. %d\n", (dbgreg&CP0_DEBUG_IEXI)?1:0);
    printf("   Dbus err exc pending   %d\n", (dbgreg&CP0_DEBUG_DBEP)?1:0);
    printf("   Cache err exc pending  %d\n", (dbgreg&CP0_DEBUG_CAEP)?1:0);
    printf("   Mach check exc pending %d\n", (dbgreg&CP0_DEBUG_MCEP)?1:0);
    printf("   Instr fetch exc pend.  %d\n", (dbgreg&CP0_DEBUG_IFEP)?1:0);
    printf("   Count runs in debug    %d\n", (dbgreg&CP0_DEBUG_CNT_DM)?1:0);
    printf("   System bus halted      %d\n", (dbgreg&CP0_DEBUG_HALT)?1:0);
    printf("   DSEG addr to main mem  %d\n", (dbgreg&CP0_DEBUG_LSDM)?1:0);
    printf("   Debug Mode             %d\n", (dbgreg&CP0_DEBUG_DM)?1:0);
    printf("   Debug exc branch dly   %d\n", (dbgreg&CP0_DEBUG_DBD)?1:0);
}

void peekpoke()
{
    char c ;
    unsigned addr, data ;

    printf("do you want to\n\t1/ peek \n\t2/ poke\n");
    do {
        c = getchar();
    }
    while (isspace(c));

    switch(c)
    {
    case '1':
        printf("\nEnter addr in hex: ");
        scanf ("%x",&addr);
        printf("(%x) read %x\n",addr, mips_ejtag_read_w(addr));
        break;
    case '2':
        printf("\nEnter addr in hex: ");
        scanf ("%x",&addr);
        printf("\nEnter data in hex: ");
        scanf ("%x",&data);
        mips_ejtag_write_w(addr,data);
        printf("(%x) written %x\n",addr, mips_ejtag_read_w(addr));
        break;
    default:
        printf("wrong selection\n");
    };
}

void do_read()
{
    char c ;
    unsigned addr, data ;

    printf("w word\nh halfword\nb byte\n");
    do {
        c = getchar();
    }
    while ( !(c=='w'||c=='h'||c=='b') );

    printf("\naddr: ");
    scanf ("%x",&addr);

    switch(c) {
    case 'w':
        if ( use_dma )
            data=mips_ejtag_read_w(addr);
        else
            data=mips_ejtag_procRead_w(addr);

        printf("(%x) read %x\n",addr, data);
        break;
    case 'h':
        printf("(%x) read %x\n",addr, mips_ejtag_read_h(addr));
        break;
    case 'b':
        printf("(%x) read %x\n",addr, mips_ejtag_read_b(addr));
        break;
    default:
        printf("something went wrong %c\n",c);
    };
}

void do_write()
{
    char c ;
    unsigned addr, data ;

    printf("w word\nh halfword\nb byte\n");
    do {
        c = getchar();
    }
    while ( !(c=='w'||c=='h'||c=='b') );

    printf("\naddr: ");
    scanf ("%x",&addr);
    printf("\ndata: ");
    scanf ("%x",&data);

    switch(c) {
    case 'w':
        if ( use_dma )
            mips_ejtag_write_w(addr,data);
        else
            mips_ejtag_procWrite_w(addr,data);
        break;
    case 'h':
        mips_ejtag_write_h(addr,data); 
        break;
    case 'b':
        mips_ejtag_write_b(addr,data); 
        break;
    default:
        printf("something went wrong %c\n",c);
    };
    printf("\n(%x) written %x\n",addr, mips_ejtag_read_w(addr&~0x3));
}

void identify_part()
{
    char * manufacturer ;

    mips_data.impl = mips_ejtag_implementation();

    if ( !mips_data.impl )
        printf(" NOTE: implementation register all zeroes!\n");

    printf ("debug register (bits)        : %d\n", mips_data.impl&1 ? 64:32);
    printf ("break channels               : %d\n", (mips_data.impl>>1) & 0xf );
    printf ("instr addr break implemented : %c\n", (mips_data.impl>>5)&1 ? 'n':'y' );
    printf ("data addr break implemented  : %c\n", (mips_data.impl>>6)&1 ? 'n':'y' );
    printf ("proc bus break implemented   : %c\n", (mips_data.impl>>7)&1 ? 'n':'y' );
  
    printf("\n");
    printf("EJtag Version: %d\n", (mips_data.impl>>29));
    printf("Dint Support : %d\n", (mips_data.impl>>24)&1);
    printf("NoDMA bit    : %d\n", (mips_data.impl>>14)&1);

    mips_data.dev_id = mips_ejtag_version();
    if ( !mips_data.dev_id )
        printf(" NOTE: device identification register all zeroes!\n");

    if (mips_data.dev_id == 0xFFFFFFFF & mips_data.impl == 0xFFFFFFFF )
    {
        printf ("Check cable is connected !!!\n");
        return;
    }
    switch((mips_data.dev_id>>1)&0x3ff)
    {
    case 0x001: 
        manufacturer = "Toshiba";
        break;
    case 0x003: 
        manufacturer = "NEC";
        break;
    case 0x007: 
        manufacturer = "IDT";
        break;
    case 0x00F: 
        manufacturer = "Sony";
        break;
    case 0x010: 
        manufacturer = "NKK";
        break;
    case 0x015: 
        manufacturer = "Philips";
        break;
    default:
        sprintf(manufacturer,"%03x", (mips_data.dev_id>>1)&0x3ff);
        break;
    }
    printf ("Part manufactured by         : %s\n", manufacturer);
    printf ("Part number                  : %d\n", (mips_data.dev_id>>12)&0xFFFF);
    printf ("Part version                 : %d\n", (mips_data.dev_id>>28)&0xF);

}

void dumpmem()
{
    unsigned int base, top, data, addr ;

    printf("Enter the address number of bytes to dump\n");
    scanf("%x %d",&base,&top);
    top=base+top;

    puts("\n\n");
    for(base=base&~0xF, top=(top+3)&~0xF ;base<top; base=base+4)
    {
        if ( (base&0xF) == 0 )
            printf ("%08x: ",base);
        if ( use_dma ) {
            data=mips_ejtag_read_w(base);
        } else {
            data=mips_ejtag_procRead_w(base);
        }
        printf ("%08x%c",data, (base&0xF)==0xC?'\n':' ');
    }
    puts("\n\n");
}


void probe_memrd(int data)
{
    unsigned addr ;
    unsigned ctrl ;
    int timeout = 0x20 ;

    while (1){
        addr=mips_ejtag_ctrl(DEV|PROBEN|PRACC);
        if ( addr & PRACC )
            if ( ctrl&PRNW ) {
                printf("%s early exit\n",__FUNCTION__);
                return ; /* early exit */
            } else {
                break;
            }
    }

    mips_ejtag_instr(JTAG_ADDRESS_IR);
    addr=mips_ejtag_data(0);

    mips_ejtag_instr(JTAG_DATA_IR);
    mips_ejtag_data(data);
    mips_ejtag_instr(JTAG_CONTROL_IR);
    mips_ejtag_data(DEV|PROBEN);

    printf ("%s %08x : %08x\n",__FUNCTION__ , addr,data);
}

int probe_memwr()
{
    unsigned addr ;
    unsigned data ;
    unsigned ctrl ;

    while (1){
        ctrl=mips_ejtag_ctrl(DEV|PROBEN|PRACC);
        mips_ejtag_instr(JTAG_ADDRESS_IR);
        addr=mips_ejtag_data(0);

        if ( ctrl & PRACC )
            if ( ctrl&PRNW )
                break ;
            else {
                printf("%s ADDR: %08x NOP\n",__FUNCTION__ ,addr);
                mips_ejtag_instr(JTAG_DATA_IR);
                mips_ejtag_data(NOP);
                mips_ejtag_instr(JTAG_CONTROL_IR);
                mips_ejtag_data(DEV|PROBEN);
            }
    }

    mips_ejtag_instr(JTAG_ADDRESS_IR);
    addr=mips_ejtag_data(0);
    printf("CPU writing probe %08x\n",addr);

    //  printf ("JTAG_ADDR  %08x\n", addr);
    //  printf ("Scan %08x mips instr into JTAG_DATA\n",data);
    mips_ejtag_instr(JTAG_DATA_IR);
    data=mips_ejtag_data(data);
    mips_ejtag_instr(JTAG_CONTROL_IR);
    mips_ejtag_data(DEV|PROBEN);

    // printf ("%08x : %08x\n", addr,data);
    return data;
}

// test ejtag/dsu jtagbrk operation, probe memory read and dsu enabling 
// last effect is used in the subsequent real time pc trace
void do_jtagbrk()
{
    unsigned ctrl ;
    // test jtag break
    //  printf ("\n**** Test JTAG Break ****\n");
    //  printf (" Scan PROBEN|JTAGBRK|PRACC into JTAG_CTRL");
    fflush(stdout);

    printf("jtag break\n");

    // these instructions will not be executed by the cpu from
    // probe memory
    mips_ejtag_pracc(0x4002b000);   // 00000000: 4002b000     mfc0    $v0,$22      
    mips_ejtag_pracc(NOP);          // 00000004: 00000000     nop                  
    mips_ejtag_pracc(0x3c030002);   // 00000010: 3c030002     lui     $v1,2        
    mips_ejtag_pracc(0x00431025);   // 00000014: 00431025     or      $v0,$v0,$v1  
    mips_ejtag_pracc(0x4082b000);   // 0000001c: 4082b000     mtc0    $v0,$22      
    mips_ejtag_pracc(NOP);          // 00000020: 00000000     nop                  
    mips_ejtag_release();
}

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

struct bp* locate_bp(int addr)
{
    struct bp* pbp;

    pbp=mips_data.pbp;
    while(pbp!=0&&pbp->addr!=addr)
        pbp=pbp->next;
    return pbp;
}

void insert_bp(int addr)
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
        mips_ejtag_pracc(MIPS_SYNC);
        mips_ejtag_pracc(NOP);
        mips_ejtag_flushICacheLine(addr);
        mips_ejtag_pracc(MIPS_SYNC);
        mips_ejtag_pracc(NOP);
        // add bp to list
        pbp->next=mips_data.pbp;
        mips_data.pbp=pbp;
    }
}

void remove_bp(int addr)
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
        mips_ejtag_flushDCacheLine(addr);
        mips_ejtag_pracc(MIPS_SYNC);
        mips_ejtag_pracc(NOP);
        mips_ejtag_flushICacheLine(addr);
        mips_ejtag_pracc(MIPS_SYNC);
        mips_ejtag_pracc(NOP);
        free(pbp);
    }
}

#define Pio_BaseAddress				0xA001A000	/* Base Address */
#define Pio_InputRegister			0x00		/* (PIO_IN)         Input Register */
#define Pio_OutputRegister			0x04		/* (PIO_OUT)        Output Register */
#define Pio_DirectionRegister			0x08		/* (PIO_DIR)        Direction Register */
#define Pio_SelectRegister			0x14		/* (PIO_SEL)        Select Register */


int download_binary(int addr,int bytes,char *fname)
{
    FILE *fp;
    int val, count=0;

    fp = fopen(fname,"r");
    if(!fp)
        return -1;

    if(bytes==0)bytes=-1;
    while(bytes && fread(&val,4,1,fp)){
	    mips_ejtag_procWrite_wfast(addr,htonl(val));
	    //	    mips_ejtag_procWrite_w(addr,htonl(val));
        if(!(count&(1024-1)))
            printf("addr = %08x\n",addr);
        count+=4;
        addr+=4;
        if(bytes>0){
            bytes-=4;
            if(bytes<0)bytes=0;
        }
    }

    fclose(fp);
    return 0;
}

int getCommand(char *line, u32 *addr, u32 *val);

int parseIniFile(char *filename)
{
    FILE *fp=NULL;
    int numToRead=0,len,cmd,n;
    u32 readVal,val,addr;
    char line[1024];
                                                                                        
    fp = fopen(filename,"r");
    if (fp == NULL) {
        fprintf(stderr,"Error reading file %s\n",filename);
        return(1);
    }
    while (fgets(line,1024,fp) != NULL) {
        len = strlen(line);
        if (line[len-1] == '\n') {
            line[--len] = '\0';
        }
        if (strlen(line) > 0) {
            cmd = getCommand(line,&addr,&val);
            switch(cmd) {
            case 0: /* Remark */
                break;
            case 1: /* Read command */
                for (n=0;n<val;n++) {
                    readVal = mips_ejtag_procRead_w(addr);
                    fprintf(stdout,"Read 0x%08x =(0x%08x)\n",addr,val);
                }
                break;
            case 2:
                mips_ejtag_procWrite_w(addr, val);
                fprintf(stdout,"Write 0x%08x =(0x%08x)\n",addr,val);
                break;
            case 3:
                mips_ejtag_setreg(addr, val);
                fprintf(stdout,"Set Register 0x%x =(0x%08x)\n",addr,val);
                break;
            case 4:
                mips_ejtag_procWrite_wfast(addr,val);
                mips_ejtag_pracc(MIPS_SYNC);
                mips_ejtag_pracc(NOP);
                fprintf(stdout,"Write with reset 0x%08x =(0x%08x)\n",addr,val);
                fprintf(stdout,"Delaying for reset... ");
                fflush(stdout);
                sleep(2);
                fprintf(stdout,"done.\n");
                mips_ejtag_jtagbrk();
                break;

            default:
                fprintf(stdout,"Unknown command in %s\n",line);
            }
        }
    }
    return(0);
}
                                                                                        
void convert2lower(char *line)
{
    while (*line != '\0') {
        *line = (char) tolower((int) *line);
        line++;
    }
}
                                                                                        
int getCommand(char *line, u32 *addr, u32 *val)
{
    char lcopy[1024],*tok,cmd[1024];
    int command=-1,more;
                                                                                        
    strcpy(&lcopy[0],line);
    tok = (char *)strtok(&lcopy[0]," ");
    if (tok != NULL) {
        strcpy(&cmd[0],tok);
    }
    convert2lower(cmd);
    more=0;
    if (strcmp(cmd,"rem") == 0) {
        *addr = 0;
        *val = 0;
        command = 0;
    }
    if (strcmp(cmd,"dml") == 0) {
        command = 1;
        more = 1;
    }
    if (strcmp(cmd,"mml") == 0) {
        command = 2;
        more = 1;
    }
    if (strcmp(cmd,"srg") == 0) {
        command = 3;
        more = 1;
    }
    if (strcmp(cmd,"mrl") == 0) {
        command = 4;
        more = 1;
    }
    if (more) {
        tok = (char *)strtok(NULL," ");
        if (tok != NULL) {
            strcpy(&cmd[0],tok);
        } else {
            fprintf (stderr,"Unable to find address in %s\n",line);
            return(-1);
        }
        sscanf(cmd,"%x",addr);
        tok = (char *)strtok(NULL," ");
        if (tok != NULL) {
            strcpy(&cmd[0],tok);
        } else {
            fprintf (stderr,"Unable to find val in %s\n",line);
            return(-1);
        }
        sscanf(cmd,"%x",val);
    }
    return(command);
}

