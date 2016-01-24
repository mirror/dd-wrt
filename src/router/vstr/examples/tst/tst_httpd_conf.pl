#! /usr/bin/perl -w

use strict;

push @INC, "$ENV{SRCDIR}/tst";

require 'httpd_tst_utils.pl';

our $truncate_segv;
our $root;

setup();

$truncate_segv = $ENV{VSTR_TST_HTTP_TRUNC_SEGV};
$truncate_segv = 1 if (!defined ($truncate_segv));

# quick tests...
if ($ENV{VSTR_TST_FAST}) {
  conf_tsts(6, 6);
  success();
}

my $old_truncate_segv = $truncate_segv;
$truncate_segv = 1; # Stop gen tests to save time...

conf_tsts($_, $_) for (1..6);

$truncate_segv = $old_truncate_segv;
conf_tsts(1, 6); # Now do all of them at once...

rmtree($root);

success();

