#!/bin/sh

#
# Arguments sent by ArpAlert are :
# 1 : MAC Address
# 2 : IP Address
# 3 : supp (used with unathrq alert)
# 4 : Type of alert (cf arpalert.conf)
#

# Intruder MAC address
intruder_MAC=$1

# Intruder IP address
intruder_IP=$2

# Alert Type
intruder_AlertType=$4

# Mail recipient
mail_To="Jean Dupont <jdupond@domain.com>"

# Subject
mail_Subject="[Warning] Intrusion Detection [Warning]"

# Body and send mail
cat << EOF | mail -s "$mail_Subject" $mail_To
/!\ Intruder Detected /!\

Intrusion time stamp : $(date)

Intruder Ip Address : $intruder_IP
Intruder MAC Address : $intruder_MAC
Type of alert : $intruder_AlertType
EOF

