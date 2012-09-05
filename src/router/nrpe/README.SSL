NRPE With SSL/TLS

NRPE now has the option for Encrypting Network traffic using
SSL/TLS from openssl. 

The Encryption is done using a set encryption routine of 
AES-256 Bit Encryption using SHA and Anon-DH. This encrypts
all traffic using the NRPE sockets from the client to the server.

Since we are using Anon-DH this allows for an encrypted 
SSL/TLS Connection without using pre-generated keys or 
certificates. The key generation information used by the 
program to dynaically create keys on daemon startup can be found
in the dh.h file in the nrpe src directory. This file was created
using the command:

openssl dhparam -C 512 

which outputs the C code in dh.h. For your own security you can replace
that file with your own dhparam generated code.

As of this time you will need to have the latest greatest version of
OpenSSL (tested against version 0.9.7a) since not all versions have
the AES algorythm in them.

I am not aware that at this time this code is restricted under export 
restrictions but I leave that verification process up to you.

Thoughts and suggestions are welcome and I can be reached on the
Nagios and NagiosPlug Mailing Lists.

	- Derrick


