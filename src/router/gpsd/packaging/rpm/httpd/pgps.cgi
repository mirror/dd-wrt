#!/usr/bin/perl
use strict;
use warnings;
use CGI qw{};
use CGI::Carp qw(fatalsToBrowser);
use Net::GPSD3 0.14;

=head1 NAME

pgps.cgi - Builds a Satellite List and displays GPS data from a POLL request to a GPSD server

=head1 COPYRIGHT

Copyright (c) 2011 - Michael R. Davis

This program is free software licensed under The BSD License.

=cut

my $cgi=CGI->new;
my $host=$cgi->param("host") || undef;
my $port=$cgi->param("port") || undef;
my $gpsd=Net::GPSD3->new(host=>$host, port=>$port);
my $poll=$gpsd->poll;

my @param=();
push @param, "host=".$host if defined $host;
push @param, "port=".$port if defined $port;
my $img=join("?", "skyview.cgi", scalar(@param) ? join(";", @param) : ());

my $content=join "",
            $cgi->start_html(-title=>"Perl GPSD CGI"),
            $cgi->table({border=>0, width=>"100%", style=>"BORDER-COLLAPSE: collapse"},
              $cgi->Tr(
                $cgi->td({width=>"30%", valign=>"top"},"Satellite List",
                  $cgi->table({border=>2, width=>"100%", style=>"BORDER-COLLAPSE: collapse"},
                    $cgi->Tr(
                      $cgi->td([qw{PRN Elev Azim SNR Used}]),
                    ),
                    map {
                      $cgi->Tr([
                        $cgi->td([$_->prn, $_->el, $_->az, $_->ss, $_->used ? "Y" : "N"])])
                    } sort {$b->used <=> $a->used or $b->ss <=> $a->ss} $poll->sky->Satellites
                  ),
                ),
                $cgi->td({valign=>"top"}, "Skyview",
                  $cgi->table({border=>2, style=>"BORDER-COLLAPSE: collapse"},
                    $cgi->Tr(
                      $cgi->td(
                        $cgi->img({-src=>$img}),
                      ),
                    ),
                  ),
                ),
              ),
              $cgi->Tr(
                $cgi->td({-colspan=>2, valign=>"top"}, "GPS data",
                  $cgi->table({border=>2, width=>"100%", style=>"BORDER-COLLAPSE: collapse"},
                    $cgi->Tr(
                      $cgi->td({align=>"right", width=>"25%"}, "Time:"),
                      $cgi->td({width=>"25%"}, $poll->tpv->timestamp),
                      $cgi->td({align=>"right", width=>"25%"}, "Status:"),
                      $cgi->td({width=>"25%"}, $poll->tpv->mode),
                    ),
                    $cgi->Tr(
                      $cgi->td({align=>"right"}, "Latitude:"),
                      $cgi->td($poll->tpv->lat),
                      $cgi->td({align=>"right"}, "EPX:"),
                      $cgi->td($poll->tpv->epx),
                    ),
                    $cgi->Tr(
                      $cgi->td({align=>"right"}, "Longitude:"),
                      $cgi->td($poll->tpv->lon),
                      $cgi->td({align=>"right"}, "EPY:"),
                      $cgi->td($poll->tpv->epy),
                    ),
                    $cgi->Tr(
                      $cgi->td({align=>"right"}, "Altitude:"),
                      $cgi->td($poll->tpv->alt),
                      $cgi->td({align=>"right"}, "EPV:"),
                      $cgi->td($poll->tpv->epv),
                    ),
                    $cgi->Tr(
                      $cgi->td({align=>"right"}, "Speed:"),
                      $cgi->td($poll->tpv->speed),
                      $cgi->td({align=>"right"}, "EPS:"),
                      $cgi->td($poll->tpv->eps),
                    ),
                    $cgi->Tr(
                      $cgi->td({align=>"right"}, "Climb:"),
                      $cgi->td($poll->tpv->climb),
                      $cgi->td({align=>"right"}, "EPC:"),
                      $cgi->td($poll->tpv->epc),
                    ),
                    $cgi->Tr(
                      $cgi->td({align=>"right"}, "Track:"),
                      $cgi->td($poll->tpv->track),
                      $cgi->td({align=>"right"}, "EPD:"),
                      $cgi->td($poll->tpv->epd),
                    ),
                  ),
                ),
              ),
            ),
            $cgi->p({align=>"center"}, "Copyright &copy; 2011 - Michael R. Davis, License: BSD"),
            $cgi->end_html, "\n";

print join "",
        $cgi->header(
                     -content_length => length($content),
                     -expires        => '+5m',
                     -Refresh        => '30',   #seconds
                    ),
        $content;
