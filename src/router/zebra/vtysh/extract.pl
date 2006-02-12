#! /usr/bin/perl
##
## Virtual terminal interface shell command extractor.
## Copyright (C) 2000 Kunihiro Ishiguro
## 
## This file is part of GNU Zebra.
## 
## GNU Zebra is free software; you can redistribute it and/or modify it
## under the terms of the GNU General Public License as published by the
## Free Software Foundation; either version 2, or (at your option) any
## later version.
## 
## GNU Zebra is distributed in the hope that it will be useful, but
## WITHOUT ANY WARRANTY; without even the implied warranty of
## MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
## General Public License for more details.
## 
## You should have received a copy of the GNU General Public License
## along with GNU Zebra; see the file COPYING.  If not, write to the Free
## Software Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
## 02111-1307, USA.  
##

print <<EOF;
#include <zebra.h>
#include "command.h"
#include "vtysh.h"

EOF

$hash{'router_rip_cmd'} = "ignore";
$hash{'router_ripng_cmd'} = "ignore";
$hash{'router_ospf_cmd'} = "ignore";
$hash{'router_ospf6_cmd'} = "ignore";
$hash{'router_bgp_cmd'} = "ignore";
$hash{'address_family_vpnv4_cmd'} = "ignore";
$hash{'address_family_vpnv4_unicast_cmd'} = "ignore";
$hash{'address_family_ipv4_multicast_cmd'} = "ignore";
$hash{'address_family_ipv6_unicast_cmd'} = "ignore";
$hash{'address_family_ipv6_cmd'} = "ignore";
$hash{'exit_address_family_cmd'} = "ignore";
$hash{'key_chain_cmd'} = "ignore";
$hash{'key_cmd'} = "ignore";
$hash{'route_map_cmd'} = "ignore";
$hash{'set_metric_cmd'} = "ignore";
$hash{'set_ip_nexthop_cmd'} = "ignore";

foreach (@ARGV) {
    $file = $_;

    open (FH, "cpp -DHAVE_CONFIG_H -DVTYSH_EXTRACT_PL -I. -I.. -I../lib $file |");
    local $/; undef $/;
    $line = <FH>;
    close (FH);

    @defun = ($line =~ /(?:DEFUN|ALIAS)\s*\((.+?)\)\n/sg);
    @install = ($line =~ /install_element \([A-Z_46]+, &[^;]*;\n/sg);

    if ($file =~ /lib/) {
	if ($file =~ /keychain.c/) {
	    $protocol = "VTYSH_RIPD";
	}
	if ($file =~ /routemap.c/) {
	    $protocol = "VTYSH_RIPD|VTYSH_OSPFD|VTYSH_BGPD";
	}
	if ($file =~ /filter.c/) {
	    $protocol = "VTYSH_RIPD|VTYSH_OSPFD|VTYSH_BGPD";
	}
	if ($file =~ /plist.c/) {
	    $protocol = "VTYSH_RIPD|VTYSH_BGPD";
	}
    } else {
	($protocol) = ($file =~ /\/([a-z0-9]+)\//);
	$protocol = "VTYSH_" . uc $protocol;
    }

    foreach (@defun) {
	my (@arg);
	@arg = split (/,/);
	$arg[0] = '';

	# Actual input command string.
	$cmd = "$arg[2]";
	$cmd =~ s/^\s+//g;
	$cmd =~ s/\s+$//g;

	# Get VTY command structure.  This is needed for searching
	# install_element() command.
	$struct = "$arg[1]";
	$struct =~ s/^\s+//g;
	$struct =~ s/\s+$//g;
	$arg[1] = $struct . "_vtysh";

	$arg_str = join (", ", @arg);

	if (! grep (/$protocol/, @{$hashp{$struct}}) 
	    && $hash{$struct} ne "ignore") {
	    $hash{$struct} = "$arg_str";
	    push (@{$hashp{$struct}}, $protocol);
	    $install{$struct . $protocol} = $struct;
	}
    }

    foreach (@install) {
	$struct = $_;
	($index) = ($struct =~ /&([^\)]+)/);
	$struct_str = $struct;
	$struct_str =~ s/_cmd/_cmd_vtysh/;
	if (defined ($install{$index . $protocol}) && ! defined ($install_check{$struct})) {
	    $install_check{$struct} = $struct;
	    push (@init, $struct_str);
	}
    }
}

foreach (keys %hash) {
    if ($hash{$_} ne "ignore") {
	@{$hashp{$_}} = join ("|", @{$hashp{$_}});
	print "DEFSH (@{$hashp{$_}}$hash{$_})\n\n";
    }
}

print <<EOF;
void
vtysh_init_cmd ()
{
EOF

foreach (@init) {
    print;
}

print <<EOF
}
EOF
