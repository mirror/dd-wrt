// Copyright (C) 2002, 2003, 2004, 2008, 2010, 2014
//               Enrico Scholz <enrico.scholz@ensc.de>
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; version 3 of the License.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program. If not, see http://www.gnu.org/licenses/.

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include "splint.h"

#include <unistd.h>
#include <errno.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <sys/mman.h>
#include <sys/stat.h>

#include "parser.h"
#include "cfg.h"
#include "util.h"
#include "wrappers.h"
#include "compat.h"
#include "output.h"
#include "inet.h"
#include "dhcp.h"

#define tkEOF			(256)

typedef unsigned int	TokenTable[257];
static unsigned int	*CHARACTERS;

#define chrDIGIT	0x01u
#define chrLOWALPHA	0x02u
#define chrUPPERALPHA	0x04u
#define chrALPHA        (chrUPPERALPHA | chrLOWALPHA)
#define chrBLANK	0x08u
#define chrIP		0x10u
#define chrNUMBER	0x20u
#define chrUNIT		0x40u
#define chrNL		0x80u
#define chrEOF		0x100u
#define chrUSERNAME	0x200u
#define chrIFNAME	0x400u
#define chrFILENAME	0x800u
#define chrBASEMOD	0x1000u
#define chrVARNAME	0x2000u

#define ensc_DHCP_FORWARDER_ULIMIT_H_I_KNOW_WHAT_I_DO
#include "ulimit_codes.h"

enum { MAX_VARNAME_SZ = 32 };

static void
initCharacterClassification(/*@out@*/unsigned int *chrs)
    /*@requires maxSet(*chrs)==257@*/
    /*@globals CHARACTERS@*/
    /*@modifies CHARACTERS, *chrs@*/
{
  int			c;
  unsigned int const	chrSYS = chrIFNAME | chrFILENAME;

    /*@-sizeoftype@*/
  memset(chrs, 0, sizeof(TokenTable));
    /*@=sizeoftype@*/

  /*@+charintliteral@*/
  for (c='0'; c<='9'; ++c) chrs[c] |= (chrDIGIT | chrNUMBER | chrIP |
				       chrSYS | chrUSERNAME | chrVARNAME);
  for (c='A'; c<='Z'; ++c) chrs[c] |= chrUPPERALPHA | chrSYS | chrUSERNAME | chrVARNAME;
  for (c='a'; c<='z'; ++c) chrs[c] |= chrLOWALPHA   | chrSYS | chrUSERNAME | chrVARNAME;
  chrs['\r'] |= chrNL;
  chrs['\n'] |= chrNL;

  chrs['\t'] |= chrBLANK;
  chrs[' ']  |= chrBLANK;
  chrs['.']  |= chrSYS | chrUSERNAME | chrIP | chrVARNAME;
  chrs['_']  |= chrSYS | chrUSERNAME | chrVARNAME;
  chrs['-']  |= chrSYS | chrUSERNAME | chrVARNAME;
  chrs[':']  |= chrSYS;
  chrs['/']  |= chrFILENAME;

  chrs['M']  |= chrUNIT;
  chrs['K']  |= chrUNIT;
  chrs['x']  |= chrBASEMOD;
  chrs['X']  |= chrBASEMOD;
  /*@-charintliteral@*/

  CHARACTERS = chrs;
}


static bool
isCharType(/*@sef@*/int c, /*@sef@*/unsigned int type) /*@*/
{
    /*@-globs@*/
  return (CHARACTERS[c] & type)!=0;
    /*@=globs@*/
}

/*@unchecked@*/static int		look_ahead = tkEOF;
/*@unchecked@*/static unsigned int	line_nr, col_nr;
/*@null@*/static char const		*filename = 0;
char const				*cfg_ptr;
char const				*cfg_end;

  /*@noreturn@*//*@unused@*/
#ifdef NEED_PRINTF
void scEXITFATAL(/*@observer@*//*@sef@*/char const *msg);
#define scEXITFATAL(msg)	exitFatal(msg, sizeof(msg)-1)
#define scWRITE(msg)		(void)write(2, msg, sizeof(msg)-1)
#else
#define scEXITFATAL(msg)  exit(-1)
#define scWRITE(msg)		(void)write(2, msg, sizeof(msg)-1)
#endif
/*@noreturn@*/

static void
exitFatal(/*@observer@*/char const msg[],
	  size_t len) __attribute__ ((noreturn)) /*@*/ ;

static void
exitFatal(char const msg[], size_t len)
{
    /*@-internalglobs@*//*@-globs@*/
  if (filename!=0) (void)write(2, filename, strlen(filename));
  else             scWRITE("<null>");
  scWRITE(":");
  writeUInt(2, line_nr);
  scWRITE(":");
  writeUInt(2, col_nr);
  scWRITE(":");
  (void)write(2, msg, len);
  scWRITE("\n");
    /*@=internalglobs@*//*@=globs@*/

  exit(3);
}

static void
setNext()
    /*@globals col_nr, line_nr, fd, look_ahead@*/
    /*@modifies col_nr, line_nr, look_ahead@*/
{
  char			c;

  ++col_nr;
  if (cfg_ptr == cfg_end)
    look_ahead = tkEOF;
  else {
    c = *cfg_ptr;
    ++cfg_ptr;

    switch (c) {
      case '\n'	:  ++line_nr; col_nr = 0; break;
      case '\t'	:  col_nr = (col_nr+7)/8 * 8;
    }
    look_ahead = static_cast(int)(c);
  }
}

static int
getLookAhead()
    /*@globals look_ahead, fd@*/
    /*@modifies look_ahead@*/
{
  if (look_ahead==tkEOF) setNext();

  return look_ahead;
}

static void
match(char /*@alt int@*/ c)
    /*@globals look_ahead, fd@*/
    /*@modifies look_ahead@*/
{
  int		got = getLookAhead();

  if (got==tkEOF) scEXITFATAL("unexpected EOF while parsing");
  if (got!=c)     scEXITFATAL("unexpected symbol");

  look_ahead = tkEOF;
}

static void
matchStr(/*@in@*/char const *str)
    /*@globals look_ahead, fd@*/
    /*@modifies look_ahead@*/
{
  assert(str!=0);
  for (; *str!='\0'; ++str) match(*str);
}

static /*@exposed@*/ struct InterfaceInfo *
newInterface(struct InterfaceInfoList *ifs)
    /*@modifies *ifs@*/
{
  size_t			new_len;
  struct InterfaceInfo		*result;

  ++ifs->len;

  new_len  = ifs->len * (sizeof(*ifs->dta));
  ifs->dta = static_cast(struct InterfaceInfo*)(Erealloc(ifs->dta, new_len));

  assert(ifs->dta!=0);

  result              = &ifs->dta[ifs->len - 1];
  memset(result, 0, sizeof *result);

  result->if_ip       = INADDR_NONE;
  result->port_client = htons(DHCP_PORT_CLIENT);
  result->port_server = htons(DHCP_PORT_SERVER);
  result->sender_fd   = -1;

  return result;
}

static /*@exposed@*/struct ServerInfo *
newServer(struct ServerInfoList *servers)
    /*@modifies *servers@*/
{
  size_t		new_len;
  struct ServerInfo	*result;

  ++servers->len;
  new_len      = servers->len * (sizeof(*servers->dta));
  servers->dta = static_cast(struct ServerInfo *)(Erealloc(servers->dta, new_len));

  assert(servers->dta!=0);
  result	 = &servers->dta[servers->len - 1];
  result->type   = svUNICAST;
  memset(&result->info, 0, sizeof result->info);

  return result;
}

static /*@exposed@*/struct DHCPSubOption *
newDHCPSubOption(struct DHCPSuboptionsList *suboptionlist, unsigned int code)
    /*@modifies *suboptionlist@*/
{
  size_t		new_len;
  struct DHCPSubOption	*result;
  size_t		i;

  assert(code < 0x100);

  for (i = 0; i < suboptionlist->len; ++i) {
	  if (suboptionlist->dta[i].code == code)
		 scEXITFATAL("duplicate suboption");
  }

  ++suboptionlist->len;
  new_len            = suboptionlist->len * (sizeof(*suboptionlist->dta));
  suboptionlist->dta = Erealloc(suboptionlist->dta, new_len);

  assert(suboptionlist->dta!=0);
  result = &suboptionlist->dta[suboptionlist->len - 1];
  memset(result, 0, sizeof *result);

  result->code = code;

  return result;
}

static /*@exposed@*/ struct UlimitInfo *
registerUlimit(struct UlimitInfoList *ulimits, int code, rlim_t val)
    /*@modifies *ulimits@*/
{
    /*@dependent@*//*@null@*/
  struct UlimitInfo *		result=0;

  if (ulimits->dta!=0) {
    struct UlimitInfo *		limit;
    for (limit=ulimits->dta; limit<&ulimits->dta[ulimits->len]; ++limit) {
      if (limit->code==code) {
	result = limit;
	break;
      }
    }
  }

  if (result==0) {
    size_t		new_len;

    ++ulimits->len;
    new_len      = ulimits->len * (sizeof(*ulimits->dta));
    ulimits->dta = static_cast(struct UlimitInfo *)(Erealloc(ulimits->dta, new_len));

    assert(ulimits->dta!=0);

    result       = &ulimits->dta[ulimits->len - 1];
    result->code = code;
  }

  assert(result!=0);
  assert(result->code==code);

  result->rlim.rlim_cur = val;
  result->rlim.rlim_max = val;

  return result;
}

static /*@exposed@*/ struct InterfaceInfo *
searchInterface(/*@in@*/struct InterfaceInfoList *ifs, /*@in@*/char const *name)
   /*@*/
{
  struct InterfaceInfo		*iface;

  assert(ifs->dta!=0 || ifs->len==0);

  /*@-ptrarith@*//*@-nullptrarith@*/
  for (iface=ifs->dta; iface < ifs->dta + ifs->len; ++iface) {
    assert(iface!=0);
    if (strcmp(name, iface->name)==0) break;
  }

  if (iface==ifs->dta + ifs->len) scEXITFATAL("unknown interface");
  /*@=ptrarith@*//*@=nullptrarith@*/

  assert(iface!=0);

  return iface;
}

static void
matchEOL()
    /*@globals fd, look_ahead@*/
    /*@modifies look_ahead@*/
{
  int			state = 0xFF00;

  while (state!=0xFFFF) {
    int			c = getLookAhead();

    switch (state) {
      case 0xFF00	:
	if      (isCharType(c, chrBLANK)) {}
	else if (isCharType(c, chrNL))    { state = 0; }
	else goto err;

	match(c);
	break;

      case 0		:
	if (isCharType(c, chrNL)) match(c);
	else                      state = 0xFFFF;

	break;

      default		:  assert(false); goto err;
    }
  }

  return;

  err:
  scEXITFATAL("unexpected character");
}

static void
readBlanks()
    /*@globals fd, look_ahead@*/
    /*@modifies look_ahead@*/
{
  int		c   = 0;
  size_t	cnt = 0;

  while (c!=tkEOF) {
    c = getLookAhead();

    if (isCharType(c, chrBLANK)) { ++cnt; match(c); }
    else                         c=tkEOF;
  }

  if (cnt==0) scEXITFATAL("Expected blank, got character");
}

/*@+charintliteral@*/
static size_t
readString(/*@out@*/char *buffer, size_t len, unsigned int char_class)
    /*@requires (maxSet(buffer)+1) >= len@*/
    /*@globals fd, look_ahead@*/
    /*@modifies look_ahead, *buffer@*/
{
  char		*ptr = buffer;

    /*@-ptrarith@*/
  while (ptr+1 < buffer+len) {
    int			c = getLookAhead();

    if (isCharType(c, char_class)) {
      *ptr++ = static_cast(char)(c);
      match(c);
    }
    else break;
  }
    /*@=ptrarith@*/

  if (len>0) *ptr = '\0';
  assertDefined(buffer);	// either buffer is empty or defined...
  assert(ptr>=buffer);

    /*@-strictops@*/
  return static_cast(size_t)(ptr-buffer);
    /*@=strictops@*/
}
/*@=charintliteral@*/

static size_t
readStringExpanded(/*@out@*/char *buffer, size_t len, unsigned int char_class)
{
  int		c = getLookAhead();
  if (c == '\\')
    match(c);
  else if (c == '$') {
    size_t	l;
    size_t	i;
    char	var[MAX_VARNAME_SZ+1];
    char const	*tmp;
    match(c);
    l = readString(var, sizeof var, chrVARNAME);

    if (l==0)
      scEXITFATAL("empty variable name");

    tmp = getenv(var);

    if (!tmp)
      scEXITFATAL("failed not expand variable");

    l = strlen(tmp);
    if (l >= len)
      scEXITFATAL("expanded value too long");

    memcpy(buffer, tmp, l+1);
    for (i = 0; i < l; ++i) {
      if (!isCharType(buffer[i], char_class))
	scEXITFATAL("bad chars in expanded variable");
    }

    return l;
  }

  return readString(buffer, len, char_class);
}

static void
readFileName(/*@out@*/char buffer[], size_t len)
    /*@globals fd, look_ahead@*/
    /*@modifies look_ahead, *buffer@*/
{
  if (readStringExpanded(buffer, len, chrFILENAME)==0)
    scEXITFATAL("Invalid filename");
}

static void
readUserName(/*@out@*/char buffer[], size_t len)
    /*@globals fd, look_ahead@*/
    /*@modifies look_ahead, *buffer@*/
{
  if (readString(buffer, len, chrUSERNAME)==0)
    scEXITFATAL("Invalid user- or groupname");
}

static int
readLimit()
    /*@globals fd, look_ahead, ULIMIT_CODES@*/
    /*@modifies look_ahead@*/
{
  char		buffer[128];
  size_t	i;

  (void)readString(buffer, sizeof buffer, chrALPHA);
  for (i=0; i<sizeof(ULIMIT_CODES)/sizeof(ULIMIT_CODES[0]); ++i) {
    if (strcmp(ULIMIT_CODES[i].name, buffer)==0) return ULIMIT_CODES[i].code;
  }

  scEXITFATAL("Unknown ulimit-specifier");
}

static long
readLong()
    /*@globals fd, look_ahead@*/
    /*@modifies look_ahead@*/
{
  char		value[64];
  char		*ptr = value;
  long		result;
  int		state = 0;
  int		base  = 10;

    /*@-ptrarith@*/
  while (state!=0xFFFF && ptr+2<&value[sizeof(value)]) {
    /*@=ptrarith@*/
    int		c = getLookAhead();

    switch (state) {
      case 0		:
      {
	bool		do_match = true;

	switch (c) {
	  case '-'	:  break;
	  case '0'	:  state = 0x0200; break;
	  default	:  state = 0x0100; do_match=false; break;
	}

	if (do_match) { match(c); *ptr++ = static_cast(char)(c); }
	break;
      }
      case 0x0100	:
	  /*@+charintliteral@*/
	if ( (c>='0' && c<='9' && (c-'0')<base) ||
	     (isCharType(c, chrLOWALPHA)   && (c-'a'+10)<base) ||
	     (isCharType(c, chrUPPERALPHA) && (c-'A'+10)<base) )
	  /*@=charintliteral@*/
	{
	  *ptr++ = static_cast(char)(c);
	  match(c);
	}
	else state = 0xFFFF;
	break;

      case 0x0200	:
	switch (c) {
	  case 'x'	:
	  case 'X'	:
	    match(c);
	    *ptr++ = static_cast(char)(c);
	    base   = 16;
	    state  = 0x0100;
	    break;
	  default	:
	      /*@+charintliteral@*/
	    if (c>'0' && c<'8') state=0x0100;	// octal
	      /*@=charintliteral@*/
	    else                state=0xFFFF;
	    break;
	}
	break;
      default		:  assert(false); goto err;
    }
  }

  assert(ptr<&value[sizeof(value)]);
  assertDefined(value);

  *ptr   = '\0';
  result = strtol(value, &ptr, 0);
  if (*ptr!='\0') goto err;

  return result;

  err:
  scEXITFATAL("Can not parse integer");
}

static int
readInteger()
    /*@globals fd, look_ahead@*/
    /*@modifies look_ahead@*/
{
  return static_cast(int)(readLong());
}

static int
readIntegerExpanded()
{
  char			buffer[1024];
  char			*err_ptr;
  long			res;

  (void)readStringExpanded(buffer, sizeof buffer, chrNUMBER);

  res = strtol(buffer, &err_ptr, 10);
  if (res == LONG_MIN || res == LONG_MAX)
    scEXITFATAL("conversion under/overflow");

  if (err_ptr == buffer || *err_ptr)
    scEXITFATAL("failed to convert number");

  return res;
}

static uint16_t
readPort(uint16_t dflt)
{
  int			res = readIntegerExpanded();

  if (res < 0 || res >= (1<<16))
    scEXITFATAL("port number out of range");
  else if (res == 0)
    res = dflt;

  return res;
}

static rlim_t
readLimitVal()
    /*@globals fd, look_ahead@*/
    /*@modifies look_ahead@*/
{
  long		tmp    = readLong();
  rlim_t	result;
  int		c      = getLookAhead();

  if (tmp==-1l) result = RLIM_INFINITY;
  else          result = static_cast(rlim_t)(tmp);

  switch (c) {
    case 'M'	:  result *= 1024; /*@fallthrough@*/
    case 'K'	:  result *= 1024; match(c); break;
    case 'm'	:  result *= 1000; /*@fallthrough@*/
    case 'k'	:  result *= 1000; match(c); break;
    default	:  break;
  }

  return result;
}

static void
readIfname(/*@out@*/char *iface)
    /*@requires (maxSet(iface)+1) >= IFNAMSIZ @*/
    /*@globals fd, look_ahead@*/
    /*@modifies look_ahead, *iface@*/
{
  if (readStringExpanded(iface, IFNAMSIZ, chrIFNAME)==0)
    scEXITFATAL("Invalid interface name");
}

static void
readIp(/*@out@*/struct in_addr	*ip)
    /*@requires maxSet(ip) == 0@*/
    /*@globals fd, look_ahead@*/
    /*@modifies look_ahead, *ip@*/
{
  char			buffer[1024];
  (void)readStringExpanded(buffer, sizeof buffer, chrIP);

  assertDefined(buffer);	// ptr is an alias for buffer
  if (inet_aton(buffer, ip)==0) scEXITFATAL("Invalid IP");
}

static void
readBool(/*@out@*/bool *val)
    /*@requires maxSet(val) >= 0@*/
    /*@globals fd, look_ahead@*/
    /*@modifies look_ahead, *val@*/
{
  int		state = 0;

  while (state!=0xFFFF) {
    int		c = getLookAhead();

    switch (state) {
      case 0	:
	switch (c) {
	  case 'f'	:  matchStr("false"); *val=false; break;
	  case 't'	:  matchStr("true");  *val=true;  break;
	  case '0'	:  match('0');        *val=false; break;
	  case '1'	:  match('1');        *val=true;  break;
	  case 'N'	:
	  case 'n'	:  state = 0x1000; break;
	  case 'Y'	:
	  case 'y'	:  state = 0x2000; break;
	  default	:  goto err;
	}
	if (state==0) state=0xFFFF;
	else          match(c);
	break;

      case 0x1000	:
	if (isCharType(c, chrBLANK | chrNL | chrEOF)) {
	  *val  = false;
	  state = 0xFFFF;
	}
	else switch (c) {
	  case 'o'	:  *val  = false; state = 0xFFFF; match(c); break;
	  default	:  goto err;
	}
	break;

      case 0x2000	:
	if (isCharType(c, chrBLANK | chrNL | chrEOF)) {
	  *val  = true;
	  state = 0xFFFF;
	}
	else switch (c) {
	  case 'e'	:  *val = true;  state = 0xFFFF; matchStr("es"); break;
	  default	:  goto err;
	}
	break;

      default		:  assert(false);  goto err;
    }
  }

  assertDefined(val);
  return;

  err:
  scEXITFATAL("unexpected character");
}

static struct InterfaceInfo *
readExistingInterface(struct InterfaceInfoList *ifs)
{
	char			ifname[IFNAMSIZ];
	struct InterfaceInfo	*iface;

	readIfname(ifname);
	iface = searchInterface(ifs, ifname);
	readBlanks();

	return iface;
}

static int suboptionCompare(void const *a_, void const *b_)
{
	struct DHCPSubOption const *a = a_;
	struct DHCPSubOption const *b = b_;

	if (a->code < b->code)
		return -1;
	else if (a->code > b->code)
		return +1;
	else
		return 0;
}

static void finishSuboptions(struct InterfaceInfo *iface)
{
	struct DHCPSubOption	*opts = iface->suboptions.dta;
	size_t			i;

	if (!opts)
		return;

	qsort(opts, iface->suboptions.len, sizeof opts[0], suboptionCompare);

	for (i = 0; i < iface->suboptions.len; ++i) {
		if (opts[i].data == NULL)
			opts[i].data = &opts[i].val;
	}
}

static void finishInterfaces(struct InterfaceInfoList *ifaces)
{
	size_t		i;

	for (i = 0; i < ifaces->len; ++i) {
		struct InterfaceInfo	*iface = &ifaces->dta[i];

		finishSuboptions(iface);
	}
}

void
parse(/*@in@*/char const		fname[],
      /*@dependent@*/struct ConfigInfo	*cfg)
    /*@globals fileSystem, internalState, fd, look_ahead, filename, line_nr, col_nr, CHARACTERS, ULIMIT_CODES@*/
    /*@modifies  fileSystem, internalState, *cfg, fd, look_ahead, filename, line_nr, col_nr, CHARACTERS@*/
{
  TokenTable		chrs;
  int			state = 0x0;
  char			ifname[IFNAMSIZ];
  char			name[PATH_MAX];
  long			nr = 0;
  struct in_addr	ip;
  bool			has_clients;
  bool			has_servers;
  uint16_t		port_client = port_client;
  uint16_t		port_server = port_server;
  bool			allow_bcast;
  struct {
      int		code;
      rlim_t		val;
  }			ulimit = { 0,0 };
  int			fd;
  struct stat		st;
  char			*cfg_start;
  struct DHCPSubOption	*suboption = suboption;

  filename = fname;
  line_nr  = 1u;
  col_nr   = 1u;

  fd = open(filename, O_RDONLY);
  if (fd==-1) {
    perror("open()");
    exit(1);
  }

  if (fstat(fd, &st)<0) {
    perror("fstat()");
    exit(1);
  }

  cfg_start = mmap(NULL, st.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
  if (!cfg_start) {
    perror("mmap()");
    exit(1);
  }

  cfg_ptr = cfg_start;
  cfg_end = cfg_start + st.st_size;

  initCharacterClassification(chrs);

  while (state!=0xFFFF) {
    int		c = getLookAhead();

    switch (state) {
      case 0xFF00	:  /* comments */
	if (isCharType(c, chrNL)) {
	  matchEOL(); state=0;
	}
	else match(c);
	break;

      case 0xFFFE	:  matchEOL(); state=0; break;

      case 0		:
	if      (isCharType(c, chrBLANK)) match(c);
	else if (isCharType(c, chrNL))    matchEOL();
	else switch (c) {
	  case '#'	:  state = 0xFF00; break; // comment
	  case 'i'	:  state = 0x0100; break; // interface
	  case 'n'	:  state = 0x0200; break; // name
	  case 's'	:  state = 0x0300; break; // server, suboption
	  case 'u'	:  state = 0x0400; break; // uid, ulimit
	  case 'g'	:  state = 0x0500; break; // gid
	  case 'c'	:  state = 0x0699; break; // chroot
	  case 'l'	:  state = 0x0700; break; // logfile, loglevel
	  case 'p'	:  state = 0x0890; break; // pidfile, ports
	  case tkEOF	:  state = 0xFFFF; break; // end of file
	  default	:  goto err;
	}
	if (state!=0 && state!=0xFFFF) match(c);
	break;

      case 0x0890:
	switch (c) {
	  case 'i':  state = 0x0800; break; /* pidfile */
	  default:
	  case 'o':  state = 0x0850; break; /* ports */
	}
	break;

      case 0x0850:
	matchStr("orts");   readBlanks();
	readIfname(ifname); readBlanks();
	port_client = readPort(DHCP_PORT_CLIENT); readBlanks();
	port_server = readPort(DHCP_PORT_SERVER);
	++state;
	break;

      case 0x0851: {
	struct InterfaceInfo	*iface;

	// Reachable from state 0x0850 only
	assertDefined(ifname);
	assertDefined(port_client);
	assertDefined(port_server);

	iface = searchInterface(&cfg->interfaces, ifname);
	if (port_client)
	  iface->port_client = htons(port_client);
	if (port_server)
	  iface->port_server = htons(port_server);

	state = 0xFFFE;
	break;
      }

      case 0x0800	:
	matchStr("idfile"); readBlanks();
	++state;
	break;

      case 0x0801	:
	readFileName(cfg->pidfile_name, sizeof(cfg->pidfile_name));
	state = 0xFFFE;
	break;

      case 0x0700	:
	matchStr("og");
	++state;
	break;

      case 0x0701	:
	switch (c) {
	  case 'f'	:  matchStr("file");  state = 0x0710; break;
	  case 'l'	:  matchStr("level"); state = 0x0720; break;
	  default	:  goto err;
	}
	readBlanks();
	break;

      case 0x0710	:
	readFileName(cfg->logfile_name, sizeof(cfg->logfile_name));
	state = 0xFFFE;
	break;

      case 0x0720	:
	cfg->loglevel = readInteger();
	state = 0xFFFE;
	break;

      case 0x0400	:
	switch (c) {
	  case 's'	:  state = 0x1401; break;
	  case 'l'	:  state = 0x0900; break;
	  default	:  goto err;
	}
	break;

      case 0x0900	:
	matchStr("limit"); readBlanks();	// (u)limit
	ulimit.code = readLimit(); readBlanks();
	ulimit.val  = readLimitVal();
	state       = 0x0910;
	break;

      case 0x0910	:
	assertDefined(&ulimit);
	(void)registerUlimit(&cfg->ulimits, ulimit.code, ulimit.val);
	state = 0xFFFE;
	break;

      case 0x0500	:
	matchStr("roup"); readBlanks();
	state = 0x501;
	break;

      case 0x1401	:
	matchStr("ser"); readBlanks(); // (u)ser
	state = 0x0401;
	break;

      case 0x0401	: /*@fallthrough@*/
      case 0x0501	:
	readUserName(name, sizeof(name));
	++state;
	break;

      case 0x0402	: /*@fallthrough@*/
      case 0x0502	:
      {
	char		*err_ptr;

	  // can be reached only from state 0x0501 where 'name' was read
	assertDefined(name);

	nr = strtol(name, &err_ptr, 0);
	if (*err_ptr!='\0') state += 0x11;
	else                state += 0x01;

	break;
      }

      case 0x0403	:
	  // Can be reached from state 0x0402 only
	assertDefined(&nr);

	cfg->uid = nr;
	state = 0xFFFE;
	break;

      case 0x0413	:
	  // Can be reached from state 0x0402 only
	assertDefined(name);

	cfg->uid = Egetpwnam(name)->pw_uid;
	state = 0xFFFE;
	break;

      case 0x0503	:
	  // Can be reached from state 0x0502 only
	assertDefined(&nr);

	cfg->gid = nr;
	state = 0xFFFE;
	break;

      case 0x0513	:
	  // Can be reached from state 0x0502 only
	assertDefined(name);

	cfg->gid = Egetgrnam(name)->gr_gid;
	state = 0xFFFE;
	break;

      case 0x0699	:
	switch (c) {
	case 'o':  state = 0x610; break; /* --> "compathack" */
	case 'h':  state = 0x600; break; /* --> "chroot" */
	}
	break;

      case 0x0600	:
	matchStr("hroot"); readBlanks();
	readFileName(name, sizeof(name));
	++state;
	break;

      case 0x0601	:
	  // can be reached from state 0x0600 only where name was read
	assertDefined(name);
	strcpy(cfg->chroot_path, name);
	state = 0xFFFE;
	break;

	/* compathack <int> */
    case 0x610:
	matchStr("ompathack"); readBlanks();
	cfg->compat_hacks |= (1Lu << readInteger());
	state = 0xFFFE;
	break;

	  /* if <name> ... case */
      case 0x100	:
	switch (c) {
	  case 'f'	:  state = 0x0105; break;
	  case 'p'	:  state = 0x0120; break;
	  default	:  goto err;
	}
	break;

      case 0x0105	:
	match('f');             readBlanks();
	readIfname(ifname);     readBlanks();
	readBool(&has_clients); readBlanks();
	readBool(&has_servers); readBlanks();
	readBool(&allow_bcast);
	state = 0x110;
	break;

      case 0x110	:
      {
	struct InterfaceInfo *	iface = newInterface(&cfg->interfaces);

	  // Reachable from state 0x0100 only
	assertDefined(ifname);
	assertDefined(&has_clients);
	assertDefined(&has_servers);
	assertDefined(&allow_bcast);


	strcpy(iface->name, ifname);
	iface->aid[0]      = '\0';
	iface->has_clients = has_clients;
	iface->has_servers = has_servers;
	iface->allow_bcast = allow_bcast;
	iface->need_mac    = has_clients;

	state = 0xFFFE;
	break;
      }

      case 0x0120	:
	match('p');	      readBlanks();
	readIfname(ifname);   readBlanks();
	readIp(&ip);

	state = 0x0125;
	break;

      case 0x0125	:
      {
	struct InterfaceInfo *	iface;

	  // Reachable from state 0x0120 only
	assertDefined(ifname);
	assertDefined(&ip);
	iface = searchInterface(&cfg->interfaces, ifname);

	iface->if_ip = ip.s_addr;
	state = 0xFFFE;
	break;
      }

      case 0x200	:
	matchStr("ame");      readBlanks();
	state = 0x201;
	break;

      case 0x201	:
      {
        struct InterfaceInfo	*iface;

	iface = readExistingInterface(&cfg->interfaces);
	suboption = newDHCPSubOption(&iface->suboptions, agREMOTEID);

	suboption->val.str = iface->aid;
	state = 0x352;
	break;
      }

      case 0x300	:
	// match option: s
	switch (c) {
	  case 'e' : state = 0x301; break;
	  case 'u' : state = 0x350; break;
	  default  : goto err;
	}
	break;

      case 0x301	:
	// match option: server
	matchStr("erver"); readBlanks();
	state = 0x302;
	break;

      case 0x302	:
	// get first parameter (ip / bcast)
	switch (c) {
	  case 'i'	:  state = 0x310; break;
	  case 'b'	:  state = 0x320; break;
	  default	:  goto err;
	}
	match(c);
	break;

      case 0x350	:
      {
	struct InterfaceInfo	*iface;
	unsigned int		code;

	/* match option: (s)uboption */
	matchStr("uboption");
	readBlanks();

	iface = readExistingInterface(&cfg->interfaces);
	code  = readInteger();

	suboption = newDHCPSubOption(&iface->suboptions, code);

	switch (code) {
	  case agLINKSELECT:
	  case agREPLACESERVER:
	    state = 0x351;
	    break;

	  case agREMOTEID:
	    suboption->val.str = iface->aid;
	    state = 0x352;
	    break;

	  /* Unknown */
	  default:
	    scEXITFATAL("Unknown suboption - feel free to implement this feature by your own");
	}

	readBlanks();
	break;
      }

      case 0x351	:
	/* read a suboption which specifies an IPv4 address */
	readIp(&ip);

	suboption->len    = sizeof(suboption->val.ip);
	suboption->data   = NULL;
	suboption->val.ip = ip.s_addr;

	state = 0xFFFE;
	break;

      case 0x352:
	/* read an interface-name like suboption */
	assertDefined(suboption->val.str);
	readIfname(suboption->val.str);

	suboption->len    = strlen(suboption->val.str);
	suboption->data   = suboption->val.str;

	suboption = NULL;		/* just for debugging... */

	state = 0xFFFE;
	break;

      case 0x310	:
	match('p'); readBlanks();
	readIp(&ip);
	state = 0x311;
	break;

      case 0x311	:
	if (isCharType(c, chrBLANK)) readBlanks();
	if (isCharType(c, chrNL)) ifname[0] = '\0';
	else                      readIfname(ifname);
	state = 0x315;
	break;

      case 0x315	:
      {
	struct ServerInfo	*server = newServer(&cfg->servers);
	struct InterfaceInfo	*iface  = 0;

	  // Reachable from state 0x311 only
	assertDefined(ifname);
	if (ifname[0]!='\0')
	  iface = searchInterface(&cfg->interfaces, ifname);

	server->type            = svUNICAST;
	server->iface           = iface;
	server->info.unicast.ip = ip;

	state = 0xFFFE;
	break;
      }


      case 0x320	:
	matchStr("cast"); readBlanks();
	readIfname(ifname);
	state = 0x321;
	break;

      case 0x321	:
      {
	struct ServerInfo	*server = newServer(&cfg->servers);
	struct InterfaceInfo	*iface;

	  // Reachable from state 0x320 only
	assertDefined(ifname);

	iface              = searchInterface(&cfg->interfaces, ifname);
	iface->need_mac    = true;
	server->type       = svBCAST;
	server->iface      = iface;

	state = 0xFFFE;
	break;
      }

      default		:  assert(false); goto err;
    }
  }

  munmap(cfg_start, st.st_size);
  Eclose(fd);

  finishInterfaces(&cfg->interfaces);

  return;

  err:
  scEXITFATAL("Bad character");
}

  // Local Variables:
  // compile-command: "make -C .. -k"
  // fill-column: 80
  // End:
