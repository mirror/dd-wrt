#!/usr/bin/perl -w
# ======================================================================
# log2xml - This script translates cacao -verbosecall output into a
#           more readable XML format. It also separates the output of
#           different threads and is able to ignore commonly called
#           (uninteresting) methods.
#
# Usage:
#     cacao -verbose -verbosecall ... | log2xml.pl
#
# For each thread the script writes a file LOG_{threadid}.xml.
#
# You can use any XML editor to browse the logs. If you want to use
# vim, which works well, take a look at log2xml.vim in this
# directory.
#
# You may want to edit the opt_ignore options below.
#
# Author  : Edwin Steiner
# Revision:
#
# $Log$
# Revision 1.5  2005/04/15 09:33:34  edwin
# preserve indentation of log text
#
# Revision 1.4  2005/04/15 09:06:54  edwin
# output more valid xml
#
# Revision 1.3  2005/04/14 20:11:04  edwin
# typo
#
# Revision 1.2  2005/04/14 20:10:20  edwin
# disabled debug print, added vim boilerplate
#
# Revision 1.1  2005/04/14 19:44:00  edwin
# added log2xml.pl and log2xml.vim
#
# ======================================================================

use strict;
use IO::File;

my @opt_ignorelist = (
	'java.lang.Character.toLowerCase(C)C',
	'java.lang.Character.toUpperCase(C)C',
	'java.lang.String.endsWith(Ljava/lang/String;)Z',
	'java.lang.String.equalsIgnoreCase(Ljava/lang/String;)Z',
	'java.lang.String.equals(Ljava/lang/Object;)Z',
	'java.lang.String.indexOf(I)I',
	'java.lang.String.indexOf(II)I',
	'java.lang.String.indexOf(Ljava/lang/String;)I',
	'java.lang.String.indexOf(Ljava/lang/String;I)I',
	'java.lang.String.lastIndexOf(Ljava/lang/String;)I',
	'java.lang.String.lastIndexOf(Ljava/lang/String;I)I',
	'java.lang.String.regionMatches(ILjava/lang/String;II)Z',
	'java.lang.String.regionMatches(ZILjava/lang/String;II)Z',
	'java.lang.String.replace(CC)Ljava/lang/String;',
	'java.lang.String.startsWith(Ljava/lang/String;)Z',
	'java.lang.String.substring(II)Ljava/lang/String;',
	'java.lang.String.toLowerCase()Ljava/lang/String;',
	'java.util.HashMap.get(Ljava/lang/Object;)Ljava/lang/Object;',
	'java.util.HashMap.put(Ljava/lang/Object;Ljava/lang/Object;)Ljava/lang/Object;',
	'java.util.Hashtable.clone()Ljava/lang/Object;',
	'java.util.Hashtable.get(Ljava/lang/Object;)Ljava/lang/Object;',
	'java.util.Hashtable.put(Ljava/lang/Object;Ljava/lang/Object;)Ljava/lang/Object;',
	'java.util.jar.JarFile$EntryInputStream.read([BII)I',
);

my @opt_ignore_regexps = (
	qr/^java\.lang\.Character\./,
	qr/^java\.lang\.String\./,
	qr/^java\.util\.jar\./,
	qr/^java\.util\.Locale\./,
	qr/^java\.util\.zip\./,
);

my $opt_details = 0;
my $opt_verbose = 1;
my $opt_no_text_for_ignored = 0;
my $opt_fileprefix = 'LOG_';

# per-thread hashes
my %stacks;
my %files;
my %ignore_level;

my %ignorehash = map { $_=>1 } @opt_ignorelist;

# ======================================================================
# OUTPUT, XML WRITING
# ======================================================================

# Quote the given text for putting it in an XML file.
sub quote ( $ )
{
	my ($s) = @_;
	$s =~ s/&/&amp;/g;
	$s =~ s/</&lt;/g;
	$s =~ s/>/&gt;/g;
	return $s;
}

sub write_xml ( $$ )
{
	my ($thread,$xml) = @_;
	my $file = $files{$thread};
	print $file $xml;
}

sub write_xml_call_info ( $$ )
{
	my ($thread,$node) = @_;
	my $file = $files{$thread};
	print $file ' return="' . quote($node->{RETURN}) . '"' if defined($node->{RETURN});
	print $file ' except="' . quote($node->{EXCEPTION}) . '"' if defined($node->{EXCEPTION});
	if ($opt_details) {
		print $file ' details="' . quote($node->{DETAILS}) . '"' if defined($node->{DETAILS});
	}
}

sub write_xml_frame ( $$ )
{
	my ($thread,$node) = @_;

	my $file = $files{$thread};
	print $file '<frame name="' . quote($node->{NAME}) . '"';
	write_xml_call_info($thread,$node);
	print $file "/>\n";
}

sub write_xml_enter ( $$ )
{
	my ($thread,$node) = @_;

	my $file = $files{$thread};
	print $file '<call name="' . quote($node->{NAME}) . '"';
	write_xml_call_info($thread,$node);
	print $file ">\n";
}

sub write_xml_leave ( $$ )
{
	my ($thread,$node) = @_;

	my $file = $files{$thread};
	print $file '<leave';
	write_xml_call_info($thread,$node);
	print $file ' name="' . quote($node->{NAME}) . '"';
	print $file "/>\n";
	print $file "</call>\n";
}

sub write_xml_never_returns ( $$ )
{
	my ($thread,$node) = @_;

	my $file = $files{$thread};
	print $file '<never_returns';
	print $file ' name="' . quote($node->{NAME}) . '"';
	print $file "/>\n";
	print $file "</call>\n";
}

sub write_xml_node ( $$ )
{
	my ($thread,$node) = @_;

	write_xml_enter($thread,$node);
	write_xml_body($thread,$node);
	write_xml_leave($thread,$node);
}

sub write_xml_body ( $$ )
{
	my ($thread,$node) = @_;

	for my $x (@{$node->{BODY}}) {
		if (ref $x) {
			write_xml_node($thread,$x);
		}
		else {
			my $file = $files{$thread};
			print $file $x;
		}
	}
}

sub write_xml_header ( $ )
{
	my ($file) = @_;
	print $file '<?xml version="1.0"?>'."\n";
	print $file "<thread>\n";
}

sub write_xml_end ( $ )
{
	my ($file) = @_;
	print $file "</thread>\n";
}

# ======================================================================
# HELPERS
# ======================================================================

# Return true if functions with this name are ignored.
sub is_ignored ( $ )
{
	my ($name) = @_;

	return 1 if $ignorehash{$name};

	for my $re (@opt_ignore_regexps) {
		return 1 if $name =~ $re;
	}

	return 0;
}

# ======================================================================
# STACK RECONSTRUCTION
# ======================================================================

sub make_root ( )
{
	return {NAME => 'root',
			DETAILS => '',
			BODY => [],
			RETURN => undef,
			EXCEPTION => undef,
			LINE => undef,
			};
}

sub thread_register ( $ )
{
	my ($thread) = @_;
	unless (exists $stacks{$thread}) {
		$stacks{$thread} = [make_root];
		my $filename = $opt_fileprefix . $thread . '.xml';
		$files{$thread} = IO::File->new(">$filename");
		$ignore_level{$thread} = 0;
		write_xml_header($files{$thread});
	}
}

sub stack_top ( $ )
{
	my ($thread) = @_;
	return $stacks{$thread}->[-1] or die "stack underflow";
}

sub stack_pop ( $ )
{
	my ($thread) = @_;
	return pop @{$stacks{$thread}} or die "stack underflow";
}

sub stack_push ( $$ )
{
	my ($thread,$value) = @_;
	push @{$stacks{$thread}},$value;
}

sub stack_slot ( $$ )
{
	my ($thread,$index) = @_;
	return $stacks{$thread}->[$index] or die "invalid stack index";
}

sub stack_write ( $ )
{
	my ($thread) = @_;
	print STDERR "$.\n";
	for (@{$stacks{$thread}}) {
		print STDERR "\t".quote($_->{NAME})."\n";
	}
}

sub process_call ( $$$ )
{
	my ($thread,$keyword,$rest) = @_;

	my $top = stack_top($thread);

	$rest =~ /(\S+?)  \( ([^)]*) \)  ([^ \t(]+)  (.*) $/x or die "could not match call log: $rest";
	my ($n,$args,$rettype,$details) = ($1,$2,$3,$4);
	my $name = "$n($args)$rettype";

	my $call = {NAME => $name,SHORTNAME => $n,DETAILS => $details,BODY => [],LINE => $.};

	$call->{FIRST} = 1 if $keyword eq '1st_call';

	stack_push($thread,$call);

	if ($ignore_level{$thread} == 0) {
		write_xml_enter($thread,$call);

		if (is_ignored($name)) {
			$ignore_level{$thread}++;
		}
	}
	else {
		$ignore_level{$thread}++;
	}
}

sub process_leave ( $$$ )
{
	my ($thread,$rest,$exception) = @_;

	my $top = stack_pop($thread);

	if ($exception) {
		$top->{EXCEPTION} = $exception;
	}
	else {
		$rest =~ /(\S+?) (\([^)]*\))?    (->(.*))?$/x or die "could not match finished log: $rest";
		my ($name,$return) = ($1,$4);

		$name eq $top->{NAME} or die "warning: mismatched leave:\n"
			."\t(line $.) $name from\n"
			."\t(line ".$top->{LINE}.") ".$top->{NAME}."\n";

		$top->{RETURN} = defined($return) ? $return : 'void';
	}

	--$ignore_level{$thread} if $ignore_level{$thread} > 0;

	if ($ignore_level{$thread} == 0) {
		write_xml_leave($thread,$top);
	}
}

sub process_exception ( $$$ )
{
	my ($thread,$exception,$rest) = @_;
	my ($name,$entry);

	my $top = stack_top($thread);

	if ($rest =~ /(\S+?) \([^)]*\)  \((0x[^)]+)\)/x) {
		($name,$entry) = ($1,$2);
	} 
	elsif ($rest =~ /call_java_method/) {
		$name = "call_java_method";
		$entry = 0;
	}
	else {
		 die "could not match exception: $exception in $rest";
	}
	
	$top->{ENTRYPOINT} = $entry unless defined($top->{ENTRYPOINT});

	if ($name eq $top->{SHORTNAME} && $entry eq $top->{ENTRYPOINT}) {
		if ($ignore_level{$thread} == 0) {
			my $handled = '<exception name="'.$exception.'"/>'."\n";
			write_xml($thread,$handled);
		}
		return; # exception handled locally
	}

	# unwind a stack frame

	while ($name ne 'call_java_method' && $name ne stack_slot($thread,-2)->{SHORTNAME}) {
		stack_write($thread);
		print STDERR "exception : $exception\n";
		print STDERR "method    : $name\n";
		print STDERR "entrypoint: $entry\n";
		print STDERR "scope     : ".$top->{SHORTNAME}."\n";
		print STDERR "scopeentry: ".$top->{ENTRYPOINT}."\n";
		print STDERR "outer     : ".stack_slot($thread,-2)->{SHORTNAME}."\n";
		print STDERR "outerentry: ".stack_slot($thread,-2)->{ENTRYPOINT}."\n";
		print STDERR "unwinding stack...\n";

		warn "heuristic unwind: $exception in $rest";
		process_leave($thread,$rest,$exception);
	}

	process_leave($thread,$rest,$exception);
}

sub process_call_log ( $$$ )
{
	my ($thread,$keyword,$rest) = @_;

	if ($keyword eq 'called' || $keyword eq '1st_call') {
		process_call($thread,$keyword,$rest);
	}
	elsif ($keyword eq 'finished') {
		process_leave($thread,$rest,undef);
	}
	else {
		die "invalid log keyword: $keyword";
	}
}

sub process_text ( $$ )
{
	my ($thread,$text) = @_;

	# print STDERR "$.: $text\n";

	if ($opt_no_text_for_ignored && $ignore_level{$thread} > 0) {
		return;
	}

	my $top = stack_top($thread);

	my $file = $files{$thread};
	print $file quote($text)."\n";
}

# ======================================================================
# MAIN PROGRAM
# ======================================================================

sub main
{
	eval {
		my $lastthread;
		while (<>) {
			chomp($_);
			if (/LOG: \[(\S+)\](\s*)(.*)/) {
				my ($thread,$space,$log) = ($1,$2,$3);
				thread_register($thread);
				$lastthread = $thread;
				if ($log =~ /(1st_call|called|finished):\s*(.*)/) {
					process_call_log($thread,$1,$2);
				}
				elsif ($log =~ /Exception\s+(\S+)\s+thrown in\s+(.*)/) {
					process_exception($thread,$1,$2);
				}
				else {
					process_text($thread,$space.$log);
				}
			}
			else {
				unless (defined($lastthread)) {
					$lastthread = '(nil)';
					thread_register($lastthread);
				}
				process_text($lastthread,$_);
			}
		}
	};
	
	if ($@) {
	    warn "error: $@";
	    warn "warning: omitting the rest of the input";
	}
	    
	for my $thread (keys %stacks) {
		my $ign = $ignore_level{$thread};
		unless ($ign == 0) {
		    warn "warning: ignore_level not reset to 0 at end of input";
		    write_xml($thread,"\n<!-- LOG ENDS IN IGNORED METHOD - STACKTRACE: -->\n\n");
		    while ($ign > 0) {
				my $top = $stacks{$thread}->[-$ign];
				write_xml_frame($thread,$top);
				$ign--;
		    }
			pop @{$stacks{$thread}} for 1..$ignore_level{$thread};
		}
		while (scalar @{$stacks{$thread}} > 1) {
				my $top = $stacks{$thread}->[-1];
				write_xml_never_returns($thread,$top);
				pop @{$stacks{$thread}};
		}
		write_xml_end($files{$thread});
		$files{$thread}->close();
	}
	print STDERR "processed $. lines\n";
}

main();

# vim: noet ts=4 sw=4 ai

