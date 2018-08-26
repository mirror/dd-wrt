% A simple emacs-emulation for SLrline

require ("rline/complete");
require ("rline/editfuns");

rline_unsetkey ("^@");
foreach $1 ([1:255])
{
   $2 = char (-$1);		       %  byte-semantics
   rline_unsetkey ($2);
   rline_setkey ("self_insert", $2);
}

rline_setkey ("bdel",			"^?");
rline_setkey ("bol",			"^A");
rline_setkey ("left",			"^B");
rline_setkey ("kbd_quit",		"^C");
rline_setkey ("del",			"^D");
rline_setkey ("eol",			"^E");
rline_setkey ("right",			"^F");
rline_setkey ("kbd_quit",		"^G");
rline_setkey ("bdel",			"^H");
rline_setkey ("enter",			"\n");
rline_setkey (&rline_kill_eol,		"^K");
rline_setkey ("redraw",			"^L");
rline_setkey ("enter",			"\r");
rline_setkey ("down",			"^N");
rline_setkey ("up",			"^P");
rline_setkey ("quoted_insert",		"^Q");
rline_setkey (&rline_kill_bol,		"^U");
rline_setkey (&rline_kill_region,	"^W");
rline_setkey (&rline_yank,		"^Y");
rline_setkey ("trim",			"\e\\");
rline_setkey (&rline_bskip_word,	"\eb");
rline_setkey (&rline_skip_word,		"\ef");
rline_setkey (&rline_copy_region,	"\ew");
rline_setkey ("complete",		"\t");

#ifdef UNIX VMS
rline_setkey (&rline_set_mark,		"^@");
rline_setkey ("up",			"\e[A");
rline_setkey ("down",			"\e[B");
rline_setkey ("right",			"\e[C");
rline_setkey ("left",			"\e[D");
rline_setkey ("up",			"\eOA");
rline_setkey ("down",			"\eOB");
rline_setkey ("right",			"\eOC");
rline_setkey ("left",			"\eOD");
#else
rline_setkey (&rline_set_mark,		"^@^C");
rline_setkey ("up",			"^@H");
rline_setkey ("down",			"^@P");
rline_setkey ("right",			"^@M");
rline_setkey ("left",			"^@K");
rline_setkey ("del",			"^@S");
rline_setkey ("eol",			"^@O");
rline_setkey ("bol",			"^@G");

rline_setkey ("up",			"\xE0H");
rline_setkey ("down",			"\xE0P");
rline_setkey ("right",			"\xE0M");
rline_setkey ("left",			"\xE0K");
rline_setkey ("del",			"\xE0S");
rline_setkey ("eol",			"\xE0O");
rline_setkey ("bol",			"\xE0G");
#endif

