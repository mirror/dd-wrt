#!/usr/bin/perl
use warnings;
use strict;
use GD::Graph::Polar;
use Net::GPSD3 0.14;
use CGI;

=head1 NAME

skyview.cgi - Builds a Skyview PNG image from a POLL request to a GPSD server

=head1 COPYRIGHT

Copyright (c) 2011 - Michael R. Davis

This program is free software licensed under The BSD License.

=cut

my $cgi=CGI->new;

my $host=$cgi->param("host") || undef;
my $port=$cgi->param("port") || undef;
my $gpsd=Net::GPSD3->new(host=>$host, port=>$port);

my $size=$cgi->param("size") || 380;
my $gd=GD::Graph::Polar->new(size=>$size, radius=>90, border=>3, ticks=>9);

my $poll;
eval '$poll=$gpsd->poll';
my $error=$@;
if ($error) {
  chomp $error;
  $gd->color([248,8,8]);   #red
  $gd->addString(127=>134.5, $error);
} else {
  foreach my $sat ($poll->sky->Satellites) {
    if ($sat->ss > 34) {
      $gd->color([16,240,16]);   #green
    } elsif ($sat->ss >= 30) {
      $gd->color([240,240,16]);  #yellow
    } elsif ($sat->ss >= 10) {
      $gd->color([240,16,16]);   #red
    } else {
      $gd->color([128,128,128]); #gray
    }
    $gd->addGeoPoint(90-$sat->el => $sat->az);
    $gd->color([16,16,16]); 
    $gd->addGeoString(90-$sat->el => $sat->az, $sat->prn);
  }
}
$gd->color([210,210,210]); #gray
$gd->addGeoString(122 => 227.0, "Copyright (c) 2011                            Michael R. Davis");

print "Content-type: image/png\n\n", $gd->draw;
