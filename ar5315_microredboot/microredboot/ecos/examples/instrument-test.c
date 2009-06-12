/* this is a program which uses eCos instrumentation buffers; it needs
   to be linked with a kernel which was compiled with support for
   instrumentation */

#include <stdio.h>
#include <pkgconf/kernel.h>
#include <cyg/kernel/instrmnt.h>
#include <cyg/kernel/kapi.h>

#ifndef CYGVAR_KERNEL_INSTRUMENT_EXTERNAL_BUFFER
# error You must configure eCos with CYGVAR_KERNEL_INSTRUMENT_EXTERNAL_BUFFER
#endif

struct Instrument_Record
{
    CYG_WORD16  type;                   // record type
    CYG_WORD16  thread;                 // current thread id
    CYG_WORD    timestamp;              // 32 bit timestamp
    CYG_WORD    arg1;                   // first arg
    CYG_WORD    arg2;                   // second arg
};

struct Instrument_Record instrument_buffer[20];
cyg_uint32        instrument_buffer_size = 20;

int main(void)
{
  int i;

  cyg_instrument_enable(CYG_INSTRUMENT_CLASS_CLOCK, 0);
  cyg_instrument_enable(CYG_INSTRUMENT_CLASS_THREAD, 0);
  cyg_instrument_enable(CYG_INSTRUMENT_CLASS_ALARM, 0);

  printf("Program to play with instrumentation buffer\n");

  cyg_thread_delay(2);

  cyg_instrument_disable(CYG_INSTRUMENT_CLASS_CLOCK, 0);
  cyg_instrument_disable(CYG_INSTRUMENT_CLASS_THREAD, 0);
  cyg_instrument_disable(CYG_INSTRUMENT_CLASS_ALARM, 0);

  for (i = 0; i < instrument_buffer_size; ++i) {
    printf("Record %02d: type 0x%04x, thread %d, ",
	   i, instrument_buffer[i].type, instrument_buffer[i].thread);
    printf("time %5d, arg1 0x%08x, arg2 0x%08x\n",
	   instrument_buffer[i].timestamp, instrument_buffer[i].arg1,
	   instrument_buffer[i].arg2);
  }
  return 0;
}
