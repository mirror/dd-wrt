/* atmsigd.c - ATM signaling demon */

/* Written 1995-2000 by Werner Almesberger, EPFL-LRC/ICA */

#if HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <signal.h>
#include <limits.h>
#include <sys/types.h>

#include "atm.h"
#include "atmd.h"
#include "qlib.h"

#include "io.h"
#include "proto.h"
#include "saal.h"
#include "trace.h"


#define COMPONENT "SIGD"


extern int yyparse(void);
extern FILE *yyin;

int debug = 0;
int pretty = A2T_PRETTY | A2T_NAME | A2T_LOCAL;
const char *dump_dir = DEFAULT_DUMP_DIR;


/* A little hack until we have full support for multiple signaling entities */

SIG_ENTITY _entity = {
	0,		/* fd */
	0,		/* unspecified UNI version */
	sm_user,	/* mode */
	-1,		/* sig_pcr; obsolete @@@ */
	NULL,		/* sig_qos */
	-1		/* max_rate */
};


/* ------------------------------ SAAL relays ------------------------------ */


static void q_estab_conf(void *user_data,void *uu_data,int uu_length)
{
    SIG_ENTITY *sig = user_data;

    saal_okay(sig);
}


static void q_rel_ind(void *user_data,void *uu_data,int uu_length)
{
    SIG_ENTITY *sig = user_data;

    saal_failure(sig);
    saal_estab_req(&sig->saal,NULL,0);
}


static void q_restart(void *user_data,void *uu_data,int uu_length,int ind)
{
    SIG_ENTITY *sig = user_data;

    saal_failure(sig);
    if (!ind) saal_okay(sig);
	/* actually, ind should probably never be zero */
}


void from_net(SIG_ENTITY *sig,void *buffer,int size)
{
    saal_pdu(&sig->saal,buffer,size);
}


void to_signaling(SIG_ENTITY *sig,void *msg,int size)
{
    trace_uni("TO NETWORK",sig,msg,size);
    diag(COMPONENT,DIAG_DEBUG,"TO SAAL (%d.%d.%d): %s (0x%02x) CR 0x%06x "
      "(%d bytes)",S_PVC(sig),
      mid2name(((unsigned char *) msg)[5]),((unsigned char *) msg)[5],
      (((unsigned char *) msg)[2] << 16) | (((unsigned char *) msg)[3] << 8) |
      ((unsigned char *) msg)[4],size);
    saal_send(&sig->saal,msg,size);
}


static void q_data_ind(void *user_data,void *data,int length)
{
    SIG_ENTITY *sig = user_data;

    trace_uni("FROM NETWORK",sig,data,length);
    to_uni(sig,data,length);
}


static void q_cpcs_send(void *user_data,void *data,int length)
{
    SIG_ENTITY *sig = user_data;

    to_net(sig,data,length);
}


static SAAL_USER_OPS ops = {
    NULL, /* no q_estab_ind - 5.5.6.9 says 5.5.6.11 and 5.5.6.11 says "may" */
    q_estab_conf,
    q_rel_ind,
    NULL, /* no q_rel_conf - what to do ? */
    q_restart,
    q_data_ind,
    NULL, /* no q_unitdata */
    q_cpcs_send
};


/* -------------------------------- signals -------------------------------- */


static volatile int got_usr1 = 0,got_usr2 = 0;


static void dump_addr(FILE *file,const char *label,struct sockaddr_atmsvc *addr)
{
    char buffer[MAX_ATM_ADDR_LEN+1];
    int i;

    if (!atmsvc_addr_in_use(*addr)) return;
    fprintf(file,"  %s ",label);
    if (atm2text(buffer,MAX_ATM_ADDR_LEN+1,(struct sockaddr *) addr,A2T_NAME |
      A2T_PRETTY | A2T_LOCAL) >= 0) fprintf(file,"%s\n",buffer);
    else {
	fprintf(file,"<invalid:");
	for (i = 0; i < sizeof(*addr); i++)
	    fprintf(file," %02X",((unsigned char *) addr)[i]);
	fprintf(file,">\n");
    }
}


static void dump_sap(FILE *file,const char *label,struct atm_sap *sap)
{
    char buffer[MAX_ATM_SAP_LEN+1];
    int i;

    fprintf(file,"  %s ",label);
    if (sap2text(buffer,MAX_ATM_SAP_LEN+1,sap,S2T_NAME | S2T_LOCAL) >= 0)
	fprintf(file,"%s\n",buffer);
    else {
	fprintf(file,"<invalid:");
	for (i = 0; i < sizeof(*sap); i++)
	    fprintf(file," %02X",((unsigned char *) sap)[i]);
	fprintf(file,">\n");
    }
}


static void dump_status(FILE *file,const char *banner)
{
    SIG_ENTITY *sig;
    SOCKET *walk;

    if (entities) fprintf(file,"%s\n\n",banner);
    for (sig = entities; sig; sig = sig->next) {
	fprintf(file,"--- Entity %d.%d.%d ---\n",S_PVC(sig));
	for (walk = sockets; walk; walk = walk->next) {
	    fprintf(file,"%s: %s, CR 0x%06lX, PVC %d.%d.%d\n",
	      kptr_print(&walk->id),
	      state_name[walk->state],walk->call_ref,walk->pvc.sap_addr.itf,
	      walk->pvc.sap_addr.vpi,walk->pvc.sap_addr.vci);
	    dump_addr(file,"local ",&walk->local);
	    dump_addr(file,"remote",&walk->remote);
	    dump_sap(file,"sap",&walk->sap);
	}
    }
}


static void dump_trace(FILE *file,const char *banner)
{
    static int busy = 0;
    char *trace;

    if (busy++) abort();
    trace = get_trace();
    if (trace) {
	fprintf(file,"%s\n\n",banner);
	fprintf(file,"%s",trace);
    }
    busy--;
}


void poll_signals(void)
{
    static int status_num = 0,trace_num = 0;
    char path[PATH_MAX+1];
    FILE *file;

    if (got_usr1) {
	got_usr1 = 0;
	if (!dump_dir) file = stderr;
	else {
	    sprintf(path,"atmsigd.%d.status.%d",getpid(),status_num++);
	    if ((file = fopen(path,"w")))
		diag(COMPONENT,DIAG_INFO,"Dumping to %s",path);
	    else {
		perror(path);
		file = stderr;
	    }
	}
	dump_status(file,"Status dump (on SIGUSR1)");
	if (file != stderr) (void) fclose(file);
    }
    if (got_usr2) {
	pid_t pid;

	got_usr2 = 0;
	if (!dump_dir) file = stderr;
	else {
	    sprintf(path,"atmsigd.%d.trace.%d",getpid(),trace_num++);
	    if ((file = fopen(path,"w")))
		diag(COMPONENT,DIAG_INFO,"Dumping to %s",path);
	    else {
		perror(path);
		file = stderr;
	    }
	}
	if (!(pid = fork()))
	     dump_trace(file,"Message trace (on SIGUSR2)");
	else if (pid < 0) perror("fork");
	if (file != stderr) (void) fclose(file);
	if (!pid) exit(0);
    }
}


static void handle_signal(int sig)
{
    switch (sig) {
	case SIGUSR1:
	    got_usr1 = 1;
	    break;
	case SIGUSR2:
	    got_usr2 = 1;
	    break;
	default:
	    break;
    }
}


static void setup_signals(void)
{
    struct sigaction act;

    (void) signal(SIGCHLD,SIG_IGN); /* reap children automatially */
    act.sa_handler = handle_signal;
    sigemptyset(&act.sa_mask);
    act.sa_flags = 0;
    if (sigaction(SIGUSR1,&act,NULL) < 0) {
	perror("sigaction");
	exit(1);
    }
    if (sigaction(SIGUSR2,&act,NULL) < 0) {
	perror("sigaction");
	exit(1);
    }
}


/* ------------------------------- main ...  ------------------------------- */


static void trace_on_exit(int status,void *dummy)
{
    char path[PATH_MAX+1];
    FILE *file;

    if (!status) return;
    if (!dump_dir) file = stderr;
    else {
	sprintf(path,"atmsigd.%d.trace.exit",getpid());
	if (!(file = fopen(path,"w"))) {
	    perror(path);
	    file = stderr;
	}
    }
    dump_trace(file,"Message trace (after error exit)");
    if (file != stderr) (void) fclose(file);
}


static void manual_override(void)
{
     /*
      * Gross hack to avoid changing the command-line parameters ... @@@
      */
     entities = &_entity;
     _entity.next = NULL;
}


static void usage(const char *name)
{
    fprintf(stderr,"usage: %s [ -b ] [ -c config_file ] [ -d ] "
      "[ -D dump_dir ]\n"
      "  [ -l logfile ] [ -n ] [ -m user|network|switch ]  [ -q qos ]\n"
      "  [ -t trace_length ] [ -u uni_version ] [ [itf.]vpi.vci "
      "[ socket_path ] ]\n",name);
    fprintf(stderr,"%6s %s -V\n","",name);
    exit(1);
}


int main(int argc,char **argv)
{
    SIG_ENTITY *sig;
    const char *config_file;
    char *end;
    int c,background;
    int net = 0,allocate_ci = 1;

    set_application("atmsigd");
    config_file = ATMSIGD_CONF;
    background = 0;
    memset(&_entity.signaling_pvc,0,sizeof(_entity.signaling_pvc));
    /* 1st pass to get the -c option */
    while ((c = getopt(argc,argv,"Abc:dD:l:m:nNP:q:t:u:V")) != EOF)
	if (c == 'c') config_file = optarg;
	else if (c == 'V') {
		printf("%s\n",VERSION);
		return 0;
	    }
    if (!(yyin = fopen(config_file,"r")))
	diag(COMPONENT,DIAG_WARN,"%s not found. - Using defaults.",config_file);
    else if (yyparse())
	    diag(COMPONENT,DIAG_FATAL,"Error in config file. - Aborting.");
    if (!atmpvc_addr_in_use(_entity.signaling_pvc))
	_entity.signaling_pvc.sap_addr.vci = 5;
    if (!_entity.uni)
	_entity.uni =
#ifdef UNI31
	    S_UNI31
#ifdef ALLOW_UNI30
	    | S_UNI30
#endif
#elif defined(UNI40)
	    S_UNI40
#ifdef Q2963_1
	    | S_Q2963_1
#endif
#else
	    S_UNI30
#endif
	    ;
    /* process all other options but -c */
    optind = 0;
    while ((c = getopt(argc,argv,"Abc:dD:l:m:nNP:q:t:u:")) != EOF)
	switch (c) {
	    case 'A':
		manual_override();
		allocate_ci = 0;
		break;
	    case 'b':
		background = 1;
		break;
	    case 'c':
		/* already handled */
		break;
	    case 'd':
		set_verbosity(NULL,DIAG_DEBUG);
		set_verbosity("QMSG",DIAG_INFO);
		set_verbosity("SSCOP",DIAG_INFO);
		debug = 1;
		/*q_dump = 1;*/
		break;
	    case 'D':
		dump_dir = optarg;
		break;
	    case 'l':
		set_logfile(optarg);
		break;
	    case 'm':
		manual_override();
		if (!strcmp(optarg,"user")) _entity.mode = sm_user;
		else if (!strcmp(optarg,"network")) _entity.mode = sm_net;
		else if (!strcmp(optarg,"switch")) _entity.mode = sm_switch;
		else usage(argv[0]);
		break;
	    case 'n':
		pretty = A2T_PRETTY;
		break;
	    case 'N':
		manual_override();
		net = 1;
		break;
	    case 'q':
		manual_override();
		if (_entity.sig_pcr != -1) usage(argv[0]);
		_entity.sig_qos = optarg;
		break;
	    case 'P': /* obsolete */
		manual_override();
		if (_entity.sig_qos) usage(argv[0]);
		_entity.sig_pcr = strtol(optarg,&end,0);
		if (*end) usage(argv[0]);
		diag(COMPONENT,DIAG_WARN,"option -P is obsolete, "
		  "please use  -q qos  instead");
		break;
	    case 't':
		trace_size = strtol(optarg,&end,0);
		if (*end) usage(argv[0]);
		break;
	    case 'u':
		manual_override();
		{
		    int uni = 0; /* silence gcc */

		    if (!strcmp(optarg,"uni30") || !strcmp(optarg,"3.0")) uni = S_UNI30;
		    else if (!strcmp(optarg,"uni31") || !strcmp(optarg,"3.1")) uni = S_UNI31;
		    else if (!strcmp(optarg,"uni31+uni30") || !strcmp(optarg,"3.1+3.0"))
			uni = S_UNI30 | S_UNI31;
		    else if (!strcmp(optarg,"uni40") || !strcmp(optarg,"4.0")) uni = S_UNI40;
		    else if (!strcmp(optarg,"uni40+q.2963.1") || !strcmp(optarg,"4.0+q.2963.1"))
			uni = S_UNI40 | S_Q2963_1;
		    else usage(argv[0]);
#ifdef DYNAMIC_UNI
		    _entity.uni = uni;
#else
	    	    diag(COMPONENT,DIAG_FATAL,"Signaling version specified "
		      "with -s conflicts with compiled-in default.");
#endif
		}
		break;
	    default:
		usage(argv[0]);
	}
    if (_entity.mode == sm_unknown) {
	if (net) {
	    if (allocate_ci) {
		_entity.mode = sm_net;
		diag(COMPONENT,DIAG_WARN,"option -N is obsolete, "
		  "please use  -m network  instead");
	    }
	    else {
		_entity.mode = sm_switch;
		diag(COMPONENT,DIAG_WARN,"options -N -A are obsolete, "
		  "please use  -m switch  instead");
	    }
	}
	else if (allocate_ci) _entity.mode = sm_user;
	    else usage(argv[0]);
    }
    if (optind < argc) {
	manual_override();
	if (text2atm(argv[optind],(struct sockaddr *) &_entity.signaling_pvc,
	  sizeof(_entity.signaling_pvc),T2A_PVC) < 0)
	    diag(COMPONENT,DIAG_FATAL,"text2atm \"%s\": failed",argv[optind]);
	optind++;
	if (optind == argc-1) {
	    open_unix(argv[optind]);
	    optind++;
	}
    }
    if (optind != argc) usage(argv[0]);
    if (!trace_size) dump_dir = NULL;
    if (dump_dir)
	if (chdir(dump_dir) < 0)
	    diag(COMPONENT,DIAG_ERROR,"chdir %s: %s",dump_dir,strerror(errno));
    for (sig = entities; sig; sig = sig->next) {
	diag(COMPONENT,DIAG_INFO,"Linux ATM signaling %s"
#ifdef DYNAMIC_UNI
	  " (dynamic)"
#endif
	  ", version " VERSION " on %d.%d.%d",
	  sig->uni & S_UNI30 ?
	    sig->uni & S_UNI31 ? "UNI 3.1+3.0compat" : "UNI 3.0" :
	    sig->uni & S_UNI31 ? "UNI 3.1" :
	      sig->uni & S_Q2963_1 ? "UNI 4.0+Q.2963.1" : "UNI 4.0",
          S_PVC(sig));
	diag(COMPONENT,DIAG_INFO,"Acting as %s",
	  sig->mode == sm_user ? "USER side" :
	  sig->mode == sm_net ? "NETWORK side" : "SWITCH");
    }
    if (open_all()) return 1;
    init_current_time();
    q_start();
    for (sig = entities; sig; sig = sig->next) {
	set_vpi_0(sig);
	if (sig->max_rate < 0) sig->max_rate = get_max_rate(sig);
	if (sig->mode != sm_switch) init_addr(sig);
	start_saal(&sig->saal,&ops,sig,
	  sig->uni == S_UNI30 ? sscop_qsaal1 : sscop_q2110);
	saal_estab_req(&sig->saal,NULL,0);
    }
    setup_signals();
    if (background) {
	pid_t pid;

	pid = fork();
	if (pid < 0)
	    diag(COMPONENT,DIAG_FATAL,"fork: %s",strerror(errno));
	if (pid) {
	    diag(COMPONENT,DIAG_DEBUG,"Backgrounding (PID %d)",pid);
	    exit(0);
	}
    }
    (void) on_exit(trace_on_exit,NULL);
    poll_loop();
    close_all();
    for (sig = entities; sig; sig = sig->next) stop_saal(&sig->saal);
    return 0;
}
