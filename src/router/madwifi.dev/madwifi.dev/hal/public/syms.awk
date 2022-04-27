#
# Copyright (c) 2002-2004 Sam Leffler, Errno Consulting
# Copyright (c) 2002-2004 Atheros Communications, Inc.
# All rights reserved.
#
# $Id: //depot/sw/releases/linuxsrc/src/802_11/madwifi/hal/main/public/syms.awk#3 $
#
func hash(s)
{
	h = 1;
	for (i = 1; i <= length(s); i++)
		h = (h*2) + ascii[substr(s,i,1)];
	hsym = sprintf("DDWRT%08x", h % 268435455);
	return hsym;
}

BEGIN	{ IFS=" ";
	  ascii["a"] = 97;	ascii["A"] = 65;
	  ascii["b"] = 98;	ascii["B"] = 66;
	  ascii["c"] = 99;	ascii["C"] = 67;
	  ascii["d"] = 100;	ascii["D"] = 68;
	  ascii["e"] = 101;	ascii["E"] = 69;
	  ascii["f"] = 102;	ascii["F"] = 70;
	  ascii["g"] = 103;	ascii["G"] = 71;
	  ascii["h"] = 104;	ascii["H"] = 72;
	  ascii["i"] = 105;	ascii["I"] = 73;
	  ascii["j"] = 106;	ascii["J"] = 74;
	  ascii["k"] = 107;	ascii["K"] = 75;
	  ascii["l"] = 108;	ascii["L"] = 76;
	  ascii["m"] = 109;	ascii["M"] = 77;
	  ascii["n"] = 110;	ascii["N"] = 78;
	  ascii["o"] = 111;	ascii["O"] = 79;
	  ascii["p"] = 112;	ascii["P"] = 80;
	  ascii["q"] = 113;	ascii["Q"] = 81;
	  ascii["r"] = 114;	ascii["R"] = 82;
	  ascii["s"] = 115;	ascii["S"] = 83;
	  ascii["t"] = 116;	ascii["T"] = 84;
	  ascii["u"] = 117;	ascii["U"] = 85;
	  ascii["v"] = 118;	ascii["V"] = 86;
	  ascii["w"] = 119;	ascii["W"] = 87;
	  ascii["x"] = 120;	ascii["X"] = 88;
	  ascii["y"] = 121;	ascii["Y"] = 89;
	  ascii["z"] = 122;	ascii["Z"] = 90;

	  ascii["_"] = 95;	ascii["."] = 46;

	  ascii["0"] = 48;	ascii["1"] = 49;
	  ascii["2"] = 50;	ascii["3"] = 51;
	  ascii["4"] = 52;	ascii["5"] = 53;
	  ascii["6"] = 54;	ascii["7"] = 55;
	  ascii["8"] = 56;	ascii["9"] = 57;
	}
$3 ~ /^[_a-zA-Z]/	{
	sym = $3;
	hsym = hash(sym);
	if (hsym in symbols) {
		if (sym != symbols[hsym]) {
			print s " collides with " symbols[hsym] " (" hsym ")" > "/dev/stderr"
			h = 1;
			for (i = 1; i <= length(s); i++) {
				h = (h*2) + ascii[substr(s,i,1)];
				print i ": " substr(s,i,1) " " h > "/dev/stderr";
			}
		}
	} else {
		symbols[hsym] = sym;
		printf "--redefine-sym %s=%s\n", sym, hsym;
	}
}
