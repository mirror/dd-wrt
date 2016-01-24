#! /usr/bin/perl -w

use strict;

my $conf_fc4_die = 0; # 1, Kills fc4 kernels


push @INC, "$ENV{SRCDIR}/tst";

require 'httpd_tst_utils.pl';

our $conf_args_nonstrict;
our $truncate_segv;
our $root;

setup();

my $conf_arg = $conf_args_nonstrict;
my $args = $conf_arg . " --unspecified-hostname=default --mime-types-xtra=$ENV{SRCDIR}/mime_types_extra.txt ";

httpd_vhost_tst("--virtual-hosts=true  --mmap=false --sendfile=false" . $args);
$truncate_segv = 1;
httpd_vhost_tst("--virtual-hosts=true  --mmap=true  --sendfile=false" .
		" --procs=2" . $args);
$truncate_segv = 0;

my $fc4_doa = '';
if ($conf_fc4_die)
  { $fc4_doa = "--accept-filter-file=$ENV{SRCDIR}/tst/ex_sock_filter_out_1";}
httpd_vhost_tst("--virtual-hosts=true --mmap=false --sendfile=true" .
		$fc4_doa . $args);

rmtree($root);

success();

