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

#include "config.h"

#include "termcap.h"

#include <sys/types.h>
#include <stdint.h>

#include "screen.h"

#include "encoding.h"
#include "kmapdef.h"
#include "misc.h"
#include "process.h"
#include "resize.h"

static void AddCap(char *);
static void MakeString(char *, char *, int, char *);
static char *findcap(char *, char **, int);
static int copyarg(char **, char *);
static int e_tgetent(char *, char *);
static char *e_tgetstr(char *, char **);
static int e_tgetflag(char *);
static int e_tgetnum(char *);
static int findseq_ge(char *, int, unsigned char **);
static void setseqoff(unsigned char *, int, int);
static int addmapseq(char *, int, int);
static int remmapseq(char *, int);

char Termcap[TERMCAP_BUFSIZE + 8];	/* new termcap +8:"TERMCAP=" */
static int Termcaplen;
static int tcLineLen;
char Term[MAXSTR + 5];		/* +5: "TERM=" */
char screenterm[MAXTERMLEN + 1];	/* new $TERM, usually "screen" */

char *extra_incap, *extra_outcap;

static const char TermcapConst[] = "DO=\\E[%dB:LE=\\E[%dD:RI=\\E[%dC:\
UP=\\E[%dA:bs:bt=\\E[Z:cd=\\E[J:ce=\\E[K:cl=\\E[H\\E[J:cm=\\E[%i%d;%dH:\
ct=\\E[3g:do=^J:nd=\\E[C:pt:rc=\\E8:rs=\\Ec:sc=\\E7:st=\\EH:up=\\EM:\
le=^H:bl=^G:cr=^M:it#8:ho=\\E[H:nw=\\EE:ta=^I:is=\\E)0:";

char *gettermcapstring(char *s)
{
	int i;

	if (display == NULL || s == NULL)
		return NULL;
	for (i = 0; i < T_N; i++) {
		if (term[i].type != T_STR)
			continue;
		if (strcmp(term[i].tcname, s) == 0)
			return D_tcs[i].str;
	}
	return NULL;
}

/*
 * Compile the terminal capabilities for a display.
 * Input: tgetent(, D_termname) extra_incap, extra_outcap.
 * Effect: display initialisation.
 */
int InitTermcap(int width, int height)
{
	char *s;
	int i;
	char tbuf[TERMCAP_BUFSIZE], *tp;
	int t, xue, xse, xme;

	memset(tbuf, 0, ARRAY_SIZE(tbuf));
	if (*D_termname == 0 || e_tgetent(tbuf, D_termname) != 1) {
		Msg(0, "Cannot find terminfo entry for '%s'.", D_termname);
		return -1;
	}

	if ((D_tentry = malloc(TERMCAP_BUFSIZE + (extra_incap ? strlen(extra_incap) + 1 : 0))) == NULL) {
		Msg(0, "%s", strnomem);
		return -1;
	}

	/*
	 * loop through all needed capabilities, record their values in the display
	 */
	tp = D_tentry;
	for (i = 0; i < T_N; i++) {
		switch (term[i].type) {
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
				D_tcs[i].str = NULL;
			break;
		default:
			Panic(0, "Illegal tc type in entry #%d", i);
		 /*NOTREACHED*/}
	}

	/*
	 * Now a good deal of sanity checks on the retrieved capabilities.
	 */
	if (D_HC) {
		Msg(0, "You can't run screen on a hardcopy terminal.");
		return -1;
	}
	if (D_OS) {
		Msg(0, "You can't run screen on a terminal that overstrikes.");
		return -1;
	}
	if (!D_CL) {
		Msg(0, "Clear screen capability required.");
		return -1;
	}
	if (!D_CM) {
		Msg(0, "Addressable cursor capability required.");
		return -1;
	}
	if ((s = getenv("COLUMNS")) && (i = atoi(s)) > 0)
		D_CO = i;
	if ((s = getenv("LINES")) && (i = atoi(s)) > 0)
		D_LI = i;
	if (width)
		D_CO = width;
	if (height)
		D_LI = height;
	if (D_CO <= 0)
		D_CO = 80;
	if (D_LI <= 0)
		D_LI = 24;

	if (D_CTF) {
		/* standard fixes for xterms etc */
		/* assume color for everything that looks ansi-compatible */
		if (!D_CAF && D_ME && (strstr(D_ME, "\033[m") || strstr(D_ME, "\033[0m"))) {
			D_CAF = "\033[3%p1%dm";
			D_CAB = "\033[4%p1%dm";
		}
		if (D_OP && strstr(D_OP, "\033[39;49m"))
			D_CAX = 1;
		if (D_OP && (strstr(D_OP, "\033[m") || strstr(D_OP, "\033[0m")))
			D_OP = NULL;
		/* ISO2022 */
		if ((D_EA && strstr(D_EA, "\033(B")) || (D_AS && strstr(D_AS, "\033(0")))
			D_CG0 = 1;
		if (strstr(D_termname, "xterm") || strstr(D_termname, "rxvt") || (D_CKM &&
					(strstr(D_CKM, "\033[M") || strstr(D_CKM, "\033[<")))) {
			D_CXT = 1;
			kmapdef[0] = D_CKM ? SaveStr(D_CKM) : NULL;
		}
		/* "be" seems to be standard for xterms... */
		if (D_CXT)
			D_BE = 1;
	}
	if (nwin_options.flowflag == nwin_undef.flowflag)
		nwin_default.flowflag = D_CNF ? FLOW_OFF : D_NX ? FLOW_ON : FLOW_AUTOFLAG;
	D_CLP |= (!D_AM || D_XV || D_XN);
	if (!D_BL)
		D_BL = "\007";
	if (!D_BC) {
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
		D_US = D_UE = NULL;
	if (D_SG > 0)
		D_SO = D_SE = NULL;
	/* Unfortunately there is no 'mg' capability.
	 * For now we think that mg > 0 if sg and ug > 0.
	 */
	if (D_UG > 0 && D_SG > 0)
		D_MH = D_MD = D_MR = D_MB = D_ME = NULL;

	xue = ATYP_U;
	xse = ATYP_S;
	xme = ATYP_M;

	if (D_SO && D_SE == NULL) {
		Msg(0, "Warning: 'so' but no 'se' capability.");
		if (D_ME)
			xse = xme;
		else
			D_SO = NULL;
	}
	if (D_US && D_UE == NULL) {
		Msg(0, "Warning: 'us' but no 'ue' capability.");
		if (D_ME)
			xue = xme;
		else
			D_US = NULL;
	}
	if ((D_MH || D_MD || D_MR || D_MB) && D_ME == NULL) {
		Msg(0, "Warning: 'm?' but no 'me' capability.");
		D_MH = D_MD = D_MR = D_MB = NULL;
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

	for (i = 0; i < NATTR; i++) {
		D_attrtab[i] = D_tcs[T_ATTR + i].str;
		D_attrtyp[i] = i == ATTR_SO ? xse : (i == ATTR_US ? xue : xme);
	}

	/* Set up missing entries (attributes are priority ordered) */
	s = NULL;
	t = 0;
	for (i = 0; i < NATTR; i++)
		if ((s = D_attrtab[i])) {
			t = D_attrtyp[i];
			break;
		}
	for (i = 0; i < NATTR; i++) {
		if (D_attrtab[i] == NULL) {
			D_attrtab[i] = s;
			D_attrtyp[i] = t;
		} else {
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
		D_IC = D_IM = NULL;
	if (D_EI == NULL)
		D_IM = NULL;
	/* some strange termcap entries have IC == IM */
	if (D_IC && D_IM && strcmp(D_IC, D_IM) == 0)
		D_IC = NULL;
	if (D_KE == NULL)
		D_KS = NULL;
	if (D_CVN == NULL)
		D_CVR = NULL;
	if (D_VE == NULL)
		D_VI = D_VS = NULL;
	if (D_CCE == NULL)
		D_CCS = NULL;

	if (D_CG0) {
		if (D_CS0 == NULL)
			D_CS0 = "\033(%p1%c";
		if (D_CE0 == NULL)
			D_CE0 = "\033(B";
		D_AC = NULL;
		D_EA = NULL;
	} else if (D_AC || (D_AS && D_AE)) {	/* some kind of graphics */
		D_CS0 = (D_AS && D_AE) ? D_AS : "";
		D_CE0 = (D_AS && D_AE) ? D_AE : "";
		D_CC0 = D_AC;
	} else {
		D_CS0 = D_CE0 = "";
		D_CC0 = NULL;
		D_AC = "";	/* enable default string */
	}

	for (i = 0; i < 256; i++)
		D_c0_tab[i] = i;
	if (D_AC) {
		/* init with default string first */
		s = "l+m+k+j+u+t+v+w+q-x|n+o~s_p\"r#`+a:f'g#~o.v-^+<,>h#I#0#y<z>";
		for (i = (strlen(s) - 2) & ~1; i >= 0; i -= 2)
			D_c0_tab[(int)(unsigned char)s[i]] = s[i + 1];
	}
	if (D_CC0)
		for (i = (strlen(D_CC0) - 2) & ~1; i >= 0; i -= 2)
			D_c0_tab[(int)(unsigned char)D_CC0[i]] = D_CC0[i + 1];
	if (D_PF == NULL)
		D_PO = NULL;

	if (D_CXC)
		if (CreateTransTable(D_CXC))
			return -1;

	/* Termcap fields Z0 & Z1 contain width-changing sequences. */
	if (D_CZ1 == NULL)
		D_CZ0 = NULL;

	CheckScreenSize(0);

	if (D_TS == NULL || D_FS == NULL || D_DS == NULL)
		D_HS = 0;
	if (D_HS) {
		if (D_WS < 0)
			D_WS = 0;
	}
	D_has_hstatus = hardstatusemu & ~HSTATUS_ALWAYS;
	if (D_HS && !(hardstatusemu & HSTATUS_ALWAYS))
		D_has_hstatus = HSTATUS_HS;

	if (D_CKJ) {
		int enc = FindEncoding(D_CKJ);
		if (enc != -1)
			D_encoding = enc;
	}
	if (!D_tcs[T_NAVIGATE].str && D_tcs[T_NAVIGATE + 1].str)
		D_tcs[T_NAVIGATE].str = D_tcs[T_NAVIGATE + 1].str;	/* kh = @1 */
	if (!D_tcs[T_NAVIGATE + 2].str && D_tcs[T_NAVIGATE + 3].str)
		D_tcs[T_NAVIGATE + 2].str = D_tcs[T_NAVIGATE + 3].str;	/* kH = @7 */

	D_UPcost = CalcCost(D_UP);
	D_DOcost = CalcCost(D_DO);
	D_NLcost = CalcCost(D_NL);
	D_LEcost = CalcCost(D_BC);
	D_NDcost = CalcCost(D_ND);
	D_CRcost = CalcCost(D_CR);
	D_IMcost = CalcCost(D_IM);
	D_EIcost = CalcCost(D_EI);

	if (D_CAN) {
		D_auto_nuke = true;
	}
	if (D_COL > 0) {
		D_obufmax = D_COL;
		D_obuflenmax = D_obuflen - D_obufmax;
	}

	/* Some xterm entries set F0 and F10 to the same string. Nuke F0. */
	if (D_tcs[T_CAPS].str && D_tcs[T_CAPS + 10].str && !strcmp(D_tcs[T_CAPS].str, D_tcs[T_CAPS + 10].str))
		D_tcs[T_CAPS].str = NULL;
	/* Some xterm entries set kD to ^?. Nuke it. */
	if (D_tcs[T_NAVIGATE_DELETE].str && !strcmp(D_tcs[T_NAVIGATE_DELETE].str, "\0177"))
		D_tcs[T_NAVIGATE_DELETE].str = NULL;
	/* wyse52 entries have kcub1 == kb == ^H. Nuke... */
	if (D_tcs[T_CURSOR + 3].str && !strcmp(D_tcs[T_CURSOR + 3].str, "\008"))
		D_tcs[T_CURSOR + 3].str = NULL;

	D_nseqs = 0;
	for (i = 0; i < T_OCAPS - T_CAPS; i++)
		remap(i, 1);
	for (i = 0; i < kmap_extn; i++)
		remap(i + (KMAP_KEYS + KMAP_AKEYS), 1);
	D_seqp = D_kmaps + 3;
	D_seql = 0;
	D_seqh = NULL;

	D_tcinited = 1;
	MakeTermcap(0);
	/* Make sure libterm uses external term properties for our tputs() calls.  */
	e_tgetent(tbuf, D_termname);
	CheckEscape();
	return 0;
}

int remap(int n, int map)
{
	char *s = NULL;
	int fl = 0, domap = 0;
	struct action *a1, *a2, *tab;
	int l = 0;
	struct kmap_ext *kme = NULL;

	a1 = NULL;
	if (n >= KMAP_KEYS + KMAP_AKEYS) {
		kme = kmap_exts + (n - (KMAP_KEYS + KMAP_AKEYS));
		s = kme->str;
		l = kme->fl & ~KMAP_NOTIMEOUT;
		fl = kme->fl & KMAP_NOTIMEOUT;
		a1 = &kme->um;
	}
	tab = umtab;
	for (;;) {
		a2 = NULL;
		if (n < KMAP_KEYS + KMAP_AKEYS) {
			a1 = &tab[n];
			if (n >= KMAP_KEYS)
				n -= T_OCAPS - T_CURSOR;
			s = D_tcs[n + T_CAPS].str;
			l = s ? strlen(s) : 0;
			if (n >= T_CURSOR - T_CAPS)
				a2 = &tab[n + (T_OCAPS - T_CURSOR)];
		}
		if (s == NULL || l == 0)
			return 0;
		if (a1 && a1->nr == RC_ILLEGAL)
			a1 = NULL;
		if (a2 && a2->nr == RC_ILLEGAL)
			a2 = NULL;
		if (a1 && a1->nr == RC_STUFF && a1->args[0] && strcmp(a1->args[0], s) == 0)
			a1 = NULL;
		if (a2 && a2->nr == RC_STUFF && a2->args[0] && strcmp(a2->args[0], s) == 0)
			a2 = NULL;
		domap |= (a1 || a2);
		if (tab == umtab) {
			tab = dmtab;
			a1 = kme ? &kme->dm : NULL;
		} else if (tab == dmtab) {
			tab = mmtab;
			a1 = kme ? &kme->mm : NULL;
		} else
			break;
	}
	if (n < KMAP_KEYS)
		domap = 1;
	if (map == 0 && domap)
		return 0;
	if (map && !domap)
		return 0;
	if (map)
		return addmapseq(s, l, n | fl);
	else
		return remmapseq(s, l);
}

void CheckEscape(void)
{
	Display *odisplay;
	int i, nr;

	if (DefaultEsc >= 0)
		return;

	odisplay = display;
	for (display = displays; display; display = display->d_next) {
		for (i = 0; i < D_nseqs; i += D_kmaps[i + 2] * 2 + 4) {
			nr = (D_kmaps[i] << 8 | D_kmaps[i + 1]) & ~KMAP_NOTIMEOUT;
			if (nr < KMAP_KEYS + KMAP_AKEYS) {
				if (umtab[nr].nr == RC_COMMAND)
					break;
				if (umtab[nr].nr == RC_ILLEGAL && dmtab[nr].nr == RC_COMMAND)
					break;
			} else {
				struct kmap_ext *kme = kmap_exts + nr - (KMAP_KEYS + KMAP_AKEYS);
				if (kme->um.nr == RC_COMMAND)
					break;
				if (kme->um.nr == RC_ILLEGAL && kme->dm.nr == RC_COMMAND)
					break;
			}
		}
	}
	if (display == NULL) {
		display = odisplay;
		return;
	}
	SetEscape(NULL, Ctrl('a'), 'a');
	if (odisplay->d_user->u_Esc == -1)
		odisplay->d_user->u_Esc = DefaultEsc;
	if (odisplay->d_user->u_MetaEsc == -1)
		odisplay->d_user->u_MetaEsc = DefaultMetaEsc;
	display = NULL;
	Msg(0, "Warning: escape char set back to ^A");
	display = odisplay;
}

static int findseq_ge(char *seq, int k, unsigned char **sp)
{
	unsigned char *p;
	int j, l;

	p = D_kmaps;
	while (p - D_kmaps < D_nseqs) {
		l = p[2];
		p += 3;
		for (j = 0;; j++) {
			if (j == k || j == l)
				j = l - k;
			else if (p[j] != ((unsigned char *)seq)[j])
				j = p[j] - ((unsigned char *)seq)[j];
			else
				continue;
			break;
		}
		if (j >= 0) {
			*sp = p - 3;
			return j;
		}
		p += 2 * l + 1;
	}
	*sp = p;
	return -1;
}

static void setseqoff(unsigned char *p, int i, int o)
{
	unsigned char *q;
	int l, k;

	k = p[2];
	if (o < 256) {
		p[k + 4 + i] = o;
		return;
	}
	/* go for the biggest offset */
	for (q = p + k * 2 + 4;; q += l * 2 + 4) {
		l = q[2];
		if ((q + l * 2 - p) / 2 >= 256) {
			p[k + 4 + i] = (q - p - 4) / 2;
			return;
		}
	}
}

static int addmapseq(char *seq, int k, int nr)
{
	int i, j, l, mo, m;
	unsigned char *p, *q;

	if (k >= 254)
		return -1;
	j = findseq_ge(seq, k, &p);
	if (j == 0) {
		p[0] = nr >> 8;
		p[1] = nr;
		return 0;
	}
	i = p - D_kmaps;
	if (D_nseqs + 2 * k + 4 >= D_aseqs) {
		D_kmaps = xrealloc((char *)D_kmaps, D_aseqs + 256);
		D_aseqs += 256;
		p = D_kmaps + i;
	}
	D_seqp = D_kmaps + 3;
	D_seql = 0;
	D_seqh = NULL;
	evdeq(&D_mapev);
	if (j > 0)
		memmove((char *)p + 2 * k + 4, (char *)p, D_nseqs - i);
	p[0] = nr >> 8;
	p[1] = nr;
	p[2] = k;
	memmove((char *)p + 3, seq, k);
	memset(p + k + 3, 0, k + 1);
	D_nseqs += 2 * k + 4;
	if (j > 0) {
		q = p + 2 * k + 4;
		l = q[2];
		for (i = 0; i < k; i++) {
			if (p[3 + i] != q[3 + i]) {
				p[k + 4 + i] = k;
				break;
			}
			setseqoff(p, i, q[l + 4 + i] ? q[l + 4 + i] + k + 2 : 0);
		}
	}
	for (q = D_kmaps; q < p; q += 2 * l + 4) {
		l = q[2];
		for (m = j = 0; j < l; j++) {
			mo = m;
			if (!m && q[3 + j] != seq[j])
				m = 1;
			if (q[l + 4 + j] == 0) {
				if (!mo && m)
					setseqoff(q, j, (p - q - 4) / 2);
			} else if (q + q[l + 4 + j] * 2 + 4 > p || (q + q[l + 4 + j] * 2 + 4 == p && !m))
				setseqoff(q, j, q[l + 4 + j] + k + 2);
		}
	}
	return 0;
}

static int remmapseq(char *seq, int k)
{
	int j, l;
	unsigned char *p, *q;

	if (k >= 254 || (j = findseq_ge(seq, k, &p)) != 0)
		return -1;
	for (q = D_kmaps; q < p; q += 2 * l + 4) {
		l = q[2];
		for (j = 0; j < l; j++) {
			if (q + q[l + 4 + j] * 2 + 4 == p)
				setseqoff(q, j, p[k + 4 + j] ? q[l + 4 + j] + p[k + 4 + j] - k : 0);
			else if (q + q[l + 4 + j] * 2 + 4 > p)
				q[l + 4 + j] -= k + 2;
		}
	}
	if (D_kmaps + D_nseqs > p + 2 * k + 4)
		memmove((char *)p, (char *)p + 2 * k + 4, (D_kmaps + D_nseqs) - (p + 2 * k + 4));
	D_nseqs -= 2 * k + 4;
	D_seqp = D_kmaps + 3;
	D_seql = 0;
	D_seqh = NULL;
	evdeq(&D_mapev);
	return 0;
}

/*
 * Appends to the static variable Termcap
 */
static void AddCap(char *s)
{
	int n;

	n = strlen(s);
	if (Termcaplen + n < TERMCAP_BUFSIZE - 1) {
		strcpy(Termcap + Termcaplen, s);
		Termcaplen += n;
		tcLineLen += n;
	}
}

/*
 * Reads a displays capabilities and reconstructs a termcap entry in the
 * global buffer "Termcap". A pointer to this buffer is returned.
 */
char *MakeTermcap(bool aflag)
{
	char buf[TERMCAP_BUFSIZE];
	char *p, *cp, *s, ch, *tname;
	int i, width, height;

	if (display) {
		width = D_width;
		height = D_height;
		tname = D_termname;
	} else {
		width = 80;
		height = 24;
		tname = "vt100";
	}
	if ((s = getenv("SCREENCAP")) && strlen(s) < TERMCAP_BUFSIZE) {
		sprintf(Termcap, "TERMCAP=%s", s);
		strcpy(Term, "TERM=screen");
		return Termcap;
	}
	Termcaplen = 0;
	if (*screenterm == '\0' || strlen(screenterm) > MAXSTR - 3) {
		strncpy(screenterm, "screen", MAXTERMLEN);
		screenterm[MAXTERMLEN] = '\0';
	}
	do {
		strcpy(Term, "TERM=");
		p = Term + 5;
		if (!aflag && strlen(screenterm) + strlen(tname) < MAXSTR - 1) {
			sprintf(p, "%s.%s", screenterm, tname);
			if (e_tgetent(buf, p) == 1)
				break;
		}
		if (nwin_default.bce) {
			sprintf(p, "%s-bce", screenterm);
			if (e_tgetent(buf, p) == 1)
				break;
		}
		strcpy(p, screenterm);
		if (e_tgetent(buf, p) == 1)
			break;
		strcpy(p, "vt100");
	}
	while (0);		/* Goto free programming... */

	tcLineLen = 100;	/* Force NL */
	if (strlen(Term) > TERMCAP_BUFSIZE - 40)
		strcpy(Term, "too_long");
	sprintf(Termcap, "TERMCAP=SC|%s|VT 100/ANSI X3.64 virtual terminal:", Term + 5);
	Termcaplen = strlen(Termcap);
	if (extra_outcap && *extra_outcap) {
		for (cp = extra_outcap; (p = strchr(cp, ':')); cp = p) {
			ch = *++p;
			*p = '\0';
			AddCap(cp);
			*p = ch;
		}
		tcLineLen = 100;	/* Force NL */
	}
	if (Termcaplen + strlen(TermcapConst) < TERMCAP_BUFSIZE) {
		strcpy(Termcap + Termcaplen, TermcapConst);
		Termcaplen += strlen(TermcapConst);
	}
	sprintf(buf, "li#%d:co#%d:", height, width);
	AddCap(buf);
	AddCap("am:");
	if (aflag || (force_vt && !D_COP) || D_CLP || !D_AM) {
		AddCap("xn:");
		AddCap("xv:");
		AddCap("LP:");
	}
	if (aflag || (D_CS && D_SR) || D_AL || D_CAL) {
		AddCap("sr=\\EM:");
		AddCap("al=\\E[L:");
		AddCap("AL=\\E[%dL:");
	} else if (D_SR)
		AddCap("sr=\\EM:");
	if (aflag || D_CS)
		AddCap("cs=\\E[%i%d;%dr:");
	if (aflag || D_CS || D_DL || D_CDL) {
		AddCap("dl=\\E[M:");
		AddCap("DL=\\E[%dM:");
	}
	if (aflag || D_DC || D_CDC) {
		AddCap("dc=\\E[P:");
		AddCap("DC=\\E[%dP:");
	}
	if (aflag || D_CIC || D_IC || D_IM) {
		AddCap("im=\\E[4h:");
		AddCap("ei=\\E[4l:");
		AddCap("mi:");
		AddCap("IC=\\E[%d@:");
	}
	AddCap("ks=\\E[?1h\\E=:");
	AddCap("ke=\\E[?1l\\E>:");
	AddCap("vi=\\E[?25l:");
	AddCap("ve=\\E[34h\\E[?25h:");
	AddCap("vs=\\E[34l:");
	AddCap("ti=\\E[?1049h:");
	AddCap("te=\\E[?1049l:");
	if (display) {
		if (D_US) {
			AddCap("us=\\E[4m:");
			AddCap("ue=\\E[24m:");
		}
		if (D_CZH) {
			AddCap("so=\\E[3m:");
			AddCap("se=\\E[23m:");
		}
		if (D_SO) {
			AddCap("so=\\E[7m:");
			AddCap("se=\\E[27m:");
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
		if (D_CG0)
			AddCap("G0:");
		if (D_CC0 || (D_CS0 && *D_CS0)) {
			AddCap("as=\\E(0:");
			AddCap("ae=\\E(B:");
			/* avoid `` because some shells dump core... */
			AddCap("ac=\\140\\140aaffggjjkkllmmnnooppqqrrssttuuvvwwxxyyzz{{||}}~~..--++,,hhII00:");
		}
		if (D_PO) {
			AddCap("po=\\E[5i:");
			AddCap("pf=\\E[4i:");
		}
		if (D_CZ0) {
			AddCap("Z0=\\E[?3h:");
			AddCap("Z1=\\E[?3l:");
		}
		if (D_CWS)
			AddCap("WS=\\E[8;%d;%dt:");
	}
	for (i = T_CAPS; i < T_ECAPS; i++) {
		struct action *act;
		if (i < T_OCAPS) {
			if (i >= T_KEYPAD)	/* don't put keypad codes in TERMCAP */
				continue;	/* - makes it too big */
#if (TERMCAP_BUFSIZE < 1024)
			if (i >= T_FEXTRA && i < T_BACKTAB) /* also skip extra vt220 keys */
				continue;
			if (i > T_BACKTAB && i < T_NAVIGATE) /* more vt220 keys */
				continue;
#endif
			if (i >= T_CURSOR && i < T_OCAPS) {
				act = &umtab[i - (T_CURSOR - T_OCAPS + T_CAPS)];
				if (act->nr == RC_ILLEGAL)
					act = &dmtab[i - (T_CURSOR - T_OCAPS + T_CAPS)];
			} else {
				act = &umtab[i - T_CAPS];
				if (act->nr == RC_ILLEGAL)
					act = &dmtab[i - T_CAPS];
			}
			if (act->nr == RC_ILLEGAL && (i == T_NAVIGATE + 1 || i == T_NAVIGATE + 3)) {
				/* kh -> @1, kH -> @7 */
				act = &umtab[i - T_CAPS - 1];
				if (act->nr == RC_ILLEGAL)
					act = &dmtab[i - T_CAPS - 1];
			}
			if (act->nr != RC_ILLEGAL) {
				if (act->nr == RC_STUFF) {
					MakeString(term[i].tcname, buf, ARRAY_SIZE(buf), act->args[0]);
					AddCap(buf);
				}
				continue;
			}
		}
		if (display == NULL)
			continue;
		switch (term[i].type) {
		case T_STR:
			if (D_tcs[i].str == NULL)
				break;
			MakeString(term[i].tcname, buf, ARRAY_SIZE(buf), D_tcs[i].str);
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
	return Termcap;
}

#define TERMCAP_MAX_WIDTH 63
void DumpTermcap(int aflag, FILE *f)
{
	const char *p, *pe;
	int n, col = 0;

	if ((p = index(MakeTermcap(aflag), '=')) == NULL)
		return;
	p++;
	/* write termcap entry with wrapping */
	while ((pe = index(p, ':')))
	{
		n = pe - p + 1;
		if ((col > 8) && ((col + n) > TERMCAP_MAX_WIDTH))
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

static void MakeString(char *cap, char *buf, int buflen, char *s)
{
	char *p, *pmax;
	unsigned int c;

	p = buf;
	pmax = p + buflen - (3 + 4 + 2);
	*p++ = *cap++;
	*p++ = *cap;
	*p++ = '=';
	while ((c = *s++) && (p < pmax)) {
		switch (c) {
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
			if (c >= 200) {
				sprintf(p, "\\%03o", c & 0377);
				p += 4;
			} else if (c < ' ') {
				*p++ = '^';
				*p++ = c + '@';
			} else
				*p++ = c;
		}
	}
	*p++ = ':';
	*p = '\0';
}

#undef QUOTES
#define QUOTES(p) \
  (*p == '\\' && (p[1] == '\\' || p[1] == ',' || p[1] == '%'))

int CreateTransTable(char *s)
{
	int curchar;
	char *templ, *arg;
	int templlen;
	int templnsub;
	char *p, *sx;
	char **ctable;
	int l, c;

	if ((D_xtable = calloc(256, sizeof(char **))) == NULL) {
		Msg(0, "%s", strnomem);
		return -1;
	}

	while (*s) {
		if (QUOTES(s))
			s++;
		curchar = (unsigned char)*s++;
		if (curchar == 'B')
			curchar = 0;	/* ASCII */
		templ = s;
		templlen = 0;
		templnsub = 0;
		if (D_xtable[curchar] == NULL) {
			if ((D_xtable[curchar] = calloc(257, sizeof(char *))) == NULL) {
				Msg(0, "%s", strnomem);
				FreeTransTable();
				return -1;
			}
		}
		ctable = D_xtable[curchar];
		for (; *s && *s != ','; s++) {
			if (QUOTES(s))
				s++;
			else if (*s == '%') {
				templnsub++;
				continue;
			}
			templlen++;
		}
		if (*s++ == 0)
			break;
		while (*s && *s != ',') {
			c = (unsigned char)*s++;
			if (QUOTES((s - 1)))
				c = (unsigned char)*s++;
			else if (c == '%')
				c = 256;
			if (ctable[c])
				free(ctable[c]);
			arg = s;
			l = copyarg(&s, NULL);
			if (c != 256)
				l = l * templnsub + templlen;
			if ((ctable[c] = malloc(l + 1)) == NULL) {
				Msg(0, "%s", strnomem);
				FreeTransTable();
				return -1;
			}
			sx = ctable[c];
			for (p = ((c == 256) ? "%" : templ); *p && *p != ','; p++) {
				if (QUOTES(p))
					p++;
				else if (*p == '%') {
					s = arg;
					sx += copyarg(&s, sx);
					continue;
				}
				*sx++ = *p;
			}
			*sx = 0;
		}
		if (*s == ',')
			s++;
	}
	return 0;
}

void FreeTransTable(void)
{
	char ***p, **q;
	int i, j;

	if ((p = D_xtable) == NULL)
		return;
	for (i = 0; i < 256; i++, p++) {
		if (*p == NULL)
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

static int copyarg(char **pp, char *s)
{
	int l;
	char *p;

	for (l = 0, p = *pp; *p && *p != ','; p++) {
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

static int e_tgetent(char *bp, char *name)
{
	int r;

	xseteuid(real_uid);
	xsetegid(real_gid);
	r = tgetent(bp, name);
	xseteuid(eff_uid);
	xsetegid(eff_gid);
	return r;
}

/* findcap:
 *   cap = capability we are looking for
 *   tepp = pointer to bufferpointer
 *   n = size of buffer (0 = infinity)
 */

static char *findcap(char *cap, char **tepp, int n)
{
	char *tep;
	char c, *p, *cp;
	int mode;		/* mode: 0=LIT  1=^  2=\x  3,4,5=\nnn */
	int num = 0, capl;

	if (!extra_incap)
		return NULL;
	tep = *tepp;
	capl = strlen(cap);
	cp = NULL;
	mode = 0;
	for (p = extra_incap; *p;) {
		if (strncmp(p, cap, capl) == 0) {
			p += capl;
			c = *p;
			if (c && c != ':' && c != '@')
				p++;
			if (c == 0 || c == '@' || c == '=' || c == ':' || c == '#')
				cp = tep;
		}
		while ((c = *p)) {
			p++;
			if (mode == 0) {
				if (c == ':')
					break;
				if (c == '^')
					mode = 1;
				if (c == '\\')
					mode = 2;
			} else if (mode == 1) {
				mode = 0;
				c = c & 0x1f;
			} else if (mode == 2) {
				mode = 0;
				switch (c) {
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
			if (mode > 2) {
				num = num * 8 + (c - '0');
				if (mode++ == 5 || (*p < '0' || *p > '9')) {
					c = num;
					mode = 0;
				}
			}
			if (mode)
				continue;

			if (cp && n != 1) {
				*cp++ = c;
				n--;
			}
		}
		if (cp) {
			*cp++ = 0;
			*tepp = cp;
			return tep;
		}
	}
	return NULL;
}

static char *e_tgetstr(char *cap, char **tepp)
{
	char *tep;
	if ((tep = findcap(cap, tepp, 0)))
		return (*tep == '@') ? NULL : tep;
	return tgetstr(cap, tepp);
}

static int e_tgetflag(char *cap)
{
	char buf[2], *bufp;
	char *tep;
	bufp = buf;
	if ((tep = findcap(cap, &bufp, 2)))
		return (*tep == '@') ? 0 : 1;
	return tgetflag(cap) > 0;
}

static int e_tgetnum(char *cap)
{
	char buf[20], *bufp;
	char *tep, c;
	int res, base = 10;

	bufp = buf;
	if ((tep = findcap(cap, &bufp, 20))) {
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
