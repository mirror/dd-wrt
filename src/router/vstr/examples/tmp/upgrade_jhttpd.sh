#! /bin/sh -e

rm -f /usr/sbin/jhttpd && cp ex_httpd /usr/sbin/jhttpd && service jhttpd softrestart

