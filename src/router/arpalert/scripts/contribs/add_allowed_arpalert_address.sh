#!/bin/bash
# DATE: 12/15/05
# AUTHOR: Robert Perriero (robert.perriero@gmail.com)
# FILE: add_allowed_arpalert_address.sh

## Variables ##
ARPALERTHOMEDIR=/usr/local/arpalert
LEASES=$ARPALERTHOMEDIR/var/lib/arpalert/arpalert.leases
ALLOWFILE=$ARPALERTHOMEDIR/etc/arpalert/maclist.allow
COMMAND=`cat $LEASES | grep -w "$1"`
COMMAND2=`cat $ALLOWFILE | grep -w "$1"`

echo -e "\nArpalert Lease Manager v0.2\n"echo -e "   IP: $1 \n DNS: `nslookup $1 | grep "in-addr" | awk '{print $4}'`\n\n"
echo -e "Searching Lease file...\n"

if [ "$COMMAND" == "" ]
then
        echo -e "   Record not found.\n"
        SWITCH=false
else
        echo "   Record found in Lease file:"
        echo "   $COMMAND"
fi

echo -e "\n\nSearching Allow File...\n"

if [ "$COMMAND2" == "" ]
then
        echo "   Record not found."
else
        echo "   Record found in allow file:"
        echo "   $COMMAND2"

        if [ $SWITCH ]
        then
                echo -e "\n\n**Attention**\nRecord for $1 exists in allow file.\n"
                exit 0
        fi

        echo ""
        echo "**Warning**"
        echo "Record for $1 exists in both Lease and Allow files."
        echo "To add new lease record, delete existing record from the"
        echo "allow file located at: "
        echo ""
        echo "$ALLOWFILE"
        echo ""
        exit 1
fi

while true; do
echo ""
echo -n "Would you like to add this to the arpalert allow list? (Y/N): "
read userinput

case $userinput in
        y* | Y* ) echo "$COMMAND" >> $ALLOWFILE; echo OK.; break ;;
        [nN]* ) echo Record not updated.; break ;;
        * ) echo "Unknown response. Try again";;
esac
done
