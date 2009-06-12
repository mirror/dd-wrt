/* asmmacro.h: Assembler macros.
 *
 * Copyright (C) 2001 Microtronix Datacom Ltd.
 *
 * Based on Altera's Excalibur sdk "NM_Macros.s"
 *
 */

#ifndef _NIOS_ASMMACRO_H
#define _NIOS_ASMMACRO_H

;----------------------------
;
; Macros I: Load and store instructions
;           who index using byte offset values they
;	    use the general LD & ST with a PFX if the
;	    the offset is none zero and checks the length
;	    /4 will fit in 11 bits.
;
; LDBO dst,src,val
; STBO dst,val,src
;
;----------------------------

.macro LDBO dst,src,val
.if (\val & 0x3)
.print "Offset not word aligned"
.err
.endif
.if (\val/4 > 0x7FF)
.print "Offset to great for K register"
.else
.if (\val/4 > 0)
	pfx	\val/4
.endif
	ld	\dst,[\src]
.endif
.endm

.macro STBO dst,val,src
.if (\val & 0x3)
.print "Offset not word aligned"
.err
.endif
.if (\val/4 > 0x7FF)
.print "Offset to great for K register"
.err
.else
.if (\val/4 > 0)
	pfx	\val/4
.endif
	st	[\dst],\src
.endif
.endm

#endif /* !(_NIOS_ASMMACRO_H) */
