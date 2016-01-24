#! /usr/bin/perl -w

use strict;

my $conf__num_max = (256 * 256 * 256) - 2;

use Getopt::Long;
use Pod::Usage;

my $man = 0;
my $help = 0;

my $conf_num = 10_000;
my $conf_policy = 0;
my $conf_reverse = 0;

GetOptions ("number=i" => \$conf_num,
	    "policy!"  => \$conf_policy,
	    "reverse!" => \$conf_reverse,
            "help|?"   => \$help,
            "man"      => \$man);
pod2usage(-exitstatus => 0, -verbose => 1) if $help;
pod2usage(-exitstatus => 0, -verbose => 2) if $man;

if (($conf_num < 0) || ($conf_num > $conf__num_max))
  { die "Invalid number of statements."; }

print <<EOL;
(org.and.jhttpd-conf-main-1.0

  (org.and.daemon-conf-1.0
; Listen on test port/address
    (listen
      (port 8008))

    (cntl-file jhttpd_cntl) ; Allow simple shutdown

    (idle-timeout 32)
    (procs 4))

  (policy <default>
    (document-root html)
    (request-configuration-directory conf))
  (policy foo [(inherit <default>)]
    (server-name "Default/1 (jhttpd)"))

EOL

my @nums = (1..$conf_num);
if ($conf_reverse)
{ @nums = reverse @nums; }

for (@nums)
  {
    my $num = int($_);
    # convert num to 127.* ip
    my @ips = ();
    push @ips, int($num % 256); $num /= 256;
    push @ips, int($num % 256); $num /= 256;
    push @ips, int($num % 256); $num /= 256;

    if ($conf_policy)
      {
print <<EOL;
  (policy foo$_ [(inherit <default>)]
    (server-name "Huge/$_ (jhttpd)"))
  (match-connection [(client-ipv4-cidr-eq 127.$ips[2].$ips[1].$ips[0])]
    (policy foo$_))

EOL
     }
    else
      {
print <<EOL;
  (match-connection [(client-ipv4-cidr-eq 127.$ips[2].$ips[1].$ips[0])]
    (policy foo))
EOL
     }
}

print <<EOL;
 ; Close the rest...
  (match-connection [(policy-eq <default>)] <close>))
EOL

__END__

=head1 NAME

gen_huge_httpd_conf.pl - Generates huge config. files for jhttpd

=head1 SYNOPSIS

gen_huge_httpd_conf.pl [options]

 Options:
  --help -?         brief help message
  --man             full documentation
  --policy          Create a new policy for each match-request
  --number          Number of match-requests to make


=cut
