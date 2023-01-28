package ProFTPD::Tests::Config::Include;

use lib qw(t/lib);
use base qw(ProFTPD::TestSuite::Child);
use strict;

use File::Path qw(mkpath);
use File::Spec;
use IO::Handle;

use ProFTPD::TestSuite::FTP;
use ProFTPD::TestSuite::Utils qw(:auth :config :running :test :testsuite);

$| = 1;

my $order = 0;

my $TESTS = {
  include_file => {
    order => ++$order,
    test_class => [qw(forking)],
  },

  include_dir_bug1696 => {
    order => ++$order,
    test_class => [qw(bug forking)],
  },

  include_filename_wildcard_bug1696 => {
    order => ++$order,
    test_class => [qw(bug forking)],
  },

  include_filename_wildcard_no_matches_bug1696 => {
    order => ++$order,
    test_class => [qw(bug forking)],
  },

  include_directory_wildcard_issue1472 => {
    order => ++$order,
    test_class => [qw(bug forking)],
  },

  include_limit => {
    order => ++$order,
    test_class => [qw(forking)],
  },

};

sub new {
  return shift()->SUPER::new(@_);
}

sub list_tests {
  return testsuite_get_runnable_tests($TESTS);
}

sub include_file {
  my $self = shift;
  my $tmpdir = $self->{tmpdir};
  my $setup = test_setup($tmpdir, 'config');

  my $test_file = File::Spec->rel2abs("$tmpdir/test.txt");
  if (open(my $fh, "> $test_file")) {
    print $fh "Hello, World\n";
    unless (close($fh)) {
      die("Can't write $test_file: $!");
    }

    # Make sure that, if we're running as root, that the test file has
    # permissions/privs set for the account we create
    if ($< == 0) {
      unless (chown($setup->{uid}, $setup->{gid}, $test_file)) {
        die("Can't set owner of $test_file to $setup->{uid}/$setup->{gid}: $!");
      }
    }

  } else {
    die("Can't open $test_file: $!");
  }

  my $include_config = File::Spec->rel2abs("$tmpdir/include.conf");
  if (open(my $fh, "> $include_config")) {
    print $fh <<EOC;
<Limit ALL>
  DenyAll
</Limit>

<Limit STOR>
  Order allow,deny
  AllowUser $setup->{user}
  DenyAll
</Limit>
EOC

    unless (close($fh)) {
      die("Can't write $include_config: $!");
    }

  } else {
    die("Can't open $include_config: $!");
  }

  my $config = {
    PidFile => $setup->{pid_file},
    ScoreboardFile => $setup->{scoreboard_file},
    SystemLog => $setup->{log_file},

    AuthUserFile => $setup->{auth_user_file},
    AuthGroupFile => $setup->{auth_group_file},
    AuthOrder => 'mod_auth_file.c',

    AllowOverwrite => 'on',
    AllowStoreRestart => 'on',
    DefaultChdir => '~',

    Include => $include_config,

    IfModules => {
      'mod_delay.c' => {
        DelayEngine => 'off',
      },
    },
  };

  my ($port, $config_user, $config_group) = config_write($setup->{config_file},
    $config);

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
      # Allow for server startup
      sleep(1);

      my $client = ProFTPD::TestSuite::FTP->new('127.0.0.1', $port);
      $client->login($setup->{user}, $setup->{passwd});

      my $filename = 'test.txt';

      my $conn = $client->retr_raw($filename);
      if ($conn) {
        die("RETR $filename succeeded unexpectedly");
      }

      my $resp_code = $client->response_code();
      my $resp_msg = $client->response_msg();

      my $expected = 550;
      $self->assert($expected == $resp_code,
        test_msg("Expected response code $expected, got $resp_code"));

      $expected = "$filename: Operation not permitted";
      $self->assert($expected eq $resp_msg,
        test_msg("Expected response message '$expected', got '$resp_msg'"));

      $conn = $client->stor_raw('test.txt');
      unless ($conn) {
        die("Failed to STOR test.txt: " . $client->response_code() . " " .
          $client->response_msg());
      }

      my $buf = "Hello, World!\n";
      $conn->write($buf, length($buf), 25);
      eval { $conn->close() };

      $resp_code = $client->response_code();
      $resp_msg = $client->response_msg();

      $expected = 226;
      $self->assert($expected == $resp_code,
        test_msg("Expected response code $expected, got $resp_code"));

      $expected = "Transfer complete";
      $self->assert($expected eq $resp_msg,
        test_msg("Expected response message '$expected', got '$resp_msg'"));

      $client->quit();
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

sub include_dir_bug1696 {
  my $self = shift;
  my $tmpdir = $self->{tmpdir};
  my $setup = test_setup($tmpdir, 'config');

  my $test_file = File::Spec->rel2abs("$tmpdir/test.txt");
  if (open(my $fh, "> $test_file")) {
    print $fh "Hello, World\n";
    unless (close($fh)) {
      die("Can't write $test_file: $!");
    }

    # Make sure that, if we're running as root, that the test file has
    # permissions/privs set for the account we create
    if ($< == 0) {
      unless (chown($setup->{uid}, $setup->{gid}, $test_file)) {
        die("Can't set owner of $test_file to $setup->{uid}/$setup->{gid}: $!");
      }
    }

  } else {
    die("Can't open $test_file: $!");
  }

  my $include_dir = File::Spec->rel2abs("$tmpdir/conf.d");
  mkpath($include_dir);

  my $include_config = File::Spec->rel2abs("$include_dir/include.conf");
  if (open(my $fh, "> $include_config")) {
    print $fh <<EOC;
<Limit ALL>
  DenyAll
</Limit>

<Limit STOR>
  Order allow,deny
  AllowUser $setup->{user}
  DenyAll
</Limit>
EOC

    unless (close($fh)) {
      die("Can't write $include_config: $!");
    }

  } else {
    die("Can't open $include_config: $!");
  }

  my $config = {
    PidFile => $setup->{pid_file},
    ScoreboardFile => $setup->{scoreboard_file},
    SystemLog => $setup->{log_file},

    AuthUserFile => $setup->{auth_user_file},
    AuthGroupFile => $setup->{auth_group_file},
    AuthOrder => 'mod_auth_file.c',

    AllowOverwrite => 'on',
    AllowStoreRestart => 'on',
    DefaultChdir => '~',

    Include => $include_dir,

    IfModules => {
      'mod_delay.c' => {
        DelayEngine => 'off',
      },
    },
  };

  my ($port, $config_user, $config_group) = config_write($setup->{config_file},
    $config);

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
      # Allow for server startup
      sleep(1);

      my $client = ProFTPD::TestSuite::FTP->new('127.0.0.1', $port);
      $client->login($setup->{user}, $setup->{passwd});

      my $filename = 'test.txt';

      my $conn = $client->retr_raw($filename);
      if ($conn) {
        die("RETR $filename succeeded unexpectedly");
      }

      my $resp_code = $client->response_code();
      my $resp_msg = $client->response_msg();

      my $expected = 550;
      $self->assert($expected == $resp_code,
        test_msg("Expected response code $expected, got $resp_code"));

      $expected = "$filename: Operation not permitted";
      $self->assert($expected eq $resp_msg,
        test_msg("Expected response message '$expected', got '$resp_msg'"));

      $conn = $client->stor_raw('test.txt');
      unless ($conn) {
        die("Failed to STOR test.txt: " . $client->response_code() . " " .
          $client->response_msg());
      }

      my $buf = "Hello, World!\n";
      $conn->write($buf, length($buf), 25);
      eval { $conn->close() };

      $resp_code = $client->response_code();
      $resp_msg = $client->response_msg();

      $expected = 226;
      $self->assert($expected == $resp_code,
        test_msg("Expected response code $expected, got $resp_code"));

      $expected = "Transfer complete";
      $self->assert($expected eq $resp_msg,
        test_msg("Expected response message '$expected', got '$resp_msg'"));

      $client->quit();
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

sub include_filename_wildcard_bug1696 {
  my $self = shift;
  my $tmpdir = $self->{tmpdir};
  my $setup = test_setup($tmpdir, 'config');

  my $test_file = File::Spec->rel2abs("$tmpdir/test.txt");
  if (open(my $fh, "> $test_file")) {
    print $fh "Hello, World\n";
    unless (close($fh)) {
      die("Can't write $test_file: $!");
    }

    # Make sure that, if we're running as root, that the test file has
    # permissions/privs set for the account we create
    if ($< == 0) {
      unless (chown($setup->{uid}, $setup->{gid}, $test_file)) {
        die("Can't set owner of $test_file to $setup->{uid}/$setup->{gid}: $!");
      }
    }

  } else {
    die("Can't open $test_file: $!");
  }

  my $include_dir = File::Spec->rel2abs("$tmpdir/conf.d");
  mkpath($include_dir);

  my $include_config = File::Spec->rel2abs("$include_dir/a.conf");
  if (open(my $fh, "> $include_config")) {
    print $fh <<EOC;
<Limit ALL>
  DenyAll
</Limit>
EOC

    unless (close($fh)) {
      die("Can't write $include_config: $!");
    }

  } else {
    die("Can't open $include_config: $!");
  }

  $include_config = File::Spec->rel2abs("$include_dir/b.conf");
  if (open(my $fh, "> $include_config")) {
    print $fh <<EOC;
<Limit STOR>
  Order allow,deny
EOC
    unless (close($fh)) {
      die("Can't write $include_config: $!");
    }

  } else {
    die("Can't open $include_config: $!");
  }

  $include_config = File::Spec->rel2abs("$include_dir/c.conf");
  if (open(my $fh, "> $include_config")) {
    print $fh <<EOC;
  AllowUser $setup->{user}
  DenyAll
</Limit>
EOC

    unless (close($fh)) {
      die("Can't write $include_config: $!");
    }

  } else {
    die("Can't open $include_config: $!");
  }

  my $config = {
    PidFile => $setup->{pid_file},
    ScoreboardFile => $setup->{scoreboard_file},
    SystemLog => $setup->{log_file},

    AuthUserFile => $setup->{auth_user_file},
    AuthGroupFile => $setup->{auth_group_file},
    AuthOrder => 'mod_auth_file.c',

    AllowOverwrite => 'on',
    AllowStoreRestart => 'on',
    DefaultChdir => '~',

    Include => "$include_dir/*.conf",

    IfModules => {
      'mod_delay.c' => {
        DelayEngine => 'off',
      },
    },
  };

  my ($port, $config_user, $config_group) = config_write($setup->{config_file},
    $config);

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
      # Allow for server startup
      sleep(1);

      my $client = ProFTPD::TestSuite::FTP->new('127.0.0.1', $port);
      $client->login($setup->{user}, $setup->{passwd});

      my $filename = 'test.txt';

      my $conn = $client->retr_raw($filename);
      if ($conn) {
        die("RETR $filename succeeded unexpectedly");
      }

      my $resp_code = $client->response_code();
      my $resp_msg = $client->response_msg();

      my $expected = 550;
      $self->assert($expected == $resp_code,
        test_msg("Expected response code $expected, got $resp_code"));

      $expected = "$filename: Operation not permitted";
      $self->assert($expected eq $resp_msg,
        test_msg("Expected response message '$expected', got '$resp_msg'"));

      $conn = $client->stor_raw('test.txt');
      unless ($conn) {
        die("Failed to STOR test.txt: " . $client->response_code() . " " .
          $client->response_msg());
      }

      my $buf = "Hello, World!\n";
      $conn->write($buf, length($buf), 25);
      eval { $conn->close() };

      $resp_code = $client->response_code();
      $resp_msg = $client->response_msg();

      $expected = 226;
      $self->assert($expected == $resp_code,
        test_msg("Expected response code $expected, got $resp_code"));

      $expected = "Transfer complete";
      $self->assert($expected eq $resp_msg,
        test_msg("Expected response message '$expected', got '$resp_msg'"));

      $client->quit();
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

sub include_filename_wildcard_no_matches_bug1696 {
  my $self = shift;
  my $tmpdir = $self->{tmpdir};
  my $setup = test_setup($tmpdir, 'config');

  my $test_file = File::Spec->rel2abs("$tmpdir/test.txt");
  if (open(my $fh, "> $test_file")) {
    print $fh "Hello, World\n";
    unless (close($fh)) {
      die("Can't write $test_file: $!");
    }

    # Make sure that, if we're running as root, that the test file has
    # permissions/privs set for the account we create
    if ($< == 0) {
      unless (chown($setup->{uid}, $setup->{gid}, $test_file)) {
        die("Can't set owner of $test_file to $setup->{uid}/$setup->{gid}: $!");
      }
    }

  } else {
    die("Can't open $test_file: $!");
  }

  my $include_dir = File::Spec->rel2abs("$tmpdir/conf.d");
  mkpath($include_dir);

  my $include_config = File::Spec->rel2abs("$include_dir/a.txt");
  if (open(my $fh, "> $include_config")) {
    print $fh <<EOC;
<Limit ALL>
  DenyAll
</Limit>
EOC

    unless (close($fh)) {
      die("Can't write $include_config: $!");
    }

  } else {
    die("Can't open $include_config: $!");
  }

  $include_config = File::Spec->rel2abs("$include_dir/b.txt");
  if (open(my $fh, "> $include_config")) {
    print $fh <<EOC;
<Limit STOR>
  Order allow,deny
EOC

    unless (close($fh)) {
      die("Can't write $include_config: $!");
    }

  } else {
    die("Can't open $include_config: $!");
  }

  $include_config = File::Spec->rel2abs("$include_dir/c.txt");
  if (open(my $fh, "> $include_config")) {
    # We deliberately break the syntax of these files, to prove that the
    # server does not actually read them.

    print $fh <<EOC;
  AllowUser $setup->{user}
  DenyAll
EOC

    unless (close($fh)) {
      die("Can't write $include_config: $!");
    }

  } else {
    die("Can't open $include_config: $!");
  }

  my $config = {
    PidFile => $setup->{pid_file},
    ScoreboardFile => $setup->{scoreboard_file},
    SystemLog => $setup->{log_file},

    AuthUserFile => $setup->{auth_user_file},
    AuthGroupFile => $setup->{auth_group_file},
    AuthOrder => 'mod_auth_file.c',

    AllowOverwrite => 'on',
    AllowStoreRestart => 'on',
    DefaultChdir => '~',

    # We deliberately use a glob pattern here which does NOT match the
    # config file names in the sub-directory.
    Include => "$include_dir/*.conf",

    IfModules => {
      'mod_delay.c' => {
        DelayEngine => 'off',
      },
    },
  };

  my ($port, $config_user, $config_group) = config_write($setup->{config_file},
    $config);

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
      # Allow for server startup
      sleep(1);

      my $client = ProFTPD::TestSuite::FTP->new('127.0.0.1', $port);
      $client->login($setup->{user}, $setup->{passwd});

      my $filename = 'test.txt';

      my $conn = $client->retr_raw($filename);
      unless ($conn) {
        die("RETR $filename failed: " . $client->response_code() . " " .
          $client->response_msg());
      }

      my $buf;
      $conn->read($buf, 1024, 25);
      eval { $conn->close() };

      my $resp_code = $client->response_code();
      my $resp_msg = $client->response_msg();

      my $expected = 226;
      $self->assert($expected == $resp_code,
        test_msg("Expected response code $expected, got $resp_code"));

      $expected = "Transfer complete";
      $self->assert($expected eq $resp_msg,
        test_msg("Expected response message '$expected', got '$resp_msg'"));

      $client->quit();
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

sub include_directory_wildcard_issue1472 {
  my $self = shift;
  my $tmpdir = $self->{tmpdir};
  my $setup = test_setup($tmpdir, 'config');

  my $test_file = File::Spec->rel2abs("$tmpdir/test.txt");
  if (open(my $fh, "> $test_file")) {
    print $fh "Hello, World\n";
    unless (close($fh)) {
      die("Can't write $test_file: $!");
    }

    # Make sure that, if we're running as root, that the test file has
    # permissions/privs set for the account we create
    if ($< == 0) {
      unless (chown($setup->{uid}, $setup->{gid}, $test_file)) {
        die("Can't set owner of $test_file to $setup->{uid}/$setup->{gid}: $!");
      }
    }

  } else {
    die("Can't open $test_file: $!");
  }

  my $include_dir = File::Spec->rel2abs("$tmpdir/conf.d");
  mkpath($include_dir);

  my $include_config = File::Spec->rel2abs("$include_dir/test.conf");
  if (open(my $fh, "> $include_config")) {
    print $fh <<EOC;
<Limit ALL>
  DenyAll
</Limit>
EOC

    unless (close($fh)) {
      die("Can't write $include_config: $!");
    }

  } else {
    die("Can't open $include_config: $!");
  }

  my $config = {
    PidFile => $setup->{pid_file},
    ScoreboardFile => $setup->{scoreboard_file},
    SystemLog => $setup->{log_file},
    TraceLog => $setup->{log_file},
    Trace => 'config:20',

    AuthUserFile => $setup->{auth_user_file},
    AuthGroupFile => $setup->{auth_group_file},
    AuthOrder => 'mod_auth_file.c',

    AllowOverwrite => 'on',
    AllowStoreRestart => 'on',
    DefaultChdir => '~',

    Include => "$tmpdir/*/test.conf",

    IfModules => {
      'mod_delay.c' => {
        DelayEngine => 'off',
      },
    },
  };

  my ($port, $config_user, $config_group) = config_write($setup->{config_file},
    $config);

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
      # Allow for server startup
      sleep(1);

      my $client = ProFTPD::TestSuite::FTP->new('127.0.0.1', $port);
      $client->login($setup->{user}, $setup->{passwd});

      my $filename = 'test.txt';

      my $conn = $client->retr_raw($filename);
      if ($conn) {
        die("RETR $filename succeeded unexpectedly");
      }

      my $resp_code = $client->response_code();
      my $resp_msg = $client->response_msg();

      my $expected = 550;
      $self->assert($expected == $resp_code,
        test_msg("Expected response code $expected, got $resp_code"));

      $expected = "$filename: Operation not permitted";
      $self->assert($expected eq $resp_msg,
        test_msg("Expected response message '$expected', got '$resp_msg'"));

      $client->quit();
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

sub include_limit {
  my $self = shift;
  my $tmpdir = $self->{tmpdir};
  my $setup = test_setup($tmpdir, 'config');

  my $include_config = File::Spec->rel2abs("$tmpdir/include.conf");
  if (open(my $fh, "> $include_config")) {
    print $fh "Allow from 127.0.0.1\n";
    unless (close($fh)) {
      die("Can't write $include_config: $!");
    }

  } else {
    die("Can't open $include_config: $!");
  }

  my $ftpaccess_file = File::Spec->rel2abs("$setup->{home_dir}/.ftpaccess");
  if (open(my $fh, "> $ftpaccess_file")) {
    print $fh <<EOC;
<Limit LOGIN>
  Include $include_config
  DenyAll
</Limit>
EOC
    unless (close($fh)) {
      die("Can't write $ftpaccess_file: $!");
    }

  } else {
    die("Can't open $ftpaccess_file: $!");
  }

  my $config = {
    PidFile => $setup->{pid_file},
    ScoreboardFile => $setup->{scoreboard_file},
    SystemLog => $setup->{log_file},

    AuthUserFile => $setup->{auth_user_file},
    AuthGroupFile => $setup->{auth_group_file},
    AuthOrder => 'mod_auth_file.c',

    AllowOverride => 'on',
    DefaultChdir => '~',

    IfModules => {
      'mod_delay.c' => {
        DelayEngine => 'off',
      },
    },
  };

  my ($port, $config_user, $config_group) = config_write($setup->{config_file},
    $config);
  if (open(my $fh, ">> $setup->{config_file}")) {
    print $fh <<EOC;
<Limit LOGIN>
  Include $include_config
  DenyAll
</Limit>
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

  my $ex;

  # Fork child
  $self->handle_sigchld();
  defined(my $pid = fork()) or die("Can't fork: $!");
  if ($pid) {
    eval {
      my $client = ProFTPD::TestSuite::FTP->new('127.0.0.1', $port, 0);
      $client->login($setup->{user}, $setup->{passwd});
      $client->quit();
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
