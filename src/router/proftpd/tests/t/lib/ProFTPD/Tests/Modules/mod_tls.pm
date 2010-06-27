package ProFTPD::Tests::Modules::mod_tls;

use lib qw(t/lib);
use base qw(Test::Unit::TestCase ProFTPD::TestSuite::Child);
use strict;

use File::Path qw(mkpath rmtree);
use File::Spec;
use IO::Handle;

use ProFTPD::TestSuite::FTP;
use ProFTPD::TestSuite::Utils qw(:auth :config :running :test :testsuite);

$| = 1;

my $order = 0;

my $TESTS = {
  tls_login_ok => {
    order => ++$order,
    test_class => [qw(forking)],
  },

  tls_double_auth => {
    order => ++$order,
    test_class => [qw(bug forking)],
  },

  tls_pkcs12_login_ok => {
    order => ++$order,
    test_class => [qw(forking)],
  },

  tls_crl_file_ok => {
    order => ++$order,
    test_class => [qw(forking)],
  },

  tls_list_no_session_reuse => {
    order => ++$order,
    test_class => [qw(forking)],
  },

  tls_list_with_no_session_reuse_required_opt => {
    order => ++$order,
    test_class => [qw(forking)],
  },

  tls_list_fails_tls_required_by_dir_bug2178 => {
    order => ++$order,
    test_class => [qw(bug forking)],
  },

  tls_list_ok_tls_required_by_dir_bug2178 => {
    order => ++$order,
    test_class => [qw(bug forking)],
  },

  tls_list_fails_tls_required_by_ftpaccess_bug2178 => {
    order => ++$order,
    test_class => [qw(bug forking)],
  },

  tls_list_ok_tls_required_by_ftpaccess_bug2178 => {
    order => ++$order,
    test_class => [qw(bug forking)],
  },

  tls_incompatible_config_bug3247 => {
    order => ++$order,
    test_class => [qw(bug forking)],
  },

  tls_implicit_ssl_bug3266 => {
    order => ++$order,
    test_class => [qw(bug forking rootprivs)],
  },

  tls_client_cert_verify_failed_embedded_nul_bug3275 => {
    order => ++$order,
    test_class => [qw(bug forking)],
  },

  tls_opts_std_env_vars => {
    order => ++$order,
    test_class => [qw(forking)],
  },

  tls_opts_std_env_vars_client_vars => {
    order => ++$order,
    test_class => [qw(forking)],
  },

  tls_opts_ipaddr_required => {
    order => ++$order,
    test_class => [qw(forking)],
  },

  tls_opts_allow_per_user_tlsrequired_on_anon_login_bug3325 => {
    order => ++$order,
    test_class => [qw(bug forking rootprivs)],
  },

  tls_opts_allow_per_user_tlsrequired_on_user_login_bug3325 => {
    order => ++$order,
    test_class => [qw(bug forking)],
  },

  tls_opts_allow_per_user_tlsrequired_auth_user_login_bug3325 => {
    order => ++$order,
    test_class => [qw(bug forking)],
  },

  tls_opts_allow_per_user_tlsrequired_ctrl_user_login_bug3325 => {
    order => ++$order,
    test_class => [qw(bug forking)],
  },

  tls_opts_allow_per_user_tlsrequired_data_user_login_bug3325 => {
    order => ++$order,
    test_class => [qw(bug forking)],
  },

  tls_opts_allow_per_user_tlsrequired_on_ifsess_login_bug3325 => {
    order => ++$order,
    test_class => [qw(bug forking mod_ifsession)],
  },

  tls_rest_2gb_last_byte => {
    order => ++$order,
    test_class => [qw(forking)],
  },

  tls_rest_4gb_last_byte => {
    order => ++$order,
    test_class => [qw(forking)],
  },

  tls_stor_empty_file => {
    order => ++$order,
    test_class => [qw(forking)],
  },

  tls_retr_empty_file => {
    order => ++$order,
    test_class => [qw(forking)],
  },

};

sub new {
  return shift()->SUPER::new(@_);
}

sub list_tests {
  # Check for the required Perl modules:
  #
  #  Net-SSLeay
  #  IO-Socket-SSL
  #  Net-FTPSSL

  my $required = [qw(
    Net::SSLeay
    IO::Socket::SSL
    Net::FTPSSL
  )];

  foreach my $req (@$required) {
    eval "use $req";
    if ($@) {
      print STDERR "\nWARNING:\n + Module '$req' not found, skipping all tests\n";

      if ($ENV{TEST_VERBOSE}) {
        print STDERR "Unable to load $req: $@\n";
      }

      return qw(testsuite_empty_test);
    }
  }

  return testsuite_get_runnable_tests($TESTS);
}

sub set_up {
  my $self = shift;
  $self->{tmpdir} = testsuite_get_tmp_dir();
  
  # Create temporary scratch dir
  eval { mkpath($self->{tmpdir}) };
  if ($@) {
    my $abs_path = File::Spec->rel2abs($self->{tmpdir});
    die("Can't create dir $abs_path: $@");
  }
}

sub tear_down {
  my $self = shift;

  # Remove temporary scratch dir
  if ($self->{tmpdir}) {
    eval { rmtree($self->{tmpdir}) };
  }

  undef $self;
}

sub tls_login_ok {
  my $self = shift;
  my $tmpdir = $self->{tmpdir};

  my $config_file = "$tmpdir/tls.conf";
  my $pid_file = File::Spec->rel2abs("$tmpdir/tls.pid");
  my $scoreboard_file = File::Spec->rel2abs("$tmpdir/tls.scoreboard");

  my $log_file = File::Spec->rel2abs('tests.log');

  my $auth_user_file = File::Spec->rel2abs("$tmpdir/tls.passwd");
  my $auth_group_file = File::Spec->rel2abs("$tmpdir/tls.group");

  my $user = 'proftpd';
  my $passwd = 'test';
  my $home_dir = File::Spec->rel2abs($tmpdir);
  my $uid = 500;
  my $gid = 500;

  # Make sure that, if we're running as root, that the home directory has
  # permissions/privs set for the account we create
  if ($< == 0) {
    unless (chmod(0755, $home_dir)) {
      die("Can't set perms on $home_dir to 0755: $!");
    }

    unless (chown($uid, $gid, $home_dir)) {
      die("Can't set owner of $home_dir to $uid/$gid: $!");
    }
  }

  auth_user_write($auth_user_file, $user, $passwd, $uid, $gid, $home_dir,
    '/bin/bash');
  auth_group_write($auth_group_file, 'ftpd', $gid, $user);

  my $cert_file = File::Spec->rel2abs('t/etc/modules/mod_tls/server-cert.pem');
  my $ca_file = File::Spec->rel2abs('t/etc/modules/mod_tls/ca-cert.pem');

  my $config = {
    PidFile => $pid_file,
    ScoreboardFile => $scoreboard_file,
    SystemLog => $log_file,

    AuthUserFile => $auth_user_file,
    AuthGroupFile => $auth_group_file,

    IfModules => {
      'mod_delay.c' => {
        DelayEngine => 'off',
      },

      'mod_tls.c' => {
        TLSEngine => 'on',
        TLSLog => $log_file,
        TLSProtocol => 'SSLv3 TLSv1',
        TLSRequired => 'on',
        TLSRSACertificateFile => $cert_file,
        TLSCACertificateFile => $ca_file,
      },
    },
  };

  my ($port, $config_user, $config_group) = config_write($config_file, $config);

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
      # Give the server a chance to start up
      sleep(2);

      my $client = Net::FTPSSL->new('127.0.0.1',
        Encryption => 'E',
        Port => $port,
      );

      unless ($client) {
        die("Can't connect to FTPS server: " . IO::Socket::SSL::errstr());
      }

      unless ($client->login($user, $passwd)) {
        die("Can't login: " . $client->last_message());
      }
    };

    if ($@) {
      $ex = $@;
    }

    $wfh->print("done\n");
    $wfh->flush();

  } else {
    eval { server_wait($config_file, $rfh) };
    if ($@) {
      warn($@);
      exit 1;
    }

    exit 0;
  }

  # Stop server
  server_stop($pid_file);

  $self->assert_child_ok($pid);

  if ($ex) {
    die($ex);
  }

  unlink($log_file);
}

sub tls_double_auth {
  my $self = shift;
  my $tmpdir = $self->{tmpdir};

  my $config_file = "$tmpdir/tls.conf";
  my $pid_file = File::Spec->rel2abs("$tmpdir/tls.pid");
  my $scoreboard_file = File::Spec->rel2abs("$tmpdir/tls.scoreboard");

  my $log_file = File::Spec->rel2abs('tests.log');

  my $auth_user_file = File::Spec->rel2abs("$tmpdir/tls.passwd");
  my $auth_group_file = File::Spec->rel2abs("$tmpdir/tls.group");

  my $user = 'proftpd';
  my $passwd = 'test';
  my $home_dir = File::Spec->rel2abs($tmpdir);
  my $uid = 500;
  my $gid = 500;

  # Make sure that, if we're running as root, that the home directory has
  # permissions/privs set for the account we create
  if ($< == 0) {
    unless (chmod(0755, $home_dir)) {
      die("Can't set perms on $home_dir to 0755: $!");
    }

    unless (chown($uid, $gid, $home_dir)) {
      die("Can't set owner of $home_dir to $uid/$gid: $!");
    }
  }

  auth_user_write($auth_user_file, $user, $passwd, $uid, $gid, $home_dir,
    '/bin/bash');
  auth_group_write($auth_group_file, 'ftpd', $gid, $user);

  my $cert_file = File::Spec->rel2abs('t/etc/modules/mod_tls/server-cert.pem');
  my $ca_file = File::Spec->rel2abs('t/etc/modules/mod_tls/ca-cert.pem');

  my $config = {
    PidFile => $pid_file,
    ScoreboardFile => $scoreboard_file,
    SystemLog => $log_file,

    AuthUserFile => $auth_user_file,
    AuthGroupFile => $auth_group_file,

    IfModules => {
      'mod_delay.c' => {
        DelayEngine => 'off',
      },

      'mod_tls.c' => {
        TLSEngine => 'on',
        TLSLog => $log_file,
        TLSProtocol => 'SSLv3 TLSv1',
        TLSRequired => 'on',
        TLSRSACertificateFile => $cert_file,
        TLSCACertificateFile => $ca_file,
      },
    },
  };

  my ($port, $config_user, $config_group) = config_write($config_file, $config);

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
      # Give the server a chance to start up
      sleep(2);

      my $client = Net::FTPSSL->new('127.0.0.1',
        Encryption => 'E',
        Port => $port,
      );

      unless ($client) {
        die("Can't connect to FTPS server: " . IO::Socket::SSL::errstr());
      }

      unless ($client->login($user, $passwd)) {
        die("Can't login: " . $client->last_message());
      }

      # Sending AUTH after we've established our SSL/TLS session should
      # fail.
      if ($client->auth()) {
        die("Second AUTH succeeded unexpectedly");
      }
    };

    if ($@) {
      $ex = $@;
    }

    $wfh->print("done\n");
    $wfh->flush();

  } else {
    eval { server_wait($config_file, $rfh) };
    if ($@) {
      warn($@);
      exit 1;
    }

    exit 0;
  }

  # Stop server
  server_stop($pid_file);

  $self->assert_child_ok($pid);

  if ($ex) {
    die($ex);
  }

  unlink($log_file);
}

sub tls_pkcs12_login_ok {
  my $self = shift;
  my $tmpdir = $self->{tmpdir};

  my $config_file = "$tmpdir/tls.conf";
  my $pid_file = File::Spec->rel2abs("$tmpdir/tls.pid");
  my $scoreboard_file = File::Spec->rel2abs("$tmpdir/tls.scoreboard");

  my $log_file = File::Spec->rel2abs('tests.log');

  my $auth_user_file = File::Spec->rel2abs("$tmpdir/tls.passwd");
  my $auth_group_file = File::Spec->rel2abs("$tmpdir/tls.group");

  my $user = 'proftpd';
  my $passwd = 'test';
  my $home_dir = File::Spec->rel2abs($tmpdir);
  my $uid = 500;
  my $gid = 500;

  # Make sure that, if we're running as root, that the home directory has
  # permissions/privs set for the account we create
  if ($< == 0) {
    unless (chmod(0755, $home_dir)) {
      die("Can't set perms on $home_dir to 0755: $!");
    }

    unless (chown($uid, $gid, $home_dir)) {
      die("Can't set owner of $home_dir to $uid/$gid: $!");
    }
  }

  auth_user_write($auth_user_file, $user, $passwd, $uid, $gid, $home_dir,
    '/bin/bash');
  auth_group_write($auth_group_file, 'ftpd', $gid, $user);

  my $pkcs12_file = File::Spec->rel2abs('t/etc/modules/mod_tls/server-cert.p12');
  my $ca_file = File::Spec->rel2abs('t/etc/modules/mod_tls/ca-cert.pem');

  my $config = {
    PidFile => $pid_file,
    ScoreboardFile => $scoreboard_file,
    SystemLog => $log_file,

    AuthUserFile => $auth_user_file,
    AuthGroupFile => $auth_group_file,

    IfModules => {
      'mod_delay.c' => {
        DelayEngine => 'off',
      },

      'mod_tls.c' => {
        TLSEngine => 'on',
        TLSLog => $log_file,
        TLSProtocol => 'SSLv3 TLSv1',
        TLSRequired => 'on',
        TLSPKCS12File => $pkcs12_file,
        TLSCACertificateFile => $ca_file,
      },
    },
  };

  my ($port, $config_user, $config_group) = config_write($config_file, $config);

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
      # Give the server a chance to start up
      sleep(2);

      my $client = Net::FTPSSL->new('127.0.0.1',
        Encryption => 'E',
        Port => $port,
      );

      unless ($client) {
        die("Can't connect to FTPS server: " . IO::Socket::SSL::errstr());
      }

      unless ($client->login($user, $passwd)) {
        die("Can't login: " . $client->last_message());
      }
    };

    if ($@) {
      $ex = $@;
    }

    $wfh->print("done\n");
    $wfh->flush();

  } else {
    eval { server_wait($config_file, $rfh) };
    if ($@) {
      warn($@);
      exit 1;
    }

    exit 0;
  }

  # Stop server
  server_stop($pid_file);

  $self->assert_child_ok($pid);

  if ($ex) {
    die($ex);
  }

  unlink($log_file);
}

sub tls_crl_file_ok {
  my $self = shift;
  my $tmpdir = $self->{tmpdir};

  my $config_file = "$tmpdir/tls.conf";
  my $pid_file = File::Spec->rel2abs("$tmpdir/tls.pid");
  my $scoreboard_file = File::Spec->rel2abs("$tmpdir/tls.scoreboard");

  my $log_file = File::Spec->rel2abs('tests.log');

  my $auth_user_file = File::Spec->rel2abs("$tmpdir/tls.passwd");
  my $auth_group_file = File::Spec->rel2abs("$tmpdir/tls.group");

  my $user = 'proftpd';
  my $passwd = 'test';
  my $home_dir = File::Spec->rel2abs($tmpdir);
  my $uid = 500;
  my $gid = 500;

  # Make sure that, if we're running as root, that the home directory has
  # permissions/privs set for the account we create
  if ($< == 0) {
    unless (chmod(0755, $home_dir)) {
      die("Can't set perms on $home_dir to 0755: $!");
    }

    unless (chown($uid, $gid, $home_dir)) {
      die("Can't set owner of $home_dir to $uid/$gid: $!");
    }
  }

  auth_user_write($auth_user_file, $user, $passwd, $uid, $gid, $home_dir,
    '/bin/bash');
  auth_group_write($auth_group_file, 'ftpd', $gid, $user);

  my $server_cert = File::Spec->rel2abs('t/etc/modules/mod_tls/server-cert.pem');
  my $client_cert = File::Spec->rel2abs('t/etc/modules/mod_tls/client-cert.pem');
  my $ca_cert = File::Spec->rel2abs('t/etc/modules/mod_tls/ca-cert.pem');
  my $crl_file = File::Spec->rel2abs('t/etc/modules/mod_tls/ca-crl.pem');

  my $config = {
    PidFile => $pid_file,
    ScoreboardFile => $scoreboard_file,
    SystemLog => $log_file,

    AuthUserFile => $auth_user_file,
    AuthGroupFile => $auth_group_file,

    IfModules => {
      'mod_delay.c' => {
        DelayEngine => 'off',
      },

      'mod_tls.c' => {
        TLSEngine => 'on',
        TLSLog => $log_file,
        TLSProtocol => 'SSLv3 TLSv1',
        TLSRequired => 'on',
        TLSRSACertificateFile => $server_cert,
        TLSCACertificateFile => $ca_cert,

        # Verifying clients via CRLs only works when verification is
        # explicitly enabled.
        TLSCARevocationFile => $crl_file,
        TLSVerifyClient => 'on',
      },
    },
  };

  my ($port, $config_user, $config_group) = config_write($config_file, $config);

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
      # Give the server a chance to start up
      sleep(2);

      my $client;

      eval {
        # IO::Socket::SSL options
        my $ssl_opts = {
          SSL_use_cert => 1,
          SSL_cert_file => $client_cert,
          SSL_key_file => $client_cert,
        };

        $client = Net::FTPSSL->new('127.0.0.1',
          Croak => 1,
          Encryption => 'E',
          Port => $port,
          SSL_Advanced => $ssl_opts,
        );
      };

      my $ex = $@;
      unless ($ex) {
        die("SSL connection succeeded unexpectedly");
      }

      my $errstr = IO::Socket::SSL::errstr();

      my $expected = 'certificate revoked';
      $self->assert(qr/$expected/, $errstr,
        test_msg("Expected '$expected', got '$errstr'"));
    };

    if ($@) {
      $ex = $@;
    }

    $wfh->print("done\n");
    $wfh->flush();

  } else {
    eval { server_wait($config_file, $rfh) };
    if ($@) {
      warn($@);
      exit 1;
    }

    exit 0;
  }

  # Stop server
  server_stop($pid_file);

  $self->assert_child_ok($pid);

  if ($ex) {
    die($ex);
  }

  unlink($log_file);
}

sub tls_list_no_session_reuse {
  my $self = shift;
  my $tmpdir = $self->{tmpdir};

  my $config_file = "$tmpdir/tls.conf";
  my $pid_file = File::Spec->rel2abs("$tmpdir/tls.pid");
  my $scoreboard_file = File::Spec->rel2abs("$tmpdir/tls.scoreboard");

  my $log_file = File::Spec->rel2abs('tests.log');

  my $auth_user_file = File::Spec->rel2abs("$tmpdir/tls.passwd");
  my $auth_group_file = File::Spec->rel2abs("$tmpdir/tls.group");

  my $user = 'proftpd';
  my $passwd = 'test';
  my $home_dir = File::Spec->rel2abs($tmpdir);
  my $uid = 500;
  my $gid = 500;

  # Make sure that, if we're running as root, that the home directory has
  # permissions/privs set for the account we create
  if ($< == 0) {
    unless (chmod(0755, $home_dir)) {
      die("Can't set perms on $home_dir to 0755: $!");
    }

    unless (chown($uid, $gid, $home_dir)) {
      die("Can't set owner of $home_dir to $uid/$gid: $!");
    }
  }

  auth_user_write($auth_user_file, $user, $passwd, $uid, $gid, $home_dir,
    '/bin/bash');
  auth_group_write($auth_group_file, 'ftpd', $gid, $user);

  my $cert_file = File::Spec->rel2abs('t/etc/modules/mod_tls/server-cert.pem');
  my $ca_file = File::Spec->rel2abs('t/etc/modules/mod_tls/ca-cert.pem');

  my $config = {
    PidFile => $pid_file,
    ScoreboardFile => $scoreboard_file,
    SystemLog => $log_file,

    AuthUserFile => $auth_user_file,
    AuthGroupFile => $auth_group_file,

    IfModules => {
      'mod_delay.c' => {
        DelayEngine => 'off',
      },

      'mod_tls.c' => {
        TLSEngine => 'on',
        TLSLog => $log_file,
        TLSProtocol => 'SSLv3 TLSv1',
        TLSRequired => 'on',
        TLSRSACertificateFile => $cert_file,
        TLSCACertificateFile => $ca_file,
      },
    },
  };

  my ($port, $config_user, $config_group) = config_write($config_file, $config);

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
      # Give the server a chance to start up
      sleep(2);

      my $client = Net::FTPSSL->new('127.0.0.1',
        Encryption => 'E',
        Port => $port,
        Croak => 1,
      );

      unless ($client->login($user, $passwd)) {
        die("Can't login: " . $client->last_message());
      }

      # Since we are requiring SSL session resuse for data transfers,
      # and this client is not using SSL session resumption, I expect
      # this data transfer to fail.
      my $res = $client->list('.');
      if ($res) {
        die("LIST succeeded unexpectedly");
      }

      my $resp = $client->message();
      my $expected = '425 Unable to build data connection: Operation not permitted';
      $self->assert($expected eq $resp,
        test_msg("Expected '$expected', got '$resp'"));
    };

    if ($@) {
      $ex = $@;
    }

    $wfh->print("done\n");
    $wfh->flush();

  } else {
    eval { server_wait($config_file, $rfh) };
    if ($@) {
      warn($@);
      exit 1;
    }

    exit 0;
  }

  # Stop server
  server_stop($pid_file);

  $self->assert_child_ok($pid);

  if ($ex) {
    die($ex);
  }

  unlink($log_file);
}

sub tls_list_with_no_session_reuse_required_opt {
  my $self = shift;
  my $tmpdir = $self->{tmpdir};

  my $config_file = "$tmpdir/tls.conf";
  my $pid_file = File::Spec->rel2abs("$tmpdir/tls.pid");
  my $scoreboard_file = File::Spec->rel2abs("$tmpdir/tls.scoreboard");

  my $log_file = File::Spec->rel2abs('tests.log');

  my $auth_user_file = File::Spec->rel2abs("$tmpdir/tls.passwd");
  my $auth_group_file = File::Spec->rel2abs("$tmpdir/tls.group");

  my $user = 'proftpd';
  my $passwd = 'test';
  my $home_dir = File::Spec->rel2abs($tmpdir);
  my $uid = 500;
  my $gid = 500;

  # Make sure that, if we're running as root, that the home directory has
  # permissions/privs set for the account we create
  if ($< == 0) {
    unless (chmod(0755, $home_dir)) {
      die("Can't set perms on $home_dir to 0755: $!");
    }

    unless (chown($uid, $gid, $home_dir)) {
      die("Can't set owner of $home_dir to $uid/$gid: $!");
    }
  }

  auth_user_write($auth_user_file, $user, $passwd, $uid, $gid, $home_dir,
    '/bin/bash');
  auth_group_write($auth_group_file, 'ftpd', $gid, $user);

  my $cert_file = File::Spec->rel2abs('t/etc/modules/mod_tls/server-cert.pem');
  my $ca_file = File::Spec->rel2abs('t/etc/modules/mod_tls/ca-cert.pem');

  my $config = {
    PidFile => $pid_file,
    ScoreboardFile => $scoreboard_file,
    SystemLog => $log_file,

    AuthUserFile => $auth_user_file,
    AuthGroupFile => $auth_group_file,

    IfModules => {
      'mod_delay.c' => {
        DelayEngine => 'off',
      },

      'mod_tls.c' => {
        TLSEngine => 'on',
        TLSLog => $log_file,
        TLSProtocol => 'SSLv3 TLSv1',
        TLSRequired => 'on',
        TLSRSACertificateFile => $cert_file,
        TLSCACertificateFile => $ca_file,

        TLSOptions => 'NoSessionReuseRequired',
      },
    },
  };

  my ($port, $config_user, $config_group) = config_write($config_file, $config);

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
      # Give the server a chance to start up
      sleep(2);

      my $client = Net::FTPSSL->new('127.0.0.1',
        Encryption => 'E',
        Port => $port,
        Croak => 1,
      );

      unless ($client->login($user, $passwd)) {
        die("Can't login: " . $client->last_message());
      }

      # Since we are NOT requiring SSL session resuse for data transfers,
      # and this client is not using SSL session resumption, I expect
      # this data transfer to succeed.
      my $res = $client->list('.');
      unless ($res) {
        die("LIST failed unexpectedly: " . $client->message());
      }
    };

    if ($@) {
      $ex = $@;
    }

    $wfh->print("done\n");
    $wfh->flush();

  } else {
    eval { server_wait($config_file, $rfh) };
    if ($@) {
      warn($@);
      exit 1;
    }

    exit 0;
  }

  # Stop server
  server_stop($pid_file);

  $self->assert_child_ok($pid);

  if ($ex) {
    die($ex);
  }

  unlink($log_file);
}

sub tls_list_fails_tls_required_by_dir_bug2178 {
  my $self = shift;
  my $tmpdir = $self->{tmpdir};

  my $config_file = "$tmpdir/tls.conf";
  my $pid_file = File::Spec->rel2abs("$tmpdir/tls.pid");
  my $scoreboard_file = File::Spec->rel2abs("$tmpdir/tls.scoreboard");

  my $log_file = File::Spec->rel2abs('tests.log');

  my $auth_user_file = File::Spec->rel2abs("$tmpdir/tls.passwd");
  my $auth_group_file = File::Spec->rel2abs("$tmpdir/tls.group");

  my $user = 'proftpd';
  my $passwd = 'test';
  my $home_dir = File::Spec->rel2abs($tmpdir);
  my $uid = 500;
  my $gid = 500;

  # Make sure that, if we're running as root, that the home directory has
  # permissions/privs set for the account we create
  if ($< == 0) {
    unless (chmod(0755, $home_dir)) {
      die("Can't set perms on $home_dir to 0755: $!");
    }

    unless (chown($uid, $gid, $home_dir)) {
      die("Can't set owner of $home_dir to $uid/$gid: $!");
    }
  }

  auth_user_write($auth_user_file, $user, $passwd, $uid, $gid, $home_dir,
    '/bin/bash');
  auth_group_write($auth_group_file, 'ftpd', $gid, $user);

  my $server_cert = File::Spec->rel2abs('t/etc/modules/mod_tls/server-cert.pem');
  my $client_cert = File::Spec->rel2abs('t/etc/modules/mod_tls/client-cert.pem');
  my $ca_cert = File::Spec->rel2abs('t/etc/modules/mod_tls/ca-cert.pem');

  my $config = {
    PidFile => $pid_file,
    ScoreboardFile => $scoreboard_file,
    SystemLog => $log_file,

    AuthUserFile => $auth_user_file,
    AuthGroupFile => $auth_group_file,

    Directory => {
      $home_dir => {
        TLSRequired => 'on',
      },
    },

    IfModules => {
      'mod_delay.c' => {
        DelayEngine => 'off',
      },

      'mod_tls.c' => {
        TLSEngine => 'on',
        TLSLog => $log_file,
        TLSProtocol => 'SSLv3 TLSv1',
        TLSRequired => 'off',
        TLSRSACertificateFile => $server_cert,
        TLSCACertificateFile => $ca_cert,
        TLSOptions => 'NoSessionReuseRequired',
      },
    },
  };

  my ($port, $config_user, $config_group) = config_write($config_file, $config);

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
      # Give the server a chance to start up
      sleep(2);

      my $client;

      # IO::Socket::SSL options
      my $ssl_opts = {
        SSL_use_cert => 1,
        SSL_cert_file => $client_cert,
        SSL_key_file => $client_cert,
      };

      $client = Net::FTPSSL->new('127.0.0.1',
        Croak => 1,
        DataProtLevel => 'C',
        Encryption => 'E',
        Port => $port,
        SSL_Advanced => $ssl_opts,
      );

      unless ($client->login($user, $passwd)) {
        die("Can't login: " . $client->last_message());
      }

      my $res = $client->list();
      if (!$@ &&
          defined($res)) {
        die("LIST succeeded unexpectedly");
      }

      my $resp_msg = $client->last_message();

      my $expected = '550 SSL/TLS required on the data channel';
      $self->assert($expected eq $resp_msg,
        test_msg("Expected '$expected', got '$resp_msg'"));
    };

    if ($@) {
      $ex = $@;
    }

    $wfh->print("done\n");
    $wfh->flush();

  } else {
    eval { server_wait($config_file, $rfh) };
    if ($@) {
      warn($@);
      exit 1;
    }

    exit 0;
  }

  # Stop server
  server_stop($pid_file);

  $self->assert_child_ok($pid);

  if ($ex) {
    die($ex);
  }

  unlink($log_file);
}

sub tls_list_ok_tls_required_by_dir_bug2178 {
  my $self = shift;
  my $tmpdir = $self->{tmpdir};

  my $config_file = "$tmpdir/tls.conf";
  my $pid_file = File::Spec->rel2abs("$tmpdir/tls.pid");
  my $scoreboard_file = File::Spec->rel2abs("$tmpdir/tls.scoreboard");

  my $log_file = File::Spec->rel2abs('tests.log');

  my $auth_user_file = File::Spec->rel2abs("$tmpdir/tls.passwd");
  my $auth_group_file = File::Spec->rel2abs("$tmpdir/tls.group");

  my $user = 'proftpd';
  my $passwd = 'test';
  my $home_dir = File::Spec->rel2abs($tmpdir);
  my $uid = 500;
  my $gid = 500;

  # Make sure that, if we're running as root, that the home directory has
  # permissions/privs set for the account we create
  if ($< == 0) {
    unless (chmod(0755, $home_dir)) {
      die("Can't set perms on $home_dir to 0755: $!");
    }

    unless (chown($uid, $gid, $home_dir)) {
      die("Can't set owner of $home_dir to $uid/$gid: $!");
    }
  }

  auth_user_write($auth_user_file, $user, $passwd, $uid, $gid, $home_dir,
    '/bin/bash');
  auth_group_write($auth_group_file, 'ftpd', $gid, $user);

  my $server_cert = File::Spec->rel2abs('t/etc/modules/mod_tls/server-cert.pem');
  my $client_cert = File::Spec->rel2abs('t/etc/modules/mod_tls/client-cert.pem');
  my $ca_cert = File::Spec->rel2abs('t/etc/modules/mod_tls/ca-cert.pem');

  my $config = {
    PidFile => $pid_file,
    ScoreboardFile => $scoreboard_file,
    SystemLog => $log_file,

    AuthUserFile => $auth_user_file,
    AuthGroupFile => $auth_group_file,

    Directory => {
      $home_dir => {
        TLSRequired => 'on',
      },
    },

    IfModules => {
      'mod_delay.c' => {
        DelayEngine => 'off',
      },

      'mod_tls.c' => {
        TLSEngine => 'on',
        TLSLog => $log_file,
        TLSProtocol => 'SSLv3 TLSv1',
        TLSRequired => 'off',
        TLSRSACertificateFile => $server_cert,
        TLSCACertificateFile => $ca_cert,
        TLSOptions => 'NoSessionReuseRequired',
      },
    },
  };

  my ($port, $config_user, $config_group) = config_write($config_file, $config);

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
      # Give the server a chance to start up
      sleep(2);

      my $client;

      # IO::Socket::SSL options
      my $ssl_opts = {
        SSL_use_cert => 1,
        SSL_cert_file => $client_cert,
        SSL_key_file => $client_cert,
      };

      $client = Net::FTPSSL->new('127.0.0.1',
        Croak => 1,
        DataProtLevel => 'P',
        Encryption => 'E',
        Port => $port,
        SSL_Advanced => $ssl_opts,
      );

      unless ($client->login($user, $passwd)) {
        die("Can't login: " . $client->last_message());
      }

      my $res = $client->list();
      unless ($res) {
        die("LIST failed unexpectedly: " . $client->last_message());
      }

      my $resp_msg = $client->last_message();
      $client->quit();

      my $expected = '226 Transfer complete';
      $self->assert($expected eq $resp_msg,
        test_msg("Expected '$expected', got '$resp_msg'"));
    };

    if ($@) {
      $ex = $@;
    }

    $wfh->print("done\n");
    $wfh->flush();

  } else {
    eval { server_wait($config_file, $rfh) };
    if ($@) {
      warn($@);
      exit 1;
    }

    exit 0;
  }

  # Stop server
  server_stop($pid_file);

  $self->assert_child_ok($pid);

  if ($ex) {
    die($ex);
  }

  unlink($log_file);
}

sub tls_list_fails_tls_required_by_ftpaccess_bug2178 {
  my $self = shift;
  my $tmpdir = $self->{tmpdir};

  my $config_file = "$tmpdir/tls.conf";
  my $pid_file = File::Spec->rel2abs("$tmpdir/tls.pid");
  my $scoreboard_file = File::Spec->rel2abs("$tmpdir/tls.scoreboard");

  my $log_file = File::Spec->rel2abs('tests.log');

  my $auth_user_file = File::Spec->rel2abs("$tmpdir/tls.passwd");
  my $auth_group_file = File::Spec->rel2abs("$tmpdir/tls.group");

  my $user = 'proftpd';
  my $passwd = 'test';
  my $home_dir = File::Spec->rel2abs($tmpdir);
  my $uid = 500;
  my $gid = 500;

  # Make sure that, if we're running as root, that the home directory has
  # permissions/privs set for the account we create
  if ($< == 0) {
    unless (chmod(0755, $home_dir)) {
      die("Can't set perms on $home_dir to 0755: $!");
    }

    unless (chown($uid, $gid, $home_dir)) {
      die("Can't set owner of $home_dir to $uid/$gid: $!");
    }
  }

  auth_user_write($auth_user_file, $user, $passwd, $uid, $gid, $home_dir,
    '/bin/bash');
  auth_group_write($auth_group_file, 'ftpd', $gid, $user);

  my $server_cert = File::Spec->rel2abs('t/etc/modules/mod_tls/server-cert.pem');
  my $client_cert = File::Spec->rel2abs('t/etc/modules/mod_tls/client-cert.pem');
  my $ca_cert = File::Spec->rel2abs('t/etc/modules/mod_tls/ca-cert.pem');

  my $sub_dir = File::Spec->rel2abs("$home_dir/subdir");
  mkpath($sub_dir);

  my $access_file = File::Spec->rel2abs("$sub_dir/.ftpaccess");
  if (open(my $fh, "> $access_file")) {
    print $fh "TLSRequired on\n";
    unless (close($fh)) {
      die("Can't write $access_file: $!");
    }

  } else {
    die("Can't open $access_file: $!");
  }

  my $config = {
    PidFile => $pid_file,
    ScoreboardFile => $scoreboard_file,
    SystemLog => $log_file,

    AllowOverride => 'on',
    AuthUserFile => $auth_user_file,
    AuthGroupFile => $auth_group_file,

    IfModules => {
      'mod_delay.c' => {
        DelayEngine => 'off',
      },

      'mod_tls.c' => {
        TLSEngine => 'on',
        TLSLog => $log_file,
        TLSProtocol => 'SSLv3 TLSv1',
        TLSRequired => 'off',
        TLSRSACertificateFile => $server_cert,
        TLSCACertificateFile => $ca_cert,
        TLSOptions => 'NoSessionReuseRequired',
      },
    },
  };

  my ($port, $config_user, $config_group) = config_write($config_file, $config);

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
      # Give the server a chance to start up
      sleep(2);

      my $client;

      # IO::Socket::SSL options
      my $ssl_opts = {
        SSL_use_cert => 1,
        SSL_cert_file => $client_cert,
        SSL_key_file => $client_cert,
      };

      $client = Net::FTPSSL->new('127.0.0.1',
        Croak => 1,
        DataProtLevel => 'C',
        Encryption => 'E',
        Port => $port,
        SSL_Advanced => $ssl_opts,
      );

      unless ($client->login($user, $passwd)) {
        die("Can't login: " . $client->last_message());
      }

      $client->cwd('subdir');

      my $res = $client->list();
      if (!$@ &&
          defined($res)) {
        die("LIST succeeded unexpectedly");
      }
      my $resp_msg = $client->last_message();

      my $expected = '550 SSL/TLS required on the data channel';
      $self->assert($expected eq $resp_msg,
        test_msg("Expected '$expected', got '$resp_msg'"));
    };

    if ($@) {
      $ex = $@;
    }

    $wfh->print("done\n");
    $wfh->flush();

  } else {
    eval { server_wait($config_file, $rfh) };
    if ($@) {
      warn($@);
      exit 1;
    }

    exit 0;
  }

  # Stop server
  server_stop($pid_file);

  $self->assert_child_ok($pid);

  if ($ex) {
    die($ex);
  }

  unlink($log_file);
}

sub tls_list_ok_tls_required_by_ftpaccess_bug2178 {
  my $self = shift;
  my $tmpdir = $self->{tmpdir};

  my $config_file = "$tmpdir/tls.conf";
  my $pid_file = File::Spec->rel2abs("$tmpdir/tls.pid");
  my $scoreboard_file = File::Spec->rel2abs("$tmpdir/tls.scoreboard");

  my $log_file = File::Spec->rel2abs('tests.log');

  my $auth_user_file = File::Spec->rel2abs("$tmpdir/tls.passwd");
  my $auth_group_file = File::Spec->rel2abs("$tmpdir/tls.group");

  my $user = 'proftpd';
  my $passwd = 'test';
  my $home_dir = File::Spec->rel2abs($tmpdir);
  my $uid = 500;
  my $gid = 500;

  # Make sure that, if we're running as root, that the home directory has
  # permissions/privs set for the account we create
  if ($< == 0) {
    unless (chmod(0755, $home_dir)) {
      die("Can't set perms on $home_dir to 0755: $!");
    }

    unless (chown($uid, $gid, $home_dir)) {
      die("Can't set owner of $home_dir to $uid/$gid: $!");
    }
  }

  auth_user_write($auth_user_file, $user, $passwd, $uid, $gid, $home_dir,
    '/bin/bash');
  auth_group_write($auth_group_file, 'ftpd', $gid, $user);

  my $server_cert = File::Spec->rel2abs('t/etc/modules/mod_tls/server-cert.pem');
  my $client_cert = File::Spec->rel2abs('t/etc/modules/mod_tls/client-cert.pem');
  my $ca_cert = File::Spec->rel2abs('t/etc/modules/mod_tls/ca-cert.pem');

  my $sub_dir = File::Spec->rel2abs("$home_dir/subdir");
  mkpath($sub_dir);

  my $access_file = File::Spec->rel2abs("$sub_dir/.ftpaccess");
  if (open(my $fh, "> $access_file")) {
    print $fh "TLSRequired on\n";
    unless (close($fh)) {
      die("Can't write $access_file: $!");
    }

  } else {
    die("Can't open $access_file: $!");
  }

  my $config = {
    PidFile => $pid_file,
    ScoreboardFile => $scoreboard_file,
    SystemLog => $log_file,

    AllowOverride => 'on',
    AuthUserFile => $auth_user_file,
    AuthGroupFile => $auth_group_file,

    IfModules => {
      'mod_delay.c' => {
        DelayEngine => 'off',
      },

      'mod_tls.c' => {
        TLSEngine => 'on',
        TLSLog => $log_file,
        TLSProtocol => 'SSLv3 TLSv1',
        TLSRequired => 'off',
        TLSRSACertificateFile => $server_cert,
        TLSCACertificateFile => $ca_cert,
        TLSOptions => 'NoSessionReuseRequired',
      },
    },
  };

  my ($port, $config_user, $config_group) = config_write($config_file, $config);

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
      # Give the server a chance to start up
      sleep(2);

      my $client;

      # IO::Socket::SSL options
      my $ssl_opts = {
        SSL_use_cert => 1,
        SSL_cert_file => $client_cert,
        SSL_key_file => $client_cert,
      };

      $client = Net::FTPSSL->new('127.0.0.1',
        Croak => 1,
        DataProtLevel => 'P',
        Encryption => 'E',
        Port => $port,
        SSL_Advanced => $ssl_opts,
      );

      unless ($client->login($user, $passwd)) {
        die("Can't login: " . $client->last_message());
      }

      $client->cwd('subdir');
      my $res = $client->list();
      if ($res) {
        die("LIST succeeded unexpectedly");
      }

      my $resp_msg = $client->last_message();
      $client->quit();

      my $expected = '226 Transfer complete';
      $self->assert($expected eq $resp_msg,
        test_msg("Expected '$expected', got '$resp_msg'"));
    };

    if ($@) {
      $ex = $@;
    }

    $wfh->print("done\n");
    $wfh->flush();

  } else {
    eval { server_wait($config_file, $rfh) };
    if ($@) {
      warn($@);
      exit 1;
    }

    exit 0;
  }

  # Stop server
  server_stop($pid_file);

  $self->assert_child_ok($pid);

  if ($ex) {
    die($ex);
  }

  unlink($log_file);
}

sub tls_incompatible_config_bug3247 {
  my $self = shift;
  my $tmpdir = $self->{tmpdir};

  my $config_file = "$tmpdir/tls.conf";
  my $pid_file = File::Spec->rel2abs("$tmpdir/tls.pid");
  my $scoreboard_file = File::Spec->rel2abs("$tmpdir/tls.scoreboard");

  my $log_file = File::Spec->rel2abs('tests.log');

  my $server_cert = File::Spec->rel2abs('t/etc/modules/mod_tls/server-cert.pem');
  my $client_cert = File::Spec->rel2abs('t/etc/modules/mod_tls/client-cert.pem');
  my $ca_cert = File::Spec->rel2abs('t/etc/modules/mod_tls/ca-cert.pem');

  my $config = {
    PidFile => $pid_file,
    ScoreboardFile => $scoreboard_file,
    SystemLog => $log_file,

    IfModules => {
      'mod_delay.c' => {
        DelayEngine => 'off',
      },

      'mod_tls.c' => {
        TLSEngine => 'on',
        TLSLog => $log_file,
        TLSProtocol => 'SSLv3 TLSv1',
        TLSRequired => 'auth',
        TLSRSACertificateFile => $server_cert,
        TLSCACertificateFile => $ca_cert,
        TLSOptions => 'AllowPerUser',
      },
    },
  };

  my ($port, $config_user, $config_group) = config_write($config_file, $config);

  eval { server_start($config_file, undef, $pid_file) };
  unless ($@) {
    die("server started unexpectedly");
  }

  unlink($log_file);
}

sub tls_implicit_ssl_bug3266 {
  my $self = shift;
  my $tmpdir = $self->{tmpdir};

  my $config_file = "$tmpdir/tls.conf";
  my $pid_file = File::Spec->rel2abs("$tmpdir/tls.pid");
  my $scoreboard_file = File::Spec->rel2abs("$tmpdir/tls.scoreboard");

  my $log_file = File::Spec->rel2abs('tests.log');

  my $auth_user_file = File::Spec->rel2abs("$tmpdir/tls.passwd");
  my $auth_group_file = File::Spec->rel2abs("$tmpdir/tls.group");

  my $user = 'proftpd';
  my $passwd = 'test';
  my $home_dir = File::Spec->rel2abs($tmpdir);
  my $uid = 500;
  my $gid = 500;

  # Make sure that, if we're running as root, that the home directory has
  # permissions/privs set for the account we create
  if ($< == 0) {
    unless (chmod(0755, $home_dir)) {
      die("Can't set perms on $home_dir to 0755: $!");
    }

    unless (chown($uid, $gid, $home_dir)) {
      die("Can't set owner of $home_dir to $uid/$gid: $!");
    }
  }

  auth_user_write($auth_user_file, $user, $passwd, $uid, $gid, $home_dir,
    '/bin/bash');
  auth_group_write($auth_group_file, 'ftpd', $gid, $user);

  my $cert_file = File::Spec->rel2abs('t/etc/modules/mod_tls/server-cert.pem');
  my $ca_file = File::Spec->rel2abs('t/etc/modules/mod_tls/ca-cert.pem');

  my $config = {

    # The stupid Net::FTPSSL client requires port 990 when implicit
    # FTPS is used, regardless of the port we pass to the constructor.
    # (What kind of client API does not allow the port to be changed??)
    Port => 990,

    PidFile => $pid_file,
    ScoreboardFile => $scoreboard_file,
    SystemLog => $log_file,
    TraceLog => $log_file,
    Trace => 'DEFAULT:10',

    AuthUserFile => $auth_user_file,
    AuthGroupFile => $auth_group_file,

    IfModules => {
      'mod_delay.c' => {
        DelayEngine => 'off',
      },

      'mod_tls.c' => {
        TLSEngine => 'on',
        TLSLog => $log_file,
        TLSProtocol => 'SSLv3 TLSv1',
        TLSRequired => 'on',
        TLSOptions => 'UseImplicitSSL',
        TLSRSACertificateFile => $cert_file,
        TLSCACertificateFile => $ca_file,
      },
    },
  };

  my ($port, $config_user, $config_group) = config_write($config_file, $config);

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
      # Give the server a chance to start up
      sleep(2);

      my $client_opts = {
        Encryption => 'I',
        Port => $port,
      };

      if ($ENV{TEST_VERBOSE}) {
        $client_opts->{Debug} = 1;
      }

      my $client = Net::FTPSSL->new('127.0.0.1', %$client_opts);

      unless ($client) {
        die("Can't connect to FTPS server: " . IO::Socket::SSL::errstr());
      }

      unless ($client->login($user, $passwd)) {
        die("Can't login: " . $client->last_message());
      }
    };

    if ($@) {
      $ex = $@;
    }

    $wfh->print("done\n");
    $wfh->flush();

  } else {
    eval { server_wait($config_file, $rfh) };
    if ($@) {
      warn($@);
      exit 1;
    }

    exit 0;
  }

  # Stop server
  server_stop($pid_file);

  $self->assert_child_ok($pid);

  if ($ex) {
    die($ex);
  }

  unlink($log_file);
}

sub tls_client_cert_verify_failed_embedded_nul_bug3275 {
  my $self = shift;
  my $tmpdir = $self->{tmpdir};

  my $config_file = "$tmpdir/tls.conf";
  my $pid_file = File::Spec->rel2abs("$tmpdir/tls.pid");
  my $scoreboard_file = File::Spec->rel2abs("$tmpdir/tls.scoreboard");

  my $log_file = File::Spec->rel2abs('tests.log');

  my $auth_user_file = File::Spec->rel2abs("$tmpdir/tls.passwd");
  my $auth_group_file = File::Spec->rel2abs("$tmpdir/tls.group");

  my $user = 'proftpd';
  my $passwd = 'test';
  my $home_dir = File::Spec->rel2abs($tmpdir);
  my $uid = 500;
  my $gid = 500;

  # Make sure that, if we're running as root, that the home directory has
  # permissions/privs set for the account we create
  if ($< == 0) {
    unless (chmod(0755, $home_dir)) {
      die("Can't set perms on $home_dir to 0755: $!");
    }

    unless (chown($uid, $gid, $home_dir)) {
      die("Can't set owner of $home_dir to $uid/$gid: $!");
    }
  }

  auth_user_write($auth_user_file, $user, $passwd, $uid, $gid, $home_dir,
    '/bin/bash');
  auth_group_write($auth_group_file, 'ftpd', $gid, $user);

  my $server_cert = File::Spec->rel2abs('t/etc/modules/mod_tls/server-cert.pem');
#  my $ca_cert = File::Spec->rel2abs('t/etc/modules/mod_tls/ca-cert.pem');
  my $ca_cert = File::Spec->rel2abs('t/etc/modules/mod_tls/ca-nul-subjaltname.pem');

  # The cert/key with embedded NULs in the subjAltName field were obtained
  # from:
  #
  #  http://people.redhat.com/thoger/certs-with-nuls/

  my $client_cert = File::Spec->rel2abs('t/etc/modules/mod_tls/cert-nul-subjaltname.pem');
  my $client_key = File::Spec->rel2abs('t/etc/modules/mod_tls/key-nul-subjaltname.pem');

  my $config = {
    PidFile => $pid_file,
    ScoreboardFile => $scoreboard_file,
    SystemLog => $log_file,

    AuthUserFile => $auth_user_file,
    AuthGroupFile => $auth_group_file,

    IfModules => {
      'mod_delay.c' => {
        DelayEngine => 'off',
      },

      'mod_tls.c' => {
        TLSEngine => 'on',
        TLSLog => $log_file,
        TLSProtocol => 'SSLv3 TLSv1',
        TLSRequired => 'on',
        TLSRSACertificateFile => $server_cert,
        TLSCACertificateFile => $ca_cert,

        TLSVerifyClient => 'on',

        # To trigger Bug#3275, we need to verify the subjAltName field in
        # the present cert, which means enabling DNS resolution
        TLSOptions => 'dNSNameRequired',

        # We need to enable reverse DNS resolution as well, for this to work
        UseReverseDNS => 'on',
      },
    },
  };

  my ($port, $config_user, $config_group) = config_write($config_file, $config);

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
      # Give the server a chance to start up
      sleep(2);

      my $client;

      eval {
        # IO::Socket::SSL options
        my $ssl_opts = {
          SSL_use_cert => 1,
          SSL_cert_file => $client_cert,
          SSL_key_file => $client_key,
        };

        $client = Net::FTPSSL->new('127.0.0.1',
          Croak => 1,
          Encryption => 'E',
          Port => $port,
          SSL_Advanced => $ssl_opts,
        );

        # Net::FTPSSL is rather stupid, and treats the initial 234 response
        # code (indicating that the SSL/TLS handshake should proceed) as
        # indicating success _of the handshake_.
        #
        # Thus if we want to test if the handshake succeeded, we have to
        # act as if it did succeed, then try to login (which should fail).
        # Sigh.

        unless ($client->login($user, $passwd)) {
          die("Can't login: " . $client->last_message());
        }
      };

      if ($@) {
        die($@);
      }
    };

    if ($@) {
      $ex = $@;
    }

    $wfh->print("done\n");
    $wfh->flush();

  } else {
    eval { server_wait($config_file, $rfh) };
    if ($@) {
      warn($@);
      exit 1;
    }

    exit 0;
  }

  # Stop server
  server_stop($pid_file);

  $self->assert_child_ok($pid);

  unless ($ex) {
    die("Test succeeded unexpectedly");
  }

  # Not quite done yet.  To make sure that this test did as expected, we
  # need to examine the log file, and look for the phrase 'spoof attempt'.
  # That indicates that mod_tls saw the embedded NUL.
  if (open(my $fh, "< $log_file")) {
    my $ok = 0;

    while (my $line = <$fh>) {
      chomp($line);

      if ($line =~ /spoof attempt/) {
        $ok = 1;
        last;
      }
    }

    close($fh);

    unless ($ok) {
      die("Expected TLSLog 'spoof attempt' not found as expected");
    }

  } else {
    die("Can't read $log_file: $!");
  }

  unlink($log_file);
}

sub tls_opts_std_env_vars {
  my $self = shift;
  my $tmpdir = $self->{tmpdir};

  my $config_file = "$tmpdir/tls.conf";
  my $pid_file = File::Spec->rel2abs("$tmpdir/tls.pid");
  my $scoreboard_file = File::Spec->rel2abs("$tmpdir/tls.scoreboard");

  my $log_file = File::Spec->rel2abs('tests.log');

  my $auth_user_file = File::Spec->rel2abs("$tmpdir/tls.passwd");
  my $auth_group_file = File::Spec->rel2abs("$tmpdir/tls.group");

  my $user = 'proftpd';
  my $passwd = 'test';
  my $home_dir = File::Spec->rel2abs($tmpdir);
  my $uid = 500;
  my $gid = 500;

  # Make sure that, if we're running as root, that the home directory has
  # permissions/privs set for the account we create
  if ($< == 0) {
    unless (chmod(0755, $home_dir)) {
      die("Can't set perms on $home_dir to 0755: $!");
    }

    unless (chown($uid, $gid, $home_dir)) {
      die("Can't set owner of $home_dir to $uid/$gid: $!");
    }
  }

  auth_user_write($auth_user_file, $user, $passwd, $uid, $gid, $home_dir,
    '/bin/bash');
  auth_group_write($auth_group_file, 'ftpd', $gid, $user);

  my $ext_log = File::Spec->rel2abs("$tmpdir/custom.log");

  my $cert_file = File::Spec->rel2abs('t/etc/modules/mod_tls/server-cert.pem');
  my $ca_file = File::Spec->rel2abs('t/etc/modules/mod_tls/ca-cert.pem');

  my $config = {
    PidFile => $pid_file,
    ScoreboardFile => $scoreboard_file,
    SystemLog => $log_file,

    AuthUserFile => $auth_user_file,
    AuthGroupFile => $auth_group_file,

    # Yes, this has a deliberately wrong variable for TLS_CLIENT_S_DN;
    # I use it to make sure that mod_log handles that particular formatting
    # as one would expect (i.e. replace it with an empty string).
    LogFormat => 'custom "FTPS=%{FTPS}e TLS_PROTOCOL=%{TLS_PROTOCOL}e TLS_SESSION_ID=%{TLS_SESSION_ID}e TLS_CIPHER=%{TLS_CIPHER}e TLS_LIBRARY_VERSION=%{TLS_LIBRARY_VERSION}e TLS_CLIENT_S_DN=\'%{env:TLS_CLIENT_S_DN}\'"',
    ExtendedLog => "$ext_log AUTH custom",

    IfModules => {
      'mod_delay.c' => {
        DelayEngine => 'off',
      },

      'mod_tls.c' => {
        TLSEngine => 'on',
        TLSLog => $log_file,
        TLSProtocol => 'SSLv3 TLSv1',
        TLSRequired => 'on',
        TLSRSACertificateFile => $cert_file,
        TLSCACertificateFile => $ca_file,
        TLSOptions => 'StdEnvVars',
      },
    },
  };

  my ($port, $config_user, $config_group) = config_write($config_file, $config);

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
      # Give the server a chance to start up
      sleep(2);

      my $client = Net::FTPSSL->new('127.0.0.1',
        Encryption => 'E',
        Port => $port,
      );

      unless ($client) {
        die("Can't connect to FTPS server: " . IO::Socket::SSL::errstr());
      }

      unless ($client->login($user, $passwd)) {
        die("Can't login: " . $client->last_message());
      }

      $client->quit();
    };

    if ($@) {
      $ex = $@;
    }

    $wfh->print("done\n");
    $wfh->flush();

  } else {
    eval { server_wait($config_file, $rfh) };
    if ($@) {
      warn($@);
      exit 1;
    }

    exit 0;
  }

  # Stop server
  server_stop($pid_file);

  $self->assert_child_ok($pid);

  if ($ex) {
    die($ex);
  }

  # Open the ExtendedLog, and check for the expected environment variables
  if (open(my $fh, "< $ext_log")) {
    my $ok = 0;

    while (my $line = <$fh>) {
      chomp($line);

      if ($line =~ /FTPS=(\d+) TLS_PROTOCOL=(\S+) TLS_SESSION_ID=(\S+) TLS_CIPHER=(\S+) TLS_LIBRARY_VERSION=(.*?) TLS_CLIENT_S_DN=''/) {
        my $ftps = $1;
        my $protocol = $2;
        my $sess_id = $3;
        my $cipher = $4;
        my $libversion = $5;

        $self->assert($ftps eq '1',
          test_msg("Expected '1', got '$ftps'"));
        $self->assert(length($protocol) > 0,
          test_msg("Expected protocol, got '$protocol'"));      
        $self->assert(length($sess_id) > 0,
          test_msg("Expected session ID, got '$sess_id'"));      
        $self->assert(length($cipher) > 0,
          test_msg("Expected cipher, got '$cipher"));      
        $self->assert(length($libversion) > 0,
          test_msg("Expected cipher, got '$libversion"));      

        $ok = 1;
        last;

      } else {
        die("Mismatched ExtendedLog line '$line'");
      }
    }

    close($fh);

    unless ($ok) {
      die("Unexpected environment variable values in ExtendedLog");
    }

  } else {
    die("Can't read $ext_log: $!");
  }

  unlink($log_file);
}

sub tls_opts_std_env_vars_client_vars {
  my $self = shift;
  my $tmpdir = $self->{tmpdir};

  my $config_file = "$tmpdir/tls.conf";
  my $pid_file = File::Spec->rel2abs("$tmpdir/tls.pid");
  my $scoreboard_file = File::Spec->rel2abs("$tmpdir/tls.scoreboard");

  my $log_file = File::Spec->rel2abs('tests.log');

  my $auth_user_file = File::Spec->rel2abs("$tmpdir/tls.passwd");
  my $auth_group_file = File::Spec->rel2abs("$tmpdir/tls.group");

  my $user = 'proftpd';
  my $passwd = 'test';
  my $home_dir = File::Spec->rel2abs($tmpdir);
  my $uid = 500;
  my $gid = 500;

  # Make sure that, if we're running as root, that the home directory has
  # permissions/privs set for the account we create
  if ($< == 0) {
    unless (chmod(0755, $home_dir)) {
      die("Can't set perms on $home_dir to 0755: $!");
    }

    unless (chown($uid, $gid, $home_dir)) {
      die("Can't set owner of $home_dir to $uid/$gid: $!");
    }
  }

  auth_user_write($auth_user_file, $user, $passwd, $uid, $gid, $home_dir,
    '/bin/bash');
  auth_group_write($auth_group_file, 'ftpd', $gid, $user);

  my $ext_log = File::Spec->rel2abs("$tmpdir/custom.log");

  my $client_cert = File::Spec->rel2abs('t/etc/modules/mod_tls/client-cert.pem');
  my $cert_file = File::Spec->rel2abs('t/etc/modules/mod_tls/server-cert.pem');
  my $ca_file = File::Spec->rel2abs('t/etc/modules/mod_tls/ca-cert.pem');

  my $config = {
    PidFile => $pid_file,
    ScoreboardFile => $scoreboard_file,
    SystemLog => $log_file,

    AuthUserFile => $auth_user_file,
    AuthGroupFile => $auth_group_file,

    LogFormat => 'custom "TLS_CLIENT_S_DN=\'%{TLS_CLIENT_S_DN}e\' TLS_CLIENT_I_DN=\'%{TLS_CLIENT_I_DN}e\'"',
    ExtendedLog => "$ext_log AUTH custom",

    IfModules => {
      'mod_delay.c' => {
        DelayEngine => 'off',
      },

      'mod_tls.c' => {
        TLSEngine => 'on',
        TLSLog => $log_file,
        TLSProtocol => 'SSLv3 TLSv1',
        TLSRequired => 'on',
        TLSRSACertificateFile => $cert_file,
        TLSCACertificateFile => $ca_file,
        TLSOptions => 'StdEnvVars',
      },
    },
  };

  my ($port, $config_user, $config_group) = config_write($config_file, $config);

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
      # Give the server a chance to start up
      sleep(2);

      # IO::Socket::SSL options
      my $ssl_opts = {
        SSL_use_cert => 1,
        SSL_cert_file => $client_cert,
        SSL_key_file => $client_cert,
      };

      my $client = Net::FTPSSL->new('127.0.0.1',
        Croak => 1,
        Encryption => 'E',
        Port => $port,
        SSL_Advanced => $ssl_opts,
      );

      unless ($client) {
        die("Can't connect to FTPS server: " . IO::Socket::SSL::errstr());
      }

      unless ($client->login($user, $passwd)) {
        die("Can't login: " . $client->last_message());
      }

      $client->quit();
    };

    if ($@) {
      $ex = $@;
    }

    $wfh->print("done\n");
    $wfh->flush();

  } else {
    eval { server_wait($config_file, $rfh) };
    if ($@) {
      warn($@);
      exit 1;
    }

    exit 0;
  }

  # Stop server
  server_stop($pid_file);

  $self->assert_child_ok($pid);

  if ($ex) {
    die($ex);
  }

  # Open the ExtendedLog, and check for the expected environment variables
  if (open(my $fh, "< $ext_log")) {
    my $ok = 0;

    while (my $line = <$fh>) {
      chomp($line);

      if ($line =~ /TLS_CLIENT_S_DN='(.*?)' TLS_CLIENT_I_DN='(.*?)'/) {
        my $subj_dn = $1;
        my $issuer_dn = $2;

        $self->assert(length($subj_dn) > 0,
          test_msg("Expected subject DN, got '$subj_dn'"));      
        $self->assert(length($issuer_dn) > 0,
          test_msg("Expected issuer DN, got '$issuer_dn'"));      

        $ok = 1;
        last;

      } else {
        die("Mismatched ExtendedLog line '$line'");
      }
    }

    close($fh);

    unless ($ok) {
      die("Unexpected environment variable values in ExtendedLog");
    }

  } else {
    die("Can't read $ext_log: $!");
  }

  unlink($log_file);
}

sub tls_opts_ipaddr_required {
  my $self = shift;
  my $tmpdir = $self->{tmpdir};

  my $config_file = "$tmpdir/tls.conf";
  my $pid_file = File::Spec->rel2abs("$tmpdir/tls.pid");
  my $scoreboard_file = File::Spec->rel2abs("$tmpdir/tls.scoreboard");

  my $log_file = File::Spec->rel2abs('tests.log');

  my $auth_user_file = File::Spec->rel2abs("$tmpdir/tls.passwd");
  my $auth_group_file = File::Spec->rel2abs("$tmpdir/tls.group");

  my $user = 'proftpd';
  my $passwd = 'test';
  my $home_dir = File::Spec->rel2abs($tmpdir);
  my $uid = 500;
  my $gid = 500;

  # Make sure that, if we're running as root, that the home directory has
  # permissions/privs set for the account we create
  if ($< == 0) {
    unless (chmod(0755, $home_dir)) {
      die("Can't set perms on $home_dir to 0755: $!");
    }

    unless (chown($uid, $gid, $home_dir)) {
      die("Can't set owner of $home_dir to $uid/$gid: $!");
    }
  }

  auth_user_write($auth_user_file, $user, $passwd, $uid, $gid, $home_dir,
    '/bin/bash');
  auth_group_write($auth_group_file, 'ftpd', $gid, $user);

  my $client_cert = File::Spec->rel2abs('t/etc/modules/mod_tls/client-cert.pem');
  my $cert_file = File::Spec->rel2abs('t/etc/modules/mod_tls/server-cert.pem');
  my $ca_file = File::Spec->rel2abs('t/etc/modules/mod_tls/ca-cert.pem');

  my $config = {
    PidFile => $pid_file,
    ScoreboardFile => $scoreboard_file,
    SystemLog => $log_file,

    AuthUserFile => $auth_user_file,
    AuthGroupFile => $auth_group_file,

    IfModules => {
      'mod_delay.c' => {
        DelayEngine => 'off',
      },

      'mod_tls.c' => {
        TLSEngine => 'on',
        TLSLog => $log_file,
        TLSProtocol => 'SSLv3 TLSv1',
        TLSRequired => 'on',
        TLSRSACertificateFile => $cert_file,
        TLSCACertificateFile => $ca_file,
        TLSOptions => 'iPAddressRequired',
        TLSVerifyClient => 'on',
      },
    },
  };

  my ($port, $config_user, $config_group) = config_write($config_file, $config);

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
      # Give the server a chance to start up
      sleep(2);

      # IO::Socket::SSL options
      my $ssl_opts = {
        SSL_use_cert => 1,
        SSL_cert_file => $client_cert,
        SSL_key_file => $client_cert,
      };

      my $client;
      eval {
        $client = Net::FTPSSL->new('127.0.0.1',
          Croak => 1,
          Encryption => 'E',
          Port => $port,
        );
      };

      unless ($@) { 
        die("Connection to server succeeded unexpectedly");
      }

      $client = Net::FTPSSL->new('127.0.0.1',
        Croak => 1,
        Encryption => 'E',
        Port => $port,
        SSL_Advanced => $ssl_opts,
      );

      unless ($client->login($user, $passwd)) {
        die("Can't login: " . $client->last_message());
      }

      $client->quit();
    };

    if ($@) {
      $ex = $@;
    }

    $wfh->print("done\n");
    $wfh->flush();

  } else {
    eval { server_wait($config_file, $rfh) };
    if ($@) {
      warn($@);
      exit 1;
    }

    exit 0;
  }

  # Stop server
  server_stop($pid_file);

  $self->assert_child_ok($pid);

  if ($ex) {
    die($ex);
  }

  unlink($log_file);
}

sub tls_opts_allow_per_user_tlsrequired_on_anon_login_bug3325 {
  my $self = shift;
  my $tmpdir = $self->{tmpdir};

  my $config_file = "$tmpdir/tls.conf";
  my $pid_file = File::Spec->rel2abs("$tmpdir/tls.pid");
  my $scoreboard_file = File::Spec->rel2abs("$tmpdir/tls.scoreboard");

  my $log_file = File::Spec->rel2abs('tests.log');

  my $auth_user_file = File::Spec->rel2abs("$tmpdir/tls.passwd");
  my $auth_group_file = File::Spec->rel2abs("$tmpdir/tls.group");

  my $user = 'proftpd';
  my $passwd = 'test';
  my $home_dir = File::Spec->rel2abs($tmpdir);
  my $uid = 500;
  my $gid = 500;

  # Make sure that, if we're running as root, that the home directory has
  # permissions/privs set for the account we create
  if ($< == 0) {
    unless (chmod(0755, $home_dir)) {
      die("Can't set perms on $home_dir to 0755: $!");
    }

    unless (chown($uid, $gid, $home_dir)) {
      die("Can't set owner of $home_dir to $uid/$gid: $!");
    }
  }

  auth_user_write($auth_user_file, $user, $passwd, $uid, $gid, $home_dir,
    '/bin/bash');
  auth_group_write($auth_group_file, 'ftpd', $gid, $user);

  my $client_cert = File::Spec->rel2abs('t/etc/modules/mod_tls/client-cert.pem');
  my $cert_file = File::Spec->rel2abs('t/etc/modules/mod_tls/server-cert.pem');
  my $ca_file = File::Spec->rel2abs('t/etc/modules/mod_tls/ca-cert.pem');

  my $config = {
    PidFile => $pid_file,
    ScoreboardFile => $scoreboard_file,
    SystemLog => $log_file,

    AuthUserFile => $auth_user_file,
    AuthGroupFile => $auth_group_file,

    IfModules => {
      'mod_delay.c' => {
        DelayEngine => 'off',
      },

      'mod_tls.c' => {
        TLSEngine => 'on',
        TLSLog => $log_file,
        TLSProtocol => 'SSLv3 TLSv1',
        TLSRequired => 'on',
        TLSRSACertificateFile => $cert_file,
        TLSCACertificateFile => $ca_file,
        TLSOptions => 'AllowPerUser',
      },
    },
  };

  my ($port, $config_user, $config_group) = config_write($config_file, $config);

  my ($anon_user, $anon_group) = config_get_identity();

  if (open(my $fh, ">> $config_file")) {
    print $fh <<EOC;
<Anonymous $home_dir>
  User $anon_user
  Group $anon_group
  UserAlias anonymous $anon_user
  RequireValidShell off
  TLSRequired off
</Anonymous>
EOC
    unless (close($fh)) {
      die("Can't write $config_file: $!");
    }

  } else {
    die("Can't open $config_file: $!");
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
      # Give the server a chance to start up
      sleep(2);

      my $client = ProFTPD::TestSuite::FTP->new('127.0.0.1', $port);

      # We should be able to connect to the server without SSL, and login
      # anonymously without it.
      my ($resp_code, $resp_msg) = $client->login('anonymous', 'ftp@nospam.org');

      my $expected;

      $expected = 230;
      $self->assert($expected == $resp_code,
        test_msg("Expected $expected, got $resp_code"));

      $expected = 'Anonymous access granted, restrictions apply';
      $self->assert($expected eq $resp_msg,
        test_msg("Expected '$expected', got '$resp_msg'"));

      $client->quit();
    };

    if ($@) {
      $ex = $@;
    }

    $wfh->print("done\n");
    $wfh->flush();

  } else {
    eval { server_wait($config_file, $rfh) };
    if ($@) {
      warn($@);
      exit 1;
    }

    exit 0;
  }

  # Stop server
  server_stop($pid_file);

  $self->assert_child_ok($pid);

  if ($ex) {
    die($ex);
  }

  unlink($log_file);
}

sub tls_opts_allow_per_user_tlsrequired_on_user_login_bug3325 {
  my $self = shift;
  my $tmpdir = $self->{tmpdir};

  my $config_file = "$tmpdir/tls.conf";
  my $pid_file = File::Spec->rel2abs("$tmpdir/tls.pid");
  my $scoreboard_file = File::Spec->rel2abs("$tmpdir/tls.scoreboard");

  my $log_file = File::Spec->rel2abs('tests.log');

  my $auth_user_file = File::Spec->rel2abs("$tmpdir/tls.passwd");
  my $auth_group_file = File::Spec->rel2abs("$tmpdir/tls.group");

  my $user = 'proftpd';
  my $passwd = 'test';
  my $home_dir = File::Spec->rel2abs($tmpdir);
  my $uid = 500;
  my $gid = 500;

  # Make sure that, if we're running as root, that the home directory has
  # permissions/privs set for the account we create
  if ($< == 0) {
    unless (chmod(0755, $home_dir)) {
      die("Can't set perms on $home_dir to 0755: $!");
    }

    unless (chown($uid, $gid, $home_dir)) {
      die("Can't set owner of $home_dir to $uid/$gid: $!");
    }
  }

  auth_user_write($auth_user_file, $user, $passwd, $uid, $gid, $home_dir,
    '/bin/bash');
  auth_group_write($auth_group_file, 'ftpd', $gid, $user);

  my $client_cert = File::Spec->rel2abs('t/etc/modules/mod_tls/client-cert.pem');
  my $cert_file = File::Spec->rel2abs('t/etc/modules/mod_tls/server-cert.pem');
  my $ca_file = File::Spec->rel2abs('t/etc/modules/mod_tls/ca-cert.pem');

  my $config = {
    PidFile => $pid_file,
    ScoreboardFile => $scoreboard_file,
    SystemLog => $log_file,

    AuthUserFile => $auth_user_file,
    AuthGroupFile => $auth_group_file,

    IfModules => {
      'mod_delay.c' => {
        DelayEngine => 'off',
      },

      'mod_tls.c' => {
        TLSEngine => 'on',
        TLSLog => $log_file,
        TLSProtocol => 'SSLv3 TLSv1',
        TLSRequired => 'on',
        TLSRSACertificateFile => $cert_file,
        TLSCACertificateFile => $ca_file,
        TLSOptions => 'AllowPerUser',
      },
    },
  };

  my ($port, $config_user, $config_group) = config_write($config_file, $config);

  my ($anon_user, $anon_group) = config_get_identity();

  if (open(my $fh, ">> $config_file")) {
    print $fh <<EOC;
<Anonymous $home_dir>
  User $anon_user
  Group $anon_group
  UserAlias anonymous $anon_user
  RequireValidShell off
  TLSRequired off
</Anonymous>
EOC
    unless (close($fh)) {
      die("Can't write $config_file: $!");
    }

  } else {
    die("Can't open $config_file: $!");
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
      # Give the server a chance to start up
      sleep(2);

      # With this config, we should not be able to login without SSL/TLS
      my $client = ProFTPD::TestSuite::FTP->new('127.0.0.1', $port);

      eval { $client->login($user, $passwd) };
      unless ($@) {
        die("Login succeeded unexpectedly");
      }

      my $resp_code = $client->response_code();
      my $resp_msg = $client->response_msg();

      my $expected;

      $expected = 530;
      $self->assert($expected == $resp_code,
        test_msg("Expected $expected, got $resp_code"));

      $expected = "Login incorrect.";
      $self->assert($expected eq $resp_msg,
        test_msg("Expected '$expected', got '$resp_msg'"));
    };

    if ($@) {
      $ex = $@;
    }

    $wfh->print("done\n");
    $wfh->flush();

  } else {
    eval { server_wait($config_file, $rfh) };
    if ($@) {
      warn($@);
      exit 1;
    }

    exit 0;
  }

  # Stop server
  server_stop($pid_file);

  $self->assert_child_ok($pid);

  if ($ex) {
    die($ex);
  }

  unlink($log_file);
}

sub tls_opts_allow_per_user_tlsrequired_auth_user_login_bug3325 {
  my $self = shift;
  my $tmpdir = $self->{tmpdir};

  my $config_file = "$tmpdir/tls.conf";
  my $pid_file = File::Spec->rel2abs("$tmpdir/tls.pid");
  my $scoreboard_file = File::Spec->rel2abs("$tmpdir/tls.scoreboard");

  my $log_file = File::Spec->rel2abs('tests.log');

  my $auth_user_file = File::Spec->rel2abs("$tmpdir/tls.passwd");
  my $auth_group_file = File::Spec->rel2abs("$tmpdir/tls.group");

  my $user = 'proftpd';
  my $passwd = 'test';
  my $home_dir = File::Spec->rel2abs($tmpdir);
  my $uid = 500;
  my $gid = 500;

  # Make sure that, if we're running as root, that the home directory has
  # permissions/privs set for the account we create
  if ($< == 0) {
    unless (chmod(0755, $home_dir)) {
      die("Can't set perms on $home_dir to 0755: $!");
    }

    unless (chown($uid, $gid, $home_dir)) {
      die("Can't set owner of $home_dir to $uid/$gid: $!");
    }
  }

  auth_user_write($auth_user_file, $user, $passwd, $uid, $gid, $home_dir,
    '/bin/bash');
  auth_group_write($auth_group_file, 'ftpd', $gid, $user);

  my $client_cert = File::Spec->rel2abs('t/etc/modules/mod_tls/client-cert.pem');
  my $cert_file = File::Spec->rel2abs('t/etc/modules/mod_tls/server-cert.pem');
  my $ca_file = File::Spec->rel2abs('t/etc/modules/mod_tls/ca-cert.pem');

  my $config = {
    PidFile => $pid_file,
    ScoreboardFile => $scoreboard_file,
    SystemLog => $log_file,

    AuthUserFile => $auth_user_file,
    AuthGroupFile => $auth_group_file,

    IfModules => {
      'mod_delay.c' => {
        DelayEngine => 'off',
      },

      'mod_tls.c' => {
        TLSEngine => 'on',
        TLSLog => $log_file,
        TLSProtocol => 'SSLv3 TLSv1',
        TLSRequired => 'auth',
        TLSRSACertificateFile => $cert_file,
        TLSCACertificateFile => $ca_file,
        TLSOptions => 'AllowPerUser',
      },
    },
  };

  my ($port, $config_user, $config_group) = config_write($config_file, $config);

  my ($anon_user, $anon_group) = config_get_identity();

  if (open(my $fh, ">> $config_file")) {
    print $fh <<EOC;
<Anonymous $home_dir>
  User $anon_user
  Group $anon_group
  UserAlias anonymous $anon_user
  RequireValidShell off
  TLSRequired off
</Anonymous>
EOC
    unless (close($fh)) {
      die("Can't write $config_file: $!");
    }

  } else {
    die("Can't open $config_file: $!");
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
      # Give the server a chance to start up
      sleep(2);

      # With this config, we should not be able to login without SSL/TLS
      my $client = ProFTPD::TestSuite::FTP->new('127.0.0.1', $port);

      eval { $client->login($user, $passwd) };
      unless ($@) {
        die("Login succeeded unexpectedly");
      }

      my $resp_code = $client->response_code();
      my $resp_msg = $client->response_msg();

      my $expected;

      $expected = 530;
      $self->assert($expected == $resp_code,
        test_msg("Expected $expected, got $resp_code"));

      $expected = "Login incorrect.";
      $self->assert($expected eq $resp_msg,
        test_msg("Expected '$expected', got '$resp_msg'"));
    };

    if ($@) {
      $ex = $@;
    }

    $wfh->print("done\n");
    $wfh->flush();

  } else {
    eval { server_wait($config_file, $rfh) };
    if ($@) {
      warn($@);
      exit 1;
    }

    exit 0;
  }

  # Stop server
  server_stop($pid_file);

  $self->assert_child_ok($pid);

  if ($ex) {
    die($ex);
  }

  unlink($log_file);
}

sub tls_opts_allow_per_user_tlsrequired_ctrl_user_login_bug3325 {
  my $self = shift;
  my $tmpdir = $self->{tmpdir};

  my $config_file = "$tmpdir/tls.conf";
  my $pid_file = File::Spec->rel2abs("$tmpdir/tls.pid");
  my $scoreboard_file = File::Spec->rel2abs("$tmpdir/tls.scoreboard");

  my $log_file = File::Spec->rel2abs('tests.log');

  my $auth_user_file = File::Spec->rel2abs("$tmpdir/tls.passwd");
  my $auth_group_file = File::Spec->rel2abs("$tmpdir/tls.group");

  my $user = 'proftpd';
  my $passwd = 'test';
  my $home_dir = File::Spec->rel2abs($tmpdir);
  my $uid = 500;
  my $gid = 500;

  # Make sure that, if we're running as root, that the home directory has
  # permissions/privs set for the account we create
  if ($< == 0) {
    unless (chmod(0755, $home_dir)) {
      die("Can't set perms on $home_dir to 0755: $!");
    }

    unless (chown($uid, $gid, $home_dir)) {
      die("Can't set owner of $home_dir to $uid/$gid: $!");
    }
  }

  auth_user_write($auth_user_file, $user, $passwd, $uid, $gid, $home_dir,
    '/bin/bash');
  auth_group_write($auth_group_file, 'ftpd', $gid, $user);

  my $client_cert = File::Spec->rel2abs('t/etc/modules/mod_tls/client-cert.pem');
  my $cert_file = File::Spec->rel2abs('t/etc/modules/mod_tls/server-cert.pem');
  my $ca_file = File::Spec->rel2abs('t/etc/modules/mod_tls/ca-cert.pem');

  my $config = {
    PidFile => $pid_file,
    ScoreboardFile => $scoreboard_file,
    SystemLog => $log_file,

    AuthUserFile => $auth_user_file,
    AuthGroupFile => $auth_group_file,

    IfModules => {
      'mod_delay.c' => {
        DelayEngine => 'off',
      },

      'mod_tls.c' => {
        TLSEngine => 'on',
        TLSLog => $log_file,
        TLSProtocol => 'SSLv3 TLSv1',
        TLSRequired => 'ctrl',
        TLSRSACertificateFile => $cert_file,
        TLSCACertificateFile => $ca_file,
        TLSOptions => 'AllowPerUser',
      },
    },
  };

  my ($port, $config_user, $config_group) = config_write($config_file, $config);

  my ($anon_user, $anon_group) = config_get_identity();

  if (open(my $fh, ">> $config_file")) {
    print $fh <<EOC;
<Anonymous $home_dir>
  User $anon_user
  Group $anon_group
  UserAlias anonymous $anon_user
  RequireValidShell off
  TLSRequired off
</Anonymous>
EOC
    unless (close($fh)) {
      die("Can't write $config_file: $!");
    }

  } else {
    die("Can't open $config_file: $!");
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
      # Give the server a chance to start up
      sleep(2);

      # With this config, we should not be able to login without SSL/TLS
      my $client = ProFTPD::TestSuite::FTP->new('127.0.0.1', $port);

      eval { $client->login($user, $passwd) };
      unless ($@) {
        die("Login succeeded unexpectedly");
      }

      my $resp_code = $client->response_code();
      my $resp_msg = $client->response_msg();

      my $expected;

      $expected = 530;
      $self->assert($expected == $resp_code,
        test_msg("Expected $expected, got $resp_code"));

      $expected = "Login incorrect.";
      $self->assert($expected eq $resp_msg,
        test_msg("Expected '$expected', got '$resp_msg'"));
    };

    if ($@) {
      $ex = $@;
    }

    $wfh->print("done\n");
    $wfh->flush();

  } else {
    eval { server_wait($config_file, $rfh) };
    if ($@) {
      warn($@);
      exit 1;
    }

    exit 0;
  }

  # Stop server
  server_stop($pid_file);

  $self->assert_child_ok($pid);

  if ($ex) {
    die($ex);
  }

  unlink($log_file);
}

sub tls_opts_allow_per_user_tlsrequired_data_user_login_bug3325 {
  my $self = shift;
  my $tmpdir = $self->{tmpdir};

  my $config_file = "$tmpdir/tls.conf";
  my $pid_file = File::Spec->rel2abs("$tmpdir/tls.pid");
  my $scoreboard_file = File::Spec->rel2abs("$tmpdir/tls.scoreboard");

  my $log_file = File::Spec->rel2abs('tests.log');

  my $auth_user_file = File::Spec->rel2abs("$tmpdir/tls.passwd");
  my $auth_group_file = File::Spec->rel2abs("$tmpdir/tls.group");

  my $user = 'proftpd';
  my $passwd = 'test';
  my $home_dir = File::Spec->rel2abs($tmpdir);
  my $uid = 500;
  my $gid = 500;

  # Make sure that, if we're running as root, that the home directory has
  # permissions/privs set for the account we create
  if ($< == 0) {
    unless (chmod(0755, $home_dir)) {
      die("Can't set perms on $home_dir to 0755: $!");
    }

    unless (chown($uid, $gid, $home_dir)) {
      die("Can't set owner of $home_dir to $uid/$gid: $!");
    }
  }

  auth_user_write($auth_user_file, $user, $passwd, $uid, $gid, $home_dir,
    '/bin/bash');
  auth_group_write($auth_group_file, 'ftpd', $gid, $user);

  my $client_cert = File::Spec->rel2abs('t/etc/modules/mod_tls/client-cert.pem');
  my $cert_file = File::Spec->rel2abs('t/etc/modules/mod_tls/server-cert.pem');
  my $ca_file = File::Spec->rel2abs('t/etc/modules/mod_tls/ca-cert.pem');

  my $config = {
    PidFile => $pid_file,
    ScoreboardFile => $scoreboard_file,
    SystemLog => $log_file,

    AuthUserFile => $auth_user_file,
    AuthGroupFile => $auth_group_file,

    IfModules => {
      'mod_delay.c' => {
        DelayEngine => 'off',
      },

      'mod_tls.c' => {
        TLSEngine => 'on',
        TLSLog => $log_file,
        TLSProtocol => 'SSLv3 TLSv1',
        TLSRequired => 'data',
        TLSRSACertificateFile => $cert_file,
        TLSCACertificateFile => $ca_file,
        TLSOptions => 'AllowPerUser',
      },
    },
  };

  my ($port, $config_user, $config_group) = config_write($config_file, $config);

  my ($anon_user, $anon_group) = config_get_identity();

  if (open(my $fh, ">> $config_file")) {
    print $fh <<EOC;
<Anonymous $home_dir>
  User $anon_user
  Group $anon_group
  UserAlias anonymous $anon_user
  RequireValidShell off
  TLSRequired off
</Anonymous>
EOC
    unless (close($fh)) {
      die("Can't write $config_file: $!");
    }

  } else {
    die("Can't open $config_file: $!");
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
      # Give the server a chance to start up
      sleep(2);

      # With this config, we should not be able to login without SSL/TLS
      my $client = ProFTPD::TestSuite::FTP->new('127.0.0.1', $port);

      my ($resp_code, $resp_msg) = $client->login($user, $passwd);

      my $expected;

      $expected = 230;
      $self->assert($expected == $resp_code,
        test_msg("Expected $expected, got $resp_code"));

      $expected = "User $user logged in";
      $self->assert($expected eq $resp_msg,
        test_msg("Expected '$expected', got '$resp_msg'"));
    };

    if ($@) {
      $ex = $@;
    }

    $wfh->print("done\n");
    $wfh->flush();

  } else {
    eval { server_wait($config_file, $rfh) };
    if ($@) {
      warn($@);
      exit 1;
    }

    exit 0;
  }

  # Stop server
  server_stop($pid_file);

  $self->assert_child_ok($pid);

  if ($ex) {
    die($ex);
  }

  unlink($log_file);
}

sub tls_opts_allow_per_user_tlsrequired_on_ifsess_login_bug3325 {
  my $self = shift;
  my $tmpdir = $self->{tmpdir};

  my $config_file = "$tmpdir/tls.conf";
  my $pid_file = File::Spec->rel2abs("$tmpdir/tls.pid");
  my $scoreboard_file = File::Spec->rel2abs("$tmpdir/tls.scoreboard");

  my $log_file = File::Spec->rel2abs('tests.log');

  my $auth_user_file = File::Spec->rel2abs("$tmpdir/tls.passwd");
  my $auth_group_file = File::Spec->rel2abs("$tmpdir/tls.group");

  my $user = 'proftpd';
  my $passwd = 'test';
  my $home_dir = File::Spec->rel2abs($tmpdir);
  my $uid = 500;
  my $gid = 500;

  # Make sure that, if we're running as root, that the home directory has
  # permissions/privs set for the account we create
  if ($< == 0) {
    unless (chmod(0755, $home_dir)) {
      die("Can't set perms on $home_dir to 0755: $!");
    }

    unless (chown($uid, $gid, $home_dir)) {
      die("Can't set owner of $home_dir to $uid/$gid: $!");
    }
  }

  auth_user_write($auth_user_file, $user, $passwd, $uid, $gid, $home_dir,
    '/bin/bash');
  auth_group_write($auth_group_file, 'ftpd', $gid, $user);

  my $client_cert = File::Spec->rel2abs('t/etc/modules/mod_tls/client-cert.pem');
  my $cert_file = File::Spec->rel2abs('t/etc/modules/mod_tls/server-cert.pem');
  my $ca_file = File::Spec->rel2abs('t/etc/modules/mod_tls/ca-cert.pem');

  my $config = {
    PidFile => $pid_file,
    ScoreboardFile => $scoreboard_file,
    SystemLog => $log_file,

    AuthUserFile => $auth_user_file,
    AuthGroupFile => $auth_group_file,

    IfModules => {
      'mod_delay.c' => {
        DelayEngine => 'off',
      },

      'mod_tls.c' => {
        TLSEngine => 'on',
        TLSLog => $log_file,
        TLSProtocol => 'SSLv3 TLSv1',
        TLSRequired => 'on',
        TLSRSACertificateFile => $cert_file,
        TLSCACertificateFile => $ca_file,
        TLSOptions => 'AllowPerUser',
      },
    },
  };

  my ($port, $config_user, $config_group) = config_write($config_file, $config);

  my ($anon_user, $anon_group) = config_get_identity();

  if (open(my $fh, ">> $config_file")) {
    print $fh <<EOC;
<IfModule mod_ifsession.c>
  <IfUser $user>
    TLSRequired off
  </IfUser>
</IfModule>
EOC
    unless (close($fh)) {
      die("Can't write $config_file: $!");
    }

  } else {
    die("Can't open $config_file: $!");
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
      # Give the server a chance to start up
      sleep(2);

      my $client = ProFTPD::TestSuite::FTP->new('127.0.0.1', $port);

      my ($resp_code, $resp_msg) = $client->login($user, $passwd);
      my $expected;

      $expected = 230;
      $self->assert($expected == $resp_code,
        test_msg("Expected $expected, got $resp_code"));

      $expected = "User $user logged in";
      $self->assert($expected eq $resp_msg,
        test_msg("Expected '$expected', got '$resp_msg'"));

      $client->quit();
    };

    if ($@) {
      $ex = $@;
    }

    $wfh->print("done\n");
    $wfh->flush();

  } else {
    eval { server_wait($config_file, $rfh) };
    if ($@) {
      warn($@);
      exit 1;
    }

    exit 0;
  }

  # Stop server
  server_stop($pid_file);

  $self->assert_child_ok($pid);

  if ($ex) {
    die($ex);
  }

  unlink($log_file);
}

sub tls_rest_2gb_last_byte {
  my $self = shift;
  my $tmpdir = $self->{tmpdir};

  my $config_file = "$tmpdir/tls.conf";
  my $pid_file = File::Spec->rel2abs("$tmpdir/tls.pid");
  my $scoreboard_file = File::Spec->rel2abs("$tmpdir/tls.scoreboard");

  my $log_file = File::Spec->rel2abs('tests.log');

  my $auth_user_file = File::Spec->rel2abs("$tmpdir/tls.passwd");
  my $auth_group_file = File::Spec->rel2abs("$tmpdir/tls.group");

  my $user = 'proftpd';
  my $passwd = 'test';
  my $home_dir = File::Spec->rel2abs($tmpdir);
  my $uid = 500;
  my $gid = 500;

  # Make sure that, if we're running as root, that the home directory has
  # permissions/privs set for the account we create
  if ($< == 0) {
    unless (chmod(0755, $home_dir)) {
      die("Can't set perms on $home_dir to 0755: $!");
    }

    unless (chown($uid, $gid, $home_dir)) {
      die("Can't set owner of $home_dir to $uid/$gid: $!");
    }
  }

  auth_user_write($auth_user_file, $user, $passwd, $uid, $gid, $home_dir,
    '/bin/bash');
  auth_group_write($auth_group_file, 'ftpd', $gid, $user);

  my $client_cert = File::Spec->rel2abs('t/etc/modules/mod_tls/client-cert.pem');
  my $cert_file = File::Spec->rel2abs('t/etc/modules/mod_tls/server-cert.pem');
  my $ca_file = File::Spec->rel2abs('t/etc/modules/mod_tls/ca-cert.pem');

  # According to:
  #
  #  http://trac.filezilla-project.org/ticket/4487
  #
  # FileZilla tests whether servers support resuming transfers over 2GB by
  # (IMHO stupidly) seeking to just before the last byte of the file on the
  # server, and expecting just a single byte.
  #
  # Make sure that proftpd does The Right Thing(tm) in a regression test for
  # this use case.

  # Create a file that is 2GB plus 24 bytes.
  my $test_len = (2 ** 31) + 24;
  my $test_file = File::Spec->rel2abs("$tmpdir/test.txt");
  if (open(my $fh, "> $test_file")) {

    # Seek to the 2GB limit, then fill the rest with 'A'
    unless (seek($fh, (2 ** 31), 0)) {
       die("Can't seek to 2GB length: $!");
    }

    print $fh "A" x 24;

    unless (close($fh)) {
      die("Can't write $test_file: $!");
    }

  } else {
    die("Can't open $test_file: $!");
  }

  my $tmp_file = File::Spec->rel2abs("$tmpdir/test.tmp");
  my $tmp_fh;
  unless (open($tmp_fh, "> $tmp_file")) {
    die("Can't open $tmp_file: $!");
  }
 
  my $config = {
    PidFile => $pid_file,
    ScoreboardFile => $scoreboard_file,
    SystemLog => $log_file,

    AuthUserFile => $auth_user_file,
    AuthGroupFile => $auth_group_file,

    IfModules => {
      'mod_delay.c' => {
        DelayEngine => 'off',
      },

      'mod_tls.c' => {
        TLSEngine => 'on',
        TLSLog => $log_file,
        TLSProtocol => 'SSLv3 TLSv1',
        TLSRequired => 'on',
        TLSRSACertificateFile => $cert_file,
        TLSCACertificateFile => $ca_file,
        TLSOptions => 'NoSessionReuseRequired',
      },
    },
  };

  my ($port, $config_user, $config_group) = config_write($config_file, $config);

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
      # Give the server a chance to start up
      sleep(2);

      my $client = Net::FTPSSL->new('127.0.0.1',
        Encryption => 'E',
        Port => $port,
      );

      unless ($client) {
        die("Can't connect to FTPS server: " . IO::Socket::SSL::errstr());
      }

      unless ($client->login($user, $passwd)) {
        die("Can't login: " . $client->last_message());
      }

      unless ($client->binary()) {
        die("Can't set TYPE to binary: " . $client->last_message());
      }

      my $rest_len = $test_len - 1;

      unless ($client->quot('REST', $rest_len)) {
        die("Can't REST $rest_len: " . $client->last_message());
      }

      my $resp_msg = $client->last_message();
      my $expected;

      $expected = "350 Restarting at $rest_len. Send STORE or RETRIEVE to initiate transfer";
      $self->assert($expected eq $resp_msg,
        test_msg("Expected '$expected', got '$resp_msg'"));

      unless ($client->get($test_file, $tmp_fh)) {
        die("Can't RETR $test_file: " . $client->last_message());
      }

      $resp_msg = $client->last_message();
      $expected = "226 Transfer complete";
      $self->assert($expected eq $resp_msg,
        test_msg("Expected '$expected', got '$resp_msg'"));

      close($tmp_fh);

      my $buflen = (stat($tmp_file))[7];
      $expected = 1;
      $self->assert($expected == $buflen,
        test_msg("Expected $expected, got $buflen"));
    };

    if ($@) {
      $ex = $@;
    }

    $wfh->print("done\n");
    $wfh->flush();

  } else {
    eval { server_wait($config_file, $rfh) };
    if ($@) {
      warn($@);
      exit 1;
    }

    exit 0;
  }

  # Stop server
  server_stop($pid_file);

  $self->assert_child_ok($pid);

  if ($ex) {
    die($ex);
  }

  unlink($log_file);
}

sub tls_rest_4gb_last_byte {
  my $self = shift;
  my $tmpdir = $self->{tmpdir};

  my $config_file = "$tmpdir/tls.conf";
  my $pid_file = File::Spec->rel2abs("$tmpdir/tls.pid");
  my $scoreboard_file = File::Spec->rel2abs("$tmpdir/tls.scoreboard");

  my $log_file = File::Spec->rel2abs('tests.log');

  my $auth_user_file = File::Spec->rel2abs("$tmpdir/tls.passwd");
  my $auth_group_file = File::Spec->rel2abs("$tmpdir/tls.group");

  my $user = 'proftpd';
  my $passwd = 'test';
  my $home_dir = File::Spec->rel2abs($tmpdir);
  my $uid = 500;
  my $gid = 500;

  # Make sure that, if we're running as root, that the home directory has
  # permissions/privs set for the account we create
  if ($< == 0) {
    unless (chmod(0755, $home_dir)) {
      die("Can't set perms on $home_dir to 0755: $!");
    }

    unless (chown($uid, $gid, $home_dir)) {
      die("Can't set owner of $home_dir to $uid/$gid: $!");
    }
  }

  auth_user_write($auth_user_file, $user, $passwd, $uid, $gid, $home_dir,
    '/bin/bash');
  auth_group_write($auth_group_file, 'ftpd', $gid, $user);

  my $client_cert = File::Spec->rel2abs('t/etc/modules/mod_tls/client-cert.pem');
  my $cert_file = File::Spec->rel2abs('t/etc/modules/mod_tls/server-cert.pem');
  my $ca_file = File::Spec->rel2abs('t/etc/modules/mod_tls/ca-cert.pem');

  # According to:
  #
  #  http://trac.filezilla-project.org/ticket/4487
  #
  # FileZilla tests whether servers support resuming transfers over 4GB by
  # (IMHO stupidly) seeking to just before the last byte of the file on the
  # server, and expecting just a single byte.
  #
  # Make sure that proftpd does The Right Thing(tm) in a regression test for
  # this use case.

  # Create a file that is 4GB plus 24 bytes.
  my $test_len = (2 ** 32) + 24;
  my $test_file = File::Spec->rel2abs("$tmpdir/test.txt");
  if (open(my $fh, "> $test_file")) {

    # Seek to the 4GB limit, then fill the rest with 'A'
    unless (seek($fh, (2 ** 32), 0)) {
       die("Can't seek to 4GB length: $!");
    }

    print $fh "A" x 24;

    unless (close($fh)) {
      die("Can't write $test_file: $!");
    }

  } else {
    die("Can't open $test_file: $!");
  }

  my $tmp_file = File::Spec->rel2abs("$tmpdir/test.tmp");
  my $tmp_fh;
  unless (open($tmp_fh, "> $tmp_file")) {
    die("Can't open $tmp_file: $!");
  }
 
  my $config = {
    PidFile => $pid_file,
    ScoreboardFile => $scoreboard_file,
    SystemLog => $log_file,

    AuthUserFile => $auth_user_file,
    AuthGroupFile => $auth_group_file,

    IfModules => {
      'mod_delay.c' => {
        DelayEngine => 'off',
      },

      'mod_tls.c' => {
        TLSEngine => 'on',
        TLSLog => $log_file,
        TLSProtocol => 'SSLv3 TLSv1',
        TLSRequired => 'on',
        TLSRSACertificateFile => $cert_file,
        TLSCACertificateFile => $ca_file,
        TLSOptions => 'NoSessionReuseRequired',
      },
    },
  };

  my ($port, $config_user, $config_group) = config_write($config_file, $config);

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
      # Give the server a chance to start up
      sleep(2);

      my $client = Net::FTPSSL->new('127.0.0.1',
        Encryption => 'E',
        Port => $port,
      );

      unless ($client) {
        die("Can't connect to FTPS server: " . IO::Socket::SSL::errstr());
      }

      unless ($client->login($user, $passwd)) {
        die("Can't login: " . $client->last_message());
      }

      unless ($client->binary()) {
        die("Can't set TYPE to binary: " . $client->last_message());
      }

      my $rest_len = $test_len - 1;

      unless ($client->quot('REST', $rest_len)) {
        die("Can't REST $rest_len: " . $client->last_message());
      }

      my $resp_msg = $client->last_message();
      my $expected;

      $expected = "350 Restarting at $rest_len. Send STORE or RETRIEVE to initiate transfer";
      $self->assert($expected eq $resp_msg,
        test_msg("Expected '$expected', got '$resp_msg'"));

      unless ($client->get($test_file, $tmp_fh)) {
        die("Can't RETR $test_file: " . $client->last_message());
      }

      $resp_msg = $client->last_message();
      $expected = "226 Transfer complete";
      $self->assert($expected eq $resp_msg,
        test_msg("Expected '$expected', got '$resp_msg'"));

      close($tmp_fh);

      my $buflen = (stat($tmp_file))[7];
      $expected = 1;
      $self->assert($expected == $buflen,
        test_msg("Expected $expected, got $buflen"));
    };

    if ($@) {
      $ex = $@;
    }

    $wfh->print("done\n");
    $wfh->flush();

  } else {
    eval { server_wait($config_file, $rfh) };
    if ($@) {
      warn($@);
      exit 1;
    }

    exit 0;
  }

  # Stop server
  server_stop($pid_file);

  $self->assert_child_ok($pid);

  if ($ex) {
    die($ex);
  }

  unlink($log_file);
}

sub tls_stor_empty_file {
  my $self = shift;
  my $tmpdir = $self->{tmpdir};

  my $config_file = "$tmpdir/tls.conf";
  my $pid_file = File::Spec->rel2abs("$tmpdir/tls.pid");
  my $scoreboard_file = File::Spec->rel2abs("$tmpdir/tls.scoreboard");

  my $log_file = File::Spec->rel2abs('tests.log');

  my $auth_user_file = File::Spec->rel2abs("$tmpdir/tls.passwd");
  my $auth_group_file = File::Spec->rel2abs("$tmpdir/tls.group");

  my $user = 'proftpd';
  my $passwd = 'test';
  my $home_dir = File::Spec->rel2abs($tmpdir);
  my $uid = 500;
  my $gid = 500;

  # Make sure that, if we're running as root, that the home directory has
  # permissions/privs set for the account we create
  if ($< == 0) {
    unless (chmod(0755, $home_dir)) {
      die("Can't set perms on $home_dir to 0755: $!");
    }

    unless (chown($uid, $gid, $home_dir)) {
      die("Can't set owner of $home_dir to $uid/$gid: $!");
    }
  }

  auth_user_write($auth_user_file, $user, $passwd, $uid, $gid, $home_dir,
    '/bin/bash');
  auth_group_write($auth_group_file, 'ftpd', $gid, $user);

  my $cert_file = File::Spec->rel2abs('t/etc/modules/mod_tls/server-cert.pem');
  my $ca_file = File::Spec->rel2abs('t/etc/modules/mod_tls/ca-cert.pem');

  my $config = {
    PidFile => $pid_file,
    ScoreboardFile => $scoreboard_file,
    SystemLog => $log_file,

    AuthUserFile => $auth_user_file,
    AuthGroupFile => $auth_group_file,

    IfModules => {
      'mod_delay.c' => {
        DelayEngine => 'off',
      },

      'mod_tls.c' => {
        TLSEngine => 'on',
        TLSLog => $log_file,
        TLSProtocol => 'SSLv3 TLSv1',
        TLSRequired => 'on',
        TLSRSACertificateFile => $cert_file,
        TLSCACertificateFile => $ca_file,
      },
    },
  };

  my ($port, $config_user, $config_group) = config_write($config_file, $config);

  my $empty_file = File::Spec->rel2abs("$tmpdir/empty.txt");
  if (open(my $fh, "> $empty_file")) {
    close($fh);

  } else {
    die("Can't open $empty_file: $!");
  }

  my $test_file = File::Spec->rel2abs("$tmpdir/test.txt");

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
      # Give the server a chance to start up
      sleep(2);

      my $client = Net::FTPSSL->new('127.0.0.1',
        Encryption => 'E',
        Port => $port,
      );

      unless ($client) {
        die("Can't connect to FTPS server: " . IO::Socket::SSL::errstr());
      }

      unless ($client->login($user, $passwd)) {
        die("Can't login: " . $client->last_message());
      }

      unless ($client->binary()) {
        die("Can't set transfer mode to binary: " . $client->last_message());
      }

      unless ($client->put($empty_file, 'test.txt')) {
        die("Can't upload '$empty_file' to 'test.txt': " .
          $client->last_message());
      }

      $client->quit();

      unless (-f $test_file) {
        die("File $test_file does not exist as expected");
      }

      unless (-z $test_file) {
        die("File $test_file is not empty as expected");
      }

    };

    if ($@) {
      $ex = $@;
    }

    $wfh->print("done\n");
    $wfh->flush();

  } else {
    eval { server_wait($config_file, $rfh) };
    if ($@) {
      warn($@);
      exit 1;
    }

    exit 0;
  }

  # Stop server
  server_stop($pid_file);

  $self->assert_child_ok($pid);

  if ($ex) {
    die($ex);
  }

  unlink($log_file);
}

sub tls_retr_empty_file {
  my $self = shift;
  my $tmpdir = $self->{tmpdir};

  my $config_file = "$tmpdir/tls.conf";
  my $pid_file = File::Spec->rel2abs("$tmpdir/tls.pid");
  my $scoreboard_file = File::Spec->rel2abs("$tmpdir/tls.scoreboard");

  my $log_file = File::Spec->rel2abs('tests.log');

  my $auth_user_file = File::Spec->rel2abs("$tmpdir/tls.passwd");
  my $auth_group_file = File::Spec->rel2abs("$tmpdir/tls.group");

  my $user = 'proftpd';
  my $passwd = 'test';
  my $home_dir = File::Spec->rel2abs($tmpdir);
  my $uid = 500;
  my $gid = 500;

  # Make sure that, if we're running as root, that the home directory has
  # permissions/privs set for the account we create
  if ($< == 0) {
    unless (chmod(0755, $home_dir)) {
      die("Can't set perms on $home_dir to 0755: $!");
    }

    unless (chown($uid, $gid, $home_dir)) {
      die("Can't set owner of $home_dir to $uid/$gid: $!");
    }
  }

  auth_user_write($auth_user_file, $user, $passwd, $uid, $gid, $home_dir,
    '/bin/bash');
  auth_group_write($auth_group_file, 'ftpd', $gid, $user);

  my $cert_file = File::Spec->rel2abs('t/etc/modules/mod_tls/server-cert.pem');
  my $ca_file = File::Spec->rel2abs('t/etc/modules/mod_tls/ca-cert.pem');

  my $config = {
    PidFile => $pid_file,
    ScoreboardFile => $scoreboard_file,
    SystemLog => $log_file,

    AuthUserFile => $auth_user_file,
    AuthGroupFile => $auth_group_file,

    IfModules => {
      'mod_delay.c' => {
        DelayEngine => 'off',
      },

      'mod_tls.c' => {
        TLSEngine => 'on',
        TLSLog => $log_file,
        TLSProtocol => 'SSLv3 TLSv1',
        TLSRequired => 'on',
        TLSRSACertificateFile => $cert_file,
        TLSCACertificateFile => $ca_file,
      },
    },
  };

  my ($port, $config_user, $config_group) = config_write($config_file, $config);

  my $empty_file = File::Spec->rel2abs("$tmpdir/empty.txt");
  if (open(my $fh, "> $empty_file")) {
    close($fh);

  } else {
    die("Can't open $empty_file: $!");
  }

  my $test_file = File::Spec->rel2abs("$tmpdir/test.txt");

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
      # Give the server a chance to start up
      sleep(2);

      my $client = Net::FTPSSL->new('127.0.0.1',
        Encryption => 'E',
        Port => $port,
      );

      unless ($client) {
        die("Can't connect to FTPS server: " . IO::Socket::SSL::errstr());
      }

      unless ($client->login($user, $passwd)) {
        die("Can't login: " . $client->last_message());
      }

      unless ($client->binary()) {
        die("Can't set transfer mode to binary: " . $client->last_message());
      }

      unless ($client->get($empty_file, $test_file)) {
        die("Can't download '$empty_file' to '$test_file': " .
          $client->last_message());
      }

      $client->quit();

      unless (-f $test_file) {
        die("File $test_file does not exist as expected");
      }

      unless (-z $test_file) {
        die("File $test_file is not empty as expected");
      }

    };

    if ($@) {
      $ex = $@;
    }

    $wfh->print("done\n");
    $wfh->flush();

  } else {
    eval { server_wait($config_file, $rfh) };
    if ($@) {
      warn($@);
      exit 1;
    }

    exit 0;
  }

  # Stop server
  server_stop($pid_file);

  $self->assert_child_ok($pid);

  if ($ex) {
    die($ex);
  }

  unlink($log_file);
}

1;
