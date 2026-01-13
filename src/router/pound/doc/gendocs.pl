# This file is part of GNU Pies.
# Copyright (C) 2020-2025 Sergey Poznyakoff
#
# GNU Pies is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 3, or (at your option)
# any later version.
#
# GNU Pies is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with GNU Pies.  If not, see <http://www.gnu.org/licenses/>.

use strict;
use warnings;
use Getopt::Long qw(:config gnu_getopt no_ignore_case);
use File::Basename;
use File::Spec;
use File::Path qw(make_path);
use Pod::Usage;

my $dirname = '.';
my $pkgname;
my $srcname;
my $output_name;
my @includes;

GetOptions("h" => sub {
                    pod2usage(-message => "$0: generate docs",
                              -exitstatus => 0);
           },
           "help" => sub {
                    pod2usage(-exitstatus => 0, -verbose => 2);
           },
           "usage" => sub {
                    pod2usage(-exitstatus => 0, -verbose => 0);
           },
	   'source|s=s' => \$srcname,
	   'directory|C=s' => \$dirname,
	   'output|o=s' => \$output_name,
	   'include|I=s@' => \@includes
    ) or exit(1);

$pkgname = shift @ARGV or pod2usage(-exitstatus => 1, -verbose => 0);
$srcname //= "${pkgname}.texi";
my $template_name = shift @ARGV or pod2usage(-exitstatus => 1, -verbose => 0);

unless (-d $dirname) {
    make_path($dirname);
}

unless ($output_name) {
    $output_name = File::Spec->catfile($dirname, (fileparse($template_name, qr/\.[^.]*/))[0]);
}

if (@includes) {
    @includes = map { '-I '.$_} @includes;
	# FIXME: Not used yet
}

sub template_scan {
    my $file = shift;
    open(FH, '<', $file) or die "can't open $file: $!\n";
    my $line = 0;
    while (<FH>) {
	chomp;
	++$line;
	s{ \$ ((?:BASE)?FILE|SIZE) \( ([a-z_]+) \) }{
	    eval { Gendocs->instance($2, $pkgname, $srcname) };
	    if ($@) {
	        if ($@ =~ m{Can't locate object method "new"}) {
	            die "$file:$line: unknown format: $2\n";
	        } else {
	            die $@;
	        }
	    }
	}gex;
    }
    close FH
}

sub template_expand {
    my ($infile, $outfile) = @_;
    open(IFH, '<', $infile) or die "can't open $infile: $!\n";
    open(OFH, '>', $outfile) or die "can't open $outfile: $!\n";
    while (<IFH>) {
	chomp;
	s{ \$ ((?:BASE)?FILE|SIZE) \( ([a-z_]+) \) }{
	   if ($1 eq 'FILE') {
	       Gendocs->instance($2)->output;
	   } elsif ($1 eq 'BASEFILE') {
	       basename(Gendocs->instance($2)->output);
	   } else {
	       Gendocs->instance($2)->size;
           }
	}gex;
	print OFH "$_\n";
    }
}

template_scan $template_name;
Gendocs->generate();
template_expand($template_name, $output_name);
Gendocs->sweep();

package Gendocs;
use strict;
use warnings;

my %registry;

sub generate {
    my ($class) = @_;
    my @keys = keys %registry;
    foreach my $k (@keys) {
	$registry{$k}->build();
	$registry{$k}->mark();
    }
}

sub sweep {
    my ($class) = @_;
    my @keys = keys %registry;
    foreach my $k (@keys) {
	unless ($registry{$k}->has_mark) {
	    $registry{$k}->remove;
	    delete $registry{$k};
	}
    }
}

sub new {
    my ($class, $pkgname, $name) = @_;
    unless (exists($registry{$class})){
	$registry{$class} = bless { pkgname => $pkgname, input => $name }, $class;
    }
    return $registry{$class} 
}
sub instance {
    my ($class, $fmt, @args) = @_;
    my $subclass = "Gendocs::".ucfirst($fmt);
    unless (exists($registry{$subclass})) {
	$registry{$subclass} = $subclass->new(@args);
    }
    return $registry{$subclass};
}

sub runcom {
    my $self = shift;
    system @_;
    if ($? == -1) {
	die "failed to execute $_[0]: $!";
    } elsif ($? & 127) {
	die sprintf("$_[0] died with signal %d\n", $? & 127);
    } elsif ($? >> 8) {
	warn sprintf("$_[0] exited with value %d\n", $? >> 8);
    }
}

sub mark { shift->{mark} = 1 }    
sub has_mark { shift->{mark} }    
sub remove {
    my ($self) = @_;
    if ($self->{output}) {
	unlink $self->{output};
	delete $self->{output};
    }
}
sub size {
    my ($self) = @_;
    my $s = (stat($self->output))[7];
    if ($s > 1048576) {
	$s = int($s / 1048576) . 'M';
    } elsif ($s > 1024) {
	$s = int($s / 1024) . 'K';
    }
    return $s;
}

sub pkgname { shift->{pkgname} }
sub input { shift->{input} }
sub output { shift->{output} }

package Gendocs::Makeinfo;
use strict;
use warnings;
use base 'Gendocs';

sub new {
    my $class = shift;
    my $self = $class->SUPER::new(@_);
    $self->{makeinfo} = $ENV{'MAKEINFO'} || 'makeinfo';
    return $self;
}

package Gendocs::Info;
use strict;
use warnings;
use base 'Gendocs::Makeinfo';

sub build {
    my ($self) = @_;
    unless ($self->{output}) {
	my $output = File::Spec->catfile($dirname, $self->pkgname . '.info');
	print "Generating info file: " . $self->input . " -> $output\n";
	$self->runcom("$self->{makeinfo} -o $output " . $self->input);
        $self->{output} = $output;
    }
    return $self->{output};
}

package Gendocs::Info_gz;
use strict;
use warnings;
use base 'Gendocs';

sub build {
    my ($self) = @_;
    unless ($self->{output}) {
	my $input = Gendocs->instance('info', $self->pkgname, $self->input)->build();
	my $output = "$input.gz";
	print "Compressing info file: $input -> $output\n";
	$self->runcom("gzip -f -9 -c $input > $output");
        $self->{output} = $output;
    }
    return $self->{output};
}

package Gendocs::Ascii;
use strict;
use warnings;
use base 'Gendocs::Makeinfo';

sub build {
    my ($self) = @_;
    unless ($self->{output}) {
	my $output = File::Spec->catfile($dirname, $self->pkgname . '.txt');
	print "Generating ascii file: " . $self->input . " -> $output\n";
        $self->runcom("$self->{makeinfo} -o $output --no-split --no-headers " . $self->input);
        $self->{output} = $output;
    }
    return $self->{output};
}

package Gendocs::Ascii_gz;
use strict;
use warnings;
use base 'Gendocs';

sub build {
    my ($self) = @_;
    unless ($self->{output}) {
	my $input = Gendocs->instance('ascii', $self->pkgname, $self->input)->build();
	my $output = "$input.gz";
	print "Compressing ascii file: $input -> $output\n";
	$self->runcom("gzip -f -9 -c $input > $output");
        $self->{output} = $output;
    }
    return $self->{output};
}

package Gendocs::Texinfo_gz;
use strict;
use warnings;
use base 'Gendocs';

sub build {
    my ($self) = @_;
    unless ($self->{output}) {
	my $output = File::Spec->catfile($dirname, $self->pkgname . '.tar.gz');
	print "Creating compressed sources: $output\n";
	$self->runcom("tar czfh $output *.texinfo *.texi *.txi *.eps 2>/dev/null || /bin/true");
        $self->{output} = $output;
    }
    return $self->{output};
}

package Gendocs::Dvi;
use strict;
use warnings;
use base 'Gendocs';
use File::Temp qw(tempdir);

sub new {
    my $class = shift;
    my $self = $class->SUPER::new(@_);
    $ENV{TEXI2DVI_BUILD_DIRECTORY} = tempdir(CLEANUP => 1);
    $self->{texi2dvi} = $ENV{'TEXI2DVI'} || 'texi2dvi --build=tidy -t @finalout';
    return $self;
}

sub build {
    my ($self) = @_;
    unless ($self->{output}) {
	my $output = File::Spec->catfile($dirname, $self->pkgname . '.dvi');
	my $cmd = "$self->{texi2dvi} -o $output $self->{input}";
	print "Creating dvi: $cmd\n";
	$self->runcom($cmd);
	$self->{output} = $output;
    }
    return $self->{output};
}

package Gendocs::Dvi_gz;
use strict;
use warnings;
use base 'Gendocs';

sub build {
    my ($self) = @_;
    unless ($self->{output}) {
	my $input = Gendocs->instance('dvi', $self->pkgname, $self->input)->build();
	my $output = "$input.gz";
	print "Compressing dvi file: $input -> $output\n";
	$self->runcom("gzip -f -9 -c $input > $output");
        $self->{output} = $output;
    }
    return $self->{output};
}

package Gendocs::Pdf;
use strict;
use warnings;
use base 'Gendocs::Dvi';

sub build {
    my ($self) = @_;
    unless ($self->{output}) {
	my $output = File::Spec->catfile($dirname, $self->pkgname . '.pdf');
	my $cmd = "$self->{texi2dvi} -o $output --pdf $self->{input}";
	print "Creating pdf: $cmd\n";
	$self->runcom($cmd);
        $self->{output} = $output;	
    }
    return $self->{output};
}
__END__
=head1 NAME

gendocs.pl - generate documentation in various formats

=head1 SYNOPSIS

B<gendocs.pl>
[B<-C> I<DIR>]    
[B<-s> I<SOURCE>]
[B<-o> I<OUTPUT-FILE>]    
[B<-I> I<INCLUDE-DIR>]
[B<--directory=>I<DIR>]
[B<--include=>I<INCLUDE-DIR>]    
[B<--output=>I<OUTPUT-FILE>]    
[B<--source=>I<SOURCE>]    
I<PACKAGE> I<TEMPLATE>

B<gendocs.pl> B<-h> | B<--help> | B<--usage>

=head1 DESCRIPTION

Generates documentation for the I<PACKAGE> in various formats.  I<TEMPLATE>
is a template file for the index page.  When processing I<TEMPLATE> the
following I<macros> inform B<gendocs.pl> about the desired documentation
formats and are expanded on output:    

=over 4

=item B<$FILE(I<FORMAT>)>

Full pathname of the documentation file in format I<FORMAT>.

=item B<$BASEFILE(I<FORMAT>)>

Base name of the documentation file for format I<FORMAT>.

=item B<$SIZE(I<FORMAT>)>

Size of the documentation file in format I<FORMAT>.  Proper size suffix
(B<K> or B<M>) is appended, as needed.

=back

The file is processed twice.  On the first pass, the program collects the
mentioned I<FORMAT>s.  Then the requested files are generated.  On the
second pass, the macros are replaced with the actual values and the output
index file is generated.  The name of the index file can be supplied using
the B<-o> (B<--output>) option.  If it is not given, the name is obtained
by removing last suffix (a substring beginning with a dot and containing
one or more characters, excepting dots) from the I<TEMPLATE> argument.

Unless the B<-C> (B<--directory>) option is given, the output will be
generated in the current working directory.  If the B<-C> option is
given, all output files, including index file, will be generated in
the supplied directory.

The following output formats are supported:

=over 4

=item B<info>

Monolithic info file.
    
=item B<info_gz>

Monolithic info file, compressed with B<gzip>
    
=item B<ascii>

Monolithic ASCII file.
    
=item B<ascii_gz>

Monolithic ASCII file, compressed with B<gzip>

=item B<texinfo_gz>

A tar archive with the Texinfo documentation sources, compressed with B<gzip>.
    
=item B<dvi>

TeX B<dvi> file.    

=item B<dvi_gz>

TeX B<dvi> file, compressed with B<gzip>
    
=item B<pdf>

A B<PDF> file.    

=back

=head1 OPTIONS

=over 4

=item B<-C>, B<--directory=>I<DIR>

Create all output files in the directory I<DIR>.
    
=item B<-s>, B<--source=>I<SOURCE>

Name of the main Texinfo source file.  By default F<I<PACKAGE>.texi> is
used.    
    
=item B<-o>, B<--output=>I<OUTPUT-FILE>

Name of the output index file.  By default it is constructed by removing
the last filename suffix from I<TEMPLATE>.  E.g. F<index.texi.in> produces
F<index.texi>.    
    
=item B<-I>, B<--include=>I<INCLUDE-DIR>

Name of the directory with Texinfo include files.  This option is not
actually used.  It is reserved for future use.

=back

=head1 ENVIRONMENT

The following environment variables affect the behavior of B<gendocs.pl>:

=over 4

=item B<MAKEINFO>

Name and initial options of the B<makeinfo> program.

=item B<TEXI2DVI>

Name and initial options of the B<texi2dvi> program.  The default is
F<texi2dvi --build=tidy -t @finalout>.    

=item B<TEXINPUTS>

Used by F<texi2dvi>.  Colon-separated list of Texinfo input directories.

=back    

=cut
