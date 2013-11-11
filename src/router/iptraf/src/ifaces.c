/* For terms of usage/redistribution/modification see the LICENSE file */
/* For authors and contributors see the AUTHORS file */

/***

ifaces.c - routine that determines whether a given interface is supported
		by IPTraf

***/

#include "iptraf-ng-compat.h"

#include "error.h"

/*
 * Open /proc/net/dev and move file pointer past the two table header lines
 * at the top of the file.
 */

FILE *open_procnetdev(void)
{
	FILE *fd;
	char buf[161];

	fd = fopen("/proc/net/dev", "r");

	/*
	 * Read and discard the table header lines in the file
	 */

	if (fd != NULL) {
		fgets(buf, 160, fd);
		fgets(buf, 160, fd);
	}

	return fd;
}

/*
 * Get the next interface from /proc/net/dev.
 */
int get_next_iface(FILE * fd, char *ifname, int n)
{
	char buf[161];

	strcpy(ifname, "");

	if (!feof(fd)) {
		strcpy(buf, "");
		fgets(buf, 160, fd);
		if (strcmp(buf, "") != 0) {
			memset(ifname, 0, n);
			strncpy(ifname, skip_whitespace(strtok(buf, ":")), n);
			if (ifname[n - 1] != '\0')
				strcpy(ifname, "");
			return 1;
		}
	}
	return 0;
}

int dev_up(char *iface)
{
	int fd;
	int ir;
	struct ifreq ifr;

	fd = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP);

	strcpy(ifr.ifr_name, iface);
	ir = ioctl(fd, SIOCGIFFLAGS, &ifr);

	close(fd);

	if ((ir != 0) || (!(ifr.ifr_flags & IFF_UP)))
		return 0;

	return 1;
}

void err_iface_down(void)
{
	write_error("Specified interface not active");
}

int dev_get_ifindex(const char *iface)
{
	int fd = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (fd == -1)
		return fd;

	struct ifreq ifr;
	strcpy(ifr.ifr_name, iface);
	int ir = ioctl(fd, SIOCGIFINDEX, &ifr);

	/* need to preserve errno across call to close() */
	int saved_errno = errno;

	close(fd);

	/* bug out if ioctl() failed */
	if (ir != 0) {
		errno = saved_errno;
		return ir;
	}

	return ifr.ifr_ifindex;
}

int dev_get_mtu(const char *iface)
{
	int fd = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (fd == -1)
		return fd;

	struct ifreq ifr;
	strcpy(ifr.ifr_name, iface);
	int ir = ioctl(fd, SIOCGIFMTU, &ifr);

	/* need to preserve errno across call to close() */
	int saved_errno = errno;

	close(fd);

	/* bug out if ioctl() failed */
	if (ir != 0) {
		errno = saved_errno;
		return ir;
	}

	return ifr.ifr_mtu;
}

int dev_get_flags(const char *iface)
{
	int fd = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (fd == -1)
		return fd;

	struct ifreq ifr;
	strcpy(ifr.ifr_name, iface);
	int ir = ioctl(fd, SIOCGIFFLAGS, &ifr);

	/* need to preserve errno across call to close() */
	int saved_errno = errno;

	close(fd);

	/* bug out if ioctl() failed */
	if (ir != 0) {
		errno = saved_errno;
		return ir;
	}

	return ifr.ifr_flags;
}

int dev_set_flags(const char *iface, int flags)
{
	int fd = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (fd == -1)
		return fd;

	struct ifreq ifr;
	strcpy(ifr.ifr_name, iface);
	int ir = ioctl(fd, SIOCGIFFLAGS, &ifr);
	if (ir == -1)
		goto err;

	ifr.ifr_flags |= flags;
	ir = ioctl(fd, SIOCSIFFLAGS, &ifr);

	int saved_errno;
err:	/* need to preserve errno across call to close() */
	saved_errno = errno;

	close(fd);

	/* bug out if ioctl() failed */
	if (ir != 0)
		errno = saved_errno;

	return ir;
}

int dev_clear_flags(const char *iface, int flags)
{
	int fd = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (fd == -1)
		return fd;

	struct ifreq ifr;
	strcpy(ifr.ifr_name, iface);
	int ir = ioctl(fd, SIOCGIFFLAGS, &ifr);
	if (ir == -1)
		goto err;

	ifr.ifr_flags &= ~flags;
	ir = ioctl(fd, SIOCSIFFLAGS, &ifr);

	int saved_errno;
err:	/* need to preserve errno across call to close() */
	saved_errno = errno;

	close(fd);

	/* bug out if ioctl() failed */
	if (ir != 0)
		errno = saved_errno;

	return ir;
}

int dev_get_ifname(int ifindex, char *ifname)
{
	int fd = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (fd == -1)
		return fd;

	struct ifreq ifr = {
		.ifr_ifindex = ifindex
	};
	int ir = ioctl(fd, SIOCGIFNAME, &ifr);

	/* need to preserve errno across call to close() */
	int saved_errno = errno;

	close(fd);

	/* bug out if ioctl() failed */
	if (ir != 0) {
		errno = saved_errno;
		return ir;
	}

	strncpy(ifname, ifr.ifr_name, IFNAMSIZ);
	return ir;
}

int dev_bind_ifindex(int fd, const int ifindex)
{
	struct sockaddr_ll fromaddr;
	socklen_t addrlen = sizeof(fromaddr);

	fromaddr.sll_family = AF_PACKET;
	fromaddr.sll_protocol = htons(ETH_P_ALL);
	fromaddr.sll_ifindex = ifindex;
	return bind(fd, (struct sockaddr *) &fromaddr, addrlen);
}

int dev_bind_ifname(int fd, const char const *ifname)
{
	int ir;
	struct ifreq ifr;

	strcpy(ifr.ifr_name, ifname);
	ir = ioctl(fd, SIOCGIFINDEX, &ifr);
	if (ir)
		return ir;

	return dev_bind_ifindex(fd, ifr.ifr_ifindex);
}

char *gen_iface_msg(char *ifptr)
{
	static char if_msg[20];

	if (ifptr == NULL)
		strcpy(if_msg, "all interfaces");
	else
		strncpy(if_msg, ifptr, 20);

	return if_msg;
}


int dev_promisc_flag(const char *dev_name)
{
	int flags = dev_get_flags(dev_name);
	if (flags < 0) {
		write_error("Unable to obtain interface parameters for %s",
			    dev_name);
		return -1;
	}

	if (flags & IFF_PROMISC)
		return -1;

	return flags;
}
