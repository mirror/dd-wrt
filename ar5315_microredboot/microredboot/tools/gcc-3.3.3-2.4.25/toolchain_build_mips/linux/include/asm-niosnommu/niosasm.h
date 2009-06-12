
;------------------------------
; Macros I: Faux Instructions
;
; The following "faux instructions" are
; implemented here as macros:
;
; MOVIP register,constant		MOVI with optional PFX & MOVHI, or BGEN
; ADDIP register,constant		PFX and ADDI with optional PFX
; SUBIP register,constant		PFX and SUBI with optional PFX
; CMPIP register,constant		PFX and CMPI with optional PFX
;
; MOVI16 register,constant		PFX and MOVI
; MOVI32 register,constant		PFX, MOVI, PFX, and MOVHI
; MOVIA  register,constant		PFX and MOVHI on Nios32, and PFX and MOVI
;
; ANDIP register,constant		PFX and ANDI
; ANDNIP register,constant		PFX and ANDN
; ORIP register,constant		PFX and ORI
; XORIP register,constant		PFX and XORI
;
; _BSR address						MOVIP address to %g7, and CALL
; _BR address						MOVIP address to %g7, and JMP
;
; BEQ address						SKPS cc_nz and BR, has delay slot
; BNE address						SKPS cc_z and BR, has delay slot
; BLE address						SKPS cc_gt and BR, has delay slot
; BLT address						SKPS cc_ge and BR, has delay slot
; RESTRET							RESTORE and JMP %i7 
;
;-------------------------------
; Macros II: Printing
;
; These macros are guaranteed *not*
; to have branch delay slot after them.
;
; nm_printchar char
; nm_print "string"
; nm_println "string"			Follows it with a carriage return
; nm_printregister reg			For debugging, prints register name & value
;
;-------------------------------
; Macros III: Inline Debugging
;
; These macros print various information
; using large sections of expanded inline code.
; They each use either few or no registers.
; Thus, they may be safely used in interrupt handlers.
;
; nm_d_txchar char			print char to UART, affects no registers
; nm_d_txregister char,char,register	prints the two characters, and the hex register value

; --------------------------------------


		.macro	_pfx_op	OP,reg,val,pForce=0
		.if		(\pForce) || ((\val) > (31)) || ((\val) < (0))
		PFX		%hi(\val)
		.endif
		\OP		\reg,%lo(\val)
		.endm

		.macro	_bgen reg,val,bit
		.if ((\val)==(1<<\bit))
		BGEN	\reg,\bit
		.equ	_bgenBit,1
		.endif
		.endm

	;------------------------
	; MOVIP %reg,32-bit-value
		.macro	MOVIP reg,val
		; Methodically test every BGEN possibility...
		.equ	_bgenBit,0
.if 1
		_bgen \reg,\val,0
		_bgen \reg,\val,1
		_bgen \reg,\val,2
		_bgen \reg,\val,3
		_bgen \reg,\val,4
		_bgen \reg,\val,5
		_bgen \reg,\val,6
		_bgen \reg,\val,7
		_bgen \reg,\val,8
		_bgen \reg,\val,9
		_bgen \reg,\val,10
		_bgen \reg,\val,11
		_bgen \reg,\val,12
		_bgen \reg,\val,13
		_bgen \reg,\val,14
		_bgen \reg,\val,15
		_bgen \reg,\val,16
		_bgen \reg,\val,17
		_bgen \reg,\val,18
		_bgen \reg,\val,19
		_bgen \reg,\val,20
		_bgen \reg,\val,21
		_bgen \reg,\val,22
		_bgen \reg,\val,23
		_bgen \reg,\val,24
		_bgen \reg,\val,25
		_bgen \reg,\val,26
		_bgen \reg,\val,27
		_bgen \reg,\val,28
		_bgen \reg,\val,29
		_bgen \reg,\val,30
		_bgen \reg,\val,31

		; If no bgen fit...
.endif
		.if !_bgenBit
			.if ((\val) & 0xFFE0)
				PFX %hi(\val)
			.endif
			MOVI \reg,%lo(\val)
			.if __nios32__
				.if ((\val) & 0xffff0000)
					.if ((\val) & 0xFFE00000)
						PFX %xhi(\val)
					.endif
					MOVHI \reg,%xlo(\val)
				.endif
			.endif
		.endif

		.endm

	; ADDIP %reg,16-bit-value
		.macro	ADDIP reg,val
		_pfx_op	ADDI,\reg,\val
		.endm

	; SUBIP %reg,16-bit-value
		.macro	SUBIP reg,val
		_pfx_op	SUBI,\reg,\val
		.endm

	; CMPIP %reg,16-bit-value
		.macro	CMPIP reg,val
		_pfx_op	CMPI,\reg,\val
		.endm

	; ANDIP %reg,16-bit-value
		.macro	ANDIP reg,val
		PFX		%hi(\val)
		AND		\reg,%lo(\val)
		.endm

	; ANDNIP %reg,16-bit-value
		.macro	ANDNIP reg,val
		PFX		%hi(\val)
		ANDN		\reg,%lo(\val)
		.endm

	; ORIP %reg,16-bit-value
		.macro	ORIP reg,val
		PFX		%hi(\val)
		OR			\reg,%lo(\val)
		.endm

	; XORIP %reg,16-bit-value
		.macro	XORIP reg,val
		PFX		%hi(\val)
		XOR		\reg,%lo(\val)
		.endm

	; BEQ addr
		.macro	BEQ addr
		IFS		cc_eq
		BR			\addr
		.endm

	; BNE addr
		.macro	BNE addr
		IFS		cc_ne
		BR			\addr
		.endm

	; BLE addr
		.macro	BLE addr
		SKPS		cc_gt
		BR			\addr
		.endm

	; BLT addr
		.macro	BLT addr
		SKPS		cc_ge
		BR			\addr
		.endm

		.macro	digitToChar reg
		ANDIP	\reg,0x000f
		CMPI	\reg,10
		SKPS	cc_lt
		ADDI	\reg,'A'-'0'-10
		PFX		%hi('0')
		ADDI	\reg,%lo('0')
		.endm

; PUSHRET == dec sp, and stash return addr
	.macro	PUSHRET
	SUBI		%sp,2
	ST			[%sp],%o7
	.endm
; POPRET == pop and jump
	.macro	POPRET
	LD			%o7,[%sp]
	JMP		%o7
	ADDI		%sp,2		; branch delay slot
	.endm

; RESTRET = restore & return
	.macro	RESTRET
	JMP		%i7
	RESTORE
	.endm

	;--------------------
	; MOVI16 %reg,Address
	;
	.macro	MOVI16	reg,val
	PFX	%hi(\val)
	MOVI	\reg,%lo(\val)
	.endm

	;--------------------
	; MOVI32 %reg,Address
	;
	.macro	MOVI32	reg,val
	PFX	%hi(\val)
	MOVI	\reg,%lo(\val)
	PFX	%xhi(\val)
	MOVHI	\reg,%xlo(\val)
	.endm

	;--------------------
	; MOVIA %reg,Address
	;
	.macro	MOVIA		reg,val
	.if __nios32__
		MOVI32 \reg,\val
	.else
		MOVI16 \reg,\val
	.endif
	.endm

	;--------------------
	; _BR

	.macro _BR target,viaRegister=%g7
	MOVIA	\viaRegister,\target@h
	JMP	\viaRegister
	.endm

	;--------------------
	; _BSR

	.macro _BSR target,viaRegister=%g7
	MOVIA	\viaRegister,\target@h
	CALL	\viaRegister
	.endm

	;---------------------
	; nm_print "Your String Here"
	;
	.macro	nm_print	string

	BR		pastStringData\@
	NOP

stringData\@:
	.asciz	"\string"
	.align 1		; aligns by 2^n
pastStringData\@:
	MOVIA		%o0,stringData\@
	_BSR		nr_uart_txstring
	NOP
	.endm

	.macro	nm_println string
	nm_print	"\string"
	_BSR		nr_uart_txcr
	NOP
	.endm

	.macro	nm_printregister reg	; affects %g0 & %g1 & %g7, but thrashes the CWP a bit
	SAVE		%sp,-16
	nm_print	"\reg = "
	RESTORE
	MOV		%g0,\reg
	SAVE		%sp,-16
	MOV		%o0,%g0
	_BSR		nr_uart_txhex
	NOP
	_BSR		nr_uart_txcr
	NOP
	RESTORE
	.endm

	.macro	nm_printchar char
	MOVIP		%o0,\char
	_BSR		nr_uart_txchar
	MOVI		%o1,0
	.endm

	.macro	nm_print2chars char1,char2
	MOVIP		%o0,(\char2<<8)+\char1
	_BSR		nr_uart_txchar
	NOP
	_BSR		nr_uart_txchar
	LSRI		%o0,8
	.endm



; ---------------------------
; Completely inline UART sends
; Send the char, or %g7 if not there.
; Trashes %g5 and %g6 and %g7...

	.macro	nm_txchar char=0
;nm_d_delay 1000
	MOVIA	%g6,nasys_printf_uart
txCharLoop\@:
	PFX	2
.if \char
	LD	%g7,[%g6]
	SKP1	%g7,6
.else
	LD	%g5,[%g6]
	SKP1	%g5,6
.endif
	BR	txCharLoop\@
	NOP
.if \char
	MOVIP	%g7,\char
.endif
	PFX	1
	ST	[%g6],%g7
;nm_d_delay 4
	.endm

		.macro nm_txcr
		nm_txchar 13
		nm_txchar 10
		.endm

		.macro nm_txhexdigit,reg,shift
		MOV		%g7,\reg
		LSRI		%g7,\shift
		ANDIP		%g7,0x000f
		CMPI		%g7,10
		SKPS		cc_lt
		ADDIP		%g7,'A'-'0'-10
		ADDIP		%g7,'0'
		nm_txchar
		.endm

		.macro nm_txhex

	.if __nios32__
		nm_txhexdigit %g0,28
		nm_txhexdigit %g0,24
		nm_txhexdigit %g0,20
		nm_txhexdigit %g0,16
	.endif

		nm_txhexdigit %g0,12
		nm_txhexdigit %g0,8
		nm_txhexdigit %g0,4
		nm_txhexdigit %g0,0
		.endm










; ----------------------
; The following macros are
; rather mighty. They expand
; to large inline code for
; printing various things to
; the serial port. They are
; useful for debugging
; trap handlers, where you
; can't just go and call
; nr_uart_TxChar and such, because,
; well, the CWP might be
; off limits!
;
; They do, however, presume
; that the stack is in good
; working order.


.macro nm_d_pushgregisters
 	SUBIP %sp,16+69				; oddball number so if we accidentally see it, it looks funny.
	STS	[%sp,16+0],%g0
	STS	[%sp,16+1],%g1
	STS	[%sp,16+2],%g2
	STS	[%sp,16+3],%g3
	STS	[%sp,16+4],%g4
	STS	[%sp,16+5],%g5
	STS	[%sp,16+6],%g6
	STS	[%sp,16+7],%g7
	.endm

.macro nm_d_popgregisters
	LDS	%g0,[%sp,16+0]
	LDS	%g1,[%sp,16+1]
	LDS	%g2,[%sp,16+2]
	LDS	%g3,[%sp,16+3]
	LDS	%g4,[%sp,16+4]
	LDS	%g5,[%sp,16+5]
	LDS	%g6,[%sp,16+6]
	LDS	%g7,[%sp,16+7]
	ADDIP	%sp,16+69				; must match the push
	.endm


.macro nm_d_txchar	c
	SUBI	%sp,16+8		; 32 or 16 bit, that's enough space
	STS	[%sp,16+0],%g6
	STS	[%sp,16+1],%g7
	nm_txchar \c
	LDS	%g6,[%sp,16+0]
	LDS	%g7,[%sp,16+1]
	ADDI	%sp,16+8
	.endm

.macro nm_d_txchar3 c1,c2,c3
 nm_d_txchar '<'
 nm_d_txchar \c1
 nm_d_txchar \c2
 nm_d_txchar \c3
 nm_d_txchar '>'
.endm

.macro nm_d_txregister r,n,reg
 nm_d_pushgregisters
 nm_txchar '('
 nm_txchar \r
 nm_txchar \n
 nm_txchar ':'
 MOV		%g0,\reg
 nm_txhex
 nm_txchar ')'
 nm_d_popgregisters
.endm

.macro nm_d_txreg r,n,reg
	nm_d_txregister \r,\n,\reg
.endm

; Do a delay loop, affects no registers.

.macro nm_d_delay d
	SUBI	%sp,16+4
	STS	[%sp,16+0],%g0
	MOVIP	%g0,\d
nm_d_delayloop\@:
	IFRnz	%g0
	 BR	nm_d_delayloop\@
	SUBI	%g0,1
	LDS	%g0,[%sp,16+0]
	ADDI	%sp,16+4
.endm

;
; File: excalibur.s
;
; This file is a machine generated address map
; for a CPU named cpu using toolchain "gnu".
; C:/Altera/Excalibur/sopc_builder_2_6_1/examples/verilog/nios_dev_board/LDK_2_0_with_91C111_full/LDK2.ptf
; Generated: 2002.09.26 13:43:23
;

.ifndef _excalibur_
.equ _excalibur_,1


	; minor macro to .equate and .global
	.macro	GEQU sym,val
	.global \sym
	.equ \sym,\val
	.endm

; The Memory Map

	GEQU na_mon               , 0x00000000 ; altera_avalon_onchip_memory
	GEQU na_mon_end           , 0x00000400
	GEQU na_cpu               , 0x00000000 ; altera_nios
	GEQU na_uart0             , 0x00000400 ; altera_avalon_uart
	GEQU na_uart0_irq         , 17
	GEQU na_timer0            , 0x00000440 ; altera_avalon_timer
	GEQU na_timer0_irq        , 16
	GEQU na_ssram_detect      , 0x00000460 ; altera_avalon_pio
	GEQU na_button_pio        , 0x00000470 ; altera_avalon_pio
	GEQU na_spi               , 0x00000480 ; altera_avalon_spi
	GEQU na_spi_irq           , 19
	GEQU na_uart1             , 0x000004a0 ; altera_avalon_uart
	GEQU na_uart1_irq         , 18
	GEQU na_ide_interface     , 0x00000500 ; altera_avalon_user_defined_interface
	GEQU na_ide_interface_irq , 25
	GEQU na_enet              , 0x00004000 ; altera_avalon_lan91c111
	GEQU na_enet_end          , 0x00004020
	GEQU na_enet_irq          , 35
	GEQU na_enet_reset_n      , 0x00004020 ; altera_avalon_pio
	GEQU na_flash             , 0x00100000 ; altera_nios_dev_board_flash
	GEQU na_flash_end         , 0x00200000
	GEQU na_flash_kernel      , 0x00800000 ; mtx_sodimm_flash_controller
	GEQU na_flash_kernel_end  , 0x01000000
	GEQU na_sdram             , 0x01000000 ; altera_avalon_sdram_controller
	GEQU na_sdram_end         , 0x02000000
 
	GEQU na_null                 , 0
	GEQU nasys_flash_count       , 1
	GEQU nasys_flash_0           , na_flash
	GEQU nasys_pio_count         , 3
	GEQU nasys_pio_0             , na_ssram_detect
	GEQU nasys_pio_1             , na_button_pio
	GEQU nasys_pio_2             , na_enet_reset_n
	GEQU nasys_spi_count         , 1
	GEQU nasys_spi_0             , na_spi
	GEQU nasys_spi_0_irq         , 19
	GEQU nasys_timer_count       , 1
	GEQU nasys_timer_0           , na_timer0
	GEQU nasys_timer_0_irq       , 16
	GEQU nasys_uart_count        , 2
	GEQU nasys_uart_0            , na_uart0
	GEQU nasys_uart_0_irq        , 17
	GEQU nasys_uart_1            , na_uart1
	GEQU nasys_uart_1_irq        , 18
	GEQU nasys_usersocket_count  , 1
	GEQU nasys_usersocket_0      , na_ide_interface
	GEQU nasys_usersocket_0_irq  , 25
	GEQU nasys_vector_table      , 0x01000000
	GEQU nasys_vector_table_size , 0x00000100
	GEQU nasys_vector_table_end  , 0x01000100
	GEQU nasys_reset_address     , 0x00000000
	GEQU nasys_clock_freq        , 33333000
	GEQU nasys_clock_freq_1000   , 33333
	GEQU nasys_debug_core        , 0
	GEQU nasys_printf_uart       , na_uart0
	GEQU nasys_printf_uart_irq   , na_uart0_irq
	GEQU nasys_debug_uart        , na_uart1
	GEQU nasys_debug_uart_irq    , na_uart1_irq
	GEQU nasys_main_flash        , na_flash
	GEQU nasys_main_flash_size   , na_flash_size
	GEQU nasys_main_flash_end    , na_flash_end
	GEQU nasys_program_mem       , 0x01000100
	GEQU nasys_program_mem_size  , 0x00ffff00
	GEQU nasys_program_mem_end   , 0x02000000
	GEQU nasys_data_mem          , 0x01000100
	GEQU nasys_data_mem_size     , 0x00ffff00
	GEQU nasys_data_mem_end      , 0x02000000
	GEQU nasys_stack_top         , 0x02000000
 
	.equ PLUGS_PLUG_COUNT          , 5 ; Maximum number of plugs
	.equ PLUGS_ADAPTER_COUNT       , 2 ; Maximum number of adapters
	.equ PLUGS_DNS                 , 1 ; Have routines for DNS lookups
	.equ PLUGS_PING                , 1 ; Respond to icmp echo (ping) messages
	.equ PLUGS_TCP                 , 1 ; Support tcp in/out connections
	.equ PLUGS_IRQ                 , 1 ; Run at interrupte level
	.equ PLUGS_DEBUG               , 1 ; Support debug routines
	.equ __nios_catch_irqs__       , 1 ; Include panic handler for all irqs (needs uart)
	.equ __nios_use_constructors__ , 1 ; Call c++ static constructors
	.equ __nios_use_cwpmgr__       , 1 ; Handle register window underflows
	.equ __nios_use_fast_mul__     , 1 ; Faster but larger int multiply routine
	.equ __nios_use_small_printf__ , 1 ; Smaller non-ANSI printf, with no floating point
	.equ __nios_use_multiply__     , 0 ; Use MUL instruction for 16x16
	.equ __nios_use_mstep__        , 0 ; Use MSTEP instruction for 16x16



	.macro	nm_system_name_string
	.asciz	"LDK2"
	.endm

	.macro	nm_cpu_name_string
	.asciz	"cpu"
	.endm

	.macro	nm_monitor_string
	.asciz	"L20e1"
	.endm

	.equ	nios_32,1

	.macro	nm_cpu_architecture
	.asciz	"nios_32"
	.endm

; Structures and Routines For Each Peripheral



.if __nios32__
    .equ nios_wordsize,4
.else
    .equ nios_wordsize,2
.endif

    GEQU    nasys_debug_core_irq,     8

;;; debug registers offsets from base
    GEQU    np_debug_interrupt,        0        ;read-only,  4 bits, reading stops trace
    GEQU    np_debug_n_samples_lsb,    1        ;read-only, 16 bits
    GEQU    np_debug_n_samples_msb,    2        ;read-only, 16 bits
    GEQU    np_debug_data_valid,       3        ;read-only, 1 bit, 
                                                ;true when trace registers contain valid sample
    GEQU    np_debug_trace_address,    4        ;read-only, 16 or 32 bits
    GEQU    np_debug_trace_data,       5        ;read-only, 16 or 32 bits
    GEQU    np_debug_trace_code,       6        ;read-only, 16 or 32 bits
    GEQU    np_debug_write_status,     7        ;read-only, 1 bit, 
                                                ;true when read to readback tracedata
    GEQU    np_debug_start,            8        ;write-only, write any value to start
    GEQU    np_debug_stop,             9        ;write-only, write any value to stop
    GEQU    np_debug_read_sample,      10        ;write-only, write any value to read
    GEQU    np_debug_trace_mode,       11        ;write-only, 1 bit
    GEQU    np_debug_mem_int_enable,   12        ;write-only, 16 or 32 bits ?????
    GEQU    np_debug_ext_brk_enable,   13        ;write-only, 1 bit
    GEQU    np_debug_sw_reset,         14        ;write-only, reset sampels and trace memory
    
    GEQU    np_debug_address_pattern_0,16        ;write-only, 16 or 32 bits
    GEQU    np_debug_address_mask_0,   17        ;write-only, 16 or 32 bits
    GEQU    np_debug_data_pattern_0,   18        ;write-only, 16 or 32 bits
    GEQU    np_debug_data_mask_0,      19        ;write-only, 16 or 32 bits
    GEQU    np_debug_code_0,           20        ;write-only, 16 or 32 bits
    
    GEQU    np_debug_address_pattern_1,24        ;write-only, 16 or 32 bits
    GEQU    np_debug_address_mask_1,   25        ;write-only, 16 or 32 bits
    GEQU    np_debug_data_pattern_1,   26        ;write-only, 16 or 32 bits
    GEQU    np_debug_data_mask_1,      27        ;write-only, 16 or 32 bits
    GEQU    np_debug_code_1,           28        ;write-only, 16 or 32 bits

;;; debug Register Bits/Codes
;;; ************************************************
;;; debug_interrupt register
;;; bit numbers
    GEQU    np_debug_interrupt_code_dbp0_bit, 0
    GEQU    np_debug_interrupt_code_dbp1_bit, 1
    GEQU    np_debug_interrupt_code_ibp0_bit, 2
    GEQU    np_debug_interrupt_code_ibp1_bit, 3
    GEQU    np_debug_interrupt_code_mem_bit, 4

;;; bit masks
    GEQU    np_debug_interrupt_code_ext_mask,  (0)
    GEQU    np_debug_interrupt_code_dbp0_mask, (1<<0)
    GEQU    np_debug_interrupt_code_dbp1_mask, (1<<1)
    GEQU    np_debug_interrupt_code_ibp0_mask, (1<<2)
    GEQU    np_debug_interrupt_code_ibp1_mask, (1<<3)
    GEQU    np_debug_interrupt_code_mem_mask,  (1<<4)

;;; ************************************************
;;; debug_trace_code register
;;; bit numbers
    GEQU    np_debug_trace_code_skp_bit, 1
    GEQU    np_debug_trace_code_fifo_full_bit, 2
    GEQU    np_debug_trace_code_bus_bit, 3
    GEQU    np_debug_trace_code_rw_bit, 4
    GEQU    np_debug_trace_code_intr_bit, 5
    
;;; bit masks
    GEQU    np_debug_trace_code_skp_mask,       (1<<0)
    GEQU    np_debug_trace_code_fifo_full_mask, (1<<1)
    GEQU    np_debug_trace_code_data_trans_mask,(1<<2)     ;instr trans if 0
    GEQU    np_debug_trace_code_write_mask,     (1<<3)     ;read if 0
    GEQU    np_debug_trace_code_intr_mask,      (1<<4)
.if __nios32__
    GEQU    np_debug_trace_code_skp_cnt_mask, (63<<2)
.else
    GEQU    np_debug_trace_code_skp_cnt_mask, (31<<2)
.endif

;;; useful constants
    GEQU    np_debug_trace_code_op_mask, (np_debug_trace_code_data_trans_mask|np_debug_trace_code_write_mask)
    GEQU    np_debug_trace_code_read, np_debug_trace_code_data_trans_mask
    GEQU    np_debug_trace_code_write, (np_debug_trace_code_data_trans_mask|np_debug_trace_code_write_mask)
    GEQU    np_debug_trace_code_fetch, 0
    
;;; ************************************************
;;; debug_code_* registers
;;; bit numbers
    GEQU    np_debug_break_code_read_bit,  0
    GEQU    np_debug_break_code_write_bit, 1
    GEQU    np_debug_break_code_fetch_bit, 2
    
;;; bit masks
    GEQU    np_debug_break_code_read_mask,  (1<<0)
    GEQU    np_debug_break_code_write_mask, (1<<1)
    GEQU    np_debug_break_code_fetch_mask, (1<<2)
    
;;; ************************************************
;;; debug_write_status register
;;; bit numbers
    GEQU    np_debug_write_status_writing_bit, 0
    GEQU    np_debug_write_status_nios32_bit,  1
    GEQU    np_debug_write_status_trace_bit,   2
    
;;; bit masks
    GEQU    np_debug_write_status_writing_mask, (1<<0)
    GEQU    np_debug_write_status_nios32_mask,  (1<<1)
    GEQU    np_debug_write_status_trace_mask,   (1<<2)
    
; ----------------------------------------------
; UART Peripheral
;
	;
	; UART Registers
	;
	.equ np_uartrxdata,      0 ; Read-only, 8-bit
	.equ np_uarttxdata,      1 ; Write-only, 8-bit
	.equ np_uartstatus,      2 ; Read-only, 8-bit
	.equ np_uartcontrol,     3 ; Read/Write, 9-bit
	.equ np_uartdivisor,     4 ; Read/Write, 16-bit, optional
	.equ np_uartendofpacket, 5 ; Read/Write, end-of-packet character

	;
	; UART Status Register
	;
	.equ np_uartstatus_eop_mask,  (1<<12)
	.equ np_uartstatus_cts_mask,  (1<<11)
	.equ np_uartstatus_dcts_mask, (1<<10)
	.equ np_uartstatus_e_mask,    (1<<8)
	.equ np_uartstatus_rrdy_mask, (1<<7)
	.equ np_uartstatus_trdy_mask, (1<<6)
	.equ np_uartstatus_tmt_mask,  (1<<5)
	.equ np_uartstatus_toe_mask,  (1<<4)
	.equ np_uartstatus_roe_mask,  (1<<3)
	.equ np_uartstatus_brk_mask,  (1<<2)
	.equ np_uartstatus_fe_mask,   (1<<1)
	.equ np_uartstatus_pe_mask,   (1<<0)

	.equ np_uartstatus_eop_bit,  12
	.equ np_uartstatus_cts_bit,  11
	.equ np_uartstatus_dcts_bit, 10
	.equ np_uartstatus_e_bit,    8
	.equ np_uartstatus_rrdy_bit, 7
	.equ np_uartstatus_trdy_bit, 6
	.equ np_uartstatus_tmt_bit,  5
	.equ np_uartstatus_toe_bit,  4
	.equ np_uartstatus_roe_bit,  3
	.equ np_uartstatus_brk_bit,  2
	.equ np_uartstatus_fe_bit,   1
	.equ np_uartstatus_pe_bit,   0

	;
	; UART Control Register
	;
	.equ np_uartcontrol_eop_mask,   (1<<10)
	.equ np_uartcontrol_tbrk_mask,  (1<<9)
	.equ np_uartcontrol_ie_mask,    (1<<8)
	.equ np_uartcontrol_irrdy_mask, (1<<7)
	.equ np_uartcontrol_itrdy_mask, (1<<6)
	.equ np_uartcontrol_itmt_mask,  (1<<5)
	.equ np_uartcontrol_itoe_mask,  (1<<4)
	.equ np_uartcontrol_iroe_mask,  (1<<3)
	.equ np_uartcontrol_ibrk_mask,  (1<<2)
	.equ np_uartcontrol_ife_mask,   (1<<1)
	.equ np_uartcontrol_ipe_mask,   (1<<0)

	.equ np_uartcontrol_eop_bit,  10
	.equ np_uartcontrol_tbrk_bit,  9
	.equ np_uartcontrol_ie_bit,    8
	.equ np_uartcontrol_irrdy_bit, 7
	.equ np_uartcontrol_itrdy_bit, 6
	.equ np_uartcontrol_itmt_bit,  5
	.equ np_uartcontrol_itoe_bit,  4
	.equ np_uartcontrol_iroe_bit,  3
	.equ np_uartcontrol_ibrk_bit,  2
	.equ np_uartcontrol_ife_bit,   1
	.equ np_uartcontrol_ipe_bit,   0

; ----------------------------------------------
; Timer Peripheral
;
	;
	; Timer Register Offsets
	;

	.equ np_timerstatus,  0  ; read only, 2 bits (any write to clear to)
	.equ np_timercontrol, 1  ; write/readable, 4 bits
	.equ np_timerperiodl, 2  ; write/readable, 16 bits
	.equ np_timerperiodh, 3  ; write/readable, 16 bits
	.equ np_timersnapl,   4  ; read only, 16 bits
	.equ np_timersnaph,   5  ; read only, 16 bits

	;
	; Timer Register Bits
	;

	.equ np_timerstatus_run_bit,    1 ; timer is running
	.equ np_timerstatus_to_bit,     0 ; timer has timed out
 
	.equ np_timercontrol_stop_bit,  3 ; stop the timer
	.equ np_timercontrol_start_bit, 2 ; start the timer
	.equ np_timercontrol_cont_bit,  1 ; continous mode
	.equ np_timercontrol_ito_bit,   0 ; enable time out interrupt

	.equ np_timerstatus_run_mask,    (1<<1) ; timer is running
	.equ np_timerstatus_tO_mask,     (1<<0) ; timer has timed out
 
	.equ np_timercontrol_stop_mask,  (1<<3) ; stop the timer
	.equ np_timercontrol_start_mask, (1<<2) ; start the timer
	.equ np_timercontrol_cont_mask,  (1<<1) ; continous mode
	.equ np_timercontrol_ito_mask,   (1<<0) ; enable time out interrupt

; ----------------------------------------------
; PIO Peripheral
;
	;
	; PIO Registers
	;
	.equ np_piodata,          0 ; read/write, up to 32 bits
	.equ np_piodirection,     1 ; write/readable, up to 32 bits, 1->output bit
	.equ np_piointerruptmask, 2 ; write/readable, up to 32 bits, 1->enable interrupt
	.equ np_pioedgecapture,   3 ; read, up to 32 bits, cleared by any write.

; ----------------------------------------------
; SPI Peripheral
;
	;
	; SPI Registers
	;
	.equ np_spirxdata,      0 ; Read-only, 1-16 bit
  .equ np_spitxdata,      1 ; Write-only, same width as rxdata
	.equ np_spistatus,      2 ; Read-only, 9-bit
	.equ np_spicontrol,     3 ; Read/Write, 9-bit
	.equ np_spireserved,    4 ; reserved
	.equ np_spislaveselect, 5 ; Read/Write, 1-16 bit, master only
  .equ np_spiendofpacket, 6 ; Read/write, same width as txdata, rxdata.

	;
	; SPI Status Register
	;
  .equ np_spistatus_eop_mask,  (1 << 9)
  .equ np_spistatus_e_mask,    (1 << 8)
	.equ np_spistatus_rrdy_mask, (1 << 7)
	.equ np_spistatus_trdy_mask, (1 << 6)
	.equ np_spistatus_tmt_mask,  (1 << 5)
	.equ np_spistatus_toe_mask,  (1 << 4)
	.equ np_spistatus_roe_mask,  (1 << 3)

  .equ np_spistatus_eop_bit,  9
  .equ np_spistatus_e_bit,    8
	.equ np_spistatus_rrdy_bit, 7
	.equ np_spistatus_trdy_bit, 6
	.equ np_spistatus_tmt_bit,  5
	.equ np_spistatus_toe_bit,  4
	.equ np_spistatus_roe_bit,  3

	;
	; SPI Control Register
	;
  .equ np_spicontrol_ieop_mask,  (1 << 9)
	.equ np_spicontrol_ie_mask,    (1 << 8)
	.equ np_spicontrol_irrdy_mask, (1 << 7)
	.equ np_spicontrol_itrdy_mask, (1 << 6)
	.equ np_spicontrol_itoe_mask,  (1 << 4)
	.equ np_spicontrol_iroe_mask,  (1 << 3)

  .equ np_spicontrol_ieop_bit,  9
	.equ np_spicontrol_ie_bit,    8
	.equ np_spicontrol_irrdy_bit, 7
	.equ np_spicontrol_itrdy_bit, 6
	.equ np_spicontrol_itoe_bit,  4
	.equ np_spicontrol_iroe_bit,  3




.endif ; _excalibur_

; end of file
