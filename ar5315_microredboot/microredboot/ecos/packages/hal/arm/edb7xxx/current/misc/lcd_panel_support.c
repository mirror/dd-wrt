//==========================================================================
//
//        panel_support.c
//
//        Cirrus Logic EDB7xxx eval board LCD touch panel support code
//
//==========================================================================
//####ECOSGPLCOPYRIGHTBEGIN####
// -------------------------------------------
// This file is part of eCos, the Embedded Configurable Operating System.
// Copyright (C) 1998, 1999, 2000, 2001, 2002 Red Hat, Inc.
//
// eCos is free software; you can redistribute it and/or modify it under
// the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 or (at your option) any later version.
//
// eCos is distributed in the hope that it will be useful, but WITHOUT ANY
// WARRANTY; without even the implied warranty of MERCHANTABILITY or
// FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
// for more details.
//
// You should have received a copy of the GNU General Public License along
// with eCos; if not, write to the Free Software Foundation, Inc.,
// 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA.
//
// As a special exception, if other files instantiate templates or use macros
// or inline functions from this file, or you compile this file and link it
// with other works to produce a work based on this file, this file does not
// by itself cause the resulting work to be covered by the GNU General Public
// License. However the source code for this file must still be made available
// in accordance with section (3) of the GNU General Public License.
//
// This exception does not invalidate any other reasons why a work based on
// this file might be covered by the GNU General Public License.
//
// Alternative licenses for eCos may be arranged by contacting Red Hat, Inc.
// at http://sources.redhat.com/ecos/ecos-license/
// -------------------------------------------
//####ECOSGPLCOPYRIGHTEND####
//==========================================================================
//#####DESCRIPTIONBEGIN####
//
// Author(s):     gthomas
// Contributors:  gthomas
// Date:          1999-09-13
// Description:   Tool used to support the LCD touch panel
//####DESCRIPTIONEND####

static char lcd_panel_server_stack[STACK_SIZE];
static cyg_thread lcd_panel_server_thread_data;
static cyg_handle_t lcd_panel_server_thread_handle;

static cyg_interrupt lcd_panel_interrupt;
static cyg_handle_t  lcd_panel_interrupt_handle;

static cyg_mbox      lcd_panel_events_mbox;
static cyg_handle_t  lcd_panel_events_mbox_handle;
static cyg_sem_t     lcd_panel_sem;

#define SYNCIO_TXFRMEN     (1<<14)
#define SYNCIO_FRAMELEN(n) (n<<8)
#define ADC_START          (1<<7)
#define ADC_CHAN(n)        (n<<4)
#define ADC_UNIPOLAR       (1<<3)
#define ADC_SINGLE         (1<<2)
#define ADC_EXT_CLOCK      (3<<0)
#define TOUCH_CTL          KBD_PORT

// FUNCTIONS

static cyg_uint32
adc_sample(int chan)
{
    cyg_uint32 val;
    *(volatile cyg_uint32 *)SYNCIO = SYNCIO_TXFRMEN | SYNCIO_FRAMELEN(24) |
        ADC_START | ADC_CHAN(chan) | ADC_UNIPOLAR | ADC_SINGLE | ADC_EXT_CLOCK;
    while (*(volatile cyg_uint32 *)SYSFLG1 & SYSFLG1_SSIBUSY) ;
    val = *(volatile cyg_uint32 *)SYNCIO;
    return (val & 0xFFFF);
}

static void
panel_delay(void)
{
    volatile int i;
    for (i = 0;  i < 800;  i++) ;
}

// This ISR is called when the touch panel interrupt occurs
static int 
lcd_panel_isr(cyg_vector_t vector, cyg_addrword_t data, HAL_SavedRegisters *regs)
{
    cyg_drv_interrupt_mask(CYGNUM_HAL_INTERRUPT_EINT2);
    return (CYG_ISR_HANDLED|CYG_ISR_CALL_DSR);  // Run the DSR
}

// This DSR starts up the touch panel [logical] processing
static void
lcd_panel_dsr(cyg_vector_t vector, cyg_ucount32 count, cyg_addrword_t data)
{
    // Tell the panel processing thread to give it a shot
    cyg_semaphore_post(&lcd_panel_sem);
}

// Assumption: if the keypress vanishes for 5*20 ms, it probably isn't there
#define LCD_PANEL_TIMEOUT 5

static __inline__ int
abs(int x)
{
    if (x < 0) return -x;
    return x;
}

static void
lcd_panel_server(cyg_addrword_t p)
{
    int iX, iY, newX, newY, diffX, diffY, timeout, samples;
    cyg_uint32 event;
    diag_printf("LCD panel server here\n");
    while (TRUE) {
        cyg_semaphore_wait(&lcd_panel_sem);
        samples = 0;  iX = 0;  iY = 0;
        // Wait for press to go away (no drag support)
        timeout = 0;
        while (timeout < LCD_PANEL_TIMEOUT) {
            *(volatile cyg_uint8 *)TOUCH_CTL = 0x00;  // Disable drives
            while (*(volatile cyg_uint32 *)INTSR1 & INTSR1_EINT2) ;
            *(volatile cyg_uint8 *)TOUCH_CTL = 0x70;  // Idle state (so interrupt works)
            cyg_thread_delay(2);  // Wait 20 ms
            if (*(volatile cyg_uint32 *)INTSR1 & INTSR1_EINT2) {
                // Still pressed
                // Drive TSPY, ground TSMY, and disconnect TSPX and TSMX
                *(volatile cyg_uint8 *)TOUCH_CTL = 0x50;  
                panel_delay();
                newY = adc_sample(2);
                // Drive TSPX, ground TSMX, and disconnect TSPY and TSMY
                *(volatile cyg_uint8 *)TOUCH_CTL = 0xA0;  
                panel_delay();
                newX = adc_sample(7);
#if 0
                diag_printf("timeout: %d, ISR: %x, newX: %d, newY: %d\n", 
                            timeout, *(volatile cyg_uint32 *)INTSR1, newX, newY);
#endif
                // See if this sample makes any sense
                if (samples) {
                    diffX = abs(iX/samples - newX);
                    diffY = abs(iY/samples - newY);
                    if ((diffX <= ((iX/samples)/4)) &&
                        (diffY <= ((iY/samples)/4))) {
                        samples++;
                        iX += newX;
                        iY += newY;
                    } else {
#if 0
                        diag_printf("Discard - newX: %d, X: %d, newY: %d, Y: %d\n",
                                    newX, iX/samples, newY, iY/samples);
#endif
                        break;
                    }
                } else {
                    iX = newX;
                    iY = newY;
                    samples = 1;
                }
                timeout = 0;
            } else {
                timeout++;
            }
        }
        if (samples) {
            // Send event to user level
            event = (iX/samples)<<16 | (iY/samples);
            if (!cyg_mbox_tryput(lcd_panel_events_mbox_handle, (void *)event)) {
                diag_printf("LCD event lost: %x\n", event);
            }
        }
        *(volatile cyg_uint8 *)TOUCH_CTL = 0x00;  // Disable drives
        while (*(volatile cyg_uint32 *)INTSR1 & INTSR1_EINT2) ;
        *(volatile cyg_uint8 *)TOUCH_CTL = 0x70;  // Idle state (so interrupt works)
        cyg_thread_delay(10);
        cyg_drv_interrupt_unmask(CYGNUM_HAL_INTERRUPT_EINT2);
    }
}

static void
lcd_panel_init(void)
{
    // Enable touch panel
    *(volatile cyg_uint8 *)PEDR   |= 0x04;  

    // Idle state (so interrupt works)
    *(volatile cyg_uint8 *)TOUCH_CTL = 0x70;  

    // Enable ADC machinery
    *(volatile cyg_uint32 *)SYSCON1 |= SYSCON1_ADC_CLOCK_128kHZ;

    cyg_drv_interrupt_create(CYGNUM_HAL_INTERRUPT_EINT2,
                             99,                     // Priority - what goes here?
                             0,                      //  Data item passed to interrupt handler
                             lcd_panel_isr,
                             lcd_panel_dsr,
                             &lcd_panel_interrupt_handle,
                             &lcd_panel_interrupt);
    cyg_drv_interrupt_attach(lcd_panel_interrupt_handle);
    cyg_drv_interrupt_acknowledge(CYGNUM_HAL_INTERRUPT_EINT2);
    cyg_drv_interrupt_unmask(CYGNUM_HAL_INTERRUPT_EINT2);
    // Set up the mbox for panel data
    cyg_mbox_create(&lcd_panel_events_mbox_handle, &lcd_panel_events_mbox);
    // This semaphore is set when there is a touch
    cyg_semaphore_init(&lcd_panel_sem, 0);  
    // Create a thread whose job it is to de-bounce the keyboard and
    // actually process the input, turning it into a series of events
    cyg_thread_create(10,                           // Priority - just a number
                      lcd_panel_server,             // entry
                      0,                            // initial parameter
                      "LCD_PANEL_server",           // Name
                      &lcd_panel_server_stack[0],   // Stack
                      STACK_SIZE,                   // Size
                      &lcd_panel_server_thread_handle,    // Handle
                      &lcd_panel_server_thread_data       // Thread data structure
            );
    cyg_thread_resume(lcd_panel_server_thread_handle);  // Start it
}
