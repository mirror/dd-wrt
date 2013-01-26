/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301, USA.
 *
 * Copyright (C) 2011 - 2012 Red Hat, Inc.
 * Copyright (C) 2010 - 2012 Google, Inc.
 */

#include <glib.h>
#include <string.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <termios.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>

typedef u_int16_t u16;
typedef u_int8_t u8;
typedef u_int32_t u32;
typedef u_int64_t u64;

typedef unsigned int qbool;
#ifndef TRUE
#define TRUE 1
#endif
#ifndef FALSE
#define FALSE 0
#endif

/***********************************************************/

static void
print_buf (const char *detail, const char *buf, size_t len)
{
    int i = 0, z;
    qbool newline = FALSE, indent = FALSE;
    char *f;
    guint flen;

    f = g_strdup_printf ("%s (%zu)  ", detail, len);
    flen = strlen (f);
    g_print ("%s", f);
    for (i = 0; i < len; i++) {
        if (indent) {
            z = flen;
            while (z--)
                g_print (" ");
            indent = FALSE;
        }
        g_print ("%02x ", buf[i] & 0xFF);
        if (((i + 1) % 16) == 0) {
            g_print ("\n");
            newline = TRUE;
            indent = TRUE;
        } else
            newline = FALSE;
    }

    if (!newline)
        g_print ("\n");
}

/**************************************************************/

typedef enum {
	QMI_SVC_CTL = 0x00,
	QMI_SVC_WDS = 0x01,
	QMI_SVC_DMS = 0x02,
	QMI_SVC_NAS = 0x03,
	QMI_SVC_QOS = 0x04,
	QMI_SVC_WMS = 0x05,
	QMI_SVC_PDS = 0x06,
	QMI_SVC_AUTH = 0x07,
	QMI_SVC_AT = 0x08,
	QMI_SVC_VOICE = 0x09,
	QMI_SVC_CAT2 = 0x0A,
	QMI_SVC_UIM = 0x0B,
	QMI_SVC_PBM = 0x0C,
	QMI_SVC_LOC = 0x10,
	QMI_SVC_SAR = 0x11,
	QMI_SVC_RMTFS = 0x14,
	QMI_SVC_CAT = 0xE0,
	QMI_SVC_RMS = 0xE1,
	QMI_SVC_OMA = 0xE2
} qmi_service_type;

struct qmux {
	u8 tf;	/* always 1 */
	u16 len;
	u8 ctrl;
	u8 service;
	u8 qmicid;
} __attribute__((__packed__));

struct qmi_tlv_status {
    u16 status;
    u16 error;
} __attribute__((__packed__));

struct getcid_req {
	struct qmux header;
	u8 req;
	u8 tid;
	u16 msgid;
	u16 tlvsize;
	u8 service;
	u16 size;
	u8 qmisvc;
} __attribute__((__packed__));

struct releasecid_req {
	struct qmux header;
	u8 req;
	u8 tid;
	u16 msgid;
	u16 tlvsize;
	u8 rlscid;
	u16 size;
	u16 cid;
} __attribute__((__packed__));

struct version_info_req {
	struct qmux header;
	u8 req;
	u8 tid;
	u16 msgid;
	u16 tlvsize;
} __attribute__((__packed__));

struct qmi_ctl_version_info_list_service {
	u8 service_type;   /* QMI_SVC_xxx */
	u16 major_version;
	u16 minor_version;
} __attribute__((__packed__));

struct qmi_tlv_ctl_version_info_list {
	u8 count;
	struct qmi_ctl_version_info_list_service services[0];
}__attribute__((__packed__));

struct seteventreport_req {
	struct qmux header;
	u8 req;
	u16 tid;
	u16 msgid;
	u16 tlvsize;
	u8 reportchanrate;
	u16 size;
	u8 period;
	u32 mask;
} __attribute__((__packed__));

struct getpkgsrvcstatus_req {
	struct qmux header;
	u8 req;
	u16 tid;
	u16 msgid;
	u16 tlvsize;
} __attribute__((__packed__));

struct getmeid_req {
	struct qmux header;
	u8 req;
	u16 tid;
	u16 msgid;
	u16 tlvsize;
} __attribute__((__packed__));

struct qmiwds_stats {
	u32 txok;
	u32 rxok;
	u32 txerr;
	u32 rxerr;
	u32 txofl;
	u32 rxofl;
	u64 txbytesok;
	u64 rxbytesok;
	qbool linkstate;
	qbool reconfigure;
};

struct nas_signal_req {
	struct qmux header;
	u8 req;
	u16 tid;
	u16 msgid;
	u16 tlvsize;
} __attribute__((__packed__));

const size_t qmux_size = sizeof(struct qmux);

static void
qmux_fill(struct qmux *qmux, u8 service, u16 cid, u16 size)
{
	qmux->tf = 1;
	qmux->len = size - 1;
	qmux->ctrl = 0;
	qmux->service = cid & 0xff;
	qmux->qmicid = cid >> 8;
}

static void *
qmictl_new_getcid(u8 tid, u8 svctype, size_t *size)
{
	struct getcid_req *req;

    req = g_malloc0 (sizeof (*req));
	req->req = 0x00;
	req->tid = tid;
	req->msgid = 0x0022;
	req->tlvsize = 0x0004;
	req->service = 0x01;
	req->size = 0x0001;
	req->qmisvc = svctype;
	*size = sizeof(*req);
    qmux_fill (&req->header, QMI_SVC_CTL, 0, *size);
	return req;
}

static void *
qmictl_new_releasecid(u8 tid, u16 cid, size_t *size)
{
	struct releasecid_req *req;

    req = g_malloc0 (sizeof (*req));
	req->req = 0x00;
	req->tid = tid;
	req->msgid = 0x0023;
	req->tlvsize = 0x05;
	req->rlscid = 0x01;
	req->size = 0x0002;
	req->cid = cid;
	*size = sizeof(*req);
    qmux_fill (&req->header, QMI_SVC_CTL, 0, *size);
	return req;
}

static void *
qmictl_new_version_info(u8 tid, size_t *size)
{
	struct version_info_req *req;

    req = g_malloc0 (sizeof (*req));
	req->req = 0x00;
	req->tid = tid;
	req->msgid = 0x21;
	req->tlvsize = 0;
	*size = sizeof(*req);
    qmux_fill (&req->header, QMI_SVC_CTL, 0, *size);
	return req;
}

static void *
qmiwds_new_seteventreport(u8 tid, size_t *size)
{
	struct seteventreport_req *req;

    req = g_malloc0 (sizeof (*req));
	req->req = 0x00;
	req->tid = tid;
	req->msgid = 0x0001;
	req->tlvsize = 0x0008;
	req->reportchanrate = 0x11;
	req->size = 0x0005;
	req->period = 0x01;
	req->mask = 0x000000ff;
	*size = sizeof(*req);
	return req;
}

static void *
qmiwds_new_getpkgsrvcstatus(u8 tid, size_t *size)
{
	struct getpkgsrvcstatus_req *req;

    req = g_malloc0 (sizeof (*req));
	req->req = 0x00;
	req->tid = tid;
	req->msgid = 0x22;
	req->tlvsize = 0x0000;
	*size = sizeof(*req);
	return req;
}

static void *
qmidms_new_getmeid(u16 cid, u8 tid, size_t *size)
{
	struct getmeid_req *req;

    req = g_malloc0 (sizeof (*req));
	req->req = 0x00;
	req->tid = tid;
	req->msgid = 0x25;
	req->tlvsize = 0x0000;
	*size = sizeof(*req);
    qmux_fill (&req->header, QMI_SVC_WDS, cid, *size);
	return req;
}

static int
qmux_parse(u16 *cid, void *buf, size_t size)
{
	struct qmux *qmux = buf;

	if (!buf || size < 12)
		return -ENOMEM;

	if (qmux->tf != 1 || qmux->len != size - 1 || qmux->ctrl != 0x80)
		return -EINVAL;

	*cid = (qmux->qmicid << 8) + qmux->service;
	return sizeof(*qmux);
}

static u16
tlv_get(void *msg, u16 msgsize, u8 type, void *buf, u16 bufsize)
{
	u16 pos;
	u16 msize = 0;

	if (!msg || !buf)
		return -ENOMEM;

	for (pos = 4;  pos + 3 < msgsize; pos += msize + 3) {
		msize = *(u16 *)(msg + pos + 1);
		if (*(u8 *)(msg + pos) == type) {
			if (bufsize < msize)
				return -ENOMEM;

			memcpy(buf, msg + pos + 3, msize);
			return msize;
		}
	}

	return -ENOMSG;
}

static int
qmi_msgisvalid(void *msg, u16 size)
{
    struct qmi_tlv_status status;

	if (tlv_get(msg, size, 2, &status, sizeof (status)) == sizeof (status)) {
        if (le16toh (status.status != 0))
            return le16toh (status.error);
		else
			return 0;
	}
	return -ENOMSG;
}

static int
qmi_msgid(void *msg, u16 size)
{
	return size < 2 ? -ENODATA : *(u16 *)msg;
}

static int
qmictl_version_info_resp(void *buf, u16 size)
{
	int result, i;
	u8 offset = sizeof(struct qmux) + 2;
    u8 svcbuf[100];
    struct qmi_tlv_ctl_version_info_list *service_list;
    struct qmi_ctl_version_info_list_service *svc;

	if (!buf || size < offset)
		return -ENOMEM;

	buf = buf + offset;
	size -= offset;

	result = qmi_msgid(buf, size);
	if (result != 0x21)
		return -EFAULT;

	result = qmi_msgisvalid(buf, size);
	if (result != 0)
		return -EFAULT;

    /* Get the services TLV */
	result = tlv_get(buf, size, 0x01, svcbuf, sizeof (svcbuf));
	if (result < 0)
		return -EFAULT;

    service_list = (struct qmi_tlv_ctl_version_info_list *) svcbuf;
    if (result < (service_list->count * sizeof (struct qmi_ctl_version_info_list_service)))
        return -EFAULT;

    svc = &(service_list->services[0]);
    for (i = 0; i < service_list->count; i++, svc++) {
        g_message ("SVC: %d  v%d.%d",
                   svc->service_type,
                   le16toh (svc->major_version),
                   le16toh (svc->minor_version));
    }

	return 0;
}

static int
qmictl_getcid_resp(void *buf, u16 size, u16 *cid)
{
	int result;
	u8 offset = sizeof(struct qmux) + 2;

	if (!buf || size < offset)
		return -ENOMEM;

	buf = buf + offset;
	size -= offset;

	result = qmi_msgid(buf, size);
	if (result != 0x22)
		return -EFAULT;

	result = qmi_msgisvalid(buf, size);
	if (result != 0)
		return -EFAULT;

	result = tlv_get(buf, size, 0x01, cid, 2);
	if (result != 2)
		return -EFAULT;

	return 0;
}

static int
qmictl_releasecid_resp(void *buf, u16 size)
{
	int result;
	u8 offset = sizeof(struct qmux) + 2;

	if (!buf || size < offset)
		return -ENOMEM;

	buf = buf + offset;
	size -= offset;

	result = qmi_msgid(buf, size);
	if (result != 0x23)
		return -EFAULT;

	result = qmi_msgisvalid(buf, size);
	if (result != 0)
		return -EFAULT;

	return 0;
}

static int
qmiwds_event_resp(void *buf, u16 size, struct qmiwds_stats *stats)
{
	int result;
	u8 status[2];

	u8 offset = sizeof(struct qmux) + 3;

	if (!buf || size < offset || !stats)
		return -ENOMEM;

	buf = buf + offset;
	size -= offset;

	result = qmi_msgid(buf, size);
	if (result == 0x01) {
		tlv_get(buf, size, 0x10, &stats->txok, 4);
		tlv_get(buf, size, 0x11, &stats->rxok, 4);
		tlv_get(buf, size, 0x12, &stats->txerr, 4);
		tlv_get(buf, size, 0x13, &stats->rxerr, 4);
		tlv_get(buf, size, 0x14, &stats->txofl, 4);
		tlv_get(buf, size, 0x15, &stats->rxofl, 4);
		tlv_get(buf, size, 0x19, &stats->txbytesok, 8);
		tlv_get(buf, size, 0x1A, &stats->rxbytesok, 8);
	} else if (result == 0x22) {
		result = tlv_get(buf, size, 0x01, &status[0], 2);
		if (result >= 1)
			stats->linkstate = status[0] == 0x02;
		if (result == 2)
			stats->reconfigure = status[1] == 0x01;

		if (result < 0)
			return result;
	} else {
		return -EFAULT;
	}

	return 0;
}

static int
qmidms_meid_resp(void *buf, u16 size, char *meid, int meidsize)
{
	int result;

	u8 offset = sizeof(struct qmux) + 3;

	if (!buf || size < offset || meidsize < 14)
		return -ENOMEM;

	buf = buf + offset;
	size -= offset;

	result = qmi_msgid(buf, size);
	if (result != 0x25)
		return -EFAULT;

	result = qmi_msgisvalid(buf, size);
	if (result)
		return -EFAULT;

	result = tlv_get(buf, size, 0x12, meid, 14);
	if (result != 14)
		return -EFAULT;

	return 0;
}

static void *
qminas_new_signal(u16 cid, u8 tid, size_t *size)
{
	struct nas_signal_req *req;

    req = g_malloc0 (sizeof (*req));
	req->req = 0x00;
	req->tid = tid;
	req->msgid = 0x20;
	req->tlvsize = 0x0000;
	*size = sizeof(*req);
    qmux_fill (&req->header, QMI_SVC_NAS, cid, *size);
	return req;
}

/* NAS/Get Signal Strength TLV 0x01 */
struct qminas_resp_signalstrength {
    u8 dbm;
    u8 act;
} __attribute__((__packed__));

static int
qminas_signal_resp(void *buf, u16 size, u8 *dbm, u8 *act)
{
	int result;
	struct qminas_resp_signalstrength signal;

	u8 offset = sizeof(struct qmux) + 3;

	if (!buf || size < offset)
		return -ENOMEM;

	buf = buf + offset;
	size -= offset;

	result = qmi_msgid(buf, size);
	if (result != 0x20)
		return -EFAULT;

	result = qmi_msgisvalid(buf, size);
	if (result)
		return -EFAULT;

	result = tlv_get(buf, size, 0x01, &signal, sizeof (signal));
	if (result != sizeof (signal))
		return -EFAULT;

    *dbm = signal.dbm;
    *act = signal.act;
	return 0;
}

/*****************************************************/

static size_t
send_and_wait_reply (int fd, void *b, size_t blen, char *reply, size_t rlen)
{
    ssize_t num;
    fd_set in;
    int result;
    struct timeval timeout = { 1, 0 };

    print_buf (">>>", b, blen);
    num = write (fd, b, blen);
    if (num != blen) {
        g_warning ("Failed to write: wrote %zd err %d", num, errno);
        return 0;
    }

    FD_ZERO (&in);
    FD_SET (fd, &in);
    result = select (fd + 1, &in, NULL, NULL, &timeout);
    if (result != 1 || !FD_ISSET (fd, &in)) {
        g_warning ("No data pending");
        return 0;
    }

    errno = 0;
    num = read (fd, reply, rlen);
    if (num < 0) {
        g_warning ("read error %d", errno);
        return 0;
    }
    print_buf ("<<<", reply, num);
    return num;
}

/* CTL service transaction ID */
static u8 ctl_tid = 1;

static int
get_meid (int fd)
{
    void *b;
    size_t blen;
    u8 dms_tid = 1;
    u16 dms_cid = 0;
    char reply[2048];
    size_t rlen;
    char meid[16];
    int err;

    /* Allocate a DMS client ID */
    b = qmictl_new_getcid (ctl_tid++, QMI_SVC_DMS, &blen);
    g_assert (b);

    rlen = send_and_wait_reply (fd, b, blen, reply, sizeof (reply));
    g_free (b);
    if (rlen <= 0)
        return 1;

    err = qmictl_getcid_resp (reply, rlen, &dms_cid);
    if (err < 0) {
        g_warning ("Failed to get DMS client ID: %d", err);
        return 1;
    }

    g_message ("DMS CID %d 0x%X", dms_cid, dms_cid);

    /* Get the MEID */
    b = qmidms_new_getmeid(dms_cid, dms_tid++, &blen);
    g_assert (b);

    rlen = send_and_wait_reply (fd, b, blen, reply, sizeof (reply));
    g_free (b);
    if (rlen <= 0)
        return 1;

    memset (meid, 0, sizeof (meid));
    err = qmidms_meid_resp (reply, rlen, meid, sizeof (meid) - 1);
    if (err < 0)
        g_warning ("Failed to get MEID: %d", err);
    else
        g_message ("MEID: %s", meid);

    /* Relese the DMS client ID */
    b = qmictl_new_releasecid (ctl_tid++, dms_cid, &blen);
    g_assert (b);

    rlen = send_and_wait_reply (fd, b, blen, reply, sizeof (reply));
    g_free (b);
    if (rlen <= 0)
        return 1;

    err = qmictl_releasecid_resp (reply, rlen);
    if (err < 0) {
        g_warning ("Failed to release DMS client ID: %d", err);
        return 1;
    }

    return 0;
}

static const char *
act_to_string (u8 act)
{
    switch (act) {
    case 0:
        return "no service";
    case 1:
        return "CDMA2000 1x";
    case 2:
        return "CDMA2000 HRPD/EVDO";
    case 3:
        return "AMPS";
    case 4:
        return "GSM";
    case 5:
        return "UMTS";
    case 8:
        return "LTE";
    default:
        break;
    }
    return "unknown";
}

static int
get_signal (int fd)
{
    void *b;
    size_t blen;
    u8 nas_tid = 1;
    u16 nas_cid = 0;
    char reply[2048];
    size_t rlen;
    int err;
    u8 dbm = 0, act = 0;

    /* Allocate a NAS client ID */
    b = qmictl_new_getcid (ctl_tid++, QMI_SVC_NAS, &blen);
    g_assert (b);

    rlen = send_and_wait_reply (fd, b, blen, reply, sizeof (reply));
    g_free (b);
    if (rlen <= 0)
        return 1;

    err = qmictl_getcid_resp (reply, rlen, &nas_cid);
    if (err < 0) {
        g_warning ("Failed to get NAS client ID: %d", err);
        return 1;
    }

    g_message ("NAS CID %d 0x%X", nas_cid, nas_cid);

    /* Get the signal strength */
    b = qminas_new_signal(nas_cid, nas_tid++, &blen);
    g_assert (b);

    rlen = send_and_wait_reply (fd, b, blen, reply, sizeof (reply));
    g_free (b);
    if (rlen <= 0)
        return 1;

    err = qminas_signal_resp (reply, rlen, &dbm, &act);
    if (err < 0)
        g_warning ("Failed to get signal: %d", err);
    else {
        g_message ("dBm: -%d", 0x100 - (dbm & 0xFF));
        g_message ("AcT: %s", act_to_string (act));
    }

    /* Relese the NAS client ID */
    b = qmictl_new_releasecid (ctl_tid++, nas_cid, &blen);
    g_assert (b);

    rlen = send_and_wait_reply (fd, b, blen, reply, sizeof (reply));
    g_free (b);
    if (rlen <= 0)
        return 1;

    err = qmictl_releasecid_resp (reply, rlen);
    if (err < 0) {
        g_warning ("Failed to release NAS client ID: %d", err);
        return 1;
    }

    return 0;
}

int main(int argc, char *argv[])
{
    int fd;
    void *b;
    size_t blen;
    u8 ctl_tid = 1;
    char reply[2048];
    size_t rlen;
    int err;

    if (argc != 2) {
        g_warning ("usage: %s <port>", argv[0]);
        return 1;
    }

    errno = 0;
    fd = open (argv[1], O_RDWR | O_EXCL | O_NONBLOCK | O_NOCTTY);
    if (fd < 0) {
        g_warning ("%s: open failed: %d", argv[1], errno);
        return 1;
    }

    /* Send the ready request */
    b = qmictl_new_version_info (ctl_tid++, &blen);
    g_assert (b);

    rlen = send_and_wait_reply (fd, b, blen, reply, sizeof (reply));
    g_free (b);
    if (rlen <= 0)
        goto out;

    err = qmictl_version_info_resp (reply, rlen);
    if (err < 0) {
        g_warning ("Failed to get version info: %d", err);
        goto out;
    }

    get_meid (fd);
    get_signal (fd);

out:
    close (fd);
    return 0;
}

