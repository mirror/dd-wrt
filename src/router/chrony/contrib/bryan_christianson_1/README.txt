Notes for installing chrony on macOS
Author: Bryan Christianson (bryan@whatroute.net)
------------------------------------------------

These files are for those admins/users who would prefer to install chrony
from the source distribution and are intended as guidelines rather than
being definitive. They can be edited with a plain text editor, such as
vi, emacs or your favourite IDE (Xcode)

It is assumed you are comfortable with installing software from the
terminal command line and know how to use sudo to acquire root access.

If you are not familiar with the macOS command line then
please consider using ChronyControl from http://whatroute.net/chronycontrol.html

ChronyControl provides a gui wrapper for installing these files and sets the
necessary permissions on each file.


Install the chrony software
---------------------------

You will need xcode and the commandline additions to build and install chrony.
These can be obtained from Apple's website via the App Store.

cd to the chrony directory
./configure
make
sudo make install

chrony is now installed in default locations (/usr/local/sbin/chronyd,
/usr/local/bin/chronyc)

Create a chrony.conf file - see the chrony website for details

The support files here assume the following directives are specified in the
chrony.conf file

keyfile /etc/chrony.d/chrony.keys
driftfile /var/db/chrony/chrony.drift
bindcmdaddress /var/db/chrony/chronyd.sock
logdir /var/log/chrony
dumpdir /var/db/chrony

Install this file as /etc/chrony.d/chrony.conf and create
the directories specified in the above directives if they don't exist.
You will need root permissions to create the directories.


Running chronyd
---------------
At this point chronyd *could* be run as a daemon. Apple discourage running
daemons and their preferred method uses the launchd facility. The
support files here provide a launchd configuration file for chronyd and also
a shell script and launchd configuration file to rotate the chronyd logs on a daily basis.


Support files
-------------
Dates and sizes may differ
-rw-r--r--  1 yourname  staff  2084  4 Aug 22:54 README.txt
-rwxr-xr-x  1 yourname  staff   676  4 Aug 21:18 chronylogrotate.sh
-rw-r--r--  1 yourname  staff   543 18 Jul 20:10 org.tuxfamily.chronyc.plist
-rw-r--r--  1 yourname  staff   511 19 Jun 18:30 org.tuxfamily.chronyd.plist

If you have used chrony support directories other than those suggested, you
will need to edit each file and make the appropriate changes.


Installing the support files
----------------------------

1. chronylogrotate.sh
This is a simple shell script that deletes old log files. Unfortunately because
of the need to run chronyc, the standard macOS logrotation does not work with
chrony logs.

This script runs on a daily basis under control of launchd and should be
installed in the /usr/local/bin directory

sudo cp chronylogrotate.sh /usr/local/bin
sudo chmod +x /usr/local/bin/chronylogrotate.sh
sudo chown root:wheel /usr/local/bin/chronylogrotate.sh


2. org.tuxfamily.chronyc.plist
This file is the launchd plist that runs logrotation each day. You may
wish to edit this file to change the time of day at which the rotation
will run, currently 04:05 am

sudo cp org.tuxfamily.chronyc.plist /Library/LaunchDaemons
sudo chown root:wheel /Library/LaunchDaemons/org.tuxfamily.chronyc.plist
sudo chmod 0644 /Library/LaunchDaemons/org.tuxfamily.chronyc.plist
sudo launchctl load -w /Library/LaunchDaemons/org.tuxfamily.chronyc.plist


3. org.tuxfamily.chronyd.plist
This file is the launchd plist that runs chronyd when the Macintosh starts.

sudo cp org.tuxfamily.chronyd.plist /Library/LaunchDaemons
sudo chown root:wheel /Library/LaunchDaemons/org.tuxfamily.chronyd.plist
sudo chmod 0644 /Library/LaunchDaemons/org.tuxfamily.chronyd.plist
sudo launchctl load -w /Library/LaunchDaemons/org.tuxfamily.chronyd.plist
