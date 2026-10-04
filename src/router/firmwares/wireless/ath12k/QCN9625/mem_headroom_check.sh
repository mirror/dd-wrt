#!/bin/sh
# mem_headroom_check.sh - Memory headroom check for QCN9625/QCN9589 firmware arenas.
# Detects board topology from --board argument or /tmp/board.json.
#
# Supported boards (RDP -> topology):
#   RDP489          -> single_mac      (3x QCN9625, 1 mac each)
#   RDP502/503/504  -> dual_mac        (2x QCN9625, PCI0=mac0+mac1, PCI1=mac0; only PCI0/mac0 checked)
#   RDP491          -> rimini          (2x QCN9589, PCI1=mac0+mac1, PCI3=mac0; only PCI1/mac0 checked (split-phy))
#
# Usage:
#   sh mem_headroom_check.sh
#   sh mem_headroom_check.sh --board RDP491
#   sh mem_headroom_check.sh --headroom-dir /custom/dir
#   sh mem_headroom_check.sh --headroom-file /custom/mem_headroom.txt

HEADROOM_FILE=""
HEADROOM_DIR=""
BOARD=""

while [ $# -gt 0 ]; do
    case "$1" in
        --headroom-dir)  HEADROOM_DIR="$2";  shift 2 ;;
        --headroom-file) HEADROOM_FILE="$2"; shift 2 ;;
        --board)         BOARD="$2";         shift 2 ;;
        *) echo "Unknown arg: $1"; exit 1 ;;
    esac
done

# Resolve headroom file:
#   1. --headroom-file explicitly given -> use it directly
#   2. --headroom-dir given             -> look for mem_headroom.txt inside it
#   3. Neither given                    -> search known firmware dirs, then fall
#      back to find(1) across /lib/firmware and /tmp
find_headroom_file() {
    local candidate
    # Known firmware locations, ordered by likelihood
    for candidate in \
        /lib/firmware/qcn9625/mem_headroom.txt \
        /lib/firmware/qcn9589/mem_headroom.txt \
        /lib/firmware/IPQ5210/WIFI_FW/qcn9625/mem_headroom.txt \
        /lib/firmware/IPQ5210/WIFI_FW/qcn9589/mem_headroom.txt \
        /tmp/mem_headroom.txt
    do
        [ -f "$candidate" ] && echo "$candidate" && return 0
    done
    # Last resort: find anywhere under /lib/firmware or /tmp
    candidate=$(find /lib/firmware /tmp -maxdepth 4 -name "mem_headroom.txt" 2>/dev/null | head -1)
    if [ -n "$candidate" ]; then
        echo "$candidate"
        return 0
    fi
    return 1
}

if [ -n "$HEADROOM_FILE" ]; then
    # Explicit file given — validate it exists
    if [ ! -f "$HEADROOM_FILE" ]; then
        echo "[ERROR] Headroom file not found: $HEADROOM_FILE"
        exit 1
    fi
elif [ -n "$HEADROOM_DIR" ]; then
    HEADROOM_FILE="${HEADROOM_DIR}/mem_headroom.txt"
    if [ ! -f "$HEADROOM_FILE" ]; then
        echo "[ERROR] Headroom file not found in dir: $HEADROOM_FILE"
        exit 1
    fi
else
    HEADROOM_FILE=$(find_headroom_file)
    if [ -z "$HEADROOM_FILE" ]; then
        echo "[ERROR] mem_headroom.txt not found in any known location."
        echo "        Copy it to /tmp/mem_headroom.txt or use --headroom-file."
        exit 1
    fi
    echo "[info]  Auto-detected headroom file: $HEADROOM_FILE"
fi

# Resolve RDP from --board arg or /tmp/board.json
if [ -n "$BOARD" ]; then
    RDP=$(echo "$BOARD" | tr 'a-z' 'A-Z' | grep -o 'RDP[0-9][0-9]*')
    if [ -z "$RDP" ]; then
        echo "[ERROR] Could not parse RDP number from --board '$BOARD'"
        exit 1
    fi
elif [ -f "/tmp/board.json" ]; then
    RDP=$(grep -o 'rdp[0-9][0-9]*' /tmp/board.json | head -1 | tr 'a-z' 'A-Z')
    if [ -z "$RDP" ]; then
        echo "[ERROR] Could not parse RDP from /tmp/board.json"
        exit 1
    fi
else
    echo "[ERROR] No --board given and /tmp/board.json not found"
    exit 1
fi

# Map RDP -> topology
case "$RDP" in
    RDP489)               TOPO="single_mac" ;;
    RDP502|RDP503|RDP504) TOPO="dual_mac"   ;;
    RDP491)               TOPO="rimini"     ;;
    *)
        echo "[ERROR] Unrecognised RDP: $RDP (supported: RDP489, RDP491, RDP502, RDP503, RDP504)"
        exit 1
        ;;
esac

echo "[check] Board=$RDP  topology=$TOPO  headroom_file=$HEADROOM_FILE"

# Discover PCIs
PCIS=$(find /sys/kernel/debug/ath12k -maxdepth 1 -type d -name "pci-*" 2>/dev/null | sort)
if [ -z "$PCIS" ]; then
    echo "[ERROR] No ath12k PCI entries found. Is firmware up?"
    exit 1
fi

# Helper: extract thresholds for a given section into a temp file.
# Prints the temp file path to stdout; exits the whole script on error.
make_thresh_file() {
    local sec="$1"
    local tfile
    tfile=$(mktemp)
    awk -v sec="$sec" '
        /^\[/{in_sec=0}
        $0 ~ "^\\[" sec "\\]" {in_sec=1; next}
        in_sec && /^[^#]/ && /[A-Z]/ {print}
    ' "$HEADROOM_FILE" > "$tfile"
    if [ ! -s "$tfile" ]; then
        rm -f "$tfile"
        echo "[ERROR] Section [$sec] not found or empty in $HEADROOM_FILE" >&2
        exit 1
    fi
    echo "$tfile"
}

TOTAL_BREACHED=0

check_mac() {
    local mac_path="$1"
    local label="$2"
    local thresh_file="$3"

    [ ! -f "$mac_path" ] && echo "  [SKIP] $label: wmi_ctrl_stats not found" && return

    echo "3 1" > "$mac_path"
    local STATS
    STATS=$(cat "$mac_path" | awk '/On-Demand Stats:/{found=1} found{print}')

    printf "\n  %-14s %12s %12s %12s %12s %10s\n" "Arena" "Total" "Allocated" "Headroom" "Threshold" "Status"
    echo "  --------------------------------------------------------------------------------"

    local breached=0
    while IFS=': ' read -r arena threshold; do
        [ -z "$arena" ] && continue

        case "$threshold" in
            ''|*[!0-9]*) printf "  %-14s %12s %12s %12s %12s %10s\n" "$arena" "N/A" "N/A" "N/A" "$threshold" "ERROR"; continue ;;
        esac

        local total allocated headroom
        total=""
        allocated=""

        total=$(echo "$STATS"     | awk "/arena = ${arena}$/{found=1} found && /total_bytes/{match(\$0,/[0-9]+/);     print substr(\$0,RSTART,RLENGTH); found=0; exit}")
        allocated=$(echo "$STATS" | awk "/arena = ${arena}$/{found=1} found && /allocated_bytes/{match(\$0,/[0-9]+/); print substr(\$0,RSTART,RLENGTH); found=0; exit}")

        if [ -z "$total" ] || [ -z "$allocated" ]; then
            printf "  %-14s %12s %12s %12s %12s %10s\n" "$arena" "N/A" "N/A" "N/A" "$threshold" "MISSING"
            continue
        fi

        headroom=$(( total - allocated ))

        if [ "$headroom" -ge "$threshold" ]; then
            printf "  %-14s %12d %12d %12d %12d %10s\n" "$arena" "$total" "$allocated" "$headroom" "$threshold" "OK"
        else
            local deficit=$(( threshold - headroom ))
            printf "  %-14s %12d %12d %12d %12d %10s  <-- BREACH (deficit=%d)\n" \
                "$arena" "$total" "$allocated" "$headroom" "$threshold" "BREACH" "$deficit"
            breached=$(( breached + 1 ))
        fi
    done < "$thresh_file"

    if [ "$breached" -gt 0 ]; then
        echo "  RESULT [$label]: $breached arena(s) BREACHED"
        TOTAL_BREACHED=$(( TOTAL_BREACHED + breached ))
    else
        echo "  RESULT [$label]: All arenas OK"
    fi
}

# Pre-build threshold files once (not per PCI iteration)
case "$TOPO" in
    single_mac)
        TF_SINGLE=$(make_thresh_file "single_mac")
        ;;
    dual_mac)
        TF_DUAL=$(make_thresh_file "dual_mac")
        ;;
    rimini)
        TF_RIMINI=$(make_thresh_file "rimini")
        ;;
esac

# Iterate PCIs and check MACs
pci_num=0
for pci_dir in $PCIS; do
    pci_name=$(basename "$pci_dir")

    case "$TOPO" in
        single_mac)
            # RDP489: every PCI has only mac0, same threshold section for all
            echo "=== Checking PCI${pci_num} (${pci_name}) / mac0 ==="
            check_mac "${pci_dir}/mac0/wmi_ctrl_stats" "PCI${pci_num}/${pci_name}/mac0" "$TF_SINGLE"
            ;;
        dual_mac)
            # RDP502/503/504: only check PCI0/mac0
            if [ -d "${pci_dir}/mac1" ]; then
                echo "=== Checking PCI${pci_num} (${pci_name}) / mac0 ==="
                check_mac "${pci_dir}/mac0/wmi_ctrl_stats" "PCI${pci_num}/${pci_name}/mac0" "$TF_DUAL"
            else
                echo "  [SKIP] PCI${pci_num} (${pci_name}): single-phy PCI, not checked for dual_mac topology"
            fi
            ;;
        rimini)
            # RDP491: only check PCI1/mac0 (split-phy QCN9589, mac0=5G)
            if [ -d "${pci_dir}/mac1" ]; then
                echo "=== Checking PCI${pci_num} (${pci_name}) / mac0 ==="
                check_mac "${pci_dir}/mac0/wmi_ctrl_stats" "PCI${pci_num}/${pci_name}/mac0" "$TF_RIMINI"
            else
                echo "  [SKIP] PCI${pci_num} (${pci_name}): single-phy PCI, not checked for rimini topology"
            fi
            ;;
    esac

    pci_num=$(( pci_num + 1 ))
done

# Cleanup temp files
rm -f "$TF_SINGLE" "$TF_DUAL" "$TF_RIMINI"

echo ""
if [ "$TOTAL_BREACHED" -gt 0 ]; then
    echo "OVERALL RESULT: $TOTAL_BREACHED arena(s) breached across all MACs."
    exit 1
else
    echo "OVERALL RESULT: All MACs within headroom threshold. OK."
    exit 0
fi
