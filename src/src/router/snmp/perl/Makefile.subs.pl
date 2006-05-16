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

      $basedir = `%COMSPEC% /c cd`;
      chomp $basedir;
      $basedir =~ /(.*?)\\perl.*/;
      $basedir = $1;
      print "Net-SNMP base directory: $basedir\n";

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

