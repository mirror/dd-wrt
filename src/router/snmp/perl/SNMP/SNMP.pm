# SNMP.pm -- Perl 5 interface to the net-snmp toolkit
#
# written by G. S. Marzot (gmarzot@nortelnetworks.com)
#
#     Copyright (c) 1995-2000 G. S. Marzot. All rights reserved.
#     This program is free software; you can redistribute it and/or
#     modify it under the same terms as Perl itself.

package SNMP;
$VERSION = '5.0.9';   # current release version number

require Exporter;
require DynaLoader;
require AutoLoader;

use NetSNMP::default_store (':all');

@SNMP::ISA = qw(Exporter AutoLoader DynaLoader);
# Items to export into callers namespace by default. Note: do not export
# names by default without a very good reason. Use EXPORT_OK instead.
# Do not simply export all your public functions/methods/constants.
@SNMP::EXPORT = qw(
	RECEIVED_MESSAGE
	SNMPERR_BAD_ADDRESS
	SNMPERR_BAD_LOCPORT
	SNMPERR_BAD_SESSION
	SNMPERR_GENERR
	SNMPERR_TOO_LONG
	SNMP_DEFAULT_ADDRESS
	SNMP_DEFAULT_COMMUNITY_LEN
	SNMP_DEFAULT_ENTERPRISE_LENGTH
	SNMP_DEFAULT_ERRINDEX
	SNMP_DEFAULT_ERRSTAT
	SNMP_DEFAULT_PEERNAME
	SNMP_DEFAULT_REMPORT
	SNMP_DEFAULT_REQID
	SNMP_DEFAULT_RETRIES
	SNMP_DEFAULT_TIME
	SNMP_DEFAULT_TIMEOUT
	SNMP_DEFAULT_VERSION
	TIMED_OUT
	snmp_get
        snmp_getnext
        snmp_set
        snmp_trap
);

sub AUTOLOAD {
    # This AUTOLOAD is used to 'autoload' constants from the constant()
    # XS function.  If a constant is not found then control is passed
    # to the AUTOLOAD in AutoLoader.
    my($val,$pack,$file,$line);
    local($constname);
    ($constname = $AUTOLOAD) =~ s/.*:://;
    # croak "&$module::constant not defined" if $constname eq 'constant';
    $val = constant($constname, @_ ? $_[0] : 0);
    if ($! != 0) {
	if ($! =~ /Invalid/) {
	    $AutoLoader::AUTOLOAD = $AUTOLOAD;
	    goto &AutoLoader::AUTOLOAD;
	}
	else {
	    ($pack,$file,$line) = caller;
	    die "Your vendor has not defined SNMP macro $constname, used at $file line $line.
";
	}
    }
    eval "sub $AUTOLOAD { $val }";
    goto &$AUTOLOAD;
}

bootstrap SNMP;

# Preloaded methods go here.

# Package variables
tie $SNMP::debugging, SNMP::DEBUGGING;
tie $SNMP::debug_internals, SNMP::DEBUG_INTERNALS;
tie $SNMP::dump_packet, SNMP::DUMP_PACKET;
tie %SNMP::MIB, SNMP::MIB;
tie $SNMP::save_descriptions, SNMP::MIB::SAVE_DESCR;
tie $SNMP::replace_newer, SNMP::MIB::REPLACE_NEWER;

%SNMP::V3_SEC_LEVEL_MAP = (noAuthNoPriv => 1, authNoPriv => 2, authPriv =>3);

$auto_init_mib = 1; # enable automatic MIB loading at session creation time
$use_long_names = 0; # non-zero to prefer longer mib textual identifiers rather
                   # than just leaf indentifiers (see translateObj)
                   # may also be set on a per session basis(see UseLongNames)
$use_sprint_value = 0; # non-zero to enable formatting of response values
                   # using the snmp libraries "snprint_value"
                   # may also be set on a per session basis(see UseSprintValue)
                   # note: returned values not suitable for 'set' operations
$use_enums = 0; # non-zero to return integers as enums and allow sets
                # using enums where appropriate - integer data will
                # still be accepted for set operations
                # may also be set on a per session basis (see UseEnums)
$use_numeric = 0; # non-zero to return object tags as numeric OID's instead
                  # of converting to textual representations.  use_long_names,
                  # if non-zero, returns the entire OID, otherwise, return just
                  # the label portion.  use_long_names is also set if the
		  # use_numeric variable is set.
%MIB = ();      # tied hash to access libraries internal mib tree structure
                # parsed in from mib files
$verbose = 0;   # controls warning/info output of SNMP module,
                # 0 => no output, 1 => enables warning and info
                # output from SNMP module itself (is also controlled
                # by SNMP::debugging)
$debugging = 0; # non-zero to globally enable libsnmp do_debugging output
                # set to >= 2 to enabling packet dumping (see below)
$dump_packet = 0; # non-zero to globally enable libsnmp dump_packet output.
                  # is also enabled when $debugging >= 2
$save_descriptions = 0; #tied scalar to control saving descriptions during
               # mib parsing - must be set prior to mib loading
$best_guess = 0;  # determine whether or not to enable best-guess regular
                  # expression object name translation
$replace_newer = 0; # determine whether or not to tell the parser to replace
                    # older MIB modules with newer ones when loading MIBs.
                    # WARNING: This can cause an incorrect hierarchy.

sub setMib {
# loads mib from file name provided
# setting second arg to true causes currently loaded mib to be replaced
# otherwise mib file will be added to existing loaded mib database
# NOTE: now deprecated in favor of addMibFiles and new module based funcs
   my $file = shift;
   my $force = shift || '0';
   return 0 if $file and not (-r $file);
   SNMP::_read_mib($file,$force);
}

sub initMib {
# eqivalent to calling the snmp library init_mib if Mib is NULL
# if Mib is already loaded this function does nothing
# Pass a zero valued argument to get minimal mib tree initialzation
# If non zero agrgument or no argument then full mib initialization

  SNMP::init_snmp("perl");
  return;


  if (defined $_[0] and $_[0] == 0) {
    SNMP::_init_mib_internals();
  } else {
    SNMP::_read_mib("");
  }
}

sub addMibDirs {
# adds directories to search path when a module is requested to be loaded
  SNMP::init_snmp("perl");
  foreach (@_) {
    SNMP::_add_mib_dir($_) or return undef;
  }
  return 1;
}

sub addMibFiles {
# adds mib definitions to currently loaded mib database from
# file(s) supplied
  SNMP::init_snmp("perl");
  foreach (@_) {
    SNMP::_read_mib($_) or return undef;
  }
  return 1;
}

sub loadModules {
# adds mib module definitions to currently loaded mib database.
# Modules will be searched from previously defined mib search dirs
# Passing and arg of 'ALL' will cause all known modules to be loaded
   SNMP::init_snmp("perl");
   foreach (@_) {
     SNMP::_read_module($_) or return undef;
   }
   return 1;
}

sub unloadModules {
# causes modules to be unloaded from mib database
# Passing and arg of 'ALL' will cause all known modules to be unloaded
  warn("SNMP::unloadModules not implemented! (yet)");
}

sub translateObj {
# translate object identifier(tag or numeric) into alternate representation
# (i.e., sysDescr => '.1.3.6.1.2.1.1.1' and '.1.3.6.1.2.1.1.1' => sysDescr)
# when $SNMP::use_long_names or second arg is non-zero the translation will
# return longer textual identifiers (e.g., system.sysDescr)
# if Mib is not loaded and $SNMP::auto_init_mib is enabled Mib will be loaded
# returns 'undef' upon failure
   SNMP::init_snmp("perl");
   my $obj = shift;
   my $long_names = shift || $SNMP::use_long_names;
   return undef if not defined $obj;
   my $res;
   if ($obj =~ /^\.?(\d+\.)*\d+$/) {
      $res = SNMP::_translate_obj($obj,1,$long_names,$SNMP::auto_init_mib,0);
   } elsif ($obj =~ /(\.\d+)*$/ && $SNMP::best_guess == 0) {
      $res = SNMP::_translate_obj($`,0,$long_names,$SNMP::auto_init_mib,0);
      $res .= $& if defined $res and defined $&;
   } elsif ($SNMP::best_guess) {
      $res = SNMP::_translate_obj($obj,0,$long_names,$SNMP::auto_init_mib,$SNMP::best_guess);
   }

   return($res);
}

sub getType {
# return SNMP data type for given textual identifier
# OBJECTID, OCTETSTR, INTEGER, NETADDR, IPADDR, COUNTER
# GAUGE, TIMETICKS, OPAQUE, or undef
  my $tag = shift;
  SNMP::_get_type($tag);
}

sub mapEnum {
# return the corresponding integer value *or* tag for a given MIB attribute
# and value. The function will sense which direction to perform the conversion
# various arg formats are supported
#    $val = SNMP::mapEnum($varbind); # note: will update $varbind
#    $val = SNMP::mapEnum('ipForwarding', 'forwarding');
#    $val = SNMP::mapEnum('ipForwarding', 1);
#
  my $var = shift;
  my ($tag, $val, $update);
  if (ref($var) =~ /ARRAY/ or ref($var) =~ /Varbind/) {
      $tag = $var->[$SNMP::Varbind::tag_f];
      $val = $var->[$SNMP::Varbind::val_f];
      $update = 1;
  } else {
      $tag = $var;
      $val = shift;
  }
  my $iflag = $val =~ /^\d+$/;
  my $res = SNMP::_map_enum($tag, $val, $iflag);
  if ($update and defined $res) { $var->[$SNMP::Varbind::val_f] = $res; }
  return($res);
}

%session_params = (DestHost => 1,
		   Community => 1,
		   Version => 1,
		   Timeout => 1,
		   Retries => 1,
		   RemotePort => 1,
                   LocalPort => 1);

sub strip_session_params {
    my @params;
    my @args;
    my $param;
    while ($param = shift) {
	push(@params,$param, shift), next
	    if $session_params{$param};
	push(@args,$param);
    }
    @_ = @args;
    @params;
}


sub snmp_get {
# procedural form of 'get' method. sometimes quicker to code
# but is less efficient since the Session is created and destroyed
# with each call. Takes all the parameters of both SNMP::Session::new and
# SNMP::Session::get (*NOTE*: this api does not support async callbacks)

    my @sess_params = &strip_session_params;
    my $sess = new SNMP::Session(@sess_params);

    $sess->get(@_);
}

sub snmp_getnext {
# procedural form of 'getnext' method. sometimes quicker to code
# but is less efficient since the Session is created and destroyed
# with each call. Takes all the parameters of both SNMP::Session::new and
# SNMP::Session::getnext (*NOTE*: this api does not support async callbacks)

    my @sess_params = &strip_session_params;
    my $sess = new SNMP::Session(@sess_params);

    $sess->getnext(@_);
}

sub snmp_set {
# procedural form of 'set' method. sometimes quicker to code
# but is less efficient since the Session is created and destroyed
# with each call. Takes all the parameters of both SNMP::Session::new and
# SNMP::Session::set (*NOTE*: this api does not support async callbacks)

    my @sess_params = &strip_session_params;
    my $sess = new SNMP::Session(@sess_params);

    $sess->set(@_);
}

sub snmp_trap {
# procedural form of 'trap' method. sometimes quicker to code
# but is less efficient since the Session is created and destroyed
# with each call. Takes all the parameters of both SNMP::TrapSession::new and
# SNMP::TrapSession::trap

    my @sess_params = &strip_session_params;
    my $sess = new SNMP::TrapSession(@sess_params);

    $sess->trap(@_);
}

sub MainLoop {
    my $time = shift;
    my $callback = shift;
    my $time_sec = ($time ? int $time : 0);
    my $time_usec = ($time ? int(($time-$time_sec)*1000000) : 0);
    SNMP::_main_loop($time_sec,$time_usec,$callback);
}

sub finish {
    SNMP::_mainloop_finish();
}

sub reply_cb {
    # callback function for async snmp calls
    # when triggered, will do a SNMP read on the
    # given fd
    my $fd = shift;
  SNMP::_read_on_fd($fd);
}

sub select_info {
    # retrieves SNMP used fd's and timeout info
    # calculates timeout in fractional seconds
    # ( easy to use with select statement )
    my($block, $to_sec, $to_usec, @fd_set)=SNMP::_get_select_info();
    my $time_sec_dec = ($block? 0 : $to_sec + $to_usec * 1e-6);
    #print "fd's for snmp -> ", @fd_set, "\n";
    #print "block		-> ", $block, "\n";
    #print "timeout_sec	-> ", $to_sec, "\n";
    #print "timeout_usec	-> ", $to_usec, "\n";
    #print "timeout dec	-> ", $time_sec_dec, "\n";
    return ($time_sec_dec,@fd_set);
}

sub check_timeout {
  # check to see if a snmp session
  # timed out, and if so triggers
  # the callback function
  SNMP::_check_timeout();
  # check to see when have to check again
  my($block, $to_sec, $to_usec, @fd_set)=SNMP::_get_select_info();
  my $time_sec_dec = ($block? 0 : $to_sec + $to_usec * 1e-6);
  #print "fd's for snmp -> ", @fd_set, "\n";
  #print "block		-> ", $block, "\n";
  #print "timeout_sec	-> ", $to_sec, "\n";
  #print "timeout_usec	-> ", $to_usec, "\n";
  #print "timeout dec	-> ", $time_sec_dec, "\n";
  return ($time_sec_dec);
}

sub _tie {
# this is a little implementation hack so ActiveState can access pp_tie
# thru perl code. All other environments allow the calling of pp_tie from
# XS code but AS was not exporting it when PERL_OBJECT was used.
#
# short term solution was call this perl func which calls 'tie'
#
# longterm fix is to supply a patch which allows AS to export pp_tie in
# such a way that it can be called from XS code. gsarathy says:
# a patch to util.c is needed to provide access to PL_paddr
# so it is possible to call PL_paddr[OP_TIE] as the compiler does
    tie($_[0],$_[1],$_[2],$_[3]);
}

package SNMP::Session;

sub new {
   my $type = shift;
   my $this = {};
   my ($name, $aliases, $host_type, $len, $thisaddr);

   SNMP::init_snmp("perl");

   %$this = @_;

   $this->{ErrorStr} = ''; # if methods return undef check for expln.
   $this->{ErrorNum} = 0;  # contains SNMP error return

   $this->{Version} ||= 
     NetSNMP::default_store::netsnmp_ds_get_int(NetSNMP::default_store::NETSNMP_DS_LIBRARY_ID, 
				      NetSNMP::default_store::NETSNMP_DS_LIB_SNMPVERSION) ||
					SNMP::SNMP_DEFAULT_VERSION();

   if ($this->{Version} eq 128) {
       # special handling of the bogus v1 definition.
       $this->{Version} = 1;
   }

   # allow override of local SNMP port
   $this->{LocalPort} ||= 0;

   # destination host defaults to localhost
   $this->{DestHost} ||= 'localhost';

   # community defaults to public
   $this->{Community} ||= NetSNMP::default_store::netsnmp_ds_get_string(NetSNMP::default_store::NETSNMP_DS_LIBRARY_ID(), 
				        NetSNMP::default_store::NETSNMP_DS_LIB_COMMUNITY()) || 'public';

   # number of retries before giving up, defaults to SNMP_DEFAULT_RETRIES
   $this->{Retries} = SNMP::SNMP_DEFAULT_RETRIES() unless defined($this->{Retries});

   # timeout before retry, defaults to SNMP_DEFAULT_TIMEOUT
   $this->{Timeout} = SNMP::SNMP_DEFAULT_TIMEOUT() unless defined($this->{Timeout});
   # flag to enable fixing pdu and retrying with a NoSuch error
   $this->{RetryNoSuch} ||= 0;

   # backwards compatibility.  Make host = host:port
   if ($this->{RemotePort} && $this->{DestHost} !~ /:/) {
       $this->{DestHost} = $this->{DestHost} . ":" . $this->{RemotePort};
   }

   if ($this->{Version} eq '1' or $this->{Version} eq '2'
       or $this->{Version} eq '2c') {
       $this->{SessPtr} = SNMP::_new_session($this->{Version},
					     $this->{Community},
					     $this->{DestHost},
					     $this->{LocalPort},
					     $this->{Retries},
					     $this->{Timeout},
					     );
   } elsif ($this->{Version} eq '3' ) {
       $this->{SecName} ||= 
	   NetSNMP::default_store::netsnmp_ds_get_string(NetSNMP::default_store::NETSNMP_DS_LIBRARY_ID(), 
		         NetSNMP::default_store::NETSNMP_DS_LIB_SECNAME()) || 
			   'initial';
       if (!$this->{SecLevel}) {
	   $this->{SecLevel} = 
	       NetSNMP::default_store::netsnmp_ds_get_int(NetSNMP::default_store::NETSNMP_DS_LIBRARY_ID(), 
			  NetSNMP::default_store::NETSNMP_DS_LIB_SECLEVEL()) || 
			      $SNMP::V3_SEC_LEVEL_MAP{'noAuthNoPriv'};
       } elsif ($this->{SecLevel} !~ /^\d+$/) {
	   $this->{SecLevel} = $SNMP::V3_SEC_LEVEL_MAP{$this->{SecLevel}};
       }
       $this->{SecEngineId} ||= '';
       $this->{ContextEngineId} ||= $this->{SecEngineId};
       $this->{Context} ||= 
	   NetSNMP::default_store::netsnmp_ds_get_string(NetSNMP::default_store::NETSNMP_DS_LIBRARY_ID(), 
		         NetSNMP::default_store::NETSNMP_DS_LIB_CONTEXT()) || '';
       $this->{AuthProto} ||= 'MD5'; # defaults XXX
       $this->{AuthPass} ||=
       NetSNMP::default_store::netsnmp_ds_get_string(NetSNMP::default_store::NETSNMP_DS_LIBRARY_ID(), 
		     NetSNMP::default_store::NETSNMP_DS_LIB_AUTHPASSPHRASE()) ||
       NetSNMP::default_store::netsnmp_ds_get_string(NetSNMP::default_store::NETSNMP_DS_LIBRARY_ID(), 
		     NetSNMP::default_store::NETSNMP_DS_LIB_PASSPHRASE()) || '';
       $this->{PrivProto} ||= 'DES';  # defaults XXX
       $this->{PrivPass} ||=
       NetSNMP::default_store::netsnmp_ds_get_string(NetSNMP::default_store::NETSNMP_DS_LIBRARY_ID(), 
		     NetSNMP::default_store::NETSNMP_DS_LIB_PRIVPASSPHRASE()) ||
       NetSNMP::default_store::netsnmp_ds_get_string(NetSNMP::default_store::NETSNMP_DS_LIBRARY_ID(), 
		     NetSNMP::default_store::NETSNMP_DS_LIB_PASSPHRASE()) || '';
       $this->{EngineBoots} = 0 if not defined $this->{EngineBoots};
       $this->{EngineTime} = 0 if not defined $this->{EngineTime};

       $this->{SessPtr} = SNMP::_new_v3_session($this->{Version},
						$this->{DestHost},
						$this->{Retries},
						$this->{Timeout},
						$this->{SecName},
						$this->{SecLevel},
						$this->{SecEngineId},
						$this->{ContextEngineId},
						$this->{Context},
						$this->{AuthProto},
						$this->{AuthPass},
						$this->{PrivProto},
						$this->{PrivPass},
						$this->{EngineBoots},
						$this->{EngineTime},
						);
   }
   unless ($this->{SessPtr}) {
       warn("unable to create session") if $SNMP::verbose;
       return undef;
   }

   SNMP::initMib($SNMP::auto_init_mib); # ensures that *some* mib is loaded

   $this->{UseLongNames} ||= $SNMP::use_long_names;
   $this->{UseSprintValue} ||= $SNMP::use_sprint_value;
   $this->{UseEnums} ||= $SNMP::use_enums;
   $this->{UseNumeric} ||= $SNMP::use_numeric;

   # Force UseLongNames if UseNumeric is in use.
   $this->{UseLongNames}++  if $this->{UseNumeric};

   bless $this, $type;
}

sub update {
# *Not Implemented*
# designed to update the fields of session to allow retargetting to different
# host, community name change, timeout, retry changes etc. Unfortunately not
# working yet because some updates (the address in particular) need to be
# done on the internal session pointer which cannot be fetched w/o touching
# globals at this point which breaks win32. A patch to the ucd-snmp toolkit
# is needed
   my $this = shift;
   my ($name, $aliases, $host_type, $len, $thisaddr);
   my %new_fields = @_;

   @$this{keys %new_fields} = values %new_fields;

   $this->{UseLongNames} ||= $SNMP::use_long_names;
   $this->{UseSprintValue} ||= $SNMP::use_sprint_value;
   $this->{UseEnums} ||= $SNMP::use_enums;
   $this->{UseNumeric} ||= $SNMP::use_numeric;

   # Force UseLongNames if UseNumeric is in use.
   $this->{UseLongNames}++  if $this->{UseNumeric};

   SNMP::_update_session($this->{Version},
		 $this->{Community},
		 $this->{DestHost},
		 $this->{RemotePort},
		 $this->{LocalPort},
		 $this->{Retries},
		 $this->{Timeout},
		);


}

sub set {
   my $this = shift;
   my $vars = shift;
   my $varbind_list_ref;
   my $res = 0;

   if (ref($vars) =~ /SNMP::VarList/) {
     $varbind_list_ref = $vars;
   } elsif (ref($vars) =~ /SNMP::Varbind/) {
     $varbind_list_ref = [$vars];
   } elsif (ref($vars) =~ /ARRAY/) {
     $varbind_list_ref = [$vars];
     $varbind_list_ref = $vars if ref($$vars[0]) =~ /ARRAY/;
   } else {
     my ($tag, $iid) = ($vars =~ /^((?:\.\d+)+|(?:\w+(?:\-*\w+)+))\.?(.*)$/);
     my $val = shift;
     $varbind_list_ref = [[$tag, $iid, $val]];
   }
   my $cb = shift;

   $res = SNMP::_set($this, $varbind_list_ref, $cb);
}

sub get {
   my $this = shift;
   my $vars = shift;
   my ($varbind_list_ref, @res);

   if (ref($vars) =~ /SNMP::VarList/) {
     $varbind_list_ref = $vars;
   } elsif (ref($vars) =~ /SNMP::Varbind/) {
     $varbind_list_ref = [$vars];
   } elsif (ref($vars) =~ /ARRAY/) {
     $varbind_list_ref = [$vars];
     $varbind_list_ref = $vars if ref($$vars[0]) =~ /ARRAY/;
   } else {
     my ($tag, $iid) = ($vars =~ /^((?:\.\d+)+|(?:\w+(?:\-*\w+)+))\.?(.*)$/);
     $varbind_list_ref = [[$tag, $iid]];
   }

   my $cb = shift;

   @res = SNMP::_get($this, $this->{RetryNoSuch}, $varbind_list_ref, $cb);

   return(wantarray() ? @res : $res[0]);
}

sub fget {
   my $this = shift;
   my $vars = shift;
   my ($varbind_list_ref, @res);

   if (ref($vars) =~ /SNMP::VarList/) {
     $varbind_list_ref = $vars;
   } elsif (ref($vars) =~ /SNMP::Varbind/) {
     $varbind_list_ref = [$vars];
   } elsif (ref($vars) =~ /ARRAY/) {
     $varbind_list_ref = [$vars];
     $varbind_list_ref = $vars if ref($$vars[0]) =~ /ARRAY/;
   } else {
     my ($tag, $iid) = ($vars =~ /^((?:\.\d+)+|(?:\w+(?:\-*\w+)+))\.?(.*)$/);
     $varbind_list_ref = [[$tag, $iid]];
   }

   my $cb = shift;

   SNMP::_get($this, $this->{RetryNoSuch}, $varbind_list_ref, $cb);

   foreach $varbind (@$varbind_list_ref) {
     $sub = $this->{VarFormats}{$varbind->[$SNMP::Varbind::tag_f]} ||
	 $this->{TypeFormats}{$varbind->[$SNMP::Varbind::type_f]};
     &$sub($varbind) if defined $sub;
     push(@res, $varbind->[$SNMP::Varbind::val_f]);
   }

   return(wantarray() ? @res : $res[0]);
}

sub getnext {
   my $this = shift;
   my $vars = shift;
   my ($varbind_list_ref, @res);

   if (ref($vars) =~ /SNMP::VarList/) {
     $varbind_list_ref = $vars;
   } elsif (ref($vars) =~ /SNMP::Varbind/) {
     $varbind_list_ref = [$vars];
   } elsif (ref($vars) =~ /ARRAY/) {
     $varbind_list_ref = [$vars];
     $varbind_list_ref = $vars if ref($$vars[0]) =~ /ARRAY/;
   } else {
     my ($tag, $iid) = ($vars =~ /^((?:\.\d+)+|(?:\w+(?:\-*\w+)+))\.?(.*)$/);
     $varbind_list_ref = [[$tag, $iid]];
   }

   my $cb = shift;

   @res = SNMP::_getnext($this, $varbind_list_ref, $cb);

   return(wantarray() ? @res : $res[0]);
}

sub fgetnext {
   my $this = shift;
   my $vars = shift;
   my ($varbind_list_ref, @res);

   if (ref($vars) =~ /SNMP::VarList/) {
     $varbind_list_ref = $vars;
   } elsif (ref($vars) =~ /SNMP::Varbind/) {
     $varbind_list_ref = [$vars];
   } elsif (ref($vars) =~ /ARRAY/) {
     $varbind_list_ref = [$vars];
     $varbind_list_ref = $vars if ref($$vars[0]) =~ /ARRAY/;
   } else {
     my ($tag, $iid) = ($vars =~ /^((?:\.\d+)+|(?:\w+(?:\-*\w+)+))\.?(.*)$/);
     $varbind_list_ref = [[$tag, $iid]];
   }

   my $cb = shift;

   SNMP::_getnext($this, $varbind_list_ref, $cb);

   foreach $varbind (@$varbind_list_ref) {
     $sub = $this->{VarFormats}{$varbind->[$SNMP::Varbind::tag_f]} ||
	 $this->{TypeFormats}{$varbind->[$SNMP::Varbind::type_f]};
     &$sub($varbind) if defined $sub;
     push(@res, $varbind->[$SNMP::Varbind::val_f]);
   }

   return(wantarray() ? @res : $res[0]);
}

sub getbulk {
   my $this = shift;
   my $nonrepeaters = shift;
   my $maxrepetitions = shift;
   my $vars = shift;
   my ($varbind_list_ref, @res);

   if (ref($vars) =~ /SNMP::VarList/) {
     $varbind_list_ref = $vars;
   } elsif (ref($vars) =~ /SNMP::Varbind/) {
     $varbind_list_ref = [$vars];
   } elsif (ref($vars) =~ /ARRAY/) {
     $varbind_list_ref = [$vars];
     $varbind_list_ref = $vars if ref($$vars[0]) =~ /ARRAY/;
   } else {
     my ($tag, $iid) = ($vars =~ /^((?:\.\d+)+|(?:\w+(?:\-*\w+)+))\.?(.*)$/);
     $varbind_list_ref = [[$tag, $iid]];
   }

   my $cb = shift;

   @res = SNMP::_getbulk($this, $nonrepeaters, $maxrepetitions, $varbind_list_ref, $cb);

   return(wantarray() ? @res : $res[0]);
}

sub bulkwalk {
   my $this = shift;
   my $nonrepeaters = shift;
   my $maxrepetitions = shift;
   my $vars = shift;
   my ($varbind_list_ref, @res);

   if (ref($vars) =~ /SNMP::VarList/) {
      $varbind_list_ref = $vars;
   } elsif (ref($vars) =~ /SNMP::Varbind/) {
      $varbind_list_ref = [$vars];
   } elsif (ref($vars) =~ /ARRAY/) {
      $varbind_list_ref = [$vars];
      $varbind_list_ref = $vars if ref($$vars[0]) =~ /ARRAY/;
   } else {
      my ($tag, $iid) = ($vars =~ /^((?:\.\d+)+|\w+)\.?(.*)$/);
      $varbind_list_ref = [[$tag, $iid]];
   }

   if (scalar @$varbind_list_ref == 0) {
      $this->{ErrorNum} = SNMP::constant("SNMPERR_GENERR", 0);
      $this->{ErrorStr} = "cannot bulkwalk() empty variable list";
      return undef;
   }
   if (scalar @$varbind_list_ref < $nonrepeaters) {
      $this->{ErrorNum} = SNMP::constant("SNMPERR_GENERR", 0);
      $this->{ErrorStr} = "bulkwalk() needs at least $nonrepeaters varbinds";
      return undef;
   }

   my $cb = shift;
   @res = SNMP::_bulkwalk($this, $nonrepeaters, $maxrepetitions,
						$varbind_list_ref, $cb);

   # Return, in list context, a copy of the array of arrays of Varbind refs.
   # In scalar context, return either a reference to the array of arrays of
   # Varbind refs, or the request ID for an asynchronous bulkwalk.  This is
   # a compromise between the getbulk()-ish return, and the more useful array
   # of arrays of Varbinds return from the synchronous bulkwalk().
   #
   return @res if (wantarray());
   return defined($cb) ? $res[0] : \@res;
}

%trap_type = (coldStart => 0, warmStart => 1, linkDown => 2, linkUp => 3,
	      authFailure => 4, egpNeighborLoss => 5, specific => 6 );
sub trap {
# (v1) enterprise, agent, generic, specific, uptime, <vars>
# $sess->trap(enterprise=>'.1.3.6.1.4.1.2021', # or 'ucdavis' [default]
#             agent => '127.0.0.1', # or 'localhost',[default 1st intf on host]
#             generic => specific,  # can be omitted if 'specific' supplied
#             specific => 5,        # can be omitted if 'generic' supplied
#             uptime => 1234,       # default to localhost uptime (0 on win32)
#             [[ifIndex, 1, 1],[sysLocation, 0, "here"]]); # optional vars
#                                                          # always last
# (v2) oid, uptime, <vars>
# $sess->trap(uptime => 1234,
#             oid => 'snmpRisingAlarm',
#             [[ifIndex, 1, 1],[sysLocation, 0, "here"]]); # optional vars
#                                                          # always last
#                                                          # always last


   my $this = shift;
   my $vars = pop if ref($_[$#_]); # last arg may be varbind or varlist
   my %param = @_;
   my ($varbind_list_ref, @res);

   if (ref($vars) =~ /SNMP::VarList/) {
     $varbind_list_ref = $vars;
   } elsif (ref($vars) =~ /SNMP::Varbind/) {
     $varbind_list_ref = [$vars];
   } elsif (ref($vars) =~ /ARRAY/) {
     $varbind_list_ref = [$vars];
     $varbind_list_ref = $vars if ref($$vars[0]) =~ /ARRAY/;
   }

   if ($this->{Version} eq '1') {
       my $enterprise = $param{enterprise} || 'ucdavis';
       $enterprise = SNMP::translateObj($enterprise)
	   unless $enterprise =~ /^[\.\d]+$/;
       my $agent = $param{agent} || '';
       my $generic = $param{generic} || 'specific';
       $generic = $trap_type{$generic} || $generic;
       my $uptime = $param{uptime} || SNMP::_sys_uptime();
       my $specific = $param{specific} || 0;
       @res = SNMP::_trapV1($this, $enterprise, $agent, $generic, $specific,
			  $uptime, $varbind_list_ref);
   } elsif  (($this->{Version} eq '2')|| ($this->{Version} eq '2c')) {
       my $trap_oid = $param{oid} || $param{trapoid} || '.0.0';
       my $uptime = $param{uptime} || SNMP::_sys_uptime();
       @res = SNMP::_trapV2($this, $uptime, $trap_oid, $varbind_list_ref);
   }

   return(wantarray() ? @res : $res[0]);
}

sub inform {
# (v3) oid, uptime, <vars>
# $sess->inform(uptime => 1234,
#             oid => 'coldStart',
#             [[ifIndex, 1, 1],[sysLocation, 0, "here"]]); # optional vars
#                                                          # then callback
                                                           # always last


   my $this = shift;
   my $vars;
   my $cb;
   $cb = pop if ref($_[$#_]) eq 'CODE'; # last arg may be code
   $vars = pop if ref($_[$#_]); # varbind or varlist
   my %param = @_;
   my ($varbind_list_ref, @res);

   if (ref($vars) =~ /SNMP::VarList/) {
     $varbind_list_ref = $vars;
   } elsif (ref($vars) =~ /SNMP::Varbind/) {
     $varbind_list_ref = [$vars];
   } elsif (ref($vars) =~ /ARRAY/) {
     $varbind_list_ref = [$vars];
     $varbind_list_ref = $vars if ref($$vars[0]) =~ /ARRAY/;
   }

   my $trap_oid = $param{oid} || $param{trapoid};
   my $uptime = $param{uptime} || SNMP::_sys_uptime();

   if($this->{Version} eq '3') {
     @res = SNMP::_inform($this, $uptime, $trap_oid, $varbind_list_ref, $cb);
   } else {
     warn("error:inform: This version doesn't support the command\n");
   }

   return(wantarray() ? @res : $res[0]);
}

package SNMP::TrapSession;
@ISA = ('SNMP::Session');

sub new {
   my $type = shift;

   # allow override of remote SNMP trap port
   unless (grep(/RemotePort/, @_)) {
       push(@_, 'RemotePort', 162); # push on new default for trap session
   }

   SNMP::Session::new($type, @_);
}

package SNMP::Varbind;

$tag_f = 0;
$iid_f = 1;
$val_f = 2;
$type_f = 3;
$time_f = 4;

sub new {
   my $type = shift;
   my $this = shift;
   $this ||= [];
   bless $this;
}

sub tag {
  $_[0]->[$tag_f];
}

sub iid {
  $_[0]->[$iid_f];
}

sub val {
  $_[0]->[$val_f];
}

sub type {
  $_[0]->[$type_f];
}

sub name {
   if (defined($_[0]->[$iid_f]) && ($_[0]->[$iid_f] =~ m/^[0-9]+$/)) {
      return $_[0]->[$tag_f] . "." . $_[0]->[$iid_f];
   }

   return $_[0]->[$tag_f];
}

sub fmt {
    my $self = shift;
    return $self->name . " = \"" . $self->val . "\" (" . $self->type . ")";
}


#sub DESTROY {
#    print "SNMP::Varbind::DESTROY($_[0])\n";
#}

package SNMP::VarList;

sub new {
   my $type = shift;
   my $this = [];
   my $varb;
   foreach $varb (@_) {
     $varb = new SNMP::Varbind($varb) unless ref($varb) =~ /SNMP::Varbind/;
     push(@{$this}, $varb);
   }

   bless $this;
}

#sub DESTROY {
#    print "SNMP::VarList::DESTROY($_[0])\n";
#}

package SNMP::DEBUGGING;
# controls info/debugging output from SNMP module and libsnmp
# $SNMP::debugging == 1    =>   enables general info and warning output
#                                (eqiv. to setting $SNMP::verbose)
# $SNMP::debugging == 2    =>   enables do_debugging from libsnmp as well
# $SNMP::debugging == 3    =>   enables packet_dump from libsnmp as well
sub TIESCALAR { my $class = shift; my $val; bless \$val, $class; }

sub FETCH { ${$_[0]}; }

sub STORE {
    $SNMP::verbose = $_[1];
    SNMP::_set_debugging($_[1]>1);
    $SNMP::dump_packet = ($_[1]>2);
    ${$_[0]} = $_[1];
}

sub DELETE {
    $SNMP::verbose = 0;
    SNMP::_set_debugging(0);
    $SNMP::dump_packet = 0;
    ${$_[0]} = undef;
}

package SNMP::DEBUG_INTERNALS;		# Controls SNMP.xs debugging.
sub TIESCALAR { my $class = shift; my $val; bless \$val, $class; }

sub FETCH { ${$_[0]}; }

sub STORE {
    SNMP::_debug_internals($_[1]);
    ${$_[0]} = $_[1];
}

sub DELETE {
    SNMP::_debug_internals(0);
    ${$_[0]} = undef;
}

package SNMP::DUMP_PACKET;
# controls packet dump output from libsnmp

sub TIESCALAR { my $class = shift; my $val; bless \$val, $class; }

sub FETCH { ${$_[0]}; }

sub STORE { SNMP::_dump_packet($_[1]); ${$_[0]} = $_[1]; }

sub DELETE { SNMP::_dump_packet(0); ${$_[0]} = 0; }

package SNMP::MIB;

sub TIEHASH {
    bless {};
}

sub FETCH {
    my $this = shift;
    my $key = shift;

    if (!defined $this->{$key}) {
	tie(%{$this->{$key}}, SNMP::MIB::NODE, $key) or return undef;
    }
    $this->{$key};
}

sub STORE {
    warn "STORE(@_) : write access to the MIB not implemented\n";
}

sub DELETE {
    delete $_[0]->{$_[1]}; # just delete cache entry
}

sub FIRSTKEY { return '.1'; } # this should actually start at .0 but
                              # because nodes are not stored in lexico
                              # order in ucd-snmp node tree walk will
                              # miss most of the tree
sub NEXTKEY { # this could be sped up by using an XS __get_next_oid maybe
   my $node = $_[0]->FETCH($_[1])->{nextNode};
   $node->{objectID};
}
sub EXISTS { exists $_[0]->{$_[1]} || $_[0]->FETCH($_[1]); }
sub CLEAR { undef %{$_[0]}; } # clear the cache

package SNMP::MIB::NODE;
my %node_elements =
    (
     objectID => 0, # dotted decimal fully qualified OID
     label => 0,    # leaf textual identifier (e.g., 'sysDescr')
     subID => 0,    # leaf numeric OID component of objectID (e.g., '1')
     moduleID => 0, # textual identifier for module (e.g., 'RFC1213-MIB')
     parent => 0,   # parent node
     children => 0, # array reference of children nodes
     indexes => 0,  # returns array of column labels
     varbinds => 0, # returns array of trap/notification varbinds
     nextNode => 0, # next lexico node (BUG! does not return in lexico order)
     type => 0,     # returns simple type (see getType for values)
     access => 0,   # returns ACCESS (ReadOnly, ReadWrite, WriteOnly,
                    # NoAccess, Notify, Create)
     status => 0,   # returns STATUS (Mandatory, Optional, Obsolete,
                    # Deprecated)
     syntax => 0,   # returns 'textualConvention' if defined else 'type'
     textualConvention => 0, # returns TEXTUAL-CONVENTION
     units => 0,    # returns UNITS
     hint => 0,     # returns HINT
     enums => 0,    # returns hash ref {tag => num, ...}
     ranges => 0,   # returns array ref of hash ref [{low => num, high => num}]
     defaultValue => 0, # returns default value
     description => 0, # returns DESCRIPTION ($SNMP::save_descriptions must
                    # be set prior to MIB initialization/parsing
    );

# sub TIEHASH - implemented in SNMP.xs

# sub FETCH - implemented in SNMP.xs

sub STORE {
    warn "STORE(@_): write access to MIB node not implemented\n";
}

sub DELETE {
    warn "DELETE(@_): write access to MIB node not implemented\n";
}

sub FIRSTKEY { my $k = keys %node_elements; (each(%node_elements))[0]; }
sub NEXTKEY { (each(%node_elements))[0]; }
sub EXISTS { exists($node_elements{$_[1]}); }
sub CLEAR {
    warn "CLEAR(@_): write access to MIB node not implemented\n";
}

#sub DESTROY {
#    warn "DESTROY(@_): write access to MIB node not implemented\n";
#    # print "SNMP::MIB::NODE::DESTROY : $_[0]->{label} ($_[0])\n";
#}
package SNMP::MIB::SAVE_DESCR;

sub TIESCALAR { my $class = shift; my $val; bless \$val, $class; }

sub FETCH { ${$_[0]}; }

sub STORE { SNMP::_set_save_descriptions($_[1]); ${$_[0]} = $_[1]; }

sub DELETE { SNMP::_set_save_descriptions(0); ${$_[0]} = 0; }

package SNMP::MIB::REPLACE_NEWER;               # Controls MIB parsing

sub TIESCALAR { my $class = shift; my $val; bless \$val, $class; }

sub FETCH { ${$_[0]}; }

sub STORE {
    SNMP::_set_replace_newer($_[1]);
    ${$_[0]} = $_[1];
}

sub DELETE {
    SNMP::_set_replace_newer(0);
    ${$_[0]} = 0;
}

package SNMP;
END{SNMP::_sock_cleanup() if defined &SNMP::_sock_cleanup;}
# Autoload methods go after __END__, and are processed by the autosplit prog.

1;
__END__

=head1 NAME

SNMP - The Perl5 'SNMP' Extension Module v3.1.0 for the UCD SNMPv3 Library

=head1 SYNOPSIS

 use SNMP;
 ...
 $sess = new SNMP::Session(DestHost => localhost, Community => public);
 $val = $sess->get('sysDescr.0');
 ...
 $vars = new SNMP::VarList([sysDescr,0], [sysContact,0], [sysLocation,0]);
 @vals = $sess->get($vars);
 ...
 $vb = new SNMP::Varbind();
 do {
    $val = $sess->getnext($vb);
    print "@{$vb}\n";
 } until ($sess->{ErrorNum});
 ...
 $SNMP::save_descriptions = 1;
 SNMP::initMib(); # assuming mib is not already loaded
 print "$SNMP::MIB{sysDescr}{description}\n";

=head1 DESCRIPTION


Note: The perl SNMP 5.0 module which comes with net-snmp 5.0 and
higher is different than previous versions in a number of ways.  Most
importantly, it behaves like a proper net-snmp application and calls
init_snmp properly, which means it will read configuration files and
use those defaults where appropriate automatically parse MIB files,
etc.  This will likely affect your perl applications if you have, for
instance, default values set up in your snmp.conf file (as the perl
module will now make use of those defaults).  The docmuentation,
however, has sadly not been updated yet (aside from this note), nor is
the read_config default usage implementation fully complete.

The basic operations of the SNMP protocol are provided by this module
through an object oriented interface for modularity and ease of use.
The primary class is SNMP::Session which encapsulates the persistent
aspects of a connection between the management application and the
managed agent. Internally the class is implemented as a blessed hash
reference. This class supplies 'get', 'getnext', 'set', 'fget', and
'fgetnext' method calls. The methods take a variety of input argument
formats and support both syncronous and asyncronous operation through
a polymorphic API (i.e., method behaviour varies dependent on args
passed - see below).

=head1 SNMP::Session

$sess = new SNMP::Session(DestHost => 'host', ...)

The following arguments may be passed to new as a hash.

=over 4

=item DestHost

default 'localhost', hostname or ip addr of SNMP agent

=item Community

default 'public', SNMP community string (used for both R/W)

=item Version

default '1', [2 (same as 2c), 2c, 3]

=item RemotePort

default '161', allow remote UDP port to be overriden

=item Timeout

default '1000000', micro-seconds before retry

=item Retries

default '5', retries before failure

=item RetryNoSuch

default '0', if enabled NOSUCH errors in 'get' pdus will
be repaired, removing the varbind in error, and resent -
undef will be returned for all NOSUCH varbinds, when set
to '0' this feature is disabled and the entire get request
will fail on any NOSUCH error (applies to v1 only)

=item SecName

default 'initial', security name (v3)

=item SecLevel

default 'noAuthNoPriv', security level [noAuthNoPriv,
authNoPriv, authPriv] (v3)

=item SecEngineId

default <none>, security engineID, will be probed if not
supplied (v3)

=item ContextEngineId

default <SecEngineId>, context engineID, will be
probed if not supplied (v3)

=item Context

default '', context name (v3)

=item AuthProto

default 'MD5', authentication protocol [MD5, SHA] (v3)

=item AuthPass

default <none>, authentication passphrase

=item PrivProto

default 'DES', privacy protocol [DES] (v3)

=item PrivPass

default <none>, privacy passphrase (v3)

=item VarFormats

default 'undef', used by 'fget[next]', holds an hash
reference of output value formatters, (e.g., {<obj> =>
<sub-ref>, ... }, <obj> must match the <obj> and format
used in the get operation. A special <obj>, '*', may be
used to apply all <obj>s, the supplied sub is called to
translate the value to a new format. The sub is called
passing the Varbind as the arg

=item TypeFormats

default 'undef', used by 'fget[next]', holds an hash
reference of output value formatters, (e.g., {<type> =>
<sub-ref>, ... }, the supplied sub is called to translate
the value to a new format, unless a VarFormat mathces first
(e.g., $sess->{TypeFormats}{INTEGER} = \&mapEnum();
although this can be done more efficiently by enabling
$SNMP::use_enums or session creation param 'UseEnums')

=item UseLongNames

defaults to the value of SNMP::use_long_names at time
of session creation. set to non-zero to have <tags>
for 'getnext' methods generated preferring longer Mib name
convention (e.g., system.sysDescr vs just sysDescr)

=item UseSprintValue

defaults to the value of SNMP::use_sprint_value at time
of session creation. set to non-zero to have return values
for 'get' and 'getnext' methods formatted with the libraries
snprint_value function. This will result in certain data types
being returned in non-canonical format Note: values returned
with this option set may not be appropriate for 'set' operations
(see discussion of value formats in <vars> description section)

=item UseEnums

defaults to the value of SNMP::use_enums at time of session
creation. set to non-zero to have integer return values
converted to enumeration identifiers if possible, these values
will also be acceptable when supplied to 'set' operations

=item UseNumeric

defaults to the value of SNMP::use_numeric at time of session
creation. set to non-zero to have <tags> for get methods returned
as numeric OID's rather than descriptions.  UseLongNames will be
set so that the full OID is returned to the caller.

=item ErrorStr

read-only, holds the error message assoc. w/ last request

=item ErrorNum

read-only, holds the snmp_err or staus of last request

=item ErrorInd

read-only, holds the snmp_err_index when appropriate

=back

Private variables:

=over

=item DestAddr

internal field used to hold the translated DestHost field

=item SessPtr

internal field used to cache a created session structure

=back

=head2 SNMP::Session methods

=over

=item $sess->update(E<lt>fieldsE<gt>)

Updates the SNMP::Session object with the values fields
passed in as a hash list (similar to new(E<lt>fieldsE<gt>))
B<(WARNING! not fully implemented)>

=item $sess->get(E<lt>varsE<gt> [,E<lt>callbackE<gt>])

do SNMP GET, multiple <vars> formats accepted.
for syncronous operation <vars> will be updated
with value(s) and type(s) and will also return
retrieved value(s). If <callback> supplied method
will operate asyncronously

=item $sess->fget(E<lt>varsE<gt> [,E<lt>callbackE<gt>])

do SNMP GET like 'get' and format the values according
the handlers specified in $sess->{VarFormats} and
$sess->{TypeFormats}

=item $sess->getnext(E<lt>varsE<gt> [,E<lt>callbackE<gt>])

do SNMP GETNEXT, multiple <vars> formats accepted,
returns retrieved value(s), <vars> passed as arguments are
updated to indicate next lexicographical <obj>,<iid>,<val>,
and <type>

Note: simple string <vars>,(e.g., 'sysDescr.0')
form is not updated. If <callback> supplied method
will operate asyncronously

=item $sess->fgetnext(E<lt>varsE<gt> [,E<lt>callbackE<gt>])

do SNMP GETNEXT like getnext and format the values according
the handlers specified in $sess->{VarFormats} and
$sess->{TypeFormats}

=item $sess->set(E<lt>varsE<gt> [,E<lt>callbackE<gt>])

do SNMP SET, multiple <vars> formats accepted.
the value field in all <vars> formats must be in a canonical
format (i.e., well known format) to ensure unambiguous
translation to SNMP MIB data value (see discussion of
canonical value format <vars> description section),
returns snmp_errno. If <callback> supplied method
will operate asyncronously

=item $sess->getbulk(E<lt>non-repeatersE<gt>, E<lt>max-repeatersE<gt>, E<lt>varsE<gt>)

do an SNMP GETBULK, from the list of Varbinds, the single
next lexico instance is fetched for the first n Varbinds
as defined by <non-repeaters>. For remaining Varbinds,
the m lexico instances are retrieved each of the remaining
Varbinds, where m is <max-repeaters>.

=item $sess->bulkwalk(E<lt>non-repeatersE<gt>, E<lt>max-repeatersE<gt>, E<lt>varsE<gt> [,E<lt>callbackE<gt>])

Do a "bulkwalk" of the list of Varbinds.  This is done by
sending a GETBULK request (see getbulk() above) for the
Varbinds.  For each requested variable, the response is
examined to see if the next lexico instance has left the
requested sub-tree.  Any further instances returned for
this variable are ignored, and the walk for that sub-tree
is considered complete.

If any sub-trees were not completed when the end of the
responses is reached, another request is composed, consisting
of the remaining variables.  This process is repeated until
all sub-trees have been completed, or too many packets have
been exchanged (to avoid loops).

The bulkwalk() method returns an array containing an array of
Varbinds, one for each requested variable, in the order of the
variable requests.  Upon error, bulkwalk() returns undef and
sets $sess->ErrorStr and $sess->ErrorNum.  If a callback is
supplied, bulkwalk() returns the SNMP request id, and returns
immediately.  The callback will be called with the supplied
argument list and the returned variables list.

Note: Because the client must "discover" that the tree is
complete by comparing the returned variables with those that
were requested, there is a potential "gotcha" when using the
max-repeaters value.  Consider the following code to print a
list of interfaces and byte counts:

    $numInts = $sess->get('ifNumber.0');
    ($desc, $in, $out) = $sess->bulkwalk(0, $numInts,
		  [['ifDescr'], ['ifInOctets'], ['ifOutOctets']]);

    for $i (0..($numInts - 1)) {
        printf "Interface %4s: %s inOctets, %s outOctets\n",
                  $$desc[$i]->val, $$in[$i]->val, $$out[$i]->val;
    }

This code will produce *two* requests to the agent -- the first
to get the interface values, and the second to discover that all
the information was in the first packet.  To get around this,
use '$numInts + 1' for the max_repeaters value.  This asks the
agent to include one additional (unrelated) variable that signals
the end of the sub-tree, allowing bulkwalk() to determine that
the request is complete.

=back

=head1 SNMP::TrapSession

$sess = new SNMP::Session(DestHost => 'host', ...)

supports all applicable fields from SNMP::Session
(see above)

=head2 SNMP::TrapSession methods

=over

=item $sess->trap(enterprise, agent, generic, specific, uptime, <vars>)

    $sess->trap(enterprise=>'.1.3.6.1.4.1.2021', # or 'ucdavis' [default]
                agent => '127.0.0.1', # or 'localhost',[dflt 1st intf on host]
                generic => specific,  # can be omitted if 'specific' supplied
                specific => 5,        # can be omitted if 'generic' supplied
                uptime => 1234,       # dflt to localhost uptime (0 on win32)
                [[ifIndex, 1, 1],[sysLocation, 0, "here"]]); # optional vars
                                                             # always last

=item trap(oid, uptime, <vars>) - v2 format

    $sess->trap(oid => 'snmpRisingAlarm',
                uptime => 1234,
                [[ifIndex, 1, 1],[sysLocation, 0, "here"]]); # optional vars
                                                             # always last

=back

=head1 Acceptable variable formats:

<vars> may be one of the following forms:

=over

=item SNMP::VarList

represents an array of MIB objects to get or set,
implemented as a blessed reference to an array of
SNMP::Varbinds, (e.g., [<varbind1>, <varbind2>, ...])

=item SNMP::Varbind

represents a single MIB object to get or set, implemented as
a blessed reference to a 4 element array;
[<obj>, <iid>, <val>, <type>].

=over

=item <obj>

one of the following forms:

=over

=item 1)

leaf identifier (e.g., 'sysDescr') assumed to be
unique for practical purposes

=item 2)

fully qualified identifier (e.g.,
'.iso.org.dod.internet.mgmt.mib-2.system.sysDescr')

=item 3)

fully qualified, dotted-decimal, numeric OID (e.g.,
'.1.3.6.1.2.1.1.1')

=back

=item <iid>

the dotted-decimal, instance identifier. for
scalar MIB objects use '0'

=item <val>

the SNMP data value retrieved from or being set
to the agents MIB. for (f)get(next) operations
<val> may have a variety of formats as determined by
session and package settings. However for set
operations the <val> format must be canonical to
ensure unambiguous translation. The canonical forms
are as follows:

=over

=item OBJECTID

dotted-decimal (e.g., .1.3.6.1.2.1.1.1)

=item OCTETSTR

perl scalar containing octets

=item INTEGER

decimal signed integer (or enum)

=item NETADDR

dotted-decimal

=item IPADDR

dotted-decimal

=item COUNTER

decimal unsigned integer

=item COUNTER64

decimal unsigned integer

=item GAUGE

decimal unsigned integer

=item UINTEGER

decimal unsigned integer

=item TICKS

decimal unsigned integer

=item OPAQUE

perl scalar containing octets

=item NULL

perl scalar containing nothing

=back

=item <type>

SNMP data type (see list above), this field is
populated by 'get' and 'getnext' operations. In
some cases the programmer needs to populate this
field when passing to a 'set' operation. this
field need not be supplied when the attribute
indicated by <tag> is already described by loaded
Mib modules. for 'set's, if a numeric OID is used
and the object is not currently in the loaded Mib,
the <type> field must be supplied

=back

=item simple string

light weight form of <var> used to 'set' or 'get' a
single attribute without constructing an SNMP::Varbind.
stored in a perl scalar, has the form '<tag>.<iid>',
(e.g., 'sysDescr.0'). for 'set' operations the value
is passed as a second arg. Note: This argument form is
not updated in get[next] operations as are the other forms.

=back

=head1 Acceptable callback formats

<callback> may be one of the following forms:

=over

=item without arguments

=over

=item \&subname

=item sub { ... }

=back

=item or with arguments

=over

=item [ \&subname, $arg1, ... ]

=item [ sub { ... }, $arg1, ... ]

=item [ "method", $obj, $arg1, ... ]

=back

=back

callback will be called when response is received or timeout
occurs. the last argument passed to callback will be a
SNMP::VarList reference. In case of timeout the last argument
will be undef.

=over

=item &SNMP::MainLoop([<timeout>, [<callback>]])

to be used with async SNMP::Session
calls. MainLoop must be called after initial async calls
so return packets from the agent will not be processed.
If no args suplied this function enters an infinite loop
so program must be exited in a callback or externally
interupted. If <timeout(sic)

=item &SNMP::finish()

This function, when called from an SNMP::MainLoop() callback
function, will cause the current SNMP::MainLoop() to return
after the callback is completed.  finish() can be used to
terminate an otherwise-infinite MainLoop.  A new MainLoop()
instance can then be started to handle further requests.

=back

=head1 SNMP package variables and functions

=over

=item $SNMP::VERSION

the current version specifier (e.g., 3.1.0)

=item $SNMP::auto_init_mib

default '1', set to 0 to disable automatic reading
of the MIB upon session creation. set to non-zero
to call initMib at session creation which will result
in MIB loading according to UCD env. variables (see
man mib_api)

=item $SNMP::verbose

default '0', controls warning/info output of
SNMP module, 0 => no output, 1 => enables warning/info
output from SNMP module itself (is also controlled
by SNMP::debugging - see below)

=item $SNMP::use_long_names

default '0', set to non-zero to enable the use of
longer Mib identifiers. see translateObj. will also
influence the formatting of <tag> in varbinds returned
from 'getnext' operations. Can be set on a per session
basis (UseLongNames)

=item $SNMP::use_sprint_value

default '0', set to non-zero to enable formatting of
response values using the snmp libraries snprint_value
function. can also be set on a per session basis (see
UseSprintValue) Note: returned values may not be
suitable for 'set' operations

=item $SNMP::use_enums

default '0',set non-zero to return values as enums and
allow sets using enums where appropriate. integer data
will still be accepted for set operations. can also be
set on a per session basis (see UseEnums)

=item $SNMP::use_numeric

default to '0',set to non-zero to have <tags> for 'get'
methods returned as numeric OID's rather than descriptions.
UseLongNames will be set so that the entire OID will be
returned.  Set on a per-session basis (see UseNumeric).

=item $SNMP::save_descriptions

default '0',set non-zero to have mib parser save
attribute descriptions. must be set prior to mib
initialization

=item $SNMP::debugging

default '0', controlls debugging output level
within SNMP module and libsnmp

=over

=item 1

enables 'SNMP::verbose' (see above)

=item 2

level 1 plus snmp_set_do_debugging(1)

=item 3

level 2 plus snmp_set_dump_packet(1)

=back

=item $SNMP::dump_packet

default '0', set [non-]zero to independently set
snmp_set_dump_packet()

=back

=head1 %SNMP::MIB

a tied hash to access parsed MIB information. After
the MIB has been loaded this hash allows access to
to the parsed in MIB meta-data(the structure of the
MIB (i.e., schema)). The hash returns blessed
references to SNMP::MIB::NODE objects which represent
a single MIB attribute. The nodes can be fetched with
multiple 'key' formats - the leaf name (e.g.,sysDescr)
or fully/partially qualified name (e.g.,
system.sysDescr) or fully qualified numeric OID. The
returned node object supports the following fields:

=over

=item objectID

dotted decimal fully qualified OID

=item label

leaf textual identifier (e.g., 'sysDescr')

=item subID

leaf numeric OID component of objectID (e.g., '1')

=item moduleID

textual identifier for module (e.g., 'RFC1213-MIB')

=item parent

parent node

=item children

array reference of children nodes

=item nextNode

next lexico node B<(BUG!does not return in lexico order)>

=item type

returns application type (see getType for values)

=item access

returns ACCESS (ReadOnly, ReadWrite, WriteOnly,
NoAccess, Notify, Create)

=item status

returns STATUS (Mandatory, Optional, Obsolete,
Deprecated)

=item syntax

returns 'textualConvention' if defined else 'type'

=item textualConvention

returns TEXTUAL-CONVENTION

=item units

returns UNITS

=item hint

returns HINT

=item enums

returns hash ref {tag => num, ...}

=item ranges

returns array ref of hash ref [{low => num, high => num}, ...]

=item description

returns DESCRIPTION ($SNMP::save_descriptions must
be set prior to MIB initialization/parsing)

=back

=head1 MIB Functions

=over

=item &SNMP::setMib(<file>)

allows dynamic parsing of the mib and explicit
specification of mib file independent of enviroment
variables. called with no args acts like initMib,
loading MIBs indicated by environment variables (see
ucd mib_api docs). passing non-zero second arg
forces previous mib to be freed and replaced
B<(Note: second arg not working since freeing previous
Mib is more involved than before)>.

=item &SNMP::initMib()

calls library init_mib function if Mib not already
loaded - does nothing if Mib already loaded. will
parse directories and load modules according to
environment variables described in UCD documentations.
(see man mib_api, MIBDIRS, MIBS, MIBFILE(S), etc.)

=item &SNMP::addMibDirs(<dir>,...)

calls library add_mibdir for each directory
supplied. will cause directory(s) to be added to
internal list and made available for searching in
subsequent loadModules calls

=item &SNMP::addMibFiles(<file>,...)

calls library read_mib function. The file(s)
supplied will be read and all Mib module definitions
contained therein will be added to internal mib tree
structure

=item &SNMP::loadModules(<mod>,...)

calls library read_module function. The
module(s) supplied will be searched for in the
current mibdirs and and added to internal mib tree
structure. Passing special <mod>, 'ALL', will cause
all known modules to be loaded.

=item &SNMP::unloadModules(<mod>,...)

B<*Not Implemented*>

=item &SNMP::translateObj(<var>[,arg])

will convert a text obj tag to an OID and
vice-versa. any iid suffix is retained numerically.
default behaviour when converting a numeric OID
to text form is to return leaf indentifier only
(e.g.,'sysDescr') but when $SNMP::use_long_names
is non-zero or a non-zero second arg is supplied
will return longer textual identifier. If no Mib
is loaded when called and $SNMP::auto_init_mib is
enabled then the Mib will be loaded. Will return
'undef' upon failure.

=item &SNMP::getType(<var>)

return SNMP data type for given textual identifier
OBJECTID, OCTETSTR, INTEGER, NETADDR, IPADDR, COUNTER
GAUGE, TIMETICKS, OPAQUE, or undef

=item &SNMP::mapEnum(<var>)

converts integer value to enumertion tag defined
in Mib or converts tag to integer depending on
input. the function will return the corresponding
integer value *or* tag for a given MIB attribute
and value. The function will sense which direction
to perform the conversion. Various arg formats are
supported

=over

=item $val = SNMP::mapEnum($varbind);

where $varbind is SNMP::Varbind or equiv.
note: $varbind will be updated

=item $val = SNMP::mapEnum('ipForwarding', 'forwarding');

=item $val = SNMP::mapEnum('ipForwarding', 1);

=back

=back

=head1 Exported SNMP utility functions

Note: utility functions do not support async operation yet.

=over

=item &snmp_get()

takes args of SNMP::Session::new followed by those of
SNMP::Session::get

=item &snmp_getnext()

takes args of SNMP::Session::new followed by those of
SNMP::Session::getnext

=item &snmp_set()

takes args of SNMP::Session::new followed by those of
SNMP::Session::set

=item &snmp_trap()

takes args of SNMP::TrapSession::new followed by those of
SNMP::TrapSession::trap

=back

=head1 Trouble Shooting

If problems occur there are number areas to look at to narrow down the
possibilities.

The first step should be to test the UCD SNMP installation
independently from the Perl5 SNMP interface.

Try running the apps from the UCD SNMP distribution.

Make sure your agent (snmpd) is running and properly configured with
read-write access for the community you are using.

Ensure that your MIBs are installed and enviroment variables are set
appropriately (see man mib_api)

Be sure to remove old ucd-snmp installations and ensure headers and
libraries from old CMU installations are not being used by mistake.

If the problem occurs during compilation/linking check that the snmp
library being linked is actually the UCD SNMP library (there have been
name conflicts with existing snmp libs).

Also check that the header files are correct and up to date.

Sometimes compiling the UCD SNMP library with
'position-independent-code' enabled is required (HPUX specifically).

If you cannot resolve the problem you can post to
comp.lang.perl.modules or
net-snmp-users@net-snmp-users@lists.sourceforge.net

please give sufficient information to analyze the problem (OS type,
versions for OS/Perl/UCD/compiler, complete error output, etc.)

=head1 Acknowledments

Many thanks to all those who supplied patches, suggestions and
feedback.

 Joe Marzot (the original author)
 Wes Hardaker and the net-snmp-coders
 Dave Perkins
 Marcel Wiget
 David Blackburn
 John Stofell
 Gary Hayward
 Claire Harrison
 Achim Bohnet
 Doug Kingston
 Jacques Vidrine
 Carl Jacobsen
 Wayne Marquette
 Scott Schumate
 Michael Slifcak
 Perl5 Porters

Apologies to any/all who's patch/feature/request was not mentioned or
included - most likely it was lost when paying work intruded on my
fun. Please try again if you do not see a desired feature. This may
actually turn out to be a decent package with such excellent help and
the fact that I have more time to work on it than in the past.

=head1 AUTHOR

bugs, comments, questions to net-snmp-users@lists.sourceforge.net

=head1 Copyright

     Copyright (c) 1995-2000 G. S. Marzot. All rights reserved.
     This program is free software; you can redistribute it and/or
     modify it under the same terms as Perl itself.

     Copyright (c) 2001-2002 Networks Associates Technology, Inc.  All
     Rights Reserved.  This program is free software; you can
     redistribute it and/or modify it under the same terms as Perl
     itself.
=cut
