package ProFTPD::Tests::Modules::mod_tls::sni;

use lib qw(t/lib);
use base qw(ProFTPD::TestSuite::Child);
use strict;

use Carp;
use File::Copy;
use File::Path qw(mkpath);
use File::Spec;
use IO::Handle;
use IPC::Open3;
use Socket;

use ProFTPD::TestSuite::FTP;
use ProFTPD::TestSuite::Utils qw(:auth :config :running :test :testsuite);

$| = 1;

my $order = 0;

# All of these tests use an external `curl` for testing the ability to
# login AND download with SNI, across multiple TLS protocol versions.

my $CURL = '/Users/tj/local/curl-7.69.0/bin/curl';

my $TESTS = {
  tls_sni_tlsv10 => {
    order => ++$order,
    test_class => [qw(forking)],
  },

  tls_sni_tlsv10_opt_ignoresni => {
    order => ++$order,
    test_class => [qw(forking)],
  },

  tls_sni_tlsv10_session_cache => {
    order => ++$order,
    test_class => [qw(forking mod_tls_shmcache)],
  },

  tls_sni_tlsv11 => {
    order => ++$order,
    test_class => [qw(forking)],
  },

  tls_sni_tlsv11_opt_ignoresni => {
    order => ++$order,
    test_class => [qw(forking)],
  },

  tls_sni_tlsv11_session_cache => {
    order => ++$order,
    test_class => [qw(forking mod_tls_shmcache)],
  },

  tls_sni_tlsv12 => {
    order => ++$order,
    test_class => [qw(forking)],
  },

  tls_sni_tlsv12_opt_ignoresni => {
    order => ++$order,
    test_class => [qw(forking)],
  },

  tls_sni_tlsv12_session_cache => {
    order => ++$order,
    test_class => [qw(forking mod_tls_shmcache)],
  },

  tls_sni_tlsv13 => {
    order => ++$order,
    test_class => [qw(forking)],
  },

  tls_sni_tlsv13_opt_ignoresni => {
    order => ++$order,
    test_class => [qw(forking)],
  },

  tls_sni_tlsv13_session_cache => {
    order => ++$order,
    test_class => [qw(forking mod_tls_shmcache)],
  },

  tls_sni_tlsv13_session_tickets_off => {
    order => ++$order,
    test_class => [qw(forking)],
  },

  tls_sni_tlsv13_session_tickets_on => {
    order => ++$order,
    test_class => [qw(forking)],
  },

  tls_sni_default_vhost_issue1369 => {
    order => ++$order,
    test_class => [qw(bug forking)],
  },

};

sub new {
  return shift()->SUPER::new(@_);
}

sub list_tests {
  return testsuite_get_runnable_tests($TESTS);
}

sub test_curl_sni {
  my $self = shift;
  my $tmpdir = $self->{tmpdir};
  my $setup = test_setup($tmpdir, 'sni');

  my $tls_protocol = shift;
  my $tls_options = shift;
  my $addl_config = shift;

  my $ec_cert_file = File::Spec->rel2abs('t/etc/modules/mod_tls/ec-server-cert.pem');
  my $ec_ca_file = File::Spec->rel2abs('t/etc/modules/mod_tls/ec-ca.pem');

  # Use RSA certs for our name-based vhost, to test that the SNI vhost switching
  # is working as expected.
  my $rsa_cert_file = File::Spec->rel2abs('t/etc/modules/mod_tls/server-cert.pem');
  my $rsa_ca_file = File::Spec->rel2abs('t/etc/modules/mod_tls/ca-cert.pem');

  my $config = {
    PidFile => $setup->{pid_file},
    ScoreboardFile => $setup->{scoreboard_file},
    SystemLog => $setup->{log_file},
    TraceLog => $setup->{log_file},
    Trace => 'tls:30 tls.trace:30',

    AuthUserFile => $setup->{auth_user_file},
    AuthGroupFile => $setup->{auth_group_file},
    TransferLog => 'none',
    WtmpLog => 'off',

    IfModules => {
      'mod_delay.c' => {
        DelayEngine => 'off',
      },

      'mod_tls.c' => {
        TLSEngine => 'on',
        TLSLog => $setup->{log_file},
        TLSRequired => 'on',
        TLSRSACertificateFile => $rsa_cert_file,
        TLSCACertificateFile => $rsa_ca_file,
        TLSProtocol => $tls_protocol,
        TLSOptions => $tls_options,
      },

      $addl_config,
    },
  };

  my ($port, $config_user, $config_group) = config_write($setup->{config_file},
    $config);

  my $host = 'castaglia';

  # Configure a name-based <VirtualHost> for our testing.
  if (open(my $fh, ">> $setup->{config_file}")) {
    print $fh <<EOC;
<VirtualHost 127.0.0.1>
  ServerAlias $host
  Port $port

  AuthUserFile $setup->{auth_user_file}
  AuthGroupFile $setup->{auth_group_file}
  AuthOrder mod_auth_file.c
  TransferLog none
  WtmpLog off

  <IfModule mod_delay.c>
    DelayEngine off
  </IfModule>

  <IfModule mod_tls.c>
    TLSEngine on
    TLSLog $setup->{log_file}
    TLSRequired on
    TLSRSACertificateFile $rsa_cert_file
    TLSCACertificateFile $rsa_ca_file
    TLSOptions $tls_options
    TLSProtocol $tls_protocol
  </IfModule>
</VirtualHost>
EOC
    unless (close($fh)) {
      die("Can't write $setup->{config_file}: $!");
    }

  } else {
    die("Can't open $setup->{config_file}: $!");
  }

  # Open pipes, for use between the parent and child processes.  Specifically,
  # the child will indicate when it's done with its test by writing a message
  # to the parent.
  my ($rfh, $wfh);
  unless (pipe($rfh, $wfh)) {
    die("Can't open pipe: $!");
  }

  require Net::FTPSSL;

  my $ex;

  # Fork child
  $self->handle_sigchld();
  defined(my $pid = fork()) or die("Can't fork: $!");
  if ($pid) {
    eval {
      my $curl_cmd = [
        $CURL,
        '-kvs',
        '-o',
        '/dev/null',
        '--user',
        "$setup->{user}:$setup->{passwd}",
        '--ssl',
        '--sessionid',
        '--resolve',
        "$host:$port:127.0.0.1",
        "ftp://$host:$port/sni.conf"
      ];

      my $curl_rh = IO::Handle->new();
      my $curl_wh = IO::Handle->new();
      my $curl_eh = IO::Handle->new();

      $curl_wh->autoflush(1);

      local $SIG{CHLD} = 'DEFAULT';

      # Give the server a chance to start up
      sleep(2);

      if ($ENV{TEST_VERBOSE}) {
        print STDERR "Executing: ", join(' ', @$curl_cmd), "\n";
      }

      my $curl_pid = open3($curl_wh, $curl_rh, $curl_eh, @$curl_cmd);
      waitpid($curl_pid, 0);
      my $exit_status = $?;

      my ($res, $errstr);
      if ($exit_status >> 8 == 0) {
        $errstr = join('', <$curl_eh>);
        $res = 0;

      } else {
        $errstr = join('', <$curl_eh>);
        if ($ENV{TEST_VERBOSE}) {
          print STDERR "Stderr: $errstr\n";
        }

        $res = 1;
      }

      unless ($res == 0) {
        die("Can't download from FTPS server: $errstr");
      }
    };
    if ($@) {
      $ex = $@;
    }

    $wfh->print("done\n");
    $wfh->flush();

  } else {
    eval { server_wait($setup->{config_file}, $rfh) };
    if ($@) {
      warn($@);
      exit 1;
    }

    exit 0;
  }

  # Stop server
  server_stop($setup->{pid_file});
  $self->assert_child_ok($pid);

  test_cleanup($setup->{log_file}, $ex);
}

# Testing TLSv1.0

sub tls_sni_tlsv10 {
  my $self = shift;
  my $tls_protocol = 'TLSv1.0';
  my $tls_options = 'EnableDiags';
  my $addl_config = {};

  test_curl_sni($self, $tls_protocol, $tls_options, $addl_config);
}

sub tls_sni_tlsv10_opt_ignoresni {
  my $self = shift;
  my $tls_protocol = 'TLSv1.0';
  my $tls_options = 'EnableDiags IgnoreSNI';
  my $addl_config = {};

  test_curl_sni($self, $tls_protocol, $tls_options, $addl_config);
}

sub tls_sni_tlsv10_session_cache {
  my $self = shift;
  my $tls_protocol = 'TLSv1.0';
  my $tls_options = 'EnableDiags';

  # For session caching
  my $shm_path = File::Spec->rel2abs("$self->{tmpdir}/tls-shmcache");

  my $addl_config = {
    'mod_tls_shmcache.c' => {
      # 10332 is the minimum number of bytes for shmcache
      TLSSessionCache => "shm:/file=$shm_path&size=41328",
    }
  };

  test_curl_sni($self, $tls_protocol, $tls_options, $addl_config);
}

# Testing TLSv1.1

sub tls_sni_tlsv11 {
  my $self = shift;
  my $tls_protocol = 'TLSv1.1';
  my $tls_options = 'EnableDiags';
  my $addl_config = {};

  test_curl_sni($self, $tls_protocol, $tls_options, $addl_config);
}

sub tls_sni_tlsv11_opt_ignoresni {
  my $self = shift;
  my $tls_protocol = 'TLSv1.1';
  my $tls_options = 'EnableDiags IgnoreSNI';
  my $addl_config = {};

  test_curl_sni($self, $tls_protocol, $tls_options, $addl_config);
}

sub tls_sni_tlsv11_session_cache {
  my $self = shift;
  my $tls_protocol = 'TLSv1.1';
  my $tls_options = 'EnableDiags';

  # For session caching
  my $shm_path = File::Spec->rel2abs("$self->{tmpdir}/tls-shmcache");

  my $addl_config = {
    'mod_tls_shmcache.c' => {
      # 10332 is the minimum number of bytes for shmcache
      TLSSessionCache => "shm:/file=$shm_path&size=41328",
    }
  };

  test_curl_sni($self, $tls_protocol, $tls_options, $addl_config);
}

# Testing TLSv1.2

sub tls_sni_tlsv12 {
  my $self = shift;
  my $tls_protocol = 'TLSv1.2';
  my $tls_options = 'EnableDiags';
  my $addl_config = {};

  test_curl_sni($self, $tls_protocol, $tls_options, $addl_config);
}

sub tls_sni_tlsv12_opt_ignoresni {
  my $self = shift;
  my $tls_protocol = 'TLSv1.2';
  my $tls_options = 'EnableDiags IgnoreSNI';
  my $addl_config = {};

  test_curl_sni($self, $tls_protocol, $tls_options, $addl_config);
}

sub tls_sni_tlsv12_session_cache {
  my $self = shift;
  my $tls_protocol = 'TLSv1.2';
  my $tls_options = 'EnableDiags';

  # For session caching
  my $shm_path = File::Spec->rel2abs("$self->{tmpdir}/tls-shmcache");

  my $addl_config = {
    'mod_tls_shmcache.c' => {
      # 10332 is the minimum number of bytes for shmcache
      TLSSessionCache => "shm:/file=$shm_path&size=41328",
    }
  };

  test_curl_sni($self, $tls_protocol, $tls_options, $addl_config);
}

# Testing TLSv1.3

sub tls_sni_tlsv13 {
  my $self = shift;
  my $tls_protocol = 'TLSv1.3';
  my $tls_options = 'EnableDiags';
  my $addl_config = {};

  test_curl_sni($self, $tls_protocol, $tls_options, $addl_config);
}

sub tls_sni_tlsv13_opt_ignoresni {
  my $self = shift;
  my $tls_protocol = 'TLSv1.3';
  my $tls_options = 'EnableDiags IgnoreSNI';
  my $addl_config = {};

  test_curl_sni($self, $tls_protocol, $tls_options, $addl_config);
}

sub tls_sni_tlsv13_session_cache {
  my $self = shift;
  my $tls_protocol = 'TLSv1.3';
  my $tls_options = 'EnableDiags';

  # For session caching
  my $shm_path = File::Spec->rel2abs("$self->{tmpdir}/tls-shmcache");

  my $addl_config = {
    'mod_tls_shmcache.c' => {
      # 10332 is the minimum number of bytes for shmcache
      TLSSessionCache => "shm:/file=$shm_path&size=41328",
    }
  };

  test_curl_sni($self, $tls_protocol, $tls_options, $addl_config);
}

sub tls_sni_tlsv13_session_tickets_off {
  my $self = shift;
  my $tls_protocol = 'TLSv1.3';
  my $tls_options = 'EnableDiags IgnoreSNI';
  my $addl_config = {
    'mod_tls.c' => {
      'TLSSessionTickets' => 'off'
    }
  };

  test_curl_sni($self, $tls_protocol, $tls_options, $addl_config);
}

sub tls_sni_tlsv13_session_tickets_on {
  my $self = shift;
  my $tls_protocol = 'TLSv1.3';
  my $tls_options = 'EnableDiags IgnoreSNI';
  my $addl_config = {
    'mod_tls.c' => {
      'TLSSessionTickets' => 'on'
    }
  };

  test_curl_sni($self, $tls_protocol, $tls_options, $addl_config);
}

sub tls_sni_default_vhost_issue1369 {
  my $self = shift;
  my $tmpdir = $self->{tmpdir};
  my $setup = test_setup($tmpdir, 'sni');

  my $ec_cert_file = File::Spec->rel2abs('t/etc/modules/mod_tls/ec-server-cert.pem');
  my $ec_ca_file = File::Spec->rel2abs('t/etc/modules/mod_tls/ec-ca.pem');

  # Use RSA certs for our name-based vhost, to test that the SNI vhost switching
  # is working as expected.
  my $rsa_cert_file = File::Spec->rel2abs('t/etc/modules/mod_tls/server-cert.pem');
  my $rsa_ca_file = File::Spec->rel2abs('t/etc/modules/mod_tls/ca-cert.pem');

  my $config = {
    PidFile => $setup->{pid_file},
    ScoreboardFile => $setup->{scoreboard_file},
    SystemLog => $setup->{log_file},
    TraceLog => $setup->{log_file},
    Trace => 'binding:30',
    Port => '0',
  };

  my ($port, $config_user, $config_group) = config_write($setup->{config_file},
    $config);

  my $host = 'first.castaglia.org';

  # Configure name-based <VirtualHost>s for our testing.
  if (open(my $fh, ">> $setup->{config_file}")) {
    print $fh <<EOC;
<Global>
  Trace binding:30 tls:30 tls.trace:30

  TransferLog none
  WtmpLog off

  <IfModule mod_delay.c>
    DelayEngine off
  </IfModule>

  <IfModule mod_tls.c>
    TLSEngine on
    TLSLog $setup->{log_file}
    TLSRequired on
    TLSECCertificateFile $ec_cert_file
    TLSCACertificateFile $ec_ca_file
    TLSOptions EnableDiags
  </IfModule>
</Global>

<VirtualHost 127.0.0.1>
  Port $port

  # No ServerAlias here; this will be the first vhost found for any connection
  # that doesn't use `HOST`, TLS SNI, etc.

  ServerName "Lacking ServerAlias"
</VirtualHost>

<VirtualHost 127.0.0.1>
  Port $port
  ServerAlias first.castaglia.org

  AuthUserFile $setup->{auth_user_file}
  AuthGroupFile $setup->{auth_group_file}
  AuthOrder mod_auth_file.c

  <IfModule mod_tls.c>
    TLSRSACertificateFile $rsa_cert_file
    TLSCACertificateFile $rsa_ca_file
    TLSOptions EnableDiags NoSessionReuseRequired
  </IfModule>
</VirtualHost>

<VirtualHost 127.0.0.1>
  Port $port
  ServerAlias second.castaglia.org
</VirtualHost>

<VirtualHost 127.0.0.1>
  Port $port
  ServerAlias \*
  ServerName "wildcard"
</VirtualHost>
EOC
    unless (close($fh)) {
      die("Can't write $setup->{config_file}: $!");
    }

  } else {
    die("Can't open $setup->{config_file}: $!");
  }

  # Open pipes, for use between the parent and child processes.  Specifically,
  # the child will indicate when it's done with its test by writing a message
  # to the parent.
  my ($rfh, $wfh);
  unless (pipe($rfh, $wfh)) {
    die("Can't open pipe: $!");
  }

  require Net::FTPSSL;

  my $ex;

  # Fork child
  $self->handle_sigchld();
  defined(my $pid = fork()) or die("Can't fork: $!");
  if ($pid) {
    eval {
      my $curl_cmd = [
        $CURL,
        '-kvs',
        '-o',
        '/dev/null',
        '--user',
        "$setup->{user}:$setup->{passwd}",
        '--ssl',
        '--resolve',
        "$host:$port:127.0.0.1",
        "ftp://$host:$port/sni.conf"
      ];

      my $curl_rh = IO::Handle->new();
      my $curl_wh = IO::Handle->new();
      my $curl_eh = IO::Handle->new();

      $curl_wh->autoflush(1);

      local $SIG{CHLD} = 'DEFAULT';

      # Give the server a chance to start up
      sleep(2);

      if ($ENV{TEST_VERBOSE}) {
        print STDERR "Executing: ", join(' ', @$curl_cmd), "\n";
      }

      my $curl_pid = open3($curl_wh, $curl_rh, $curl_eh, @$curl_cmd);
      waitpid($curl_pid, 0);
      my $exit_status = $?;

      my ($res, $errstr);
      if ($exit_status >> 8 == 0) {
        $errstr = join('', <$curl_eh>);
        $res = 0;

      } else {
        $errstr = join('', <$curl_eh>);
        if ($ENV{TEST_VERBOSE}) {
          print STDERR "Stderr: $errstr\n";
        }

        $res = 1;
      }

      unless ($res == 0) {
        die("Can't download from FTPS server: $errstr");
      }
    };
    if ($@) {
      $ex = $@;
    }

    $wfh->print("done\n");
    $wfh->flush();

  } else {
    eval { server_wait($setup->{config_file}, $rfh) };
    if ($@) {
      warn($@);
      exit 1;
    }

    exit 0;
  }

  # Stop server
  server_stop($setup->{pid_file});
  $self->assert_child_ok($pid);

  test_cleanup($setup->{log_file}, $ex);
}

1;
