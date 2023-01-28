package ProFTPD::Tests::Commands::ABOR;

use lib qw(t/lib);
use base qw(ProFTPD::TestSuite::Child);
use strict;

use File::Spec;
use IO::Handle;
use Socket;

use ProFTPD::TestSuite::FTP;
use ProFTPD::TestSuite::Utils qw(:auth :config :features :running :test :testsuite);

$| = 1;

my $order = 0;

# NOTE: Net::FTP::abort() automatically does all of the following, in this
# order:
#  - Sends TCP OOB marker
#  - Sends ABOR command
#  - Closes data connection
#
# Thus for testing other client behavior, where data EOF might occur before
# the ABOR command, or ABOR might be sent by itself without OOB, we use other
# custom functions.
#
# "ABOR only" tests will refer to sending _just_ the ABOR command; "data EOF"
# tests refer to only closing the data connections.  And then we will have
# tests for data EOF before ABOR, not after.

my $TESTS = {
  abor_retr_binary_ok => {
    order => ++$order,
    test_class => [qw(forking)],
  },

  abor_retr_ascii_ok => {
    order => ++$order,
    test_class => [qw(forking)],
  },

  abor_retr_ascii_largefile_ok => {
    order => ++$order,
    test_class => [qw(forking)],
  },

  abor_retr_ascii_largefile_followed_by_list_ok => {
    order => ++$order,
    test_class => [qw(forking)],
  },

  abor_retr_binary_largefile_followed_by_retr_ok => {
    order => ++$order,
    test_class => [qw(forking)],
  },

  abor_retr_binary_largefile_with_sendfile => {
    order => ++$order,
    test_class => [qw(feature_sendfile forking)],
  },

  abor_retr_binary_largefile_without_sendfile => {
    order => ++$order,
    test_class => [qw(forking)],
  },

  abor_stor_binary_ok => {
    order => ++$order,
    test_class => [qw(forking)],
  },

  abor_stor_ascii_ok => {
    order => ++$order,
    test_class => [qw(forking)],
  },

  abor_with_cyrillic_encoding_ok => {
    order => ++$order,
    test_class => [qw(feature_nls forking)],
  },

  abor_no_xfer_ok => {
    order => ++$order,
    test_class => [qw(forking)],
  },

  abor_list_ok => {
    order => ++$order,
    test_class => [qw(forking)],
  },

  abor_mlsd_ok => {
    order => ++$order,
    test_class => [qw(forking)],
  },

  abor_only_retr_ascii => {
    order => ++$order,
    test_class => [qw(forking)],
  },

  abor_only_retr_binary_with_sendfile => {
    order => ++$order,
    test_class => [qw(feature_sendfile forking)],
  },

  abor_only_retr_binary_without_sendfile => {
    order => ++$order,
    test_class => [qw(forking)],
  },

  abor_only_stor_ascii => {
    order => ++$order,
    test_class => [qw(forking)],
  },

  abor_only_stor_binary => {
    order => ++$order,
    test_class => [qw(forking)],
  },

  abor_only_list => {
    order => ++$order,
    test_class => [qw(forking)],
  },

  abor_only_mlsd => {
    order => ++$order,
    test_class => [qw(forking)],
  },

  abor_only_no_xfer => {
    order => ++$order,
    test_class => [qw(forking)],
  },

  data_eof_retr_ascii => {
    order => ++$order,
    test_class => [qw(forking)],
  },

  data_eof_retr_binary_with_sendfile => {
    order => ++$order,
    test_class => [qw(feature_sendfile forking)],
  },

  data_eof_retr_binary_without_sendfile => {
    order => ++$order,
    test_class => [qw(forking)],
  },

  data_eof_stor_ascii => {
    order => ++$order,
    test_class => [qw(forking)],
  },

  data_eof_stor_binary => {
    order => ++$order,
    test_class => [qw(forking)],
  },

  data_eof_list => {
    order => ++$order,
    test_class => [qw(forking)],
  },

  data_eof_mlsd => {
    order => ++$order,
    test_class => [qw(forking)],
  },

  data_eof_before_abor_retr_ascii => {
    order => ++$order,
    test_class => [qw(forking)],
  },

  data_eof_before_abor_retr_binary_with_sendfile => {
    order => ++$order,
    test_class => [qw(feature_sendfile forking)],
  },

  data_eof_before_abor_retr_binary_without_sendfile => {
    order => ++$order,
    test_class => [qw(forking)],
  },

  data_eof_before_abor_stor_ascii => {
    order => ++$order,
    test_class => [qw(forking)],
  },

  data_eof_before_abor_stor_binary => {
    order => ++$order,
    test_class => [qw(forking)],
  },

  data_eof_before_abor_list => {
    order => ++$order,
    test_class => [qw(forking)],
  },

  data_eof_before_abor_mlsd => {
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

sub abor_retr_binary_ok {
  my $self = shift;
  my $tmpdir = $self->{tmpdir};
  my $setup = test_setup($tmpdir, 'abor');

  my $test_file = File::Spec->rel2abs($setup->{config_file});

  my $config = {
    PidFile => $setup->{pid_file},
    ScoreboardFile => $setup->{scoreboard_file},
    SystemLog => $setup->{log_file},
    TraceLog => $setup->{log_file},
    Trace => 'DEFAULT:10',

    AuthUserFile => $setup->{auth_user_file},
    AuthGroupFile => $setup->{auth_group_file},
    AuthOrder => 'mod_auth_file.c',

    TimeoutLinger => 5,

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
      my $client = ProFTPD::TestSuite::FTP->new('127.0.0.1', $port);
      $client->login($setup->{user}, $setup->{passwd});
      $client->pasv();
      $client->type('binary');

      my $conn = $client->retr_raw($test_file);
      unless ($conn) {
        die("Failed to RETR: " . $client->response_code() . " " .
          $client->response_msg());
      }

      # Read 128 bytes of the file, then abort the download
      my $buf;
      $conn->read($buf, 128, 30);
      eval { $conn->abort() };

      my $resp_code = $client->response_code();
      my $resp_msg = $client->response_msg();
      $self->assert_transfer_ok($resp_code, $resp_msg, 1);

      # Make sure the control connection did not close because of the abort.
      ($resp_code, $resp_msg) = $client->quit();

      my $expected = 221;
      $self->assert($expected == $resp_code,
        test_msg("Expected response code $expected, got $resp_code"));

      $expected = 'Goodbye.';
      $self->assert($expected eq $resp_msg,
        test_msg("Expected response message '$expected', got '$resp_msg'"));
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

sub abor_retr_ascii_ok {
  my $self = shift;
  my $tmpdir = $self->{tmpdir};
  my $setup = test_setup($tmpdir, 'abor');

  my $test_file = File::Spec->rel2abs($setup->{config_file});

  my $config = {
    PidFile => $setup->{pid_file},
    ScoreboardFile => $setup->{scoreboard_file},
    SystemLog => $setup->{log_file},
    TraceLog => $setup->{log_file},
    Trace => 'DEFAULT:10',

    AuthUserFile => $setup->{auth_user_file},
    AuthGroupFile => $setup->{auth_group_file},
    AuthOrder => 'mod_auth_file.c',

    TimeoutLinger => 5,

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
      my $client = ProFTPD::TestSuite::FTP->new('127.0.0.1', $port);
      $client->login($setup->{user}, $setup->{passwd});
      $client->pasv();
      $client->type('ascii');

      my $conn = $client->retr_raw($test_file);
      unless ($conn) {
        die("Failed to RETR: " . $client->response_code() . " " .
          $client->response_msg());
      }

      # Read 128 bytes of the file, then abort the download
      my $buf;
      $conn->read($buf, 128, 30);
      eval { $conn->abort() };

      my $resp_code = $client->response_code();
      my $resp_msg = $client->response_msg();
      $self->assert_transfer_ok($resp_code, $resp_msg, 1);

      # Make sure the control connection did not close because of the abort.
      ($resp_code, $resp_msg) = $client->quit();

      my $expected = 221;
      $self->assert($expected == $resp_code,
        test_msg("Expected response code $expected, got $resp_code"));

      $expected = 'Goodbye.';
      $self->assert($expected eq $resp_msg,
        test_msg("Expected response message '$expected', got '$resp_msg'"));
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

sub abor_retr_ascii_largefile_ok {
  my $self = shift;
  my $tmpdir = $self->{tmpdir};
  my $setup = test_setup($tmpdir, 'abor');

  my $test_file = File::Spec->rel2abs("$tmpdir/largefile.txt");
  if (open(my $fh, "> $test_file")) {
    print $fh "ABCDEFG\n" x 40960;

    unless (close($fh)) {
      die("Can't write $test_file: $!");
    }

  } else {
    die("Can't open $test_file: $!");
  }

  my $config = {
    PidFile => $setup->{pid_file},
    ScoreboardFile => $setup->{scoreboard_file},
    SystemLog => $setup->{log_file},
    TraceLog => $setup->{log_file},
    Trace => 'DEFAULT:10',

    AuthUserFile => $setup->{auth_user_file},
    AuthGroupFile => $setup->{auth_group_file},
    AuthOrder => 'mod_auth_file.c',

    TimeoutLinger => 5,

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
      my $client = ProFTPD::TestSuite::FTP->new('127.0.0.1', $port);
      $client->login($setup->{user}, $setup->{passwd});
      $client->pasv();
      $client->type('ascii');

      my $conn = $client->retr_raw($test_file);
      unless ($conn) {
        die("Failed to RETR: " . $client->response_code() . " " .
          $client->response_msg());
      }

      # Read 128 bytes of the file, then abort the download
      my $buf;
      $conn->read($buf, 128, 30);
      eval { $conn->abort() };

      my $resp_code = $client->response_code();
      my $resp_msg = $client->response_msg();
      $self->assert_transfer_ok($resp_code, $resp_msg, 1);

      # We expect 2 responses here: first, a 426 for the aborted data transfer,
      # followed by 226 for the successful ABOR command.  Order matters.
      my $resp_msgs = $client->response_msgs();

      my $resp_nmsgs = scalar(@$resp_msgs);
      $self->assert($resp_nmsgs == 2,
        test_msg("Expected 2 responses, got $resp_nmsgs"));

      my $expected = 'Transfer aborted. Data connection closed.';
      $self->assert($expected eq $resp_msgs->[0],
        test_msg("Expected response message '$expected', got '$resp_msgs->[0]'"));

      $expected = 'Abort successful';
      $self->assert($expected eq $resp_msgs->[1],
        test_msg("Expected response message '$expected', got '$resp_msgs->[1]'"));

      # Make sure the control connection did not close because of the abort.
      ($resp_code, $resp_msg) = $client->quit();

      $expected = 221;
      $self->assert($expected == $resp_code,
        test_msg("Expected response code $expected, got $resp_code"));

      $expected = 'Goodbye.';
      $self->assert($expected eq $resp_msg,
        test_msg("Expected response message '$expected', got '$resp_msg'"));
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

sub abor_retr_ascii_largefile_followed_by_list_ok {
  my $self = shift;
  my $tmpdir = $self->{tmpdir};
  my $setup = test_setup($tmpdir, 'abor');

  my $test_file = File::Spec->rel2abs("$tmpdir/largefile.txt");
  if (open(my $fh, "> $test_file")) {
    print $fh "ABCDEFG\n" x 4096;

    unless (close($fh)) {
      die("Can't write $test_file: $!");
    }

  } else {
    die("Can't open $test_file: $!");
  }

  my $config = {
    PidFile => $setup->{pid_file},
    ScoreboardFile => $setup->{scoreboard_file},
    SystemLog => $setup->{log_file},
    TraceLog => $setup->{log_file},
    Trace => 'DEFAULT:10',

    AuthUserFile => $setup->{auth_user_file},
    AuthGroupFile => $setup->{auth_group_file},
    AuthOrder => 'mod_auth_file.c',

    TimeoutLinger => 5,

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
      my $client = ProFTPD::TestSuite::FTP->new('127.0.0.1', $port);
      $client->login($setup->{user}, $setup->{passwd});
      $client->pasv();
      $client->type('ascii');

      my $conn = $client->retr_raw($test_file);
      unless ($conn) {
        die("Failed to RETR: " . $client->response_code() . " " .
          $client->response_msg());
      }

      # Read 128 bytes of the file, then abort the download
      my $buf;
      $conn->read($buf, 128, 30);
      eval { $conn->abort() };

      my $resp_code = $client->response_code();
      my $resp_msg = $client->response_msg();
      $self->assert_transfer_ok($resp_code, $resp_msg, 1);

      # Make sure we can do another data transfer after the abort
      $conn = $client->list_raw();
      unless ($conn) {
        die("Failed to LIST: " . $client->response_code() . " " .
          $client->response_msg());
      }

      $buf = '';
      my $info;
      while ($conn->read($info, 8192, 30)) {
        $buf .= $info;
      }

      eval { $conn->close() };

      # We have to be careful of the fact that readdir returns directory
      # entries in an unordered fashion.
      my $res = {};
      my $lines = [split(/\n/, $buf)];
      foreach my $line (@$lines) {
        if ($line =~ /^\S+\s+\d+\s+\S+\s+\S+\s+.*?\s+(\S+)$/) {
          $res->{$1} = 1;
        }
      }

      my $expected = {
        'abor.conf' => 1,
        'abor.group' => 1,
        'abor.passwd' => 1,
        'abor.pid' => 1,
        'abor.scoreboard' => 1,
        'abor.scoreboard.lck' => 1,
        'largefile.txt' => 1,
      };

      my $ok = 1;
      my $mismatch;
      foreach my $name (keys(%$res)) {
        unless (defined($expected->{$name})) {
          $mismatch = $name;
          $ok = 0;
          last;
        }
      }

      unless ($ok) {
        die("Unexpected name '$mismatch' appeared in LIST data")
      }

      # Make sure the control connection did not close because of the abort.
      ($resp_code, $resp_msg) = $client->quit();

      $expected = 221;
      $self->assert($expected == $resp_code,
        test_msg("Expected response code $expected, got $resp_code"));

      $expected = 'Goodbye.';
      $self->assert($expected eq $resp_msg,
        test_msg("Expected response message '$expected', got '$resp_msg'"));
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

sub abor_retr_binary_largefile_followed_by_retr_ok {
  my $self = shift;
  my $tmpdir = $self->{tmpdir};
  my $setup = test_setup($tmpdir, 'abor');

  my $test_file = File::Spec->rel2abs("$tmpdir/largefile.txt");
  if (open(my $fh, "> $test_file")) {
    print $fh "ABCDEFG\n" x 16384;

    unless (close($fh)) {
      die("Can't write $test_file: $!");
    }

  } else {
    die("Can't open $test_file: $!");
  }

  my $test_filesz = -s $test_file;

  my $config = {
    PidFile => $setup->{pid_file},
    ScoreboardFile => $setup->{scoreboard_file},
    SystemLog => $setup->{log_file},
    TraceLog => $setup->{log_file},
    Trace => 'DEFAULT:10',

    AuthUserFile => $setup->{auth_user_file},
    AuthGroupFile => $setup->{auth_group_file},
    AuthOrder => 'mod_auth_file.c',

    TimeoutLinger => 1,
    TimeoutIdle => 15,
    UseSendfile => 'off',

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
      my $client = ProFTPD::TestSuite::FTP->new('127.0.0.1', $port);
      $client->login($setup->{user}, $setup->{passwd});
      $client->pasv();
      $client->type('binary');

      my $conn = $client->retr_raw($test_file);
      unless ($conn) {
        die("Failed to RETR: " . $client->response_code() . " " .
          $client->response_msg());
      }

      my $buf;
      my $count = 0;
      while ($count != $test_filesz) {
        $count += $conn->read($buf, 8192, 30);

        if ($count > ($test_filesz - 8192)) {
          eval { $conn->abort() };
          last;
        }
      }

      my $resp_code = $client->response_code();
      my $resp_msg = $client->response_msg();
      $self->assert_transfer_ok($resp_code, $resp_msg, 1);

      # Make sure we can do another data transfer (this time, a RETR) after
      # the abort

      $client->pasv();
      $client->type('binary');
      $conn = $client->retr_raw($test_file);
      unless ($conn) {
        die("Failed to RETR: " . $client->response_code() . " " .
          $client->response_msg());
      }

      while ($conn->read($buf, 8192, 30)) {
      }

      eval { $conn->close() };

      $resp_code = $client->response_code();
      $resp_msg = $client->response_msg();
      $self->assert_transfer_ok($resp_code, $resp_msg);

      # Make sure the control connection did not close because of the abort.
      ($resp_code, $resp_msg) = $client->quit();

      my $expected = 221;
      $self->assert($expected == $resp_code,
        test_msg("Expected response code $expected, got $resp_code"));

      $expected = 'Goodbye.';
      $self->assert($expected eq $resp_msg,
        test_msg("Expected response message '$expected', got '$resp_msg'"));
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

sub abor_retr_binary_largefile_with_sendfile {
  my $self = shift;
  my $tmpdir = $self->{tmpdir};
  my $setup = test_setup($tmpdir, 'abor');

  my $test_file = File::Spec->rel2abs("$tmpdir/largefile.txt");
  if (open(my $fh, "> $test_file")) {
    print $fh "ABCDEFG\n" x 40960;

    unless (close($fh)) {
      die("Can't write $test_file: $!");
    }

  } else {
    die("Can't open $test_file: $!");
  }

  my $config = {
    PidFile => $setup->{pid_file},
    ScoreboardFile => $setup->{scoreboard_file},
    SystemLog => $setup->{log_file},
    TraceLog => $setup->{log_file},
    Trace => 'DEFAULT:10',

    AuthUserFile => $setup->{auth_user_file},
    AuthGroupFile => $setup->{auth_group_file},
    AuthOrder => 'mod_auth_file.c',

    TimeoutLinger => 5,

    IfModules => {
      'mod_delay.c' => {
        DelayEngine => 'off',
      },
    },
  };

  if (feature_have_feature_enabled('sendfile')) {
    $config->{UseSendfile} = 'on';
  }

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
      my $client = ProFTPD::TestSuite::FTP->new('127.0.0.1', $port);
      $client->login($setup->{user}, $setup->{passwd});
      $client->pasv();
      $client->type('binary');

      my $conn = $client->retr_raw($test_file);
      unless ($conn) {
        die("Failed to RETR: " . $client->response_code() . " " .
          $client->response_msg());
      }

      # Read 128 bytes of the file, then abort the download
      my $buf;
      $conn->read($buf, 128, 30);
      eval { $conn->abort() };

      my $resp_code = $client->response_code();
      my $resp_msg = $client->response_msg();
      $self->assert_transfer_ok($resp_code, $resp_msg, 1);

      # We expect 2 responses here: first, a 426 for the aborted data transfer,
      # followed by 226 for the successful ABOR command.  Order matters.
      my $resp_msgs = $client->response_msgs();

      my $resp_nmsgs = scalar(@$resp_msgs);
      $self->assert($resp_nmsgs == 2,
        test_msg("Expected 2 responses, got $resp_nmsgs"));

      my $expected = 'Transfer aborted. Data connection closed.';
      $self->assert($expected eq $resp_msgs->[0],
        test_msg("Expected response message '$expected', got '$resp_msgs->[0]'"));

      $expected = 'Abort successful';
      $self->assert($expected eq $resp_msgs->[1],
        test_msg("Expected response message '$expected', got '$resp_msgs->[1]'"));

      # Make sure the control connection did not close because of the abort.
      ($resp_code, $resp_msg) = $client->quit();

      $expected = 221;
      $self->assert($expected == $resp_code,
        test_msg("Expected response code $expected, got $resp_code"));

      $expected = 'Goodbye.';
      $self->assert($expected eq $resp_msg,
        test_msg("Expected response message '$expected', got '$resp_msg'"));
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

sub abor_retr_binary_largefile_without_sendfile {
  my $self = shift;
  my $tmpdir = $self->{tmpdir};
  my $setup = test_setup($tmpdir, 'abor');

  my $test_file = File::Spec->rel2abs("$tmpdir/largefile.txt");
  if (open(my $fh, "> $test_file")) {
    print $fh "ABCDEFG\n" x 40960;

    unless (close($fh)) {
      die("Can't write $test_file: $!");
    }

  } else {
    die("Can't open $test_file: $!");
  }

  my $config = {
    PidFile => $setup->{pid_file},
    ScoreboardFile => $setup->{scoreboard_file},
    SystemLog => $setup->{log_file},
    TraceLog => $setup->{log_file},
    Trace => 'DEFAULT:10',

    AuthUserFile => $setup->{auth_user_file},
    AuthGroupFile => $setup->{auth_group_file},
    AuthOrder => 'mod_auth_file.c',

    TimeoutLinger => 5,
    UseSendfile => 'off',

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
      my $client = ProFTPD::TestSuite::FTP->new('127.0.0.1', $port);
      $client->login($setup->{user}, $setup->{passwd});
      $client->pasv();
      $client->type('binary');

      my $conn = $client->retr_raw($test_file);
      unless ($conn) {
        die("Failed to RETR: " . $client->response_code() . " " .
          $client->response_msg());
      }

      # Read 128 bytes of the file, then abort the download
      my $buf;
      $conn->read($buf, 128, 30);
      eval { $conn->abort() };

      my $resp_code = $client->response_code();
      my $resp_msg = $client->response_msg();
      $self->assert_transfer_ok($resp_code, $resp_msg, 1);

      # We expect 2 responses here: first, a 426 for the aborted data transfer,
      # followed by 226 for the successful ABOR command.  Order matters.
      my $resp_msgs = $client->response_msgs();

      my $resp_nmsgs = scalar(@$resp_msgs);
      $self->assert($resp_nmsgs == 2,
        test_msg("Expected 2 responses, got $resp_nmsgs"));

      my $expected = 'Transfer aborted. Data connection closed.';
      $self->assert($expected eq $resp_msgs->[0],
        test_msg("Expected response message '$expected', got '$resp_msgs->[0]'"));

      $expected = 'Abort successful';
      $self->assert($expected eq $resp_msgs->[1],
        test_msg("Expected response message '$expected', got '$resp_msgs->[1]'"));

      # Make sure the control connection did not close because of the abort.
      ($resp_code, $resp_msg) = $client->quit();

      $expected = 221;
      $self->assert($expected == $resp_code,
        test_msg("Expected response code $expected, got $resp_code"));

      $expected = 'Goodbye.';
      $self->assert($expected eq $resp_msg,
        test_msg("Expected response message '$expected', got '$resp_msg'"));
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

sub abor_stor_binary_ok {
  my $self = shift;
  my $tmpdir = $self->{tmpdir};
  my $setup = test_setup($tmpdir, 'abor');

  my $test_file = File::Spec->rel2abs("$tmpdir/foo.txt");

  my $config = {
    PidFile => $setup->{pid_file},
    ScoreboardFile => $setup->{scoreboard_file},
    SystemLog => $setup->{log_file},
    TraceLog => $setup->{log_file},
    Trace => 'DEFAULT:10',

    AuthUserFile => $setup->{auth_user_file},
    AuthGroupFile => $setup->{auth_group_file},
    AuthOrder => 'mod_auth_file.c',

    TimeoutLinger => 5,

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
      my $client = ProFTPD::TestSuite::FTP->new('127.0.0.1', $port);
      $client->login($setup->{user}, $setup->{passwd});
      $client->pasv();
      $client->type('binary');

      my $conn = $client->stor_raw($test_file);
      unless ($conn) {
        die("Failed to STOR: " . $client->response_code() . " " .
          $client->response_msg());
      }

      # Write data to the file, then abort the upload
      my $buf = "A\r\nB\r\nC\r\nD\r\n";
      $conn->write($buf, length($buf));
      eval { $conn->abort() };

      my $resp_code = $client->response_code();
      my $resp_msg = $client->response_msg();
      $self->assert_transfer_ok($resp_code, $resp_msg, 1);

      # We expect 2 responses here: first, a 426 for the aborted data transfer,
      # followed by 226 for the successful ABOR command.  Order matters.
      my $resp_msgs = $client->response_msgs();

      my $resp_nmsgs = scalar(@$resp_msgs);
      $self->assert($resp_nmsgs == 2,
        test_msg("Expected 2 responses, got $resp_nmsgs"));

      my $expected = 'Transfer aborted. Data connection closed.';
      $self->assert($expected eq $resp_msgs->[0],
        test_msg("Expected response message '$expected', got '$resp_msgs->[0]'"));

      $expected = 'Abort successful';
      $self->assert($expected eq $resp_msgs->[1],
        test_msg("Expected response message '$expected', got '$resp_msgs->[1]'"));

      # Make sure the control connection did not close because of the abort.
      ($resp_code, $resp_msg) = $client->quit();

      $expected = 221;
      $self->assert($expected == $resp_code,
        test_msg("Expected response message $expected, got $resp_code"));

      $expected = 'Goodbye.';
      $self->assert($expected eq $resp_msg,
        test_msg("Expected response message '$expected', got '$resp_msg'"));
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

sub abor_stor_ascii_ok {
  my $self = shift;
  my $tmpdir = $self->{tmpdir};
  my $setup = test_setup($tmpdir, 'abor');

  my $test_file = File::Spec->rel2abs("$tmpdir/foo.txt");

  my $config = {
    PidFile => $setup->{pid_file},
    ScoreboardFile => $setup->{scoreboard_file},
    SystemLog => $setup->{log_file},
    TraceLog => $setup->{log_file},
    Trace => 'DEFAULT:10',

    AuthUserFile => $setup->{auth_user_file},
    AuthGroupFile => $setup->{auth_group_file},
    AuthOrder => 'mod_auth_file.c',

    TimeoutLinger => 5,

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
      my $client = ProFTPD::TestSuite::FTP->new('127.0.0.1', $port);
      $client->login($setup->{user}, $setup->{passwd});
      $client->pasv();
      $client->type('ascii');

      my $conn = $client->stor_raw($test_file);
      unless ($conn) {
        die("Failed to STOR: " . $client->response_code() . " " .
          $client->response_msg());
      }

      # Write data to the file, then abort the upload
      my $buf = "A\r\nB\r\nC\r\nD\r\n";
      $conn->write($buf, length($buf));
      eval { $conn->abort() };

      my $resp_code = $client->response_code();
      my $resp_msg = $client->response_msg();
      $self->assert_transfer_ok($resp_code, $resp_msg, 1);

      # We expect 2 responses here: first, a 426 for the aborted data transfer,
      # followed by 226 for the successful ABOR command.  Order matters.
      my $resp_msgs = $client->response_msgs();

      my $resp_nmsgs = scalar(@$resp_msgs);
      $self->assert($resp_nmsgs == 2,
        test_msg("Expected 2 responses, got $resp_nmsgs"));

      my $expected = 'Transfer aborted. Data connection closed.';
      $self->assert($expected eq $resp_msgs->[0],
        test_msg("Expected response message '$expected', got '$resp_msgs->[0]'"));

      $expected = 'Abort successful';
      $self->assert($expected eq $resp_msgs->[1],
        test_msg("Expected response message '$expected', got '$resp_msgs->[1]'"));

      # Make sure the control connection did not close because of the abort.
      ($resp_code, $resp_msg) = $client->quit();

      $expected = 221;
      $self->assert($expected == $resp_code,
        test_msg("Expected response message $expected, got $resp_code"));

      $expected = 'Goodbye.';
      $self->assert($expected eq $resp_msg,
        test_msg("Expected response message '$expected', got '$resp_msg'"));
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

sub abor_with_cyrillic_encoding_ok {
  my $self = shift;
  my $tmpdir = $self->{tmpdir};
  my $setup = test_setup($tmpdir, 'abor');

  my $test_file = File::Spec->rel2abs($setup->{config_file});

  my $config = {
    PidFile => $setup->{pid_file},
    ScoreboardFile => $setup->{scoreboard_file},
    SystemLog => $setup->{log_file},
    TraceLog => $setup->{log_file},
    Trace => 'DEFAULT:10',

    AuthUserFile => $setup->{auth_user_file},
    AuthGroupFile => $setup->{auth_group_file},
    AuthOrder => 'mod_auth_file.c',

    TimeoutLinger => 15,

    IfModules => {
      'mod_delay.c' => {
        DelayEngine => 'off',
      },

      'mod_lang.c' => {
        UseEncoding => 'koi8-r cp1251',
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
      my $client = ProFTPD::TestSuite::FTP->new('127.0.0.1', $port);
      $client->login($setup->{user}, $setup->{passwd});
      $client->pasv();
      $client->type('binary');

      my $conn = $client->retr_raw($test_file);
      unless ($conn) {
        die("Failed to RETR: " . $client->response_code() . " " .
          $client->response_msg());
      }

      # Read one byte of the file, then abort the download
      my $buf;
      $conn->read($buf, 1, 30);
      eval { $conn->abort() };

      my $resp_code = $client->response_code();
      my $resp_msg = $client->response_msg();

      my $expected;

      # We expect the ABOR command to be misunderstood here, since it will
      # be prefaced by the Telnet IAC/IP, IAC/DM sequences.  With Cyrillic
      # charsets, the Telnet sequences are not stripped out (since the values
      # collide with some of the Cyrillic charset values); this means that
      # rather than seeing an "ABOR" string, the server sees a string like
      # "<FF><F4><FF><F2>ABOR", which it clearly will not handle.

      $expected = 500;
      $self->assert($expected == $resp_code,
        test_msg("Expected response code $expected, got $resp_code"));

      $expected = 'ABOR not understood';
      $self->assert(qr/$expected/, $resp_msg,
        test_msg("Expected response message '$expected', got '$resp_msg'"));

      # Make sure the control connection did not close because of the abort.
      ($resp_code, $resp_msg) = $client->quit();

      $expected = 221;
      $self->assert($expected == $resp_code,
        test_msg("Expected response code $expected, got $resp_code"));

      $expected = 'Goodbye.';
      $self->assert($expected eq $resp_msg,
        test_msg("Expected response message '$expected', got '$resp_msg'"));
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

sub abor_no_xfer_ok {
  my $self = shift;
  my $tmpdir = $self->{tmpdir};
  my $setup = test_setup($tmpdir, 'abor');

  my $test_file = File::Spec->rel2abs($setup->{config_file});

  my $config = {
    PidFile => $setup->{pid_file},
    ScoreboardFile => $setup->{scoreboard_file},
    SystemLog => $setup->{log_file},
    TraceLog => $setup->{log_file},
    Trace => 'DEFAULT:10',

    AuthUserFile => $setup->{auth_user_file},
    AuthGroupFile => $setup->{auth_group_file},
    AuthOrder => 'mod_auth_file.c',

    TimeoutLinger => 5,

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
      my $client = ProFTPD::TestSuite::FTP->new('127.0.0.1', $port);
      $client->login($setup->{user}, $setup->{passwd});
      $client->abort();

      my $resp_code = $client->response_code();
      my $resp_msg = $client->response_msg();
      $self->assert_transfer_ok($resp_code, $resp_msg, 1);

      # Make sure the control connection did not close because of the abort.
      ($resp_code, $resp_msg) = $client->quit();

      my $expected = 221;
      $self->assert($expected == $resp_code,
        test_msg("Expected response code $expected, got $resp_code"));

      $expected = 'Goodbye.';
      $self->assert($expected eq $resp_msg,
        test_msg("Expected response message '$expected', got '$resp_msg'"));
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

sub abor_list_ok {
  my $self = shift;
  my $tmpdir = $self->{tmpdir};
  my $setup = test_setup($tmpdir, 'abor');

  my $config = {
    PidFile => $setup->{pid_file},
    ScoreboardFile => $setup->{scoreboard_file},
    SystemLog => $setup->{log_file},
    TraceLog => $setup->{log_file},
    Trace => 'DEFAULT:10',

    AuthUserFile => $setup->{auth_user_file},
    AuthGroupFile => $setup->{auth_group_file},
    AuthOrder => 'mod_auth_file.c',

    TimeoutLinger => 5,

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
      my $client = ProFTPD::TestSuite::FTP->new('127.0.0.1', $port);
      $client->login($setup->{user}, $setup->{passwd});

      # Use a recursive listing, to generate more data, such that it will
      # not all fit in the transfer buffer, so we can interrupt the buffer.
      # A too-small request will fit in the buffer, and be fulfilled, before
      # our abort is read.
      my $conn = $client->list_raw('-R /');
      unless ($conn) {
        die("Failed to LIST: " . $client->response_code() . " " .
          $client->response_msg());
      }

      # Read 128 bytes of data, then abort the download
      my $buf;
      $conn->read($buf, 128, 30);
      eval { $conn->abort() };

      my $resp_code = $client->response_code();
      my $resp_msg = $client->response_msg();
      $self->assert_transfer_ok($resp_code, $resp_msg, 1);

      # We expect 2 responses here: first, a 426 for the aborted data transfer,
      # followed by 226 for the successful ABOR command.  Order matters.
      my $resp_msgs = $client->response_msgs();

      my $resp_nmsgs = scalar(@$resp_msgs);
      $self->assert($resp_nmsgs == 2,
        test_msg("Expected 2 responses, got $resp_nmsgs"));

      my $expected = 'Transfer aborted. Data connection closed.';
      $self->assert($expected eq $resp_msgs->[0],
        test_msg("Expected response message '$expected', got '$resp_msgs->[0]'"));

      $expected = 'Abort successful';
      $self->assert($expected eq $resp_msgs->[1],
        test_msg("Expected response message '$expected', got '$resp_msgs->[1]'"));

      # Make sure the control connection did not close because of the abort.
      ($resp_code, $resp_msg) = $client->quit();

      $expected = 221;
      $self->assert($expected == $resp_code,
        test_msg("Expected response code $expected, got $resp_code"));

      $expected = 'Goodbye.';
      $self->assert($expected eq $resp_msg,
        test_msg("Expected response message '$expected', got '$resp_msg'"));
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

sub abor_mlsd_ok {
  my $self = shift;
  my $tmpdir = $self->{tmpdir};
  my $setup = test_setup($tmpdir, 'abor');

  # Generate enough files in the directory to lead to a large enough response,
  # such that the response does not all fit in the initial transfer buffer.
  for (my $i = 0; $i < 5000; $i++) {
    my $path = File::Spec->rel2abs("$tmpdir/$i.dat");

    if (open(my $fh, "> $path")) {
      close($fh);

    } else {
      die("Can't open $path: $!");
    }
  }

  my $config = {
    PidFile => $setup->{pid_file},
    ScoreboardFile => $setup->{scoreboard_file},
    SystemLog => $setup->{log_file},
    TraceLog => $setup->{log_file},
    Trace => 'DEFAULT:10',

    AuthUserFile => $setup->{auth_user_file},
    AuthGroupFile => $setup->{auth_group_file},
    AuthOrder => 'mod_auth_file.c',

    TimeoutLinger => 5,

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
      my $client = ProFTPD::TestSuite::FTP->new('127.0.0.1', $port);
      $client->login($setup->{user}, $setup->{passwd});

      my $conn = $client->mlsd_raw($tmpdir);
      unless ($conn) {
        die("Failed to MLSD: " . $client->response_code() . " " .
          $client->response_msg());
      }

      # Read 128 bytes of data, then abort the download
      my $buf;
      $conn->read($buf, 128, 30);
      eval { $conn->abort() };

      my $resp_code = $client->response_code();
      my $resp_msg = $client->response_msg();
      $self->assert_transfer_ok($resp_code, $resp_msg, 1);

      # We expect 2 responses here: first, a 426 for the aborted data transfer,
      # followed by 226 for the successful ABOR command.  Order matters.
      my $resp_msgs = $client->response_msgs();

      my $resp_nmsgs = scalar(@$resp_msgs);
      $self->assert($resp_nmsgs == 2,
        test_msg("Expected 2 responses, got $resp_nmsgs"));

      my $expected = 'Transfer aborted. Data connection closed.';
      $self->assert($expected eq $resp_msgs->[0],
        test_msg("Expected response message '$expected', got '$resp_msgs->[0]'"));

      $expected = 'Abort successful';
      $self->assert($expected eq $resp_msgs->[1],
        test_msg("Expected response message '$expected', got '$resp_msgs->[1]'"));

      # Make sure the control connection did not close because of the abort.
      ($resp_code, $resp_msg) = $client->quit();

      $expected = 221;
      $self->assert($expected == $resp_code,
        test_msg("Expected response code $expected, got $resp_code"));

      $expected = 'Goodbye.';
      $self->assert($expected eq $resp_msg,
        test_msg("Expected response message '$expected', got '$resp_msg'"));
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

sub abor_only_retr_ascii {
  my $self = shift;
  my $tmpdir = $self->{tmpdir};
  my $setup = test_setup($tmpdir, 'abor');

  my $test_file = File::Spec->rel2abs("$tmpdir/largefile.txt");
  if (open(my $fh, "> $test_file")) {
    print $fh "ABCDEFG\n" x 40960;

    unless (close($fh)) {
      die("Can't write $test_file: $!");
    }

  } else {
    die("Can't open $test_file: $!");
  }

  my $config = {
    PidFile => $setup->{pid_file},
    ScoreboardFile => $setup->{scoreboard_file},
    SystemLog => $setup->{log_file},
    TraceLog => $setup->{log_file},
    Trace => 'DEFAULT:10 data:20',

    AuthUserFile => $setup->{auth_user_file},
    AuthGroupFile => $setup->{auth_group_file},
    AuthOrder => 'mod_auth_file.c',

    TimeoutLinger => 5,

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
      my $client = ProFTPD::TestSuite::FTP->new('127.0.0.1', $port);
      $client->login($setup->{user}, $setup->{passwd});
      $client->pasv();
      $client->type('ascii');

      my $conn = $client->retr_raw($test_file);
      unless ($conn) {
        die("Failed to RETR: " . $client->response_code() . " " .
          $client->response_msg());
      }

      # Read 128 bytes of the file, then abort the download
      my $buf;
      $conn->read($buf, 128, 30);

      eval { $client->quote('ABOR') };
      unless ($@) {
        die("ABOR succeeded unexpectedly");
      }

      # We expect 2 responses here: first, a 426 for the aborted data transfer,
      # followed by 226 for the successful ABOR command.  Order matters.
      my $resp_msgs = $client->response_msgs();

      my $resp_nmsgs = scalar(@$resp_msgs);
      $self->assert($resp_nmsgs == 2,
        test_msg("Expected 2 responses, got $resp_nmsgs"));

      my $expected = 'Transfer aborted. Data connection closed.';
      $self->assert($expected eq $resp_msgs->[0],
        test_msg("Expected response message '$expected', got '$resp_msgs->[0]'"));

      $expected = 'Abort successful';
      $self->assert($expected eq $resp_msgs->[1],
        test_msg("Expected response message '$expected', got '$resp_msgs->[1]'"));

      # Make sure the control connection did not close because of the abort.
      my ($resp_code, $resp_msg) = $client->quit();

      $expected = 221;
      $self->assert($expected == $resp_code,
        test_msg("Expected response code $expected, got $resp_code"));

      $expected = 'Goodbye.';
      $self->assert($expected eq $resp_msg,
        test_msg("Expected response message '$expected', got '$resp_msg'"));
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

sub abor_only_retr_binary_with_sendfile {
  my $self = shift;
  my $tmpdir = $self->{tmpdir};
  my $setup = test_setup($tmpdir, 'abor');

  my $test_file = File::Spec->rel2abs("$tmpdir/largefile.txt");
  if (open(my $fh, "> $test_file")) {
    print $fh "ABCDEFG\n" x 40960;

    unless (close($fh)) {
      die("Can't write $test_file: $!");
    }

  } else {
    die("Can't open $test_file: $!");
  }

  my $config = {
    PidFile => $setup->{pid_file},
    ScoreboardFile => $setup->{scoreboard_file},
    SystemLog => $setup->{log_file},
    TraceLog => $setup->{log_file},
    Trace => 'DEFAULT:10 data:20',

    AuthUserFile => $setup->{auth_user_file},
    AuthGroupFile => $setup->{auth_group_file},
    AuthOrder => 'mod_auth_file.c',

    TimeoutLinger => 5,

    IfModules => {
      'mod_delay.c' => {
        DelayEngine => 'off',
      },
    },
  };

  if (feature_have_feature_enabled('sendfile')) {
    $config->{UseSendfile} = 'on';
  }

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
      my $client = ProFTPD::TestSuite::FTP->new('127.0.0.1', $port);
      $client->login($setup->{user}, $setup->{passwd});
      $client->type('binary');

      my $conn = $client->retr_raw($test_file);
      unless ($conn) {
        die("Failed to RETR: " . $client->response_code() . " " .
          $client->response_msg());
      }

      # Read 128 bytes of the file, then abort the download
      my $buf;
      $conn->read($buf, 128, 30);

      # With sendfile enabled, it's quite probable that all of the data will
      # have been transferred already.  Thus we expect to see two success
      # responses here.
      eval { $client->quote('ABOR') };
      if ($@) {
        die("ABOR failed unexpectedly");
      }

      # We expect 2 responses here: first, a 226 for the (completed) data
      # transfer, followed by 226 for the successful ABOR command.  Order
      # matters.
      my $resp_msgs = $client->response_msgs();

      my $resp_nmsgs = scalar(@$resp_msgs);
      $self->assert($resp_nmsgs == 2,
        test_msg("Expected 2 responses, got $resp_nmsgs"));

      my $expected = 'Transfer complete';
      $self->assert($expected eq $resp_msgs->[0],
        test_msg("Expected response message '$expected', got '$resp_msgs->[0]'"));

      $expected = 'Abort successful';
      $self->assert($expected eq $resp_msgs->[1],
        test_msg("Expected response message '$expected', got '$resp_msgs->[1]'"));

      # Make sure the control connection did not close because of the abort.
      my ($resp_code, $resp_msg) = $client->quit();

      $expected = 221;
      $self->assert($expected == $resp_code,
        test_msg("Expected response code $expected, got $resp_code"));

      $expected = 'Goodbye.';
      $self->assert($expected eq $resp_msg,
        test_msg("Expected response message '$expected', got '$resp_msg'"));
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

sub abor_only_retr_binary_without_sendfile {
  my $self = shift;
  my $tmpdir = $self->{tmpdir};
  my $setup = test_setup($tmpdir, 'abor');

  my $test_file = File::Spec->rel2abs("$tmpdir/largefile.txt");
  if (open(my $fh, "> $test_file")) {
    print $fh "ABCDEFG\n" x 40960;

    unless (close($fh)) {
      die("Can't write $test_file: $!");
    }

  } else {
    die("Can't open $test_file: $!");
  }

  my $config = {
    PidFile => $setup->{pid_file},
    ScoreboardFile => $setup->{scoreboard_file},
    SystemLog => $setup->{log_file},
    TraceLog => $setup->{log_file},
    Trace => 'DEFAULT:10 data:20',

    AuthUserFile => $setup->{auth_user_file},
    AuthGroupFile => $setup->{auth_group_file},
    AuthOrder => 'mod_auth_file.c',

    TimeoutLinger => 5,
    UseSendfile => 'off',

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
      my $client = ProFTPD::TestSuite::FTP->new('127.0.0.1', $port);
      $client->login($setup->{user}, $setup->{passwd});
      $client->type('binary');

      my $conn = $client->retr_raw($test_file);
      unless ($conn) {
        die("Failed to RETR: " . $client->response_code() . " " .
          $client->response_msg());
      }

      # Read 128 bytes of the file, then abort the download
      my $buf;
      $conn->read($buf, 128, 30);

      # With sendfile enabled, it's quite probable that all of the data will
      # have been transferred already.  Thus we expect to see two success
      # responses here.
      eval { $client->quote('ABOR') };
      unless ($@) {
        die("ABOR succeeded unexpectedly");
      }

      # We expect 2 responses here: first, a 426 for the aborted data transfer,
      # followed by 226 for the successful ABOR command.  Order matters.
      my $resp_msgs = $client->response_msgs();

      my $resp_nmsgs = scalar(@$resp_msgs);
      $self->assert($resp_nmsgs == 2,
        test_msg("Expected 2 responses, got $resp_nmsgs"));

      my $expected = 'Transfer aborted. Data connection closed.';
      $self->assert($expected eq $resp_msgs->[0],
        test_msg("Expected response message '$expected', got '$resp_msgs->[0]'"));

      $expected = 'Abort successful';
      $self->assert($expected eq $resp_msgs->[1],
        test_msg("Expected response message '$expected', got '$resp_msgs->[1]'"));

      # Make sure the control connection did not close because of the abort.
      my ($resp_code, $resp_msg) = $client->quit();

      $expected = 221;
      $self->assert($expected == $resp_code,
        test_msg("Expected response code $expected, got $resp_code"));

      $expected = 'Goodbye.';
      $self->assert($expected eq $resp_msg,
        test_msg("Expected response message '$expected', got '$resp_msg'"));
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

sub abor_only_stor_ascii {
  my $self = shift;
  my $tmpdir = $self->{tmpdir};
  my $setup = test_setup($tmpdir, 'abor');

  my $test_file = File::Spec->rel2abs("$tmpdir/foo.txt");

  my $config = {
    PidFile => $setup->{pid_file},
    ScoreboardFile => $setup->{scoreboard_file},
    SystemLog => $setup->{log_file},
    TraceLog => $setup->{log_file},
    Trace => 'DEFAULT:10',

    AuthUserFile => $setup->{auth_user_file},
    AuthGroupFile => $setup->{auth_group_file},
    AuthOrder => 'mod_auth_file.c',

    TimeoutLinger => 5,

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
      my $client = ProFTPD::TestSuite::FTP->new('127.0.0.1', $port);
      $client->login($setup->{user}, $setup->{passwd});
      $client->pasv();
      $client->type('ascii');

      my $conn = $client->stor_raw($test_file);
      unless ($conn) {
        die("Failed to STOR: " . $client->response_code() . " " .
          $client->response_msg());
      }

      # Write data to the file, then abort the upload
      my $buf = "A\r\nB\r\nC\r\nD\r\n";
      $conn->write($buf, length($buf));

      eval { $client->quote('ABOR') };
      unless ($@) {
        die("ABOR succeeded unexpectedly");
      }

      # We expect 2 responses here: first, a 426 for the aborted data transfer,
      # followed by 226 for the successful ABOR command.  Order matters.
      my $resp_msgs = $client->response_msgs();

      my $resp_nmsgs = scalar(@$resp_msgs);
      $self->assert($resp_nmsgs == 2,
        test_msg("Expected 2 responses, got $resp_nmsgs"));

      my $expected = 'Transfer aborted. Data connection closed.';
      $self->assert($expected eq $resp_msgs->[0],
        test_msg("Expected response message '$expected', got '$resp_msgs->[0]'"));

      $expected = 'Abort successful';
      $self->assert($expected eq $resp_msgs->[1],
        test_msg("Expected response message '$expected', got '$resp_msgs->[1]'"));

      # Make sure the control connection did not close because of the abort.
      my ($resp_code, $resp_msg) = $client->quit();

      $expected = 221;
      $self->assert($expected == $resp_code,
        test_msg("Expected response message $expected, got $resp_code"));

      $expected = 'Goodbye.';
      $self->assert($expected eq $resp_msg,
        test_msg("Expected response message '$expected', got '$resp_msg'"));
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

sub abor_only_stor_binary {
  my $self = shift;
  my $tmpdir = $self->{tmpdir};
  my $setup = test_setup($tmpdir, 'abor');

  my $test_file = File::Spec->rel2abs("$tmpdir/foo.txt");

  my $config = {
    PidFile => $setup->{pid_file},
    ScoreboardFile => $setup->{scoreboard_file},
    SystemLog => $setup->{log_file},
    TraceLog => $setup->{log_file},
    Trace => 'DEFAULT:10',

    AuthUserFile => $setup->{auth_user_file},
    AuthGroupFile => $setup->{auth_group_file},
    AuthOrder => 'mod_auth_file.c',

    TimeoutLinger => 5,

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
      my $client = ProFTPD::TestSuite::FTP->new('127.0.0.1', $port);
      $client->login($setup->{user}, $setup->{passwd});
      $client->pasv();
      $client->type('binary');

      my $conn = $client->stor_raw($test_file);
      unless ($conn) {
        die("Failed to STOR: " . $client->response_code() . " " .
          $client->response_msg());
      }

      # Write data to the file, then abort the upload
      my $buf = "A\r\nB\r\nC\r\nD\r\n";
      $conn->write($buf, length($buf));

      eval { $client->quote('ABOR') };
      unless ($@) {
        die("ABOR succeeded unexpectedly");
      }

      # We expect 2 responses here: first, a 426 for the aborted data transfer,
      # followed by 226 for the successful ABOR command.  Order matters.
      my $resp_msgs = $client->response_msgs();

      my $resp_nmsgs = scalar(@$resp_msgs);
      $self->assert($resp_nmsgs == 2,
        test_msg("Expected 2 responses, got $resp_nmsgs"));

      my $expected = 'Transfer aborted. Data connection closed.';
      $self->assert($expected eq $resp_msgs->[0],
        test_msg("Expected response message '$expected', got '$resp_msgs->[0]'"));

      $expected = 'Abort successful';
      $self->assert($expected eq $resp_msgs->[1],
        test_msg("Expected response message '$expected', got '$resp_msgs->[1]'"));

      # Make sure the control connection did not close because of the abort.
      my ($resp_code, $resp_msg) = $client->quit();

      $expected = 221;
      $self->assert($expected == $resp_code,
        test_msg("Expected response message $expected, got $resp_code"));

      $expected = 'Goodbye.';
      $self->assert($expected eq $resp_msg,
        test_msg("Expected response message '$expected', got '$resp_msg'"));
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

sub abor_only_list {
  my $self = shift;
  my $tmpdir = $self->{tmpdir};
  my $setup = test_setup($tmpdir, 'abor');

  my $config = {
    PidFile => $setup->{pid_file},
    ScoreboardFile => $setup->{scoreboard_file},
    SystemLog => $setup->{log_file},
    TraceLog => $setup->{log_file},
    Trace => 'DEFAULT:10',

    AuthUserFile => $setup->{auth_user_file},
    AuthGroupFile => $setup->{auth_group_file},
    AuthOrder => 'mod_auth_file.c',

    TimeoutLinger => 5,

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
      my $client = ProFTPD::TestSuite::FTP->new('127.0.0.1', $port);
      $client->login($setup->{user}, $setup->{passwd});

      # Use a recursive listing, to generate more data, such that it will
      # not all fit in the transfer buffer, so we can interrupt the buffer.
      # A too-small request will fit in the buffer, and be fulfilled, before
      # our abort is read.
      my $conn = $client->list_raw('-R /');
      unless ($conn) {
        die("Failed to LIST: " . $client->response_code() . " " .
          $client->response_msg());
      }

      # Read 128 bytes of data, then abort the download
      my $buf;
      $conn->read($buf, 128, 30);

      eval { $client->quote('ABOR') };
      unless ($@) {
        die("ABOR succeeded unexpectedly");
      }

      # We expect 2 responses here: first, a 426 for the aborted data transfer,
      # followed by 226 for the successful ABOR command.  Order matters.
      my $resp_msgs = $client->response_msgs();

      my $resp_nmsgs = scalar(@$resp_msgs);
      $self->assert($resp_nmsgs == 2,
        test_msg("Expected 2 responses, got $resp_nmsgs"));

      my $expected = 'Transfer aborted. Data connection closed.';
      $self->assert($expected eq $resp_msgs->[0],
        test_msg("Expected response message '$expected', got '$resp_msgs->[0]'"));

      $expected = 'Abort successful';
      $self->assert($expected eq $resp_msgs->[1],
        test_msg("Expected response message '$expected', got '$resp_msgs->[1]'"));

      # Make sure the control connection did not close because of the abort.
      my ($resp_code, $resp_msg) = $client->quit();

      $expected = 221;
      $self->assert($expected == $resp_code,
        test_msg("Expected response code $expected, got $resp_code"));

      $expected = 'Goodbye.';
      $self->assert($expected eq $resp_msg,
        test_msg("Expected response message '$expected', got '$resp_msg'"));
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

sub abor_only_mlsd {
  my $self = shift;
  my $tmpdir = $self->{tmpdir};
  my $setup = test_setup($tmpdir, 'abor');

  # Generate enough files in the directory to lead to a large enough response,
  # such that the response does not all fit in the initial transfer buffer.
  for (my $i = 0; $i < 5000; $i++) {
    my $path = File::Spec->rel2abs("$tmpdir/$i.dat");

    if (open(my $fh, "> $path")) {
      close($fh);

    } else {
      die("Can't open $path: $!");
    }
  }

  my $config = {
    PidFile => $setup->{pid_file},
    ScoreboardFile => $setup->{scoreboard_file},
    SystemLog => $setup->{log_file},
    TraceLog => $setup->{log_file},
    Trace => 'DEFAULT:10',

    AuthUserFile => $setup->{auth_user_file},
    AuthGroupFile => $setup->{auth_group_file},
    AuthOrder => 'mod_auth_file.c',

    TimeoutLinger => 5,

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
      my $client = ProFTPD::TestSuite::FTP->new('127.0.0.1', $port);
      $client->login($setup->{user}, $setup->{passwd});

      my $conn = $client->mlsd_raw($tmpdir);
      unless ($conn) {
        die("Failed to MLSD: " . $client->response_code() . " " .
          $client->response_msg());
      }

      # Read 128 bytes of data, then abort the download
      my $buf;
      $conn->read($buf, 128, 30);

      eval { $client->quote('ABOR') };
      unless ($@) {
        die("ABOR succeeded unexpectedly");
      }

      # We expect 2 responses here: first, a 426 for the aborted data transfer,
      # followed by 226 for the successful ABOR command.  Order matters.
      my $resp_msgs = $client->response_msgs();

      my $resp_nmsgs = scalar(@$resp_msgs);
      $self->assert($resp_nmsgs == 2,
        test_msg("Expected 2 responses, got $resp_nmsgs"));

      my $expected = 'Transfer aborted. Data connection closed.';
      $self->assert($expected eq $resp_msgs->[0],
        test_msg("Expected response message '$expected', got '$resp_msgs->[0]'"));

      $expected = 'Abort successful';
      $self->assert($expected eq $resp_msgs->[1],
        test_msg("Expected response message '$expected', got '$resp_msgs->[1]'"));

      # Make sure the control connection did not close because of the abort.
      my ($resp_code, $resp_msg) = $client->quit();

      $expected = 221;
      $self->assert($expected == $resp_code,
        test_msg("Expected response code $expected, got $resp_code"));

      $expected = 'Goodbye.';
      $self->assert($expected eq $resp_msg,
        test_msg("Expected response message '$expected', got '$resp_msg'"));
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

sub abor_only_no_xfer {
  my $self = shift;
  my $tmpdir = $self->{tmpdir};
  my $setup = test_setup($tmpdir, 'abor');

  my $test_file = File::Spec->rel2abs($setup->{config_file});

  my $config = {
    PidFile => $setup->{pid_file},
    ScoreboardFile => $setup->{scoreboard_file},
    SystemLog => $setup->{log_file},
    TraceLog => $setup->{log_file},
    Trace => 'DEFAULT:10',

    AuthUserFile => $setup->{auth_user_file},
    AuthGroupFile => $setup->{auth_group_file},
    AuthOrder => 'mod_auth_file.c',

    TimeoutLinger => 5,

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
      my $client = ProFTPD::TestSuite::FTP->new('127.0.0.1', $port);
      $client->login($setup->{user}, $setup->{passwd});

      $client->quote('ABOR');

      my $resp_code = $client->response_code();
      my $resp_msg = $client->response_msg();
      $self->assert_transfer_ok($resp_code, $resp_msg, 1);

      # Make sure the control connection did not close because of the abort.
      ($resp_code, $resp_msg) = $client->quit();

      my $expected = 221;
      $self->assert($expected == $resp_code,
        test_msg("Expected response code $expected, got $resp_code"));

      $expected = 'Goodbye.';
      $self->assert($expected eq $resp_msg,
        test_msg("Expected response message '$expected', got '$resp_msg'"));
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

sub data_eof_retr_ascii {
  my $self = shift;
  my $tmpdir = $self->{tmpdir};
  my $setup = test_setup($tmpdir, 'abor');

  my $test_file = File::Spec->rel2abs("$tmpdir/largefile.txt");
  if (open(my $fh, "> $test_file")) {
    print $fh "ABCDEFG\n" x 40960;

    unless (close($fh)) {
      die("Can't write $test_file: $!");
    }

  } else {
    die("Can't open $test_file: $!");
  }

  my $config = {
    PidFile => $setup->{pid_file},
    ScoreboardFile => $setup->{scoreboard_file},
    SystemLog => $setup->{log_file},
    TraceLog => $setup->{log_file},
    Trace => 'DEFAULT:10 data:20',

    AuthUserFile => $setup->{auth_user_file},
    AuthGroupFile => $setup->{auth_group_file},
    AuthOrder => 'mod_auth_file.c',

    TimeoutLinger => 5,

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
      my $client = ProFTPD::TestSuite::FTP->new('127.0.0.1', $port);
      $client->login($setup->{user}, $setup->{passwd});
      $client->pasv();
      $client->type('ascii');

      my $conn = $client->retr_raw($test_file);
      unless ($conn) {
        die("Failed to RETR: " . $client->response_code() . " " .
          $client->response_msg());
      }

      # Read 128 bytes of the file, then abort the download
      my $buf;
      $conn->read($buf, 128, 30);
      $conn->_close();

      # There is a potential race here, between data EOF and our next command.
      # We thus sleep here, to let the data EOF win the race.  With that,
      # we should now receive our end-of-transfer response.
      sleep(1);

      my ($resp_code, $resp_msg) = $client->read_response();

      my $expected = 426;
      $self->assert($expected == $resp_code,
        test_msg("Expected response code $expected, got $resp_code"));

      $expected = 'Transfer aborted. Data connection closed';
      $self->assert($expected eq $resp_msg,
        test_msg("Expected response message '$expected', got '$resp_msg'"));

      # Make sure the control connection did not close because of the abort.
      ($resp_code, $resp_msg) = $client->quit();

      $expected = 221;
      $self->assert($expected == $resp_code,
        test_msg("Expected response code $expected, got $resp_code"));

      $expected = 'Goodbye.';
      $self->assert($expected eq $resp_msg,
        test_msg("Expected response message '$expected', got '$resp_msg'"));
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

sub data_eof_retr_binary_with_sendfile {
  my $self = shift;
  my $tmpdir = $self->{tmpdir};
  my $setup = test_setup($tmpdir, 'abor');

  my $test_file = File::Spec->rel2abs("$tmpdir/largefile.txt");
  if (open(my $fh, "> $test_file")) {
    print $fh "ABCDEFG\n" x 40960;

    unless (close($fh)) {
      die("Can't write $test_file: $!");
    }

  } else {
    die("Can't open $test_file: $!");
  }

  my $config = {
    PidFile => $setup->{pid_file},
    ScoreboardFile => $setup->{scoreboard_file},
    SystemLog => $setup->{log_file},
    TraceLog => $setup->{log_file},
    Trace => 'DEFAULT:10 data:20',

    AuthUserFile => $setup->{auth_user_file},
    AuthGroupFile => $setup->{auth_group_file},
    AuthOrder => 'mod_auth_file.c',

    TimeoutLinger => 5,

    IfModules => {
      'mod_delay.c' => {
        DelayEngine => 'off',
      },
    },
  };

  if (feature_have_feature_enabled('sendfile')) {
    $config->{UseSendfile} = 'on';
  }

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
      my $client = ProFTPD::TestSuite::FTP->new('127.0.0.1', $port);
      $client->login($setup->{user}, $setup->{passwd});
      $client->pasv();
      $client->type('binary');

      my $conn = $client->retr_raw($test_file);
      unless ($conn) {
        die("Failed to RETR: " . $client->response_code() . " " .
          $client->response_msg());
      }

      # Read 128 bytes of the file, then abort the download
      my $buf;
      $conn->read($buf, 128, 30);
      $conn->_close();

      # With sendfile support, it's probably that the entire file was already
      # sent.  Thus we need to expect the 226 transfer response here.
      my ($resp_code, $resp_msg) = $client->read_response();
      $self->assert_transfer_ok($resp_code, $resp_msg);

      # Make sure the control connection did not close because of the abort.
      ($resp_code, $resp_msg) = $client->quit();

      my $expected = 221;
      $self->assert($expected == $resp_code,
        test_msg("Expected response code $expected, got $resp_code"));

      $expected = 'Goodbye.';
      $self->assert($expected eq $resp_msg,
        test_msg("Expected response message '$expected', got '$resp_msg'"));
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

sub data_eof_retr_binary_without_sendfile {
  my $self = shift;
  my $tmpdir = $self->{tmpdir};
  my $setup = test_setup($tmpdir, 'abor');

  my $test_file = File::Spec->rel2abs("$tmpdir/largefile.txt");
  if (open(my $fh, "> $test_file")) {
    print $fh "ABCDEFG\n" x 40960;

    unless (close($fh)) {
      die("Can't write $test_file: $!");
    }

  } else {
    die("Can't open $test_file: $!");
  }

  my $config = {
    PidFile => $setup->{pid_file},
    ScoreboardFile => $setup->{scoreboard_file},
    SystemLog => $setup->{log_file},
    TraceLog => $setup->{log_file},
    Trace => 'DEFAULT:10 data:20',

    AuthUserFile => $setup->{auth_user_file},
    AuthGroupFile => $setup->{auth_group_file},
    AuthOrder => 'mod_auth_file.c',

    TimeoutLinger => 5,
    UseSendfile => 'off',

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
      my $client = ProFTPD::TestSuite::FTP->new('127.0.0.1', $port);
      $client->login($setup->{user}, $setup->{passwd});
      $client->pasv();
      $client->type('binary');

      my $conn = $client->retr_raw($test_file);
      unless ($conn) {
        die("Failed to RETR: " . $client->response_code() . " " .
          $client->response_msg());
      }

      # Read 128 bytes of the file, then abort the download
      my $buf;
      $conn->read($buf, 128, 30);
      $conn->_close();

      # There is a potential race here, between data EOF and our next command.
      # We thus sleep here, to let the data EOF win the race.  With that,
      # we should now receive our end-of-transfer response.
      sleep(1);

      my ($resp_code, $resp_msg) = $client->read_response();

      my $expected = 426;
      $self->assert($expected == $resp_code,
        test_msg("Expected response code $expected, got $resp_code"));

      $expected = 'Transfer aborted. Data connection closed';
      $self->assert($expected eq $resp_msg,
        test_msg("Expected response message '$expected', got '$resp_msg'"));

      # Make sure the control connection did not close because of the abort.
      ($resp_code, $resp_msg) = $client->quit();

      $expected = 221;
      $self->assert($expected == $resp_code,
        test_msg("Expected response code $expected, got $resp_code"));

      $expected = 'Goodbye.';
      $self->assert($expected eq $resp_msg,
        test_msg("Expected response message '$expected', got '$resp_msg'"));
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

sub data_eof_stor_ascii {
  my $self = shift;
  my $tmpdir = $self->{tmpdir};
  my $setup = test_setup($tmpdir, 'abor');

  my $test_file = File::Spec->rel2abs("$tmpdir/foo.txt");

  my $config = {
    PidFile => $setup->{pid_file},
    ScoreboardFile => $setup->{scoreboard_file},
    SystemLog => $setup->{log_file},
    TraceLog => $setup->{log_file},
    Trace => 'DEFAULT:10',

    AuthUserFile => $setup->{auth_user_file},
    AuthGroupFile => $setup->{auth_group_file},
    AuthOrder => 'mod_auth_file.c',

    TimeoutLinger => 5,

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
      my $client = ProFTPD::TestSuite::FTP->new('127.0.0.1', $port);
      $client->login($setup->{user}, $setup->{passwd});
      $client->pasv();
      $client->type('ascii');

      my $conn = $client->stor_raw($test_file);
      unless ($conn) {
        die("Failed to STOR: " . $client->response_code() . " " .
          $client->response_msg());
      }

      # Write data to the file, then abort the upload
      my $buf = "A\r\nB\r\nC\r\nD\r\n";
      $conn->write($buf, length($buf));
      $conn->_close();

      # There is a potential race here, between data EOF and our next command.
      # We thus sleep here, to let the data EOF win the race.  With that,
      # we should now receive our end-of-transfer response.
      sleep(1);

      my ($resp_code, $resp_msg) = $client->read_response();

      # Since the server does not know, a priori, how much data we will be
      # sending, it cannot tell when we completed successfully or not.  Thus
      # an EOF on upload is always a "successful transfer".
      $self->assert_transfer_ok($resp_code, $resp_msg);

      # Make sure the control connection did not close because of the abort.
      ($resp_code, $resp_msg) = $client->quit();

      my $expected = 221;
      $self->assert($expected == $resp_code,
        test_msg("Expected response message $expected, got $resp_code"));

      $expected = 'Goodbye.';
      $self->assert($expected eq $resp_msg,
        test_msg("Expected response message '$expected', got '$resp_msg'"));
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

sub data_eof_stor_binary {
  my $self = shift;
  my $tmpdir = $self->{tmpdir};
  my $setup = test_setup($tmpdir, 'abor');

  my $test_file = File::Spec->rel2abs("$tmpdir/foo.txt");

  my $config = {
    PidFile => $setup->{pid_file},
    ScoreboardFile => $setup->{scoreboard_file},
    SystemLog => $setup->{log_file},
    TraceLog => $setup->{log_file},
    Trace => 'DEFAULT:10',

    AuthUserFile => $setup->{auth_user_file},
    AuthGroupFile => $setup->{auth_group_file},
    AuthOrder => 'mod_auth_file.c',

    TimeoutLinger => 5,

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
      my $client = ProFTPD::TestSuite::FTP->new('127.0.0.1', $port);
      $client->login($setup->{user}, $setup->{passwd});
      $client->pasv();
      $client->type('binary');

      my $conn = $client->stor_raw($test_file);
      unless ($conn) {
        die("Failed to STOR: " . $client->response_code() . " " .
          $client->response_msg());
      }

      # Write data to the file, then abort the upload
      my $buf = "A\r\nB\r\nC\r\nD\r\n";
      $conn->write($buf, length($buf));
      $conn->_close();

      # There is a potential race here, between data EOF and our next command.
      # We thus sleep here, to let the data EOF win the race.  With that,
      # we should now receive our end-of-transfer response.
      sleep(1);

      my ($resp_code, $resp_msg) = $client->read_response();

      # Since the server does not know, a priori, how much data we will be
      # sending, it cannot tell when we completed successfully or not.  Thus
      # an EOF on upload is always a "successful transfer".
      $self->assert_transfer_ok($resp_code, $resp_msg);

      # Make sure the control connection did not close because of the abort.
      ($resp_code, $resp_msg) = $client->quit();

      my $expected = 221;
      $self->assert($expected == $resp_code,
        test_msg("Expected response message $expected, got $resp_code"));

      $expected = 'Goodbye.';
      $self->assert($expected eq $resp_msg,
        test_msg("Expected response message '$expected', got '$resp_msg'"));
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

sub data_eof_list {
  my $self = shift;
  my $tmpdir = $self->{tmpdir};
  my $setup = test_setup($tmpdir, 'abor');

  my $config = {
    PidFile => $setup->{pid_file},
    ScoreboardFile => $setup->{scoreboard_file},
    SystemLog => $setup->{log_file},
    TraceLog => $setup->{log_file},
    Trace => 'DEFAULT:10',

    AuthUserFile => $setup->{auth_user_file},
    AuthGroupFile => $setup->{auth_group_file},
    AuthOrder => 'mod_auth_file.c',

    TimeoutLinger => 5,

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
      my $client = ProFTPD::TestSuite::FTP->new('127.0.0.1', $port);
      $client->login($setup->{user}, $setup->{passwd});

      # Use a recursive listing, to generate more data, such that it will
      # not all fit in the transfer buffer, so we can interrupt the buffer.
      # A too-small request will fit in the buffer, and be fulfilled, before
      # our abort is read.
      my $conn = $client->list_raw('-R /');
      unless ($conn) {
        die("Failed to LIST: " . $client->response_code() . " " .
          $client->response_msg());
      }

      # Read 128 bytes of data, then abort the download
      my $buf;
      $conn->read($buf, 128, 30);
      $conn->_close();

      # There is a potential race here, between data EOF and our next command.
      # We thus sleep here, to let the data EOF win the race.  With that,
      # we should now receive our end-of-transfer response.
      sleep(1);

      my ($resp_code, $resp_msg) = $client->read_response();

      my $expected = 426;
      $self->assert($expected == $resp_code,
        test_msg("Expected response code $expected, got $resp_code"));

      $expected = 'Transfer aborted. Data connection closed';
      $self->assert($expected eq $resp_msg,
        test_msg("Expected response message '$expected', got '$resp_msg'"));

      # Make sure the control connection did not close because of the abort.
      ($resp_code, $resp_msg) = $client->quit();

      $expected = 221;
      $self->assert($expected == $resp_code,
        test_msg("Expected response code $expected, got $resp_code"));

      $expected = 'Goodbye.';
      $self->assert($expected eq $resp_msg,
        test_msg("Expected response message '$expected', got '$resp_msg'"));
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

sub data_eof_mlsd {
  my $self = shift;
  my $tmpdir = $self->{tmpdir};
  my $setup = test_setup($tmpdir, 'abor');

  # Generate enough files in the directory to lead to a large enough response,
  # such that the response does not all fit in the initial transfer buffer.
  for (my $i = 0; $i < 5000; $i++) {
    my $path = File::Spec->rel2abs("$tmpdir/$i.dat");

    if (open(my $fh, "> $path")) {
      close($fh);

    } else {
      die("Can't open $path: $!");
    }
  }

  my $config = {
    PidFile => $setup->{pid_file},
    ScoreboardFile => $setup->{scoreboard_file},
    SystemLog => $setup->{log_file},
    TraceLog => $setup->{log_file},
    Trace => 'DEFAULT:10 lock:0 scoreboard:0',

    AuthUserFile => $setup->{auth_user_file},
    AuthGroupFile => $setup->{auth_group_file},
    AuthOrder => 'mod_auth_file.c',

    TimeoutLinger => 5,

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
      my $client = ProFTPD::TestSuite::FTP->new('127.0.0.1', $port);
      $client->login($setup->{user}, $setup->{passwd});

      my $conn = $client->mlsd_raw($tmpdir);
      unless ($conn) {
        die("Failed to MLSD: " . $client->response_code() . " " .
          $client->response_msg());
      }

      # Read 8 bytes of data, then abort the download
      my $buf;
      $conn->read($buf, 8, 30);
      $conn->_close();

      # There is a potential race here, between data EOF and our next command.
      # We thus sleep here, to let the data EOF win the race.  With that,
      # we should now receive our end-of-transfer response.
      sleep(1);

      my ($resp_code, $resp_msg) = $client->read_response();

      my $expected = 426;
      $self->assert($expected == $resp_code,
        test_msg("Expected response code $expected, got $resp_code"));

      $expected = 'Transfer aborted. Data connection closed';
      $self->assert($expected eq $resp_msg,
        test_msg("Expected response message '$expected', got '$resp_msg'"));

      # Make sure the control connection did not close because of the abort.
      ($resp_code, $resp_msg) = $client->quit();

      $expected = 221;
      $self->assert($expected == $resp_code,
        test_msg("Expected response code $expected, got $resp_code"));

      $expected = 'Goodbye.';
      $self->assert($expected eq $resp_msg,
        test_msg("Expected response message '$expected', got '$resp_msg'"));
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

sub data_eof_before_abor_retr_ascii {
  my $self = shift;
  my $tmpdir = $self->{tmpdir};
  my $setup = test_setup($tmpdir, 'abor');

  my $test_file = File::Spec->rel2abs("$tmpdir/largefile.txt");
  if (open(my $fh, "> $test_file")) {
    print $fh "ABCDEFG\n" x 40960;

    unless (close($fh)) {
      die("Can't write $test_file: $!");
    }

  } else {
    die("Can't open $test_file: $!");
  }

  my $config = {
    PidFile => $setup->{pid_file},
    ScoreboardFile => $setup->{scoreboard_file},
    SystemLog => $setup->{log_file},
    TraceLog => $setup->{log_file},
    Trace => 'DEFAULT:10 data:20',

    AuthUserFile => $setup->{auth_user_file},
    AuthGroupFile => $setup->{auth_group_file},
    AuthOrder => 'mod_auth_file.c',

    TimeoutLinger => 5,

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
      my $client = ProFTPD::TestSuite::FTP->new('127.0.0.1', $port);
      $client->login($setup->{user}, $setup->{passwd});
      $client->pasv();
      $client->type('ascii');

      my $conn = $client->retr_raw($test_file);
      unless ($conn) {
        die("Failed to RETR: " . $client->response_code() . " " .
          $client->response_msg());
      }

      # Read 128 bytes of the file, then abort the download
      my $buf;
      $conn->read($buf, 128, 30);
      $conn->_close();

      # There is a potential race here, between data EOF and our next command.
      # We thus sleep here, to let the data EOF win the race.  With that,
      # we should now receive our end-of-transfer response.
      sleep(1);

      my ($resp_code, $resp_msg) = $client->read_response();

      my $expected = 426;
      $self->assert($expected == $resp_code,
        test_msg("Expected response code $expected, got $resp_code"));

      $expected = 'Transfer aborted. Data connection closed';
      $self->assert($expected eq $resp_msg,
        test_msg("Expected response message '$expected', got '$resp_msg'"));

      # We use the Net::FTP::abort() method here, so that we use, in this
      # order: data EOF, TCP OOB, ABOR.
      $client->abort();
      $resp_code = $client->response_code();
      $resp_msg = $client->response_msg();

      $expected = 226;
      $self->assert($expected == $resp_code,
        test_msg("Expected response code $expected, got $resp_code"));

      $expected = 'Abort successful';
      $self->assert($expected eq $resp_msg,
        test_msg("Expected response message '$expected', got '$resp_msg'"));

      # Make sure the control connection did not close because of the abort.
      ($resp_code, $resp_msg) = $client->quit();

      $expected = 221;
      $self->assert($expected == $resp_code,
        test_msg("Expected response code $expected, got $resp_code"));

      $expected = 'Goodbye.';
      $self->assert($expected eq $resp_msg,
        test_msg("Expected response message '$expected', got '$resp_msg'"));
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

sub data_eof_before_abor_retr_binary_with_sendfile {
  my $self = shift;
  my $tmpdir = $self->{tmpdir};
  my $setup = test_setup($tmpdir, 'abor');

  my $test_file = File::Spec->rel2abs("$tmpdir/largefile.txt");
  if (open(my $fh, "> $test_file")) {
    print $fh "ABCDEFG\n" x 40960;

    unless (close($fh)) {
      die("Can't write $test_file: $!");
    }

  } else {
    die("Can't open $test_file: $!");
  }

  my $config = {
    PidFile => $setup->{pid_file},
    ScoreboardFile => $setup->{scoreboard_file},
    SystemLog => $setup->{log_file},
    TraceLog => $setup->{log_file},
    Trace => 'DEFAULT:10 data:20',

    AuthUserFile => $setup->{auth_user_file},
    AuthGroupFile => $setup->{auth_group_file},
    AuthOrder => 'mod_auth_file.c',

    TimeoutLinger => 5,

    IfModules => {
      'mod_delay.c' => {
        DelayEngine => 'off',
      },
    },
  };

  if (feature_have_feature_enabled('sendfile')) {
    $config->{UseSendfile} = 'on';
  }

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
      my $client = ProFTPD::TestSuite::FTP->new('127.0.0.1', $port);
      $client->login($setup->{user}, $setup->{passwd});
      $client->pasv();
      $client->type('binary');

      my $conn = $client->retr_raw($test_file);
      unless ($conn) {
        die("Failed to RETR: " . $client->response_code() . " " .
          $client->response_msg());
      }

      # Read 128 bytes of the file, then abort the download
      my $buf;
      $conn->read($buf, 128, 30);
      $conn->_close();

      # There is a potential race here, between data EOF and our next command.
      # We thus sleep here, to let the data EOF win the race.  With that,
      # we should now receive our end-of-transfer response.
      sleep(1);

      my ($resp_code, $resp_msg) = $client->read_response();

      # We expect 226 here because sendfile will probably have successfully
      # written all of its bytes to the network.
      my $expected = 226;
      $self->assert($expected == $resp_code,
        test_msg("Expected response code $expected, got $resp_code"));

      $expected = 'Transfer complete';
      $self->assert($expected eq $resp_msg,
        test_msg("Expected response message '$expected', got '$resp_msg'"));

      # We use the Net::FTP::abort() method here, so that we use, in this
      # order: data EOF, TCP OOB, ABOR.
      $client->abort();
      $resp_code = $client->response_code();
      $resp_msg = $client->response_msg();

      $expected = 226;
      $self->assert($expected == $resp_code,
        test_msg("Expected response code $expected, got $resp_code"));

      $expected = 'Abort successful';
      $self->assert($expected eq $resp_msg,
        test_msg("Expected response message '$expected', got '$resp_msg'"));

      # Make sure the control connection did not close because of the abort.
      ($resp_code, $resp_msg) = $client->quit();

      $expected = 221;
      $self->assert($expected == $resp_code,
        test_msg("Expected response code $expected, got $resp_code"));

      $expected = 'Goodbye.';
      $self->assert($expected eq $resp_msg,
        test_msg("Expected response message '$expected', got '$resp_msg'"));
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

sub data_eof_before_abor_retr_binary_without_sendfile {
  my $self = shift;
  my $tmpdir = $self->{tmpdir};
  my $setup = test_setup($tmpdir, 'abor');

  my $test_file = File::Spec->rel2abs("$tmpdir/largefile.txt");
  if (open(my $fh, "> $test_file")) {
    print $fh "ABCDEFG\n" x 40960;

    unless (close($fh)) {
      die("Can't write $test_file: $!");
    }

  } else {
    die("Can't open $test_file: $!");
  }

  my $config = {
    PidFile => $setup->{pid_file},
    ScoreboardFile => $setup->{scoreboard_file},
    SystemLog => $setup->{log_file},
    TraceLog => $setup->{log_file},
    Trace => 'DEFAULT:10 data:20',

    AuthUserFile => $setup->{auth_user_file},
    AuthGroupFile => $setup->{auth_group_file},
    AuthOrder => 'mod_auth_file.c',

    TimeoutLinger => 5,
    UseSendfile => 'off',

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
      my $client = ProFTPD::TestSuite::FTP->new('127.0.0.1', $port);
      $client->login($setup->{user}, $setup->{passwd});
      $client->pasv();
      $client->type('binary');

      my $conn = $client->retr_raw($test_file);
      unless ($conn) {
        die("Failed to RETR: " . $client->response_code() . " " .
          $client->response_msg());
      }

      # Read 128 bytes of the file, then abort the download
      my $buf;
      $conn->read($buf, 128, 30);
      $conn->_close();

      # There is a potential race here, between data EOF and our next command.
      # We thus sleep here, to let the data EOF win the race.  With that,
      # we should now receive our end-of-transfer response.
      sleep(1);

      my ($resp_code, $resp_msg) = $client->read_response();

      my $expected = 426;
      $self->assert($expected == $resp_code,
        test_msg("Expected response code $expected, got $resp_code"));

      $expected = 'Transfer aborted. Data connection closed';
      $self->assert($expected eq $resp_msg,
        test_msg("Expected response message '$expected', got '$resp_msg'"));

      # We use the Net::FTP::abort() method here, so that we use, in this
      # order: data EOF, TCP OOB, ABOR.
      $client->abort();
      $resp_code = $client->response_code();
      $resp_msg = $client->response_msg();

      $expected = 226;
      $self->assert($expected == $resp_code,
        test_msg("Expected response code $expected, got $resp_code"));

      $expected = 'Abort successful';
      $self->assert($expected eq $resp_msg,
        test_msg("Expected response message '$expected', got '$resp_msg'"));

      # Make sure the control connection did not close because of the abort.
      ($resp_code, $resp_msg) = $client->quit();

      $expected = 221;
      $self->assert($expected == $resp_code,
        test_msg("Expected response code $expected, got $resp_code"));

      $expected = 'Goodbye.';
      $self->assert($expected eq $resp_msg,
        test_msg("Expected response message '$expected', got '$resp_msg'"));
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

sub data_eof_before_abor_stor_ascii {
  my $self = shift;
  my $tmpdir = $self->{tmpdir};
  my $setup = test_setup($tmpdir, 'abor');

  my $test_file = File::Spec->rel2abs("$tmpdir/foo.txt");

  my $config = {
    PidFile => $setup->{pid_file},
    ScoreboardFile => $setup->{scoreboard_file},
    SystemLog => $setup->{log_file},
    TraceLog => $setup->{log_file},
    Trace => 'DEFAULT:10',

    AuthUserFile => $setup->{auth_user_file},
    AuthGroupFile => $setup->{auth_group_file},
    AuthOrder => 'mod_auth_file.c',

    TimeoutLinger => 5,

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
      my $client = ProFTPD::TestSuite::FTP->new('127.0.0.1', $port);
      $client->login($setup->{user}, $setup->{passwd});
      $client->pasv();
      $client->type('ascii');

      my $conn = $client->stor_raw($test_file);
      unless ($conn) {
        die("Failed to STOR: " . $client->response_code() . " " .
          $client->response_msg());
      }

      # Write data to the file, then abort the upload
      my $buf = "A\r\nB\r\nC\r\nD\r\n";
      $conn->write($buf, length($buf));
      $conn->_close();

      # There is a potential race here, between data EOF and our next command.
      # We thus sleep here, to let the data EOF win the race.  With that,
      # we should now receive our end-of-transfer response.
      sleep(1);

      my ($resp_code, $resp_msg) = $client->read_response();

      # Since the server does not know, a priori, how much data we will be
      # sending, it cannot tell when we completed successfully or not.  Thus
      # an EOF on upload is always a "successful transfer".
      $self->assert_transfer_ok($resp_code, $resp_msg);

      # We use the Net::FTP::abort() method here, so that we use, in this
      # order: data EOF, TCP OOB, ABOR.
      $client->abort();
      $resp_code = $client->response_code();
      $resp_msg = $client->response_msg();

      my $expected = 226;
      $self->assert($expected == $resp_code,
        test_msg("Expected response code $expected, got $resp_code"));

      $expected = 'Abort successful';
      $self->assert($expected eq $resp_msg,
        test_msg("Expected response message '$expected', got '$resp_msg'"));

      # Make sure the control connection did not close because of the abort.
      ($resp_code, $resp_msg) = $client->quit();

      $expected = 221;
      $self->assert($expected == $resp_code,
        test_msg("Expected response message $expected, got $resp_code"));

      $expected = 'Goodbye.';
      $self->assert($expected eq $resp_msg,
        test_msg("Expected response message '$expected', got '$resp_msg'"));
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

sub data_eof_before_abor_stor_binary {
  my $self = shift;
  my $tmpdir = $self->{tmpdir};
  my $setup = test_setup($tmpdir, 'abor');

  my $test_file = File::Spec->rel2abs("$tmpdir/foo.txt");

  my $config = {
    PidFile => $setup->{pid_file},
    ScoreboardFile => $setup->{scoreboard_file},
    SystemLog => $setup->{log_file},
    TraceLog => $setup->{log_file},
    Trace => 'DEFAULT:10',

    AuthUserFile => $setup->{auth_user_file},
    AuthGroupFile => $setup->{auth_group_file},
    AuthOrder => 'mod_auth_file.c',

    TimeoutLinger => 5,

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
      my $client = ProFTPD::TestSuite::FTP->new('127.0.0.1', $port);
      $client->login($setup->{user}, $setup->{passwd});
      $client->pasv();
      $client->type('binary');

      my $conn = $client->stor_raw($test_file);
      unless ($conn) {
        die("Failed to STOR: " . $client->response_code() . " " .
          $client->response_msg());
      }

      # Write data to the file, then abort the upload
      my $buf = "A\r\nB\r\nC\r\nD\r\n";
      $conn->write($buf, length($buf));
      $conn->_close();

      # There is a potential race here, between data EOF and our next command.
      # We thus sleep here, to let the data EOF win the race.  With that,
      # we should now receive our end-of-transfer response.
      sleep(1);

      my ($resp_code, $resp_msg) = $client->read_response();

      # Since the server does not know, a priori, how much data we will be
      # sending, it cannot tell when we completed successfully or not.  Thus
      # an EOF on upload is always a "successful transfer".
      $self->assert_transfer_ok($resp_code, $resp_msg);

      # We use the Net::FTP::abort() method here, so that we use, in this
      # order: data EOF, TCP OOB, ABOR.
      $client->abort();
      $resp_code = $client->response_code();
      $resp_msg = $client->response_msg();

      my $expected = 226;
      $self->assert($expected == $resp_code,
        test_msg("Expected response code $expected, got $resp_code"));

      $expected = 'Abort successful';
      $self->assert($expected eq $resp_msg,
        test_msg("Expected response message '$expected', got '$resp_msg'"));

      # Make sure the control connection did not close because of the abort.
      ($resp_code, $resp_msg) = $client->quit();

      $expected = 221;
      $self->assert($expected == $resp_code,
        test_msg("Expected response message $expected, got $resp_code"));

      $expected = 'Goodbye.';
      $self->assert($expected eq $resp_msg,
        test_msg("Expected response message '$expected', got '$resp_msg'"));
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

sub data_eof_before_abor_list {
  my $self = shift;
  my $tmpdir = $self->{tmpdir};
  my $setup = test_setup($tmpdir, 'abor');

  my $config = {
    PidFile => $setup->{pid_file},
    ScoreboardFile => $setup->{scoreboard_file},
    SystemLog => $setup->{log_file},
    TraceLog => $setup->{log_file},
    Trace => 'DEFAULT:10',

    AuthUserFile => $setup->{auth_user_file},
    AuthGroupFile => $setup->{auth_group_file},
    AuthOrder => 'mod_auth_file.c',

    TimeoutLinger => 5,

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
      my $client = ProFTPD::TestSuite::FTP->new('127.0.0.1', $port);
      $client->login($setup->{user}, $setup->{passwd});

      # Use a recursive listing, to generate more data, such that it will
      # not all fit in the transfer buffer, so we can interrupt the buffer.
      # A too-small request will fit in the buffer, and be fulfilled, before
      # our abort is read.
      my $conn = $client->list_raw('-R /');
      unless ($conn) {
        die("Failed to LIST: " . $client->response_code() . " " .
          $client->response_msg());
      }

      # Read 128 bytes of data, then abort the download
      my $buf;
      $conn->read($buf, 128, 30);
      $conn->_close();

      # There is a potential race here, between data EOF and our next command.
      # We thus sleep here, to let the data EOF win the race.  With that,
      # we should now receive our end-of-transfer response.
      sleep(1);

      my ($resp_code, $resp_msg) = $client->read_response();

      my $expected = 426;
      $self->assert($expected == $resp_code,
        test_msg("Expected response code $expected, got $resp_code"));

      $expected = 'Transfer aborted. Data connection closed';
      $self->assert($expected eq $resp_msg,
        test_msg("Expected response message '$expected', got '$resp_msg'"));

      # We use the Net::FTP::abort() method here, so that we use, in this
      # order: data EOF, TCP OOB, ABOR.
      $client->abort();
      $resp_code = $client->response_code();
      $resp_msg = $client->response_msg();

      $expected = 226;
      $self->assert($expected == $resp_code,
        test_msg("Expected response code $expected, got $resp_code"));

      $expected = 'Abort successful';
      $self->assert($expected eq $resp_msg,
        test_msg("Expected response message '$expected', got '$resp_msg'"));

      # Make sure the control connection did not close because of the abort.
      ($resp_code, $resp_msg) = $client->quit();

      $expected = 221;
      $self->assert($expected == $resp_code,
        test_msg("Expected response code $expected, got $resp_code"));

      $expected = 'Goodbye.';
      $self->assert($expected eq $resp_msg,
        test_msg("Expected response message '$expected', got '$resp_msg'"));
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

sub data_eof_before_abor_mlsd {
  my $self = shift;
  my $tmpdir = $self->{tmpdir};
  my $setup = test_setup($tmpdir, 'abor');

  # Generate enough files in the directory to lead to a large enough response,
  # such that the response does not all fit in the initial transfer buffer.
  for (my $i = 0; $i < 5000; $i++) {
    my $path = File::Spec->rel2abs("$tmpdir/$i.dat");

    if (open(my $fh, "> $path")) {
      close($fh);

    } else {
      die("Can't open $path: $!");
    }
  }

  my $config = {
    PidFile => $setup->{pid_file},
    ScoreboardFile => $setup->{scoreboard_file},
    SystemLog => $setup->{log_file},
    TraceLog => $setup->{log_file},
    Trace => 'DEFAULT:10 lock:0 scoreboard:0',

    AuthUserFile => $setup->{auth_user_file},
    AuthGroupFile => $setup->{auth_group_file},
    AuthOrder => 'mod_auth_file.c',

    TimeoutLinger => 5,

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
      my $client = ProFTPD::TestSuite::FTP->new('127.0.0.1', $port);
      $client->login($setup->{user}, $setup->{passwd});

      my $conn = $client->mlsd_raw($tmpdir);
      unless ($conn) {
        die("Failed to MLSD: " . $client->response_code() . " " .
          $client->response_msg());
      }

      # Read 8 bytes of data, then abort the download
      my $buf;
      $conn->read($buf, 8, 30);
      $conn->_close();

      # There is a potential race here, between data EOF and our next command.
      # We thus sleep here, to let the data EOF win the race.  With that,
      # we should now receive our end-of-transfer response.
      sleep(1);

      my ($resp_code, $resp_msg) = $client->read_response();

      my $expected = 426;
      $self->assert($expected == $resp_code,
        test_msg("Expected response code $expected, got $resp_code"));

      $expected = 'Transfer aborted. Data connection closed';
      $self->assert($expected eq $resp_msg,
        test_msg("Expected response message '$expected', got '$resp_msg'"));

      # We use the Net::FTP::abort() method here, so that we use, in this
      # order: data EOF, TCP OOB, ABOR.
      $client->abort();
      $resp_code = $client->response_code();
      $resp_msg = $client->response_msg();

      $expected = 226;
      $self->assert($expected == $resp_code,
        test_msg("Expected response code $expected, got $resp_code"));

      $expected = 'Abort successful';
      $self->assert($expected eq $resp_msg,
        test_msg("Expected response message '$expected', got '$resp_msg'"));

      # Make sure the control connection did not close because of the abort.
      ($resp_code, $resp_msg) = $client->quit();

      $expected = 221;
      $self->assert($expected == $resp_code,
        test_msg("Expected response code $expected, got $resp_code"));

      $expected = 'Goodbye.';
      $self->assert($expected eq $resp_msg,
        test_msg("Expected response message '$expected', got '$resp_msg'"));
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
