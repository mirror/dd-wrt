#! /usr/bin/perl -w

use strict;

push @INC, "$ENV{SRCDIR}/tst";

require 'httpd_tst_utils.pl';

our $conf_args_strict;
our $root;

setup();

# V
my $conf_arg = $conf_args_strict;
my $args = $conf_arg . " --unspecified-hostname=default --mime-types-xtra=$ENV{SRCDIR}/mime_types_extra.txt ";

daemon_init("ex_httpd", $root, "--pid-file=$root/abcd" . $args);
my $abcd = daemon_get_io_r("$root/abcd");
chomp $abcd;
if (daemon_pid() != $abcd) { failure("pid doesn't match pid-file"); }
all_nonvhost_tsts();
daemon_exit();

rmtree($root);

success();

