/*
 * Copyright (C) 2010 Red Hat, Inc. All rights reserved.
 *
 * This copyrighted material is made available to anyone wishing to use,
 * modify, copy, or redistribute it subject to the terms and conditions
 * of the GNU Lesser General Public License v.2.1.
 */
#ifndef _LVM_CLOG_COMPAT_H
#define _LVM_CLOG_COMPAT_H

/*
 * The intermachine communication structure version are:
 *	0: Unused
 *	1: Never in the wild
 *	2: RHEL 5.2
 *	3: RHEL 5.3
 *	4: RHEL 5.4, RHEL 5.5
 *	5: RHEL 6, Current Upstream Format
 */
#define CLOG_TFR_VERSION 5

int clog_request_to_network(struct clog_request *rq);
int clog_request_from_network(void *data, size_t data_len);

#endif /* _LVM_CLOG_COMPAT_H */
