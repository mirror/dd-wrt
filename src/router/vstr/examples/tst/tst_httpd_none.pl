#! /usr/bin/perl -w

use strict;

push @INC, "$ENV{SRCDIR}/tst";

require 'httpd_tst_utils.pl';

our $conf_args_strict;
our $root;

setup();

# V
my $conf_arg = $conf_args_strict;
my $nargs  = $conf_arg . " --unspecified-hostname=default ";
   $nargs .= "--mime-types-main=$ENV{SRCDIR}/mime_types_extra.txt ";
   $nargs .= "--mime-types-xtra=$ENV{SRCDIR}/tst/ex_httpd_bad_mime ";
   $nargs .= "--virtual-hosts=true ";
   $nargs .= "--keep-alive=false ";
   $nargs .= "--range=false ";
   $nargs .= "--gzip-content-replacement=false ";
   $nargs .= "--error-406=false ";
   $nargs .= "--defer-accept=1 ";
   $nargs .= "--max-connections=32 ";
   $nargs .= "--max-header-sz=2048 ";
   $nargs .= "--nagle=true ";
   $nargs .= "--host=127.0.0.2 ";
   $nargs .= "--idle-timeout=16 ";
   $nargs .= "--dir-filename=welcome.html ";
   $nargs .= "--accept-filter-file=$ENV{SRCDIR}/tst/ex_httpd_null_tst_1 ";
   $nargs .= "--server-name='Apache/2.0.40 (Red Hat Linux)' ";
   $nargs .= "--canonize-host=true ";
   $nargs .= "--error-host-400=false ";

   $nargs .= "--configuration-data-jhttpd";
   $nargs .= " '(policy <default> (MIME/types-default-type bar/baz))' ";

daemon_init("ex_httpd", $root, $nargs);
http_cntl_list();
all_none_tsts();
daemon_exit();


rmtree($root);

success();

