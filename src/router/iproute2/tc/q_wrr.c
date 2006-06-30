#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <syslog.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <math.h>

#include "utils.h"
#include "tc_util.h"

#define usage() return(-1)

// Returns -1 on error
static int wrr_parse_qdisc_weight(int argc, char** argv, 
                              struct tc_wrr_qdisc_modf* opt) {
  int i;
  
  opt->weight1.weight_mode=-1;
  opt->weight2.weight_mode=-1;
  
  for(i=0; i<argc; i++) {  
    if(!memcmp(argv[i],"wmode1=",7)) {
      opt->weight1.weight_mode=atoi(argv[i]+7);            
    } else if(!memcmp(argv[i],"wmode2=",7)) {
      opt->weight2.weight_mode=atoi(argv[i]+7);
    } else {
      printf("Usage: ... [wmode1=0|1|2|3] [wmode2=0|1|2|3]\n");
      return -1;
    }
  }
  return 0;
}

static int wrr_parse_class_modf(int argc, char** argv, 
                                struct tc_wrr_class_modf* modf) {
  int i;
  
  if(argc<1) {
//    fprintf(stderr, "Usage: ... [weight1=val] [decr1=val] [incr1=val] [min1=val] [max1=val] [val2=val] ...\n");
//    fprintf(stderr, "  The values can be floating point like 0.42 or divisions like 42/100\n");
    return -1;
  }
  
  // Set meaningless values:
  modf->weight1.val=0;
  modf->weight1.decr=(__u64)-1;
  modf->weight1.incr=(__u64)-1;
  modf->weight1.min=0;
  modf->weight1.max=0;
  modf->weight2.val=0;
  modf->weight2.decr=(__u64)-1;
  modf->weight2.incr=(__u64)-1;
  modf->weight2.min=0;
  modf->weight2.max=0;
  
  // And read values:
  for(i=0; i<argc; i++) {
    char arg[80];
    char* name,*value1=0,*value2=0;
    long double f_val1,f_val2=1,value;
    if(strlen(argv[i])>=sizeof(arg)) {
//      fprintf(stderr,"Argument too long: %s\n",argv[i]);
      return -1;
    }
    strcpy(arg,argv[i]);
    
    name=strtok(arg,"=");
    if(name) value1=strtok(0,"/");
    if(value1) value2=strtok(0,"");
    
    if(!value1) {
//      fprintf(stderr,"No = found in argument: %s\n",argv[i]);
      return -1;
    }
    
    f_val1=atof(value1);
    if(value2) f_val2=atof(value2);    
    
    if(f_val2==0)  {
//      fprintf(stderr,"Division by 0\n");
      return -1;
    }
        
    value=f_val1/f_val2;    
    if(value>1) value=1;
    if(value<0) value=0;            
    value*=((__u64)-1);
    
    // And find the value set
    if(!strcmp(name,"weight1"))    modf->weight1.val=value;
    else if(!strcmp(name,"decr1")) modf->weight1.decr=value;
    else if(!strcmp(name,"incr1")) modf->weight1.incr=value;
    else if(!strcmp(name,"min1"))  modf->weight1.min=value;
    else if(!strcmp(name,"max1"))  modf->weight1.max=value;
    else if(!strcmp(name,"weight2")) modf->weight2.val=value;
    else if(!strcmp(name,"decr2")) modf->weight2.decr=value;
    else if(!strcmp(name,"incr2")) modf->weight2.incr=value;
    else if(!strcmp(name,"min2"))  modf->weight2.min=value;
    else if(!strcmp(name,"max2"))  modf->weight2.max=value;
    else {
//      fprintf(stderr,"illegal value: %s\n",name);
      return -1;
    }
  }    

  return 0;
}

static int wrr_parse_opt(struct qdisc_util *qu, int argc, char **argv, struct nlmsghdr *n)
{
  if(n->nlmsg_flags & NLM_F_CREATE) {
    // This is a create request:
    struct tc_wrr_qdisc_crt opt;
	
    int sour,dest,ip,mac,masq;

    if(argc<4) {
//      fprintf(stderr, "Usage: ... wrr sour|dest ip|masq|mac maxclasses proxymaxcon [penalty-setup]\n");
      return -1;
    }	  
  
    // Read sour/dest:
    memset(&opt,0,sizeof(opt));
    sour=!strcmp(argv[0],"sour");
    dest=!strcmp(argv[0],"dest");	
	
    if(!sour && !dest) {
//      fprintf(stderr,"sour or dest must be specified\n");
      return -1;
    }	

    // Read ip/mac
    ip=!strcmp(argv[1],"ip");
    mac=!strcmp(argv[1],"mac");	
    masq=!strcmp(argv[1],"masq");	

    if(!ip && !mac && !masq) {
//      fprintf(stderr,"ip, masq or mac must be specified\n");
      return -1;
    }	

    opt.srcaddr=sour;		
    opt.usemac=mac;
    opt.usemasq=masq;		
    opt.bands_max=atoi(argv[2]);
    
    opt.proxy_maxconn=atoi(argv[3]);
    
    // Read weights:
    if(wrr_parse_qdisc_weight(argc-4,argv+4,&opt.qdisc_modf)<0) return -1;
    if(opt.qdisc_modf.weight1.weight_mode==-1) opt.qdisc_modf.weight1.weight_mode=0;
    if(opt.qdisc_modf.weight2.weight_mode==-1) opt.qdisc_modf.weight2.weight_mode=0;
		
    addattr_l(n, 1024, TCA_OPTIONS, &opt, sizeof(opt));
  } else {
    struct tc_wrr_qdisc_modf_std opt;
    char qdisc,class;
    
    // This is a modify request:
    if(argc<1) {
//      fprintf(stderr,"... qdisc ... or ... class ...\n");
      return -1;
    }
            
    qdisc=!strcmp(argv[0],"qdisc");
    class=!strcmp(argv[0],"class");

    if(!qdisc && !class) {
//      fprintf(stderr,"qdisc or class must be specified\n");
      return -1;
    }
      
    argc--;
    argv++;
      
    opt.proxy=0;
    
    if(qdisc) {
      opt.change_class=0;
      if(wrr_parse_qdisc_weight(argc, argv, &opt.qdisc_modf)<0) return -1;
    } else {
      int a0,a1,a2,a3,a4=0,a5=0;      

      opt.change_class=1;
      
      if(argc<1) {
//        fprintf(stderr,"... <mac>|<ip>|<masq> ...\n");
        return -1;
      }
      memset(opt.addr,0,sizeof(opt.addr));

      if((sscanf(argv[0],"%i.%i.%i.%i",&a0,&a1,&a2,&a3)!=4) &&
         (sscanf(argv[0],"%x:%x:%x:%x:%x:%x",&a0,&a1,&a2,&a3,&a4,&a5)!=6)) {
//	fprintf(stderr,"Wrong format of mac or ip address\n");
	return -1;
      }
      
      opt.addr[0]=a0; opt.addr[1]=a1; opt.addr[2]=a2;
      opt.addr[3]=a3; opt.addr[4]=a4; opt.addr[5]=a5;

      if(wrr_parse_class_modf(argc-1, argv+1, &opt.class_modf)<0) return -1;
    }  
  
    addattr_l(n, 1024, TCA_OPTIONS, &opt, sizeof(opt));
  }
  return 0;
}

static int wrr_parse_copt(struct qdisc_util *qu, int argc, char **argv, struct nlmsghdr *n) {
  struct tc_wrr_class_modf opt;
  
  memset(&opt,0,sizeof(opt));
  if(wrr_parse_class_modf(argc,argv,&opt)<0) return -1;
  
  addattr_l(n, 1024, TCA_OPTIONS, &opt, sizeof(opt));
  return 0;  
}  

static int wrr_print_opt(struct qdisc_util *qu, FILE *f, struct rtattr *opt)
{
	struct tc_wrr_qdisc_stats *qopt;

	if (opt == NULL)
		return 0;

	if (RTA_PAYLOAD(opt)  < sizeof(*qopt))
		return -1;
	qopt = RTA_DATA(opt);
	
	fprintf(f,"\n  (%s/%s) (maxclasses %i) (usedclasses %i) (reused classes %i)\n",
	  qopt->qdisc_crt.srcaddr ? "sour" : "dest",
	  qopt->qdisc_crt.usemac  ? "mac"  : (qopt->qdisc_crt.usemasq ? "masq" : "ip"),	  
	  qopt->qdisc_crt.bands_max,	  	  	  
	  qopt->bands_cur,
	  qopt->bands_reused
	  );
	  
	if(qopt->qdisc_crt.proxy_maxconn) {
	  fprintf(f,"  (proxy maxcon %i) (proxy curcon %i)\n",
	    qopt->qdisc_crt.proxy_maxconn,qopt->proxy_curconn);
	}
	
	fprintf(f,"  (waiting classes %i) (packets requeued %i) (priosum: %Lg)\n",
	  qopt->nodes_in_heap,
	  qopt->packets_requed,
	  qopt->priosum/((long double)((__u32)-1))
	  );

	fprintf(f,"  (wmode1 %i) (wmode2 %i) \n",
	  qopt->qdisc_crt.qdisc_modf.weight1.weight_mode,
	  qopt->qdisc_crt.qdisc_modf.weight2.weight_mode);
	  
	return 0;
}

static int wrr_print_copt(struct qdisc_util *qu, FILE *f, struct rtattr *opt) {
  struct tc_wrr_class_stats *copt;
  long double d=(__u64)-1;

  if (opt == NULL) return 0;

  if (RTA_PAYLOAD(opt)  < sizeof(*copt))
    return -1;
  copt = RTA_DATA(opt);

  if(!copt->used) {
    fprintf(f,"(unused)");
    return 0;
  }
  
  if(copt->usemac) {
    fprintf(f,"\n  (address: %.2X:%.2X:%.2X:%.2X:%.2X:%.2X)\n",
      copt->addr[0],copt->addr[1],copt->addr[2],
      copt->addr[3],copt->addr[4],copt->addr[5]);
  } else {     
    fprintf(f,"\n  (address: %i.%i.%i.%i)\n",copt->addr[0],copt->addr[1],copt->addr[2],copt->addr[3]);
  }    
  
  fprintf(f,"  (total weight: %Lg) (current position: %i) (counters: %u %u : %u %u)\n",
    (copt->class_modf.weight1.val/d)*(copt->class_modf.weight2.val/d),
    copt->heappos,
    (unsigned)(copt->penal_ms>>32),
    (unsigned)(copt->penal_ms & 0xffffffffU),
    (unsigned)(copt->penal_ls>>32),
    (unsigned)(copt->penal_ls & 0xffffffffU)
    );
    
  fprintf(f,"  Pars 1: (weight %Lg) (decr: %Lg) (incr: %Lg) (min: %Lg) (max: %Lg)\n",
    copt->class_modf.weight1.val/d,
    copt->class_modf.weight1.decr/d,
    copt->class_modf.weight1.incr/d,
    copt->class_modf.weight1.min/d,
    copt->class_modf.weight1.max/d);

  fprintf(f,"  Pars 2: (weight %Lg) (decr: %Lg) (incr: %Lg) (min: %Lg) (max: %Lg)",
    copt->class_modf.weight2.val/d,
    copt->class_modf.weight2.decr/d,
    copt->class_modf.weight2.incr/d,
    copt->class_modf.weight2.min/d,
    copt->class_modf.weight2.max/d);
  
  return 0;
}

static int wrr_print_xstats(struct qdisc_util *qu, FILE *f, struct rtattr *xstats)
{
	return 0;
}


struct qdisc_util wrr_qdisc_util = {
	.id = "wrr",
	.parse_qopt = wrr_parse_opt,
	.print_qopt = wrr_print_opt,
	.print_xstats = wrr_print_xstats,
	.parse_copt = wrr_parse_copt,
	.print_copt = wrr_print_copt
};
