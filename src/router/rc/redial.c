#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <errno.h>
#include <shutils.h>

#define start_service(a) eval("startservice", a);
#define start_service_force(a) eval("startservice", a, "-f");
#define start_service_f(a) eval("startservice_f", a);
#define start_service_force_f(a) eval("startservice_f", a, "-f");
#define start_services() eval("startservices");
#define stop_service(a) eval("stopservice", a);
#define stop_service_force(a) eval("stopservice", "-f", a);
#define stop_running(a) eval("stop_running");
#define stop_service_f(a) eval("stopservice_f", a);
#define stop_service_force_f(a) eval("stopservice_f", a, "-f");
#define stop_services() eval("stopservices");
#define restart(a) eval("restart", a);
#define restart_f(a) eval("restart_f", a);

int main(int argc, char **argv)
{
	switch (fork()) {
	case -1:
		// can't fork
		exit(0);
		break;
	case 0:
		/* 
		 * child process 
		 */
		// fork ok
		(void)setsid();
		break;
	default:
		/* 
		 * parent process should just die 
		 */
		_exit(0);
	}
	int need_redial = 0;
	int status;
	pid_t pid;
	int _count = 1;
	int num;

	while (1) {
#if defined(HAVE_3G)
#if defined(HAVE_LIBMBIM) || defined(HAVE_UMBIM)
		if (nvram_match("wan_proto", "3g") &&
		    nvram_match("3gdata", "mbim")) {
			start_service_force("check_mbim");
		}
#endif
		if (nvram_match("wan_proto", "3g") &&
		    nvram_match("3gdata", "sierradirectip")) {
			start_service_force("check_sierradirectip");
		} else if (nvram_match("wan_proto", "3g") &&
			   nvram_match("3gnmvariant", "1")) {
			start_service_force("check_sierrappp");
		}
#endif
#if defined(HAVE_UQMI) || defined(HAVE_LIBQMI)
		if (nvram_match("wan_proto", "3g") &&
		    nvram_match("3gdata", "qmi") && _count == 1) {
			start_service_force("check_qmi");
		}
#endif
		if (argc < 2)
			sleep(30);
		else
			sleep(atoi(argv[1]));
		num = 0;
		_count++;

		//               fprintf(stderr, "check PPPoE %d\n", num);
		if (!check_wan_link(num)) {
			//                       fprintf(stderr, "PPPoE %d need to redial\n", num);
			need_redial = 1;
		} else {
			//                       fprintf(stderr, "PPPoE %d not need to redial\n", num);
			continue;
		}

		if (need_redial) {
			pid = fork();
			switch (pid) {
			case -1:
				perror("fork failed");
				exit(1);
			case 0:
#ifdef HAVE_PPPOE
				if (nvram_match("wan_proto", "pppoe")) {
					sleep(1);
					start_service_force("wan_redial");
				}
#if defined(HAVE_PPTP) || defined(HAVE_L2TP) || defined(HAVE_HEARTBEAT) || \
	defined(HAVE_PPPOATM) || defined(HAVE_PPPOEDUAL)
				else
#endif
#endif

#ifdef HAVE_PPPOEDUAL
					if (nvram_match("wan_proto",
							"pppoe_dual")) {
					sleep(1);
					start_service_force("wan_redial");
				}
#if defined(HAVE_PPTP) || defined(HAVE_L2TP) || defined(HAVE_HEARTBEAT) || \
	defined(HAVE_PPPOATM)
				else
#endif
#endif

#ifdef HAVE_PPPOATM
					if (nvram_match("wan_proto", "pppoa")) {
					sleep(1);
					start_service_force("wan_redial");
				}
#if defined(HAVE_PPTP) || defined(HAVE_L2TP) || defined(HAVE_HEARTBEAT)
				else
#endif
#endif

#ifdef HAVE_PPTP
					if (nvram_match("wan_proto", "pptp")) {
					stop_service_force("pptp");
					unlink("/tmp/services/pptp.stop");
					sleep(1);
					start_service_force("wan_redial");
				}
#if defined(HAVE_L2TP) || defined(HAVE_HEARTBEAT)
				else
#endif
#endif
#ifdef HAVE_L2TP
					if (nvram_match("wan_proto", "l2tp")) {
					stop_service_force("l2tp");
					unlink("/tmp/services/l2tp.stop");
					sleep(1);
					start_service_force("wan_redial");
				}
#ifdef HAVE_HEARTBEAT
				else
#endif
#endif
				// Moded by Boris Bakchiev
				// We dont need this at all.
				// But if this code is executed by any of pppX programs
				// we might have to do this.

#ifdef HAVE_HEARTBEAT
					if (nvram_match("wan_proto",
							"heartbeat")) {
					if (pidof("bpalogin") == -1) {
						stop_service_force(
							"heartbeat_redial");
						sleep(1);
						start_service_force(
							"heartbeat_redial");
					}

				}
#endif
#ifdef HAVE_3G
				else if (nvram_match("wan_proto", "3g")) {
					sleep(1);
					start_service_force("wan_redial");
				}
#endif
#ifdef HAVE_IPETH
				else if (nvram_match("wan_proto", "iphone")) {
					sleep(1);
					start_service_force("wan_redial");
				}
#endif
				exit(0);
				break;
			default:
				waitpid(pid, &status, 0);
				// dprintf("parent\n");
				break;
			}
		}
	}
}
