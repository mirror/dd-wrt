/* Copyright (c) 2008, 2009
 *      Juergen Weigert (jnweiger@immd4.informatik.uni-erlangen.de)
 *      Michael Schroeder (mlschroe@immd4.informatik.uni-erlangen.de)
 *      Micah Cowan (micah@cowan.name)
 *      Sadrul Habib Chowdhury (sadrul@users.sourceforge.net)
 * Copyright (c) 1993-2002, 2003, 2005, 2006, 2007
 *      Juergen Weigert (jnweiger@immd4.informatik.uni-erlangen.de)
 *      Michael Schroeder (mlschroe@immd4.informatik.uni-erlangen.de)
 * Copyright (c) 1987 Oliver Laumann
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program (see the file COPYING); if not, see
 * https://www.gnu.org/licenses/, or contact Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA  02111-1301  USA
 *
 ****************************************************************
 */

#include <sys/types.h>
#include "config.h"
#include "screen.h"
#include "extern.h"

extern struct display *display, *displays;
extern int real_uid, real_gid, eff_uid, eff_gid;
extern struct term term[];	/* terminal capabilities */
extern struct NewWindow nwin_undef, nwin_default, nwin_options;
extern int force_vt;
extern int hardstatusemu;
#ifdef MAPKEYS
extern char *kmapdef[];
extern struct action umtab[];
extern struct action mmtab[];
extern struct action dmtab[];
extern struct action ktab[];
extern struct kmap_ext *kmap_exts;
extern int kmap_extn;
extern int DefaultEsc;
#endif

static void  AddCap __P((char *));
static void  MakeString __P((char *, char *, int, char *));
static char *findcap __P((char *, char **, int));
static int   copyarg __P((char **, char *));
static int   e_tgetent __P((char *, char *));
static char *e_tgetstr __P((char *, char **));
static int   e_tgetflag __P((char *));
static int   e_tgetnum __P((char *));
#ifdef MAPKEYS
static int   findseq_ge __P((char *, int, unsigned char **));
static void  setseqoff __P((unsigned char *, int, int));
static int   addmapseq __P((char *, int, int));
static int   remmapseq __P((char *, int));
#ifdef DEBUGG
static void  dumpmap __P((void));
#endif
#endif


char Termcap[TERMCAP_BUFSIZE + 8];	/* new termcap +8:"TERMCAP=" */
static int Termcaplen;
static int tcLineLen;
char Term[MAXSTR+5];		/* +5: "TERM=" */
char screenterm[MAXTERMLEN + 1];	/* new $TERM, usually "screen" */

char *extra_incap, *extra_outcap;

static const char TermcapConst[] = "DO=\\E[%dB:LE=\\E[%dD:RI=\\E[%dC:\
UP=\\E[%dA:bs:bt=\\E[Z:cd=\\E[J:ce=\\E[K:cl=\\E[H\\E[J:cm=\\E[%i%d;%dH:\
ct=\\E[3g:do=^J:nd=\\E[C:pt:rc=\\E8:rs=\\Ec:sc=\\E7:st=\\EH:up=\\EM:\
le=^H:bl=^G:cr=^M:it#8:ho=\\E[H:nw=\\EE:ta=^I:is=\\E)0:";

char *
gettermcapstring(s)
char *s;
{
  int i;

  if (display == 0 || s == 0)
    return 0;
  for (i = 0; i < T_N; i++)
    {
      if (term[i].type != T_STR)
	continue;
      if (strcmp(term[i].tcname, s) == 0)
	return D_tcs[i].str;
    }
  return 0;
}

/*
 * Compile the terminal capabilities for a display.
 * Input: tgetent(, D_termname) extra_incap, extra_outcap.
 * Effect: display initialisation.
 */
int
InitTermcap(wi, he)
int wi;
int he;
{
  register char *s;
  int i;
  char tbuf[TERMCAP_BUFSIZE], *tp;
  int t, xue, xse, xme;

  ASSERT(display);
  bzero(tbuf, sizeof(tbuf));
  debug1("InitTermcap: looking for tgetent('%s')\n", D_termname);
  if (*D_termname == 0 || e_tgetent(tbuf, D_termname) != 1)
    {
#ifdef TERMINFO
      Msg(0, "Cannot find terminfo entry for '%s'.", D_termname);
#else
      Msg(0, "Cannot find termcap entry for '%s'.", D_termname);
#endif
      return -1;
    }
  debug1("got it:\n%s\n", tbuf);
#ifdef DEBUG
  if (extra_incap)
    debug1("Extra incap: %s\n", extra_incap);
  if (extra_outcap)
    debug1("Extra outcap: %s\n", extra_outcap);
#endif

  if ((D_tentry = (char *)malloc(TERMCAP_BUFSIZE + (extra_incap ? strlen(extra_incap) + 1 : 0))) == 0)
    {
      Msg(0, "%s", strnomem);
      return -1;
    }

  /*
   * loop through all needed capabilities, record their values in the display
   */
  tp = D_tentry;
  for (i = 0; i < T_N; i++)
    {
      switch(term[i].type)
	{
	case T_FLG:
	  D_tcs[i].flg = e_tgetflag(term[i].tcname);
	  break;
	case T_NUM:
	  D_tcs[i].num = e_tgetnum(term[i].tcname);
	  break;
	case T_STR:
	  D_tcs[i].str = e_tgetstr(term[i].tcname, &tp);
	  /* no empty strings, please */
	  if (D_tcs[i].str && *D_tcs[i].str == 0)
	    D_tcs[i].str = 0;
	  break;
	default:
	  Panic(0, "Illegal tc type in entry #%d", i);
	  /*NOTREACHED*/
	}
    }

  /*
   * Now a good deal of sanity checks on the retrieved capabilities.
   */
  if (D_HC)
    {
      Msg(0, "You can't run screen on a hardcopy terminal.");
      return -1;
    }
  if (D_OS)
    {
      Msg(0, "You can't run screen on a terminal that overstrikes.");
      return -1;
    }
  if (!D_CL)
    {
      Msg(0, "Clear screen capability required.");
      return -1;
    }
  if (!D_CM)
    {
      Msg(0, "Addressable cursor capability required.");
      return -1;
    }
  if ((s = getenv("COLUMNS")) && (i = atoi(s)) > 0)
    D_CO = i;
  if ((s = getenv("LINES")) && (i = atoi(s)) > 0)
    D_LI = i;
  if (wi)
    D_CO = wi;
  if (he)
    D_LI = he;
  if (D_CO <= 0)
    D_CO = 80;
  if (D_LI <= 0)
    D_LI = 24;

  if (D_CTF)
    {
      /* standard fixes for xterms etc */
      /* assume color for everything that looks ansi-compatible */
      if (!D_CAF && D_ME && (InStr(D_ME, "\033[m") || InStr(D_ME, "\033[0m")))
	{
#ifdef TERMINFO
	  D_CAF = "\033[3%p1%dm";
	  D_CAB = "\033[4%p1%dm";
#else
	  D_CAF = "\033[3%dm";
	  D_CAB = "\033[4%dm";
#endif
	}
      if (D_OP && InStr(D_OP, "\033[39;49m"))
        D_CAX = 1;
      if (D_OP && (InStr(D_OP, "\033[m") || InStr(D_OP, "\033[0m")))
        D_OP = 0;
      /* ISO2022 */
      if ((D_EA && InStr(D_EA, "\033(B")) || (D_AS && InStr(D_AS, "\033(0")))
	D_CG0 = 1;
      if (InStr(D_termname, "xterm") || InStr(D_termname, "rxvt") ||
	  (D_CKM && (InStr(D_CKM, "\033[M") || InStr(D_CKM, "\033[<"))))
        {
          D_CXT = 1;
          kmapdef[0] = D_CKM ? SaveStr(D_CKM) : NULL;
        }
      /* "be" seems to be standard for xterms... */
      if (D_CXT)
	D_BE = 1;
    }
  if (nwin_options.flowflag == nwin_undef.flowflag)
    nwin_default.flowflag = D_CNF ? FLOW_NOW * 0 : 
			    D_NX ? FLOW_NOW * 1 :
			    FLOW_AUTOFLAG;
  D_CLP |= (!D_AM || D_XV || D_XN);
  if (!D_BL)
    D_BL = "\007";
  if (!D_BC)
    {
      if (D_BS)
	D_BC = "\b";
      else
	D_BC = D_LE;
    }
  if (!D_CR)
    D_CR = "\r";
  if (!D_NL)
    D_NL = "\n";

  /*
   *  Set up attribute handling.
   *  This is rather complicated because termcap has different
   *  attribute groups.
   */

  if (D_UG > 0)
    D_US = D_UE = 0;
  if (D_SG > 0)
    D_SO = D_SE = 0;
  /* Unfortunately there is no 'mg' capability.
   * For now we think that mg > 0 if sg and ug > 0.
   */
  if (D_UG > 0 && D_SG > 0)
    D_MH = D_MD = D_MR = D_MB = D_ME = 0;

  xue = ATYP_U;
  xse = ATYP_S;
  xme = ATYP_M;

  if (D_SO && D_SE == 0)
    {
      Msg(0, "Warning: 'so' but no 'se' capability.");
      if (D_ME)
	xse = xme;
      else
	D_SO = 0;
    }
  if (D_US && D_UE == 0)
    {
      Msg(0, "Warning: 'us' but no 'ue' capability.");
      if (D_ME)
	xue = xme;
      else
	D_US = 0;
    }
  if ((D_MH || D_MD || D_MR || D_MB) && D_ME == 0)
    {
      Msg(0, "Warning: 'm?' but no 'me' capability.");
      D_MH = D_MD = D_MR = D_MB = 0;
    }
  /*
   * Does ME also reverse the effect of SO and/or US?  This is not
   * clearly specified by the termcap manual. Anyway, we should at
   * least look whether ME and SE/UE are equal:
   */
  if (D_UE && D_SE && strcmp(D_SE, D_UE) == 0)
    xse = xue;
  if (D_SE && D_ME && strcmp(D_ME, D_SE) == 0)
    xse = xme;
  if (D_UE && D_ME && strcmp(D_ME, D_UE) == 0)
    xue = xme;

  for (i = 0; i < NATTR; i++)
    {
      D_attrtab[i] = D_tcs[T_ATTR + i].str;
      D_attrtyp[i] = i == ATTR_SO ? xse : (i == ATTR_US ? xue : xme);
    }
  
  /* Set up missing entries (attributes are priority ordered) */
  s = 0;
  t = 0;
  for (i = 0; i < NATTR; i++)
    if ((s = D_attrtab[i]))
      {
	t = D_attrtyp[i];
	break;
      }
  for (i = 0; i < NATTR; i++)
    {
      if (D_attrtab[i] == 0)
	{
	  D_attrtab[i] = s;
	  D_attrtyp[i] = t;
	}
      else
        {
	  s = D_attrtab[i];
	  t = D_attrtyp[i];
        }
    }
  if (D_CAF || D_CAB || D_CSF || D_CSB)
    D_hascolor = 1;
  if (D_UT)
    D_BE = 1;	/* screen erased with background color */

  if (!D_DO)
    D_DO = D_NL;
  if (!D_SF)
    D_SF = D_NL;
  if (D_IN)
    D_IC = D_IM = 0;
  if (D_EI == 0)
    D_IM = 0;
  /* some strange termcap entries have IC == IM */
  if (D_IC && D_IM && strcmp(D_IC, D_IM) == 0)
    D_IC = 0;
  if (D_KE == 0)
    D_KS = 0;
  if (D_CVN == 0)
    D_CVR = 0;
  if (D_VE == 0)
    D_VI = D_VS = 0;
  if (D_CCE == 0)
    D_CCS = 0;

#ifdef FONT
  if (D_CG0)
    {
      if (D_CS0 == 0)
#ifdef TERMINFO
        D_CS0 = "\033(%p1%c";
#else
        D_CS0 = "\033(%.";
#endif
      if (D_CE0 == 0)
        D_CE0 = "\033(B";
      D_AC = 0;
      D_EA = 0;
    }
  else if (D_AC || (D_AS && D_AE))	/* some kind of graphics */
    {
      D_CS0 = (D_AS && D_AE) ? D_AS : "";
      D_CE0 = (D_AS && D_AE) ? D_AE : "";
      D_CC0 = D_AC;
    }
  else
    {
      D_CS0 = D_CE0 = "";
      D_CC0 = 0;
      D_AC = "";	/* enable default string */
    }

  for (i = 0; i < 256; i++)
    D_c0_tab[i] = i;
  if (D_AC)
    {
      /* init with default string first */
      s = "l+m+k+j+u+t+v+w+q-x|n+o~s_p\"r#`+a:f'g#~o.v-^+<,>h#I#0#y<z>";
      for (i = (strlen(s) - 2) & ~1; i >= 0; i -= 2)
	D_c0_tab[(int)(unsigned char)s[i]] = s[i + 1];
    }
  if (D_CC0)
    for (i = (strlen(D_CC0) - 2) & ~1; i >= 0; i -= 2)
      D_c0_tab[(int)(unsigned char)D_CC0[i]] = D_CC0[i + 1];
  debug1("ISO2022 = %d\n", D_CG0);
#endif /* FONT */
  if (D_PF == 0)
    D_PO = 0;
  debug2("terminal size is %d, %d (says TERMCAP)\n", D_CO, D_LI);

#ifdef FONT
  if (D_CXC)
    if (CreateTransTable(D_CXC))
      return -1;
#endif

  /* Termcap fields Z0 & Z1 contain width-changing sequences. */
  if (D_CZ1 == 0)
    D_CZ0 = 0;

  CheckScreenSize(0);

  if (D_TS == 0 || D_FS == 0 || D_DS == 0)
    D_HS = 0;
  if (D_HS)
    {
      debug("oy! we have a hardware status line, says termcap\n");
      if (D_WS < 0)
        D_WS = 0;
    }
  D_has_hstatus = hardstatusemu & ~HSTATUS_ALWAYS;
  if (D_HS && !(hardstatusemu & HSTATUS_ALWAYS))
    D_has_hstatus = HSTATUS_HS;

#ifdef ENCODINGS
  if (D_CKJ)
    {
      int enc = FindEncoding(D_CKJ);
      if (enc != -1)
	D_encoding = enc;
    }
#endif
  if (!D_tcs[T_NAVIGATE].str && D_tcs[T_NAVIGATE + 1].str)
    D_tcs[T_NAVIGATE].str = D_tcs[T_NAVIGATE + 1].str;  /* kh = @1 */
  if (!D_tcs[T_NAVIGATE + 2].str && D_tcs[T_NAVIGATE + 3].str)
    D_tcs[T_NAVIGATE + 2].str = D_tcs[T_NAVIGATE + 3].str; /* kH = @7 */

  D_UPcost = CalcCost(D_UP);
  D_DOcost = CalcCost(D_DO);
  D_NLcost = CalcCost(D_NL);
  D_LEcost = CalcCost(D_BC);
  D_NDcost = CalcCost(D_ND);
  D_CRcost = CalcCost(D_CR);
  D_IMcost = CalcCost(D_IM);
  D_EIcost = CalcCost(D_EI);

#ifdef AUTO_NUKE
  if (D_CAN)
    {
      debug("termcap has AN, setting autonuke\n");
      D_auto_nuke = 1;
    }
#endif
  if (D_COL > 0)
    {
      debug1("termcap has OL (%d), setting limit\n", D_COL);
      D_obufmax = D_COL;
      D_obuflenmax = D_obuflen - D_obufmax;
    }

  /* Some xterm entries set F0 and F10 to the same string. Nuke F0. */
  if (D_tcs[T_CAPS].str && D_tcs[T_CAPS + 10].str && !strcmp(D_tcs[T_CAPS].str, D_tcs[T_CAPS + 10].str))
    D_tcs[T_CAPS].str = 0;
  /* Some xterm entries set kD to ^?. Nuke it. */
  if (D_tcs[T_NAVIGATE_DELETE].str && !strcmp(D_tcs[T_NAVIGATE_DELETE].str, "\0177"))
    D_tcs[T_NAVIGATE_DELETE].str = 0;
  /* wyse52 entries have kcub1 == kb == ^H. Nuke... */
  if (D_tcs[T_CURSOR + 3].str && !strcmp(D_tcs[T_CURSOR + 3].str, "\008"))
    D_tcs[T_CURSOR + 3].str = 0;

#ifdef MAPKEYS
  D_nseqs = 0;
  for (i = 0; i < T_OCAPS - T_CAPS; i++)
    remap(i, 1);
  for (i = 0; i < kmap_extn; i++)
    remap(i + (KMAP_KEYS+KMAP_AKEYS), 1);
  D_seqp = D_kmaps + 3;
  D_seql = 0;
  D_seqh = 0;
#endif

  D_tcinited = 1;
  MakeTermcap(0);
  /* Make sure libterm uses external term properties for our tputs() calls.  */
  e_tgetent(tbuf, D_termname);
#ifdef MAPKEYS
  CheckEscape();
#endif
  return 0;
}

#ifdef MAPKEYS

int
remap(n, map)
int n;
int map;
{
  char *s = 0;
  int fl = 0, domap = 0;
  struct action *a1, *a2, *tab;
  int l = 0;
  struct kmap_ext *kme = 0;

  a1 = 0;
  if (n >= KMAP_KEYS+KMAP_AKEYS)
    {
      kme = kmap_exts + (n - (KMAP_KEYS+KMAP_AKEYS));
      s = kme->str;
      l = kme->fl & ~KMAP_NOTIMEOUT;
      fl = kme->fl & KMAP_NOTIMEOUT;
      a1 = &kme->um;
    }
  tab = umtab;
  for (;;)
    {
      a2 = 0;
      if (n < KMAP_KEYS+KMAP_AKEYS)
	{
	  a1 = &tab[n];
	  if (n >= KMAP_KEYS)
	    n -= T_OCAPS-T_CURSOR;
	  s = D_tcs[n + T_CAPS].str;
          l = s ? strlen(s) : 0;
	  if (n >= T_CURSOR-T_CAPS)
	    a2 = &tab[n + (T_OCAPS-T_CURSOR)];
	}
      if (s == 0 || l == 0)
	return 0;
      if (a1 && a1->nr == RC_ILLEGAL)
	a1 = 0;
      if (a2 && a2->nr == RC_ILLEGAL)
	a2 = 0;
      if (a1 && a1->nr == RC_STUFF && a1->args[0] && strcmp(a1->args[0], s) == 0)
	a1 = 0;
      if (a2 && a2->nr == RC_STUFF && a2->args[0] && strcmp(a2->args[0], s) == 0)
	a2 = 0;
      domap |= (a1 || a2);
      if (tab == umtab)
	{
	  tab = dmtab;
	  a1 = kme ? &kme->dm : 0;
	}
      else if (tab == dmtab)
	{
	  tab = mmtab;
	  a1 = kme ? &kme->mm : 0;
	}
      else
	break;
    }
  if (n < KMAP_KEYS)
    domap = 1;
  if (map == 0 && domap)
    return 0;
  if (map && !domap)
    return 0;
  debug3("%smapping %s %#x\n", map? "" :"un",s,n);
  if (map)
    return addmapseq(s, l, n | fl);
  else
    return remmapseq(s, l);
}

void
CheckEscape()
{
  struct display *odisplay;
  int i, nr;

  if (DefaultEsc >= 0)
    return;

  odisplay = display;
  for (display = displays; display; display = display->d_next)
    {
      for (i = 0; i < D_nseqs; i += D_kmaps[i + 2] * 2 + 4)
        {
	  nr = (D_kmaps[i] << 8 | D_kmaps[i + 1]) & ~KMAP_NOTIMEOUT;
	  if (nr < KMAP_KEYS+KMAP_AKEYS)
	    {
	      if (umtab[nr].nr == RC_COMMAND)
		break;
	      if (umtab[nr].nr == RC_ILLEGAL && dmtab[nr].nr == RC_COMMAND)
		break;
	    }
	  else
	    {
	      struct kmap_ext *kme = kmap_exts + nr - (KMAP_KEYS+KMAP_AKEYS);
	      if (kme->um.nr == RC_COMMAND)
		break;
	      if (kme->um.nr == RC_ILLEGAL && kme->dm.nr == RC_COMMAND)
		break;
	    }
        }
    }
  if (display == 0)
    {
      display = odisplay;
      return;
    }
  SetEscape((struct acluser *)0, Ctrl('a'), 'a');
  if (odisplay->d_user->u_Esc == -1)
    odisplay->d_user->u_Esc = DefaultEsc;
  if (odisplay->d_user->u_MetaEsc == -1)
    odisplay->d_user->u_MetaEsc = DefaultMetaEsc;
  display = 0;
  Msg(0, "Warning: escape char set back to ^A");
  display = odisplay;
}

static int
findseq_ge(seq, k, sp)
char *seq;
int k;
unsigned char **sp;
{
  unsigned char *p;
  int j, l;

  p = D_kmaps;
  while (p - D_kmaps < D_nseqs)
    {
      l = p[2];
      p += 3;
      for (j = 0; ; j++)
	{
	  if (j == k || j == l)
	    j = l - k;
          else if (p[j] != ((unsigned char *)seq)[j])
	    j = p[j] - ((unsigned char *)seq)[j];
	  else
	    continue;
	  break;
	}
      if (j >= 0)
	{
	  *sp = p - 3;
	  return j;
	}
      p += 2 * l + 1;
    }
  *sp = p;
  return -1;
}

static void
setseqoff(p, i, o)
unsigned char *p;
int i;
int o;
{
  unsigned char *q;
  int l, k;

  k = p[2];
  if (o < 256)
    {
      p[k + 4 + i] = o;
      return;
    }
  /* go for the biggest offset */
  for (q = p + k * 2 + 4; ; q += l * 2 + 4)
    {
      l = q[2];
      if ((q + l * 2 - p) / 2 >= 256)
	{
	  p[k + 4 + i] = (q - p - 4) / 2;
	  return;
	}
    }
}

static int
addmapseq(seq, k, nr)
char *seq;
int k;
int nr;
{
  int i, j, l, mo, m;
  unsigned char *p, *q;

  if (k >= 254)
    return -1;
  j = findseq_ge(seq, k, &p);
  if (j == 0)
    {
      p[0] = nr >> 8;
      p[1] = nr;
      return 0;
    }
  i = p - D_kmaps;
  if (D_nseqs + 2 * k + 4 >= D_aseqs)
    {
      D_kmaps = (unsigned char *)xrealloc((char *)D_kmaps, D_aseqs + 256);
      D_aseqs += 256;
      p = D_kmaps + i;
    }
  D_seqp = D_kmaps + 3;
  D_seql = 0;
  D_seqh = 0;
  evdeq(&D_mapev);
  if (j > 0)
    bcopy((char *)p, (char *)p + 2 * k + 4, D_nseqs - i);
  p[0] = nr >> 8;
  p[1] = nr;
  p[2] = k;
  bcopy(seq, (char *)p + 3, k);
  bzero(p + k + 3, k + 1);
  D_nseqs += 2 * k + 4;
  if (j > 0)
    {
      q = p + 2 * k + 4;
      l = q[2];
      for (i = 0; i < k; i++)
        {
	  if (p[3 + i] != q[3 + i])
	    {
	      p[k + 4 + i] = k;
	      break;
	    }
	  setseqoff(p, i, q[l + 4 + i] ? q[l + 4 + i] + k + 2: 0);
	}
    }
  for (q = D_kmaps; q < p; q += 2 * l + 4)
    {
      l = q[2];
      for (m = j = 0; j < l; j++)
	{
	  mo = m;
	  if (!m && q[3 + j] != seq[j])
	    m = 1;
	  if (q[l + 4 + j] == 0)
	    {
	      if (!mo && m)
	        setseqoff(q, j, (p - q - 4) / 2);
	    }
	  else if (q + q[l + 4 + j] * 2 + 4 > p || (q + q[l + 4 + j] * 2 + 4 == p && !m))
	    setseqoff(q, j, q[l + 4 + j] + k + 2);
	}
    }
#ifdef DEBUGG
  dumpmap();
#endif
  return 0;
}

static int
remmapseq(seq, k)
char *seq;
int k;
{
  int j, l;
  unsigned char *p, *q;

  if (k >= 254 || (j = findseq_ge(seq, k, &p)) != 0)
    return -1;
  for (q = D_kmaps; q < p; q += 2 * l + 4)
    {
      l = q[2];
      for (j = 0; j < l; j++)
        {
	  if (q + q[l + 4 + j] * 2 + 4 == p)
	    setseqoff(q, j, p[k + 4 + j] ? q[l + 4 + j] + p[k + 4 + j] - k : 0);
	  else if (q + q[l + 4 + j] * 2 + 4 > p)
	    q[l + 4 + j] -= k + 2;
        }
    }
  if (D_kmaps + D_nseqs > p + 2 * k + 4)
    bcopy((char *)p + 2 * k + 4, (char *)p, (D_kmaps + D_nseqs) - (p + 2 * k + 4));
  D_nseqs -= 2 * k + 4;
  D_seqp = D_kmaps + 3;
  D_seql = 0;
  D_seqh = 0;
  evdeq(&D_mapev);
#ifdef DEBUGG
  dumpmap();
#endif
  return 0;
}

#ifdef DEBUGG
static void
dumpmap()
{
  unsigned char *p;
  int j, n, l, o, oo;
  debug("Mappings:\n");
  p = D_kmaps;
  if (!p)
    return;
  while (p < D_kmaps + D_nseqs)
    {
      l = p[2];
      debug1("%d: ", p - D_kmaps + 3);
      for (j = 0; j < l; j++)
	{
	  o = oo = p[l + 4 + j];
	  if (o)
	    o = 2 * o + 4 + (p + 3 + j - D_kmaps);
	  if (p[j + 3] > ' ' && p[j + 3] < 0177)
	    {
              debug3("%c[%d:%d] ", p[j + 3], oo, o);
	    }
          else
            debug3("\\%03o[%d:%d] ", p[j + 3], oo, o);
	}
      n = p[0] << 8 | p[1];
      debug2(" ==> %d%s\n", n & ~KMAP_NOTIMEOUT, (n & KMAP_NOTIMEOUT) ? " (no timeout)" : "");
      p += 2 * l + 4;
    }
}
#endif /* DEBUGG */

#endif /* MAPKEYS */

/*
 * Appends to the static variable Termcap
 */
static void
AddCap(s)
char *s;
{
  register int n;
  n=strlen(s);
  if (Termcaplen + n < TERMCAP_BUFSIZE - 1)
    {
      strcpy(Termcap + Termcaplen, s);
      Termcaplen += n;
      tcLineLen += n;
    }
}

/*
 * Reads a displays capabilities and reconstructs a termcap entry in the 
 * global buffer "Termcap". A pointer to this buffer is returned.
 */
char *
MakeTermcap(aflag)
int aflag;
{
  char buf[TERMCAP_BUFSIZE];
  register char *p, *cp, *s, ch, *tname;
  int i, wi, he;
#if 0
  int found;
#endif

  if (display)
    {
      wi = D_width;
      he = D_height;
      tname = D_termname;
    }
  else
    {
      wi = 80;
      he = 24;
      tname = "vt100";
    }
  debug1("MakeTermcap(%d)\n", aflag);
  if ((s = getenv("SCREENCAP")) && strlen(s) < TERMCAP_BUFSIZE)
    {
      sprintf(Termcap, "TERMCAP=%s", s);
      strcpy(Term, "TERM=screen");
      debug("getenvSCREENCAP o.k.\n");
      return Termcap;
    }
  Termcaplen = 0;
  debug1("MakeTermcap screenterm='%s'\n", screenterm);
  debug1("MakeTermcap termname='%s'\n", tname);
  if (*screenterm == '\0' || strlen(screenterm) > MAXSTR - 3)
    {
      debug("MakeTermcap sets screenterm=screen\n");
      strncpy(screenterm, "screen", MAXTERMLEN);
      screenterm[MAXTERMLEN] = '\0';
    }
#if 0
  found = 1;
#endif
  do
    {
      strcpy(Term, "TERM=");
      p = Term + 5;
      if (!aflag && strlen(screenterm) + strlen(tname) < MAXSTR-1)
	{
	  sprintf(p, "%s.%s", screenterm, tname);
	  if (e_tgetent(buf, p) == 1)
	    break;
	}
#ifdef COLOR
      if (nwin_default.bce)
	{
	  sprintf(p, "%s-bce", screenterm);
          if (e_tgetent(buf, p) == 1)
	    break;
	}
#endif
#ifdef CHECK_SCREEN_W
      if (wi >= 132)
	{
	  sprintf(p, "%s-w", screenterm);
          if (e_tgetent(buf, p) == 1)
	    break;
	}
#endif
      strcpy(p, screenterm);
      if (e_tgetent(buf, p) == 1)
	break;
      strcpy(p, "vt100");
#if 0
      found = 0;
#endif
    }
  while (0);		/* Goto free programming... */

#if 0
#ifndef TERMINFO
  /* check for compatibility problems, displays == 0 after fork */
  if (found)
    {
      char xbuf[TERMCAP_BUFSIZE], *xbp = xbuf;
      if (tgetstr("im", &xbp) && tgetstr("ic", &xbp) && displays)
	{
	  Msg(0, "Warning: im and ic set in %s termcap entry", p);
	}
    }
#endif
#endif

  tcLineLen = 100;	/* Force NL */
  if (strlen(Term) > TERMCAP_BUFSIZE - 40)
    strcpy(Term, "too_long");
  sprintf(Termcap, "TERMCAP=SC|%s|VT 100/ANSI X3.64 virtual terminal:", Term + 5);
  Termcaplen = strlen(Termcap);
  debug1("MakeTermcap decided '%s'\n", p);
  if (extra_outcap && *extra_outcap)
    {
      for (cp = extra_outcap; (p = index(cp, ':')); cp = p)
	{
	  ch = *++p;
	  *p = '\0';
	  AddCap(cp);
	  *p = ch;
	}
      tcLineLen = 100;	/* Force NL */
    }
  debug1("MakeTermcap after outcap '%s'\n", (char *)TermcapConst);
  if (Termcaplen + strlen(TermcapConst) < TERMCAP_BUFSIZE)
    {
      strcpy(Termcap + Termcaplen, (char *)TermcapConst);
      Termcaplen += strlen(TermcapConst);
    }
  sprintf(buf, "li#%d:co#%d:", he, wi);
  AddCap(buf);
  AddCap("am:");
  if (aflag || (force_vt && !D_COP) || D_CLP || !D_AM)
    {
      AddCap("xn:");
      AddCap("xv:");
      AddCap("LP:");
    }
  if (aflag || (D_CS && D_SR) || D_AL || D_CAL)
    {
      AddCap("sr=\\EM:");
      AddCap("al=\\E[L:");
      AddCap("AL=\\E[%dL:");
    }
  else if (D_SR)
    AddCap("sr=\\EM:");
  if (aflag || D_CS)
    AddCap("cs=\\E[%i%d;%dr:");
  if (aflag || D_CS || D_DL || D_CDL)
    {
      AddCap("dl=\\E[M:");
      AddCap("DL=\\E[%dM:");
    }
  if (aflag || D_DC || D_CDC)
    {
      AddCap("dc=\\E[P:");
      AddCap("DC=\\E[%dP:");
    }
  if (aflag || D_CIC || D_IC || D_IM)
    {
      AddCap("im=\\E[4h:");
      AddCap("ei=\\E[4l:");
      AddCap("mi:");
      AddCap("IC=\\E[%d@:");
    }
#ifdef MAPKEYS
  AddCap("ks=\\E[?1h\\E=:");
  AddCap("ke=\\E[?1l\\E>:");
#endif
  AddCap("vi=\\E[?25l:");
  AddCap("ve=\\E[34h\\E[?25h:");
  AddCap("vs=\\E[34l:");
  AddCap("ti=\\E[?1049h:");
  AddCap("te=\\E[?1049l:");
  if (display)
    {
      if (D_US)
	{
	  AddCap("us=\\E[4m:");
	  AddCap("ue=\\E[24m:");
	}
      if (D_SO)
	{
	  AddCap("so=\\E[3m:");
	  AddCap("se=\\E[23m:");
	}
      if (D_MB)
	AddCap("mb=\\E[5m:");
      if (D_MD)
	AddCap("md=\\E[1m:");
      if (D_MH)
	AddCap("mh=\\E[2m:");
      if (D_MR)
	AddCap("mr=\\E[7m:");
      if (D_MB || D_MD || D_MH || D_MR)
	AddCap("me=\\E[m:ms:");
      if (D_hascolor)
	AddCap("Co#8:pa#64:AF=\\E[3%dm:AB=\\E[4%dm:op=\\E[39;49m:AX:");
      if (D_VB)
	AddCap("vb=\\Eg:");
#ifndef MAPKEYS
      if (D_KS)
	{
	  AddCap("ks=\\E=:");
	  AddCap("ke=\\E>:");
	}
      if (D_CCS)
	{
	  AddCap("CS=\\E[?1h:");
	  AddCap("CE=\\E[?1l:");
	}
#endif
      if (D_CG0)
	AddCap("G0:");
      if (D_CC0 || (D_CS0 && *D_CS0))
	{
	  AddCap("as=\\E(0:");
	  AddCap("ae=\\E(B:");
	  /* avoid `` because some shells dump core... */
	  AddCap("ac=\\140\\140aaffggjjkkllmmnnooppqqrrssttuuvvwwxxyyzz{{||}}~~..--++,,hhII00:");
	}
      if (D_PO)
	{
	  AddCap("po=\\E[5i:");
	  AddCap("pf=\\E[4i:");
	}
      if (D_CZ0)
	{
	  AddCap("Z0=\\E[?3h:");
	  AddCap("Z1=\\E[?3l:");
	}
      if (D_CWS)
	AddCap("WS=\\E[8;%d;%dt:");
    }
  for (i = T_CAPS; i < T_ECAPS; i++)
    {
#ifdef MAPKEYS
      struct action *act;
      if (i < T_OCAPS)
	{
	  if (i >= T_KEYPAD)	/* don't put keypad codes in TERMCAP */
	    continue;		/* - makes it too big */
#if (TERMCAP_BUFSIZE < 1024)
          if (i >= T_FEXTRA && i < T_BACKTAB) /* also skip extra vt220 keys */
            continue;
          if (i > T_BACKTAB && i < T_NAVIGATE) /* more vt220 keys */
            continue;
#endif
	  if (i >= T_CURSOR && i < T_OCAPS)
	    {
	      act = &umtab[i - (T_CURSOR - T_OCAPS + T_CAPS)];
	      if (act->nr == RC_ILLEGAL)
		act = &dmtab[i - (T_CURSOR - T_OCAPS + T_CAPS)];
	    }
	  else
	    {
	      act = &umtab[i - T_CAPS];
	      if (act->nr == RC_ILLEGAL)
		act = &dmtab[i - T_CAPS];
	    }
	  if (act->nr == RC_ILLEGAL && (i == T_NAVIGATE + 1 || i == T_NAVIGATE + 3))
	    {
	      /* kh -> @1, kH -> @7 */
	      act = &umtab[i - T_CAPS - 1];
	      if (act->nr == RC_ILLEGAL)
		act = &dmtab[i - T_CAPS - 1];
	    }
	  if (act->nr != RC_ILLEGAL)
	    {
	      if (act->nr == RC_STUFF)
		{
		  MakeString(term[i].tcname, buf, sizeof(buf), act->args[0]);
		  AddCap(buf);
		}
	      continue;
	    }
	}
#endif
      if (display == 0)
	continue;
      switch(term[i].type)
	{
	case T_STR:
	  if (D_tcs[i].str == 0)
	    break;
	  MakeString(term[i].tcname, buf, sizeof(buf), D_tcs[i].str);
	  AddCap(buf);
	  break;
	case T_FLG:
	  if (D_tcs[i].flg == 0)
	    break;
	  sprintf(buf, "%s:", term[i].tcname);
	  AddCap(buf);
	  break;
	default:
	  break;
	}
    }
  debug("MakeTermcap: end\n");
  return Termcap;
}

#define TERMCAP_MAX_WIDTH 63
void
DumpTermcap(aflag, f)
int aflag;
FILE *f;
{
  register const char *p, *pe;
  int n, col=0;

  if ((p = index(MakeTermcap(aflag), '=')) == NULL)
    return;
  p++;
  debug1("DumpTermcap: '%s'\n", p);
  /* write termcap entry with wrapping */
  while((pe = index(p, ':')))
    {
      n = pe - p + 1;
      if((col > 8) && ((col + n) > TERMCAP_MAX_WIDTH))
        {
          fwrite("\\\n\t:", 1, 4, f);
          col = 8;
        }
      fwrite(p, 1, n, f);
      col += n;
      p = ++pe;
    }
  if(*p)
    fwrite(p, 1, strlen(p), f);
  fputc('\n', f);
}

static void
MakeString(cap, buf, buflen, s)
char *cap, *buf;
int buflen;
char *s;
{
  register char *p, *pmax;
  register unsigned int c;

  p = buf;
  pmax = p + buflen - (3+4+2);
  *p++ = *cap++;
  *p++ = *cap;
  *p++ = '=';
  while ((c = *s++) && (p < pmax))
    {
      switch (c)
	{
	case '\033':
	  *p++ = '\\';
	  *p++ = 'E';
	  break;
	case ':':
	  strcpy(p, "\\072");
	  p += 4;
	  break;
	case '^':
	case '\\':
	  *p++ = '\\';
	  *p++ = c;
	  break;
	default:
	  if (c >= 200)
	    {
	      sprintf(p, "\\%03o", c & 0377);
	      p += 4;
	    }
	  else if (c < ' ')
	    {
	      *p++ = '^';
	      *p++ = c + '@';
	    }
	  else
	    *p++ = c;
	}
    }
  *p++ = ':';
  *p = '\0';
}


#undef QUOTES
#define QUOTES(p) \
  (*p == '\\' && (p[1] == '\\' || p[1] == ',' || p[1] == '%'))

#ifdef FONT
int
CreateTransTable(s)
char *s;
{
  int curchar;
  char *templ, *arg;
  int templlen;
  int templnsub;
  char *p, *sx;
  char **ctable;
  int l, c;

  if ((D_xtable = (char ***)calloc(256, sizeof(char **))) == 0)
    {
      Msg(0, "%s", strnomem);
      return -1;
    }

  while (*s)
    {
      if (QUOTES(s))
	s++;
      curchar = (unsigned char)*s++;
      if (curchar == 'B')
	curchar = 0;	/* ASCII */
      templ = s;
      templlen = 0;
      templnsub = 0;
      if (D_xtable[curchar] == 0)
        {
          if ((D_xtable[curchar] = (char **)calloc(257, sizeof(char *))) == 0)
	    {
	      Msg(0, "%s", strnomem);
	      FreeTransTable();
	      return -1;
	    }
        }
      ctable = D_xtable[curchar];
      for(; *s && *s != ','; s++)
	{
	  if (QUOTES(s))
	      s++;
	  else if (*s == '%')
	    {
	      templnsub++;
	      continue;
	    }
	  templlen++;
	}
      if (*s++ == 0)
	break;
      while (*s && *s != ',')
	{    
	  c = (unsigned char)*s++;
	  if (QUOTES((s - 1)))
	    c = (unsigned char)*s++;
	  else if (c == '%')
	    c = 256;
	  if (ctable[c])
	    free(ctable[c]);
	  arg = s;
	  l = copyarg(&s, (char *)0);
	  if (c != 256)
	    l = l * templnsub + templlen;
	  if ((ctable[c] = (char *)malloc(l + 1)) == 0)
	    {
	      Msg(0, "%s", strnomem);
	      FreeTransTable();
	      return -1;
	    }
	  sx = ctable[c];
	  for (p = ((c == 256) ? "%" : templ); *p && *p != ','; p++)
	    {
	      if (QUOTES(p))
		p++;
	      else if (*p == '%')
		{
		  s = arg;
		  sx += copyarg(&s, sx);
		  continue;
		}
	      *sx++ = *p;
	    }
	  *sx = 0;
	  ASSERT(ctable[c] + l * templnsub + templlen == sx);
	  debug3("XC: %c %c->%s\n", curchar, c, ctable[c]);
	}
      if (*s == ',')
	s++;
    }
  return 0;
}

void
FreeTransTable()
{
  char ***p, **q;
  int i, j;

  if ((p = D_xtable) == 0)
    return;
  for (i = 0; i < 256; i++, p++)
    {
      if (*p == 0)
	continue;
      q = *p;
      for (j = 0; j < 257; j++, q++)
	if (*q)
	  free(*q);
      free(*p);
    }
  free(D_xtable);
  D_xtable = NULL;
}
#endif /* FONT */

static int
copyarg(pp, s)
char **pp, *s;
{
  int l;
  char *p;

  for (l = 0, p = *pp; *p && *p != ','; p++)
    {
      if (QUOTES(p))
	p++;
      if (s)
        *s++ = *p;
      l++;
    }
  if (*p == ',')
    p++;
  *pp = p;
  return l;
}


/*
**
**  Termcap routines that use our extra_incap
**
*/

static int
e_tgetent(bp, name)
char *bp, *name;
{
  int r;

#ifdef USE_SETEUID
  xseteuid(real_uid);
  xsetegid(real_gid);
#endif
  r = tgetent(bp, name);
#ifdef USE_SETEUID
  xseteuid(eff_uid);
  xsetegid(eff_gid);
#endif
  return r;
}


/* findcap:
 *   cap = capability we are looking for
 *   tepp = pointer to bufferpointer
 *   n = size of buffer (0 = infinity)
 */

static char *
findcap(cap, tepp, n)
char *cap;
char **tepp;
int n;
{
  char *tep;
  char c, *p, *cp;
  int mode;	/* mode: 0=LIT  1=^  2=\x  3,4,5=\nnn */
  int num = 0, capl;

  if (!extra_incap)
    return 0;
  tep = *tepp;
  capl = strlen(cap);
  cp = 0;
  mode = 0;
  for (p = extra_incap; *p; )
    {
      if (strncmp(p, cap, capl) == 0)
	{
	  p += capl;
	  c = *p;
	  if (c && c != ':' && c != '@')
	    p++;
	  if (c == 0 || c == '@' || c == '=' || c == ':' || c == '#')
	    cp = tep;
	}
      while ((c = *p))
	{
	  p++;
	  if (mode == 0)
	    {
	      if (c == ':')
	        break;
	      if (c == '^')
		mode = 1;
	      if (c == '\\')
		mode = 2;
	    }
	  else if (mode == 1)
	    {
	      mode = 0;
	      c = c & 0x1f;
	    }
	  else if (mode == 2)
	    {
	      mode = 0;
	      switch(c)
		{
		case '0':
		case '1':
		case '2':
		case '3':
		case '4':
		case '5':
		case '6':
		case '7':
		case '8':
		case '9':
		  mode = 3;
		  num = 0;
		  break;
		case 'E':
		  c = 27;
		  break;
		case 'n':
		  c = '\n';
		  break;
		case 'r':
		  c = '\r';
		  break;
		case 't':
		  c = '\t';
		  break;
		case 'b':
		  c = '\b';
		  break;
		case 'f':
		  c = '\f';
		  break;
		}
	    }
	  if (mode > 2)
	    {
	      num = num * 8 + (c - '0');
	      if (mode++ == 5 || (*p < '0' || *p > '9'))
		{
		  c = num;
		  mode = 0;
		}
	    }
	  if (mode)
	    continue;

	  if (cp && n != 1)
	    {
	      *cp++ = c;
	      n--;
	    }
	}
      if (cp)
	{
	  *cp++ = 0;
	  *tepp = cp;
	  debug2("'%s' found in extra_incap -> %s\n", cap, tep);
	  return tep;
	}
    }
  return 0;
}

static char *
e_tgetstr(cap, tepp)
char *cap;
char **tepp;
{
  char *tep;
  if ((tep = findcap(cap, tepp, 0)))
    return (*tep == '@') ? 0 : tep;
  return tgetstr(cap, tepp);
}

static int
e_tgetflag(cap)
char *cap;
{
  char buf[2], *bufp;
  char *tep;
  bufp = buf;
  if ((tep = findcap(cap, &bufp, 2)))
    return (*tep == '@') ? 0 : 1;
  return tgetflag(cap) > 0;
}

static int
e_tgetnum(cap)
char *cap;
{
  char buf[20], *bufp;
  char *tep, c;
  int res, base = 10;

  bufp = buf;
  if ((tep = findcap(cap, &bufp, 20)))
    {
      c = *tep;
      if (c == '@')
	return -1;
      if (c == '0')
	base = 8;
      res = 0;
      while ((c = *tep++) >= '0' && c <= '9')
	res = res * base + (c - '0');
      return res;
    }
  return tgetnum(cap);
}

