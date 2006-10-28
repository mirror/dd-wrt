#include <sys/types.h>

#include "./def.h"

#ifdef WINDOWS
#include <windows.h>
#else
#include <unistd.h>
#include <signal.h>
#endif

#include <sys/time.h>
#include <time.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "btconfig.h"
#include "btcontent.h"
#include "downloader.h"
#include "peerlist.h"
#include "tracker.h"

#include "./config.h"

#ifndef WINDOWS
#include "sigint.h"
#endif

void usage();
int param_check(int argc, char **argv);

#ifdef WINDOWS

int APIENTRY WinMain(HINSTANCE hInstance,
                     HINSTANCE hPrzevInstance,
                     LPSTR     lpCmdLine,
                     int       nCmdShow)
{
}

#else

void Random_init()
{
  struct timeval tv; 
  gettimeofday(&tv,(struct timezone*) 0);
  unsigned int seed = tv.tv_usec + tv.tv_sec + getpid();
  return srandom(seed);
}

int main(int argc, char **argv)
{

  Random_init();
  arg_user_agent = new char[MAX_PF_LEN];
  strcpy(arg_user_agent,PEER_PFX);
    
  if( argc < 2 || param_check(argc,argv) < 0 ){
    usage();
    exit(1);
  }

  if( arg_flg_make_torrent ){
    if( !arg_announce ){ fprintf(stderr,"please use -u to specify a announce url!\n"); exit(1);}
    if( !arg_save_as ){ fprintf(stderr,"please use -s to specify a metainfo file name!\n"); exit(1);}
    if( BTCONTENT.InitialFromFS(arg_metainfo_file, arg_announce,  arg_piece_length) < 0 ||
	BTCONTENT.CreateMetainfoFile(arg_save_as) < 0){
      fprintf(stderr,"create metainfo failed.\n");
      exit(1);
    }
    printf("create metainfo file %s successful.\n", arg_save_as);
    exit(0);
  }
  
  if( BTCONTENT.InitialFromMI(arg_metainfo_file, arg_save_as) < 0){
    fprintf(stderr,"error,initial meta info failed.\n");
    exit(1);
  }

  if( !arg_flg_exam_only && !arg_flg_check_only){
    if(WORLD.Initial_ListenPort() < 0){
      fprintf(stderr,"warn, you couldn't accept connection.\n");
    }else 
      printf("Listen on: %d\n",cfg_listen_port);

	Tracker.Initial();

	signal(SIGPIPE,SIG_IGN);
    signal(SIGINT,sigint_catch);
    Downloader();
  }

  exit(0);
}

#endif

int param_check(int argc, char **argv)
{
  int c, l;
  while ( ( c = getopt(argc,argv,"b:B:cC:e:fl:M:m:P:p:s:tu:xhH")) != -1)
    switch( c ){
    case 'b':
      arg_bitfield_file = new char[strlen(optarg) + 1];
#ifndef WINDOWS
      if( !arg_bitfield_file ) return -1;
#endif
      strcpy(arg_bitfield_file, optarg);
      break;

    case 'p':			// listen on Port XXXX
      cfg_listen_port = atoi(optarg);
      break;

    case 's':			// Save as FILE/DIR NAME
      if( arg_save_as ) return -1;
      arg_save_as = new char[strlen(optarg) + 1];
#ifndef WINDOWS
      if( !arg_save_as ) return -1;
#endif
      strcpy(arg_save_as,optarg);
      break;

    case 'e':			// Exit while complete
      cfg_seed_hours = atoi(optarg);
      break;

    case 'c':			// Check exist only
      arg_flg_check_only = 1;
      break;

    case 'C':
      cfg_cache_size = atoi(optarg);
      break;
      
    case 'M':			// Max peers
      cfg_max_peers = atoi(optarg);
      if( cfg_max_peers > 1000 ||
	  cfg_max_peers < 20){
	return -1;
      }
      break;
      
    case 'm':			// Min peers
      cfg_min_peers = atoi(optarg);
      if( cfg_min_peers > 1000 ||
	  cfg_min_peers < 20){
	return -1;
      }
      break;

    case 'f':			// force seed mode, skip sha1 check when startup.
      arg_flg_force_seed_mode = 1;
      break;
      
    case 'B':
      cfg_max_bandwidth = atoi(optarg);
      break;

    case 'P':
		l = strlen(optarg);
		if (l > MAX_PF_LEN) {printf("-P arg must be 8 or less characters\n"); exit(1);}
		if (l == 1 && *optarg == '-') *arg_user_agent = (char) 0;
		else strcpy(arg_user_agent,optarg);
      break;

     // BELLOW OPTIONS USED FOR CREATE TORRENT.
    case 'u':			// Announce url
      if( arg_announce ) return -1;
      arg_announce = new char[strlen(optarg) + 1];
      strcpy(arg_announce, optarg);
      break;

    case 't':			// make Torrent
      arg_flg_make_torrent = 1;
      break;

    case 'l':			// piece Length (default 262144)
      arg_piece_length = atoi(optarg);
      if( arg_piece_length < 65536 ||
	  arg_piece_length > 1310720 ){
	// warn message:
	// piece length range is 65536 =>> 1310720
	return -1;
      }
      break;

    case 'x':
      arg_flg_exam_only = 1;
      break;

    case 'h':
    case 'H':
    default:
      //unknow option.
      return -1;
    }

  argc -= optind; argv += optind;
  if( cfg_min_peers >= cfg_max_peers ) cfg_min_peers = cfg_max_peers - 1;
  if( argc != 1 ) return -1;
  arg_metainfo_file = new char[strlen(*argv) + 1];
  
#ifndef WINDOWS
  if( !arg_metainfo_file ) return -1;
#endif
  strcpy(arg_metainfo_file, *argv);
  return 0;
}

void usage()
{
  fprintf(stderr,"%s		Copyright: YuHong(992126018601033)",PACKAGE_STRING);
  fprintf(stderr,"\nWARNING: THERE IS NO WARRANTY FOR CTorrent. USE AT YOUR OWN RISK!!!\n");
  fprintf(stderr,"\nGeneric Options:\n");
  fprintf(stderr,"-h/-H\t\tShow this message.\n");
  fprintf(stderr,"-x\t\tDecode metainfo(torrent) file only, don't download.\n");
  fprintf(stderr,"-c\t\tCheck exist only. don't download.\n");
  fprintf(stderr,"\nDownload Options:\n");
  fprintf(stderr,"-e int\t\tExit while seed <int> hours later. (default 72 hours)\n");
  fprintf(stderr,"-p port\t\tListen port. (default 2706 -> 2106)\n");
  fprintf(stderr,"-s save_as\tSave file/directory/metainfo as... \n");
  fprintf(stderr,"-C cache_size\tCache size,unit MB. (default 16MB)\n");
  fprintf(stderr,"-f\t\tForce seed mode. skip hash check at startup.\n");
  fprintf(stderr,"-b bf_filename\tBit field filename. (use it carefully)\n");
  fprintf(stderr,"-M max_peers\tMax peers count.\n");
  fprintf(stderr,"-m min_peers\tMin peers count.\n");
  fprintf(stderr,"-B rate\t\tMax bandwidth (unit KB/s)\n");
  fprintf(stderr,"-P peer_id\tSet Peer ID ["PEER_PFX"]\n");
  fprintf(stderr,"\nMake metainfo(torrent) file Options:\n");
  fprintf(stderr,"-t\t\tWith make torrent. must specify this option.\n");
  fprintf(stderr,"-u url\t\tTracker's url.\n");
  fprintf(stderr,"-l piece_len\tPiece length.(default 262144)\n");
  fprintf(stderr,"\neg.\n");
  fprintf(stderr,"hong> ctorrent -s new_filename -e 12 -C 32 -p 6881 eg.torrent\n\n");
  fprintf(stderr,"home page: http://ctorrent.sourceforge.net/\n");
  fprintf(stderr,"bug report: %s\n\n",PACKAGE_BUGREPORT);
}
