use ExtUtils::MakeMaker;
require 5;
use Config;
use Getopt::Long;
my %MakeParams = ();

%MakeParams = InitMakeParams();

WriteMakefile(%MakeParams);


sub InitMakeParams {
    my $opts;
    my %Params = (
		  'NAME'		=> 'NetSNMP::default_store',
		  'VERSION_FROM'	=> 'default_store.pm', # finds $VERSION
		  'XSPROTOARG'          => '-prototypes',
		  );

    my ($snmp_lib, $snmp_llib, $sep);
    if ($Config{'osname'} eq 'MSWin32') {
      $opts = NetSNMPGetOpts();	
      $snmp_lib = 'libsnmp.lib';
      $snmp_link_lib = 'libsnmp';
      $sep = '\\';

      $Params{'DEFINE'} = "-DMSVC_PERL";
      
      if ($opts->{'insource'}) {
	$Params{'LIBS'} = "-L$basedir\\win32\\lib\\ -l$snmp_link_lib";
      }
      else {
	my @LibDirs = split (';',$ENV{LIB});
	if ($opts->{'prefix'}) {
	  push (@LibDirs,"$ENV{'NET-SNMP-PATH'}${sep}lib");
	}
	$noLibDir = 1;
	while ($noLibDir) {
	  $LibDir = find_files(["$snmp_lib"],\@LibDirs);
	  if ($LibDir ne '') {
	    $noLibDir = 0;
	  }
	  else
	  {
	    @LibDirs = ();
	    @LibDirs[0] = prompt("The Net-SNMP library ($snmp_lib) could not be found.\nPlease enter the directory where it is located:");
	    @LibDirs[0] =~ s/\\$//;
	  }
	}
	$Params{LIBS} = "-L$LibDir -l$snmp_link_lib";
      }

      $Params{'INC'} = "-I$basedir\\include\\ -I$basedir\\include\\net-snmp\\ -I$basedir\\win32\\ ";
    }
    else {
	$opts = NetSNMPGetOpts();
	$Params{'LIBS'}    = `$opts->{'nsconfig'} --libs`;
	chomp($Params{'LIBS'});
	$Params{'CCFLAGS'} = `$opts->{'nsconfig'} --cflags`;
	chomp($Params{'CCFLAGS'});
	$Params{'CCFLAGS'} .= " " . $Config{'ccflags'};
	if ($opts->{'insource'} eq "true") {
	    $Params{'LIBS'} = "-L../../snmplib/.libs -L../../snmplib/ " . $Params{'LIBS'};
	    $Params{'CCFLAGS'} = "-I../../include " . $Params{'CCFLAGS'};
	}
	$Params{'CCFLAGS'} =~ s/ -W[-\w]+//g; # ignore developer warnings
	if ($Params{'LIBS'} eq "" || $Params{'CCFLAGS'} eq "") {
	    die "You need to install net-snmp first (I can't find net-snmp-config)";
	}
    }

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

