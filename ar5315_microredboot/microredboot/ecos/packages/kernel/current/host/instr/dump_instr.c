
#include <pkgconf/kernel.h>
#include <cyg/kernel/ktypes.h>         // base kernel types
#include <cyg/kernel/instrmnt.h>

#include <stdio.h>
#include <stdlib.h>

// -------------------------------------------------------------------------
// Instrumentation record.

struct Instrument_Record
{
    CYG_WORD16  type;                   // record type
    CYG_WORD16  thread;                 // current thread id
    CYG_WORD    timestamp;              // 32 bit timestamp
    CYG_WORD    arg1;                   // first arg
    CYG_WORD    arg2;                   // second arg
};

// -------------------------------------------------------------------------

#ifdef CYGDBG_KERNEL_INSTRUMENT_MSGS
#define CYGDBG_KERNEL_INSTRUMENT_MSGS_DEFINE_TABLE
#include <cyg/kernel/instrument_desc.h>
#define NELEM(x) (sizeof(x)/sizeof*(x))
externC char * cyg_instrument_msg(CYG_WORD16 type) {

  struct instrument_desc_s *record;
  struct instrument_desc_s *end_record;
  CYG_WORD cl, event;

  record = instrument_desc;
  end_record = &instrument_desc[NELEM(instrument_desc)-1];
  cl = type & 0xff00;
  event = type & 0x00ff;

  while ((record != end_record) && (record->num != cl)) {
    record++;
  }

  if (record->num == cl) {
    record++;
    while ((record != end_record) && (record->num != event) &&
           (record->num < 0xff)) {
      record++;
    }

    if (record->num == event) {
      return (record->msg);
    }
  }
  return("Unknown event");
}
#endif // CYGDBG_KERNEL_INSTRUMENT_MSGS

void usage(char *myname) 
{
  fprintf(stderr,"Usage: %s <filename>\n",myname);
  fprintf(stderr,"where filename is that of the instrumentation data");
}

int main(int argc, char * argv[]) 
{
  
  FILE * file;
  char * filename;
  struct Instrument_Record record;
  int cnt=0;

  if (argc != 2) {
    usage(argv[0]);
    exit(1);
  }

  filename = argv[1];

  file = fopen(filename, "r");
  if (!file) {
    fprintf(stderr,"Error opening file %s: ",filename);
    perror("");
    exit(1);
  }
  
  while (!feof(file)) {
    fread(&record,sizeof(record),1,file);
    if (record.type == 0) {
      break;
    }

#ifdef CYGDBG_KERNEL_INSTRUMENT_MSGS 
    printf("%4d Record type (0x%04x): %-20s thread %2d, ",
           cnt++,record.type,cyg_instrument_msg(record.type), 
           record.thread);
#else
    printf("%4d Record type 0x%04x, thread %2d, ",
           cnt++,record.type, record.thread);
#endif
    printf("time %5d, arg1 0x%08x, arg2 0x%08x\n",
           record.timestamp, record.arg1,
           record.arg2);
  }

  fclose(file);
  return (0);
}
