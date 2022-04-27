#!/usr/bin/perl
#
# Copyright (c) 2007 Michael Taylor
# All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions
# are met:
# 1. Redistributions of source code must retain the above copyright
#    notice, this list of conditions and the following disclaimer,
#    without modification.
# 2. Redistributions in binary form must reproduce at minimum a disclaimer
#    similar to the "NO WARRANTY" disclaimer below ("Disclaimer") and any
#    redistribution must be conditioned upon including a substantially
#    similar Disclaimer requirement for further binary redistribution.
# 3. Neither the names of the above-listed copyright holders nor the names
#    of any contributors may be used to endorse or promote products derived
#    from this software without specific prior written permission.
#
# Alternatively, this software may be distributed under the terms of the
# GNU General Public License ("GPL") version 2 as published by the Free
# Software Foundation.
#
# NO WARRANTY
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
# ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
# LIMITED TO, THE IMPLIED WARRANTIES OF NONINFRINGEMENT, MERCHANTIBILITY
# AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL
# THE COPYRIGHT HOLDERS OR CONTRIBUTORS BE LIABLE FOR SPECIAL, EXEMPLARY,
# OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
# SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
# INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER
# IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
# ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
# THE POSSIBILITY OF SUCH DAMAGES.
#
# $Id: foo $
#
# This script is invoked at build time to regenerate a wrapper for the HAL
# binary API that adds locking and (optionally) tracing with human readable
# names.
#
use strict;
use warnings;

#
# This section defines the name translation from the binary HAL's function
# pointers to our API names.  Please provide all function names here.  If
# the wrapper is not needed, use undef.
#
my %wrapper_names = (
    "ah_beaconInit"               => "ath_hal_beaconinit",
    "ah_detach"                   => undef,
    "ah_disablePhyErrDiag"        => "ath_hal_disablePhyDiag",
    "ah_enablePhyErrDiag"         => "ath_hal_enablePhyDiag",
    "ah_enableReceive"            => "ath_hal_rxena",
    "ah_fillTxDesc"               => "ath_hal_filltxdesc",
    "ah_getAckCTSRate"            => "ath_hal_getackctsrate",
    "ah_setAckCTSRate"            => "ath_hal_setackctsrate",
    "ah_updateMibCounters"        => "ath_hal_updatemibcounters",
    "ah_getAntennaSwitch"         => "ath_hal_getantennaswitch",
    "ah_setAntennaSwitch"         => "ath_hal_setantennaswitch",
    "ah_getAckTimeout"            => "ath_hal_getacktimeout",
    "ah_getBssIdMask"             => "ath_hal_getbssidmask",
    "ah_getCapability"            => "ath_hal_getcapability",
    "ah_getChanNoise"             => "ath_hal_get_channel_noise",
    "ah_getCTSTimeout"            => "ath_hal_getctstimeout",
    "ah_getDefAntenna"            => "ath_hal_getdefantenna",
    "ah_getDiagState"             => "ath_hal_getdiagstate",
    "ah_getInterrupts"            => "ath_hal_intrget",
    "ah_getKeyCacheSize"          => "ath_hal_keycachesize",
    "ah_getMacAddress"            => "ath_hal_getmac",
    "ah_getPendingInterrupts"     => "ath_hal_getisr",
    "ah_getPowerMode"             => "ath_hal_getPowerMode",
    "ah_getRateTable"             => "ath_hal_getratetable",
    "ah_getRfGain"                => "ath_hal_getrfgain",
    "ah_getRxDP"                  => "ath_hal_getrxbuf",
    "ah_getRxFilter"              => "ath_hal_getrxfilter",
    "ah_getSlotTime"              => "ath_hal_getslottime",
    "ah_getTsf32"                 => "ath_hal_gettsf32",
    "ah_getTsf64"                 => "ath_hal_gettsf64",
    "ah_getTxDP"                  => "ath_hal_gettxbuf",
    "ah_getTxIntrQueue"           => "ath_hal_gettxintrtxqs",
    "ah_getTxQueueProps"          => "ath_hal_gettxqueueprops",
    "ah_gpioCfgOutput"            => "ath_hal_gpioCfgOutput",
    "ah_gpioSet"                  => "ath_hal_gpioset",
    "ah_gpioGet"                  => "ath_hal_gpioget",
    "ah_gpioSetIntr"              => "ath_hal_gpiosetintr",
    "ah_gpioCfgInput"             => "ath_hal_gpiCfgInput",
    "ah_isInterruptPending"       => "ath_hal_intrpend",
    "ah_isKeyCacheEntryValid"     => "ath_hal_keyisvalid",
    "ah_numTxPending"             => "ath_hal_numtxpending",
    "ah_perCalibration"           => "ath_hal_calibrate",
    "ah_phyDisable"               => "ath_hal_phydisable",
    "ah_disable"                  => "ath_hal_disable",
    "ah_procMibEvent"             => "ath_hal_mibevent",
    "ah_procRxDesc"               => "ath_hal_rxprocdesc",
    "ah_procTxDesc"               => "ath_hal_txprocdesc",
    "ah_radarWait"                => "ath_hal_radar_wait",
    "ah_releaseTxQueue"           => "ath_hal_releasetxqueue",
    "ah_reqTxIntrDesc"            => "ath_hal_txreqintrdesc",
    "ah_reset"                    => "ath_hal_reset",
    "ah_resetKeyCacheEntry"       => "ath_hal_keyreset",
    "ah_resetStationBeaconTimers" => "ath_hal_beaconreset",
    "ah_resetTsf"                 => "ath_hal_resettsf",
    "ah_resetTxQueue"             => "ath_hal_resettxqueue",
    "ah_rxMonitor"                => "ath_hal_rxmonitor",
    "ah_setAckTimeout"            => "ath_hal_setacktimeout",
    "ah_setBssIdMask"             => "ath_hal_setbssidmask",
    "ah_setCapability"            => "ath_hal_setcapability",
    "ah_setChannel"               => "ath_hal_setchannel",
    "ah_setCoverageClass"         => "ath_hal_setcoverageclass",
    "ah_setCTSTimeout"            => "ath_hal_setctstimeout",
    "ah_setDecompMask"            => "ath_hal_setdecompmask",
    "ah_setDefAntenna"            => "ath_hal_setdefantenna",
    "ah_setInterrupts"            => "ath_hal_intrset",
    "ah_setKeyCacheEntry"         => "ath_hal_keyset",
    "ah_setKeyCacheEntryMac"      => "ath_hal_keysetmac",
    "ah_setLedState"              => "ath_hal_setledstate",
    "ah_setMacAddress"            => "ath_hal_setmac",
    "ah_setMulticastFilter"       => "ath_hal_setmcastfilter",
    "ah_setMulticastFilterIndex"  => "ath_hal_setmcastfilterindex",
    "ah_setPCUConfig"             => "ath_hal_setopmode",
    "ah_setPowerMode"             => "ath_hal_setpower",
    "ah_setRxDP"                  => "ath_hal_putrxbuf",
    "ah_setRxFilter"              => "ath_hal_setrxfilter",
    "ah_setRegulatoryDomain"      => "ath_hal_setregulatorydomain",
    "ah_setSlotTime"              => "ath_hal_setslottime",
    "ah_setStationBeaconTimers"   => "ath_hal_beacontimers",
    "ah_setTxDP"                  => "ath_hal_puttxbuf",
    "ah_setTxQueueProps"          => "ath_hal_settxqueueprops",
    "ah_setTxPowerLimit"          => "ath_hal_settxpowlimit",
    "ah_setBeaconTimers"          => "ath_hal_setbeacontimers",
    "ah_setupRxDesc"              => "ath_hal_setuprxdesc",
    "ah_setupTxDesc"              => "ath_hal_setuptxdesc",
    "ah_setupTxQueue"             => "ath_hal_setuptxqueue",
    "ah_setupXTxDesc"             => "ath_hal_setupxtxdesc",
    "ah_startPcuReceive"          => "ath_hal_startpcurecv",
    "ah_startTxDma"               => "ath_hal_txstart",
    "ah_stopDmaReceive"           => "ath_hal_stopdmarecv",
    "ah_stopPcuReceive"           => "ath_hal_stoppcurecv",
    "ah_stopTxDma"                => "ath_hal_stoptxdma",
    "ah_updateCTSForBursting"     => "ath_hal_updateCTSForBursting",
    "ah_updateTxTrigLevel"        => "ath_hal_updatetxtriglevel",
    "ah_waitForBeaconDone"        => "ath_hal_waitforbeacon",
    "ah_writeAssocid"             => "ath_hal_setassocid",
    "ah_clrMulticastFilterIndex"  => "ath_hal_clearmcastfilter",
    "ah_detectCardPresent"        => "ath_hal_detectcardpresent"
);

#
# This text is copied verbatim to the top of the output header file
#
my $header = <<EOF
/*-
 * Copyright (c) 2007 Michael Taylor
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer,
	without modification.
 * 2. Redistributions in binary form must reproduce at minimum a disclaimer
 *    similar to the "NO WARRANTY" disclaimer below ("Disclaimer") and any
 *    redistribution must be conditioned upon including a substantially
 *    similar Disclaimer requirement for further binary redistribution.
 * 3. Neither the names of the above-listed copyright holders nor the names
 *    of any contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * Alternatively, this software may be distributed under the terms of the
 * GNU General Public License ("GPL") version 2 as published by the Free
 * Software Foundation.
 *
 * NO WARRANTY
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF NONINFRINGEMENT, MERCHANTIBILITY
 * AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL
 * THE COPYRIGHT HOLDERS OR CONTRIBUTORS BE LIABLE FOR SPECIAL, EXEMPLARY,
 * OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER
 * IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGES.
 */
/* **************************************************************
 * *   WARNING: THIS IS A GENERATED FILE.  PLEASE DO NOT EDIT   *
 * ************************************************************** */

#include "if_ath_hal_macros.h"
#ifdef CONFIG_KALLSYMS
#include "linux/kallsyms.h"
#endif /* #ifdef CONFIG_KALLSYMS */

#ifndef _IF_ATH_HAL_H_
#define _IF_ATH_HAL_H_
EOF
  ;

#
# This text is copied verbatim to the bottom of the output header file
#
my $footer = <<EOF

#include "if_ath_hal_wrappers.h"

#endif				/* #ifndef _IF_ATH_HAL_H_ */
 /* *** THIS IS A GENERATED FILE -- DO NOT EDIT *** */
 /* *** THIS IS A GENERATED FILE -- DO NOT EDIT *** */
 /* *** THIS IS A GENERATED FILE -- DO NOT EDIT *** */
EOF
  ;

# Parsed Function Data

# hash of string->string (hal's function name to return type)
my %return_types = ();

# hash of string->list of strings (ordered list of parameter names)
my %parameter_names = ();

# hash of string->list of strings (ordered list of parameter types)
my %parameter_types = ();

# Pick apart the return type, parameter types, and parameter names for each HAL function
sub parse_prototype($) {
    my $proto = shift @_;
    $proto =~
/^((?:(?:const|struct)\s*)*[^\s]+(?:[\s]*\*)?)[\s]*__ahdecl\(\*([^\)]*)\)\((.*)\);/;
    my $return_type   = $1;
    my $member_name   = $2;
    my $parameterlist = $3;

    my $api_name = $wrapper_names{$member_name};
    if ( exists $wrapper_names{$member_name} ) {
        if ( !defined $api_name ) {
            return;    # known name, but no wrapper needed
        }
    }
    else {
        print STDERR "No wrapper name for $member_name\n";
        exit 1;
    }

    $return_types{"$member_name"} = $return_type;
    @{ $parameter_names{"$member_name"} } = ();
    @{ $parameter_types{"$member_name"} } = ();
    my @parameters = split /,\s?/, $parameterlist;
    my $argnum = 0;

    foreach (@parameters) {
        s/ \*/\* /;
        /^((?:(?:const|struct|\*)\s*)*)([^\s]+\*?)\s*([^\s]*)\s*/;
        my $type = "$1$2";
        my $name = "$3";
        if ( 0 == length($name) ) {
            if ( $argnum == 0 && $type =~ /ath_hal/ ) {
                $name = "ah";
            }
            else {
                $name = "a$argnum";
            }
        }

        push @{ $parameter_names{$member_name} }, $name;
        push @{ $parameter_types{$member_name} }, $type;
        $argnum++;
    }
}

# Parse and scrub the hal structure's member function declarations
sub parse_input() {
    my $line_continued = 0;
    my $line_buffer    = "";
    foreach (<INPUT>) {
        chomp($_);
        s/\s+$//g;
        s/^\s+//g;
        s/\s+/ /g;
        if ( /__ahdecl\s*\(.*/ || $line_continued ) {
            $line_buffer .= "$_";
            if ( /__ahdecl.*;/ || ( $line_continued && /;/ ) ) {
                parse_prototype($line_buffer);
                $line_buffer    = "";
                $line_continued = 0;
            }
            else {
                $line_buffer .= " ";
                $line_continued = 1;
            }
        }
    }
}

# Arrange spaces in the type name nicely
sub format_type($) {
    my $type = shift @_;
    if ( $type =~ /\*$/ ) {
        $type =~ s/^([^*]*[^*\s])\s*(\*+)$/$1 $2/;
    }
    else {
        $type .= " ";
    }
    return $type;
}

# Generate the header file
sub generate_output() {
    print OUTPUT $header;

    for my $member_name ( keys %return_types ) {
        my $api_name        = $wrapper_names{$member_name};
        my $api_return_type = $return_types{$member_name};
        my $ret_void        = ( $api_return_type =~ /void/ );
        print OUTPUT "\nstatic inline "
          . format_type($api_return_type)
          . "$api_name(";
        my @names = @{ $parameter_names{$member_name} };
        my @types = @{ $parameter_types{$member_name} };
        for my $i ( 0 .. $#names ) {
            if ($i) {
                print OUTPUT ", ";
            }
            print OUTPUT format_type( $types[$i] ) . $names[$i];
        }
        print OUTPUT ")\n{";
        if ( !$ret_void ) {
            print OUTPUT "\n\t" . format_type($api_return_type) . "ret;";
        }
        print OUTPUT "\n\tATH_HAL_LOCK_IRQ(ah->ah_sc);";
        print OUTPUT "\n\tath_hal_set_function(__func__);";
        print OUTPUT "\n\t";
        if ( !$ret_void ) {
            print OUTPUT "ret = ";
        }

        print OUTPUT "ah->$member_name(";
        for my $j ( 0 .. $#names ) {
            if ($j) {
                print OUTPUT ", ";
            }
            print OUTPUT $names[$j];
        }
        print OUTPUT ");";
        print OUTPUT "\n\tath_hal_set_function(NULL);";
        print OUTPUT "\n\tATH_HAL_UNLOCK_IRQ(ah->ah_sc);";
        if ( !$ret_void ) {
            print OUTPUT "\n\treturn ret;";
        }
        print OUTPUT "\n}\n";
    }
    print OUTPUT "\n/* Example script to create a HAL function unmangling SED script: ";
    print OUTPUT "\n";
    print OUTPUT "\n   dmesg -c &>/dev/null && iwpriv ath0 dump_hal_map && dmesg | \\";
    print OUTPUT "\n           sed -n -r -e \"/zz[0-9a-f]{8}/ { s~^([^+]*)[^=]*=(.*)~s/\\1\\\/\\2 (\\1)/g~; p; } \" \\";
    print OUTPUT "\n           >hal_unmangle.sed";
    print OUTPUT "\n";
    print OUTPUT "\n * Example usage:";
    print OUTPUT "\n";
    print OUTPUT "\n           tail -f /var/log/messages | sed -f hal_unmangle.sed ";
    print OUTPUT "\n */";
    print OUTPUT "\nstatic inline void ath_hal_dump_map(struct ath_hal* ah) {";
    print OUTPUT "\n#ifdef CONFIG_KALLSYMS\n";
    for my $member_name ( keys %return_types ) {
	my $api_name        = $member_name;
	my $api_return_type = $return_types{$member_name};
	my $ret_void        = ( $api_return_type =~ /void/ );
	print OUTPUT "\n\t/* "
	  . format_type($api_return_type)
	  . "$api_name(";
	my @names = @{ $parameter_names{$member_name} };
	my @types = @{ $parameter_types{$member_name} };
	for my $i ( 0 .. $#names ) {
	    if ($i) {
		print OUTPUT ", ";
	    }
	    print OUTPUT format_type( $types[$i] ) . $names[$i];
	}
	print OUTPUT ") */";
	print OUTPUT "\n\t\t__print_symbol(\"%s=" . $member_name 
	   . "\\n\", (unsigned long)ah->"
	   . $member_name . ");";
    }
    print OUTPUT "\n#else /* #ifdef CONFIG_KALLSYMS */\n";
    print OUTPUT "\nprintk(\"To use this feature you must enable "
	       . "CONFIG_KALLSYMS in your kernel.\");\n";
    print OUTPUT "\n#endif /* #ifndef CONFIG_KALLSYMS */\n";
    print OUTPUT "\n}\n";
    print OUTPUT $footer;
}

sub main () {

    # Get input and output files from the arguments
    if ( $#ARGV != 1 ) {
        print STDERR "Need two arguments: input and output\n";
        exit 1;
    }

    my $input_header  = $ARGV[0];
    my $output_header = $ARGV[1];

    if ( !open INPUT, "<$input_header" ) {
        die "Cannot open \"$input_header\": $!";
    }
    parse_input();
    close INPUT;

    if ( !open OUTPUT, ">$output_header" ) {
        close INPUT;
        die "Cannot open \"$output_header\": $!";
    }
    generate_output();
    close OUTPUT;
}

main();
