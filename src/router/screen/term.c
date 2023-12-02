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

#include "term.h"

#define KMAPDEF(s)
#define KMAPADEF(s)
#define KMAPMDEF(s)

struct term term[T_N] =
{
/* display size */
  { "li", T_NUM  },
  { "co", T_NUM  },

/* terminal types*/
  { "hc", T_FLG  },
  { "os", T_FLG  },
  { "ns", T_FLG  },
/* cursor movement */
  { "cm", T_STR  },
  { "ho", T_STR  },
  { "cr", T_STR  },
  { "up", T_STR  },
  { "UP", T_STR  },
  { "do", T_STR  },
  { "DO", T_STR  },
  { "bs", T_FLG  },
  { "bc", T_STR  },
  { "le", T_STR  },
  { "LE", T_STR  },
  { "nd", T_STR  },
  { "RI", T_STR  },

/* scroll */
  { "cs", T_STR  },
  { "nl", T_STR  },
  { "sf", T_STR  },
  { "sr", T_STR  },
  { "al", T_STR  },
  { "AL", T_STR  },
  { "dl", T_STR  },
  { "DL", T_STR  },

/* insert/delete */
  { "in", T_FLG  },
  { "im", T_STR  },
  { "ei", T_STR  },
  { "ic", T_STR  },
  { "IC", T_STR  },
  { "dc", T_STR  },
  { "DC", T_STR  },

/* erase */
  { "ut", T_FLG  },
  { "cl", T_STR  },
  { "cd", T_STR  },
  { "CD", T_STR  },
  { "ce", T_STR  },
  { "cb", T_STR  },

/* initialise */
  { "is", T_STR  },
  { "ti", T_STR  },
  { "te", T_STR  },

/* bell */
  { "bl", T_STR  },
  { "vb", T_STR  },

/* resizing */
  { "WS", T_STR  },
  { "Z0", T_STR  },
  { "Z1", T_STR  },

/* attributes */
/* define T_ATTR */
  { "mh", T_STR  },
  { "us", T_STR  },
  { "md", T_STR  },
  { "mr", T_STR  },
  { "so", T_STR  },
  { "mb", T_STR  },
  { "ue", T_STR  },
  { "se", T_STR  },
  { "me", T_STR  },
  { "ms", T_FLG  },
  { "sg", T_NUM  },
  { "ug", T_NUM  },
  { "sa", T_STR  },

/* color */
  { "AF", T_STR  },
  { "AB", T_STR  },
  { "Sf", T_STR  },
  { "Sb", T_STR  },
  { "op", T_STR  },
  { "Co", T_NUM  },
  { "be", T_FLG  },
  { "AX", T_FLG  },
  { "C8", T_FLG  },

/* keypad/cursorkeys */
  { "ks", T_STR  },
  { "ke", T_STR  },
  { "CS", T_STR  },
  { "CE", T_STR  },

/* printer */
  { "po", T_STR  },
  { "pf", T_STR  },

/* status line */
  { "hs", T_FLG  },
  { "ws", T_NUM  },
  { "ts", T_STR  },
  { "fs", T_STR  },
  { "ds", T_STR  },

/* cursor visibility */
  { "vi", T_STR  },
  { "vs", T_STR  },
  { "ve", T_STR  },

/* margin handling */
  { "am", T_FLG  },
  { "xv", T_FLG  },
  { "xn", T_FLG  },
  { "OP", T_FLG  },
  { "LP", T_FLG  },

/* special settings */
  { "NF", T_FLG  },
  { "nx", T_FLG  },
  { "AN", T_FLG  },
  { "OL", T_NUM  },
  { "KJ", T_STR  },
  { "VR", T_STR  },
  { "VN", T_STR  },
  { "TF", T_FLG  },
  { "XT", T_FLG  },

/* d_font setting */
  { "G0", T_FLG  },
  { "S0", T_STR  },
  { "E0", T_STR  },
  { "C0", T_STR  },
  { "as", T_STR  },
  { "ae", T_STR  },
  { "ac", T_STR  },
  { "eA", T_STR  },
  { "XC", T_STR  },

/* keycaps */
/* define T_CAPS */

/* mouse */
  { "Km", T_STR  }, KMAPDEF("\033[M")		KMAPMDEF("\222")

/* nolist */
  { "k0", T_STR  }, KMAPDEF("\033[10~")
  { "k1", T_STR  }, KMAPDEF("\033OP")
  { "k2", T_STR  }, KMAPDEF("\033OQ")
  { "k3", T_STR  }, KMAPDEF("\033OR")
  { "k4", T_STR  }, KMAPDEF("\033OS")
  { "k5", T_STR  }, KMAPDEF("\033[15~")
  { "k6", T_STR  }, KMAPDEF("\033[17~")
  { "k7", T_STR  }, KMAPDEF("\033[18~")
  { "k8", T_STR  }, KMAPDEF("\033[19~")
  { "k9", T_STR  }, KMAPDEF("\033[20~")
  { "k;", T_STR  }, KMAPDEF("\033[21~")
  { "F1", T_STR  }, KMAPDEF("\033[23~")
  { "F2", T_STR  }, KMAPDEF("\033[24~")
  /* extra keys for vt220 (David.Leonard@it.uq.edu.au) */
/* define T_FEXTRA */
  { "F3", T_STR  },
  { "F4", T_STR  },
  { "F5", T_STR  },
  { "F6", T_STR  },
  { "F7", T_STR  },
  { "F8", T_STR  },
  { "F9", T_STR  },
  { "FA", T_STR  },
  { "FB", T_STR  },
  { "FC", T_STR  },
  { "FD", T_STR  },
  { "FE", T_STR  },

  { "kb", T_STR  },
  { "K1", T_STR  },
  { "K2", T_STR  },
  { "K3", T_STR  },
  { "K4", T_STR  },
  { "K5", T_STR  },
/* more keys for Andrew A. Chernov (ache@astral.msk.su) */
  { "kA", T_STR  },
  { "ka", T_STR  },
/* define T_BACKTAB */
  { "kB", T_STR  },
  { "kC", T_STR  },
  { "kE", T_STR  },
  { "kF", T_STR  },                    KMAPMDEF("\004")
  { "kL", T_STR  },
  { "kM", T_STR  },
  { "kR", T_STR  },                    KMAPMDEF("\025")
  { "kS", T_STR  },
  { "kT", T_STR  },
  { "kt", T_STR  },
  { "*4", T_STR  },	/* kDC */
  { "*7", T_STR  },	/* kEND */
  { "#2", T_STR  },	/* kHOM */
  { "#3", T_STR  },	/* kIC */
  { "#4", T_STR  },	/* kLFT */
  { "%c", T_STR  },	/* kNXT */
  { "%e", T_STR  },	/* kPRV */
  { "%i", T_STR  },	/* kRIT */

/* keys above the cursor */
/* define T_NAVIGATE */
  { "kh", T_STR  }, KMAPDEF("\033[1~") KMAPMDEF("\201")
  { "@1", T_STR  },
  { "kH", T_STR  }, KMAPDEF("\033[4~") KMAPMDEF("\205")
  { "@7", T_STR  }, KMAPDEF("\033[4~") KMAPMDEF("\205")
  { "kN", T_STR  }, KMAPDEF("\033[6~") KMAPMDEF("\006")
  { "kP", T_STR  }, KMAPDEF("\033[5~") KMAPMDEF("\002")
  { "kI", T_STR  }, KMAPDEF("\033[2~")
/* define T_NAVIGATE_DELETE */
  { "kD", T_STR  }, KMAPDEF("\033[3~")

/* keys that can have two bindings */
/* define T_CURSOR */
  { "ku", T_STR  }, KMAPDEF("\033[A") KMAPADEF("\033OA") KMAPMDEF("\220")
  { "kd", T_STR  }, KMAPDEF("\033[B") KMAPADEF("\033OB") KMAPMDEF("\216")
  { "kr", T_STR  }, KMAPDEF("\033[C") KMAPADEF("\033OC") KMAPMDEF("\206")
  { "kl", T_STR  }, KMAPDEF("\033[D") KMAPADEF("\033OD") KMAPMDEF("\202")
/* define T_KEYPAD */
  { "f0", T_STR  }, KMAPDEF("0") KMAPADEF("\033Op")
  { "f1", T_STR  }, KMAPDEF("1") KMAPADEF("\033Oq")
  { "f2", T_STR  }, KMAPDEF("2") KMAPADEF("\033Or")
  { "f3", T_STR  }, KMAPDEF("3") KMAPADEF("\033Os")
  { "f4", T_STR  }, KMAPDEF("4") KMAPADEF("\033Ot")
  { "f5", T_STR  }, KMAPDEF("5") KMAPADEF("\033Ou")
  { "f6", T_STR  }, KMAPDEF("6") KMAPADEF("\033Ov")
  { "f7", T_STR  }, KMAPDEF("7") KMAPADEF("\033Ow")
  { "f8", T_STR  }, KMAPDEF("8") KMAPADEF("\033Ox")
  { "f9", T_STR  }, KMAPDEF("9") KMAPADEF("\033Oy")
  { "f+", T_STR  }, KMAPDEF("+") KMAPADEF("\033Ok")
  { "f-", T_STR  }, KMAPDEF("-") KMAPADEF("\033Om")
  { "f*", T_STR  }, KMAPDEF("*") KMAPADEF("\033Oj")
  { "f/", T_STR  }, KMAPDEF("/") KMAPADEF("\033Oo")
  { "fq", T_STR  }, KMAPDEF("=") KMAPADEF("\033OX")
  { "f.", T_STR  }, KMAPDEF(".") KMAPADEF("\033On")
  { "f,", T_STR  }, KMAPDEF(",") KMAPADEF("\033Ol")
  { "fe", T_STR  }, KMAPDEF("\015") KMAPADEF("\033OM")
/* other things related to keycaps */
/* define T_OCAPS */
  { "km", T_FLG  },
  { "ko", T_STR  },
  { "l0", T_STR  },
  { "l1", T_STR  },
  { "l2", T_STR  },
  { "l3", T_STR  },
  { "l4", T_STR  },
  { "l5", T_STR  },
  { "l6", T_STR  },
  { "l7", T_STR  },
  { "l8", T_STR  },
  { "l9", T_STR  },
  { "la", T_STR  },
/* list */
/* define T_ECAPS */
/* define T_N */
};
