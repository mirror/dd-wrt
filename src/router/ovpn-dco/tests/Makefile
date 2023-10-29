# SPDX-License-Identifier: GPL-2.0
# Copyright (C) 2020-2023 OpenVPN, Inc.
#
#  Author:	Antonio Quartulli <antonio@openvpn.net>

RM ?= rm -f
CFLAGS = -Wall


ovpn-cli: ovpn-cli.c
	$(CC) $(CFLAGS) $@.c -I../include/uapi \
		`pkg-config --cflags --libs libnl-3.0 libnl-genl-3.0` \
		-lmbedtls -lmbedcrypto -Wall -o $@

clean:
	$(RM) ovpn-cli
