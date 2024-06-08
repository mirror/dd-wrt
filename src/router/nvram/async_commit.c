#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <bcmnvram.h>

int main(int argc, char *argv[])
{
#if defined(HAVE_WZRHPG300NH) || defined(HAVE_WHRHPGN) || defined(HAVE_WZRHPAG300NH) || defined(HAVE_DIR825) || defined(HAVE_TEW632BRP) || defined(HAVE_TG2521) || defined(HAVE_WR1043)  || defined(HAVE_WRT400) || defined(HAVE_WZRHPAG300NH) || defined(HAVE_WZRG450) || defined(HAVE_DANUBE) || defined(HAVE_WR741) || defined(HAVE_NORTHSTAR) || defined(HAVE_DIR615I) || defined(HAVE_WDR4900) || defined(HAVE_VENTANA) || defined(HAVE_UBNTM) || defined(HAVE_IPQ806X) || defined(HAVE_IPQ6018)
	system("ledtool 1");
#elif HAVE_LSX
	//nothing
#elif HAVE_XSCALE
	//nothing
#else
	system("ledtool 1");
#endif

	/* 
	 * Run it under background 
	 */
	switch (fork()) {
	case -1:
		perror("fork failed");
		exit(1);
		break;
	case 0:
		/* 
		 * child process 
		 */
		(void)setsid();
		break;
	default:
		/* 
		 * parent process should just die 
		 */
		_exit(0);
	}
	_nvram_commit();
	return 0;
}
