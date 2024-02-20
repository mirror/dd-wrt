/* 
 * libxt_ndpi.c
 * Copyright (C) 2010-2012 G. Elian Gidoni <geg@gnu.org>
 *               2012 Ed Wildgoose <lists@wildgooses.com>
 * 
 * This file is part of nDPI, an open source deep packet inspection
 * library based on the PACE technology by ipoque GmbH
 * 
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; version 2 of the License.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <getopt.h>
#include <errno.h>
#include <inttypes.h>
#include <arpa/inet.h>
#include <xtables.h>

#include <linux/version.h>

#define NDPI_IPTABLES_EXT
#include "xt_ndpi.h"
#include "ndpi_config.h"

#include "regexp.c"

/* copy from ndpi_main.c */

int NDPI_BITMASK_IS_EMPTY(NDPI_PROTOCOL_BITMASK a) {
  int i;

  for(i=0; i<NDPI_NUM_FDS_BITS; i++)
    if(a.fds_bits[i] != 0)
      return(0);

  return(1);
}

#ifndef ARRAY_SIZE
#define ARRAY_SIZE(x) (sizeof(x) / sizeof(*(x)))
#endif

//#if NDPI_LAST_IMPLEMENTED_PROTOCOL != NDPI_PROTOCOL_MAXNUM
//#error LAST_IMPLEMENTED_PROTOCOL != PROTOCOL_MAXNUM
//#endif

static char *prot_short_str[NDPI_NUM_BITS] = { /*NDPI_PROTOCOL_SHORT_STRING,*/ NULL, };
static char  prot_disabled[NDPI_NUM_BITS+1] = { 0, };
static int risk_index_max = 0;
static uint64_t risk_map = 0;
static int proto_init=0;
static int risk_init=0;

#define EXT_OPT_BASE 0
// #define EXT_OPT_BASE NDPI_LAST_IMPLEMENTED_PROTOCOL
enum ndpi_opt_index {
	NDPI_OPT_UNKNOWN=0,
	NDPI_OPT_ALL,
	NDPI_OPT_ERROR,
	NDPI_OPT_PROTO,
	NDPI_OPT_MPROTO,
	NDPI_OPT_APROTO,
	NDPI_OPT_HMASTER,
	NDPI_OPT_HOST,
	NDPI_OPT_INPROGRESS,
	NDPI_OPT_JA3S,
	NDPI_OPT_JA3C,
	NDPI_OPT_JA4C,
	NDPI_OPT_TLSFP,
	NDPI_OPT_TLSV,
	NDPI_OPT_UNTRACKED,
	NDPI_OPT_CLEVEL,
	NDPI_OPT_RISK,
	NDPI_OPT_LAST
};

#define FLAGS_ALL 0x1
#define FLAGS_ERR 0x2
#define FLAGS_HMASTER 0x4
#define FLAGS_MPROTO 0x8
#define FLAGS_APROTO 0x10
#define FLAGS_HOST 0x20
#define FLAGS_INPROGRESS 0x40
#define FLAGS_PROTO 0x80
#define FLAGS_JA3S 0x100
#define FLAGS_JA3C 0x200
#define FLAGS_TLSFP 0x400
#define FLAGS_TLSV 0x800
#define FLAGS_UNTRACKED 0x1000
#define FLAGS_CLEVEL 0x2000
#define FLAGS_HPROTO 0x4000
#define FLAGS_RISK 0x8000
#define FLAGS_JA4C 0x10000

static void load_kernel_proto (void) {
	char buf[128],*c,pname[32],mark[32];
	uint32_t index;
	FILE *f_proto;

	if(proto_init) return;

	f_proto = fopen("/proc/net/xt_ndpi/proto","r");

	if(!f_proto)
		xtables_error(PARAMETER_PROBLEM, "xt_ndpi: kernel module is not loaded.");
	pname[0] = '\0';
	index = 0;
	while(!feof(f_proto)) {
		c = fgets(buf,sizeof(buf)-1,f_proto);
		if(!c) break;
		if(buf[0] == '#') {
			if(!pname[0] && !strncmp(buf,"#id",3)) {
			   char *vs;
			   vs = strchr(buf,'\n');
			   if(vs) *vs = '\0';
			   vs = strstr(buf,"#version");
			   if(!vs)
	    			xtables_error(PARAMETER_PROBLEM, "xt_ndpi: #version missing");
			   if(!strstr(vs+8,NDPI_GIT_RELEASE))
	    			xtables_error(PARAMETER_PROBLEM, "xt_ndpi: module version %s != %s",
						vs+8,NDPI_GIT_RELEASE);
			    pname[0] = ' ';
			}
			continue;
		}
		if(!pname[0]) continue;
		if(sscanf(buf,"%x %s %s",&index,mark,pname) != 3) continue;
		if(index >= NDPI_NUM_BITS) continue;
		prot_disabled[index] = strncmp(mark,"disable",7) == 0;
		prot_short_str[index] = strdup(pname);	
	}
	fclose(f_proto);
	if(index >= NDPI_NUM_BITS)
	    xtables_error(PARAMETER_PROBLEM, "xt_ndpi: kernel module version missmatch.");
	proto_init = 1;
}
static void load_kernel_risk (void) {
	char buf[128],*c,re;
	FILE *f_risk;
	int ri;

	if(risk_init) return;

	f_risk = fopen("/proc/net/xt_ndpi/risks","r");
	if(!f_risk)
		xtables_error(PARAMETER_PROBLEM, "xt_ndpi: no risks file");
	while(!feof(f_risk)) {
		c = fgets(buf,sizeof(buf)-1,f_risk);
		if(!c) break;
		if(sscanf(buf,"%d %c ",&ri,&re) != 2) 
			xtables_error(PARAMETER_PROBLEM, "xt_ndpi: bad risks format.");
		
		if(ri < 0 || ri >= 64)
			xtables_error(PARAMETER_PROBLEM, "xt_ndpi: bad risk index.");
		if(risk_index_max < ri)
			risk_index_max = ri;
		if(re == 'd')
			risk_map |= (1ull << ri);
	}
	fclose(f_risk);
	risk_init  = 1;
}

static void ndpi_mt_init(struct xt_entry_match *match)
{
	struct xt_ndpi_mtinfo *info = (void *)match->data;
	NDPI_BITMASK_RESET(info->flags);
}
static char *_clevel2str[] = {
	"unknown", "port", "ip", "user",
	"nbpf",	"dpart",  "dcpart", "dcache", "dpi" };

#define clevel2num (sizeof(_clevel2str)/sizeof(_clevel2str[0]))

static const char *clevel2str(int l) {
	return (l > 0 && l < clevel2num) ? _clevel2str[l] : "?";
}
static const char *clevel_op2str(int l) {
	switch(l) {
	  case 1: return "-";
	  case 2: return "+";
	}
	return "";
}
static int str2clevel(const char *s) {
	int i,n,l;
	char *e;

	for(i=0; i < clevel2num; i++)
	    if(!strcasecmp(_clevel2str[i],s)) return i;
	n = -1;
	l = strlen(s);
	for(i=0; i < clevel2num; i++)
	    if(!strncasecmp(_clevel2str[i],s,l)) {
		if(n < 0) n = i;
		   else return -1;
	    }
	if(n >= 0) return n;
	i = strtol(s,&e,0);
	if(*e) return -1;
	return i < 0 || i >= clevel2num ? -1 : i;
}

static int set_risk(uint64_t *risk,int v) {
	if(v < 0 || v > risk_index_max) {
		printf("Error: invalid risk index %d\n",v);
		return 1;
	}
	if(risk_map & (1ULL << v)) {
		printf("Error: risk index %d is disabled\n",v);
		return 1;
	}
	*risk |= 1ULL << v;
	return 0;
}

static int str2risk(const char *s, uint64_t *res) {
	uint64_t risk = 0;
	int v;
	char *e;
	const char *c;

	if(s[0] == '0' && s[1] == 'x') {
		*res = strtoull(s+2,&e,16);
		if(*e)
			printf("Error: invalid hex string '%s'\n",s);
		return *e ? -1:0;
	}
	*res = 0ULL;
	for(c=s,v=0; *c; c++) {
		if(*c == ',') {
			if(set_risk(&risk,v))
				return -EINVAL;
			v = 0;
			continue;
		}
		if(*c < '0' || *c > '9') {
			printf("Error: invalid risk number %s\n",s);
			return -EINVAL;
		}
		v *= 10;
		v += *c - '0';
	}
	if(v)
	    if(set_risk(&risk,v)) return -EINVAL;
	
	*res = risk;
	return 0;
}

static char *risk2str(uint64_t risk) { 
    static char buf[22];
    size_t len = sizeof(buf)-1;
    int ri=0,l=0;
    uint64_t risk_s = risk;

    while(risk != 0 && l < len-4) { // ,XX\0
        if((risk & 0xffffffff) == 0) { ri+=32; risk >>=32; }
        if((risk & 0xffff) == 0) { ri+=16; risk >>=16; }
        if((risk & 0xff) == 0) { ri+=8; risk >>=8; }
        if((risk & 0xf) == 0) { ri+=4; risk >>=4; }
        if((risk & 0x3) == 0) { ri+=2; risk >>=2; }
        if((risk & 0x1) == 0) { ri+=1; risk >>=1; }
        if(l) buf[l++] = ',';
        l += snprintf(&buf[l],3,"%d",ri); 
        ri++;
        risk >>=1;
    }
    if(risk)
        snprintf(buf,len-1,"0x%" PRIx64 ,risk_s);
    else
        buf[l] = '\0';
    return buf;
}

static void 
_ndpi_mt4_save(const void *entry, const struct xt_entry_match *match,int save)
{
	const struct xt_ndpi_mtinfo *info = (const void *)match->data;
	const char *cinv = info->invert ? "! ":"";
	const char *csave = save ? "--":"";
        int i,c,l,t;

	load_kernel_proto();
        for (t = l = c = i = 0; i < NDPI_NUM_BITS; i++) {
		if (!prot_short_str[i] || prot_disabled[i] || 
				!strncmp(prot_short_str[i],"badproto_",9)) continue;
		t++;
		if (NDPI_COMPARE_PROTOCOL_TO_BITMASK(info->flags, i) != 0) {
			l = i; c++;
		}
	}

	if(!save)
		printf(" ndpi");
	if(info->error) {
		printf(" %s%serror",cinv,csave);
		return;
	}
	if(info->untracked) {
		printf(" %s%suntracked",cinv,csave);
		return;
	}
	if(info->invert)
		printf(" !");

	if(info->hostname[0]) {
		printf(" %shost %s",csave,info->hostname);
	}
	if(info->have_master) {
		printf(" %shave-master",csave);
	}
	if(info->clevel) {
		printf(" %sclevel %s%s", csave, clevel_op2str(info->clevel_op),
				clevel2str(info->clevel-1));
	}
	if(info->risk) {
		load_kernel_risk();
		printf(" %srisk %s", csave, risk2str(info->risk));
	}

	if(info->m_proto && !info->p_proto)
		printf(" %smatch-m-proto",csave);
	if(!info->m_proto && info->p_proto)
		printf(" %smatch-a-proto",csave);

	if(!c) return;
	printf(" %s",csave);
	if(info->inprogress) {
	  printf("inprogress");
	} else if(info->ja3s) {
	  printf("ja3s");
	} else if(info->ja3c) {
	  printf("ja3c");
	} else if(info->ja4c) {
	  printf("ja4c");
	} else if(info->tlsfp) {
	  printf("tlsfp");
	} else if(info->tlsv) {
	  printf("tlsv");
	} else
	  printf("proto");

	if( c == 1) {
		printf(" %s",prot_short_str[l]);
		return;
	}
	if( c == t-1 && 
	    !NDPI_COMPARE_PROTOCOL_TO_BITMASK(info->flags,NDPI_PROTOCOL_UNKNOWN) ) { 
		printf(" all");
		return;
	}

	if(c > t/2 + 1) {
	    printf(" all");
	    for (i = 1; i < NDPI_NUM_BITS; i++) {
                if (prot_short_str[i] && !prot_disabled[i] && NDPI_COMPARE_PROTOCOL_TO_BITMASK(info->flags, i) == 0)
			printf(",-%s", prot_short_str[i]);
	    }
	    return;
	}
	printf(" ");
        for (l = i = 0; i < NDPI_NUM_BITS; i++) {
                if (prot_short_str[i] && !prot_disabled[i] && NDPI_COMPARE_PROTOCOL_TO_BITMASK(info->flags, i) != 0)
			printf("%s%s",l++ ? ",":"", prot_short_str[i]);
        }
}

static void 
ndpi_mt4_save(const void *entry, const struct xt_entry_match *match) {
	_ndpi_mt4_save(entry,match,1);
}

static void 
ndpi_mt4_print(const void *entry, const struct xt_entry_match *match,
                  int numeric)
{
	_ndpi_mt4_save(entry,match,0);
}

static int 
ndpi_mt4_parse(int c, char **argv, int invert, unsigned int *flags,
                  const void *entry, struct xt_entry_match **match)
{
	struct xt_ndpi_mtinfo *info = (void *)(*match)->data;
        int i;

	info->invert = invert;

	if(c == NDPI_OPT_ERROR) {
		info->error = 1;
        	*flags |= FLAGS_ERR;
		return true;
	}
	if(c == NDPI_OPT_HMASTER ) {
		info->have_master = 1;
        	*flags |= FLAGS_HMASTER;
		return true;
	}
	if(c == NDPI_OPT_UNTRACKED ) {
		info->untracked = 1;
                *flags |= FLAGS_UNTRACKED;
                return true;
	}
	if(c == NDPI_OPT_MPROTO) {
		info->m_proto = 1;
        	*flags |= FLAGS_MPROTO;
		return true;
	}
	if(c == NDPI_OPT_CLEVEL) {
		int cl = 0;
		info->clevel_op = 0;
		if(optarg[0] == '-') {
			info->clevel_op = 1;
			cl = str2clevel(optarg+1);
		} else if(optarg[0] == '+') {
			info->clevel_op = 2;
			cl = str2clevel(optarg+1);
		} else
			cl = str2clevel(optarg);
		if(cl < 0) {
			printf("Error: invalid clevel %s\n",optarg);
			return false;
		}
		if(cl == 0 && info->clevel_op == 1) {
			printf("Error: impossible condition '-unknown'\n");
			return false;
		}
		if(cl >= NDPI_CONFIDENCE_MAX && info->clevel_op == 2) {
			printf("Error: impossible condition '+dpi'\n");
			return false;
		}
		info->clevel = cl + 1;
        	*flags |= FLAGS_CLEVEL;
		return true;
	}
	if(c == NDPI_OPT_RISK) {
		load_kernel_risk();
		if(str2risk(optarg,&info->risk))
			return false;

        	*flags |= FLAGS_RISK;
		return true;
	}
	if(c == NDPI_OPT_APROTO) {
		info->p_proto = 1;
        	*flags |= FLAGS_APROTO;
		return true;
	}

	if(c == NDPI_OPT_HOST) {
		char *s;
		int re_len = strlen(optarg);

		if(re_len >= sizeof(info->hostname)-1) {
			printf("Error: host name too long. Allowed %zu chars\n",
					sizeof(info->hostname)-1);
			return false;
		}
		if(!*optarg) {
			printf("Error: empty host name\n");
			return false;
		}
		if(info->hostname[0]) {
			printf("Error: Double --host\n");
			return false;
		}
		strncpy(info->hostname,optarg,sizeof(info->hostname)-1);

		for(s = &info->hostname[0]; *s; s++) *s = tolower(*s);

		info->host = 1;
		*flags |= FLAGS_HOST;

		if(info->hostname[0] == '/') {
			char re_buf[sizeof(info->hostname)];
			regexp *pattern;

			if(re_len < 3 || info->hostname[re_len-1] != '/') {
				printf("Invalid regexp '%s'\n",info->hostname);
				return false;
			}
			re_len -= 2;
			strncpy(re_buf,&info->hostname[1],re_len);
			re_buf[re_len] = '\0';

			pattern = ndpi_regcomp(re_buf, &re_len);

			if(!pattern) {
				printf("Bad regexp '%s' '%s'\n",&info->hostname[1],re_buf);
				return false;
			}
			ndpi_regexec(pattern," "); /* no warning about unused regexec */
			free(pattern);
			info->re = 1;

		} else info->re = 0;

		return true;
	}
	if(c == NDPI_OPT_PROTO || c == NDPI_OPT_INPROGRESS ||
	   c == NDPI_OPT_JA3S  || c == NDPI_OPT_JA3C || c == NDPI_OPT_JA4C ||
	   c == NDPI_OPT_TLSFP || c == NDPI_OPT_TLSV) {
		char *np = optarg,*n;
		int num;
		int op;
		load_kernel_proto();
		while((n = strtok(np,",")) != NULL) {
			num = -1;
			op = 1;
			if(*n == '-') {
				op = 0;
				n++;
			}
			for (i = 0; i < NDPI_NUM_BITS; i++) {
			    if(prot_short_str[i] && !strcasecmp(prot_short_str[i],n)) {
				    num = i;
				    break; 
			    }
			}
			if(num < 0) {
			    if(strcmp(n,"all")) {
				printf("Unknown proto '%s'\n",n);
				return false;
			    }
			    for (i = 1; i < NDPI_NUM_BITS; i++) {
				if(prot_short_str[i] && strncmp(prot_short_str[i],"badproto_",9) && !prot_disabled[i]) {
				    if(op)
					NDPI_ADD_PROTOCOL_TO_BITMASK(info->flags,i);
				     else
					NDPI_DEL_PROTOCOL_FROM_BITMASK(info->flags,i);
				}
			    }
			} else {
			    if(prot_disabled[num]) {
				printf("Disabled proto '%s'\n",n);
				return false;
			    }
			    if(op)
				NDPI_ADD_PROTOCOL_TO_BITMASK(info->flags,num);
			     else
				NDPI_DEL_PROTOCOL_FROM_BITMASK(info->flags,num);
			}
			np = NULL;
		}
		if(c == NDPI_OPT_PROTO) { *flags |= FLAGS_PROTO; info->proto = 1; }
		if(c == NDPI_OPT_JA3S)  { *flags |= FLAGS_JA3S;  info->ja3s = 1; }
		if(c == NDPI_OPT_JA3C)  { *flags |= FLAGS_JA3C;  info->ja3c = 1; }
		if(c == NDPI_OPT_JA4C)  { *flags |= FLAGS_JA4C;  info->ja4c = 1; }
		if(c == NDPI_OPT_TLSFP) { *flags |= FLAGS_TLSFP; info->tlsfp = 1; }
		if(c == NDPI_OPT_TLSV)  { *flags |= FLAGS_TLSV;  info->tlsv = 1; }
		if(c == NDPI_OPT_INPROGRESS ) { *flags |= FLAGS_INPROGRESS;
						info->inprogress = 1; }
		if(NDPI_BITMASK_IS_EMPTY(info->flags)) {
			info->empty = 1;
			*flags &= ~FLAGS_PROTO;
		} else
			*flags |= FLAGS_HPROTO;

		return true;
	}
	if(c == NDPI_OPT_UNKNOWN) {
		NDPI_ADD_PROTOCOL_TO_BITMASK(info->flags,NDPI_PROTOCOL_UNKNOWN);
		info->proto = 1;
        	*flags |= FLAGS_PROTO | FLAGS_HPROTO;
		return true;
	}
	if(c == NDPI_OPT_ALL) {
		load_kernel_proto();
		for (i = 1; i < NDPI_NUM_BITS; i++) {
	    	    if(prot_short_str[i] && strncmp(prot_short_str[i],"badproto_",9) && !prot_disabled[i])
			NDPI_ADD_PROTOCOL_TO_BITMASK(info->flags,i);
		}
		info->proto = 1;
        	*flags |= FLAGS_PROTO | FLAGS_HPROTO | FLAGS_ALL;
		return true;
	}
	return false;
}

#ifndef xtables_error
#define xtables_error exit_error
#endif

static void
ndpi_mt_check (unsigned int flags)
{
	int nopt = 0;
	if (!flags)
		xtables_error(PARAMETER_PROBLEM, "xt_ndpi: missing options.");
	
	if (flags & FLAGS_ERR) {
	   if (flags != FLAGS_ERR)
		xtables_error(PARAMETER_PROBLEM, "xt_ndpi: cant use '--error' with other options");
	    else
		return;
	}
	if (flags & FLAGS_UNTRACKED) {
	   if (flags != FLAGS_UNTRACKED)
		xtables_error(PARAMETER_PROBLEM, "xt_ndpi: cant use '--untracked' with other options");
	    else
		return;
	}
	if (flags & FLAGS_HMASTER) {
	   if(flags & (FLAGS_ALL | FLAGS_MPROTO | FLAGS_APROTO))
		 xtables_error(PARAMETER_PROBLEM, "xt_ndpi: cant use '--have-master' with options match-m-proto,match-a-proto,proto");
	      else
		 return;
	}

	if (flags & (FLAGS_APROTO|FLAGS_MPROTO)) {
	    if(!(flags & FLAGS_HPROTO))
		 xtables_error(PARAMETER_PROBLEM, "xt_ndpi: You need to specify at least one protocol");
	}

	if (flags & (FLAGS_PROTO|FLAGS_JA3S|FLAGS_JA3C|FLAGS_JA4C|FLAGS_TLSFP|FLAGS_TLSV|FLAGS_INPROGRESS)) {
	    if(!(flags & FLAGS_HPROTO))
		 xtables_error(PARAMETER_PROBLEM, "xt_ndpi: You need to specify at least one protocol");
	}
	if(flags & FLAGS_PROTO) nopt++;
	if(flags & FLAGS_JA3S)  nopt++;
	if(flags & FLAGS_JA3C)  nopt++;
	if(flags & FLAGS_JA4C)  nopt++;
	if(flags & FLAGS_TLSFP) nopt++;
	if(flags & FLAGS_TLSV)  nopt++;
	if(flags & FLAGS_RISK)  nopt++;
	if(flags & FLAGS_INPROGRESS) nopt++;
	if(nopt != 1)
		 xtables_error(PARAMETER_PROBLEM, "xt_ndpi: --proto|--ja3s|--ja3c|--tlsfp|--tlsv|--inprogress %x %d",flags,nopt);
}

static int cmp_pname(const void *p1, const void *p2) {
	const char *a,*b;
	a = *(const char **)p1;
	b = *(const char **)p2;
	if(a && b) {
		return strcmp( a, b);
	}
	if(a)	return -1;
	if(b)	return 1;
	return 0;
}

static int ndpi_print_prot_list(int cond, char *msg) {
        int i,c,d,l,cp;
	char line[128];
	char *pn[NDPI_NUM_BITS+1];

	load_kernel_proto();
	bzero((char *)&pn[0],sizeof(pn));

        for (i = 1,d = 0,cp = 0; i < NDPI_NUM_BITS; i++) {
	    if(!prot_short_str[i] ||
			  !strncmp(prot_short_str[i],"badproto_",9) ||
			  !strncmp(prot_short_str[i],"free",4)) continue;
	    if(prot_disabled[i] != cond) { 
		    d++;
		    continue;
	    }
	    if(cond && !strncmp(prot_short_str[i],"custom",6)) continue;
	    pn[i-1] = prot_short_str[i];
	    cp++;
	}
	if(!cp) return d;
	if(msg)
		puts(msg);
	qsort(&pn[0],NDPI_NUM_BITS,sizeof(pn[0]),cmp_pname);

        for (i = 0,c = 0,l=0; i < NDPI_NUM_BITS; i++) {
	    if(!pn[i]) break;
	    l += snprintf(&line[l],sizeof(line)-1-l,"%-20s ", pn[i]);
	    if(!c) printf("  ");
	    c++;
	    if(c == 4) {
		    printf("%s\n",line);
		    c = 0; l = 0;
	    }
	}
	if(c > 0) printf("%s\n",line);
	return d;
}

static void
ndpi_mt_help(void)
{
        int d;

	printf( "nDPI version %s\nndpi match options:\n"
		"  --error                Match error detecting process\n"
		"  --untracked            Match if detection is not started for this connection\n"
		"  --host str             Match server host name\n"
		"                         Use /str/ for regexp match.\n"
		"  --clevel L             Match confidence level. -L - level < L, +L - level > L\n"
		"                         Levels: unknown,port,ip,user,nbpf,dpart,dcpart,dcache,dpi\n"
		"  --risk risklist        Match if at least one of the listed risks is present.\n"
		"                         risklist - risk numbers separated by commas.\n"
		"  --have-master          Match if master protocol detected\n"
		"  --match-m-proto        Match master protocol only\n"
		"  --match-a-proto        Match application protocol only\n"
		"  --proto protocols      Match if protocols detected\n"
		"                         (list of protocols comma separated)\n"
		"  --inprogress protocols Match if protocols detection is not finished yet\n"
		"  --ja3s protocols       Match ja3 server hash (user defined protocols)\n"
		"  --ja3c protocols       Match ja3 client hash (user defined protocols)\n"
		"  --ja4c protocols       Match ja4 client hash (user defined protocols)\n"
		"  --tlsfp protocols      Match tls fingerprint (user defined protocols)\n"
		"  --tlsv  protocols      Match tls version (user defined protocols)\n"
		"Special protocol names:\n"
		"  --all              Match any known protocol\n"
		"  --unknown          Match unknown protocol packets\n",
	    NDPI_GIT_RELEASE);
	d = ndpi_print_prot_list(0,
			"Enabled protocols:\n");
	if(!d) return;
	ndpi_print_prot_list(1,"Disabled protocols:\n");
}

static struct option ndpi_mt_opts[NDPI_OPT_LAST+2]; // 0 + last NULL

static struct xtables_match
ndpi_mt4_reg = {
	.version = XTABLES_VERSION,
	.name = "ndpi",
	.revision = 0,
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,28)
	.family = AF_INET,
#else
	.family = NFPROTO_UNSPEC,
#endif
	.size = XT_ALIGN(sizeof(struct xt_ndpi_mtinfo)),
	.userspacesize = offsetof(struct xt_ndpi_mtinfo, reg_data),
	.help = ndpi_mt_help,
	.init = ndpi_mt_init,
	.parse = ndpi_mt4_parse,
	.final_check = ndpi_mt_check,
	.print = ndpi_mt4_print,
	.save = ndpi_mt4_save,
	.extra_opts = ndpi_mt_opts,
};
/* target NDPIMARK */
enum {
        O_SET_VALUE=0,
        O_SET_NDPI,
        O_SET_NDPI_M,
        O_SET_NDPI_P,
        O_SET_MARK,
        O_SET_MARK2,
        O_SET_CLSF,
        O_SET_FLOW,
        O_ACCEPT,
        F_SET_VALUE  = 1 << O_SET_VALUE,
        F_SET_NDPI   = 1 << O_SET_NDPI,
        F_SET_NDPI_M = 1 << O_SET_NDPI_M,
        F_SET_NDPI_P = 1 << O_SET_NDPI_P,
        F_SET_MARK  = 1 << O_SET_MARK,
        F_SET_MARK2 = 1 << O_SET_MARK2,
        F_SET_CLSF = 1 << O_SET_CLSF,
        F_SET_FLOW = 1 << O_SET_FLOW,
        F_ACCEPT   = 1 << O_ACCEPT,
        F_ANY         = F_SET_VALUE | F_SET_NDPI | F_SET_NDPI_M |
			F_SET_NDPI_P |F_SET_MARK | F_SET_CLSF |
			F_ACCEPT,
};

static void NDPI_help(void)
{
        printf(
"nDPI version %s\n"
"NDPI target options:\n"
"  --value value/mask                  Set value = (value & ~mask) | value\n"
"  --ndpi-id                           Set value = (value & ~proto_mask) | proto_mark by any proto\n"
"  --ndpi-id-p                         Set value = (value & ~proto_mask) | proto_mark by proto\n"
"  --ndpi-id-m                         Set value = (value & ~proto_mask) | proto_mark by master protocol\n"
"  --set-mark                          Set nfmark = value\n"
"  --set-clsf                          Set priority = value\n"
"  --flow-info                         Save flow info\n"
"  --accept                            -j ACCEPT\n",
NDPI_GIT_RELEASE
);
load_kernel_proto(); // for version checking
}

static const struct xt_option_entry NDPI_opts[] = {
        {.name = "value",     .id = O_SET_VALUE,  .type = XTTYPE_MARKMASK32},
        {.name = "ndpi-id",   .id = O_SET_NDPI,   .type = XTTYPE_NONE},
        {.name = "ndpi-id-m", .id = O_SET_NDPI_M, .type = XTTYPE_NONE},
        {.name = "ndpi-id-p", .id = O_SET_NDPI_P, .type = XTTYPE_NONE},
        {.name = "set-mark",  .id = O_SET_MARK,   .type = XTTYPE_NONE},
        {.name = "set-mark2", .id = O_SET_MARK2,  .type = XTTYPE_NONE},
        {.name = "set-clsf",  .id = O_SET_CLSF,   .type = XTTYPE_NONE},
        {.name = "flow-info", .id = O_SET_FLOW,   .type = XTTYPE_NONE},
        {.name = "accept",    .id = O_ACCEPT,     .type = XTTYPE_NONE},
        XTOPT_TABLEEND,
};
static void NDPI_parse_v0(struct xt_option_call *cb)
{
        struct xt_ndpi_tginfo *markinfo = cb->data;

        xtables_option_parse(cb);
        switch (cb->entry->id) {
        case O_SET_VALUE:
                markinfo->mark = cb->val.mark;
                markinfo->mask = ~cb->val.mask;
                break;
	case O_SET_NDPI:
		markinfo->any_proto_id = 1;
		markinfo->m_proto_id = 0;
		markinfo->p_proto_id = 0;
		break;
	case O_SET_NDPI_M:
		markinfo->m_proto_id = 1;
		if(markinfo->p_proto_id) {
			markinfo->m_proto_id = 0;
			markinfo->p_proto_id = 0;
			markinfo->any_proto_id = 1;
		}
		break;
	case O_SET_NDPI_P:
		markinfo->p_proto_id = 1;
		if(markinfo->m_proto_id) {
			markinfo->m_proto_id = 0;
			markinfo->p_proto_id = 0;
			markinfo->any_proto_id = 1;
		}
		break;
	case O_SET_MARK:
		markinfo->t_mark = 1;
		break;
	case O_SET_MARK2:
		markinfo->t_mark2 = 1;
		break;
	case O_SET_CLSF:
		markinfo->t_clsf = 1;
		break;
	case O_SET_FLOW:
		markinfo->flow_yes = 1;
		break;
	case O_ACCEPT:
		markinfo->t_accept = 1;
		break;
        default:
                xtables_error(PARAMETER_PROBLEM,
                           "NDPI target: unknown --%s",
                           cb->entry->name);
        }
}

static void NDPI_print_v0(const void *ip,
                          const struct xt_entry_target *target, int numeric)
{
        const struct xt_ndpi_tginfo *info =
                (const struct xt_ndpi_tginfo *)target->data;
char buf[128];
int l;
        l = snprintf(buf,sizeof(buf)-1," NDPI");
	if(info->flow_yes)
	     l += snprintf(&buf[l],sizeof(buf)-l-1, " NETFLOW");
	if(info->t_mark2)
	     l += snprintf(&buf[l],sizeof(buf)-l-1, " set MARK2 ");
	  else
	    if(info->t_mark)
		l += snprintf(&buf[l],sizeof(buf)-l-1, " set MARK ");
	if(info->t_clsf)
	     l += snprintf(&buf[l],sizeof(buf)-l-1, " set CLSF ");
	if(info->mask || info->mark) {
	     if(info->mask)
		l += snprintf(&buf[l],sizeof(buf)-l-1," and 0x%x or 0x%x",
				info->mask,info->mark);
	     else
	        l += snprintf(&buf[l],sizeof(buf)-l-1," set 0x%x", info->mark);
	}
	if(info->any_proto_id)
	     l += snprintf(&buf[l],sizeof(buf)-l-1,
			     " by any ndpi-id");
	else {
		if(info->m_proto_id)
		     l += snprintf(&buf[l],sizeof(buf)-l-1,
			     " by master ndpi-id");
		if(info->p_proto_id)
		     l += snprintf(&buf[l],sizeof(buf)-l-1,
			     " by proto ndpi-id");
	}
	if(info->t_accept)
	     l += snprintf(&buf[l],sizeof(buf)-l-1," ACCEPT");
	printf(buf);
}

static void NDPI_save_v0(const void *ip, const struct xt_entry_target *target)
{
        const struct xt_ndpi_tginfo *info =
                (const struct xt_ndpi_tginfo *)target->data;
	char buf[128];
	int l = 0;
	if(info->mask || info->mark) {
	     l += snprintf(&buf[l],sizeof(buf)-l-1," --value 0x%x", info->mark);
	     if(info->mask)
		l += snprintf(&buf[l],sizeof(buf)-l-1,"/0x%x",~info->mask);
	}
	if(info->any_proto_id)
	     l += snprintf(&buf[l],sizeof(buf)-l-1, " --ndpi-id");
	else {
		if(info->m_proto_id)
		     l += snprintf(&buf[l],sizeof(buf)-l-1, " --ndpi-id-m");
		if(info->p_proto_id)
		     l += snprintf(&buf[l],sizeof(buf)-l-1, " --ndpi-id-p");
	}
	if(info->t_mark2)
	     l += snprintf(&buf[l],sizeof(buf)-l-1, " --set-mark2");
	  else
	    if(info->t_mark)
		l += snprintf(&buf[l],sizeof(buf)-l-1, " --set-mark");
	if(info->t_clsf)
	     l += snprintf(&buf[l],sizeof(buf)-l-1, " --set-clsf");
	if(info->flow_yes)
	     l += snprintf(&buf[l],sizeof(buf)-l-1, " --flow-info");
	if(info->t_accept)
	     l += snprintf(&buf[l],sizeof(buf)-l-1," --accept");
	printf(buf);

}

static void NDPI_check(struct xt_fcheck_call *cb)
{
        if (!(cb->xflags & (F_SET_MARK|F_SET_CLSF|F_SET_FLOW))) 
                xtables_error(PARAMETER_PROBLEM,
                           "NDPI target: Parameter --set-mark, --set-clsf or --flow-info"
                           " is required");
	if(cb->xflags & (F_SET_MARK|F_SET_CLSF))
           if (!(cb->xflags & (F_SET_VALUE|F_SET_NDPI|F_SET_NDPI_M|F_SET_NDPI_P)))
                  xtables_error(PARAMETER_PROBLEM,
                           "NDPI target: Parameter --value or --ndpi-id[-[mp]]"
                           " is required");
}

static struct xtables_target ndpi_tg_reg[] = {
        {
                .family        = NFPROTO_UNSPEC,
                .name          = "NDPI",
                .version       = XTABLES_VERSION,
                .revision      = 0,
                .size          = XT_ALIGN(sizeof(struct xt_ndpi_tginfo)),
                .userspacesize = XT_ALIGN(sizeof(struct xt_ndpi_tginfo)),
                .help          = NDPI_help,
                .print         = NDPI_print_v0,
                .save          = NDPI_save_v0,
                .x6_parse      = NDPI_parse_v0,
                .x6_fcheck     = NDPI_check,
                .x6_options    = NDPI_opts,
        },
};

void _init(void)
{
        int i;

#define MT_OPT(np,protoname,nargs) { i=(np); \
	ndpi_mt_opts[i].name = protoname; ndpi_mt_opts[i].flag = NULL; \
	ndpi_mt_opts[i].has_arg = nargs;  ndpi_mt_opts[i].val = i; }

	MT_OPT(NDPI_OPT_UNKNOWN,"unknown",0)
	MT_OPT(NDPI_OPT_ALL,"all",0)
	MT_OPT(NDPI_OPT_ERROR,"error",0)
	MT_OPT(NDPI_OPT_PROTO,"proto",1)
	MT_OPT(NDPI_OPT_MPROTO,"match-m-proto",0)
	MT_OPT(NDPI_OPT_APROTO,"match-a-proto",0)
	MT_OPT(NDPI_OPT_HMASTER,"have-master",0)
	MT_OPT(NDPI_OPT_HOST,"host",1)
	MT_OPT(NDPI_OPT_INPROGRESS,"inprogress",1)
	MT_OPT(NDPI_OPT_JA3S,"ja3s",1)
	MT_OPT(NDPI_OPT_JA3C,"ja3c",1)
	MT_OPT(NDPI_OPT_JA4C,"ja4c",1)
	MT_OPT(NDPI_OPT_TLSFP,"tlsfp",1)
	MT_OPT(NDPI_OPT_TLSV,"tlsv",1)
	MT_OPT(NDPI_OPT_UNTRACKED,"untracked",0)
	MT_OPT(NDPI_OPT_CLEVEL,"clevel",1)
	MT_OPT(NDPI_OPT_RISK,"risk",1)
	MT_OPT(NDPI_OPT_LAST,NULL,0)

	xtables_register_match(&ndpi_mt4_reg);
	xtables_register_targets(ndpi_tg_reg, ARRAY_SIZE(ndpi_tg_reg));
}
