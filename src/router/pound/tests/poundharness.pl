# This file is part of pound testsuite
# Copyright (C) 2018-2024 Sergey Poznyakoff
#
# Pound is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 3 of the License, or
# (at your option) any later version.
#
# Pound is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with pound.  If not, see <http://www.gnu.org/licenses/>.
use strict;
use warnings;
use Socket qw(:DEFAULT :crlf);
use threads;
use threads::shared;
use Getopt::Long;
use HTTP::Tiny;
use POSIX qw(:sys_wait_h);
use Cwd qw(abs_path);
use File::Spec;

my $config = 'pound.cfi:pound.cfg';
my @preproc_files;
my $log_level = 2;
my $log_file = 'pound.log';
my $pid_file = 'pound.pid';
my $include_dir;
my $transcript_file;
my $verbose;
my $dry_run;
my $listeners = ListenerList->new(1);
my $backends = ListenerList->new(0);
my $pound_pid;
my $startup_timeout = 2;
my $statistics;

use constant {
    EX_SUCCESS => 0,
    EX_FAILURE => 1,
    EX_ERROR => 2,
    EX_USAGE => 3,
    EX_SKIP => 77
};

## Early checks
## ------------

# Check for perl version.
eval "require 5.026_003";
if ($@) {
    print STDERR "$@";
    exit(EX_SKIP);
}

# Check for external modules
eval "require IO::FDPass";
if ($@) {
    print STDERR "required module IO::FDPass not present\n";
    exit(EX_SKIP);
}

sub cleanup {
    if ($pound_pid) {
	if ($verbose) {
	    print "Stopping pound ($pound_pid)\n";
	}
	kill 'HUP', $pound_pid;
    }
}

## Signal handling and program cleanup
## -----------------------------------

my %status_codes;

$SIG{QUIT} = $SIG{HUP} = $SIG{TERM} = $SIG{INT} = \&cleanup;
sub sigchild {
    my $pid = waitpid(-1, 0);
    if ($pid != $pound_pid) {
	$status_codes{$pid} = $?;
	$SIG{CHLD} = \&sigchild;
	return;
    }
    $pound_pid = 0;
    if (WIFEXITED($?)) {
	if (WEXITSTATUS($?)) {
	    print STDERR "pound terminated with code " . WEXITSTATUS($?) . "\n";
	} else {
	    if ($verbose) {
		print "pound finished\n";
	    }
	    $SIG{CHLD} = \&sigchild;
	    return;
	}
    } elsif (WIFSIGNALED($?)) {
	print STDERR "pound terminated on signal " . WTERMSIG($?) . "\n";
    } else {
	print STDERR "pound terminated with unrecognized status " . $? . "\n";
    }
    exit(EX_ERROR);
};
$SIG{CHLD} = \&sigchild;

END {
    cleanup;
}

## Start the program
## -----------------
sub usage_error {
    my $msg = shift;
    print STDERR "$msg\n";
    exit(EX_USAGE);
}

GetOptions('config|f=s' => \$config,
	   'preproc=s@' => \@preproc_files,
	   'verbose|v+' => \$verbose,
	   'log-level|l=n' => \$log_level,
	   'transcript|x=s' => \$transcript_file,
	   'statistics|s' => \$statistics,
	   'startup-timeout|t=n' => \$startup_timeout,
	   'include-dir=s' => \$include_dir)
    or exit(EX_USAGE);

my $script_file = shift @ARGV or usage_error "required parameter missing";
usage_error "too many arguments\n" if @ARGV;

foreach my $file (@preproc_files) {
    if ($file =~ m{(?<src>.+):(?<dst>.+)}) {
	preproc($+{src}, $+{dst});
    } else {
	preproc($file, "$file.cfg");
    }
}

if ($config =~ m{(?<src>.+):(?<dst>.+)}) {
    preproc($+{src}, $+{dst}, 1);
    $config = $+{dst};
} else {
    my $ofile = 'pound.cfg';
    preproc($config, $ofile, 1);
    $config = $ofile;
}

if ($include_dir) {
    $config = abs_path($config)
}

# Start HTTP listeners
$backends->read_and_process;

# Start pound
runner();

# Parse and run the script file
my $ps = PoundScript->new($script_file, $transcript_file);
$ps->parse;

# Terminate

if ($statistics) {
    print "Total tests: ".$ps->tests. "\n";
    print "Failures: ".$ps->failures. "\n";
} elsif ($ps->failures) {
    print STDERR $ps->failures . " from " . $ps->tests . " tests failed\n";
}
exit ($ps->failures ? EX_FAILURE : EX_SUCCESS);

## Configuration file processing
## -----------------------------

sub dequote {
    my $arg = shift;
    $arg =~ s/^\"(.+)\"$/$1/;
    return $arg;
}

sub addport {
    my ($infile, $outfile, $port) = @_;
    open(my $in, '<', $infile)
	or die "can't open $infile for reading: $!";
    open(my $out, '>', $outfile)
	or die "can't create $outfile: $!";
    while (<$in>) {
	chomp;
	print $out "$_:$port\n";
    }
    close $in;
    close $out;
}

sub preproc {
    my ($infile, $outfile, $init, $initstate) = @_;
    if (!$init && $include_dir) {
	unless (File::Spec->file_name_is_absolute($infile)) {
	    $infile = File::Spec->catfile($include_dir, $infile)
	}
	unless (File::Spec->file_name_is_absolute($outfile)) {
	    $outfile = File::Spec->catfile($include_dir, $outfile)
	}
    }
    if ($verbose) {
	print "Preprocessing $infile into $outfile\n";
    }
    open(my $in, $infile)
	or die "can't open $infile for reading: $!";
    open(my $out, '>', $outfile)
	or die "can't create $outfile: $!";

    use constant {
	ST_INIT => 0,
	ST_LISTENER => 1,
	ST_SERVICE => 2,
	ST_BACKEND => 3,
	ST_SESSION => 4,
	ST_SECTION => 5
    };
    my @state;
    unshift @state, $initstate // ST_INIT;
    if ($init) {
	print $out <<EOT;
# Initial settings by $0
Daemon 0
LogFacility -
EOT
    ;
	if ($log_level >= 0) {
	    print $out "LogLevel $log_level\n";
	}
    }
    my $be;
    while (<$in>) {
	chomp;
	if (/^\s*(?:#.)?$/) {
	    ;
	} elsif (/^\s*(Daemon|LogFacility)/i) {
	    $_ = "# Commented out: $_";
	} elsif ($log_level >= 0 && /^\s*LogLevel/i) {
	    $_ = "# Commented out: $_";
	} elsif (/^(\s*)Listen(HTTPS?)/i) {
	    unshift @state, ST_LISTENER;
	    print $out "$_\n";
	    my $lst = $listeners->create("$infile:$.", $2);
	    if ($verbose) {
		print $lst->ident . ": Listener ".$lst->address."\n";
	    }
	    print $out "# Addition by $0\n";
	    print $out "$1\tSocketFrom \"".$lst->sockname."\"\n";
	    next;
	} elsif (/^(?<indent>\s*)Include\s+(?<arg>.+)/) {
	    my $arg = dequote($+{arg});
	    my $file = $arg . '.pha';
	    preproc($arg, $file, 0, $state[0]);
	    print $out "$+{indent}# Edited by $0\n";
	    $_ = "$+{indent}Include \"$file\"";
	} elsif (/^\s*Service/i) {
	    unshift @state, ST_SERVICE;
	} elsif (/^\s*Session/i) {
	    unshift @state, ST_SESSION;
	} elsif (/^\s*(Backend|Emergency)/i) {
	    unshift @state, ST_BACKEND;
	    $be = $backends->create("$infile:$.");
	    if ($verbose) {
		print "$infile:$.: Backend ".$be->ident . ": " . $be->address."\n";
	    }
	} elsif (/^s*(TrustedIP|ACL|CombineHeaders)\b/) {
	    unshift @state, ST_SECTION
	} elsif (/^\s*End/i) {
	    shift @state
	} elsif ($state[0] == ST_BACKEND) {
	    if (/^(\s*Address)/i) {
		$_ = $1 . ' ' . $be->host;
	    } elsif (/^(\s*Port)/i) {
		$_ = $1 . ' ' . $be->port;
	    }
	} elsif ($state[0] == ST_LISTENER) {
	    if (/^\s*(Address|Port|SocketFrom)/i) {
		    $_ = "# Commented out: $_";
	    }
	} elsif ($state[0] == ST_SERVICE) {
	    if (/^(\s*)Host\s+(.+)\s*$/) {
		if (my $lst = $listeners->last()) {
		    my $indent = $1;
		    my $hostline = $2;
		    my @opts;
		    my $exact = 1;
		    my $file;
		    while ($hostline =~ m/^\s*(-\S+)(.*)/) {
			push @opts, $1;
			if ($1 eq '-re' || $1 eq '-beg' || $1 eq '-end') {
			    $exact = 0;
			} elsif ($1 eq '-file') {
			    $file = 1;
			}
			$hostline = $2;
		    }
		    $hostline =~ s/^\s+//;
		    $hostline = dequote($hostline);
		    if ($exact || @opts == 0) {
			if ($file) {
			    my $ofile = $hostline . '.port';
			    addport($hostline, $ofile, $lst->port);
			    $hostline = $ofile;
			} else {
			    $hostline = $hostline . ':' . $lst->port;
			}
			print $out "$indent# Edited by $0\n"
		    }
		    $_ = $indent . "Host";
		    if (@opts) {
			$_ .= ' ' . join(' ', @opts);
		    }
		    $_ .= " \"$hostline\"";
		}
	    }
	} elsif (/^\s*Socket/) {
	    $_ = "# Commented out: $_";
	}
	print $out "$_\n";
    }
    close $in;
    close $out;
}

## Start pound
## -----------
sub runner {
    $pound_pid = fork();
    die "fork: $!" unless defined $pound_pid;
    if ($pound_pid > 0) {
	# Send all listener descriptors to pound
	$listeners->send_fd;

	# Wait for the things to settle.
	if ($verbose) {
	    print "waiting for pound to start up\n";
	}

	$listeners->wait;
	if ($verbose) {
	    print "pound is ready\n";
	}
	return;
    }
    open(STDOUT, '>', $log_file);
    open(STDERR, ">&STDOUT");
    exec 'pound', '-p', $pid_file, '-f', $config, '-v', '-W', 'no-dns', '-W',
	  $include_dir ? "include-dir=$include_dir" : 'no-include-dir';
}

package PoundScript;
use strict;
use warnings;
use Carp;
use HTTP::Tiny;
use Data::Dumper;
use IO::Select;
use IPC::Open3;
use Symbol 'gensym';

sub new {
    my ($class, $file, $xscript) = @_;
    my $self = bless { tests => 0, failures => 0 }, $class;
    if ($file ne '-') {
	open(my $fh, '<', $file)
	    or croak "can't open script $file: $!";
	$self->{filename} = $file;
	$self->{fh} = $fh;
    } else {
	$self->{filename} = "<stdin>";
	$self->{fh} = \*STDIN;
    }
    if ($xscript) {
	open($self->{xscript}, '>', $xscript)
	    or croak "can't create transcript file $xscript: $!";
    }
    $self->{server} = 0;
    $self->{http} = HTTP::Tiny->new(max_redirect => 0);
    return $self;
}

sub tests { shift->{tests} }
sub failures { shift->{failures} }

sub http { shift->{http} }

sub send_and_expect {
    my ($self, $lst, $host, $stats) = @_;
    my $url = $lst->proto . '://' .
	      $host .
	      ':' .
	      $lst->port .
	      $self->{REQ}{URI};
    my %options;
    $options{peer} = $lst->host;
    if (exists($self->{REQ}{HEADERS})) {
	$options{headers} = $self->{REQ}{HEADERS};
    }
    if (exists($self->{REQ}{BODY})) {
	my $body = $self->{REQ}{BODY};
	if ($body =~ /@@/) {
	    my @chunks = split /@@/, $body;
	    $options{content} = sub {
		return shift(@chunks)
	    }
	} else {
	    $options{content} = $body;
	}
    }
    if ($verbose) {
	print "URL $self->{REQ}{METHOD} $url\n";
    }
    if (my $fh = $self->{xscript}) {
	print $fh "$self->{filename}:$self->{REQ}{BEG}-$self->{REQ}{END}: sending $self->{REQ}{METHOD} $url to server $self->{server}\n";
    }
    my $response = $self->http->request($self->{REQ}{METHOD}, $url, \%options);
    if (my $fh = $self->{xscript}) {
	print $fh "$self->{filename}:$self->{REQ}{BEG}-$self->{REQ}{END}: got:\n";
	print $fh Dumper([$response]);
    }

    $self->{tests}++;
    my $ok = $self->assert($response);
    if ($ok) {
	if ($stats) {
	    $stats->incr($response->{headers}{'x-backend-number'});
	}
    } else {
	$self->{failures}++;
    }
    if (my $fh = $self->{xscript}) {
	print $fh "$self->{filename}:$self->{EXP}{BEG}-$self->{EXP}{END}: " .
		   ($ok ? "OK" : "FAIL")."\n";
    }
    return $ok
}

sub check_expect {
    my ($self, $value, $exp) = @_;
    return $exp->{min} <= $value && $value <= $exp->{max};
}

sub send {
    my $self = shift;
    my $lst = $listeners->get($self->{server});
    my $host = delete $self->{REQ}{HEADERS}{host} || $lst->host;

    if ($self->{stats}) {
	my $n = $self->{stats}{samples}{value};
	my $s = Stats->new($backends->count);

	for (my $i = 0; $i < $n; $i++) {
	    $self->send_and_expect($lst, $host, $s) or goto end;
	}

	if (defined(my $i = $self->{stats}{index}{value})) {
	    $s = $s->elemstat($i);
	}

	# Assume success
	my $ok = 1;

	# Iterate over criteria applying them to the received statistics
	# and correcting $ok accordingly.
	foreach my $k (qw(sum min max avg stddev)) {
	    if (exists($self->{stats}{$k})) {
		unless ($self->check_expect($s->${ \$k }, $self->{stats}{$k})) {
		    $ok = 0;
		    if (my $fh = $self->{xscript}) {
			print $fh "$self->{filename}:$self->{EXP}{BEG}-$self->{EXP}{END}: $k: expected value mismatch\n";
		    }
		}
	    }
	}

	# Additionally, apply any individual element criteria.
	if (exists($self->{stats}{expect})) {
	    foreach my $i (sort keys %{$self->{stats}{expect}}) {
		my $val = $s->sample($i);
		unless ($self->check_expect($s->sample($i),
					    $self->{stats}{expect}{$i})) {
		    $ok = 0;
		    if (my $fh = $self->{xscript}) {
			print $fh "$self->{filename}:$self->{EXP}{BEG}-$self->{EXP}{END}: element $i: expected value mismatch\n";
		    }
		}
	    }
	}

	$self->{failures}++ unless $ok;

	if (my $fh = $self->{xscript}) {
	    printf $fh "min=%f, avg=%f, max=%f, stddev=%f\n", $s->min, $s->avg, $s->max, $s->stddev;
	    local $Data::Dumper::Sortkeys=1;
	    print $fh Dumper([$s->samples]);
	    print $fh "$self->{filename}:$self->{EXP}{BEG}-$self->{EXP}{END}: " .
		($ok ? "OK" : "FAIL")."\n";
	}
    } else {
	$self->send_and_expect($lst, $host);
    }
 end:
    delete $self->{REQ};
    delete $self->{EXP};
    delete $self->{stats};
}

sub assert {
    my ($self, $response) = @_;
    if ($self->{EXP}) {
	if ($self->{EXP}{STATUS} != $response->{status}) {
	    print STDERR "$self->{filename}:$self->{EXP}{BEG}: expected status $self->{EXP}{STATUS}, but got $response->{status}\n";
	    return 0;
	}
	if ($self->{EXP}{HEADERS}) {
	    foreach my $h (sort keys %{$self->{EXP}{HEADERS}}) {
		unless (exists($response->{headers}{$h})) {
		    print STDERR "$self->{filename}:$self->{EXP}{BEG}-$self->{EXP}{END}: expected header $h not present in the response\n";
		    return 0;
		}

		if ($self->{EXP}{HEADERS}{$h} =~ m{^/(.+)/$}) {
		    my $rx = qr($1);
		    if ($response->{headers}{$h} !~ $rx) {
			print STDERR "$self->{filename}:$self->{EXP}{BEG}-$self->{EXP}{END}: expected header $h value mismatch (regexp)\n";
			return 0;
		    }
		} else {
		    (my $v = $self->{EXP}{HEADERS}{$h}) =~ s{^\\}{};
		    if ($v ne $response->{headers}{$h}) {
			print STDERR "$self->{filename}:$self->{EXP}{BEG}-$self->{EXP}{END}: expected header $h value mismatch (expected \"$v\" got \"$response->{headers}{$h}\")\n";
			return 0;
		    }
		}
	    }
	}
	if ($self->{EXP}{NOTHEADERS}) {
	    foreach my $h (sort keys %{$self->{EXP}{NOTHEADERS}}) {
		if (exists($response->{headers}{$h}) &&
		    $response->{headers}{$h} eq $self->{EXP}{NOTHEADERS}{$h}) {
		    print STDERR "$self->{filename}:$self->{EXP}{BEG}-$self->{EXP}{END}: header \"$h: $self->{EXP}{NOTHEADERS}{$h}\" is present in the response\n";
		}
	    }
	}

	if (exists($self->{EXP}{BODY}) &&
	    $self->{EXP}{BODY} ne $response->{content}) {
	    print STDERR "$self->{filename}:$self->{EXP}{BEG}-$self->{EXP}{END}: response content mismatch\n";
	    print STDERR "EXP '".$self->{EXP}{BODY}."'\n";
	    print STDERR "GOT '".$response->{content}."'\n";
	}
    }
    return 1
}


sub parse {
    my $self = shift;
    while (!$self->{eof}) {
	$self->parse_req;
    }
}

sub syntax_error {
    my ($self, $message) = @_;
    $message //= "syntax error\n";
    my $locus = $self->{filename} . ':' . $self->{line};
    confess "$locus: $message";
}

sub parse_req {
    my $self = shift;
    my $fh = $self->{fh};

    while (<$fh>) {
	$self->{line}++;
	chomp;

	if (/^(?:#.*)?$/) {
	    next;
	}
	if (/^server\s+(\d+)/) {
	    $self->{server} = $1;
	    next;
	}
	if (s/^stats//) {
	    foreach my $k (qw(samples min max avg stddev index)) {
		if (s{\s+$k = (?:
				(?: (?<value>\d+(?:\.\d+)?)
				    (?: % (?: (?<dev>\d+(?:\.\d+)?) ) )? ) |
				(?: \[ \s* (?<min>\d+(?:\.\d+)?)
				      \s* , \s*
				      (?<max>\d+(?:\.\d+)?) \] )
			      )}{}x) {
		    if (defined($+{value})) {
			my $v = $+{value};
			$self->{stats}{$k}{value} = $v;
			if (defined($+{dev})) {
			    $self->{stats}{$k}{min} = $v - $+{dev} * $v / 100;
			    $self->{stats}{$k}{max} = $v + $+{dev} * $v / 100;
			} else {
			    my $d;
			    if ($v =~ /\.(\d+)/) {
				$d = (1/(10**length($1)))/2;
			    } else {
				$d = 0;
			    }
			    $self->{stats}{$k}{min} = $v - $d;
			    $self->{stats}{$k}{max} = $v + $d;
			}
		    } else {
			$self->{stats}{$k}{min} = $+{min};
			$self->{stats}{$k}{max} = $+{max};
		    }
		}
	    }

	    if (s/\s+expect\s+(?<i>\d+)\s+(?<value>\d+(?:\.\d+)?)(?:%(?<dev>\d+(?:\.\d+)?))?//) {
		$self->{stats}{expect}{$+{i}}{value} = $+{value};
		$self->{stats}{expect}{$+{i}}{dev} = $+{dev};
	    }
	    unless (/^\s*$/) {
		$self->syntax_error("unrecognized keywords in stats statement: $_");
	    }
	    unless (exists($self->{stats}{samples})) {
		$self->syntax_error("required keyword \"samples\" is missing");
	    }

	    next;
	}
	if (m/^run\s+(.+)$/) {
	    $self->parse_runcom($1);
	    next;
	}
	if (/^end$/) {
	    if ($self->{REQ}{BEG}) {
		$self->{REQ}{END} = $self->{line};
		$self->parse_expect;
	    } else {
		$self->syntax_error("unexpected end");
	    }
	    return;
	}
	if (/^(?<method>[A-Z_]+)\s+(?<uri>.+)$/) {
	    ($self->{REQ}{METHOD}, $self->{REQ}{URI}) = ($+{method}, $+{uri});
	    unless ($self->{REQ}{URI} =~ m{^/}) {
		$self->{REQ}{URI} = "/$self->{REQ}{URI}";
	    }
	    $self->{REQ}{PROTO} = 'http';
	    $self->{REQ}{BEG} = $self->{line};

	    $self->parse_headers;
	    return;
	}
	$self->syntax_error;
	return;
    }
    $self->{eof} = 1;
    $self->syntax_error("unexpected end of file") if $self->{REQ}{BEG};
}

sub parse_headers {
    my $self = shift;
    my $fh = $self->{fh};

    while (<$fh>) {
	$self->{line}++;
	chomp;

	if (/^$/) {
	    $self->parse_body;
	    return;
	}

	if (/^end$/) {
	    $self->{REQ}{END} = $self->{line};
	    $self->parse_expect;
	    return;
	}

	if (/^#.*$/) {
	    next;
	}

	if (/^(?<name>[A-Za-z][A-Za-z0-9_-]*):\s*(?<value>.*)$/) {
	    my $name = lc($+{name});
	    if (exists($self->{REQ}{HEADERS}{$name})) {
		my $val = $self->{REQ}{HEADERS}{$name};
		if (ref($val) ne 'ARRAY') {
		    $self->{REQ}{HEADERS}{$name} = [$val];
		}
		push @{$self->{REQ}{HEADERS}{$name}}, $self->expandvars($+{value})
	    } else {
		$self->{REQ}{HEADERS}{lc($+{name})} = $self->expandvars($+{value});
	    }
	} else {
	    $self->syntax_error;
	    return;
	}
    }
    $self->{eof} = 1;
    $self->syntax_error("unexpected end of file");
}

sub parse_body {
    my $self = shift;
    my $fh = $self->{fh};

    while (<$fh>) {
	$self->{line}++;
	chomp;

	if (/^end$/) {
	    $self->{REQ}{END} = $self->{line};
	    if ($self->{REQ}{BODY}) {
		$self->{REQ}{BODY} = join("\n", @{$self->{REQ}{BODY}})."\n";
	    }
	    $self->parse_expect;
	    return;
	}

	if (/^(?:#.*)?$/) {
	    next;
	}

	if (/\\(.*)/) {
	    push @{$self->{REQ}{BODY}}, $1;
	} else {
	    push @{$self->{REQ}{BODY}}, $_;
	}
    }
    $self->{eof} = 1;
    $self->syntax_error("unexpected end of file");
}

sub parse_runcom {
    my ($self, $command) = @_;
    my $fh = $self->{fh};

    my $collect;

    $self->{RUNCOM} = {
	command => $command,
	BEG => $self->{line}
    };
    while (<$fh>) {
	$self->{line}++;
	chomp;

	if ($collect && s{^\\}{}) {
	    push @{$self->{RUNCOM}{$collect}}, $_;
	    next;
	}

	if (/^end$/) {
	    if ($collect) {
		$collect = undef;
		next;
	    }
	    $self->{RUNCOM}{END} = $self->{line};
	    $self->runcom;
	    return;
	}

	if ($collect) {
	    push @{$self->{RUNCOM}{$collect}}, $_;
	    next;
	}

	if (/^status\s+(\d+)/) {
	    $self->{RUNCOM}{status} = $1;
	    next;
	}

	if (/^(stdout|stderr)\s*$/) {
	    $collect = $1;
	    next;
	}

	$self->syntax_error;
    }
    $self->{eof} = 1;
    $self->syntax_error("unexpected end of file");
}

sub runcom {
    my $self = shift;

    my ($child_stdin, $child_stdout, $child_stderr);
    $child_stderr = gensym();
    %status_codes = ();
    my $pid = open3($child_stdin, $child_stdout, $child_stderr,
		    $self->{RUNCOM}{command});
    close $child_stdin;

    my $sel = IO::Select->new();
    $sel->add($child_stdout, $child_stderr);
    my $CHUNK_SIZE = 1000;
    my @ready;
    my %data = ( $child_stdout => '', $child_stderr => '' );

    while (!defined($status_codes{$pid}) && (@ready = $sel->can_read)) {
	foreach my $fh (@ready) {
	    my $data;
	    while (1) {
		my $len = sysread($fh, $data{$fh}, $CHUNK_SIZE,
				  length($data{$fh}));
		die "sysread: $!" unless defined($len);
		if ($len == 0) {
		    $sel->remove($fh);
		    $fh->close;
		    last;
		}
	    }
	}
    }

    if (!defined($status_codes{$pid})) {
	sleep 10; # FIXME: hardcoded timeout
    }

    my $code;
    if (!defined($status_codes{$pid})) {
	die "failed to execute " . $self->{RUNCOM}{command} . ": $!";
    } elsif ($status_codes{$pid} & 127) {
	die "\"".$self->{RUNCOM}{command}."\" terminated on signal ".($status_codes{$pid} & 127);
    } else {
	$code = $status_codes{$pid} >> 8;
    }

    if (my $fh = $self->{xscript}) {
	print $fh "Command: " . $self->{RUNCOM}{command} . "\n";
	print $fh "Status code: $code\n";
	print $fh "Stdout:\n";
	print $fh $data{$child_stdout};
	print $fh "\nEnd\n";
	print $fh "Stderr:\n";
	print $fh $data{$child_stderr};
	print $fh "\nEnd\n";
    }

    $self->{tests}++;
    if (exists($self->{RUNCOM}{status}) && $code != $self->{RUNCOM}{status}) {
	$self->{failures}++;
	if (my $fh = $self->{xscript}) {
	    print $fh "$self->{filename}:$self->{RUNCOM}{BEG}-$self->{RUNCOM}{END}: exit code differs\n";
	    return 0;
	}
    }

    if (exists($self->{RUNCOM}{stdout})) {
	my $s = join("\n", @{$self->{RUNCOM}{stdout}});
	if ($data{$child_stdout} !~ m{$s}ms) {
	    $self->{failures}++;
	    if (my $fh = $self->{xscript}) {
		print $fh "$self->{filename}:$self->{RUNCOM}{BEG}-$self->{RUNCOM}{END}: stdout differs\n";
		print $fh "rx: $s\n";
		return 0;
	    }
	}
    }

    if (exists($self->{RUNCOM}{stderr})) {
	my $s = join("\n", @{$self->{RUNCOM}{stderr}});
	if ($data{$child_stderr} !~ m{$s}ms) {
	    $self->{failures}++;
	    if (my $fh = $self->{xscript}) {
		print $fh "$self->{filename}:$self->{RUNCOM}{BEG}-$self->{RUNCOM}{END}: stderr differs\n";
		print $fh "rx: $s\n";
		return 0;
	    }
	}
    }

    return 1
}

sub parse_expect {
    my $self = shift;
    my $fh = $self->{fh};

    while (<$fh>) {
	$self->{line}++;
	chomp;

	if (/^end$/) {
	    $self->send;
	    return;
	}

	if (/^(?:#.*)?$/) {
	    next;
	}

	if (/^\d{3}$/) {
	    $self->{EXP}{BEG} = $self->{line};
	    $self->{EXP}{STATUS} = $_;

	    $self->parse_expect_headers;
	    return;
	} else {
	    $self->syntax_error;
	}
    }
    $self->{eof} = 1;
    $self->syntax_error("unexpected end of file");
}

sub replvar {
    my ($self, $var) = @_;
    if ($var =~ m{(LISTENER|BACKEND)(\d+)?(?::(PORT|IP))?}) {
	my $var = ($1 eq 'LISTENER') ? $listeners : $backends;
	my $meth = 'address';
	if (defined($3)) {
	    $meth = ($3 eq 'PORT') ? 'port' : 'host';
	}
	return $var->get($2//0)->${ \$meth };
    }
    return '${' . $var . '}';
}

sub expandvars {
    my ($self, $v) = @_;
    $v =~ s{ \$ \{ ([A-Za-z][A-Za-z0-9:]*) \} }
	   { $self->replvar($1) }xeg;
    return $v;
}

sub parse_expect_headers {
    my $self = shift;
    my $fh = $self->{fh};

    while (<$fh>) {
	$self->{line}++;
	chomp;

	if (/^$/) {
	    $self->parse_expect_body;
	    return;
	}

	if (/^end$/) {
	    $self->{EXP}{END} = $self->{line};
	    $self->send;
	    return;
	}

	if (/^#.*$/) {
	    next;
	}

	if (/^(?<name>[A-Za-z][A-Za-z0-9_-]*):\s*(?<value>.*)$/) {
	    $self->{EXP}{HEADERS}{lc($+{name})} = $self->expandvars($+{value});
	} elsif (/^-(?<name>[A-Za-z][A-Za-z0-9_-]*):\s*(?<value>.*)$/) {
	    $self->{EXP}{NOTHEADERS}{lc($+{name})} = $self->expandvars($+{value});
	} else {
	    $self->syntax_error;
	    return;
	}
    }
    $self->{eof} = 1;
    $self->syntax_error("unexpected end of file");
}

sub parse_expect_body {
    my $self = shift;
    my $fh = $self->{fh};

    while (<$fh>) {
	$self->{line}++;
	chomp;

	if (/^end$/) {
	    $self->{EXP}{END} = $self->{line};
	    $self->{EXP}{BODY} = join("\n", @{$self->{EXP}{BODY}})."\n";
	    $self->send;
	    return;
	}

	# if (/^(?:#.*)?$/) {
	#     next;
	# }

	if (/\\(.*)/) {
	    push @{$self->{EXP}{BODY}}, $1;
	} else {
	    push @{$self->{EXP}{BODY}}, $_;
	}
    }
    $self->{eof} = 1;
    $self->syntax_error("unexpected end of file");
}

package Stats;
use strict;
use warnings;
use Carp;

sub new {
    my ($class, $n) = @_;
    bless { samples => [ (0) x ($n //= 0) ] }, $class;
}

sub incr {
    my ($self, $n, $v) = @_;
    $v //= 1;
    $self->{samples}[$n] += $v;
    $self->{total} += $v;
    $self->clear;
}

sub clear { shift->{dirty} = 1 }

sub total { shift->{total} }

sub numsamples {
    my $self = shift;
    return 0 + @{$self->{samples}}
}

sub sample {
    my ($self, $n) = @_;
    if ($n < 0 || $n >= $self->numsamples) {
	croak "n out of range";
    }
    return $self->{samples}[$n]
}

sub samples { @{shift->{samples}} }
sub ratio {
    my $self = shift;
    return map { $_ / $self->avg } $self->samples
}

sub elemstat {
    my ($self, $n) = @_;
    my $s = Stats->new();
    $s->incr(0, $self->sample($n) / $self->sum);
    return $s;
}

sub _compute {
    my ($self, $what) = @_;
    if ($self->{dirty}) {
	my $nsamples = $self->numsamples;
	my $sum = 0;
	my $sumq = 0;
	my $min = $self->{samples}[0] // 0;
	my $max = $self->{samples}[0] // 0;

	foreach my $v (@{$self->{samples}}) {
	    $v //= 0;
	    $sum += $v;
	    $sumq += $v * $v;
	    if ($v < $min) {
		$min = $v;
	    }
	    if ($v > $max) {
		$max = $v;
	    }
	}

	$self->{sum} = $sum;
	$self->{min} = $min;
	$self->{max} = $max;
	$self->{avg} = $sum / $nsamples;
	$self->{stddev} = sqrt($sumq / $nsamples - $self->{avg} * $self->{avg});

	$self->{dirty} = 0;
    }
    return $self->{$what}
}

sub sum { shift->_compute('sum') }
sub min { shift->_compute('min') }
sub max { shift->_compute('max') }
sub avg { shift->_compute('avg') }
sub stddev { shift->_compute('stddev') }

#
# Connection
#
package Listener;
use strict;
use warnings;
use Carp;
use Socket qw(:DEFAULT :crlf);;
use Fcntl;

eval "require IO::FDPass";
if ($@) {
    print STDERR "required module IO::FDPass not present\n";
    exit(::EX_SKIP);
}

sub new {
    my ($class, $number, $ident, $keepopen, $proto) = @_;
    my $socket;
    socket($socket, PF_INET, SOCK_STREAM, 0)
	or croak "socket: $!";
    setsockopt($socket, SOL_SOCKET, SO_REUSEADDR, 1);
    bind($socket, pack_sockaddr_in(0, inet_aton('127.0.0.1')))
	or die "bind: $!";
    if ($keepopen) {
	listen($socket, 128);
	my $flags = fcntl($socket, F_GETFD, 0)
	    or croak "fcntl F_GETFD: $!";
	fcntl($socket, F_SETFD, $flags & ~FD_CLOEXEC)
	    or croak "fcntl F_SETFD: $!";
    }
    my $sa = getsockname($socket);
    my ($port, $ip) = sockaddr_in($sa);
    bless {
	number => $number,
	ident => $ident,
	socket => $socket,
	host => inet_ntoa($ip),
	port => $port,
	proto => lc($proto // "http")
    }, $class;
}

sub proto { shift->{proto} }
sub number { shift->{number} }
sub ident { shift->{ident} }
sub socket_handle { shift->{socket} }
sub fd { fileno(shift->socket_handle) }
sub host { shift->{host} }
sub port { shift->{port} }
sub address {
    my $ls = shift;
    return $ls->host . ":" . $ls->port
}
sub sockname { shift->{sockname} }
sub pass_fd { shift->{pass_fd} }
sub set_pass_fd {
    my ($self, $sockname) = @_;
    unlink($sockname);
    socket(my $pfs, PF_UNIX, SOCK_STREAM, 0)
	or croak "socket: $!";
    bind($pfs, pack_sockaddr_un($sockname))
	or croak "bind($sockname): $!";
    listen($pfs, 8);
    $self->{sockname} = $sockname;
    $self->{pass_fd} = $pfs;
}

sub send_fd {
    my $self = shift;
    croak "not applicable" unless $self->pass_fd;
    accept(my $fh, $self->pass_fd);
    IO::FDPass::send(fileno($fh), $self->fd)
	or croak "failed to pass socket: $!";
    close $self->socket_handle;
    $self->{socket_handle} = undef;
    close $self->pass_fd;
    $self->{pass_fd} = undef;
}

sub listen {
    my ($self, $backlog) = @_;
    listen($self->socket_handle, $backlog // 8)
	or croak "listen: $!";
}

package ListenerList;
use strict;
use warnings;
use Carp;
use Socket;

sub new {
    my ($class, $keepopen) = @_;
    return bless { listeners => [], keepopen => $keepopen };
}
sub keepopen { shift->{keepopen} }
sub count { scalar @{shift->{listeners}} }
sub create {
    my ($self, $ident, $proto) = @_;
    if (defined($proto) && lc($proto) eq 'HTTPS') {
	my ($ok, $why) = HTTP::Tiny->can_ssl;
	unless ($ok) {
	    print STDERR "testing HTTPS is not supported: $why\n";
	    exit(main::EX_SKIP);
	}
    }
    my $lst = Listener->new($self->count(), $ident, $self->keepopen, $proto);
    if ($self->keepopen) {
	$lst->set_pass_fd("lst".$self->count().".sock");
    }
    push @{$self->{listeners}}, $lst;
    return $lst;
}
sub get {
    my ($self, $n) = @_;
    return ${$self->{listeners}}[$n];
}
sub last {
    my ($self) = @_;
    return $self->get($self->count - 1)
}
sub send_fd {
    my $self = shift;
    croak "not applicable" unless $self->keepopen;
    foreach my $lst (@{$self->{listeners}}) {
	$lst->send_fd()
    }
}

sub find_socket {
    my ($self, $sock) = @_;
    foreach my $lst (@{$self->{listeners}}) {
	return $lst if ($lst->socket_handle == $sock);
    }
    croak "Listener not found";
}

sub wait {
    my $self = shift;
    my @lst = @{$self->{listeners}};
    my $start = time();
    while (my $s = shift @lst) {
	if (time() - $start > $startup_timeout) {
	    print STDERR "pound didn't start up within $startup_timeout seconds\n";
	    print STDERR "examine $log_file\n";
	    exit(::EX_ERROR);
	}
	socket(my $sock, PF_INET, SOCK_STREAM, 0);
	my $res = connect($sock, pack_sockaddr_in($s->port, inet_aton($s->host)));
	unless ($res) {
	    push @lst, $s;
	}
	close($sock);
    }
}

sub read_and_process {
    my $self = shift;

    foreach my $lst (@{$self->{listeners}}) {
	$lst->listen;
    }

    foreach my $lst (@{$self->{listeners}}) {
	threads->create(sub {
	    my $lst = shift;
	    $lst->listen;
	    while (1) {
		my $fh;
		accept($fh, $lst->socket_handle);
		process_http_request($fh, $lst)
	    }
	}, $lst)->detach;
    }
}

sub http_echo {
    my $http = shift;
    my %headers = (
	'x-backend-ident' => $http->header('x-backend-ident'),
	'x-backend-number' => $http->backend->number,
	'x-orig-uri' => $http->uri,
	);
    while (my ($k, $v) = each %{$http->header}) {
	$headers{'x-orig-header-' . $k} = $v;
    }
    my @argv = (200, "OK", headers => \%headers);

    if (my $body = $http->body) {
	push @argv, body => $body
    }
    $http->reply(@argv);
}

sub http_redirect {
    my ($http, $rest) = @_;
    my $redir = $http->header('x-redirect')//'';
    $http->reply(301, "Moved Permanently", headers => {
			'location' => $redir . '/echo' . $rest
		 });
}

sub process_http_request {
    my ($sock, $backend) = @_;

    my %endpoints = (
	'echo' => \&http_echo,
	'redirect' => \&http_redirect,
    );

    local $| = 1;
    my $http = HTTPServ->new($sock, $backend);
    $http->parse();
    if ($http->uri =~ m{^/([^/]+)(/.*)?}) {
	my ($dir, $rest) = ($1, $2);
	if (my $ep = $endpoints{$dir}) {
	    &{$ep}($http, $2);
	} else {
	    $http->reply(404, "Not found",
			 headers => {
			     'x-orig-uri' => $http->uri,
			     map { ('x-orig-header-' . $_) => $http->header->{$_} } keys %{$http->header}
			 });
	}
    } else {
	$http->reply(500, "Malformed URI");
    }
    $http->close;
}

#
# HTTP server
#
package HTTPServ;
use strict;
use warnings;
use Socket qw(:crlf);
use Carp;

sub new {
    my ($class, $fh, $backend) = @_;
    bless { fh => $fh, backend => $backend }, $class;
}

sub backend { shift->{backend} }
sub ident { shift->backend->ident }
sub method { shift->{METHOD} }
sub version { shift->{VERSION} }
sub uri { shift->{URI} }
sub body { shift->{BODY} }
sub header {
    my ($http, $name) = @_;
    if (defined($name)) {
	return $http->{HEADERS}{lc($name)};
    }
    return $http->{HEADERS}
}

sub close {
    my $http = shift;
    close $http->{fh};
}

sub getline {
    my $http = shift;
    local $/ = $CRLF;
    my $fh = $http->{fh};
    my $ret = <$fh>;
    chomp($ret) if $ret;
    return $ret
}

sub ParseRequest {
    my $http = shift;

    my $input = $http->getline() or threads->exit();
    #    print "GOT $input\n";
    my @res = split " ", $input;
    if (@res != 3) {
	croak "Invalid input: $input";
    }

    ($http->{METHOD}, $http->{URI}, $http->{VERSION}) = @res;
}

sub ParseHeader {
    my $http = shift;

    while (my $input = $http->getline()) {
	last if $input eq '';
	#print "INPUT $input\n";
	my ($name, $value) = split ":", $input, 2;
	$value =~ s/^\s+//;
	$http->{HEADERS}{lc($name)} = $value;
    }
    $http->{HEADERS}{'x-backend-ident'} = $http->ident;
}

sub GetBody {
    my $http = shift;
    if ($http->header('Transfer-Encoding')//'' eq 'chunked') {
	my @chunks;
	my $chunk_ext;
	while (my $chunk_size_ext = $http->getline()) {
	    (my $chunk_size, $chunk_ext) = split /;/, $chunk_size_ext, 2;
	    my $len = hex($chunk_size);
	    last if $len == 0;
	    read($http->{fh}, my $chunk, $len);
	    push @chunks, $chunk_size_ext, $chunk;
	    read($http->{fh}, my $s, 2);
	    # FIXME: error checking
	}
	$http->{BODY} = join("\n", @chunks);
    } elsif (my $len = $http->header('Content-Length')) {
	read($http->{fh}, $http->{BODY}, $len);
    } # FIXME: else...
}

sub parse {
    my $http = shift;
    $http->ParseRequest;
    $http->ParseHeader;
    $http->GetBody;
}

sub reply {
    my ($http, $code, $descr, %opt) = @_;
    my $fh = $http->{fh};
    print $fh "$http->{VERSION} $code ${descr}$CRLF";
    if ($opt{headers}) {
	foreach my $h (keys %{$opt{headers}}) {
	    print $fh "$h: ".$opt{headers}{$h}.$CRLF;
	}
    }
    print $fh "connection: close$CRLF";
    print $fh "content-length: ". ($opt{body} ? length($opt{body}) : 0) . $CRLF;
    print $fh $CRLF;
    if ($opt{body}) {
	print $fh $opt{body};
    }
}
1;
__END__

=head1 NAME

poundharness - run pound tests

=head1 SYNOPSIS

B<poundharness>
[B<-sv>]
[B<-f I<FILE>>]
[B<-l I<N>>]
[B<-t I<N>>]
[B<-x I<FILE>>]
[B<--config=>I<SRC>[B<:>I<DST>]]
[B<--log-level=>I<N>]
[B<--startup-timeout=>I<N>]
[B<--statistics>]
[B<--transcript=>I<FILE>]
[B<--verbose>]
I<SCRIPT>

=head1 DESCRIPTION

Upon startup, B<poundharness> reads B<pound> configuration file F<pound.cfi>,
modifies the settings as described below and writes the resulting configuration
to B<pound.cfg>.  During modification, the following statements are removed:
B<Daemon>, B<LogFacility> and B<LogLevel>.  The settings suitable for running
B<pound> as a subordinate process are inserted istead.  In particular,
B<LogLevel> is set from the value passed with the B<--log-level> command
line option.  For each B<ListenHTTP> and B<ListenHTTPS> section, the actual
socket configuration (i.e. the B<Address>, B<Port>, or B<SocketFrom> statements)
is removed, an IPv4 socket is opened, bound to the arbitrary unused port at
127.0.0.1, and then passed to B<pound> using the B<SocketFrom> statement.
Similar operation is performed on each B<Backend>, except that the created
socket information is stored in the B<Address> and B<Port> statements that
replace the removed ones and a backend HTTP server is started listening
on that socket.

B<Include> statements are processed in the following manner: the file
supplied as the argument is preprocessed and the result is written to
a file with the name obtained by appending suffix C<.pha> to the original
name.  An B<Include> statement with this name is output.

When a B<Host> statement with exact comparison method is encountered,
a colon and actual port number of the enclosing listener is appended
to its argument.  If the argument is a file name, this operation is performed
on each line of the file and the result is written to the file F<I<file>.port>.
This file name is used in the output.  If B<Host> appears in a top-level
B<Service>, port number of the last declared listener is used instead.
If no listeners are defined (i.e. if the B<Service> statement appears
before the first listener), the statement is output unmodified.  Notice,
that this means that it cannot be matched.

When this configuration processing is finished, B<pound> is started in
foreground mode.  When it is up and ready to serve requests, the I<SCRIPT>
file is opened and processed.  It consists of a series of HTTP requests and
expected responses.  Each request is sent to the specified listener (the one
created in the preprocessing step described above), and the obtained response
is compared with the expectation.  If it matches, the test succeeds.  See
the section B<SCRIPT FILE>, for a detailed discussion of the script file
format.

When all the tests from I<SCRIPT> are run, the program terminates.  Its
status code reflects the results of the run: 0 if all tests succeeded, 1
if some of them failed, 2 if another error occurred and 77 if tests cannot
be run because of missing Perl module.

B<Poundharness> requires Perl 5.22.1 or later and B<IO::FDPass> module.
If these requirements are not met, it exits with code 77.

Status 3 is returned if B<poundharness> is used with wrong command line
switches or arguments,

=head1 OPTIONS

=over 4

=item B<-f>, B<--config=> I<SRC>[B<:>I<DST>]

Read source configuration file from I<SRC>, write processed configuration
to I<DST> and use it as configuration file when running B<pound>.  If
I<DST> is omitted, F<pound.cfg> is assumed.  If this option is not given,
I<SRC> defaults to F<pound.cfi>.

=item B<-l>, B<--log-level=> I<N>

Set B<pound> I<LogLevel> configuration parameter.  If I<N> is B<-1>,
I<LogLevel> set in the configuration file is used.

=item B<-s>, B<--statistics>

Print short statistics at the end of the run.

=item B<-t>, B<--startup-timeout=> I<N>

Timeout for B<pound> startup, in seconds.  Default is 2.

=item B<-v>, B<--verbose>

Increase output verbosity.

=item B<-x>, B<--transcript=> I<FILE>

Write test transcript to I<FILE>.

=back

=head1 SCRIPT FILE

B<Poundharness> script file consists of a sequence of send/expect
stanzas.  Empty lines end comments (lines starting with B<#>)
are allowed between them.

Two types of send/expect stanzas are available:

=head2 HTTP send/expect

An HTTP send/expect defined a HTTP requests and expected response.

A request starts with a line specifying the request method in uppercase
(e.g. B<GET>, B<POST>, etc.) followed by an URI.  This line may be followed
by arbitrary number of HTTP headers, newline and request body.  The request
is terminated with the word B<end> on a line alone.

The sequence B<@@> has a special meaning when used in the body.  These
characters instruct B<poundharness> to send the body using B<chunked> transfer
coding and indicate the beginning of each chunk.

An expected response must follow each request.  It begins with a
three digit response code on a line alone.  The code may be followed by
any number of response headers.  If present, the test will succeed only
if the actual response contains all expected headers and their corresponding
values coincide.  Header values are compared literally, except when the
expected header value begins and ends with a slash, in which case regular
expression matching is used.  To start header value with a literal slash,
precede it with a backslash.

Header lines starting with a minus sign denote headers, that must be absent
in the response. E.g. the header line

    -X-Forwarded-Proto: https

means that the response may not contain B<X-Forwarded-Proto: https> header.
The test will fail if such header is present.

Headers may be followed by a newline and response body (content).  If
present, it will be matched literally against the actual response.
The response is terminated with the word B<end> on a line alone.

Values of both request and expected headers may contain the following
I<variables>, which are expanded when reading the file:

=over 4

=item B<${LISTENERI<n>}>

Expands to the full address of the I<n>th listener (I<IP>:I<PORT>).

=item B<${LISTENERI<n>:IP}>

Expands to the IP address of the I<n>th listener.

=item B<${LISTENERI<n>:PORT}>

Expands to the port number of the I<n>th listener.

=item B<${LISTENER}>, B<${LISTENER:IP}>, B<${LISTENER:PORT}>

Same as B<${LISTENER0}>, B<${LISTENER0:IP}>, and B<${LISTENER0:PORT}>,
correspondingly.

=item B<${BACKENDI<n>}>

Expands to the address of the I<n>th backend (I<IP>:I<PORT>).

=item B<${BACKENDI<n>:IP}>

Expands to the IP address of the I<n>th backend.

=item B<${BACKENDI<n>:PORT}>

Expands to the port number of the I<n>th backend.

=item B<${BACKEND}>, B<${BACKEND:IP}>, B<${BACKEND:PORT}>

Same as B<${BACKEND0}>, B<${BACKEND0:IP}>, and B<${BACKEND0:PORT}>,
correspondingly.

=back

Numbering of listeners and backends starts from 0.

The B<server> statement can be used to specify the B<pound> listener to
send the requests to.  It has the form

    server N

where N is the ordinal 0-based number of the B<ListenHTTP> statement in
the configuration file.  The B<server> statement affects all requests that
follow it up to the next B<server> statement or end of file, whichever
occurs first.

=head2 External program send/expect

This type of stanza allow you to run an external program and examine its
exit code, standard output and error streams.  It is inlcuded mainly for
testing the B<poundctl> command.

The stanza begins with the keyword B<run> followed by the command
and its argument.  It can be followed by one or more of expect statements:

=over 4

=item B<status> I<N>

Expect program to return exit status I<N> (a decimal number).

=item B<stdout>

Expect text on stdout.  Everything below this keyword and up to the
B<end> keyword appearing on a line alone is taken to be the expected
text.  When matching actiual program output, this text is treated as
Perl multi-line regular expression (see the B<m> and B<s> flags in
B<perlre>).  To expect a line containing the word C<end> alone, prefix
it with a backslash.

=item B<stderr>

Expect text on stderr.  See the description of B<stdout> above for
its syntax.

=back

=head1 BACKENDS

Each B<Backend> statement in the configuration file causes creation of
a HTTP server behind it.  These built-in servers currently two endpoints:

=head2 /echo

Any requests with URLs under that endpoint are replied with exact copy of the
incoming requests, with the addition of the following headers:

=over 4

=item B<x-backend-number>

Ordinal number of the backend in configuration (0-based).

=item B<x-backend-ident>

Identifier of the backend, in form B<I<FILE>:I<LINE>>, where I<FILE> is the
source configuration file and I<LINE> is the number of line in it where the
corresponding B<Backend> keyword is located.

=item B<x-orig-uri>

Copy of the original URI.

=item B<x-orig-header->I<header>

The value of the header I<header> in the request.

=back

If the body was sent using B<chunked> encoding, it is reproduced
verbatim, instead of reconstructing it as per RFC 9112, 7.1.3.

=head2 /redirect

Redirects the request to the B</echo> endpoint.  The value of the
B<x-redirect> header, if any, is prepended to the value of the B<Location>
header.  E.g. the following request

    GET /redirect/foo?bar=0 HTTP/1.1
    X-Redirect: https://example.org

will get the following response:

    HTTP/1.1 301 Moved permanently
    Location: https://example.org/echo/foo?bar=0

This backend is used to test the B<RewriteLocation> functionality.

=head1 FILES

=over 4

=item pound.cfi

Source configuration file.  Can be changed using the B<--config> option.

=item pound.cfg

Destination configuration file.  The modified configuration is written to
it and it is then passed to B<pound> as its configuration file.

Can be changed using the B<--config> option.

=item pound.log

Log output from running instance of B<pound> is written to this file.

=item pound.pid

Keeps PID of the running B<pound> instance.

=item lstI<N>.sock

Temporary UNIX sockets for passing created socket descriptors to B<pound>.
These are removed on success.

=item I<FILE>.pha

Preprocessed output of the include file I<FILE>.

=item I<FILE>.port

Preprocessed output of the I<FILE> from B<Host -file "I<FILE>"> statement.

=back

=head1 SEE ALSO

B<pound> (8)

=head1 AUTHOR

Sergey Poznyakoff <gray@gnu.org>

=head1 LICENSE

GPLv3+: GNU GPL version 3 or later, see L<http://gnu.org/licenses/gpl.html>

This is free software: you are free to change and redistribute it.
There is NO WARRANTY, to the extent permitted by law.

=cut
