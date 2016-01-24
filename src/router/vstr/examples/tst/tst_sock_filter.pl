#! /usr/bin/perl -w

use strict;

push @INC, "$ENV{SRCDIR}/tst";
require 'vstr_tst_examples.pl';

run_tst("ex_sock_filter");

run_tst("ex_sock_filter", undef, "--mmap");

run_tst("ex_sock_filter", "ex_sock_filter_help", "--help");
run_tst("ex_sock_filter", "ex_sock_filter_version", "--version");

success();
