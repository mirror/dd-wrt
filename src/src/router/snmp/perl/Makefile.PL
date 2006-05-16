use ExtUtils::MakeMaker;
use Config;
use Getopt::Long;
require 5;

%MakeParams = InitMakeParams();

WriteMakefile(%MakeParams);

sub InitMakeParams {
    $nsconfig="net-snmp-config"; # in path by default
    my %Params = (
		  'NAME' => 'Bundle::NetSNMP',
		  'DIR' => [qw(default_store ASN OID agent SNMP)]
		  );

    # bogus, but these options need to be passed to the lower levels
    $opts = NetSNMPGetOpts("./");

    return(%Params);
}

# common subroutines -- DO NOT EDIT.
# They are imported from the Makefile.subs.pl file
sub NetSNMPGetOpts {
    my %ret;
    my $rootpath = shift;
    $rootpath = "../" if (!$rootpath);
    $rootpath .= '/' if ($rootpath !~ /\/$/);
    
    if ($Config{'osname'} eq 'MSWin32') {
      if ($ENV{'NET-SNMP-IN-SOURCE'})
      {
	$ret{'insource'} = $ENV{'NET-SNMP-IN-SOURCE'};
      }
      elsif ($ENV{'NET-SNMP-PATH'})
      {
	$ret{'prefix'} = $ENV{'NET-SNMP-PATH'};
      }
      else
      {
	# don't have env vars, pull from command line and put there
	GetOptions("NET-SNMP-IN-SOURCE=s" => \$ret{'insource'},
	           "NET-SNMP-PATH=s"      => \$ret{'prefix'});

	$ENV{'NET-SNMP-IN-SOURCE'}    = $ret{'insource'};
	$ENV{'NET-SNMP-PATH'}         = $ret{'prefix'};
      }
    }
    else
    {
      if ($ENV{'NET-SNMP-CONFIG'} && 
        $ENV{'NET-SNMP-IN-SOURCE'}) {
	# have env vars, pull from there
	$ret{'nsconfig'} = $ENV{'NET-SNMP-CONFIG'};
	$ret{'insource'} = $ENV{'NET-SNMP-IN-SOURCE'};
      } else {
	# don't have env vars, pull from command line and put there
	GetOptions("NET-SNMP-CONFIG=s" => \$ret{'nsconfig'},
	           "NET-SNMP-IN-SOURCE=s" => \$ret{'insource'});

	if ($ret{'insource'} eq "true" && $ret{'nsconfig'} eq "") {
	    $ret{'nsconfig'}="sh ROOTPATH../net-snmp-config";
	} elsif ($ret{'nsconfig'} eq "") {
	    $ret{'nsconfig'}="net-snmp-config";
	}

	$ENV{'NET-SNMP-CONFIG'}    = $ret{'nsconfig'};
	$ENV{'NET-SNMP-IN-SOURCE'} = $ret{'insource'};
      }
    }	
    
    $ret{'nsconfig'} =~ s/ROOTPATH/$rootpath/;

    $ret{'rootpath'} = $rootpath;

    \%ret;
}

