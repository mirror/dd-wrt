#!/usr/bin/env python
# -*- Mode: python; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-
#
# This program is free software; you can redistribute it and/or modify it under
# the terms of the GNU General Public License as published by the Free Software
# Foundation, version 2.
#
# This program is distributed in the hope that it will be useful, but WITHOUT
# ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
# FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
# details.
#
# You should have received a copy of the GNU General Public License along with
# this program; if not, write to the Free Software Foundation, Inc., 51
# Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
#
# Copyright (C) 2011 - 2012 Red Hat, Inc.
#

def constname(name):
    rlist = { '(': '',
              ')': '',
              ' ': '_',
              '/': '_',
              '.': '',
              ',': '' }
    # Capture stuff like "-9 dB" correctly
    if name.find("dB") == -1:
        rlist['-'] = '_'
    else:
        rlist['-'] = 'NEG_'

    sanitized = ""
    for c in name:
        try:
            sanitized += rlist[c]
        except KeyError:
            sanitized += c

    # Handle E_UTRA -> EUTRA and E_UTRAN -> EUTRAN
    foo = sanitized.upper().strip('_')
    foo.replace('E_UTRA', 'EUTRA')

    # And HSDPA+ -> HSDPA_PLUS then strip all plusses
    foo = foo.replace('HSDPA+', 'HSDPA_PLUS')
    foo = foo.replace('+', '')

    # 1xEV-DO -> 1x EVDO
    foo = foo.replace("1XEV_DO", "1X_EVDO")
    foo = foo.replace("EV_DO", "EVDO")

    # Wi-Fi -> WiFi
    foo = foo.replace("WI_FI", "WIFI")

    # modify certain words
    words = foo.split('_')
    klist = [ 'UNSUPPORTED' ]
    slist = [ 'STATES', 'REASONS', 'FORMATS', 'SELECTIONS', 'TYPES', 'TAGS',
              'PROTOCOLS', 'CLASSES', 'ACTIONS', 'VALUES', 'OPTIONS',
              'DOMAINS', 'DEVICES', 'MODES', 'MONTHS', 'PREFERENCES',
              'PREFS', 'DURATIONS', 'CAPABILITIES', 'INTERFACES',
              'TECHNOLOGIES', 'NETWORKS', 'DAYS', 'SYSTEMS', 'SCHEMES',
              'INDICATORS', 'ENCODINGS', 'INITIALS', 'BITS', 'MEDIUMS',
              'BASES', 'ERRORS', 'RESULTS', 'RATIOS', 'DELIVERIES',
              'FAMILIES', 'SETTINGS', 'SOURCES', 'ORDERS' ]
    blist = { 'CLASSES': 'CLASS' }
    final = ''
    for word in words:
        if word in klist or len(word) == 0:
            continue
        if len(final):
            final += '_'
        if word in blist:
            final += blist[word]
        elif word in slist:
            if word.endswith("IES"):
                final += word[:len(word) - 3] + "Y"
            elif word.endswith("S"):
                final += word[:len(word) - 1]
        else:
            final += word
    return final

def nicename(name):
    name = name.lower()
    name = name.replace("1xev-do", "1x evdo")
    name = name.replace("ev-do", "evdo")
    name = name.replace("wi-fi", "wifi")
    name = name.replace("%", "pct")
    name = name.replace(' ', '_').replace('/', '_').replace('-', '_').replace('.','').replace('(', '').replace(')', '')
    name = name.replace("___", "_").replace("__", "_")
    return name.strip('_')


class DbFile:
    # Base class for objects that handle reading a database file like
    # Enum.txt or Struct.txt

    def __init__(self, path):
        raise Exception("init() method must be implemented")

    def validate(self):
        pass

    def has_child(self, cid):
        raise Exception("has_child() method must be implemented")

    def get_child(self, cid):
        raise Exception("get_child() method must be implemented")

    def emit(self):
        pass

    def emit_unused(self, used, fields, enums):
        pass

