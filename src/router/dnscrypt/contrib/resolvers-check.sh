#! /bin/sh

RESOLVERS_LIST=dnscrypt-resolvers.csv
ONLINE_RESOLVERS_LIST=dnscrypt-online-resolvers.csv
DNSCRYPT_PROXY=dnscrypt-proxy
MARGIN=10
CSV_FILE="${RESOLVERS_LIST}"

tmpfile=$(mktemp .${ONLINE_RESOLVERS_LIST}.XXXXXXXXXXXX) || exit 1
trap "rm -f ${tmpfile}" EXIT

if which csvlint > /dev/null; then
  csvlint "$RESOLVERS_LIST" || echo "*** Invalid CSV file ***" >&2
fi

if [ $(cut -d, -f1 $RESOLVERS_LIST | sort | uniq -d | wc -l) -gt 0 ]; then
  echo "Duplicate resolver name" >&2
  exit 1
fi

if [ $(cut -d, -f2 $RESOLVERS_LIST | sort | uniq -d | wc -l) -gt 0 ]; then
  echo "Duplicate resolver long name" >&2
  exit 1
fi

exec < "$RESOLVERS_LIST"
exec > "$tmpfile"

read header
echo "$header" | egrep -q '^Name,' || echo "*** Invalid CSV file ***" >&2

echo "$header"

res=0
while read line; do
  if [ "x${IPV4_ONLY}" != "x" ]; then
    n=$(echo "$line" | egrep -c ',\[[0-9a-fA-F:]+\](:[0-9]+)?,')
    if [ $n -ne 0 ]; then
      continue
    fi
  fi
  resolver_name=$(echo "$line" | cut -d, -f1)
  eval "${DNSCRYPT_PROXY} -L ${CSV_FILE} -R ${resolver_name} -t ${MARGIN} -m 1"
  if [ $? -eq 0 ]; then
    echo "$line"
    echo "+ ${resolver_name} - OK" >&2
  else
    echo "- ${resolver_name} - Failed" >&2
    res=1
  fi
done

mv -f "$tmpfile" "$ONLINE_RESOLVERS_LIST"

exit $res
