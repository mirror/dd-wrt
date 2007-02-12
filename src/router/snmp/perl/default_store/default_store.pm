package NetSNMP::default_store;

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

# This allows declaration	use NetSNMP::default_store ':all';
# If you do not need this, moving things directly into @EXPORT or @EXPORT_OK
# will save memory.
%EXPORT_TAGS = ( 'all' => [ qw(
				   NETSNMP_DS_MAX_IDS
				   NETSNMP_DS_MAX_SUBIDS
				   NETSNMP_DS_LIBRARY_ID
				   NETSNMP_DS_APPLICATION_ID
				   NETSNMP_DS_TOKEN_ID
				   NETSNMP_DS_LIB_MIB_ERRORS
				   NETSNMP_DS_LIB_SAVE_MIB_DESCRS
				   NETSNMP_DS_LIB_MIB_COMMENT_TERM
				   NETSNMP_DS_LIB_MIB_PARSE_LABEL
				   NETSNMP_DS_LIB_DUMP_PACKET
				   NETSNMP_DS_LIB_LOG_TIMESTAMP
				   NETSNMP_DS_LIB_DONT_READ_CONFIGS
				   NETSNMP_DS_LIB_MIB_REPLACE
				   NETSNMP_DS_LIB_PRINT_NUMERIC_ENUM
				   NETSNMP_DS_LIB_PRINT_NUMERIC_OIDS
				   NETSNMP_DS_LIB_DONT_BREAKDOWN_OIDS
				   NETSNMP_DS_LIB_ALARM_DONT_USE_SIG
				   NETSNMP_DS_LIB_PRINT_FULL_OID
				   NETSNMP_DS_LIB_QUICK_PRINT
				   NETSNMP_DS_LIB_RANDOM_ACCESS
				   NETSNMP_DS_LIB_REGEX_ACCESS
				   NETSNMP_DS_LIB_DONT_CHECK_RANGE
				   NETSNMP_DS_LIB_NO_TOKEN_WARNINGS
				   NETSNMP_DS_LIB_NUMERIC_TIMETICKS
				   NETSNMP_DS_LIB_ESCAPE_QUOTES
				   NETSNMP_DS_LIB_REVERSE_ENCODE
				   NETSNMP_DS_LIB_PRINT_BARE_VALUE
				   NETSNMP_DS_LIB_EXTENDED_INDEX
				   NETSNMP_DS_LIB_PRINT_HEX_TEXT
				   NETSNMP_DS_LIB_PRINT_UCD_STYLE_OID
				   NETSNMP_DS_LIB_READ_UCD_STYLE_OID
				   NETSNMP_DS_LIB_HAVE_READ_PREMIB_CONFIG
				   NETSNMP_DS_LIB_HAVE_READ_CONFIG
				   NETSNMP_DS_LIB_QUICKE_PRINT
				   NETSNMP_DS_LIB_MIB_WARNINGS
				   NETSNMP_DS_LIB_SECLEVEL
				   NETSNMP_DS_LIB_SNMPVERSION
				   NETSNMP_DS_LIB_DEFAULT_PORT
				   NETSNMP_DS_LIB_OID_OUTPUT_FORMAT
				   NETSNMP_DS_SNMP_VERSION_1
				   NETSNMP_DS_SNMP_VERSION_2c
				   NETSNMP_DS_SNMP_VERSION_3
				   NETSNMP_DS_LIB_SECNAME
				   NETSNMP_DS_LIB_CONTEXT
				   NETSNMP_DS_LIB_PASSPHRASE
				   NETSNMP_DS_LIB_AUTHPASSPHRASE
				   NETSNMP_DS_LIB_PRIVPASSPHRASE
				   NETSNMP_DS_LIB_OPTIONALCONFIG
				   NETSNMP_DS_LIB_APPTYPE
				   NETSNMP_DS_LIB_COMMUNITY
				   NETSNMP_DS_LIB_PERSISTENT_DIR
				   NETSNMP_DS_LIB_CONFIGURATION_DIR
				   NETSNMP_DS_LIB_SECMODEL
				   NETSNMP_DS_LIB_MIBDIRS
	netsnmp_ds_get_boolean
	netsnmp_ds_get_int
	netsnmp_ds_get_string
	netsnmp_ds_get_void
	netsnmp_ds_register_config
	netsnmp_ds_register_premib
	netsnmp_ds_set_boolean
	netsnmp_ds_set_int
	netsnmp_ds_set_string
	netsnmp_ds_set_void
	netsnmp_ds_shutdown
	netsnmp_ds_toggle_boolean
) ] );

@EXPORT_OK = ( @{ $EXPORT_TAGS{'all'} } );

@EXPORT = qw(
				   NETSNMP_DS_MAX_IDS
				   NETSNMP_DS_MAX_SUBIDS
				   NETSNMP_DS_LIBRARY_ID
				   NETSNMP_DS_APPLICATION_ID
				   NETSNMP_DS_TOKEN_ID
				   NETSNMP_DS_LIB_MIB_ERRORS
				   NETSNMP_DS_LIB_SAVE_MIB_DESCRS
				   NETSNMP_DS_LIB_MIB_COMMENT_TERM
				   NETSNMP_DS_LIB_MIB_PARSE_LABEL
				   NETSNMP_DS_LIB_DUMP_PACKET
				   NETSNMP_DS_LIB_LOG_TIMESTAMP
				   NETSNMP_DS_LIB_DONT_READ_CONFIGS
				   NETSNMP_DS_LIB_MIB_REPLACE
				   NETSNMP_DS_LIB_PRINT_NUMERIC_ENUM
				   NETSNMP_DS_LIB_PRINT_NUMERIC_OIDS
				   NETSNMP_DS_LIB_DONT_BREAKDOWN_OIDS
				   NETSNMP_DS_LIB_ALARM_DONT_USE_SIG
				   NETSNMP_DS_LIB_PRINT_FULL_OID
				   NETSNMP_DS_LIB_QUICK_PRINT
				   NETSNMP_DS_LIB_RANDOM_ACCESS
				   NETSNMP_DS_LIB_REGEX_ACCESS
				   NETSNMP_DS_LIB_DONT_CHECK_RANGE
				   NETSNMP_DS_LIB_NO_TOKEN_WARNINGS
				   NETSNMP_DS_LIB_NUMERIC_TIMETICKS
				   NETSNMP_DS_LIB_ESCAPE_QUOTES
				   NETSNMP_DS_LIB_REVERSE_ENCODE
				   NETSNMP_DS_LIB_PRINT_BARE_VALUE
				   NETSNMP_DS_LIB_EXTENDED_INDEX
				   NETSNMP_DS_LIB_PRINT_HEX_TEXT
				   NETSNMP_DS_LIB_PRINT_UCD_STYLE_OID
				   NETSNMP_DS_LIB_READ_UCD_STYLE_OID
				   NETSNMP_DS_LIB_HAVE_READ_PREMIB_CONFIG
				   NETSNMP_DS_LIB_HAVE_READ_CONFIG
				   NETSNMP_DS_LIB_QUICKE_PRINT
				   NETSNMP_DS_LIB_MIB_WARNINGS
				   NETSNMP_DS_LIB_SECLEVEL
				   NETSNMP_DS_LIB_SNMPVERSION
				   NETSNMP_DS_LIB_DEFAULT_PORT
				   NETSNMP_DS_LIB_OID_OUTPUT_FORMAT
				   NETSNMP_DS_SNMP_VERSION_1
				   NETSNMP_DS_SNMP_VERSION_2c
				   NETSNMP_DS_SNMP_VERSION_3
				   NETSNMP_DS_LIB_SECNAME
				   NETSNMP_DS_LIB_CONTEXT
				   NETSNMP_DS_LIB_PASSPHRASE
				   NETSNMP_DS_LIB_AUTHPASSPHRASE
				   NETSNMP_DS_LIB_PRIVPASSPHRASE
				   NETSNMP_DS_LIB_OPTIONALCONFIG
				   NETSNMP_DS_LIB_APPTYPE
				   NETSNMP_DS_LIB_COMMUNITY
				   NETSNMP_DS_LIB_PERSISTENT_DIR
				   NETSNMP_DS_LIB_CONFIGURATION_DIR
				   NETSNMP_DS_LIB_SECMODEL
				   NETSNMP_DS_LIB_MIBDIRS
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
	    croak "Your vendor has not defined NetSNMP::default_store macro $constname";
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

bootstrap NetSNMP::default_store $VERSION;

# Preloaded methods go here.

# Autoload methods go after =cut, and are processed by the autosplit program.

1;
__END__
# Below is stub documentation for your module. You better edit it!

=head1 NAME

NetSNMP::default_store - Perl extension for blah blah blah

=head1 SYNOPSIS

  use NetSNMP::default_store;
  $port = netsnmp_ds_get_int(NETSNMP_DS_LIBRARY_ID, DS_LIB_DEFAULT_PORT);
  netsnmp_ds_set_int(NETSNMP_DS_LIBRARY_ID, DS_LIB_DEFAULT_PORT, 161);

=head1 DESCRIPTION

This module is a wrapper around the net-snmp default store routines.
See the net-snmp default_store manual page for details on what the
various functions do and the values that can be set/retrieved.

=head2 EXPORT

None by default.

=head2 Exportable constants

				   NETSNMP_DS_MAX_IDS
				   NETSNMP_DS_MAX_SUBIDS
				   NETSNMP_DS_LIBRARY_ID
				   NETSNMP_DS_APPLICATION_ID
				   NETSNMP_DS_TOKEN_ID
				   NETSNMP_DS_LIB_MIB_ERRORS
				   NETSNMP_DS_LIB_SAVE_MIB_DESCRS
				   NETSNMP_DS_LIB_MIB_COMMENT_TERM
				   NETSNMP_DS_LIB_MIB_PARSE_LABEL
				   NETSNMP_DS_LIB_DUMP_PACKET
				   NETSNMP_DS_LIB_LOG_TIMESTAMP
				   NETSNMP_DS_LIB_DONT_READ_CONFIGS
				   NETSNMP_DS_LIB_MIB_REPLACE
				   NETSNMP_DS_LIB_PRINT_NUMERIC_ENUM
				   NETSNMP_DS_LIB_PRINT_NUMERIC_OIDS
				   NETSNMP_DS_LIB_DONT_BREAKDOWN_OIDS
				   NETSNMP_DS_LIB_ALARM_DONT_USE_SIG
				   NETSNMP_DS_LIB_PRINT_FULL_OID
				   NETSNMP_DS_LIB_QUICK_PRINT
				   NETSNMP_DS_LIB_RANDOM_ACCESS
				   NETSNMP_DS_LIB_REGEX_ACCESS
				   NETSNMP_DS_LIB_DONT_CHECK_RANGE
				   NETSNMP_DS_LIB_NO_TOKEN_WARNINGS
				   NETSNMP_DS_LIB_NUMERIC_TIMETICKS
				   NETSNMP_DS_LIB_ESCAPE_QUOTES
				   NETSNMP_DS_LIB_REVERSE_ENCODE
				   NETSNMP_DS_LIB_PRINT_BARE_VALUE
				   NETSNMP_DS_LIB_EXTENDED_INDEX
				   NETSNMP_DS_LIB_PRINT_HEX_TEXT
				   NETSNMP_DS_LIB_PRINT_UCD_STYLE_OID
				   NETSNMP_DS_LIB_READ_UCD_STYLE_OID
				   NETSNMP_DS_LIB_HAVE_READ_PREMIB_CONFIG
				   NETSNMP_DS_LIB_HAVE_READ_CONFIG
				   NETSNMP_DS_LIB_QUICKE_PRINT
				   NETSNMP_DS_LIB_MIB_WARNINGS
				   NETSNMP_DS_LIB_SECLEVEL
				   NETSNMP_DS_LIB_SNMPVERSION
				   NETSNMP_DS_LIB_DEFAULT_PORT
				   NETSNMP_DS_LIB_OID_OUTPUT_FORMAT
				   NETSNMP_DS_SNMP_VERSION_1
				   NETSNMP_DS_SNMP_VERSION_2c
				   NETSNMP_DS_SNMP_VERSION_3
				   NETSNMP_DS_LIB_SECNAME
				   NETSNMP_DS_LIB_CONTEXT
				   NETSNMP_DS_LIB_PASSPHRASE
				   NETSNMP_DS_LIB_AUTHPASSPHRASE
				   NETSNMP_DS_LIB_PRIVPASSPHRASE
				   NETSNMP_DS_LIB_OPTIONALCONFIG
				   NETSNMP_DS_LIB_APPTYPE
				   NETSNMP_DS_LIB_COMMUNITY
				   NETSNMP_DS_LIB_PERSISTENT_DIR
				   NETSNMP_DS_LIB_CONFIGURATION_DIR
				   NETSNMP_DS_LIB_SECMODEL
				   NETSNMP_DS_LIB_MIBDIRS

=head2 Exportable functions

  int netsnmp_ds_get_boolean(int storeid, int which)
  int netsnmp_ds_get_int(int storeid, int which)
  char *netsnmp_ds_get_string(int storeid, int which)
  void *netsnmp_ds_get_void(int storeid, int which)
  int netsnmp_ds_register_config(unsigned char type, const char *ftype, const char *token,
                       int storeid, int which)
  int netsnmp_ds_register_premib(unsigned char type, const char *ftype, const char *token,
                       int storeid, int which)
  int netsnmp_ds_set_boolean(int storeid, int which, int value)
  int netsnmp_ds_set_int(int storeid, int which, int value)
  int netsnmp_ds_set_string(int storeid, int which, const char *value)
  int netsnmp_ds_set_void(int storeid, int which, void *value)
  void netsnmp_ds_shutdown(void)
  int netsnmp_ds_toggle_boolean(int storeid, int which)

=head1 AUTHOR

Wes Hardaker, hardaker@users.sourceforge.net

=head1 SEE ALSO

perl(1), default_store(3).

=cut
