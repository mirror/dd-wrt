#!/bin/sh
# Erzeugt Schluessel und selbstsigniertes Zertifikat fuer das Web-Interface.
# POSIX sh / busybox ash, benoetigt nur eccert (kein openssl).
#
# Aufruf: gencert.sh [-f] [-d verzeichnis] [-n name] [-i ip]
#   ohne -f  nur erzeugen, wenn key.pem/cert.pem fehlen, kein P-256-Schluessel sind
#            oder das Zertifikat in weniger als RENEW_DAYS Tagen ablaeuft
#   -f       immer neu erzeugen, auch einen neuen Schluessel
#   -d       Zielverzeichnis (Default: aktuelles Verzeichnis)
#   -n       Hostname fuer CN und SAN (Default: nvram router_name, sonst Kernel-Hostname)
#   -i       IP-Adresse fuer SAN (Default: nvram lan_ipaddr, falls vorhanden)
#
# Ausgabe: key.pem (0600), cert.pem (0644), cert.expires (Ablauf als Unix-Zeit)
# Exit 0 = alles in Ordnung (neu erzeugt oder noch gueltig), 1 = Fehler
# Gut geeignet fuer Aufruf beim Start des Webservers und taeglich per cron.

ECCERT=${ECCERT:-eccert}
DAYS=${DAYS:-397}                  # Apple erlaubt max. 825 Tage, Chrome-Grenzen gelten nur fuer oeffentliche CAs
RENEW_DAYS=${RENEW_DAYS:-30}
MIN_EPOCH=${MIN_EPOCH:-1767225600} # 2026-01-01 = ECCERT_MIN_EPOCH in eccert: davor gilt die Uhr als nicht gestellt

DIR=.
NAME=
IP=
FORCE=0
while getopts "fd:n:i:" o; do
	case $o in
	f) FORCE=1 ;;
	d) DIR=$OPTARG ;;
	n) NAME=$OPTARG ;;
	i) IP=$OPTARG ;;
	*) echo "usage: $0 [-f] [-d dir] [-n name] [-i ip]" >&2; exit 1 ;;
	esac
done

if command -v nvram >/dev/null 2>&1; then
	[ -n "$NAME" ] || NAME=$(nvram get router_name 2>/dev/null)
	[ -n "$IP" ] || IP=$(nvram get lan_ipaddr 2>/dev/null)
fi
[ -n "$NAME" ] || NAME=$(cat /proc/sys/kernel/hostname 2>/dev/null)
# nur Zeichen, die in Hostnamen erlaubt sind (eccert trennt -subj an "/" und -san an ",")
NAME=$(printf '%s' "$NAME" | tr -cd 'A-Za-z0-9.-')
[ -n "$NAME" ] || NAME=router
case $IP in 0.0.0.0) IP= ;; esac

KEY=$DIR/key.pem
CERT=$DIR/cert.pem
EXP=$DIR/cert.expires
now=$(date +%s)

# --- muss erneuert werden?
if [ "$FORCE" = 0 ] && [ -s "$KEY" ] && [ -s "$CERT" ] && "$ECCERT" pub -key "$KEY" >/dev/null 2>&1; then
	[ "$now" -lt "$MIN_EPOCH" ] && exit 0          # Uhr nicht gestellt: vorhandenes Zertifikat behalten
	exp=$(cat "$EXP" 2>/dev/null)
	case $exp in ''|*[!0-9]*) exp=0 ;; esac         # fehlt (z. B. altes openssl-Zertifikat): erneuern
	[ $((exp - now)) -ge $((RENEW_DAYS * 86400)) ] && exit 0
	newkey=$KEY                                     # Schluessel behalten, nur Zertifikat erneuern
else
	newkey=
fi

# --- erzeugen (erst in temporaere Dateien, dann umbenennen)
mkdir -p "$DIR" || exit 1
umask 077
tmp=$DIR/.gencert.$$
trap 'rm -f "$tmp.key" "$tmp.crt" "$tmp.exp"' EXIT INT TERM

if [ -z "$newkey" ]; then
	"$ECCERT" genkey -out "$tmp.key" || exit 1
	newkey=$tmp.key
fi

SAN="DNS:$NAME"
[ -n "$IP" ] && SAN="$SAN,IP:$IP"
"$ECCERT" cert -key "$newkey" -subj "/CN=$NAME" -san "$SAN" -days "$DAYS" -out "$tmp.crt" || exit 1

# Ablaufzeitpunkt wie eccert ihn setzt: notBefore = max(jetzt, MIN_EPOCH) - 5 min
start=$now
[ "$start" -lt "$MIN_EPOCH" ] && start=$MIN_EPOCH
echo $((start - 300 + DAYS * 86400)) > "$tmp.exp" || exit 1

if [ "$newkey" != "$KEY" ]; then mv -f "$tmp.key" "$KEY" || exit 1; fi
chmod 644 "$tmp.crt" "$tmp.exp"
mv -f "$tmp.crt" "$CERT" && mv -f "$tmp.exp" "$EXP" || exit 1
echo "gencert: neues Zertifikat CN=$NAME SAN=$SAN, $DAYS Tage" >&2
exit 0
