#!/bin/sh

#SECS=$1

# create the key and certificate request
openssl req -new -out cert.csr -config openssl.cnf -keyout privkey.pem -newkey rsa:2048 -passout pass:password
# remove the passphrase from the key
openssl rsa -in privkey.pem -out key.pem -passin pass:password
# convert the certificate request into a signed certificate
SECS="1145611923"
openssl x509 -in cert.csr -out cert.pem -req -signkey key.pem -days 3650
# Show human-readable format
openssl x509 -in cert.pem -text -noout
# Remove unused files
#rm -f /tmp/cert.csr /tmp/privkey.pem
