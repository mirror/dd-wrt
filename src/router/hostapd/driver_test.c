/*
 * hostapd / Driver interface for development testing
 * Copyright (c) 2004-2006, Jouni Malinen <jkmaline@cc.hut.fi>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * Alternatively, this software may be distributed under the terms of BSD
 * license.
 *
 * See README and COPYING for more details.
 */

#include "includes.h"
#include <sys/ioctl.h>
#include <sys/un.h>
#include <dirent.h>

#include "hostapd.h"
#include "driver.h"
#include "sha1.h"
#include "eloop.h"
#include "ieee802_1x.h"
#include "sta_info.h"
#include "eapol_sm.h"
#include "wpa.h"
#include "accounting.h"
#include "radius.h"
#include "l2_packet.h"
#include "hostap_common.h"
#include "ieee802_11.h"
#include "hw_features.h"


struct test_client_socket {
	struct test_client_socket *next;
	u8 addr[ETH_ALEN];
	struct sockaddr_un un;
	socklen_t unlen;
};

struct test_driver_data {
	struct driver_ops ops;
	struct hostapd_data *hapd;
	struct test_client_socket *cli;
	int test_socket;
	u8 *ie;
	size_t ielen;
	char *socket_dir;
	char *own_socket_path;
};

static const struct driver_ops test_driver_ops;


static void test_driver_free_priv(struct test_driver_data *drv)
{
	if (drv == NULL)
		return;

	free(drv->ie);
	free(drv->own_socket_path);
	free(drv->socket_dir);
	free(drv);
}


static struct test_client_socket *
test_driver_get_cli(struct test_driver_data *drv, struct sockaddr_un *from,
		    socklen_t fromlen)
{
	struct test_client_socket *cli = drv->cli;

	while (cli) {
		if (cli->unlen == fromlen &&
		    strncmp(cli->un.sun_path, from->sun_path,
			    fromlen - sizeof(cli->un.sun_family)) == 0)
			return cli;
		cli = cli->next;
	}

	return NULL;
}


static int test_driver_send_eapol(void *priv, const u8 *addr, const u8 *data,
				  size_t data_len, int encrypt)
{
	struct test_driver_data *drv = priv;
	struct test_client_socket *cli;
	struct msghdr msg;
	struct iovec io[3];
	struct l2_ethhdr eth;

	if (drv->test_socket < 0)
		return -1;

	cli = drv->cli;
	while (cli) {
		if (memcmp(cli->addr, addr, ETH_ALEN) == 0)
			break;
		cli = cli->next;
	}

	if (!cli) {
		wpa_printf(MSG_DEBUG, "%s: no destination client entry",
			   __func__);
		return -1;
	}

	memcpy(eth.h_dest, addr, ETH_ALEN);
	memcpy(eth.h_source, drv->hapd->own_addr, ETH_ALEN);
	eth.h_proto = htons(ETH_P_EAPOL);

	io[0].iov_base = "EAPOL ";
	io[0].iov_len = 6;
	io[1].iov_base = &eth;
	io[1].iov_len = sizeof(eth);
	io[2].iov_base = (u8 *) data;
	io[2].iov_len = data_len;

	memset(&msg, 0, sizeof(msg));
	msg.msg_iov = io;
	msg.msg_iovlen = 3;
	msg.msg_name = &cli->un;
	msg.msg_namelen = cli->unlen;
	return sendmsg(drv->test_socket, &msg, 0);
}


static int test_driver_send_mgmt_frame(void *priv, const void *buf,
				  size_t len, int flags)
{
	struct test_driver_data *drv = priv;
	struct msghdr msg;
	struct iovec io[2];
	const u8 *dest;
	int ret = 0, broadcast = 0;
	char desttxt[30];
	struct sockaddr_un addr;
	struct dirent *dent;
	DIR *dir;
	struct ieee80211_hdr *hdr;
	u16 fc;

	if (drv->test_socket < 0 || len < 10 || drv->socket_dir == NULL)
		return -1;

	dest = buf;
	dest += 4;
	broadcast = memcmp(dest, "\xff\xff\xff\xff\xff\xff", ETH_ALEN) == 0;
	snprintf(desttxt, sizeof(desttxt), MACSTR, MAC2STR(dest));

	io[0].iov_base = "MLME ";
	io[0].iov_len = 5;
	io[1].iov_base = (void *) buf;
	io[1].iov_len = len;

	memset(&msg, 0, sizeof(msg));
	msg.msg_iov = io;
	msg.msg_iovlen = 2;

	dir = opendir(drv->socket_dir);
	if (dir == NULL)
		return -1;
	while ((dent = readdir(dir))) {
#ifdef _DIRENT_HAVE_D_TYPE
		/* Skip the file if it is not a socket. Also accept
		 * DT_UNKNOWN (0) in case the C library or underlying file
		 * system does not support d_type. */
		if (dent->d_type != DT_SOCK && dent->d_type != DT_UNKNOWN)
			continue;
#endif /* _DIRENT_HAVE_D_TYPE */
		if (strcmp(dent->d_name, ".") == 0 ||
		    strcmp(dent->d_name, "..") == 0)
			continue;

		memset(&addr, 0, sizeof(addr));
		addr.sun_family = AF_UNIX;
		snprintf(addr.sun_path, sizeof(addr.sun_path), "%s/%s",
			 drv->socket_dir, dent->d_name);

		if (strcmp(addr.sun_path, drv->own_socket_path) == 0)
			continue;
		if (!broadcast && strstr(dent->d_name, desttxt) == NULL)
			continue;

		wpa_printf(MSG_DEBUG, "%s: Send management frame to %s",
			   __func__, dent->d_name);

		msg.msg_name = &addr;
		msg.msg_namelen = sizeof(addr);
		ret = sendmsg(drv->test_socket, &msg, 0);
	}
	closedir(dir);

	hdr = (struct ieee80211_hdr *) buf;
	fc = le_to_host16(hdr->frame_control);
	ieee802_11_mgmt_cb(drv->hapd, (u8 *) buf, len, WLAN_FC_GET_STYPE(fc),
			   ret >= 0);

	return ret;
}


static void test_driver_scan(struct test_driver_data *drv,
			     struct sockaddr_un *from, socklen_t fromlen)
{
	char buf[512], *pos, *end;
	int ret;

	pos = buf;
	end = buf + sizeof(buf);

	wpa_printf(MSG_DEBUG, "test_driver: SCAN");

	/* reply: SCANRESP BSSID SSID IEs */
	ret = snprintf(pos, end - pos, "SCANRESP " MACSTR " ",
		       MAC2STR(drv->hapd->own_addr));
	if (ret < 0 || ret >= end - pos)
		return;
	pos += ret;
	pos += wpa_snprintf_hex(pos, end - pos, drv->hapd->conf->ssid.ssid,
				drv->hapd->conf->ssid.ssid_len);
	ret = snprintf(pos, end - pos, " ");
	if (ret < 0 || ret >= end - pos)
		return;
	pos += ret;
	pos += wpa_snprintf_hex(pos, end - pos, drv->ie, drv->ielen);

	sendto(drv->test_socket, buf, pos - buf, 0,
	       (struct sockaddr *) from, fromlen);
}


static int test_driver_new_sta(struct test_driver_data *drv, const u8 *addr,
			       const u8 *ie, size_t ielen)
{
	struct hostapd_data *hapd = drv->hapd;
	struct sta_info *sta;
	int new_assoc, res;

	hostapd_logger(hapd, addr, HOSTAPD_MODULE_IEEE80211,
		HOSTAPD_LEVEL_INFO, "associated");

	sta = ap_get_sta(hapd, addr);
	if (sta) {
		accounting_sta_stop(hapd, sta);
	} else {
		sta = ap_sta_add(hapd, addr);
		if (sta == NULL)
			return -1;
	}
	accounting_sta_get_id(hapd, sta);

	if (hapd->conf->wpa) {
		if (ie == NULL || ielen == 0) {
			printf("test_driver: no IE from STA\n");
			return -1;
		}
		if (sta->wpa_sm == NULL)
			sta->wpa_sm = wpa_auth_sta_init(hapd->wpa_auth,
							sta->addr);
		if (sta->wpa_sm == NULL) {
			printf("test_driver: Failed to initialize WPA state "
			       "machine\n");
			return -1;
		}
		res = wpa_validate_wpa_ie(hapd->wpa_auth, sta->wpa_sm,
					  ie, ielen);
		if (res != WPA_IE_OK) {
			printf("WPA/RSN information element rejected? "
			       "(res %u)\n", res);
			return -1;
		}
	}

	new_assoc = (sta->flags & WLAN_STA_ASSOC) == 0;
	sta->flags |= WLAN_STA_ASSOC;
	wpa_auth_sm_event(sta->wpa_sm, WPA_ASSOC);

	hostapd_new_assoc_sta(hapd, sta, !new_assoc);

	ieee802_1x_notify_port_enabled(sta->eapol_sm, 1);

	return 0;
}


static void test_driver_assoc(struct test_driver_data *drv,
			      struct sockaddr_un *from, socklen_t fromlen,
			      char *data)
{
	struct test_client_socket *cli;
	u8 ie[256];
	size_t ielen;
	char *pos, *pos2, cmd[50];

	/* data: STA-addr SSID(hex) IEs(hex) */

	cli = wpa_zalloc(sizeof(*cli));
	if (cli == NULL)
		return;

	if (hwaddr_aton(data, cli->addr)) {
		printf("test_socket: Invalid MAC address '%s' in ASSOC\n",
		       data);
		free(cli);
		return;
	}
	pos = data + 17;
	while (*pos == ' ')
		pos++;
	pos2 = strchr(pos, ' ');
	ielen = 0;
	if (pos2) {
		/* TODO: verify SSID */

		pos = pos2 + 1;
		ielen = strlen(pos) / 2;
		if (ielen > sizeof(ie))
			ielen = sizeof(ie);
		if (hexstr2bin(pos, ie, ielen) < 0)
			ielen = 0;
	}

	memcpy(&cli->un, from, sizeof(cli->un));
	cli->unlen = fromlen;
	cli->next = drv->cli;
	drv->cli = cli;
	wpa_hexdump_ascii(MSG_DEBUG, "test_socket: ASSOC sun_path",
			  cli->un.sun_path,
			  cli->unlen - sizeof(cli->un.sun_family));

	snprintf(cmd, sizeof(cmd), "ASSOCRESP " MACSTR " 0",
		 MAC2STR(drv->hapd->own_addr));
	sendto(drv->test_socket, cmd, strlen(cmd), 0,
	       (struct sockaddr *) from, fromlen);

	if (test_driver_new_sta(drv, cli->addr, ie, ielen) < 0) {
		wpa_printf(MSG_DEBUG, "test_driver: failed to add new STA");
	}
}


static void test_driver_disassoc(struct test_driver_data *drv,
				 struct sockaddr_un *from, socklen_t fromlen)
{
	struct test_client_socket *cli;
	struct sta_info *sta;

	cli = test_driver_get_cli(drv, from, fromlen);
	if (!cli)
		return;

	hostapd_logger(drv->hapd, cli->addr, HOSTAPD_MODULE_IEEE80211,
		       HOSTAPD_LEVEL_INFO, "disassociated");

	sta = ap_get_sta(drv->hapd, cli->addr);
	if (sta != NULL) {
		sta->flags &= ~WLAN_STA_ASSOC;
		wpa_auth_sm_event(sta->wpa_sm, WPA_DISASSOC);
		sta->acct_terminate_cause =
			RADIUS_ACCT_TERMINATE_CAUSE_USER_REQUEST;
		ieee802_1x_notify_port_enabled(sta->eapol_sm, 0);
		ap_free_sta(drv->hapd, sta);
	}
}


static void test_driver_eapol(struct test_driver_data *drv,
			      struct sockaddr_un *from, socklen_t fromlen,
			      u8 *data, size_t datalen)
{
	struct test_client_socket *cli;
	if (datalen > 14) {
		/* Skip Ethernet header */
		data += 14;
		datalen -= 14;
	}
	cli = test_driver_get_cli(drv, from, fromlen);
	if (cli)
		ieee802_1x_receive(drv->hapd, cli->addr, data, datalen);
	else {
		wpa_printf(MSG_DEBUG, "test_socket: EAPOL from unknown "
			   "client");
	}
}


static void test_driver_mlme(struct test_driver_data *drv,
			     struct sockaddr_un *from, socklen_t fromlen,
			     u8 *data, size_t datalen)
{
	struct ieee80211_hdr *hdr;
	u16 fc;

	hdr = (struct ieee80211_hdr *) data;

	if (test_driver_get_cli(drv, from, fromlen) == NULL && datalen >= 16) {
		struct test_client_socket *cli;
		cli = wpa_zalloc(sizeof(*cli));
		if (cli == NULL)
			return;
		wpa_printf(MSG_DEBUG, "Adding client entry for " MACSTR,
			   MAC2STR(hdr->addr2));
		memcpy(cli->addr, hdr->addr2, ETH_ALEN);
		memcpy(&cli->un, from, sizeof(cli->un));
		cli->unlen = fromlen;
		cli->next = drv->cli;
		drv->cli = cli;
	}

	wpa_hexdump(MSG_MSGDUMP, "test_driver_mlme: received frame",
		    data, datalen);
	fc = le_to_host16(hdr->frame_control);
	if (WLAN_FC_GET_TYPE(fc) != WLAN_FC_TYPE_MGMT) {
		wpa_printf(MSG_ERROR, "%s: received non-mgmt frame",
			   __func__);
		return;
	}
	ieee802_11_mgmt(drv->hapd, data, datalen, WLAN_FC_GET_STYPE(fc), NULL);
}


static void test_driver_receive_unix(int sock, void *eloop_ctx, void *sock_ctx)
{
	struct test_driver_data *drv = eloop_ctx;
	char buf[2000];
	int res;
	struct sockaddr_un from;
	socklen_t fromlen = sizeof(from);

	res = recvfrom(sock, buf, sizeof(buf) - 1, 0,
		       (struct sockaddr *) &from, &fromlen);
	if (res < 0) {
		perror("recvfrom(test_socket)");
		return;
	}
	buf[res] = '\0';

	wpa_printf(MSG_DEBUG, "test_driver: received %u bytes", res);

	if (strcmp(buf, "SCAN") == 0) {
		test_driver_scan(drv, &from, fromlen);
	} else if (strncmp(buf, "ASSOC ", 6) == 0) {
		test_driver_assoc(drv, &from, fromlen, buf + 6);
	} else if (strcmp(buf, "DISASSOC") == 0) {
		test_driver_disassoc(drv, &from, fromlen);
	} else if (strncmp(buf, "EAPOL ", 6) == 0) {
		test_driver_eapol(drv, &from, fromlen, buf + 6, res - 6);
	} else if (strncmp(buf, "MLME ", 5) == 0) {
		test_driver_mlme(drv, &from, fromlen, buf + 5, res - 5);
	} else {
		wpa_hexdump_ascii(MSG_DEBUG, "Unknown test_socket command",
				  (u8 *) buf, res);
	}
}


static int test_driver_set_generic_elem(void *priv,
					const u8 *elem, size_t elem_len)
{
	struct test_driver_data *drv = priv;

	free(drv->ie);
	drv->ie = malloc(elem_len);
	if (drv->ie) {
		memcpy(drv->ie, elem, elem_len);
		drv->ielen = elem_len;
		return 0;
	} else {
		drv->ielen = 0;
		return -1;
	}
}


static int test_driver_sta_deauth(void *priv, const u8 *addr, int reason)
{
	struct test_driver_data *drv = priv;
	struct test_client_socket *cli;

	if (drv->test_socket < 0)
		return -1;

	cli = drv->cli;
	while (cli) {
		if (memcmp(cli->addr, addr, ETH_ALEN) == 0)
			break;
		cli = cli->next;
	}

	if (!cli)
		return -1;

	return sendto(drv->test_socket, "DEAUTH", 6, 0,
		      (struct sockaddr *) &cli->un, cli->unlen);
}


static int test_driver_sta_disassoc(void *priv, const u8 *addr, int reason)
{
	struct test_driver_data *drv = priv;
	struct test_client_socket *cli;

	if (drv->test_socket < 0)
		return -1;

	cli = drv->cli;
	while (cli) {
		if (memcmp(cli->addr, addr, ETH_ALEN) == 0)
			break;
		cli = cli->next;
	}

	if (!cli)
		return -1;

	return sendto(drv->test_socket, "DISASSOC", 8, 0,
		      (struct sockaddr *) &cli->un, cli->unlen);
}


static struct hostapd_hw_modes *
test_driver_get_hw_feature_data(void *priv, u16 *num_modes, u16 *flags)
{
	struct hostapd_hw_modes *modes;

	*num_modes = 1;
	*flags = 0;
	modes = wpa_zalloc(*num_modes * sizeof(struct hostapd_hw_modes));
	if (modes == NULL)
		return NULL;
	modes[0].mode = HOSTAPD_MODE_IEEE80211G;
	modes[0].num_channels = 1;
	modes[0].num_rates = 1;
	modes[0].channels = wpa_zalloc(sizeof(struct hostapd_channel_data));
	modes[0].rates = wpa_zalloc(sizeof(struct hostapd_rate_data));
	if (modes[0].channels == NULL || modes[0].rates == NULL) {
		hostapd_free_hw_features(modes, *num_modes);
		return NULL;
	}
	modes[0].channels[0].chan = 1;
	modes[0].channels[0].freq = 2412;
	modes[0].channels[0].flag = HOSTAPD_CHAN_W_SCAN |
		HOSTAPD_CHAN_W_ACTIVE_SCAN;
	modes[0].rates[0].rate = 10;
	modes[0].rates[0].flags = HOSTAPD_RATE_BASIC | HOSTAPD_RATE_SUPPORTED |
		HOSTAPD_RATE_CCK | HOSTAPD_RATE_MANDATORY;

	return modes;
}


static int test_driver_init(struct hostapd_data *hapd)
{
	struct test_driver_data *drv;
	struct sockaddr_un addr;

	drv = wpa_zalloc(sizeof(struct test_driver_data));
	if (drv == NULL) {
		printf("Could not allocate memory for test driver data\n");
		return -1;
	}

	drv->ops = test_driver_ops;
	drv->hapd = hapd;

	/* Generate a MAC address to help testing with multiple APs */
	hapd->own_addr[0] = 0x02; /* locally administered */
	sha1_prf(hapd->conf->iface, strlen(hapd->conf->iface),
		 "hostapd test bssid generation",
		 hapd->conf->ssid.ssid, hapd->conf->ssid.ssid_len,
		 hapd->own_addr + 1, ETH_ALEN - 1);

	if (hapd->conf->test_socket) {
		if (strlen(hapd->conf->test_socket) >= sizeof(addr.sun_path)) {
			printf("Too long test_socket path\n");
			test_driver_free_priv(drv);
			return -1;
		}
		if (strncmp(hapd->conf->test_socket, "DIR:", 4) == 0) {
			size_t len = strlen(hapd->conf->test_socket) + 30;
			drv->socket_dir = strdup(hapd->conf->test_socket + 4);
			drv->own_socket_path = malloc(len);
			if (drv->own_socket_path) {
				snprintf(drv->own_socket_path, len,
					 "%s/AP-" MACSTR,
					 hapd->conf->test_socket + 4,
					 MAC2STR(hapd->own_addr));
			}
		} else {
			drv->own_socket_path = strdup(hapd->conf->test_socket);
		}
		if (drv->own_socket_path == NULL) {
			test_driver_free_priv(drv);
			return -1;
		}

		drv->test_socket = socket(PF_UNIX, SOCK_DGRAM, 0);
		if (drv->test_socket < 0) {
			perror("socket(PF_UNIX)");
			test_driver_free_priv(drv);
			return -1;
		}

		memset(&addr, 0, sizeof(addr));
		addr.sun_family = AF_UNIX;
		strncpy(addr.sun_path, drv->own_socket_path,
			sizeof(addr.sun_path));
		if (bind(drv->test_socket, (struct sockaddr *) &addr,
			 sizeof(addr)) < 0) {
			perror("bind(PF_UNIX)");
			close(drv->test_socket);
			unlink(drv->own_socket_path);
			test_driver_free_priv(drv);
			return -1;
		}
		eloop_register_read_sock(drv->test_socket,
					 test_driver_receive_unix, drv, NULL);
	} else
		drv->test_socket = -1;

	hapd->driver = &drv->ops;
	return 0;
}


static void test_driver_deinit(void *priv)
{
	struct test_driver_data *drv = priv;
	struct test_client_socket *cli, *prev;

	cli = drv->cli;
	while (cli) {
		prev = cli;
		cli = cli->next;
		free(prev);
	}

	if (drv->test_socket >= 0) {
		eloop_unregister_read_sock(drv->test_socket);
		close(drv->test_socket);
		unlink(drv->own_socket_path);
	}

	drv->hapd->driver = NULL;

	test_driver_free_priv(drv);
}


static const struct driver_ops test_driver_ops = {
	.name = "test",
	.init = test_driver_init,
	.deinit = test_driver_deinit,
	.send_eapol = test_driver_send_eapol,
	.send_mgmt_frame = test_driver_send_mgmt_frame,
	.set_generic_elem = test_driver_set_generic_elem,
	.sta_deauth = test_driver_sta_deauth,
	.sta_disassoc = test_driver_sta_disassoc,
	.get_hw_feature_data = test_driver_get_hw_feature_data,
};


void test_driver_register(void)
{
	driver_register(test_driver_ops.name, &test_driver_ops);
}
