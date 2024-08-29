/* Copyright 1998 by Andi Kleen. Subject to the GPL. */
/* rewritten by bernd eckenfels because of complicated alias semantic */
/* $Id: nstrcmp.c,v 1.4 2004/06/03 22:49:17 ecki Exp $ */
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include "util.h"


/* return numerical :999 suffix or null. sideeffect: replace ':' with \0 */
char* cutalias(char* name)
{
	int digit = 0;
	int pos;

	for(pos=strlen(name); pos>0; pos--)
	{
		if (name[pos-1]==':' && digit)
		{
			name[pos-1]='\0';
			return name+pos;
		}
		if (!isdigit(name[pos-1]))
			break;
		digit = 1;
	}
	return NULL;
}


/* return index of last non digit or -1 if it does not end with digits */
int rindex_nondigit(char *name)
{
	int pos = strlen(name);

	for(pos=strlen(name); pos>0; pos--)
	{
		if (!isdigit(name[pos-1]))
			return pos;
	}
	return 0;
}


/* like strcmp(), but knows about numbers and ':' alias suffix */
int nstrcmp(const char *ap, const char *bp)
{
	char *a = xstrdup(ap);
	char *b = xstrdup(bp);
	char *an, *bn;
	int av = 0, bv = 0;
	char *aalias=cutalias(a);
	char *balias=cutalias(b);
	int aindex=rindex_nondigit(a);
	int bindex=rindex_nondigit(b);
	int complen=(aindex<bindex)?aindex:bindex;
	int res = strncmp(a, b, complen);

	if (res != 0) {
		goto out;
	}

	if (aindex > bindex) {
		res = 1;
		goto out;
	}

	if (aindex < bindex) {
		res = -1;
		goto out;
	}

	an = a+aindex;
	bn = b+bindex;

	av = atoi(an);
	bv = atoi(bn);

	if (av < bv) {
		res = -1;
		goto out;
	}

	if (av > bv) {
		res = 1;
		goto out;
	}

	res = strcmp(a, b);
	if (res != 0) {
		goto out;
	}

	av = -1;
	if (aalias != NULL)
		av = atoi(aalias);

	bv = -1;
	if (balias != NULL)
		bv = atoi(balias);

	if (av < bv) {
		res = -1;
		goto out;
	}

	if (av > bv) {
		res = 1;
		goto out;
	}

	if (aalias && balias) {
		res = strcmp(aalias, balias);
	}

out:

	free(a); free(b);

	return res;
}


#ifdef NSTRCMP_TEST

int cs(int s)
{
	if (s < 0) return -1;
	if (s > 0) return 1;
	return 0;
}


int dotest(char* a, char* b, int exp)
{
	int res = nstrcmp(a, b);
	int err = (cs(res) != cs(exp));
	printf("nstrcmp(\"%s\", \"%s\")=%d %d %s\n", a, b, res, exp, err?"WRONG":"OK");
	return err;
}

int main()
{
	int err = 0;

	err |= dotest("eth1", "eth1", 0);
	err |= dotest("eth0:1", "eth0:1", 0);
	err |= dotest("lan", "lan", 0);
	err |= dotest("100", "100", 0);
	err |= dotest("", "", 0);
	err |= dotest(":", ":", 0);
	err |= dotest("a:b:c", "a:b:c", 0);
	err |= dotest("a:", "a:", 0);
	err |= dotest(":a", ":a", 0);

	err |= dotest("a", "aa", -1);
	err |= dotest("eth0", "eth1", -1);
	err |= dotest("eth1", "eth20", -1);
	err |= dotest("eth20", "eth100", -1);
	err |= dotest("eth1", "eth13", -1);
	err |= dotest("eth", "eth2", -1);
	err |= dotest("eth0:1", "eth0:2", -1);
	err |= dotest("eth1:10", "eth13:10", -1);
	err |= dotest("eth1:1", "eth1:13", -1);
	err |= dotest("a", "a:", -1);

	err |= dotest("aa", "a", 1);
	err |= dotest("eth2", "eth1", 1);
	err |= dotest("eth13", "eth1", 1);
	err |= dotest("eth2", "eth", 1);
	err |= dotest("eth2:10", "eth2:1", 1);
	err |= dotest("eth2:5", "eth2:4", 1);
	err |= dotest("eth3:2", "eth2:3", 1);
	err |= dotest("eth13:1", "eth1:0", 1);
	err |= dotest("a:", "a", 1);
	err |= dotest("a1b12", "a1b2", 1);

	err |= dotest("eth1", "eth01", 1);
	err |= dotest("eth01", "eth1", -1);
	err |= dotest("eth1:1", "eth01:1", 1);
	err |= dotest("eth01:1", "eth1:1", -1);
	err |= dotest("eth1:1", "eth01:01", 1);
	err |= dotest("eth1:01", "eth01:1", 1);
	err |= dotest("eth01:1", "eth1:01", -1);
	err |= dotest("eth01:01", "eth1:1", -1);

	return err;
}

#endif
