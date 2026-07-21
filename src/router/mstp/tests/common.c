/* SPDX-License-Identifier: GPL-2.0-or-later */

#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <asm/byteorder.h>
#include <linux/if_bridge.h>
#include <linux/if_ether.h>

#include "common.h"

#include "mstp.h"
#include "log.h"

#define LOG_STRING_LEN 256

typedef struct mock_port_s {
    port_t port;

    bpdu_t last_tx_bpdu;
    size_t last_tx_bpdu_len;
    int num_tx;

    struct mock_port_s *link;
} mock_port_t;

static port_t *alloc_port(bridge_t *br, const char *name)
{
    mock_port_t *p;

    p = calloc(1, sizeof(*p));
    if (!p)
        return NULL;

    strlcpy(p->port.sysdeps.name, name, IFNAMSIZ);
    p->port.bridge = br;

    return &p->port;
}

static bridge_t *alloc_bridge(const char *name, __u64 mac)
{
    bridge_t *br;
    int i;

    br = calloc(1, sizeof(*br));
    if (!br)
        return NULL;

    strncpy(br->sysdeps.name, name, IFNAMSIZ - 1);
    for (i = 5; i >= 0; i--) {
        br->sysdeps.macaddr[i] = (mac & 0xffu);
        mac >>= 8;
    }

    return br;
}

int prepare_test(void **state)
{
    struct list_head *bridges = NULL;

    bridges = malloc(sizeof(*bridges));
    if (!bridges)
        return -1;
    INIT_LIST_HEAD(bridges);
    *state = bridges;

    return 0;
}

int teardown_test(void **state)
{
    struct list_head *bridges = *state;
    bridge_t *br, *next;

    log_level = 0;

    if (!bridges)
       return 0;

    list_for_each_entry_safe(br, next, bridges, list) {
        MSTP_IN_set_bridge_enable(br, false);
        MSTP_IN_delete_bridge(br);
        list_del(&br->list);
        free(br);
    }

    free(bridges);

    return 0;
}

int alloc_bridge_ports(void **state, bridge_t **br, const char *name, __u64 mac,
                       port_t *(*p)[], int num_ports)
{
    struct list_head *bridges = NULL;
    char port_name[IFNAMSIZ];
    int i;

    if (state)
        bridges = *state;

    *br = alloc_bridge(name, mac);
    if (!*br)
        return -1;

    if (bridges)
        list_add_tail(&(*br)->list, bridges);

    if (!MSTP_IN_bridge_create(*br, (*br)->sysdeps.macaddr)) {
        if (bridges)
            list_del(&(*br)->list);
        free(*br);
	*br = NULL;
        return -1;
    }

    for (i = 0; i < num_ports; i++) {
        /* name ports <bridgename>p<portid> */
        snprintf(port_name, IFNAMSIZ, "%sp%i", name, (i + 1));
        (*p)[i] = alloc_port(*br, port_name);
        if (!(*p)[i])
           break;
        if (!MSTP_IN_port_create_and_add_tail((*p)[i], i + 1))
           break;
    }

    if (i != num_ports) {
        if ((*p)[i])
            free((*p)[i]);

        if (bridges)
                list_del(&(*br)->list);

        /* will free all added ports */
        MSTP_IN_delete_bridge(*br);
        free(*br);
	*br = NULL;

        return -1;
    }

    return 0;
}

void test_one_second(void **state)
{
    struct list_head *bridges = *state;
    bridge_t *br;

    list_for_each_entry(br, bridges, list)
        MSTP_IN_one_second(br);
}

void link_ports(port_t *p1, port_t *p2)
{
    mock_port_t *mp1 = (mock_port_t *)p1;
    mock_port_t *mp2 = (mock_port_t *)p2;

    mp1->link = mp2;
    mp2->link = mp1;
}

void unlink_ports(port_t *p1, port_t *p2)
{
    mock_port_t *mp1 = (mock_port_t *)p1;
    mock_port_t *mp2 = (mock_port_t *)p2;

    assert_ptr_equal(mp1->link, mp2);
    assert_ptr_equal(mp2->link, mp1);

    mp1->link = mp2->link = NULL;
}

void set_port_state(port_t *port, bool up, int speed, bool duplex)
{
    mock_port_t *mp = (mock_port_t *)port;

    /* nothing to do if state is the same */
    if (port->sysdeps.up == up && port->sysdeps.speed == speed
        && port->sysdeps.duplex == duplex)
        return;

    port->sysdeps.up = up;
    port->sysdeps.speed = speed;
    port->sysdeps.duplex = duplex;
    MSTP_IN_set_port_enable(port, up, speed, duplex);

    /* if linked, also update the linked port */
    if (mp->link) {
        mp->link->port.sysdeps.up = up;
        mp->link->port.sysdeps.speed = speed;
        mp->link->port.sysdeps.duplex = duplex;
        MSTP_IN_set_port_enable(&mp->link->port, up, speed, duplex);
    }
}

void port_rx_bpdu(port_t *p, const void *data, size_t len)
{
    bpdu_t bpdu;

    assert_true(len <= sizeof(bpdu));

    /* MSTP_IN_rx_bpdu() may modify the BPDU, so we need a copy */
    memcpy(&bpdu, data, len);
    MSTP_IN_rx_bpdu(p, &bpdu, len);
}

int port_last_tx_bpdu(port_t *p, bpdu_t **data, size_t *len)
{
    mock_port_t *mp = (mock_port_t *)p;

    if (!mp->last_tx_bpdu_len)
        return -1;

    *data = &mp->last_tx_bpdu;
    *len = mp->last_tx_bpdu_len;

    return 0;
}
void port_reset_last_tx_bpdu(port_t *p)
{
    mock_port_t *mp = (mock_port_t *)p;

    mp->last_tx_bpdu_len = 0;
    memset(&mp->last_tx_bpdu, 0, sizeof(mp->last_tx_bpdu));
}

/* functions called by mstp.c */



void Dprintf(int level, const char *fmt, ...)
{
    char logbuf[LOG_STRING_LEN];
    logbuf[sizeof(logbuf) - 1] = 0;
    va_list ap;

    if (level > log_level)
        return;

    va_start(ap, fmt);
    vsnprintf(logbuf, sizeof(logbuf) - 1, fmt, ap);
    va_end(ap);
    printf("%s\n", logbuf);
}

void _ctl_err_log(char *fmt, ...)
{
    /* unused */
}

int log_level = 0;
int ctl_in_handler = 0;

void MSTP_OUT_set_state(per_tree_port_t *ptp, int new_state)
{
    ptp->state = new_state;
}

void MSTP_OUT_flush_all_fids(per_tree_port_t *ptp)
{
    MSTP_IN_all_fids_flushed(ptp);
}

void MSTP_OUT_set_ageing_time(port_t *prt, unsigned int ageingTime)
{
    /* nothing to do */
}

void MSTP_OUT_tx_bpdu(port_t *p, bpdu_t *bpdu, int size)
{
    mock_port_t *mp = (mock_port_t *)p;

    /* remember the last sent out BPDU */
    memcpy(&mp->last_tx_bpdu, bpdu, size);
    mp->last_tx_bpdu_len = size;
    mp->num_tx++;

    /* if linked to a port, forward it */
    if (mp->link)
        MSTP_IN_rx_bpdu(&mp->link->port, bpdu, size);
}

void MSTP_OUT_shutdown_port(port_t *prt)
{
    /* nothing to do */
}
