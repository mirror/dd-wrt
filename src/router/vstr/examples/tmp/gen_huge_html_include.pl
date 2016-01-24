#! /usr/bin/perl -w

my $conf_dir_fname = "index";
my $conf_iframe    = 0;
my $conf_num       = 200;
my $conf_force     = 0;

use Getopt::Long;
use Pod::Usage;

my $man = 0;
my $help = 0;

pod2usage(0) if !
GetOptions ("dir-fname|d=s"   => \$conf_dir_fname,
	    "iframe!"   => \$conf_iframe,
	    "num|n=i"    => \$conf_num,
	    "force!"    => \$conf_force,
	    "help|?"   => \$help,
	    "man"      => \$man);
pod2usage(-exitstatus => 0, -verbose => 1) if $help;
pod2usage(-exitstatus => 0, -verbose => 2) if $man;

if (! -d $ARGV[0])
{
	die " Not passed a directory as an argument.";
}

if (!$conf_force && -f $ARGV[0] . "/$conf_dir_fname")
{
	die " Passed a directory that already has an $conf_dir_fname, as an argument.";
}

open(M, "> $ARGV[0]/$conf_dir_fname") || die "open(): $!";

print M <<EOL;
<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.01 Transitional//EN">
<html>
  <head>
    <title>html iframe test</title>

<style>
  A:hover { color: #20bbaa; }
                                                                                
  P { text-indent: 0.4cm; }

  body { background: #FFF; color: #000; }
</style>

  </head>

  <body>
    <h1>html iframe test</h1>
EOL

for my $i (1..$conf_num)
{
	my $name = "itst$i.embed.html";
	if ($conf_iframe)
	{ print M "<iframe src=\"$name\"></iframe>\n"; }
	else
	{ print M "<object data=\"$name\"></object>\n"; }
	open(F, "> $ARGV[0]/$name") || die "open($name): $!";
	print F "test $i<br />";
	close(F);
}

print M <<'EOL';
    <hr>
    <address><a href="mailto:james-web@and.org">James Antill</a></address>
  </body>
</html>
EOL
