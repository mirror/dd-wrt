# {{{  Banner                                                   

# ============================================================================
# 
#      ethernet.tcl
# 
#      Ethernet support for the eCos synthetic target I/O auxiliary
# 
# ============================================================================
# ####COPYRIGHTBEGIN####
#                                                                           
#  ----------------------------------------------------------------------------
#  Copyright (C) 2002 Bart Veer
# 
#  This file is part of the eCos host tools.
# 
#  This program is free software; you can redistribute it and/or modify it 
#  under the terms of the GNU General Public License as published by the Free 
#  Software Foundation; either version 2 of the License, or (at your option) 
#  any later version.
#  
#  This program is distributed in the hope that it will be useful, but WITHOUT 
#  ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or 
#  FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for 
#  more details.
#  
#  You should have received a copy of the GNU General Public License along with
#  this program; if not, write to the Free Software Foundation, Inc., 
#  59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
#  ----------------------------------------------------------------------------
#                                                                           
# ####COPYRIGHTEND####
# ============================================================================
# #####DESCRIPTIONBEGIN####
# 
#  Author(s):   bartv
#  Contact(s):  bartv
#  Date:        2002/08/07
#  Version:     0.01
#  Description:
#      Implementation of the ethernet device. This script should only ever
#      be run from inside the ecosynth auxiliary.
# 
# ####DESCRIPTIONEND####
# ============================================================================

# }}}

# Overview.
#
# Linux provides a number of different ways of performing low-level
# ethernet I/O from user space, including accessing an otherwise
# unused ethernet card via a PF_PACKET socket, and the tap facility.
# The necessary functionality is not readily accessible from Tcl,
# and performing this low-level I/O generally requires special
# privileges. Therefore the actual I/O happens in a C program
# rawether, installed suid root,
#
# The synthetic ethernet package supports up to four ethernet devices,
# eth0 to eth3. The target definition file maps these onto the
# underlying I/O facility. Instantiation requires spawning a rawether
# process with appropriate arguments, and then waiting for a message
# from that process indicating whether or not the instantiation
# succeeded. That message includes the MAC address. A file event
# handler is installed to handle data detected by raw ether.
#
# eCos can send a number of requests: transmit a packet, start the
# interface (possibly in promiscuous mode), stop the interface,
# or get the various parameters such as the MAC address. All those
# requests can just be passed on to the rawether process. Incoming
# ethernet packets are slightly more complicated: rawether will
# immediately pass these up to this Tcl script, which will buffer
# the packets until they are requested by eCos; in addition an
# interrupt will be raised.

namespace eval ethernet {
    # The protocol between eCos and this script.
    variable SYNTH_ETH_TX           0x01
    variable SYNTH_ETH_RX           0x02
    variable SYNTH_ETH_START        0x03
    variable SYNTH_ETH_STOP         0x04
    variable SYNTH_ETH_GETPARAMS    0x05
    variable SYNTH_ETH_MULTIALL     0x06
    
    # This array holds all the interesting data for all the
    # interfaces, indexed by the instance id. It is also useful
    # to keep track of the instance id's associated with ethernet
    # devices.
    array set data [list]
    set ids [list]

    # One-off initialization, for example loading images. If this fails
    # then all attempts at instantiation will fail as well.
    variable init_ok 1
    variable install_dir $synth::device_install_dir
    variable rawether_executable [file join $ethernet::install_dir "rawether"]

    if { ![file exists $rawether_executable] } {
	synth::report_error "Ethernet device, rawether executable has not been installed in $ethernet::install_dir.\n"
	set init_ok 0
    } elseif { ![file executable $rawether_executable] } {
	synth::report_error "Ethernet device, installed program $rawether_executable is not executable.\n"
	set init_ok 0
    }

    if { $synth::flag_gui } {
	foreach _image [list "netrecord.xbm"] {
	    variable image_[file rootname $_image]
	    if { ! [synth::load_image "ethernet::image_[file rootname $_image]" [file join $ethernet::install_dir $_image]] } {
		set init_ok 0
	    }
	}
	unset _image
    }
    
    # Maximum number of packets that should be buffered per interface.
    # This can be changed in the target definition
    variable max_buffered_packets   16

    if { [synth::tdf_has_option "ethernet" "max_buffer"] } {
	set ethernet::max_buffered_packets [synth::tdf_get_option "ethernet" "max_buffer"]
	if { ![string is integer -strict $ethernet::max_buffered_packets] } {
	    synth::report_error "Ethernet device, invalid value in target definition file $synth::target_definition\n   \
		                 Entry max_buffer should be a simple integer, not $ethernet::max_buffered_packets\n"
	    set init_ok 0
	}
    }

    # Define hooks for tx and rx packets
    synth::hook_define "ethernet_tx"
    synth::hook_define "ethernet_rx"

    # Get a list of known ethernet devices
    proc devices_get_list { } {
	set result [list]
	foreach id $ids {
	    lappend result $::ethernet::data($id,name)
	}
	return $result
    }
    
    # ----------------------------------------------------------------------------
    proc instantiate { id name data } {
	if { ! $ethernet::init_ok } {
	    synth::report_warning "Cannot instantiate ethernet device $name, initialization failed.\n"
	    return ""
	}
	
	# id is a small number that uniquely identifies this device. It will
	# be used as an array index.
	# name is something like eth0 or eth1
	# There should be no device-specific data

	# The hard work is done by an auxiliary process which needs to be
	# spawned off. It requires some additional information to map the
	# eCos device name on to a suitable Linux network device such
	# as tap0. That information has to come from the config file.
	if { ![synth::tdf_has_option "ethernet" $name] } {
	    synth::report_error "Cannot instantiate ethernet device $name\n   \
		    No entry in target definition file $synth::target_definition\n"
	    return ""
	}
	set use [synth::tdf_get_option "ethernet" $name]

	# Do some validation here, before the rawether process is started.
	# Typical entries would look like
	#     eth0 real eth1
	#     eth1 ethertap [[tap-device] [MAC] [persistent]]
	set junk ""
	set optional ""
	set mac      ""
	if { [regexp -- {^\s*real\s*[a-zA-z0-9_]+$} $use] } {
	    # Real ethernet.
	} elseif { [regexp -- {^\s*ethertap\s*(.*)$} $use junk optional ] } {
	    if { "" != $optional } {
		if { ! [regexp -- {^tap[0-9]+\s*(.*)$} $optional junk mac ] } {
		    synth::report_error "Cannot instantiate ethernet device $name\n   \
			    Invalid entry \"$use\" in target definition file $synth::target_definition\n   \
			    Should be \"ethertap \[<tap-device> \[<MAC address>\]\] [persistent]\"\n"
		    return ""
		}
		if { "" != $mac } {
		    if { ! [regexp -- {^\s*([0-9a-fA-F]{2}:){5}[0-9a-fA-F]{2}\s*} $mac ] } {
			synth::report_error "Cannot instantiate ethernet device $name\n   \
				Invalid entry \"$use\" in target definition file $synth::target_definition\n   \
				MAC address should be of the form xx:xx:xx:xx:xx:xx, all hexadecimal digits.\n"
			return ""
		    }
		}
	    }
	} else {
	    synth::report_error "Cannot instantiate ethernet device $name\n   \
		    Invalid entry \"$use\" in target definition file $synth::target_definition\n   \
		    Should be \"real <Linux ethernet device>\" or \"ethertap \[<tap-device> \[<MAC address>\]\]\"\n"
	    return ""
	}

	# Now spawn the rawether process. Its stdin and stdout are
        # pipes connected to ecosynth. Its stderr is redirected to
	# the current tty to avoid confusion between incoming ethernet
	# packets and diagnostics.
	if { [catch { set rawether [open "|$ethernet::rawether_executable $use 2>/dev/tty" w+] } message ] } {
	    synth::report_error "Failed to spawn rawether process for device $name\n   $message"
	    return ""
	}

	# No translation on this pipe please.
	fconfigure $rawether -translation binary -encoding binary -buffering none 

	# Now wait for the rawether device to initialize. It should send back a single
	# byte, '0' for failure or '1' for success. Failure is followed by a text
	# message which should be reported. Success is followed by a six-byte MAC
	# address.
	set reply [read $rawether 1]
	if { "" == $reply } {
	    synth::report_error "rawether process for device $name exited unexpectedly.\n"
	    catch { close $rawether }
	    return ""
	}

	if { "1" != $reply } {
	    set message [read $rawether 1024]
	    synth::report_error "rawether process was unable to initialize eCos device $name ($use)\n    $message"
	    catch { close $rawether }
	    return ""
	}

	set reply [read $rawether 7]
	if { [string length $reply] != 7 } {
	    synth::report_error "rawether process for eCos device $name ($use) failed to provide the initialization response.\n"
	    catch { close $rawether }
	    return ""
	}
	set mac [string range $reply 0 5]
	set multi [string index $reply 6]

	# Finally allocate an interrupt vector
	set vector [synth::interrupt_allocate $name]
	if { -1 == $vector } {
	    # No more interrupts left. An error will have been reported already.
	    catch { close $rawether }
	    return ""
	}
	
	# The device is up and running. Fill in the array entries
	lappend ethernet::ids                       $id
	set ethernet::data($id,alive)               1
	set ethernet::data($id,name)                $name
	set ethernet::data($id,rawether)            $rawether
	set ethernet::data($id,packets)             [list]
	set ethernet::data($id,packet_count)        0
	set ethernet::data($id,up)                  0
	set ethernet::data($id,interrupt_vector)    $vector
	set ethernet::data($id,MAC)                 $mac
	set ethernet::data($id,multi)               $multi

	# Set up the event handler to handle incoming packets. There should
	# not be any until the interface is brought up
	fileevent $rawether readable [list ethernet::handle_packet $name $id $rawether]

	# Finally return the request handler. The eCos device driver will
	# automatically get back an ack.
	return ethernet::handle_request
    }

    # ----------------------------------------------------------------------------
    # eCos has sent a request to a device instance. Most of these requests should
    # just be forwarded to rawether. Some care has to be taken to preserve
    # packet boundaries and avoid confusion. It is also necessary to worry
    # about the rawether process exiting unexpectedly, which may cause
    # puts operations to raise an error (subject to buffering).
    #
    # Note: it might actually be more efficient to always send a header plus
    # 1514 bytes of data, reducing the number of system calls at the cost of
    # some extra data copying, but with at least two process switches per
    # ethernet transfer efficiency is not going to be particularly good
    # anyway.
    
    proc send_rawether { id packet } {
	if { $ethernet::data($id,alive) } {
	    set chan $ethernet::data($id,rawether)
	    if { [catch { puts -nonewline $chan $packet } ] } {
		set ethernet::data($id,alive) 0
		# No further action is needed here, instead the read handler
		# will detect EOF and report abnormal termination.
	    }
	}
    }
    
    proc handle_request { id reqcode arg1 arg2 reqdata reqlen reply_len } {

	if { $reqcode == $ethernet::SYNTH_ETH_TX } {
	    # Transmit a single packet. To preserve packet boundaries
	    # this involves a four-byte header containing opcode and
	    # size, followed by the data itself.
	    set header [binary format "ccs" $reqcode 0 [string length $reqdata]]
	    ethernet::send_rawether $id $header
	    ethernet::send_rawether $id $reqdata
	    if { $ethernet::logging_enabled } {
		ethernet::log_packet $ethernet::data($id,name) "tx" $reqdata
	    }
	    synth::hook_call "ethernet_tx" $ethernet::data($id,name) $reqdata
	    
	} elseif { $reqcode == $ethernet::SYNTH_ETH_RX } {
	    # Return a single packet to eCos, plus a count of the number
	    # of remaining packets. All packets are buffered here, not
	    # in rawether.
	    if { $ethernet::data($id,packet_count) == 0 } {
		synth::send_reply 0 0 ""
	    } else {
		incr ethernet::data($id,packet_count) -1
		set packet [lindex $ethernet::data($id,packets) 0]
		set ethernet::data($id,packets) [lrange $ethernet::data($id,packets) 1 end]
		synth::send_reply $ethernet::data($id,packet_count) [string length $packet] $packet
		if { $ethernet::logging_enabled } {
		    ethernet::log_packet $ethernet::data($id,name) "rx" $packet
		}
		synth::hook_call "ethernet_rx" $ethernet::data($id,name) $packet
	    }
	} elseif { $reqcode == $ethernet::SYNTH_ETH_START } {
	    # Start the interface in either normal or promiscuous
	    # mode, depending on arg1. No reply is expected. Also
	    # mark the interface as up so that any packets transmitted
	    # by rawether will not be discarded
	    set ethernet::data($id,up) 1
	    set header [binary format "ccs" $reqcode $arg1 0]
	    ethernet::send_rawether $id $header
	} elseif { $reqcode == $ethernet::SYNTH_ETH_STOP } {
	    # Stop the interface. All pending packets should be
	    # discarded and no new packets should be accepted.
	    # No reply is expected so just pass this on to rawether
	    set ethernet::data($id,up) 0
	    set ethernet::data($id,packets) [list]
	    set ethernet::data($id,packet_count) 0
	    set header [binary format "ccs" $reqcode 0 0]
	    ethernet::send_rawether $id $header
	} elseif { $reqcode == $ethernet::SYNTH_ETH_GETPARAMS } {
	    # Retrieve the interrupt number, the MAC address,
	    # and the multicast flag for this interface. eCos should be
	    # expecting back 6 bytes of data for the MAC, plus an
	    # extra byte for the multi flag, and the interrupt
	    # number as the return code. This is all known locally.
	    set reply "$ethernet::data($id,MAC)$ethernet::data($id,multi)"
	    synth::send_reply $ethernet::data($id,interrupt_vector) 7 $reply
	} elseif { $reqcode == $ethernet::SYNTH_ETH_MULTIALL } {
	    set header [binary format "ccs" $reqcode $arg1 0]
	    ethernet::send_rawether $id $header
	} else {
	    synth::report_error "Received unexpected request $reqcode for ethernet device"
	}
    }

    # ----------------------------------------------------------------------------
    # Incoming data.
    #
    # The rawether process continually reads packets from the low-level device
    # and tries to forward them on to this script, where they will be received
    # by an event handler. The packet consists of a four-byte header containing
    # the size, followed by the ethernet data itself. This ensures that
    # packet boundaries are preserved. Incoming packets are buffered inside
    # the auxiliary until eCos sends an RX request, and an interrupt is
    # generated.
    #
    # If eCos stops accepting data or if it cannot process the ethernet packets
    # quickly enough then the auxiliary could end up buffering an unbounded
    # amount of data. That is a bad idea, so there is an upper bound on the
    # number of buffered packets. Any excess packets get dropped.
    #
    # Error conditions or EOF indicate that rawether has terminated. This
    # should not happen during normal operation. rawether should only exit
    # because of an ecos_exit hook when the channel gets closed, and the
    # event handler gets removed first.
    #
    # Incoming packets are logged when they are received by eCos, not when
    # they are received from the rawether device. That gives a somewhat more
    # accurate view of what is happening inside eCos - a packet stuck in
    # a fifo has little impact.
    proc _handle_packet_error { msg id } {
	append msg "    No further I/O will happen on this interface.\n"
	synth::report_warning $msg
	set ethernet::data($id,alive) 0
	fileevent $ethernet::data($id,rawether) readable ""
	catch { close $ethernet::data($id,rawether) }
    }
    
    proc handle_packet { name id chan } {
	set header [read $chan 4]
	if { 4 != [string length $header] } {
	    ethernet::_handle_packet_error "rawether process for $name has terminated unexpectedly.\n" $id
	    return
	}

	binary scan $header "ccs" code arg1 len
	if { $ethernet::SYNTH_ETH_RX  != $code } {
	    set msg    "protocol mismatch from rawether process for $name\n"
	    append msg "    Function code $code not recognised.\n"
	    ethernet::_handle_packet_error $msg $id
	    return
	}
	if { ($len < 14) || ($len > 1514) } {
	    set msg    "protocol mismatch from rawether process for $name\n"
	    append msg "    Invalid transfer length $len\n"
	    ethernet::_handle_packet_error $msg $id
	    return
	}

	set data [read $chan $len]
	if { $len != [string length $data] } {
	    set msg    "protocol mismatch from rawether process for $name\n"
	    append msg "    Expected $len byte ethernet packet, received [string length $data] bytes\n"
	    ethernet::_handle_packet_error $msg $id
	    return
	}

	# The data has been received correctly. Should it be buffered?
	if { !$ethernet::data($id,up) } {
	    return
	}
	if { $ethernet::data($id,packet_count) >= $ethernet::max_buffered_packets } {
	    return
	}

	# Store the packet, and inform eCos there is work to be done
	lappend ethernet::data($id,packets) $data
	incr ethernet::data($id,packet_count)
	synth::interrupt_raise $ethernet::data($id,interrupt_vector)
	
    }
    
    # ----------------------------------------------------------------------------
    # When eCos has exited, the rawether processes can and should be
    # shut down immediately.
    proc ecos_exited { arg_list } {
	foreach id $ethernet::ids {
	    if { $ethernet::data($id,alive) } {
		set ethernet::data($id,alive) 0
		fileevent $ethernet::data($id,rawether) readable ""
		catch { close $ethernet::data($id,rawether) }
	    }
	}
    }
    synth::hook_add "ecos_exit" ethernet::ecos_exited

    # ----------------------------------------------------------------------------
    # Read in various data files for use by the filters
    #
    # Other possible sources of information include arp, ypcat, and
    # dns. Those are avoided for now because they involve running
    # additional processes that might hang for a while. Also arp
    # would only give useful information for very recently accessed
    # machines, NIS might not be running, and dns could involve an
    # expensive lookup while the system is running .
    
    array set services [list]
    array set hosts [list]
    array set protocols [list]
    
    proc read_services { } {
	catch {
	    set fd [open "/etc/services" "r"]
	    while { -1 != [gets $fd line] } {
		set junk     ""
		set name     ""
		set number   ""
		set protocol ""
		if { [regexp -- {^([-a-zA-Z0-9_]+)\s*([0-9]+)/((?:tcp)|(?:udp)).*$} $line junk name number protocol] } {
		    set ethernet::services($number,$protocol) $name
		}
	    }
	    close $fd
	}
    }

    proc read_protocols { } {
	catch {
	    set fd [open "/etc/protocols" "r"]
	    while { -1 != [gets $fd line] } {
		set junk   ""
		set name   ""
		set number ""
		if { [regexp -- {^([-a-zA-Z0-9_]+)\s*([0-9]+)\s.*} $line junk name number] } {
		    set ethernet::protocols($number) $name
		}
	    }
	    close $fd
	}
    }
    
    proc read_hosts { } {
	catch {
	    set fd [open "/etc/hosts" "r"]
	    while { -1 != [gets $fd line] } {
		set junk   ""
		set name   ""
		set number ""

		# Deliberately ignore parts of the name after the first .
		if { [regexp -- {^([0-9]{1,3}.[0-9]{1,3}.[0-9]{1,3}.[0-9]{1,3})\s*([a-zA-Z0-9]+)(\.|\s|$)} $line junk number name] } {
		    # The number should be naturalized if it is going to match reliably
		    scan $line "%d.%d.%d.%d" a b c d
		    set index [expr (($a & 0x0FF) << 24) | (($b & 0x0FF) << 16) | (($c & 0x0FF) << 8) | ($d & 0x0FF)]
		    set ethernet::hosts($index) $name
		}
	    }
	    close $fd
	}
    }
    
    # ----------------------------------------------------------------------------
    # Filtering support. This is only really used when running in GUI mode.
    # However all the relevant options are still extracted and validated,
    # to avoid warnings about unrecognised options.
    
    variable logging_enabled 0
    variable max_show        64

    # Construct a string for the data, either all of it or up to max_show bytes.
    # This is just hex in chunks of four bytes.
    proc format_hex_data { data } {
	set result ""

	set len [string length $data]
	if { $len > $ethernet::max_show } {
	    set len $ethernet::max_show
	}
	binary scan $data "H[expr 2 * $len]" hex
	for { set i 0 } { $i < $len } { incr i 4 } {
	    append result "[string range $hex [expr $i * 2] [expr ($i * 2) + 7]] "
	}
	set result [string trimright $result]
	return $result
    }

    # Given an IPv4 network address, turn it into a.b.c.d and the
    # host name as well (if known). The argument should be a 32-bit
    # integer.
    proc inet_ipv4_ntoa { number } {
	set result [format "%d.%d.%d.%d" [expr ($number >> 24) & 0x0FF] [expr ($number >> 16) & 0x0FF] \
		[expr ($number >> 8) & 0x0FF] [expr $number & 0x0FF]]
	if { [info exists ethernet::hosts($number) ] } {
	    append result "($ethernet::hosts($number))"
	}
	return $result
    }

    # Given an ipv4 address encapsulated in an IPv6 address, do the necessary
    # conversion. We have something like 123:4567, we want a.b.c.d plus
    # a host address.
    proc inet_ipv4_in_ipv6_ntoa { top bottom } {
	if { "" == $top } {
	    set top 0
	}
	if { "" == $bottom } {
	    set bottom 0
	}
	set top "0x$top"
	set bottom "0x$bottom"

	set ipv4 [expr ($top << 16) | $bottom]
	return inet_ipv4_ntoa $ipv4
    }
    
    # Ditto for IPv6. The argument should be a 32-digit hexadecimal string.
    # For now there is no simple way of mapping these onto host names,
    # unless the address is an IPv4-mapped or compatible one, or one of
    # special cases such as loopback.
    proc inet_ipv6_ntoa { number } {
	# We have something like 12345678abcdef. Start by inserting the appropriate
	# colons.
	set result [format "%s:%s:%s:%s:%s:%s:%s:%s" [string range $number 0 3] [string range $number 4 7] \
		[string range $number 8 11] [string range $number 12 15] [string range $number 16 19] \
		[string range $number 20 23] [string range $number 24 27] [string range $number 28 31]]
	# Now eliminate unwanted 0's at the start of each range.
	regsub {^0+} $result {} result
	regsub -all {:0+} $result {:} result

	# If we have ended up with sequences of colons, abbreviate
        # them into pairs.
	regsub -all {::+} $result {::} result

	# There are a couple of special addresses
	if { "::1" == $result } {
	    return "::1(loopback)"
	} elseif { "::" == $result } {
	    return "::(IN6ADDR_ANY)"
	}

	# Look for IPv4-mapped addresses.
	set junk ""
	set ipv4_1 ""
	set ipv4_2 ""
	if { [regexp -nocase -- {::ffff:([0-9a-f]{0,3}):([0-9a-f]{0,3})$} $result junk ipv4_1 ipv4_2] } {
	    set result [inet_ipv4_in_ipv6_nto $ipv4_1 $ipv4_2]
	    return "::FFFF:$result"
	} elseif { [regexp -nocase -- {::([0-9a-f]{0,3}):([0-9a-f]{0,3})$} $result junk ipv4_1 ipv4_2] } {
	    set result [inet_ipv4_in_ipv6_nto $ipv4_1 $ipv4_2]
	    return "::$result"
	} else {
	    # Could still be aggregatable global unicast, link-local, site-local or multicast.
	    # But not decoded further for now.
	    return $result
	}
    }
    
    proc log_packet { device direction packet } {
	if { [string length $packet] < 14 } {
	    return
	}
	binary scan $packet {H2H2H2H2H2H2 H2H2H2H2H2H2 S} dest5 dest4 dest3 dest2 dest1 dest0 src5 src4 src3 src2 src1 src0 eth_protocol
	set packet [string range $packet 14 end]
	
	set ether_msg "$device $direction: [string length $packet] bytes, "
	append ether_msg [format ">%s:%s:%s:%s:%s:%s <%s:%s:%s:%s:%s:%s" $dest5 $dest4 $dest3 $dest2 $dest1 $dest0 $src5 $src4 $src3 $src2 $src1 $src0]
	set eth_protocol [expr $eth_protocol & 0x0FFFF]
	if { $eth_protocol <= 1536 } {
	    append ether_msg " 802.3 "
	    if { [string length $packet] < 8 } {
		return
	    }
	    binary scan $packet {a6 S} junk eth_protocol
	    set packet [string range $packet 8 end]
	}
	append ether_msg [format " %04x" $eth_protocol]
	if { $eth_protocol == 0x0800 } {
	    append ether_msg "(ip)"
	} elseif { $eth_protocol == 0x00806 } {
	    append ether_msg "(arp)"
	} elseif { $eth_protocol == 0x08035 } {
	    append ether_msg "(rarp)"
	}
	append ether_msg " [ethernet::format_hex_data $packet]\n"
	synth::output $ether_msg "eth_ether"

	if { 0x0806 == $eth_protocol } {
	    # An ARP request. This should always be 28 bytes.
	    if { [string length $packet] < 28 } {
		return
	    }
	    binary scan $packet {SSccS H2H2H2H2H2H2 I H2H2H2H2H2H2 I} hard_type prot_type hard_size prot_size op \
		    sender5 sender4 sender3 sender2 sender1 sender0 sender_ip \
		    target5 target4 target3 target2 target1 target0 target_ip
	    set hard_type [expr $hard_type & 0x0FFFF]
	    set prot_type [expr $prot_type & 0x0FFFF]
	    set hard_size [expr $hard_size & 0x0FF]
	    set prot_size [expr $prot_size & 0x0FF]
	    set op        [expr $op & 0x0FFFF]
	    set sender_ip [expr $sender_ip & 0x0FFFFFFFF]
	    set target_ip [expr $target_ip & 0x0FFFFFFFF]

	    set arp_msg "$device $direction: ARP "
	    if { $op == 1 } {
		append arp_msg "request "
	    } elseif { $op == 2 } {
		append arp_msg "reply "
	    } else {
		append_arp_msg "<unknown opcode> "
	    }
	    if { $hard_type != 1 } {
		append arp_msg "(unexpected hard_type field $hard_type, should be 1) "
	    }
	    if { $prot_type != 0x0800 } {
		append arp_msg "(unexpected prot_type field $prot_type, should be 0x0800) "
	    }
	    if { $hard_size != 6 } {
		append arp_msg "(unexpected hard_size field $hard_size, should be 6) "
	    }
	    if { $prot_size != 4 } {
		append arp_msg "(unexpected prot_size field $prot_size, should be 4) "
	    }
	    append arp_msg [format ", sender %s:%s:%s:%s:%s:%s " $sender5 $sender4 $sender3 $sender2 $sender1 $sender0]
	    append arp_msg [ethernet::inet_ipv4_ntoa $sender_ip]
	    append arp_msg [format ", target %s:%s:%s:%s:%s:%s " $target5 $target4 $target3 $target2 $target1 $target0]
	    append arp_msg [ethernet::inet_ipv4_ntoa $target_ip]
	    append arp_msg "\n"

	    synth::output $arp_msg "eth_arp"
	    return
	}

	if { 0x0800 != $eth_protocol } {
	    return
	}

	# We have an IP packet. Is this IPv4 or IPv6? The first byte contains
	# the version and the overall length of the IP header in 32-bit words
	if { [string length $packet] < 20 } {
	    return
	}
	binary scan $packet {c} tmp
	set ip_version [expr ($tmp >> 4) & 0x0F]
	set ip_hdrsize [expr $tmp & 0x0F]
	if { 4 == $ip_version } {
	    binary scan $packet {ccSSSccSII} tmp tos len id frag ttl ip_protocol checksum source_ip dest_ip
	    set ipv4_msg "$device $direction: IPv4"
	    if { 0 != $tos } {
		append ipv4_msg [format " tos %02x," [expr $tos & 0x0FF]]
	    }
	    append ipv4_msg [format " len %d, id %d," [expr $len & 0x0FFFF] [expr $id & 0x0FFFF]]
	    if { 0 != $frag } {
		append ipv4_msg [format " frag %u" [expr 8 * ($frag & 0x01FFF)]]
		if { 0 != ($frag & 0x04000) } {
		    append ipv4_msg " DF"
		}
		if { 0 != ($frag & 0x02000) } {
		    append ipv4_msg " MF"
		}
		append ipv4_msg ","
	    }
	    append ipv4_msg [format " ttl %d," $ttl]
	    set ip_protocol [expr $ip_protocol & 0x0FF]
	    if { [info exists ethernet::protocols($ip_protocol)] } {
		append ipv4_msg " $ethernet::protocols($ip_protocol),"
	    } else {
		append ipv4_msg [format " protocol %d" $ip_protocol]
	    }

	    set source_name [ethernet::inet_ipv4_ntoa $source_ip]
	    set dest_name   [ethernet::inet_ipv4_ntoa $dest_ip]
	    append ipv4_msg " >${dest_name}, <${source_name}\n"

	    synth::output $ipv4_msg "eth_ipv4"

	    # If this packet is a fragment other than the first, do not try to decode
	    # subsequent packets. The header information will not be present.
	    if { 0 != ($frag & 0x01FFF)} {
		return
	    }
	    set packet [string range $packet [expr 4 * $ip_hdrsize] end]
	    
	} elseif { 6 == $ip_version } {
	    if { [string length $packet] < 40 } {
		return
	    }
	    binary scan $packet {ISccH16H16} flow payload_length next_header hop_limit source_ip dest_ip
	    set ipv6_msg "$device $direction: IPv6"
	    set prio [expr ($flow & 0x0F000000) >> 24]
	    set flow [expr $flow & 0x00FFFFFF]
	    if { 0 != $flow } {
		append ipv6_msg [format " flow %04x prio %x," $flow $prio]
	    }
	    append ipv6_msg " payload [expr $payload bytes & 0x0FFFF],"
	    append ipv6_msg " hop limit [expr $hop_limit & 0x0FF],"
	    set next_header [expr $next_header & 0x0FF]
	    if { [info exists ethernet::protocols($next_header)] } {
		append ipv6_msg " $ethernet::protocols($next_header),"
	    } else {
		append ipv6_msg [format " protocol %d," $next_header]
	    }

	    set source_name [ethernet::inet_ipv6_ntoa $source_ip]
	    set dest_name [ethernet::inet_ipv6_ntoa $dest_ip]
	    append ipv6_msg " >${dest_name}, <${source_name}\n"

	    synth::output $ipv6_msg "eth_ipv6"
	    
	    set packet [string range $packet 40 end]
	    
	} else {
	    synth::output "$device $direction: unknown IP version $ip_version\n" "eth_ipv4"
	    return
	}


	# Now for some known protocols, icmp, tcp, udp and icmpv6
	# Possible ipv6-frag should be handled here as well. The
	# fragment header should be followed by another header such
	# as tcp or udp.
	if { 1 == $ip_protocol } {
	    # ipv4 ICMP
	    if { [string length $packet] < 4 } {
		return
	    }
	    binary scan $packet {ccS} code type checksum

	    set icmpv4_msg "$device $direction: ICMPv4 "
	    set error 0
	    set data  0
	    switch -- $code {
		0 {
		    append icmpv4_msg "ping reply"
		    if { [string length $packet] >= 8 } {
			# The id and seq are in the sender's format, not network format.
			# We have to assume either little or bigendian, so go for the former
			binary scan $packet {iss} junk id seq
			append icmpv4_msg [format " id %u, seq %u" [expr $id & 0x0FFFF] [expr $seq & 0x0FFFF]]
			set data 1
			set packet [string range $packet 8 end]
		    }
		}
		3 {
		    append icmpv4_msg "unreachable/"
		    switch -- $type {
			 0   { append icmpv4_msg "network" }
			 1   { append icmpv4_msg "host" }
			 2   { append icmpv4_msg "protocol" }
			 3   { append icmpv4_msg "port" }
			 4   { append icmpv4_msg "frag needed but don't frag set" }
			 5   { append icmpv4_msg "source route failed" }
			 6   { append icmpv4_msg "destination network unknown" }
			 7   { append icmpv4_msg "destination host unknown" }
			 8   { append icmpv4_msg "source host isolated" }
			 9   { append icmpv4_msg "destination network prohibited" }
			10   { append icmpv4_msg "destination host prohibited" }
			11   { append icmpv4_msg "network for TOS" }
			12   { append icmpv4_msg "host for TOS" }
			13   { append icmpv4_msg "communication prohibited" }
			14   { append icmpv4_msg "host precedence violation" }
			15   { append icmpv4_msg "precedence cutoff" }
			default { append icmpv4_msg "unknown" }
		    }
		    set error 1
		}
		4 {
		    append icmpv4_msg "source quench"
		    set error 1
		}
		5 {
		    append icmpv4_msg "redirect/"
		    switch -- $type {
			0 { append icmpv4_msg "network" }
			1 { append icmpv4_msg "host" }
			2 { append icmpv4_msg "tos & network" }
			3 { append icmpv4_msg "tos & host" }
			default { append icmpv4_msg "unknown" }
		    }
		    set error 1
		}
                8 {
		    append icmpv4_msg "ping request"
		    if { [string length $packet] >= 8 } {
			binary scan $packet {iss} junk id seq
			append icmpv4_msg [format " id %u, seq %u" [expr $id & 0x0FFFF] [expr $seq & 0x0FFFF]]
			set data 1
			set packet [string range $packet 8 end]
		    }
		}
		9 {
		    append icmpv4_msg "router advertisement"
		}
		10 {
		    append icmpv4_msg "router solicitation"
		}
		11 {
		    append icmpv4_msg "time exceeded/"
		    switch -- $type {
			0 { append icmpv4_msg "transit" }
			1 { append icmpv4_msg "reassembly" }
			default { append icmpv4_msg "unknown" }
		    }
		    set error 1
		}
		12 {
		    append icmpv4_msg "parameter problem/"
		    switch -- $type {
			0 { append icmpv4_msg "IP header bad" }
			1 { append icmpv4_msg "required option missing" }
			default { append icmpv4_msg "unknown" }
		    }
		    set error 1
		}
		13 {
		    append icmpv4_msg "timestamp request"
		}
		14 {
		    append icmpv4_msg "timestamp reply"
		}
		15 {
		    append icmpv4_msg "information request"
		}
		16 {
		    append icmpv4_msg "information reply"
		}
		17 {
		    append icmpv4_msg "address mask request"
		}
		18 {
		    append icmpv4_msg "address mask reply"
		}
		default {
		    append icmpv4_msg "unknown"
		}
	    }
	    if { $error && ([string length $packet] >= 36) } {
		# The ICMP message contains an IP header and hopefully the TCP or UDP ports as well
		# Only deal with the simple cases.
		binary scan $packet {iiccSiccSIISS} icmp_junk1 icmp_junk2 ip_lenver ip_junk1 ip_junk2 ip_junk3 ip_junk4 ip_protocol ip_junk5 \
			ip_source ip_dest ip_source_port ip_dest_port
		if { (5 == ($ip_lenver & 0x0F)) && ((6 == $ip_protocol) || (17 == $ip_protocol)) } {
		    if { 6 == $ip_protocol } {
			append icmpv4_msg ", tcp"
		    } else {
			append icmpv4_msg ", udp"
		    }
		    append icmpv4_msg " >[ethernet::inet_ipv4_ntoa $ip_dest]:$ip_dest_port <[ethernet::inet_ipv4_ntoa $ip_source]:$ip_source_port"
		}
	    }

	    append icmpv4_msg "\n"
	    synth::output $icmpv4_msg "eth_icmpv4"

	    # Only some of the requests contain additional data that should be displayed
	    if { !$data } {
		return
	    }
	    
	} elseif { 58 == $ip_protocol } {
	    # ipv6 ICMP
	    if { [string length $packet] < 4 } {
		return
	    }
	    binary scan $packet {ccS} code type checksum

	    set icmpv6_msg "$device $direction: ICMPv6 "
	    set error 0
	    set data  0
	    switch -- $code {
		1 {
		    append icmpv6_msg "unreachable/"
		    switch -- $type {
			0 { append icmpv6_msg "no route" }
			1 { append icmpv6_msg "prohibited" }
			2 { append icmpv6_msg "not a neighbour" }
			3 { append icmpv6_msg "any other reason" }
			4 { append icmpv6_msg "UDP port unreachable" }
			default { append icmpv6_msg "unknown" }
		    }
		    set error 1
		}
		2 {
		    append icmpv6_msg "packet too big"
		    set error 1
		}
		3 {
		    append icmpv6_msg "time exceeded/"
		    switch -- $type {
			0 { append icmpv6_msg "hop limit" }
			1 { append icmpv6_msg "fragment reassembly" }
			default { append icmpv6_msg "unknown" }
		    }
		    set error 1
		}
		4 {
		    append icmpv6_msg "parameter problem"
		    switch -- $type {
			0 { append icmpv6_msg "erroneous header" }
			1 { append icmpv6_msg "unrecognized next header" }
			2 { append icmpv6_msg "unrecognized option" }
			default { append icmpv6_msg "unknown" }
		    }
		    set error 1
		}
		128 {
		    append icmpv6_msg "ping request"
		    # FIXME: is this the same format as for icmpv4?
		}
		129 {
		    append icmpv6_msg "ping reply"
		    # FIXME: is this the same format as for icmpv4?
		}
		130 {
		    append icmpv6_msg "group membership query"
		}
		131 {
		    append icmpv6_msg "group membership report"
		}
		132 {
		    append icmpv6_msg "group membership reduction"
		}
		133 {
		    append icmpv6_msg "router solicitation"
		}
		134 {
		    append icmpv6_msg "router advertisement"
		}
		135 {
		    append icmpv6_msg "neighbour solicitation"
		}
		136 {
		    append icmpv6_msg "neighbour advertisement"
		}
		137 {
		    append icmpv6_msg "redirect"
		}
	    }

	    if { $error && ([string length $packet] >= 44) } {
		# The ICMP message contains an IPv6 header and hopefully the TCP or UDP ports as well
		binary scan $packet {isccH16H16SS} icmp_junk1 icmp_junk2 ip_protocol icmp_junk3 ip_source ip_dest ip_source_port ip_dest_port
		if { 6 == $ip_protocol } {
		    append icmpv6_msg ", tcp"
		} elseif { 17 == $ip_protocol } {
		    append icmpv6_msg ", udp"
		}
		append icmpv6_msg " >[ethernet::inet_ipv4_ntoa $ip_dest]:$ip_dest_port <[ethernet::inet_ipv6_ntoa $ip_source]:$ip_source_port"
	    }
	    append icmpv6_msg "\n"
	    synth::output $icmpv6_msg "eth_icmpv6"

	    if { !$data } {
		return
	    }
	    
	} elseif { 6 == $ip_protocol } {
	    # TCP
	    if { [string length $packet] < 20 } {
		return
	    }
	    binary scan $packet {SSIIccSSS} source_port dest_port seq ack hdrsize flags winsize checksum urg
	    set source_port [expr $source_port & 0x0FFFF]
	    set dest_port   [expr $dest_port & 0x0FFFF]
	    set hdrsize     [expr ($hdrsize >> 4) & 0x0F]
	    set winsize     [expr $winsize & 0x0FFFF]
	    set urg         [expr $urg & 0x0FFFF]

	    set tcp_msg "$device $direction tcp: "
	    append tcp_msg " >${dest_name}:${dest_port}"
	    if { [info exists ethernet::services($dest_port,udp)] } {
		append tcp_msg "($ethernet::services($dest_port,udp))"
	    }
	    append tcp_msg "<${source_name}:$source_port"
	    if { [info exists ethernet::services($source_port,udp)] } {
		append tcp_msg "($ethernet::services($source_port,udp))"
	    }

	    append tcp_msg ", "
	    if { $flags & 0x08 } {
		append tcp_msg "PSH "
	    }
	    if { $flags & 0x04 } {
		append tcp_msg "RST "
	    }
	    if { $flags & 0x02 } {
		append tcp_msg "SYN "
	    }
	    if { $flags & 0x01 } {
		append tcp_msg "FIN "
	    }
	    append tcp_msg [format "seq %u" $seq]
	    
	    if { 0 != ($flags & 0x010) } {
		append tcp_msg [format ", ACK %u" $ack]
	    }
	    append tcp_msg ", win $winsize"
	    if { 0 != ($flags & 0x020) } {
		append tcp_msg ", URG $urg"
	    }
	    append tcp_msg "\n"
	    synth::output $tcp_msg "eth_tcp"
	    
	    set packet [string range $packet [expr 4 * $hdrsize] end]
	} elseif { 17 == $ip_protocol } {
	    # UDP
	    if { [string length $packet] < 8 } {
		return
	    }
	    set udp_msg "$device $direction: udp "
	    binary scan $packet {SSSS} source_port dest_port len checksum
	    set source_port [expr $source_port & 0x0FFFF]
	    set dest_port   [expr $dest_port   & 0x0FFFF]
	    append udp_msg [format "%d bytes, " [expr $len & 0x0FFFF]]
	    append udp_msg " >${dest_name}:$dest_port"
	    if { [info exists ethernet::services($dest_port,udp)] } {
		append udp_msg "($ethernet::services($dest_port,udp))"
	    }
	    append udp_msg "<${source_name}:$source_port"
	    if { [info exists ethernet::services($source_port,udp)] } {
		append udp_msg "($ethernet::services($source_port,udp))"
	    }
	    append udp_msg "\n"
	    synth::output $udp_msg "eth_udp"
	    set packet [string range $packet 8 end]
	} else {
	    # Unknown protocol, so no way of knowing where the data starts.
	    return
	}

	# At this point we may have a payload. This should be
	# dumped in both hex and ascii. The code tries to preserve
	# alignment.
	if { [string length $packet] == 0 } {
	    return
	}
	set hexdata_msg "$device $direction: data [format_hex_data $packet]\n"
	set asciidata_msg "$device $direction: data "
	set len [string length $packet]
	if { $len > $ethernet::max_show } {
	    set len $ethernet::max_show
	}
	for { set i 0 } { $i < $len } { incr i } {
	    set char [string index $packet $i]
	    if { "\r" == $char } {
		append asciidata_msg "\\r"
	    } elseif { "\n" == $char } {
		append asciidata_msg "\\n"
	    } elseif { "\t" == $char } {
		append asciidata_msg "\\t"
	    } elseif { [string is print -strict $char] } {
		append asciidata_msg " $char"
	    } else {
		append asciidata_msg "??"
	    }
	    if { 3 == ($i % 4) } {
		append asciidata_msg " "
	    }
	}
	append asciidata_msg "\n"
	synth::output $hexdata_msg "eth_hexdata"
	synth::output $asciidata_msg "eth_asciidata"
	
	return
    }

    # A utility for handling the ethernet record button on the toolbar
    proc logging_button_toggle { } {
	if { $ethernet::logging_enabled } {
	    set ethernet::logging_enabled 0
	    .toolbar.ethernet_logging configure -relief flat
	} else {
	    set ethernet::logging_enabled 1
	    .toolbar.ethernet_logging configure -relief sunken
	}
    }
    
    # A dummy procedure for initialization. All of this could execute at
    # the toplevel, but there are lots of locals.
    proc filters_initialize { } {
	ethernet::read_services
	ethernet::read_protocols
	ethernet::read_hosts

	# Add a button on the toolbar for enabling/disabling logging.
	# Also add an entry to the help menu
	if { $synth::flag_gui } {
	    button .toolbar.ethernet_logging -image $ethernet::image_netrecord -borderwidth 2 -relief flat -command ethernet::logging_button_toggle
	    pack .toolbar.ethernet_logging -side left -padx 2
	    synth::register_balloon_help .toolbar.ethernet_logging "Record ethernet traffic"

	    if { [synth::tdf_has_option "ethernet" "logging"] } {
		set ethernet::logging_enabled [synth::tdf_get_option "ethernet" "logging"]
	    } else {
		# Default to logging ethernet traffic. This may not be the right thing to do
		# because users may see too much output by default, but it is easy enough
		# to disable.
		set ethernet::logging_enabled 1
	    }
	    if { $ethernet::logging_enabled } {
		.toolbar.ethernet_logging configure -relief sunken
	    }

	    set ethernet_help [file join $synth::device_src_dir "doc" "devs-eth-synth-ecosynth.html"]
	    if { ![file readable $ethernet_help] } {
		synth::report_warning "Failed to locate synthetic ethernet documentation $ethernet_help\n   \
			Help->Ethernet target menu option disabled.\n"
		set ethernet_help ""
	    }
	    if { "" == $ethernet_help } {
		.menubar.help add command -label "Ethernet" -state disabled
	    } else {
		.menubar.help add command -label "Ethernet" -command [list synth::handle_help "file://$ethernet_help"]
	    }
	}

	if { [synth::tdf_has_option "ethernet" "max_show"] } {
	    set ethernet::max_show [synth::tdf_get_option "ethernet" "max_show"]
	    if { ! [string is integer -strict $ethernet::max_show] } {
		synth::report_error "Ethernet device, invalid value in target definition file $synth::target_definition\n   \
			             Entry max_show should be a simple integer, not $ethernet::max_show\n"
		set ethernet::init_ok 0
	    }
	}

	# Filters. First, perform some validation.
	set known_filters [list "ether" "arp" "ipv4" "ipv6" "icmpv4" "icmpv6" "udp" "tcp" "hexdata" "asciidata"]
	set tdf_filters [synth::tdf_get_options "ethernet" "filter"]
	array set filter_options [list]

	foreach filter $tdf_filters {
	    if { 0 == [llength $filter] } {
		synth::report_error "Ethernet device, invalid value in target definition file $synth::target_definition\n   \
			             Option \"filter\" requires the name of a known filters.\n"
		set ethernet::init_ok 0
		continue
	    }
	    set name [lindex $filter 0]
	    if { [info exists filter_options($name)] } {
		synth::report_error "Ethernet device, invalid value in target definition file $synth::target_definition\n   \
			             \"filter $name\" should be defined only once.\n"
		set ethernet::init_ok 0
		continue
	    }
	    if { -1 == [lsearch -exact $known_filters $name] } {
		synth::report_error "Ethernet device, invalid value in target definition file $synth::target_definition\n   \
			             Unknown filter \"$name\".\n   \
				     Known filters are $known_filters\n"
		set ethernet::init_ok 0
		continue
	    }
	    set filter_options($name) [lrange $filter 1 end]
	}

	# We now know about all the filter entries in the target definition file.
	# Time to create the filters themselves, provided we are running in GUI mode.
	if { $synth::flag_gui }	{
	    foreach filter $known_filters {
		if { ! [info exists filter_options($filter)] } {
		    synth::filter_add "eth_$filter" -text "ethernet $filter"
		} else {
		    array set parsed_options [list]
		    set message ""
		    if { ![synth::filter_parse_options $filter_options($filter) parsed_options message] } {
			synth::report_error \
			    "Invalid entry in target definition file $synth::target_definition\n   \
			     Ethernet filter $filter\n   $message"
			set ethernet::init_ok 0
		    } else {
			set parsed_options("-text") "ethernet $filter"
			synth::filter_add_parsed "eth_$filter" parsed_options
		    }
		}
	    }
	}
    }
    ethernet::filters_initialize
}

return ethernet::instantiate
