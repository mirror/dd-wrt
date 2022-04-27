#! /bin/sh
#
# Copyright (c) 2002-2006 Sam Leffler, Errno Consulting
# Copyright (c) 2002-2006 Atheros Communications, Inc.
# All rights reserved.
#
# $Id: //depot/sw/branches/sam_hal/regdomain/cmp.sh#1 $
#
# Shell script to compare the output of regdomain against
# the Atheros "golden file" for a known set of regulatory
# domains.  By default we compare the channel sets and the
# calculated transmit poewr for each channel.  If you specify
# a "-n" option then only the channel sets are compared.
#
ATHEROS=golden-20040207.log		# golden file
OPTS="-o"			# options to regdomain

if [ $1"x" = "-x" ]; then
	DOTXPOWER="yes";
	shift;
else
	DOTXPOWER="no";
fi

COUNTRIES=${@:-"
	DB NA AL DZ AR AM AU AT AZ BH BY BE BZ BO BR BN BG
	CA CL CN CO CR HR CY CZ DK DO EC EG SV EE FI FR GE
	DE GR GT HN HK HU IS IN ID IR IE IL IT JP J1 J2 J3
	J4 J5 JO KZ KP KR K2 KW LV LB LI LT LU MO MK MY MX
	MC MA NL NZ NO OM PK PA PE PH PL PT PR QA RO RU SA
	SG SK SI ZA ES SE CH SY TW TH TT TN TR UA AE GB US
	UY UZ VE VN YE ZW
"}

TEMPS="atheros.cmp madwifi.cmp"
trap "rm -f ${TEMPS}" 0

(for i in $COUNTRIES
do
	./regdomain $OPTS $i
done) | awk -v dotxpower=${DOTXPOWER} '
BEGIN		{ IFS = "[ \t]"; }
/^Weird/	{ next; }
/^[A-Z]+/	{ i = index($0, " (");
		  cc_full = substr($0, 1, i-1);
		  cc = substr($0, i+2, 2);
		  i = index($0, ")");
		  rd = substr($0, i+2);
		  sub(" .*", "", rd);
		  if (cc_full == "FCC1_FCCA") {
			cc = "NA";
			rd = cc_full;
		  }
		}
/^[0-9]+/	{ for (i = 1; i <= NF; i += 2) {
		      if (dotxpower == "yes")
			  print cc " " $i " " $(i+1);
		      else
			  print cc " " $i;
		  }
		}
END		{ }
' > madwifi.cmp

cat $ATHEROS | tr -d "\r" | awk -v dotxpower=${DOTXPOWER} '
function doit(start, a, na)
{
    for (i = start; i <= NF; i++)
	a[na++] = $i;
    return na;
}

#
# Sort array a[l..r]
#
function qsort(a, l, r) {
    i = l;
    k = r+1;
    item = a[l];
    for (;;) {
	while (i < r) {
            i++;
	    if (a[i] >= item)
		break;
        }
	while (k > l) {
            k--;
	    if (a[k] <= item)
		break;
        }
        if (i >= k)
	    break;
	t = a[i]; a[i] = a[k]; a[k] = t;
    }
    t = a[l]; a[l] = a[k]; a[k] = t;
    if (k != 0 && l < k-1)
	qsort(a, l, k-1);
    if (k+1 < r)
	qsort(a, k+1, r);
}

function merge(a1, n1, a2, n2)
{
    for (i = 0; i < n1; i++)
	for (j = 0; j < n2; j++) {
	    if (a1[i] == a2[j]) {
		for (; j < n2-1; j++)
		   a2[j] = a2[j+1];
		n2--;
		break;
	    }
	    if (a1[i] > a2[j])
		break;
	}
    return n2;
}

function dump()
{
    if (cc != "") {
	if (nB > 0) {
	    qsort(modeB, 0, nB-1);
	    if (nG > 0) {
		qsort(modeG, 0, nG-1);
		nB = merge(modeG, nG, modeB, nB);
	    }
	}
	if (nA > 0) {
	    qsort(modeA, 0, nA-1);
	    if (nT > 0) {
		qsort(modeT, 0, nT-1);
		nA = merge(modeT, nT, modeA, nA);
	    }
	}
	if (nB > 0) {
	    for (i = 0; i < nB; i++)
		print cc " " modeB[i] "B";
	    nB = 0;
	}
	if (nG > 0) {
	    for (i = 0; i < nG; i++)
		print cc " " modeG[i] "G";
	    nG = 0;
	}
	if (nA > 0) {
	    for (i = 0; i < nA; i++)
		print cc " " modeA[i] "A";
	    nA = 0;
	}
	if (nT > 0) {
	    for (i = 0; i < nT; i++)
		print cc " " modeT[i] "T";
	    nT = 0;
	}
    }
    nTS = nTD = nGS = nGD = 0;
}

BEGIN		{ IFS = "[ \t]"; cc = ""; }
/^Weird/	{ next; }
/^Channel/	{ dump(); cc = $NF; }
/^11a:/		{ mode = "A"; nA = doit(2, modeA, nA); }
/^11b:/		{ mode = "B"; nB = doit(2, modeB, nB); }
/^11g:/		{ mode = "G"; nG = doit(2, modeG, nG); }
/^turbo:/	{ mode = "T"; nT = doit(2, modeT, nT); }
/^turbo static:/{ mode = "TS"; nTS = doit(3,modeTS, nTS); }
/^turbo dynamic:/{ mode = "TD"; nTD = doit(3,modeTD, nTD); }
/^108g static:/	{ mode = "GS"; nGS = doit(3, modeGS, nGS); }
/^108g dynamic:/{ mode = "GD"; nGD = doit(3, modeGD, nGD); }
/^[ \t]+[25][0-9]+/ {
		  if (mode == "A")
			nA = doit(1, modeA, nA);
		  else if (mode == "B")
			nB = doit(1, modeB, nB);
		  else if (mode == "G")
			nG = doit(1, modeG, nG);
		  else if (mode == "T")
			nT = doit(1, modeT, nT);
		  else if (mode == "TS")
			nTS = doit(1, modeTS,nTS);
		  else if (mode == "TD")
			nTD = doit(1, modeTD,nTD);
		  else if (mode == "GS")
			nS = doit(1, modeGS, nGS);
		  else if (mode == "GD")
			nS = doit(1, modeGD, nGD);
		}
END		{ dump(); }
' > atheros.cmp
diff atheros.cmp madwifi.cmp
