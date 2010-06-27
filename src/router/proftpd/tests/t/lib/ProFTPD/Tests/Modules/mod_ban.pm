package ProFTPD::Tests::Modules::mod_ban;

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
  ban_on_event_max_login_attempts => {
    order => ++$order,
    test_class => [qw(forking)],
  },

  ban_message => {
    order => ++$order,
    test_class => [qw(forking)],
  },

  ban_ifclass_engine_on => {
    order => ++$order,
    test_class => [qw(forking mod_ifsession)],
  },

  ban_ifclass_engine_off => {
    order => ++$order,
    test_class => [qw(forking mod_ifsession)],
  },

  ban_max_logins_exceeded_bug3281 => {
    order => ++$order,
    test_class => [qw(bug forking)],
  },

  ban_timeout_login_exceeded_bug3281 => {
    order => ++$order,
    test_class => [qw(bug forking)],
  },

  ban_engine_vhost_bug3355 => {
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

sub ban_on_event_max_login_attempts {
  my $self = shift;
  my $tmpdir = $self->{tmpdir};

  my $config_file = "$tmpdir/ban.conf";
  my $pid_file = File::Spec->rel2abs("$tmpdir/ban.pid");
  my $scoreboard_file = File::Spec->rel2abs("$tmpdir/ban.scoreboard");

  my $log_file = File::Spec->rel2abs('tests.log');

  my $ban_tab = File::Spec->rel2abs("$tmpdir/ban.tab");

  my $auth_user_file = File::Spec->rel2abs("$tmpdir/ban.passwd");
  my $auth_group_file = File::Spec->rel2abs("$tmpdir/ban.group");

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

  my $config = {
    PidFile => $pid_file,
    ScoreboardFile => $scoreboard_file,
    SystemLog => $log_file,
    TraceLog => $log_file,
    Trace => 'event:10',

    AuthUserFile => $auth_user_file,
    AuthGroupFile => $auth_group_file,

    MaxLoginAttempts => 1,

    IfModules => {
      'mod_ban.c' => {
        BanEngine => 'on',
        BanLog => $log_file,

        # This says to ban a client which exceeds the MaxLoginAttempts
        # limit once within the last 1 minute will be banned for 5 secs
        BanOnEvent => 'MaxLoginAttempts 1/00:01:00 00:00:05',

        BanTable => $ban_tab,
      },

      'mod_delay.c' => {
        DelayEngine => 'off',
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

  my $ex;

  # Fork child
  $self->handle_sigchld();
  defined(my $pid = fork()) or die("Can't fork: $!");
  if ($pid) {
    eval {
      my $client = ProFTPD::TestSuite::FTP->new('127.0.0.1', $port);

      my ($resp_code, $resp_msg);

      eval { $client->login($user, 'foo') };
      unless ($@) {
        die("Login succeeded unexpectedly");

      } else {
        $resp_code = $client->response_code();
        $resp_msg = $client->response_msg();
      }

      my $expected;

      $expected = 530;
      $self->assert($expected == $resp_code,
        test_msg("Expected $expected, got $resp_code"));

      $expected = "Login incorrect.";
      $self->assert($expected eq $resp_msg,
        test_msg("Expected '$expected', got '$resp_msg'"));

      eval { $client->login($user, 'foo') };
      unless ($@) {
        die("Login succeeded unexpectedly");

      } else {
        $resp_code = $client->response_code();
        $resp_msg = $client->response_msg();
      }

      $expected = 530;
      $self->assert($expected == $resp_code,
        test_msg("Expected $expected, got $resp_code"));

      $expected = "Login incorrect.";
      $self->assert($expected eq $resp_msg,
        test_msg("Expected '$expected', got '$resp_msg'"));

      # Now try again with the correct info; we should be banned.  Note
      # that we have to create a separate connection for this.

      eval { $client = ProFTPD::TestSuite::FTP->new('127.0.0.1', $port,
        undef, 0) };
      unless ($@) {
        die("Connect succeeded unexpectedly");
      }

      my $conn_ex = ProFTPD::TestSuite::FTP::get_connect_exception();

      $expected = "";
      $self->assert($expected eq $conn_ex,
        test_msg("Expected '$expected', got '$conn_ex'"));
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

sub ban_message {
  my $self = shift;
  my $tmpdir = $self->{tmpdir};

  my $config_file = "$tmpdir/ban.conf";
  my $pid_file = File::Spec->rel2abs("$tmpdir/ban.pid");
  my $scoreboard_file = File::Spec->rel2abs("$tmpdir/ban.scoreboard");

  my $log_file = File::Spec->rel2abs('tests.log');

  my $ban_tab = File::Spec->rel2abs("$tmpdir/ban.tab");

  my $auth_user_file = File::Spec->rel2abs("$tmpdir/ban.passwd");
  my $auth_group_file = File::Spec->rel2abs("$tmpdir/ban.group");

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

  my $config = {
    PidFile => $pid_file,
    ScoreboardFile => $scoreboard_file,
    SystemLog => $log_file,

    AuthUserFile => $auth_user_file,
    AuthGroupFile => $auth_group_file,

    MaxLoginAttempts => 1,

    IfModules => {
      'mod_ban.c' => {
        BanEngine => 'on',
        BanLog => $log_file,

        # This says to ban a client which exceeds the MaxLoginAttempts
        # limit once within the last 1 minute will be banned for 5 secs
        BanOnEvent => 'MaxLoginAttempts 1/00:01:00 00:00:05',

        BanTable => $ban_tab,

        BanMessage => '"Go away %a (%c), or I shall taunt you (%u) a second time!"',
      },

      'mod_delay.c' => {
        DelayEngine => 'off',
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

  my $ex;

  # Fork child
  $self->handle_sigchld();
  defined(my $pid = fork()) or die("Can't fork: $!");
  if ($pid) {
    eval {
      my $client = ProFTPD::TestSuite::FTP->new('127.0.0.1', $port);

      my ($resp_code, $resp_msg);

      eval { $client->login($user, 'foo') };
      unless ($@) {
        die("Login succeeded unexpectedly");

      } else {
        $resp_code = $client->response_code();
        $resp_msg = $client->response_msg();
      }

      my $expected;

      $expected = 530;
      $self->assert($expected == $resp_code,
        test_msg("Expected $expected, got $resp_code"));

      $expected = "Login incorrect.";
      $self->assert($expected eq $resp_msg,
        test_msg("Expected '$expected', got '$resp_msg'"));

      eval { $client->login($user, 'foo') };
      unless ($@) {
        die("Login succeeded unexpectedly");

      } else {
        $resp_code = $client->response_code();
        $resp_msg = $client->response_msg();
      }

      $expected = 530;
      $self->assert($expected == $resp_code,
        test_msg("Expected $expected, got $resp_code"));

      $expected = "Login incorrect.";
      $self->assert($expected eq $resp_msg,
        test_msg("Expected '$expected', got '$resp_msg'"));

      # Now try again with the correct info; we should be banned.
      eval { $client = ProFTPD::TestSuite::FTP->new('127.0.0.1', $port,
        undef, 0) };
      unless ($@) {
        die("Connect succeeded unexpectedly");
      }

      my $conn_ex = ProFTPD::TestSuite::FTP::get_connect_exception();
      
      $expected = "Go away 127.0.0.1 ((none)), or I shall taunt you ((none)) a second time!";
      $self->assert($expected eq $conn_ex,
        test_msg("Expected '$expected', got '$conn_ex'"));
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

sub ban_ifclass_engine_on {
  my $self = shift;
  my $tmpdir = $self->{tmpdir};

  my $config_file = "$tmpdir/ban.conf";
  my $pid_file = File::Spec->rel2abs("$tmpdir/ban.pid");
  my $scoreboard_file = File::Spec->rel2abs("$tmpdir/ban.scoreboard");

  my $log_file = File::Spec->rel2abs('tests.log');

  my $ban_tab = File::Spec->rel2abs("$tmpdir/ban.tab");

  my $auth_user_file = File::Spec->rel2abs("$tmpdir/ban.passwd");
  my $auth_group_file = File::Spec->rel2abs("$tmpdir/ban.group");

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

  my $config = {
    PidFile => $pid_file,
    ScoreboardFile => $scoreboard_file,
    SystemLog => $log_file,

    AuthUserFile => $auth_user_file,
    AuthGroupFile => $auth_group_file,

    MaxLoginAttempts => 1,

    Class => {
      test => {
        From => '127.0.0.1',
      },
    },

    IfModules => {
      'mod_ban.c' => {
        BanLog => $log_file,

        # This says to ban a client which exceeds the MaxLoginAttempts
        # limit once within the last 1 minute will be banned for 5 secs
        BanOnEvent => 'MaxLoginAttempts 1/00:01:00 00:00:05',

        BanTable => $ban_tab,
      },

      'mod_delay.c' => {
        DelayEngine => 'off',
      },
    },
  };

  my ($port, $config_user, $config_group) = config_write($config_file, $config);

  if (open(my $fh, ">> $config_file")) {
    print $fh <<EOI;
<IfClass test>
  BanEngine on
</IfClass>
EOI
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

  my $ex;

  # Fork child
  $self->handle_sigchld();
  defined(my $pid = fork()) or die("Can't fork: $!");
  if ($pid) {
    eval {
      my $client = ProFTPD::TestSuite::FTP->new('127.0.0.1', $port);

      my ($resp_code, $resp_msg);

      eval { $client->login($user, 'foo') };
      unless ($@) {
        die("Login succeeded unexpectedly");

      } else {
        $resp_code = $client->response_code();
        $resp_msg = $client->response_msg();
      }

      my $expected;

      $expected = 530;
      $self->assert($expected == $resp_code,
        test_msg("Expected $expected, got $resp_code"));

      $expected = "Login incorrect.";
      $self->assert($expected eq $resp_msg,
        test_msg("Expected '$expected', got '$resp_msg'"));

      eval { $client->login($user, 'foo') };
      unless ($@) {
        die("Login succeeded unexpectedly");

      } else {
        $resp_code = $client->response_code();
        $resp_msg = $client->response_msg();
      }

      my $expected;

      $expected = 530;
      $self->assert($expected == $resp_code,
        test_msg("Expected $expected, got $resp_code"));

      $expected = "Login incorrect.";
      $self->assert($expected eq $resp_msg,
        test_msg("Expected '$expected', got '$resp_msg'"));

      # Now try again with the correct info; we should be banned.  Note
      # that we have to create a separate connection for this.

      eval { $client = ProFTPD::TestSuite::FTP->new('127.0.0.1', $port,
        undef, 0) };
      unless ($@) {
        die("Connect succeeded unexpectedly");
      }

      my $conn_ex = ProFTPD::TestSuite::FTP::get_connect_exception();

      $expected = "";
      $self->assert($expected eq $conn_ex,
        test_msg("Expected '$expected', got '$conn_ex'"));
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

sub ban_ifclass_engine_off {
  my $self = shift;
  my $tmpdir = $self->{tmpdir};

  my $config_file = "$tmpdir/ban.conf";
  my $pid_file = File::Spec->rel2abs("$tmpdir/ban.pid");
  my $scoreboard_file = File::Spec->rel2abs("$tmpdir/ban.scoreboard");

  my $log_file = File::Spec->rel2abs('tests.log');

  my $ban_tab = File::Spec->rel2abs("$tmpdir/ban.tab");

  my $auth_user_file = File::Spec->rel2abs("$tmpdir/ban.passwd");
  my $auth_group_file = File::Spec->rel2abs("$tmpdir/ban.group");

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

  my $config = {
    PidFile => $pid_file,
    ScoreboardFile => $scoreboard_file,
    SystemLog => $log_file,

    AuthUserFile => $auth_user_file,
    AuthGroupFile => $auth_group_file,

    MaxLoginAttempts => 1,

    Class => {
      test => {
        From => '127.0.0.1',
      },
    },

    IfModules => {
      'mod_ban.c' => {
        BanEngine => 'on',
        BanLog => $log_file,

        # This says to ban a client which exceeds the MaxLoginAttempts
        # limit once within the last 1 minute will be banned for 5 secs
        BanOnEvent => 'MaxLoginAttempts 1/00:01:00 00:00:05',

        BanTable => $ban_tab,
      },

      'mod_delay.c' => {
        DelayEngine => 'off',
      },
    },
  };

  my ($port, $config_user, $config_group) = config_write($config_file, $config);

  if (open(my $fh, ">> $config_file")) {
    print $fh <<EOI;
<IfClass test>
  BanEngine off
</IfClass>
EOI
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

  my $ex;

  # Fork child
  $self->handle_sigchld();
  defined(my $pid = fork()) or die("Can't fork: $!");
  if ($pid) {
    eval {
      my $client = ProFTPD::TestSuite::FTP->new('127.0.0.1', $port);

      my ($resp_code, $resp_msg);

      eval { $client->login($user, 'foo') };
      unless ($@) {
        die("Login succeeded unexpectedly");

      } else {
        $resp_code = $client->response_code();
        $resp_msg = $client->response_msg();
      }

      my $expected;

      $expected = 530;
      $self->assert($expected == $resp_code,
        test_msg("Expected $expected, got $resp_code"));

      $expected = "Login incorrect.";
      $self->assert($expected eq $resp_msg,
        test_msg("Expected '$expected', got '$resp_msg'"));

      # Now try again with the correct info; we should NOT be banned.  Note
      # that we have to create a separate connection for this.

      eval { $client = ProFTPD::TestSuite::FTP->new('127.0.0.1', $port,
        undef, 0) };
      if ($@) {
        die("Connect failed unexpectedly");
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

sub ban_max_logins_exceeded_bug3281 {
  my $self = shift;
  my $tmpdir = $self->{tmpdir};

  my $config_file = "$tmpdir/ban.conf";
  my $pid_file = File::Spec->rel2abs("$tmpdir/ban.pid");
  my $scoreboard_file = File::Spec->rel2abs("$tmpdir/ban.scoreboard");

  my $log_file = File::Spec->rel2abs('tests.log');

  my $ban_tab = File::Spec->rel2abs("$tmpdir/ban.tab");

  my $auth_user_file = File::Spec->rel2abs("$tmpdir/ban.passwd");
  my $auth_group_file = File::Spec->rel2abs("$tmpdir/ban.group");

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

  my $config = {
    PidFile => $pid_file,
    ScoreboardFile => $scoreboard_file,
    SystemLog => $log_file,
    TraceLog => $log_file,
    Trace => 'lock:10 scoreboard:10',

    AuthUserFile => $auth_user_file,
    AuthGroupFile => $auth_group_file,

    MaxLoginAttempts => 1,

    IfModules => {
      'mod_ban.c' => {
        BanEngine => 'on',
        BanLog => $log_file,

        # This says to ban a client which exceeds the MaxLoginAttempts
        # limit once within the last 1 minute will be banned for 5 secs
        BanOnEvent => 'MaxLoginAttempts 1/00:01:00 00:00:05',

        BanTable => $ban_tab,
      },

      'mod_delay.c' => {
        DelayEngine => 'off',
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

  my $ex;

  # Fork child
  $self->handle_sigchld();
  defined(my $pid = fork()) or die("Can't fork: $!");
  if ($pid) {
    eval {
      my $client = ProFTPD::TestSuite::FTP->new('127.0.0.1', $port);

      my ($resp_code, $resp_msg);

      eval { $client->login($user, 'foo') };
      unless ($@) {
        die("Login succeeded unexpectedly");

      } else {
        $resp_code = $client->response_code();
        $resp_msg = $client->response_msg();
      }

      my $expected;

      $expected = 530;
      $self->assert($expected == $resp_code,
        test_msg("Expected $expected, got $resp_code"));

      $expected = "Login incorrect.";
      $self->assert($expected eq $resp_msg,
        test_msg("Expected '$expected', got '$resp_msg'"));

      # Try the login again
      eval { $client->login($user, 'foo') };
      unless ($@) {
        die("Login succeeded unexpectedly");

      } else {
        $resp_code = $client->response_code();
        $resp_msg = $client->response_msg();
      }

      $expected = 530;
      $self->assert($expected == $resp_code,
        test_msg("Expected $expected, got $resp_code"));

      $expected = "Login incorrect.";
      $self->assert($expected eq $resp_msg,
        test_msg("Expected '$expected', got '$resp_msg'"));

      # According to Bug#3281, the session process should be have closed/ended.
      # Make sure this is the case.  This QUIT command should fail.
      eval { $client->quit() };
      unless ($@) {
        die("QUIT succeeded unexpectedly");
      }

      # Now try again with the correct info; we should be banned.  Note
      # that we have to create a separate connection for this.

      eval { $client = ProFTPD::TestSuite::FTP->new('127.0.0.1', $port,
        undef, 0) };
      unless ($@) {
        die("Connect succeeded unexpectedly");
      }

      my $conn_ex = ProFTPD::TestSuite::FTP::get_connect_exception();

      $expected = "";
      $self->assert($expected eq $conn_ex,
        test_msg("Expected '$expected', got '$conn_ex'"));
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

sub ban_timeout_login_exceeded_bug3281 {
  my $self = shift;
  my $tmpdir = $self->{tmpdir};

  my $config_file = "$tmpdir/ban.conf";
  my $pid_file = File::Spec->rel2abs("$tmpdir/ban.pid");
  my $scoreboard_file = File::Spec->rel2abs("$tmpdir/ban.scoreboard");

  my $log_file = File::Spec->rel2abs('tests.log');

  my $ban_tab = File::Spec->rel2abs("$tmpdir/ban.tab");

  my $auth_user_file = File::Spec->rel2abs("$tmpdir/ban.passwd");
  my $auth_group_file = File::Spec->rel2abs("$tmpdir/ban.group");

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

  my $timeout_login = 1;

  my $config = {
    PidFile => $pid_file,
    ScoreboardFile => $scoreboard_file,
    SystemLog => $log_file,
    TraceLog => $log_file,
    Trace => 'event:10 lock:10 scoreboard:10',

    AuthUserFile => $auth_user_file,
    AuthGroupFile => $auth_group_file,

    TimeoutLogin => $timeout_login,

    IfModules => {
      'mod_ban.c' => {
        BanEngine => 'on',
        BanLog => $log_file,

        # This says to ban a client which exceeds the TimeoutLogin
        # limit once within the last 1 minute will be banned for 5 secs
        BanOnEvent => 'TimeoutLogin 1/00:01:00 00:00:05',

        BanTable => $ban_tab,
      },

      'mod_delay.c' => {
        DelayEngine => 'off',
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

  my $ex;

  # Fork child
  $self->handle_sigchld();
  defined(my $pid = fork()) or die("Can't fork: $!");
  if ($pid) {
    eval {
      my $client = ProFTPD::TestSuite::FTP->new('127.0.0.1', $port);

      my ($resp_code, $resp_msg);

      # Wait more than the TimeoutLogin interval
      sleep($timeout_login + 1);

      eval { $client->user($user) };
      unless ($@) {
        die("USER succeeded unexpectedly");
      }

      # According to Bug#3281, the session process should be have closed/ended.
      # Make sure this is the case.  This QUIT command should fail.
      eval { $client->quit() };
      unless ($@) {
        die("QUIT succeeded unexpectedly");
      }

      # Connect again, and again let the TimeoutLogin be exceed.
      $client = ProFTPD::TestSuite::FTP->new('127.0.0.1', $port);

      # Wait more than the TimeoutLogin interval
      sleep($timeout_login + 1);

      eval { $client->user($user) };
      unless ($@) {
        die("USER succeeded unexpectedly");
      }

      # According to Bug#3281, the session process should be have closed/ended.
      # Make sure this is the case.  This QUIT command should fail.
      eval { $client->quit() };
      unless ($@) {
        die("QUIT succeeded unexpectedly");
      }

      # Now try to connect; we should be banned.
      eval { $client = ProFTPD::TestSuite::FTP->new('127.0.0.1', $port,
        undef, 0) };
      unless ($@) {
        die("Connect succeeded unexpectedly");
      }

      my $conn_ex = ProFTPD::TestSuite::FTP::get_connect_exception();

      my $expected = "";
      $self->assert($expected eq $conn_ex,
        test_msg("Expected '$expected', got '$conn_ex'"));
    };

    if ($@) {
      $ex = $@;
    }

    $wfh->print("done\n");
    $wfh->flush();

  } else {
    eval { server_wait($config_file, $rfh, 15) };
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

sub ban_engine_vhost_bug3355 {
  my $self = shift;
  my $tmpdir = $self->{tmpdir};

  my $config_file = "$tmpdir/ban.conf";
  my $pid_file = File::Spec->rel2abs("$tmpdir/ban.pid");
  my $scoreboard_file = File::Spec->rel2abs("$tmpdir/ban.scoreboard");

  my $log_file = File::Spec->rel2abs('tests.log');

  my $ban_tab = File::Spec->rel2abs("$tmpdir/ban.tab");

  my $auth_user_file = File::Spec->rel2abs("$tmpdir/ban.passwd");
  my $auth_group_file = File::Spec->rel2abs("$tmpdir/ban.group");

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

  my $config = {
    PidFile => $pid_file,
    ScoreboardFile => $scoreboard_file,
    SystemLog => $log_file,
    TraceLog => $log_file,
    Trace => 'event:10',
    DefaultServer => 'off',

    AuthUserFile => $auth_user_file,
    AuthGroupFile => $auth_group_file,

    MaxLoginAttempts => 1,

    IfModules => {
      'mod_ban.c' => {
        BanLog => $log_file,

        # This says to ban a client which exceeds the MaxLoginAttempts
        # limit once within the last 1 minute will be banned for 5 secs
        BanOnEvent => 'MaxLoginAttempts 1/00:01:00 00:00:05',

        BanTable => $ban_tab,
      },

      'mod_delay.c' => {
        DelayEngine => 'off',
      },
    },
  };

  my ($port, $config_user, $config_group) = config_write($config_file, $config);
  my $vhost_port = $port + 21;
 
  if (open(my $fh, ">> $config_file")) {
    print $fh <<EOC;
<VirtualHost 127.0.0.1>
  Port $vhost_port
  AuthUserFile $auth_user_file
  AuthGroupFile $auth_group_file
  MaxLoginAttempts 1
  <IfModule mod_ban.c>
    BanEngine off
  </IfModule>
</VirtualHost>
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

  my $ex;

  # Fork child
  $self->handle_sigchld();
  defined(my $pid = fork()) or die("Can't fork: $!");
  if ($pid) {
    eval {
      my $client = ProFTPD::TestSuite::FTP->new('127.0.0.1', $vhost_port);

      my ($resp_code, $resp_msg);

      eval { $client->login($user, 'foo') };
      unless ($@) {
        die("Login succeeded unexpectedly");

      } else {
        $resp_code = $client->response_code();
        $resp_msg = $client->response_msg();
      }

      my $expected;

      $expected = 530;
      $self->assert($expected == $resp_code,
        test_msg("Expected $expected, got $resp_code"));

      $expected = "Login incorrect.";
      $self->assert($expected eq $resp_msg,
        test_msg("Expected '$expected', got '$resp_msg'"));

      eval { $client->login($user, 'foo') };
      unless ($@) {
        die("Login succeeded unexpectedly");

      } else {
        $resp_code = $client->response_code();
        $resp_msg = $client->response_msg();
      }

      $expected = 530;
      $self->assert($expected == $resp_code,
        test_msg("Expected $expected, got $resp_code"));

      $expected = "Login incorrect.";
      $self->assert($expected eq $resp_msg,
        test_msg("Expected '$expected', got '$resp_msg'"));

      # Now try again with the correct info; we should be banned, except
      # that we are connecting to the <VirtualHost> which has mod_ban disabled.
      # Note that we have to create a separate connection for this.

      $client = ProFTPD::TestSuite::FTP->new('127.0.0.1', $vhost_port);
      $client->login($user, $passwd);
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

1;
