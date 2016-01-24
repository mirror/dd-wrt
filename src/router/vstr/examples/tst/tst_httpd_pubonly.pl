#! /usr/bin/perl -w

use strict;

push @INC, "$ENV{SRCDIR}/tst";

require 'httpd_tst_utils.pl';

our $conf_args_strict;
our $root;

setup();

my $conf_arg = $conf_args_strict;
my $args = $conf_arg . " --mime-types-xtra=$ENV{SRCDIR}/mime_types_extra.txt ";

daemon_init("ex_httpd", $root, "-d -d -d --virtual-hosts=true " .
	    "--public-only=true" . $args);
all_public_only_tsts("no gen tsts");
daemon_exit();

rmtree($root);

success();

