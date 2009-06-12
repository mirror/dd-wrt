/*
 * File: excalibur.h
 *
 * This file is a machine generated address map
 * for a CPU named cpu.
 * C:/Altera/Excalibur/sopc_builder_2_6_1/examples/verilog/nios_dev_board/LDK_2_0_with_91C111_full/LDK2.ptf
 * Generated: 2002.09.26 13:43:23
 */

#ifndef _excalibur_
#define _excalibur_

#ifdef __cplusplus
extern "C" {
#endif

// The Memory Map

#define na_mon                ((void *)          0x00000000) // altera_avalon_onchip_memory
#define na_mon_base                              0x00000000
#define na_mon_end            ((void *)          0x00000400)
#define na_mon_size                              0x00000400
#define na_cpu                ((void *)          0x00000000) // altera_nios
#define na_cpu_base                              0x00000000
#define na_uart0              ((np_uart *)       0x00000400) // altera_avalon_uart
#define na_uart0_base                            0x00000400
#define na_uart0_irq                             17
#define na_timer0             ((np_timer *)      0x00000440) // altera_avalon_timer
#define na_timer0_base                           0x00000440
#define na_timer0_irq                            16
#define na_ssram_detect       ((np_pio *)        0x00000460) // altera_avalon_pio
#define na_ssram_detect_base                     0x00000460
#define na_button_pio         ((np_pio *)        0x00000470) // altera_avalon_pio
#define na_button_pio_base                       0x00000470
#define na_spi                ((np_spi *)        0x00000480) // altera_avalon_spi
#define na_spi_base                              0x00000480
#define na_spi_irq                               19
#define na_uart1              ((np_uart *)       0x000004a0) // altera_avalon_uart
#define na_uart1_base                            0x000004a0
#define na_uart1_irq                             18
#define na_ide_interface      ((np_usersocket *) 0x00000500) // altera_avalon_user_defined_interface
#define na_ide_interface_base                    0x00000500
#define na_ide_interface_irq                     25
#define na_enet               ((void *)          0x00004000) // altera_avalon_lan91c111
#define na_enet_base                             0x00004000
#define na_enet_end           ((void *)          0x00004020)
#define na_enet_size                             0x00000020
#define na_enet_irq                              35
#define na_enet_reset_n       ((np_pio *)        0x00004020) // altera_avalon_pio
#define na_enet_reset_n_base                     0x00004020
#define na_flash              ((void *)          0x00100000) // altera_nios_dev_board_flash
#define na_flash_base                            0x00100000
#define na_flash_end          ((void *)          0x00200000)
#define na_flash_size                            0x00100000
#define na_flash_kernel       ((void *)          0x00800000) // mtx_sodimm_flash_controller
#define na_flash_kernel_base                     0x00800000
#define na_flash_kernel_end   ((void *)          0x01000000)
#define na_flash_kernel_size                     0x00800000
#define na_sdram              ((void *)          0x01000000) // altera_avalon_sdram_controller
#define na_sdram_base                            0x01000000
#define na_sdram_end          ((void *)          0x02000000)
#define na_sdram_size                            0x01000000
 
#define na_null                           0
#define nasys_flash_count                 1
#define nasys_flash_0                     na_flash
#define nasys_pio_count                   3
#define nasys_pio_0                       na_ssram_detect
#define nasys_pio_1                       na_button_pio
#define nasys_pio_2                       na_enet_reset_n
#define nasys_spi_count                   1
#define nasys_spi_0                       na_spi
#define nasys_spi_0_irq                   19
#define nasys_timer_count                 1
#define nasys_timer_0                     na_timer0
#define nasys_timer_0_irq                 16
#define nasys_uart_count                  2
#define nasys_uart_0                      na_uart0
#define nasys_uart_0_irq                  17
#define nasys_uart_1                      na_uart1
#define nasys_uart_1_irq                  18
#define nasys_usersocket_count            1
#define nasys_usersocket_0                na_ide_interface
#define nasys_usersocket_0_irq            25
#define nasys_vector_table      ((int *)  0x01000000)
#define nasys_vector_table_size           0x00000100
#define nasys_vector_table_end  ((int *)  0x01000100)
#define nasys_reset_address     ((void *) 0x00000000)
#define nasys_clock_freq                  33333000
#define nasys_clock_freq_1000             33333
#define nasys_debug_core                  0
#define nasys_printf_uart                 na_uart0
#define nasys_printf_uart_irq             na_uart0_irq
#define nm_printf_txchar                  nr_uart_txchar
#define nm_printf_rxchar                  nr_uart_rxchar
#define nasys_debug_uart                  na_uart1
#define nasys_debug_uart_irq              na_uart1_irq
#define nasys_main_flash                  na_flash
#define nasys_main_flash_size             na_flash_size
#define nasys_main_flash_end              na_flash_end
#define nasys_program_mem       ((void *) 0x01000100)
#define nasys_program_mem_size            0x00ffff00
#define nasys_program_mem_end   ((void *) 0x02000000)
#define nasys_data_mem          ((void *) 0x01000100)
#define nasys_data_mem_size               0x00ffff00
#define nasys_data_mem_end      ((void *) 0x02000000)
#define nasys_stack_top         ((void *) 0x02000000)
 
#define PLUGS_PLUG_COUNT           5 // Maximum number of plugs
#define PLUGS_ADAPTER_COUNT        2 // Maximum number of adapters
#define PLUGS_DNS                  1 // Have routines for DNS lookups
#define PLUGS_PING                 1 // Respond to icmp echo (ping) messages
#define PLUGS_TCP                  1 // Support tcp in/out connections
#define PLUGS_IRQ                  1 // Run at interrupte level
#define PLUGS_DEBUG                1 // Support debug routines
#define __nios_catch_irqs__        1 // Include panic handler for all irqs (needs uart)
#define __nios_use_constructors__  1 // Call c++ static constructors
#define __nios_use_cwpmgr__        1 // Handle register window underflows
#define __nios_use_fast_mul__      1 // Faster but larger int multiply routine
#define __nios_use_small_printf__  0 // Smaller non-ANSI printf, with no floating point
#define __nios_use_multiply__      0 // Use MUL instruction for 16x16
#define __nios_use_mstep__         0 // Use MSTEP instruction for 16x16



#define nm_system_name_string "LDK2"
#define nm_cpu_name_string "cpu"
#define nm_monitor_string "L20e1"
#define nm_cpu_architecture nios_32
#define nm_cpu_architecture_string "nios_32"
#define nios_32 1

// Structures and Routines For Each Peripheral


// Nios CPU Routines
void nr_installcwpmanager(void);	// called automatically at by nr_setup.s
void nr_delay(int milliseconds);	// approximate timing based on clock speed
void nr_zerorange(char *rangeStart,int rangeByteCount);
void nr_jumptoreset(void);

// Nios ISR Manager Routines
typedef void (*nios_isrhandlerproc)(int context);
typedef void (*nios_isrhandlerproc2)(int context,int irq_number,int interruptee_pc);
void nr_installuserisr(int trapNumber,nios_isrhandlerproc handlerProc,int context);
void nr_installuserisr2(int trapNumber,nios_isrhandlerproc2 handlerProc,int context);

// Nios GDB Stub Functions
void nios_gdb_install(int active);
#define nios_gdb_breakpoint() asm("TRAP 5")

// Default UART routines
void nr_txchar(int c);
void nr_txstring(char *s);
int nr_rxchar(void);

// Debug UART routines
void nr_debug_txchar(int c);
void nr_debug_txstring(char *s);
int nr_debug_rxchar(void);

// Nios Private Printf Routines
int nr_printf(const char *fmt,...);
int nr_sprintf(char *sOut,const char *fmt,...);

#if __nios_use_small_printf__
	#define printf nr_printf
	#define sprintf nr_sprintf
#endif

#ifdef na_enet
void nios_gdb_install_ethernet(int active);
int nr_debug_plugs_idle (void);
#endif

#if __nios_debug__
	#define NIOS_GDB_SETUP 		\
		nios_gdb_install(1);	\
		nios_gdb_breakpoint(void);
#else
	#define NIOS_GDB_SETUP
#endif

// Nios setjmp/longjmp support
// (This will collide with <setjmp.h>
// if you include it! These are the
// nios-correct versions, however.)

typedef int jmp_buf[2];

int nr_setjmp(jmp_buf env);
int nr_longjmp(jmp_buf env,int value);

#define setjmp(a) nr_setjmp((a))
#define longjmp(a,b) nr_longjmp((a),(b))


// debug Core Declarations

#define nasys_debug_core_irq                8

// debug registers offsets from base
enum
{
    np_debug_interrupt =		0,		//read-only,  4 bits, reading stops trace
    np_debug_n_samples_lsb,			//read-only, 16 bits
    np_debug_n_samples_msb,			//read-only, 16 bits
    np_debug_data_valid,				//read-only, 1 bit, 
    						//true when trace registers contain valid sample
    np_debug_trace_address,			//read-only, 16 or 32 bits
    np_debug_trace_data,				//read-only, 16 or 32 bits
    np_debug_trace_code,				//read-only, 16 or 32 bits
    np_debug_write_status,			//read-only, 1 bit, 
    						//true when read to readback tracedata
    np_debug_start,				//write-only, write any value to start
    np_debug_stop,				//write-only, write any value to stop
    np_debug_read_sample,				//write-only, write any value to read
    np_debug_trace_mode,				//write-only, 1 bit
    np_debug_mem_int_enable,			//write-only, 16 or 32 bits ?????
    np_debug_ext_brk_enable,			//write-only, 1 bit
    np_debug_sw_reset,				//write-only, reset sampels and trace memory

    np_debug_address_pattern_0 =	16,		//write-only, 16 or 32 bits
    np_debug_address_mask_0,			//write-only, 16 or 32 bits
    np_debug_data_pattern_0,			//write-only, 16 or 32 bits
    np_debug_data_mask_0,				//write-only, 16 or 32 bits
    np_debug_code_0,				//write-only, 16 or 32 bits

    np_debug_address_pattern_1 =	24,		//write-only, 16 or 32 bits
    np_debug_address_mask_1,			//write-only, 16 or 32 bits
    np_debug_data_pattern_1,			//write-only, 16 or 32 bits
    np_debug_data_mask_1,				//write-only, 16 or 32 bits
    np_debug_code_1,				//write-only, 16 or 32 bits
};

// debug Register Bits/Codes
enum 
{ 
    /************************************************/
    // debug_interrupt register
    // bit numbers
    np_debug_interrupt_code_dbp0_bit = 0,
    np_debug_interrupt_code_dbp1_bit = 1,
    np_debug_interrupt_code_ibp0_bit = 2,
    np_debug_interrupt_code_ibp1_bit = 3,
    np_debug_interrupt_code_mem_bit = 4,

    // bit masks
    np_debug_interrupt_code_ext_mask = (0), 
    np_debug_interrupt_code_dbp0_mask = (1<<0),
    np_debug_interrupt_code_dbp1_mask = (1<<1),
    np_debug_interrupt_code_ibp0_mask = (1<<2),
    np_debug_interrupt_code_ibp1_mask = (1<<3),
    np_debug_interrupt_code_mem_mask = (1<<4),

    /************************************************/
    // debug_trace_code register
    // bit numbers
    np_debug_trace_code_skp_bit = 1,
    np_debug_trace_code_fifo_full_bit = 2,
    np_debug_trace_code_bus_bit = 3,
    np_debug_trace_code_rw_bit = 4,
    np_debug_trace_code_intr_bit = 5,
    
    // bit masks
    np_debug_trace_code_skp_mask = (1<<0),
    np_debug_trace_code_fifo_full_mask = (1<<1),
    np_debug_trace_code_data_trans_mask = (1<<2), //instr trans if 0
    np_debug_trace_code_write_mask = (1<<3),	//read if 0
    np_debug_trace_code_intr_mask = (1<<4),
#ifdef __nios32__
    np_debug_trace_code_skp_cnt_mask = (63<<2),
#else
    np_debug_trace_code_skp_cnt_mask = (31<<2),
#endif

    // useful constants
    np_debug_trace_code_op_mask = (np_debug_trace_code_data_trans_mask|np_debug_trace_code_write_mask),
    np_debug_trace_code_read = np_debug_trace_code_data_trans_mask,
    np_debug_trace_code_write = (np_debug_trace_code_data_trans_mask|np_debug_trace_code_write_mask),
    np_debug_trace_code_fetch = 0,

    /************************************************/
    // debug_code_* registers
    // bit numbers
    np_debug_break_code_read_bit = 0,
    np_debug_break_code_write_bit = 1,
    np_debug_break_code_fetch_bit = 2,

    // bit masks
    np_debug_break_code_read_mask = (1<<0),
    np_debug_break_code_write_mask = (1<<1),
    np_debug_break_code_fetch_mask = (1<<2),

    /************************************************/
    // debug_write_status register
    // bit numbers
    np_debug_write_status_writing_bit = 0,
    np_debug_write_status_nios32_bit = 1,
    np_debug_write_status_trace_bit = 2,

    // bit masks
    np_debug_write_status_writing_mask = (1<<0),
    np_debug_write_status_nios32_mask = (1<<1),
    np_debug_write_status_trace_mask = (1<<2)
};

// debug Routine


//Get the number of trace samples 
unsigned long nr_debug_num_samples (void);	

//Stop debug core - this must be a function so that a branch is the last
//                  thing in the fifo.  Otherwise a skip or a read/write
//                  might be logged without the ability to sync it up
void nr_debug_stop (void);

//Read a trace sample
void nr_debug_get_sample (unsigned int *trace_addr, 
			unsigned int *trace_data, 
			unsigned char *trace_code);

//must have a uart for these functions
#ifdef nasys_uart_count
#if (nasys_uart_count > 0)

  //print cause of break
  void nr_debug_show_break (void *uart);
                                  // 0 for default printf uart

  //dump the trace memory
  void nr_debug_dump_trace (void *uart);
                             // 0 for defualt printf uart

  //ISR for debug interrupts
  //Show cause of break, dump trace, and halt
  void nr_debug_isr_halt (int context);		
                            // 0 for default printf uart otherwise uart base address

  //Show cause of break, dump trace, and continue
  void nr_debug_isr_continue (int context);		
                                // 0 for default printf uart otherwise uart base address

#endif
#endif

// debug macros
//Read a debug register
#define nm_debug_get_reg(ret, offset) 		\
		asm volatile (			\
			"PFX 3 \n		\
			WRCTL %1 \n		\
			PFX 4 \n		\
			RDCTL %0;"		\
			:"=r" (ret)		\
			:"r"(offset)		\
		);

//Write a debug register
#define nm_debug_set_reg(val,offset) 		\
		asm volatile (			\
			"PFX 3 \n		\
			WRCTL %1 \n		\
			PFX 4 \n		\
			WRCTL %0;"		\
			: /* no outputs */	\
			:"r"(val),"r"(offset)	\
		);

//Set breakpoint 0
#define nm_debug_set_bp0(ap,am,dp,dm,cd) nm_debug_set_reg(ap,np_debug_address_pattern_0);\
				       nm_debug_set_reg(am,np_debug_address_mask_0);\
				       nm_debug_set_reg(dp,np_debug_data_pattern_0);\
				       nm_debug_set_reg(dm,np_debug_data_mask_0);\
				       nm_debug_set_reg(cd,np_debug_code_0);

//Set breakpoint 1
#define nm_debug_set_bp1(ap,am,dp,dm,cd) nm_debug_set_reg(ap,np_debug_address_pattern_1);\
				       nm_debug_set_reg(am,np_debug_address_mask_1);\
				       nm_debug_set_reg(dp,np_debug_data_pattern_1);\
				       nm_debug_set_reg(dm,np_debug_data_mask_1);\
				       nm_debug_set_reg(cd,np_debug_code_1);

//Set extended trace mode
#define nm_debug_set_extended_trace nm_debug_set_reg(1,np_debug_trace_mode);

//Set memory interrupt point
#define nm_debug_set_wrap_point(size) nm_debug_set_reg((size>>2),np_debug_mem_int_enable);

// UART Registers
typedef volatile struct
	{
	int np_uartrxdata;      // Read-only, 8-bit
	int np_uarttxdata;      // Write-only, 8-bit
	int np_uartstatus;      // Read-only, 8-bit
	int np_uartcontrol;     // Read/Write, 9-bit
	int np_uartdivisor;     // Read/Write, 16-bit, optional
	int np_uartendofpacket; // Read/Write, end-of-packet character
	} np_uart;

// UART Status Register Bits
enum
	{
	np_uartstatus_eop_bit  = 12,
	np_uartstatus_cts_bit  = 11,
	np_uartstatus_dcts_bit = 10,
	np_uartstatus_e_bit    = 8,
	np_uartstatus_rrdy_bit = 7,
	np_uartstatus_trdy_bit = 6,
	np_uartstatus_tmt_bit  = 5,
	np_uartstatus_toe_bit  = 4,
	np_uartstatus_roe_bit  = 3,
	np_uartstatus_brk_bit  = 2,
	np_uartstatus_fe_bit   = 1,
	np_uartstatus_pe_bit   = 0,

	np_uartstatus_eop_mask  = (1<<12),
	np_uartstatus_cts_mask  = (1<<11),
	np_uartstatus_dcts_mask = (1<<10),
	np_uartstatus_e_mask    = (1<<8),
	np_uartstatus_rrdy_mask = (1<<7),
	np_uartstatus_trdy_mask = (1<<6),
	np_uartstatus_tmt_mask  = (1<<5),
	np_uartstatus_toe_mask  = (1<<4),
	np_uartstatus_roe_mask  = (1<<3),
	np_uartstatus_brk_mask  = (1<<2),
	np_uartstatus_fe_mask   = (1<<1),
	np_uartstatus_pe_mask   = (1<<0)
	};

// UART Control Register Bits
enum
	{
	np_uartcontrol_ieop_bit  = 12,
	np_uartcontrol_rts_bit   = 11,
	np_uartcontrol_idcts_bit = 10,
	np_uartcontrol_tbrk_bit  = 9,
	np_uartcontrol_ie_bit    = 8,
	np_uartcontrol_irrdy_bit = 7,
	np_uartcontrol_itrdy_bit = 6,
	np_uartcontrol_itmt_bit  = 5,
	np_uartcontrol_itoe_bit  = 4,
	np_uartcontrol_iroe_bit  = 3,
	np_uartcontrol_ibrk_bit  = 2,
	np_uartcontrol_ife_bit   = 1,
	np_uartcontrol_ipe_bit   = 0,

	np_uartcontrol_ieop_mask  = (1<<12),
	np_uartcontrol_rts_mask   = (1<<11),
	np_uartcontrol_idcts_mask = (1<<10),
	np_uartcontrol_tbrk_mask  = (1<<9),
	np_uartcontrol_ie_mask    = (1<<8),
	np_uartcontrol_irrdy_mask = (1<<7),
	np_uartcontrol_itrdy_mask = (1<<6),
	np_uartcontrol_itmt_mask  = (1<<5),
	np_uartcontrol_itoe_mask  = (1<<4),
	np_uartcontrol_iroe_mask  = (1<<3),
	np_uartcontrol_ibrk_mask  = (1<<2),
	np_uartcontrol_ife_mask   = (1<<1),
	np_uartcontrol_ipe_mask   = (1<<0)
	};

// UART Routines
int nr_uart_rxchar(np_uart *uartBase);        // 0 for default UART
void nr_uart_txcr(void);
void nr_uart_txchar(int c,np_uart *uartBase); // 0 for default UART
void nr_uart_txhex(int x);                     // 16 or 32 bits
void nr_uart_txhex16(short x);
void nr_uart_txhex32(long x);
void nr_uart_txstring(char *s);


// ----------------------------------------------
// Timer Peripheral

// Timer Registers
typedef volatile struct
	{
	int np_timerstatus;  // read only, 2 bits (any write to clear TO)
	int np_timercontrol; // write/readable, 4 bits
	int np_timerperiodl; // write/readable, 16 bits
	int np_timerperiodh; // write/readable, 16 bits
	int np_timersnapl;   // read only, 16 bits
	int np_timersnaph;   // read only, 16 bits
	} np_timer;

// Timer Register Bits
enum
	{
	np_timerstatus_run_bit    = 1, // timer is running
	np_timerstatus_to_bit     = 0, // timer has timed out

	np_timercontrol_stop_bit  = 3, // stop the timer
	np_timercontrol_start_bit = 2, // start the timer
	np_timercontrol_cont_bit  = 1, // continous mode
	np_timercontrol_ito_bit   = 0, // enable time out interrupt

	np_timerstatus_run_mask    = (1<<1), // timer is running
	np_timerstatus_to_mask     = (1<<0), // timer has timed out

	np_timercontrol_stop_mask  = (1<<3), // stop the timer
	np_timercontrol_start_mask = (1<<2), // start the timer
	np_timercontrol_cont_mask  = (1<<1), // continous mode
	np_timercontrol_ito_mask   = (1<<0)  // enable time out interrupt
	};

// Timer Routines
int nr_timer_milliseconds(void);	// Starts on first call, hogs timer1.

// PIO Peripheral

// PIO Registers
typedef volatile struct
	{
	int np_piodata;          // read/write, up to 32 bits
	int np_piodirection;     // write/readable, up to 32 bits, 1->output bit
	int np_piointerruptmask; // write/readable, up to 32 bits, 1->enable interrupt
	int np_pioedgecapture;   // read, up to 32 bits, cleared by any write
	} np_pio;

// PIO Routines
void nr_pio_showhex(int value); // shows low byte on pio named na_seven_seg_pio

// SPI Registers
typedef volatile struct
	{
	int np_spirxdata;       // Read-only, 1-16 bit
	int np_spitxdata;       // Write-only, same width as rxdata
	int np_spistatus;       // Read-only, 9-bit
	int np_spicontrol;      // Read/Write, 9-bit
	int np_spireserved;     // reserved
	int np_spislaveselect;  // Read/Write, 1-16 bit, master only
  int np_spiendofpacket;  // Read/write, same width as txdata, rxdata.
	} np_spi;

// SPI Status Register Bits
enum
{
  np_spistatus_eop_bit  = 9,
	np_spistatus_e_bit    = 8,
	np_spistatus_rrdy_bit = 7,
	np_spistatus_trdy_bit = 6,
	np_spistatus_tmt_bit  = 5,
	np_spistatus_toe_bit  = 4,
	np_spistatus_roe_bit  = 3,

  np_spistatus_eop_mask  = (1 << 9),
	np_spistatus_e_mask    = (1 << 8),
	np_spistatus_rrdy_mask = (1 << 7),
	np_spistatus_trdy_mask = (1 << 6),
	np_spistatus_tmt_mask  = (1 << 5),
	np_spistatus_toe_mask  = (1 << 4),
	np_spistatus_roe_mask  = (1 << 3),
	};

// SPI Control Register Bits
enum
	{
  np_spicontrol_ieop_bit  = 9,
	np_spicontrol_ie_bit    = 8,
	np_spicontrol_irrdy_bit = 7,
	np_spicontrol_itrdy_bit = 6,
	np_spicontrol_itoe_bit  = 4,
	np_spicontrol_iroe_bit  = 3,

  np_spicontrol_ieop_mask  = (1 << 9),
	np_spicontrol_ie_mask    = (1 << 8),
	np_spicontrol_irrdy_mask = (1 << 7),
	np_spicontrol_itrdy_mask = (1 << 6),

	np_spicontrol_itoe_mask  = (1 << 4),
	np_spicontrol_iroe_mask  = (1 << 3),
	};

// SPI Routines.
int nr_spi_rxchar(np_spi *spiBase);
int nr_spi_txchar(int i, np_spi *spiBase);



// ----------------------------------------------
// User Socket

typedef void *np_usersocket;


// Nios Flash Memory Routines

// All routines take a "flash base" parameter.  If -1 is supplied,
// nasys_main_flash is used.

int nr_flash_erase_sector
		(
		unsigned short *flash_base,
  		unsigned short *sector_address
		);

int nr_flash_erase
		(
		unsigned short *flash_base
		);

int nr_flash_write
		(
		unsigned short *flash_base,
  		unsigned short *address,
		unsigned short value
		);

int nr_flash_write_buffer
		(
		unsigned short *flash_base,
  		unsigned short *start_address,
  		unsigned short *buffer,
		int halfword_count
		);





// ===========================================================
// Parameters for Each Peripheral, Excerpted From The PTF File



// ------------------
// Parameters for altera_avalon_onchip_memory named mon

// Writeable              = 0
// Size_Value             = 1
// Size_Multiple          = 1024
// Contents               = blank
// Shrink_to_fit_contents = 0
// CONTENTS               = srec
// use_altsyncram         = 0



// ------------------
// Parameters for altera_nios named cpu

// CPU_Architecture     = nios_32
// mstep                = 1
// multiply             = 0
// rom_decoder          = 1
// wvalid_wr            = 0
// num_regs             = 512
// do_generate          = 1
// include_debug        = 0
// include_trace        = 0
// reset_slave          = mon/s1
// reset_offset         = 0x00000000
// vecbase_slave        = sdram/s1
// vecbase_offset       = 0x00000000
// support_interrupts   = 1
// implement_forward_b1 = 1
// support_rlc_rrc      = 0
// advanced             = 1
// CONSTANTS            =
// mainmem_slave        = sdram/s1
// datamem_slave        = sdram/s1
// maincomm_slave       = uart0/s1
// debugcomm_slave      = uart1/s1
// germs_monitor_id     = L20e1



// ------------------
// Parameters for altera_avalon_uart named uart0

// baud             = 115200
// data_bits        = 8
// fixed_baud       = 1
// parity           = N
// stop_bits        = 2
// use_cts_rts      = 0
// use_eop_register = 0
// sim_true_baud    = 0
// sim_char_stream  =



// ------------------
// Parameters for altera_avalon_timer named timer0

// always_run           = 0
// fixed_period         = 0
// snapshot             = 1
// period               = 1
// period_units         = msec
// reset_output         = 0
// timeout_pulse_output = 0
// mult                 = 0.001



// ------------------
// Parameters for altera_avalon_pio named ssram_detect

// has_tri   = 0
// has_out   = 0
// has_in    = 1
// capture   = 0
// edge_type = NONE
// irq_type  = NONE



// ------------------
// Parameters for altera_avalon_pio named button_pio

// has_tri   = 0
// has_out   = 0
// has_in    = 1
// capture   = 0
// edge_type = NONE
// irq_type  = NONE



// ------------------
// Parameters for altera_avalon_spi named spi

// databits      = 16
// targetclock   = 250
// clockunits    = kHz
// clockmult     = 1000
// numslaves     = 2
// ismaster      = 1
// clockpolarity = 0
// clockphase    = 1
// lsbfirst      = 0
// extradelay    = 1
// targetssdelay = 2
// delayunits    = us
// delaymult     = 1e-006
// clockunit     = kHz
// delayunit     = us



// ------------------
// Parameters for altera_avalon_uart named uart1

// baud             = 115200
// data_bits        = 8
// fixed_baud       = 1
// parity           = N
// stop_bits        = 1
// use_cts_rts      = 0
// use_eop_register = 0
// sim_true_baud    = 0
// sim_char_stream  =



// ------------------
// Parameters for altera_avalon_user_defined_interface named ide_interface

// HDL_Import              = 0
// Imported_Wait           = 0
// Nios_Gen_Waits          = 0
// Synthesize_Imported_HDL = 1
// Component_Desc          =
// Component_Name          =
// Module_Name             =
// Technology              =
// Port_Type               = Avalon Slave
// Timing_Units            = ns
// Address_Width           = 32
// Module_List             =



// ------------------
// Parameters for altera_avalon_lan91c111 named enet

// CONSTANTS =



// ------------------
// Parameters for altera_avalon_pio named enet_reset_n

// has_tri   = 0
// has_out   = 1
// has_in    = 0
// capture   = 0
// edge_type = NONE
// irq_type  = NONE



// ------------------
// Parameters for altera_nios_dev_board_flash named flash

// CONTENTS = srec



// ------------------
// Parameters for altera_avalon_sdram_controller named sdram

// sdram_data_width      = 32
// sdram_addr_width      = 11
// sdram_bank_width      = 2
// sdram_row_width       = 11
// sdram_col_width       = 8
// sdram_num_chipselects = 2
// refresh_period        = 15.625
// powerup_delay         = 100
// cas_latency           = 2
// precharge_control_bit = 10
// t_rfc                 = 70
// t_rp                  = 20
// t_mrd                 = 2
// t_rcd                 = 20
// t_ac                  = 8
// t_wr_auto_precharge_a = 1
// t_wr_auto_precharge_b = 7
// t_wr_precharge        = 14
// init_refresh_commands = 2
// init_nop_delay        = 0
// shared_data           = 0
// enable_ifetch         = 0
// highperf              = 0
// sim_model_base        =



#ifdef __cplusplus
}
#endif

#endif //_excalibur_

// end of file
