#! /usr/bin/perl -w

use strict;
use FileHandle;

my $docs = undef;

if (0) {}
elsif (-x "../configure") # In docs dir...
  {
    $docs ="../Documentation";
  }
elsif (-x "../../configure") # in build subdir
  {
    $docs ="../../Documentation";
  }

if (!defined ($docs))
  {
    STDERR->print("Can't find configure.\n");
    exit (1);
  }

my $hdr_date = `date '+%d-%b-%Y'`;
chomp($hdr_date);

my $hdr_ver = undef;
if (!open(VER, "< ./VERSION") && !open(VER, "< ../VERSION"))
  { die "open(VERSION: $!"; }
$_ = <VER>; chomp; $hdr_ver = $_;
close(VER);


my $man_funcs_header = <<EOL;
.TH vstr 3 "$hdr_date" "Vstr $hdr_ver" "Vstr String Library"
.SH "SYNOPSIS"
.in \\w'  'u
#include <vstr.h>
.sp
.NH
EOL

my $man_consts_header = <<EOL;
.TH vstr_const 3 "$hdr_date" "Vstr $hdr_ver" "Vstr String Library"
.SH "SYNOPSIS"
.in \\w'  'u
#include <vstr.h>
.sp
.NH
EOL

my $man_funcs_desc = <<EOL;
.ti
.HY
.SH "DESCRIPTION"
 A very simple overview is that you call vstr_init() at the start of your
program and vstr_exit() at the end.
 You can make new Vstr strings by calling vstr_make_base(), and free them by
calling vstr_free_base(). There are also a vstr_dup_* set of functions to
make a new Vstr string with data in them.
 You can then add/delete data from this string, using the provided functions,
if you need to use all or part of the string with a "C string" interface
then you can call vstr_export_cstr_ptr() or vstr_export_cstr_malloc().
EOL

my $man_consts_desc = <<EOL;
.ti
.HY
.SH "DESCRIPTION"
EOL

my $man_funcs_seealso = <<EOL;
.SH "SEE ALSO"
.BR vstr_const (3)
EOL

my $man_consts_seealso = <<EOL;
.SH "SEE ALSO"
.BR vstr (3)
EOL

sub synopsis
  {
    my $funcs = shift;
    my $func = undef;
    my $args = 0;

    sub fin
      {
	my $funcs = shift;
	my $args = shift;

	if (!$funcs) { return; }
	if (!$args) { die "Parameter missing in docs"; }
	OUT->print(");\n");
      }

    while (<IN>)
      {
	if (s!^(Function): (.*)\(\)$!$2! ||
	    s!^(Constant|Member): (.*)$!$2!)
	  {
	    chomp;

	    if (!$funcs)
	      {
		OUT->print(".br\n");
		OUT->print(".ti \\w'  'u\n");
		OUT->print("\\fB$_\\fR\n");
	      }
	    else
	      {
		if ($func) { fin($funcs, $args); }
		$args = 0;
		$func = $_;
	      }
	    if (( $funcs && $1 ne "Function") ||
		(!$funcs && $1 eq "Function") ||
		0)
	      { die "Bad txt documentation."; }
	  }
	elsif ($funcs && /^ Type: (.*)/)
	  {
	    my $spc = " ";

	    $_ = $1;
	    chomp;

	    if (/\*$/)
	      { $spc = ""; }

	    OUT->print(".br\n"); # FIXME: \w doesn't account for space filling
	    OUT->print(".in \\w'  $_$spc\\fB$func\\fR('u\n");
	    OUT->print(".ti \\w'  'u\n");
	    OUT->print("$_$spc\\fB$func\\fR(");
	  }
	elsif ($funcs && /^ Type\[.+\]: (.*)/)
	  {
	    $_ = $1;
	    chomp;
	    if ($args)
	      {
		OUT->print(", ");
	      }

	    ++$args;
	    OUT->print("$1");
	  }
	elsif (/^Section:/)
	  {
	    if ($func) { fin($funcs, $args); }
	    $args = 0;
	    $func = 0;
	    OUT->print(".sp\n");
	  }
      }

    fin($funcs, $args);
    OUT->print("\n");
  }

sub convert()
  {
    my $in_pre_tag = "";
    my $in_const = 0;

    while (<IN>)
      {
	my $next_in_const = 0;

	my $beg_replace = ".br\n";

	if ($in_const)
	  {
	    $beg_replace = "\n";
	  }

	if (s!^(Constant|Function|Member): (.*)$!$beg_replace\\fB$1: \\fR $2! ||
	    s!^ Explanation:\s*$!$beg_replace\\fBExplanation:\\fR! ||
	    s!^ Note:\s*$!.sp\n\\fBNote:\\fR! ||
	    s!^Section:\s*(.*)$!.SH $1! ||
	    0)
	  {
	    if (defined ($1) && ($1 eq "Function"))
              {
	        $_ = ".ti -2\n" . $_;
              }
	    if (defined ($1) && ($1 eq "Constant"))
	      {
		$next_in_const = 1;
	      }
	  }
	elsif (m!^ ([A-Z][a-z]+)(\[\d\]|\[ \.\.\. \])?: (.*)$!)
	  {
	    if (defined $2)
	      {
		if ($1 eq "Type")
		  {
		    $_ = ".br\n$1\\fB$2\\fR: $3\n";
		  }
		else
		  {
		    $_ = ".br\n$1\\fB$2\\fR: $3\n";
		  }
	      }
	    else
	      {
		if ($1 eq "Type")
		  {
		    $_ = ".br\n$1: $3\n";
		  }
		else
		  {
		    $_ = ".br\n$1: $3\n";
		  }
	      }
	  }
	elsif (/^ \.\.\./)
	  {
	    if (/\.\.\.$/)
	      {
		$_ = ".Ve\n$_.Vb 4\n";
		$in_pre_tag = "</pre>";
	      }
	    else
	      {
		$_ = ".Ve\n$_";
		$in_pre_tag = "";
	      }
	  }
	elsif (/\.\.\.$/)
	  {
	    $_ = "$_\n.Vb 4";
	    $in_pre_tag = "\n.Ve";
	  }
	elsif (!$in_pre_tag)
	  {
	    if (!/^$/)
	      {
		chomp;
		if (/^  /)
		  {
		    $_ = "\n.br\n" . $_;
		  }
	      }
	  }

	$in_const = $next_in_const;

	OUT->print($_);
      }
  }

# MAIN

# functions man page...
open (IN, "< $docs/functions.txt") || die "open(functions.txt): $!";

open (OUT, "> functions.3")        || die "open(functions.3): $!";

OUT->print($man_funcs_header);

synopsis(1);

open (IN, "< $docs/functions.txt") || die "open(functions.txt): $!";

OUT->print($man_funcs_desc);

convert();

OUT->print($man_funcs_seealso);

# constants man page...
open (IN, "< $docs/constants.txt") || die "open(constants.txt): $!";

open (OUT, "> constants.3")        || die "open(constants.3): $!";

OUT->print($man_consts_header);

synopsis(0);

open (IN, "< $docs/constants.txt") || die "open(constants.txt): $!";

OUT->print($man_consts_desc);

convert();

OUT->print($man_consts_seealso);

exit (0);
