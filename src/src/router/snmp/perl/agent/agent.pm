package NetSNMP::agent;

use strict;
use Carp;

require Exporter;
require DynaLoader;
use AutoLoader;

use NetSNMP::default_store (':all');
use NetSNMP::agent::default_store (':all');
use NetSNMP::OID (':all');
use NetSNMP::agent::netsnmp_request_infoPtr;

use vars qw(@ISA %EXPORT_TAGS @EXPORT_OK @EXPORT $VERSION $AUTOLOAD);

@ISA = qw(Exporter AutoLoader DynaLoader);

# Items to export into callers namespace by default. Note: do not export
# names by default without a very good reason. Use EXPORT_OK instead.
# Do not simply export all your public functions/methods/constants.

# This allows declaration	use NetSNMP::agent ':all';
# If you do not need this, moving things directly into @EXPORT or @EXPORT_OK
# will save memory.
%EXPORT_TAGS = ( 'all' => [ qw(
	MODE_GET
	MODE_GETBULK
	MODE_GETNEXT
	MODE_SET_ACTION
	MODE_SET_BEGIN
	MODE_SET_COMMIT
	MODE_SET_FREE
	MODE_SET_RESERVE1
	MODE_SET_RESERVE2
	MODE_SET_UNDO
) ] );

@EXPORT_OK = ( @{ $EXPORT_TAGS{'all'} } );

@EXPORT = qw(
	MODE_GET
	MODE_GETBULK
	MODE_GETNEXT
	MODE_SET_ACTION
	MODE_SET_BEGIN
	MODE_SET_COMMIT
	MODE_SET_FREE
	MODE_SET_RESERVE1
	MODE_SET_RESERVE2
	MODE_SET_UNDO
);
$VERSION = '0.01';

sub AUTOLOAD {
    # This AUTOLOAD is used to 'autoload' constants from the constant()
    # XS function.  If a constant is not found then control is passed
    # to the AUTOLOAD in AutoLoader.

    my $constname;
    ($constname = $AUTOLOAD) =~ s/.*:://;
    croak "& not defined" if $constname eq 'constant';
    my $val = constant($constname, @_ ? $_[0] : 0);
    if ($! != 0) {
	if ($! =~ /Invalid/ || $!{EINVAL}) {
	    $AutoLoader::AUTOLOAD = $AUTOLOAD;
	    goto &AutoLoader::AUTOLOAD;
	}
	else {
	    croak "Your vendor has not defined NetSNMP::agent macro $constname";
	}
    }
    {
	no strict 'refs';
	# Fixed between 5.005_53 and 5.005_61
#	if ($] >= 5.00561) {
#	    *$AUTOLOAD = sub () { $val };
#	}
#	else {
	    *$AUTOLOAD = sub { $val };
#	}
    }
    goto &$AUTOLOAD;
}

{
    my $haveinit = 0;

    sub mark_init_agent_done {
	$haveinit = 1;
    }

    sub maybe_init_agent {
	return if ($haveinit);
	$haveinit = 1;

	snmp_enable_stderrlog();
	my $flags = $_[0];
	if ($flags->{'AgentX'}) {
	    netsnmp_ds_set_boolean(NETSNMP_DS_APPLICATION_ID, NETSNMP_DS_AGENT_ROLE, 1);
	}
	init_agent($flags->{'Name'} || "perl");
	if ($flags->{'Ports'}) {
	    netsnmp_ds_set_string(NETSNMP_DS_APPLICATION_ID, NETSNMP_DS_AGENT_PORTS, $flags->{'Ports'});
	}
	init_mib();
    }
}

{
    my $haveinit = 0;

    sub mark_init_lib_done {
	$haveinit = 1;
    }

    sub maybe_init_lib {
	return if ($haveinit);
	$haveinit = 1;

	my $flags = $_[0];
	init_snmp($flags->{'Name'} || "perl");
	if (netsnmp_ds_get_boolean(NETSNMP_DS_APPLICATION_ID, NETSNMP_DS_AGENT_ROLE) != 1) {
	    init_master_agent();
	}
    }
}

sub new {
    my $type = shift;
    my ($self);
    %$self = @_;
    bless($self, $type);
    if ($self->{'dont_init_agent'}) {
	$self->mark_init_agent_done();
    } else {
	$self->maybe_init_agent();
    }
    if ($self->{'dont_init_lib'}) {
	$self->mark_init_lib_done();
    }
    return $self;
}

sub register($$$$) {
    my ($self, $name, $oid, $sub) = @_;
    my $reg = NetSNMP::agent::netsnmp_handler_registration::new($name, $oid, $sub);
    $reg->register() if ($reg);
    return $reg;
}

sub main_loop {
    my $self = shift;
    while(1) {
	$self->agent_check_and_process(1);
    }
}

sub agent_check_and_process {
    my ($self, $blocking) = @_;
    $self->maybe_init_lib();
    __agent_check_and_process($blocking || 0);
}

bootstrap NetSNMP::agent $VERSION;

# Preloaded methods go here.

# Autoload methods go after =cut, and are processed by the autosplit program.

1;
__END__
# Below is stub documentation for your module. You better edit it!

=head1 NAME

NetSNMP::agent - Perl extension for the net-snmp agent.

=head1 SYNOPSIS

  use NetSNMP::agent;
  my $agent = new NetSNMP::agent('Name' -> 'my_agent_name');
  $agent->register("a_name", ".1.3.6.1.2.1", \&myhandler);
  $agent->main_loop();

    --- or, within the net-snmp snmpd.conf file: ---

  perl $agent->register("a_name", ".1.3.6.1.2.1", \&myhandler);

=head1 DESCRIPTION

This module implements a snmp agent and/or can be embedded within the
net-snmp agent.

=head1 AUTHOR

Please mail the net-snmp-users@lists.sourceforge.net mailing list for
help, questions or comments about this module.

Wes Hardaker, hardaker@users.sourceforge.net

=head1 SEE ALSO

perl(1).

=cut
