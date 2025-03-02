# Macros to probe for multicast headers and IPv4/IPv6 support

AC_DEFUN([AC_CHECK_MROUTE_HEADERS],[
	AC_CHECK_HEADERS([linux/mroute.h linux/filter.h], [], [],[
		#ifdef HAVE_SYS_SOCKET_H
		# include <sys/socket.h>
		#endif
		#ifdef HAVE_NETINET_IN_H
		# include <netinet/in.h>
		#endif
		#define _LINUX_IN_H             /* For Linux <= 2.6.25 */
		#include <linux/types.h>
	])

	AC_CHECK_HEADERS([netinet/ip_mroute.h], [], [],[
		#ifdef HAVE_SYS_SOCKET_H
		# include <sys/socket.h>
		#endif
		#ifdef HAVE_SYS_TYPES_H
		# include <sys/types.h>
		#endif
		#ifdef HAVE_NETINET_IN_H
		# include <netinet/in.h>
		#endif
		#ifdef HAVE_NET_ROUTE_H
		# include <net/route.h>
		#endif
	])
])

AC_DEFUN([AC_CHECK_MROUTE],[
	AC_CHECK_MROUTE_HEADERS()
])

AC_DEFUN([AC_CHECK_MROUTE6_HEADERS],[
	AC_CHECK_HEADERS([linux/mroute6.h], [], [],[
		#ifdef HAVE_SYS_SOCKET_H
		# include <sys/socket.h>
		#endif
		#ifdef HAVE_NETINET_IN_H
		# include <netinet/in.h>
		#endif
	])

	AC_CHECK_HEADERS([netinet6/ip6_mroute.h], [], [],[
		#ifdef HAVE_NETINET_IN_H
		# include <netinet/in.h>
		#endif
		#ifdef HAVE_SYS_PARAM_H
		# include <sys/param.h>
		#endif
	])
])

AC_DEFUN([AC_CHECK_MROUTE6],[
	AC_CHECK_MROUTE6_HEADERS()

	AC_MSG_CHECKING(for IPv6 multicast host support)
	AC_COMPILE_IFELSE([
		AC_LANG_PROGRAM([[
			#ifdef HAVE_SYS_SOCKET_H
			# include <sys/socket.h>
			#endif
			#ifdef HAVE_NETINET_IN_H
			# include <netinet/in.h>
			#endif
		]],[[
			struct ipv6_mreq mreq;
		]])],[
		AC_DEFINE(HAVE_IPV6_MULTICAST_HOST, 1, [Define if your OS supports acting as an IPv6 multicast host])
		AC_MSG_RESULT(yes)],[
		AC_MSG_RESULT(no)])

	AC_MSG_CHECKING(for IPv6 multicast routing support)
	AC_COMPILE_IFELSE([
		AC_LANG_PROGRAM([[
			#ifdef HAVE_SYS_SOCKET_H
			# include <sys/socket.h>
			#endif
			#ifdef HAVE_NETINET_IN_H
			# include <netinet/in.h>
			#endif
			#ifdef HAVE_LINUX_MROUTE6_H
			# include <linux/mroute6.h>
			#endif
			#ifdef HAVE_SYS_PARAM_H
			# include <sys/param.h>
			#endif
			#ifdef HAVE_NETINET6_IP6_MROUTE_H
			# include <netinet6/ip6_mroute.h>
			#endif
		]],[[
			int dummy = MRT6_INIT;
		]])],[
		AC_DEFINE(HAVE_IPV6_MULTICAST_ROUTING, 1, [Define if your OS supports IPv6 multicast routing])
   		AC_MSG_RESULT(yes)
		enable_ipv6=yes],[
		AC_MSG_RESULT(no)
		enable_ipv6=no])

	AC_MSG_CHECKING(for vifc_rate_limit member in struct mif6ctl)
	AC_COMPILE_IFELSE([
		AC_LANG_PROGRAM([[
			#ifdef HAVE_LINUX_MROUTE6_H
			# include <linux/mroute6.h>
			#endif
			#ifdef HAVE_NETINET6_IP6_MROUTE_H
			# include <netinet6/ip6_mroute.h>
			#endif
		]],[[
			struct mif6ctl dummy;
			dummy.vifc_rate_limit = 1;
		]])],[
		AC_DEFINE(HAVE_MIF6CTL_VIFC_RATE_LIMIT, 1, [Define if the struct mif6ctl has a member vifc_rate_limit on your OS])
		AC_MSG_RESULT(yes)],[
		AC_MSG_RESULT(no)])

	AC_MSG_CHECKING(for vifc_threshold member in struct mif6ctl)
	AC_COMPILE_IFELSE([
		AC_LANG_PROGRAM([[
			#ifdef HAVE_LINUX_MROUTE6_H
			#include <linux/mroute6.h>
			#endif
			#ifdef HAVE_NETINET6_IP6_MROUTE_H
			#include <netinet6/ip6_mroute.h>
			#endif
		]],[[
			struct mif6ctl dummy;
			dummy.vifc_threshold = 1;
		]])],[
		AC_DEFINE(HAVE_MIF6CTL_VIFC_THRESHOLD, 1, [Define if the struct mif6ctl has a member vifc_threshold on your OS])
		AC_MSG_RESULT(yes)],[
		AC_MSG_RESULT(no)])
])
