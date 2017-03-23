#! /usr/bin/env python

import re
import sys
import urllib2


def parse_blacklist(content, trusted=False):
    rx_comment = re.compile(r'^(#|$)')
    rx_u = re.compile(r'^@*\|\|([a-z0-9.-]+[.][a-z]{2,})\^?(\$(popup|third-party))?$')
    rx_l = re.compile(r'^([a-z0-9.-]+[.][a-z]{2,})$')
    rx_h = re.compile(r'^[0-9]{1,3}[.][0-9]{1,3}[.][0-9]{1,3}[.][0-9]{1,3}\s+([a-z0-9.-]+[.][a-z]{2,})$')
    rx_mdl = re.compile(r'^"[^"]+","([a-z0-9.-]+[.][a-z]{2,})",')
    rx_b = re.compile(r'^([a-z0-9.-]+[.][a-z]{2,}),.+,[0-9: /-]+,')
    rx_trusted = re.compile(r'^([*a-z0-9.-]+)$')

    names = set()
    rx_set = [rx_u, rx_l, rx_h, rx_mdl, rx_b]
    if trusted:
        rx_set = [rx_trusted]
    for line in content.splitlines():
        line = str.lower(str.strip(line))
        if rx_comment.match(line):
            continue
        for rx in rx_set:
            matches = rx.match(line)
            if not matches:
                continue
            name = matches.group(1)
            names.add(name)
    return names


def blacklist_from_url(url):
    sys.stderr.write("Loading data from [{}]\n".format(url))
    req = urllib2.Request(url)
    trusted = False
    if req.get_type() == "file":
        trusted = True
    response = None
    try:
        response = urllib2.urlopen(req, timeout=10)
    except urllib2.URLError as err:
        sys.stderr.write("[{}] could not be loaded: {}\n".format(err))
        exit(1)
    if trusted is False and response.getcode() != 200:
        sys.stderr.write("[{}] returned HTTP code {}\n".format(url, response.getcode()))
        exit(1)
    content = response.read()
    return parse_blacklist(content, trusted)


def name_cmp(name):
    parts = name.split('.')
    parts.reverse()
    return str.join('.', parts)


def has_suffix(names, name):
    parts = str.split(name, ".")
    while parts:
        parts = parts[1:]
        if str.join(".", parts) in names:
            return True

    return False


def blacklists_from_config_file(file):
    blacklists = {}
    all_names = set()
    unique_names = set()

    with open(file) as fd:
        for line in fd:
            line = str.strip(line)
            if str.startswith(line, "#") or line == "":
                continue
            url = line
            names = blacklist_from_url(url)
            blacklists[url] = names
            all_names |= names

    for url, names in blacklists.items():
        print("\n\n########## Blacklist from {} ##########\n".format(url))
        ignored = 0
        list_names = list()
        for name in names:
            if has_suffix(all_names, name) or name in unique_names:
                ignored = ignored + 1
            else:
                list_names.append(name)
                unique_names.add(name)

        list_names.sort(key=name_cmp)
        if ignored:
            print("# Ignored duplicates: {}\n".format(ignored))
        for name in list_names:
            print(name)


blacklists_from_config_file("domains-blacklist.conf")
