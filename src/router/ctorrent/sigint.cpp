#ifndef WINDOWS

#include <sys/types.h>
#include <signal.h>

#include "btcontent.h"
#include "peerlist.h"
#include "btconfig.h"

void sigint_catch(int sig_no)
{
  if(SIGINT == sig_no){
    if( cfg_cache_size ) BTCONTENT.FlushCache();
    if( arg_bitfield_file ) BTCONTENT.pBF->WriteToFile(arg_bitfield_file);
    WORLD.CloseAll();
    signal(SIGINT,SIG_DFL);
    raise(SIGINT);
  }
}

#endif
