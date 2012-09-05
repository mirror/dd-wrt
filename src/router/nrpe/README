-----------
NRPE README
-----------


** UPDATED DOCUMENTATION!

For installation instructions and information on the design overview
of the NRPE addon, please read the PDF documentation that is found in 
this directory: NRPE.pdf




Purpose
-------
The purpose of this addon is to allow you to execute Nagios 
plugins on a remote host in as transparent a manner as possible.


Contents
--------

There are two pieces to this addon:

  1) NRPE       - This program runs as a background process on the 
                  remote host and processes command execution requests
	          from the check_nrpe plugin on the Nagios host.
		  Upon receiving a plugin request from an authorized
                  host, it will execute the command line associated
                  with the command name it received and send the
                  program output and return code back to the 
                  check_nrpe plugin

  2) check_nrpe - This is a plugin that is run on the Nagios host
                  and is used to contact the NRPE process on remote
	          hosts.  The plugin requests that a plugin be
                  executed on the remote host and wait for the NRPE
                  process to execute the plugin and return the result.
                  The plugin then uses the output and return code
                  from the plugin execution on the remote host for
                  its own output and return code.


Compiling
---------

The code is very basic and may not work on your particular
system without some tweaking.  I just haven't put a lot of effort
into this addond.  Most Linux users should be able to compile
NRPE and the check_nrpe plugin with the following commands...

./configure
make all

The binaries will be located in the src/ directory after you
run 'make all' and will have to be installed manually somewhere
on your system.


NOTE: Since the check_nrpe plugin and nrpe daemon run on different
      machines (the plugin runs on the Nagios host and the daemon
      runs on the remote host), you will have to compile the nrpe
      daemon on the target machine.



Installing
----------

The check_nrpe plugin should be placed on the Nagios host along
with your other plugins.  In most cases, this will be in the
/usr/local/nagios/libexec directory.

The nrpe program and the configuration file (nrpe.cfg) should
be placed somewhere on the remote host.  Note that you will also
have to install some plugins on the remote host if you want to
make much use of this addon.



Configuring
-----------

Sample config files for the NRPE daemon are located in the
sample-config/ subdirectory.



Running Under INETD or XINETD
-----------------------------

If you plan on running nrpe under inetd or xinetd and making use
of TCP wrappers, you need to do the following things:



1) Add a line to your /etc/services file as follows (modify the port
   number as you see fit)

	nrpe            5666/tcp	# NRPE



2) Add entries for the NRPE daemon to either your inetd or xinetd
   configuration files.  Which one your use will depend on which
   superserver is installed on your system.  Both methods are described
   below.  NOTE: If you run nrpe under inetd or xinetd, the server_port
   and allowed_hosts variables in the nrpe configuration file are
   ignored.


   ***** INETD *****
   If your system uses the inetd superserver WITH tcpwrappers, add an
   entry to /etc/inetd.conf as follows:

	nrpe 	stream 	tcp 	nowait 	<user> /usr/sbin/tcpd <nrpebin> -c <nrpecfg> --inetd

   If your system uses the inetd superserver WITHOUT tcpwrappers, add an
   entry to /etc/inetd.conf as follows:

	nrpe 	stream 	tcp 	nowait 	<user> <nrpebin> -c <nrpecfg> --inetd


   - Replace <user> with the name of the user that the nrpe server should run as.
     	Example: nagios
   - Replace <nrpebin> with the path to the nrpe binary on your system.
	Example: /usr/local/nagios/nrpe
   - Replace <nrpecfg> with the path to the nrpe config file on your system.
	Example: /usr/local/nagios/nrpe.cfg


   ***** XINETD *****
   If your system uses xinetd instead of inetd, you'll probably
   want to create a file called 'nrpe' in your /etc/xinetd.d
   directory that contains the following entries:


	# default: on
	# description: NRPE
	service nrpe
	{
        	flags           = REUSE
	        socket_type     = stream        
        	wait            = no
	        user            = <user>
        	server          = <nrpebin>
	        server_args     = -c <nrpecfg> --inetd
        	log_on_failure  += USERID
	        disable         = no
		only_from       = <ipaddress1> <ipaddress2> ...
	}


   - Replace <user> with the name of the user that the nrpe server should run as.
   - Replace <nrpebin> with the path to the nrpe binary on your system.
   - Replace <nrpecfg> with the path to the nrpe config file on your system.
   - Replace the <ipaddress> fields with the IP addresses of hosts which
     are allowed to connect to the NRPE daemon.  This only works if xinetd was
     compiled with support for tcpwrappers.



3) Restart inetd or xinetd will the following command (pick the
   on that is appropriate for your system:

	/etc/rc.d/init.d/inet restart

	/etc/rc.d/init.d/xinetd restart

   OpenBSD users can use the following command to restart inetd:

	kill -HUP `cat /var/run/inet.pid`



4) Add entries to your /etc/hosts.allow and /etc/hosts.deny
   file to enable TCP wrapper protection for the nrpe service.
   This is optional, although highly recommended.




Configuring Things On The Nagios Host
---------------------------------------

Examples for configuring the nrpe daemon are found in the sample
nrpe.cfg file included in this distribution.  That config file
resides on the remote host(s) along with the nrpe daemon.  The
check_nrpe plugin gets installed on the Nagios host.  In order
to use the check_nrpe plugin from within Nagios, you'll have
to define a few things in the host config file.  An example
command definition for the check_nrpe plugin would look like this:

define command{
	command_name	check_nrpe
	command_line	/usr/local/nagios/libexec/check_nrpe -H $HOSTADDRESS$ -c $ARG1$
	}

In any service definitions that use the nrpe plugin/daemon to
get their results, you would set the service check command portion
of the definition to something like this (sample service definition
is simplified for this example):

define service{
	host_name		someremotehost
	service_description	someremoteservice
	check_command		check_nrpe!yourcommand
	... etc ...
	}

where "yourcommand" is a name of a command that you define in 
your nrpe.cfg file on the remote host (see the docs in the 
sample nrpe.cfg file for more information).




Questions?
----------

If you have questions about this addon, or problems getting things
working, first try searching the nagios-users mailing list archives.
Details on searching the list archives can be found at 
http://www.nagios.org

If all else fails, you can email me and I'll try and respond as
soon as I get a chance.

	-- Ethan Galstad (nagios@nagios.org)




