#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include "../config.h"


extern int mchilli_main(int argc, char **argv);
extern int mopt_main(int argc, char **argv);
extern int mproxy_main(int argc, char **argv);
extern int mquery_main(int argc, char **argv);
extern int mradconfig_main(int argc, char **argv);
extern int mresponse_main(int argc, char **argv);
extern int mradsec_main(int argc, char **argv);

int main(int argc, char **argv)
{
	const char *prog = argv[0];
	if (strstr(prog, "chilli_opt"))
		return mopt_main(argc, argv);
	else if (strstr(prog, "chilli_query"))
		return mquery_main(argc, argv);
	else if (strstr(prog, "chilli_radconfig"))
		return mradconfig_main(argc, argv);
	else if (strstr(prog, "chilli_response"))
		return mresponse_main(argc, argv);

	#if defined(ENABLE_CHILLIPROXY)
	else if (strstr(prog, "chilli_proxy"))
		return mproxy_main(argc, argv);
	#endif
	#if defined(ENABLE_CHILLIRADSEC)
	else if (strstr(prog, "chilli_radsec"))
		return mradsec_main(argc, argv);
	#endif
	#if defined(ENABLE_CHILLIREDIR)
	else if (strstr(prog, "chilli_redir"))
		return mredir_main(argc, argv);
	#endif
	#if defined(ENABLE_MULTIROUTE)
	else if (strstr(prog, "chilli_rtmon"))
		return mrtmon_main(argc, argv);
	#endif
	#if defined(ENABLE_CHILLISCRIPT)
	else if (strstr(prog, "chilli_script"))
		return mscript_main(argc, argv);
	#endif
	else if (strstr(prog, "chilli"))
		return mchilli_main(argc, argv);

	fprintf(stderr, "Bad programm %s\n", prog);
	return 255;
}
