#! /usr/bin/perl -w

use strict;

use File::Find;
use File::Temp qw/tempfile/;
use File::Basename;

use FileHandle;

# It's there on FC1, but not on RHEL3
my $have_perl_cmpstat = 0;


#  Might want to add .iso or some .mov type exts ... however non-trivial savings
# are often on those files.
my $filter_re = qr/(?:
		   ^[.]nfs. |
		   [.]gz$  |
		   [.]bz2$ |
		   [.]rpm$ |
		   [.]zip$ |
		   [.]tmp$ |
		   ~$      |
		   \#$
		  )/x;

use Getopt::Long;
use Pod::Usage;

my $man = 0;
my $help = 0;

my $tidy_compress = 0;
my $force_compress = 0;
my $re_compress = 0;
my $chown_compress = 0;
my $once_compress = undef;
my $verbose_compress = 0;
my $zero_compress = 0;
my $type_compress = "gzip";

pod2usage(0) if !
GetOptions ("force!"   => \$force_compress,
	    "all!"     => \$re_compress,
	    "chown!"   => \$chown_compress,
	    "output|o=s" => \$once_compress,
	    "tidy!"    => \$tidy_compress,
	    "zero!"    => \$zero_compress,
	    "type|t=s" => \$type_compress,
	    "verbose+" => \$verbose_compress,
	    "help|?"   => \$help,
	    "man"      => \$man);
pod2usage(-exitstatus => 0, -verbose => 1) if $help;
pod2usage(-exitstatus => 0, -verbose => 2) if $man;

if (($type_compress ne "gzip") && ($type_compress ne "bzip2") &&
    ($type_compress ne "all"))
  { pod2usage(-exitstatus => 1); }

sub grep_files
  { # Don't compress compressed files, or nfs...
    grep(!/$filter_re/,  @_)
  }

our $out;
our $fname;


use Math::BigInt;

sub p95
  { # 95% of value
    my $val = Math::BigInt->new(shift);

    $val->bmul(95);
    $val->bdiv(100);

    return $val->bfloor();
  }


sub cleanup
  {
    my $in = shift;

    close($in);
    if (defined ($out))   { close($out);    $out   = undef; }
    if (defined ($fname)) { unlink($fname); $fname = undef; }

    return shift;
  }

sub zip__file
  {
    my $name              = shift;
    my $type_compress     = shift;
    my $ext_compress      = shift;
    my $cmd_compress_args = shift;
    my $other_sz          = shift;

    my $namegz = shift || ($name . $ext_compress);

    if (-l $name && -f $name)
      { # deal with symlinks...
	my $dst = readlink $name;

	defined($dst) || die "Can't readlink $name: $!";

	my $dst_gz = $dst . $ext_compress;
	if (($dst !~ /$filter_re/) && -f $dst_gz)
	  {
	    unlink($namegz);
	    print STDOUT "Symlink: $name => $dst\n" if ($verbose_compress > 1);
	    symlink($dst_gz, $namegz) || die "Can't symlink($namegz): $!";
	  }
	return 0;
      }

    if (! -f _ || ! -r _)
      {
	return 0;
      }

    my @st_name   = stat _;
    if (!$other_sz)
      { $other_sz = $st_name[7]; }
    if (!$re_compress)
      {
	if (-f $namegz)
	  { # If .gz file is already newer, skip it...
	    my @st_namegz = stat _;

	    if ($st_name[9] < $st_namegz[9])
	      {
		if ($tidy_compress && # remove old
		    (($st_namegz[7] >= p95($other_sz))))
		  {
		    unlink($namegz);
		    return $other_sz;
		  }
		return $st_namegz[7];
	      }
	  }
      }

    if (!$force_compress)
      { # This will error out...
	($out, $fname) = tempfile("gzip-r.XXXXXXXX", SUFFIX => ".tmp",
				  DIR => File::Basename::dirname($namegz));
      }
    else
      {
	eval {
	  ($out, $fname) = tempfile("gzip-r.XXXXXXXX", SUFFIX => ".tmp",
				    DIR => File::Basename::dirname($namegz));
	};
	return $other_sz if ($@);
      }
    binmode $out;

    print STDOUT "Compress: $name\n" if ($verbose_compress > 0);

    my $in = undef;
    if (!$force_compress)
      {
	open($in, "-|", @$cmd_compress_args, "--", $name) ||
	  die("Can't $$cmd_compress_args[0]: $!");
      }
    else
      {
	open($in, "-|", @$cmd_compress_args, "--", $name) ||
	  return cleanup(undef, $other_sz);
      }
    binmode $in;

    my $bs = 1024 * 8; # Do IO in 8k blocks
    $/ = \$bs;

    while (<$in>) { $out->print($_); }

    # If the the gzip file is 95% of the original, delete it
    # Or we are doing bzip2 and we already have a gzip file that is smaller
    $out->autoflush(1);
    my @st_namegz = stat $out;
    if ($st_namegz[7] >= p95($other_sz))
      {
	if ($zero_compress)
	  {
	    $st_namegz[7] = p95($other_sz);
	    truncate($out, 0);
	  }
	else
	  {
	    return cleanup($in, $other_sz);
	  }
      }
    close($in) || die "Failed closing input: $!";

    rename($fname, $namegz)                 || die "Can't rename($namegz): $!";
    if ($have_perl_cmpstat)
      {
	File::Temp::cmpstat($out, $namegz)  || die "File moved $namegz: $!";
      }
    close($out)                             || die "Failed closing output: $!";
    $out = undef;

    # No stupid fchmod/fchown in perl, Grr....
    chmod($st_name[2] & 0777, $namegz);
    if ($chown_compress)
      { chown($st_name[4], $st_name[5], $namegz); }
    return $st_namegz[7];
  }

sub zip_file
  {
    my $name = $_;
    my $other_sz = 0;

    my ($dev,$ino,$mode,$nlink,$uid,$gid);

    if ((($dev,$ino,$mode,$nlink,$uid,$gid) = lstat($name)) &&
	($dev != $File::Find::topdev))
      {
	$File::Find::prune = 1;
	return;
      }

    $other_sz = zip__file($name, "gzip", ".gz",
			  ["gzip", "--to-stdout", "--no-name", "--best"], 0)
      if (($type_compress eq "gzip")  || ($type_compress eq "all"));

    $other_sz = zip__file($name, "bzip2", ".bz2",
			  ["bzip2", "--stdout", "--best"], $other_sz)
      if (($type_compress eq "bzip2") || ($type_compress eq "all"));
  }

if (defined ($once_compress))
  {
    die " Can't use type=all with --output" if ($type_compress eq "all");

    my $name = shift;

    die " Too many arguments for --output"  if (@ARGV);

    zip__file($name, "gzip",  ".gz",
	      ["gzip", "--to-stdout", "--no-name", "--best"], 0, $once_compress)
      if ($type_compress eq "gzip");

    zip__file($name, "bzip2", ".bz2",
	      ["bzip2", "--stdout", "--best"], 0, $once_compress)
      if ($type_compress eq "bzip2");

    exit;
  }

find({ preprocess => \&grep_files, wanted => \&zip_file }, @ARGV);

END {
  if (defined($out) && defined($fname))
    {
      File::Temp::unlink0($out, $fname) || die "Can't unlink($fname): $!"; $?;
    }
}

__END__

=head1 NAME

gzip-r - Recursive "intelligent" gzip/bzip2

=head1 SYNOPSIS

gzip-r [options] [dirs|files ...]

 Options:
  --help -?         brief help message
  --man             full documentation
  --force           force mode
  --all             compress files that already have a compressed version
  --tidy            tidy unused files
  --verbose         print filenames
  --chown           chown compressed files
  --output -o       compress a single file, passing the compressed filename
  --type -t         type of compression files

=head1 OPTIONS

=over 8

=item B<--help>

Print a brief help message and exits.

=item B<--man>

Prints the manual page and exits.

=item B<--force>

Carry on compressing even if errors are encountered during tempfile creation.

=item B<--all>

Recompresses files even when the compressed versions are newer than their
source.

=item B<--chown>

Make the compressed output files have the same owner as the input files.

=item B<--output>

Only compress a single file, the argument for the option is the compressed
output filename.
 Note: You can't use this option when using --type=all.
 Note: You can't specify multiple sources.

=item B<--tidy>

Cleanup any old compressed files that wouldn't be created (due to not being
significantly smaller).

=item B<--type>

Make the compression type either gzip, bzip2 or all.

=item B<--verbose>

Prints the name of each file being compressed followed by a newline, if
specified once. If specified more than once also prints the name of each symlink
created.

=back


=head1 DESCRIPTION

B<gzip-r> will take all files from the directories and filenames passed
as fname. If the extensions of the files are not likely to be compressible
(Ie. .gz, .bz2, .rpm, .zip) or are tmp files (Ie. .tmp, ~, #) then they are
skipped.
 If the fname is a regular file a fname.gz output file will be generated
(without removing the input fname file).
 If the fname is a symlink a fname.gz symlink pointing to the target of fname
with a .gz extension added will be created.

 gzip is called with the options: --no-name --best

=cut
