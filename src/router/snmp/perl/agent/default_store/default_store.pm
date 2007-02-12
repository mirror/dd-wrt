package NetSNMP::agent::default_store;

use strict;
use Carp;

require Exporter;
require DynaLoader;
use AutoLoader;

use vars qw(@ISA %EXPORT_TAGS @EXPORT_OK @EXPORT $VERSION $AUTOLOAD);

@ISA = qw(Exporter DynaLoader);

# Items to export into callers namespace by default. Note: do not export
# names by default without a very good reason. Use EXPORT_OK instead.
# Do not simply export all your public functions/methods/constants.

# This allows declaration	use NetSNMP::agent::default_store ':all';
# If you do not need this, moving things directly into @EXPORT or @EXPORT_OK
# will save memory.
%EXPORT_TAGS = ( 'all' => [ qw(
				   NETSNMP_DS_AGENT_VERBOSE
				   NETSNMP_DS_AGENT_ROLE
				   NETSNMP_DS_AGENT_NO_ROOT_ACCESS
				   NETSNMP_DS_AGENT_AGENTX_MASTER
				   NETSNMP_DS_AGENT_QUIT_IMMEDIATELY
				   NETSNMP_DS_AGENT_DISABLE_PERL
				   NETSNMP_DS_AGENT_PROGNAME
				   NETSNMP_DS_AGENT_X_SOCKET
				   NETSNMP_DS_AGENT_PORTS
				   NETSNMP_DS_AGENT_INTERNAL_SECNAME
				   NETSNMP_DS_AGENT_PERL_INIT_FILE
				   NETSNMP_DS_AGENT_FLAGS
				   NETSNMP_DS_AGENT_USERID
				   NETSNMP_DS_AGENT_GROUPID
				   NETSNMP_DS_AGENT_AGENTX_PING_INTERVAL
) ] );

@EXPORT_OK = ( @{ $EXPORT_TAGS{'all'} } );

@EXPORT = qw(
				   NETSNMP_DS_AGENT_VERBOSE
				   NETSNMP_DS_AGENT_ROLE
				   NETSNMP_DS_AGENT_NO_ROOT_ACCESS
				   NETSNMP_DS_AGENT_AGENTX_MASTER
				   NETSNMP_DS_AGENT_QUIT_IMMEDIATELY
				   NETSNMP_DS_AGENT_DISABLE_PERL
				   NETSNMP_DS_AGENT_PROGNAME
				   NETSNMP_DS_AGENT_X_SOCKET
				   NETSNMP_DS_AGENT_PORTS
				   NETSNMP_DS_AGENT_INTERNAL_SECNAME
				   NETSNMP_DS_AGENT_PERL_INIT_FILE
				   NETSNMP_DS_AGENT_FLAGS
				   NETSNMP_DS_AGENT_USERID
				   NETSNMP_DS_AGENT_GROUPID
				   NETSNMP_DS_AGENT_AGENTX_PING_INTERVAL
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
	    croak "Your vendor has not defined NetSNMP::agent::default_store macro $constname";
	}
    }
    {
	no strict 'refs';
	# Fixed between 5.005_53 and 5.005_61
# 	if ($] >= 5.00561) {
# 	    *$AUTOLOAD = sub () { $val };
# 	}
# 	else {
	    *$AUTOLOAD = sub { $val };
# 	}
    }
    goto &$AUTOLOAD;
}

bootstrap NetSNMP::agent::default_store $VERSION;

# Preloaded methods go here.

# Autoload methods go after =cut, and are processed by the autosplit program.

1;
__END__
# Below is stub documentation for your module. You better edit it!

=head1 NAME

NetSNMP::agent::default_store - Perl extension for blah blah blah

=head1 SYNOPSIS

  use NetSNMP::agent::default_store;
  blah blah blah

=head1 DESCRIPTION

Stub documentation for NetSNMP::agent::default_store, created by h2xs. It looks like the
author of the extension was negligent enough to leave the stub
unedited.

Blah blah blah.

=head2 EXPORT

None by default.

=head2 Exportable constants

				   NETSNMP_DS_AGENT_VERBOSE
				   NETSNMP_DS_AGENT_ROLE
				   NETSNMP_DS_AGENT_NO_ROOT_ACCESS
				   NETSNMP_DS_AGENT_AGENTX_MASTER
				   NETSNMP_DS_AGENT_QUIT_IMMEDIATELY
				   NETSNMP_DS_AGENT_DISABLE_PERL
				   NETSNMP_DS_AGENT_PROGNAME
				   NETSNMP_DS_AGENT_X_SOCKET
				   NETSNMP_DS_AGENT_PORTS
				   NETSNMP_DS_AGENT_INTERNAL_SECNAME
				   NETSNMP_DS_AGENT_PERL_INIT_FILE
				   NETSNMP_DS_AGENT_FLAGS
				   NETSNMP_DS_AGENT_USERID
				   NETSNMP_DS_AGENT_GROUPID
				   NETSNMP_DS_AGENT_AGENTX_PING_INTERVAL


  DS_AGENT_AGENTX_MASTER
  DS_AGENT_AGENTX_PING_INTERVAL
  DS_AGENT_FLAGS
  DS_AGENT_GROUPID
  DS_AGENT_H
  DS_AGENT_INTERNAL_SECNAME
  DS_AGENT_NO_ROOT_ACCESS
  DS_AGENT_PORTS
  DS_AGENT_PROGNAME
  DS_AGENT_ROLE
  DS_AGENT_USERID
  DS_AGENT_VERBOSE
  DS_AGENT_X_SOCKET


=head1 AUTHOR

A. U. Thor, a.u.thor@a.galaxy.far.far.away

=head1 SEE ALSO

perl(1).

=cut
