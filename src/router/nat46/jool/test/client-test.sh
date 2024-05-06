#!/bin/bash

if [[ $UID != 0 ]]; then
	echo "Please start the script as root or sudo."
	exit 1
fi

modprobe -r jool
modprobe -r jool_siit

JSON=/tmp/jool-test.conf

function start() {
		clear
		echo -e "\\x1b[36m$1\\x1b[0m"
}

function pause() {
	read -p "Press Enter to continue"
}

# --------------
# -- instance --
# --------------

start "Errors: Modules not modprobed"
( set -x; jool instance display; jool_siit instance display )
pause

function single_module_instance_test() {
	THIS=$1
	OTHER=$2

	start "Error: $THIS modprobed, $OTHER not modprobed"
	( set -x; modprobe $THIS; $OTHER instance display )
	pause

	start "Display empty table"
	( set -x; $THIS instance display; )
	pause

	start "'Running', followed by single entry table"
	( set -x
		$THIS instance add --netfilter -6 64::/96 xlat-1
		$THIS -i xlat-1 instance status
		$THIS instance display
	)
	pause
	
	start "Error: Too many Netfilter instances"
	( set -x; $THIS instance add --netfilter -6 128::/96 dummy )
	pause

	start "Error: Duplicate instance name"
	( set -x; $THIS instance add --iptables -6 128::/96 xlat-1 )
	pause

	start "2-entry table (CSV format)"
	( set -x
		$THIS instance add --iptables -6 32::/96 xlat-2
		$THIS instance display --csv
	)
	pause

	start "3-entry table (No headers)"
	( set -x
		$THIS instance add --iptables -6 16::/96 xlat-3
		$THIS instance display --no-headers
	)
	pause

	start "table with 1 and 3 (CSV no headers)"
	( set -x
		$THIS instance remove xlat-2
		$THIS instance display --csv --no-headers
	)
	pause

	start "Empty table"
	( set -x; $THIS instance flush; $THIS instance display )
	pause

	modprobe -r $THIS
}

single_module_instance_test jool jool_siit
single_module_instance_test jool_siit jool

# --------------

function add_dummy_entries() {
	jool instance add --netfilter -6 64::/96 xlat-1
	jool instance add --iptables -6 64::/96 xlat-2
	jool instance add --iptables -6 64::/96 xlat-3
	jool_siit instance add --netfilter -6 64::/96 xlat-1
	jool_siit instance add --iptables -6 64::/96 xlat-2
	jool_siit instance add --iptables -6 64::/96 xlat-3
}

start  "Test instance database management: Remove entries before r-modprobing"
( set -x
	modprobe jool
	modprobe jool_siit
	add_dummy_entries
	jool instance flush
	jool_siit instance flush
	modprobe -r jool
	modprobe -r jool_siit
)
pause

start "This time, r-modprobe with populated database"
( set -x
	modprobe jool
	modprobe jool_siit
	add_dummy_entries
	modprobe -r jool
	modprobe -r jool_siit
)
pause

# -------------
# --- Stats ---
# -------------

modprobe jool
jool instance add --iptables -6 64::/96
jool pool4 add --tcp 192.0.2.99 200-300
jool bib add --tcp 192.0.2.99#250 2001:db8::15#250

start "Stats: Normal"
( set -x; jool stats display )
pause

start "Stats: all"
( set -x; jool stats display --all )
pause

start "Stats: CSV"
( set -x; jool stats display --csv )
pause

start "Stats: No headers"
( set -x; jool stats display --csv --no-headers )
pause

start "Stats: Explain"
( set -x; jool stats display --explain )
pause

# --------------
# --- Global ---
# --------------

modprobe jool
jool instance add --iptables -6 64::/96

start "Globals normal display"
( set -x; jool global display )
pause

start "Tweak Boolean, CSV"
( set -x; jool global update manually-enabled false; jool global display --csv )
pause

start "Tweak Integer, no headers"
( set -x; jool global update tos 32; jool global display --csv --no-headers )
pause

start "Other types changed"
( set -x
	jool global update mtu-plateaus 1,2,3
	jool global update udp-timeout 1:00:00
	jool global update f-args 2
	jool global display
)
pause

start "Error: pool6 edit attempt"
( set -x; jool global update pool6 32::/96 )
pause

modprobe -r jool
modprobe jool_siit
jool_siit instance add --iptables

start "Force"
( set -x
	jool_siit global update pool6 64:0:0:0:ff00::/96
	jool_siit global update pool6 64:0:0:0:ff00::/96 --force
	jool_siit global update rfc6791v4-prefix 0.0.0.0/8
	jool_siit global update rfc6791v4-prefix 0.0.0.0/8 --force
	jool_siit global display
)
pause

modprobe -r jool_siit

# -------------
# --- pool4 ---
# -------------

modprobe jool
jool instance add --iptables -6 64::/96

function display_pool4_outputs() {
	jool pool4 display
	jool pool4 display --no-headers
	jool pool4 display --csv
	jool pool4 display --csv --no-headers
}

function display_pool4() {
	jool pool4 display --tcp  --no-headers
	jool pool4 display --udp  --no-headers
	jool pool4 display --icmp --no-headers
}

start "Empty TCP pool4 - All output types"
( set -x; display_pool4_outputs )
pause

start "Empty pool4 - All tables"
( set -x; display_pool4 )
pause

# -----------------------

start "Add entry"
( set -x
	jool pool4 add --tcp 192.0.2.1 300-400
	display_pool4_outputs
)
pause

start "Add same entry; no changes (also test inverted port range)"
( set -x
	jool pool4 add --tcp 192.0.2.1 400-300
	display_pool4_outputs
)
pause

start "Engulf the old ports (was 300-400)"
( set -x
	jool pool4 add --tcp 192.0.2.1 200-500
	display_pool4
)

start "Add address group which contains all of the above (was .1 200-500)"
( set -x
	jool pool4 add --tcp 192.0.2.0/30 100-600
	display_pool4
)
pause

start "Remove first address"
( set -x
	jool pool4 remove --tcp 192.0.2.0 100-600
	display_pool4
)

start "Remove last address"
( set -x
	jool pool4 remove --tcp 192.0.2.3 100-600
	display_pool4
)

start "Flush"
( set -x
	jool pool4 flush
	display_pool4
)

# -------------------------

start "Add entry"
( set -x
	jool pool4 add --tcp 192.0.2.1 200-300
	display_pool4
)
pause

start "Add adjacent ports left (was 200-300)"
( set -x
	jool pool4 add --tcp 192.0.2.1 100-199
	display_pool4
)
pause

start "Add adjacent ports right (was 100-300)"
( set -x
	jool pool4 add --tcp 192.0.2.1 301-400
	display_pool4
)
pause

start "Remove exactly everything"
( set -x
	jool pool4 remove --tcp 192.0.2.1 100-400
	display_pool4
)
pause

# ----------------------

start "Add address, punch a hole"
( set -x
	jool pool4 add --tcp 192.0.2.1 200-500
	display_pool4
	jool pool4 remove --tcp 192.0.2.1 301-399
	display_pool4
)
pause

start "Add separate entries"
( set -x
	jool pool4 add --tcp 192.0.2.1 100-198
	jool pool4 add --tcp 192.0.2.1 302-398
	jool pool4 add --tcp 192.0.2.1 502-600
	display_pool4
)

start "Fill in the holes (also test port non-ranges)"
( set -x
	jool pool4 add --tcp 192.0.2.1 199
	jool pool4 add --tcp 192.0.2.1 301
	jool pool4 add --tcp 192.0.2.1 399
	jool pool4 add --tcp 192.0.2.1 501
	display_pool4
)

start "Remove everything and more"
( set -x
	jool pool4 remove --tcp 0.0.0.0/0 0-65535
	display_pool4
)
pause

# ---------------------------

start "Modify other columns"
( set -x
	jool pool4 add --tcp  --max-iterations 5        0.0.0.1 100-300 --mark 1
	jool pool4 add --udp  --max-iterations auto     0.0.0.3 500-600
	jool pool4 add --icmp --max-iterations infinity 0.0.0.3 500-600
	display_pool4
)
pause

start "--quick (no visible special effects)"
( set -x
	jool pool4 remove --quick --tcp 0.0.0.1 100-300 --mark 1
	display_pool4
)
pause

# ---------------------------

start "Error: Too many addresses"
( set -x; jool pool4 add --tcp 192.0.2.0/23 100-200; display_pool4 )
pause

start "Force lots of addresses"
( set -x; jool pool4 add --tcp --force 192.0.2.0/23 100-200; display_pool4 )
pause

start "Flush again"
( set -x; jool pool4 flush; display_pool4 )
pause

# ----------------------------

start "Errors: Malformed stuff during add"
( set -x
	jool pool4 add --tcp 192.0.2.1/-1 100-200
	jool pool4 add --tcp 192.0.2.1/33 100-200
	jool pool4 add --tcp 192.0.2.1/24 100-200
	jool pool4 add --tcp 192.a 100-200
	jool pool4 add --tcp 192.0.2.1 a100-200
	jool pool4 add --tcp 192.0.2.1 -1
	jool pool4 add --tcp 192.0.2.1 65536
	jool pool4 add --tcp 192.0.2.1 100 --max-iterations 0
	display_pool4
)
pause

start "Errors: Malformed stuff during remove"
( set -x
	jool pool4 remove 0.0.0.0/-1
	jool pool4 remove 192.0.2.1/33
	jool pool4 remove 192.0.2.1/24
	jool pool4 remove 192.a
	jool pool4 remove 192.0.2.1 a100-200
	jool pool4 remove 192.0.2.1 -1
	jool pool4 remove 192.0.2.1 65536
	display_pool4
)
pause

modprobe -r jool

# -------------
# ---- BIB ----
# -------------

function display_bib_outputs() {
	jool bib display --numeric
	jool bib display --numeric --no-headers
	jool bib display --numeric --csv
	jool bib display --numeric --csv --no-headers
}

function display_bib() {
	jool bib display --numeric --tcp
	jool bib display --numeric --udp
	jool bib display --numeric --icmp
}

modprobe jool
jool instance add --iptables -6 64::/96

start "Show empty table (output variations)"
( set -x; display_bib_outputs )
pause

start "Show empty tables (multiple protocols)"
( set -x; display_bib )
pause

start "Add failure: addr4 not in pool4"
( set -x; jool bib add 2001:db8::1#1234 192.0.2.1#1234 )
pause

start "Add success"
( set -x
	jool pool4 add 192.0.2.1 1000-2000 --tcp
	jool pool4 add 192.0.2.1 1000-2000 --udp
	jool pool4 add 192.0.2.1 1000-2000 --icmp
	jool bib add 2001:db8::1#1234 192.0.2.1#1234
	jool bib add 2001:db8::1#1235 192.0.2.1#1235 --tcp
	jool bib add 2001:db8::1#1234 192.0.2.1#1234 --udp
	jool bib add 2001:db8::1#1234 192.0.2.1#1234 --icmp
	display_bib
)
pause

start "Display populated table (output variations)"
( set -x; display_bib_outputs )
pause

start "Entry already exists (error, error, success)"
( set -x
	jool bib add 2001:db8::1#1234 192.0.2.1#1236
	jool bib add 2001:db8::1#1236 192.0.2.1#1234
	jool bib add 2001:db8::1#1234 192.0.2.1#1234
)
pause

start "Remove error: Entry does not exist"
( set -x
	jool bib remove 2001:db8::2#1234 192.0.2.2#1234
	jool bib remove 2001:db8::2#1235
	jool bib remove 192.0.2.2#1234
)
pause

start "Remove success"
( set -x
	jool bib remove 2001:db8::1#1234 192.0.2.1#1234
	jool bib remove 2001:db8::1#1235 192.0.2.1#1235 --tcp
	jool bib remove 2001:db8::1#1234 --udp
	jool bib remove 192.0.2.1#1234 --icmp
	display_bib
)
pause

start "Errors: Malformed input during add"
( set -x
	jool bib add --tcp a2001:db8::2#1234 192.0.2.2#1234
	jool bib add --tcp 2001:db8::2#a1234 192.0.2.2#1234
	jool bib add --tcp 2001:db8::2#1234 a192.0.2.2#1234
	jool bib add --tcp 2001:db8::2#1234 192.0.2.2#a1234
	jool bib add --tcp 2001:db8:#1234 192.0.2.2#1234
	jool bib add --tcp 2001:db8::2#1234 192.0.2#1234
	jool bib add --tcp 2001:db8::2#-1 192.0.2.2#1234
	jool bib add --tcp 2001:db8::2#1234 192.0.2.2#-1
	jool bib add --tcp 2001:db8::2#65536 192.0.2.2#1234
	jool bib add --tcp 2001:db8::2#1234 192.0.2.2#65536
	jool bib add --tcp 2001:db8::2#1234
	jool bib add --tcp 192.0.2.2#1234
	jool bib add
	jool bib add --tcp 2001:db8::2#1234 192.0.2.2#1234 potato
)
pause

start "Errors: Malformed input during remove"
( set -x
	jool bib remove --tcp a2001:db8::2#1234 192.0.2.2#1234
	jool bib remove --tcp 2001:db8::2#a1234 192.0.2.2#1234
	jool bib remove --tcp 2001:db8::2#1234 a192.0.2.2#1234
	jool bib remove --tcp 2001:db8::2#1234 192.0.2.2#a1234
	jool bib remove --tcp 2001:db8:#1234 192.0.2.2#1234
	jool bib remove --tcp 2001:db8::2#1234 192.0.2#1234
	jool bib remove --tcp 2001:db8::2#-1 192.0.2.2#1234
	jool bib remove --tcp 2001:db8::2#1234 192.0.2.2#-1
	jool bib remove --tcp 2001:db8::2#65536 192.0.2.2#1234
	jool bib remove --tcp 2001:db8::2#1234 192.0.2.2#65536
	jool bib remove
	jool bib remove --tcp 2001:db8::2#1234 192.0.2.2#1234 potato
)
pause

modprobe -r jool

# --------------
# ---- File ----
# --------------

function create_valid_file() {
	echo "{
		\"framework\": \"$1\",
		\"instance\": \"$2\",
		\"global\": { \"pool6\": \"$3::/96\" }
	}" > $JSON
}

function add_many_instances() {
	jool instance add --iptables --pool6 64::/96 client-1
	create_valid_file iptables file-1 64
	jool file handle $JSON
	jool instance add --iptables --pool6 64::/96 client-2
	create_valid_file iptables file-2 64
	jool file handle $JSON
	jool instance add --iptables --pool6 64::/96 client-3
	create_valid_file iptables file-3 64
	jool file handle $JSON
	jool instance add --iptables --pool6 64::/96 client-4
	jool instance display
}

modprobe jool

start "Empty file"
( set -x
	echo '{}' > $JSON
	jool file handle $JSON
)
pause

start "Framework included"
( set -x
	echo '{ "framework": "iptables" }' > $JSON
	jool file handle $JSON
)
pause

start "instance included"
( set -x
	echo '{ "framework": "iptables", "instance": "file" }' > $JSON
	jool file handle $JSON
)
pause

start "pool6 included"
( set -x
	create_valid_file iptables file 64
	jool file handle $JSON
)
pause

start "Illegal changes"
( set -x
	create_valid_file netfilter file 64
	jool file handle $JSON
	create_valid_file iptables file 32
	jool file handle $JSON
)
pause

start "Legal changes"
( set -x
	echo "{
		\"framework\": \"iptables\",
		\"instance\": \"file\",
		\"global\": { \"pool6\": \"64::/96\", \"tos\": 128 }
	}" > $JSON
	jool file handle $JSON
	jool -i file global display
)
pause

start "Modify file instance via client"
( set -x
	jool -i file pool4 add 192.0.2.1 100-200 --tcp
	jool -i file pool4 display
)
pause

start "Modify client instance via file"
( set -x
	jool instance add --iptables -6 64::/96 client
	echo '{
		"framework": "iptables",
		"instance": "client",
		"global": {
			"pool6": "64::/96",
			"tos": 123
		}
	}' > $JSON
	jool file handle $JSON
	jool -i client global display
)
pause

start "Remove file instance via client"
( set -x; jool instance remove file; jool instance display )
pause

start "Remove client instance via file"
( set -x; jool -f $JSON instance remove; jool instance display )
pause

start "Add many instances, flush"
( set -x
	add_many_instances
	jool instance flush
	jool instance display
)
pause

start "Add many instances, modprobe -r"
( set -x
	add_many_instances
	modprobe -r jool
)
pause

# ---------------------------------
# --- More Instance Manhandling ---
# ---------------------------------

modprobe jool
modprobe jool_siit
ip netns add jool1
ip netns add jool2

function add_namespace_instances() {
	jool      instance add --iptables -6 64::/96 $1
	jool_siit instance add --iptables -6 64::/96 $1
	ip netns exec jool1 jool      instance add --iptables -6 64::/96 $1
	ip netns exec jool1 jool_siit instance add --iptables -6 64::/96 $1
	ip netns exec jool2 jool      instance add --iptables -6 64::/96 $1
	ip netns exec jool2 jool_siit instance add --iptables -6 64::/96 $1
}

function remove_namespace_instances() {
	jool      instance remove $1
	jool_siit instance remove $1
	ip netns exec jool1 jool      instance remove $1
	ip netns exec jool1 jool_siit instance remove $1
	ip netns exec jool2 jool      instance remove $1
	ip netns exec jool2 jool_siit instance remove $1
}

start "Check instance uniqueness - Same name, different types and namespaces (success)"
( set -x; add_namespace_instances "name1" )
pause

start "Check instance uniqueness - Collide everything (failure)"
( set -x; add_namespace_instances "name1" )
pause

start "Check instance uniqueness - Different names (success)"
( set -x; add_namespace_instances "name2" )
pause

# Try to make sure xlator_replace() is not leaving stray pointers around
start "Replace some instances, then delete them one by one"
( set -x
	echo "{
		\"framework\": \"iptables\",
		\"instance\": \"name1\",
		\"global\": { \"pool6\": \"64::/96\", \"tos\": 128 }
	}" > $JSON
	jool      file handle $JSON
	jool_siit file handle $JSON
	ip netns exec jool1 jool      file handle $JSON
	ip netns exec jool1 jool_siit file handle $JSON
	ip netns exec jool2 jool      file handle $JSON
	ip netns exec jool2 jool_siit file handle $JSON

	remove_namespace_instances "name1"
	jool      instance display
	jool_siit instance display
	remove_namespace_instances "name2"
	jool      instance display
	jool_siit instance display
)
pause

ip netns del jool1
ip netns del jool2
modprobe -r jool_siit
modprobe -r jool

# --------------
# --- Footer ---
# --------------

clear
echo "Done."
echo "Missing tests:"
echo "- sudoless request"
echo "- request from differently-versioned client"
