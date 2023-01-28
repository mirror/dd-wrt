package ProFTPD::Tests::Config::MaxLoginAttempts;

use lib qw(t/lib);
use base qw(ProFTPD::TestSuite::Child);
use strict;

use File::Spec;
use IO::Handle;

use ProFTPD::TestSuite::FTP;
use ProFTPD::TestSuite::Utils qw(:auth :config :running :test :testsuite);

$| = 1;

my $order = 0;

my $TESTS = {
  maxloginattempts_one => {
    order => ++$order,
    test_class => [qw(forking)],
  },

  maxloginattempts_absent => {
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

sub maxloginattempts_one {
  my $self = shift;
  my $tmpdir = $self->{tmpdir};
  my $setup = test_setup($tmpdir, 'config');

  my $max_logins = 1;

  my $config = {
    PidFile => $setup->{pid_file},
    ScoreboardFile => $setup->{scoreboard_file},
    SystemLog => $setup->{log_file},

    AuthUserFile => $setup->{auth_user_file},
    AuthGroupFile => $setup->{auth_group_file},
    AuthOrder => 'mod_auth_file.c',

    MaxLoginAttempts => $max_logins,

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

      eval { $client->login($setup->{user}, 'foo') };
      unless ($@) {
        die("Logged in unexpectedly");
      }

      my $resp_code = $client->response_code();
      my $resp_msg = $client->response_msg(0);

      my $expected = 530;
      $self->assert($expected == $resp_code,
        test_msg("Expected response code $expected, got $resp_code"));

      $expected = "Login incorrect.";
      $self->assert($expected eq $resp_msg,
        test_msg("Expected response message '$expected', got '$resp_msg'"));

      # A MaxLoginAttempts of one should have caused our connection to be
      # closed above.

      eval { ($resp_code, $resp_msg) = $client->login($setup->{user}, 'foo') };
      unless ($@) {
        die("Logged in unexpectedly ($resp_code $resp_msg)");
      }

      $resp_code = $client->response_code();
      $resp_msg = $client->response_msg(0);

      # Perl's Net::Cmd module uses a very non-standard 599 code to
      # indicate that the connection is closed, depending on version.
      $self->assert($resp_code == 421 || $resp_code == 599,
        test_msg("Expected response code 421 or 599, got $resp_code"));
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

  # We can peruse the generated debug log messages for what we want, but
  # only if the TEST_VERBOSE environment variable is true.
  unless ($ENV{TEST_VERBOSE}) {
    test_cleanup($setup->{log_file}, $ex);
    return;
  }

  eval {
    if (open(my $fh, "< $setup->{log_file}")) {
      my $expected_post_cmd_err = 0;
      my $expected_log_cmd_err = 0;

      while (my $line = <$fh>) {
        chomp($line);

        if ($ENV{TEST_VERBOSE}) {
          print STDERR "# $line\n";
        }

        if ($line =~ /POST_CMD_ERR command 'PASS/) {
          $expected_post_cmd_err = 1;
          next;
        }

        if ($line =~ /LOG_CMD_ERR command 'PASS/) {
          $expected_log_cmd_err = 1;
          next;
        }
      }

      close($fh);

      $self->assert($expected_post_cmd_err && $expected_log_cmd_err,
        test_msg("Did not see expected PASS POST_CMD_ERR and LOG_CMD_ERR log messages"));

    } else {
      die("Can't read $setup->{log_file}: $!");
    }
  };
  if ($@) {
    $ex = $@;
  }

  test_cleanup($setup->{log_file}, $ex);
}

sub maxloginattempts_absent {
  my $self = shift;
  my $tmpdir = $self->{tmpdir};
  my $setup = test_setup($tmpdir, 'config');

  my $config = {
    PidFile => $setup->{pid_file},
    ScoreboardFile => $setup->{scoreboard_file},
    SystemLog => $setup->{log_file},

    AuthUserFile => $setup->{auth_user_file},
    AuthGroupFile => $setup->{auth_group_file},
    AuthOrder => 'mod_auth_file.c',

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

      for (my $i = 0; $i < 3; $i++) {
        eval { $client->login($setup->{user}, 'foo') };
        unless ($@) {
          die("Logged in unexpectedly");
        }

        my $resp_code = $client->response_code();
        my $resp_msg = $client->response_msg(0);

        my $expected = 530;
        $self->assert($expected == $resp_code,
          test_msg("Expected response code $expected, got $resp_code"));

        $expected = "Login incorrect.";
        $self->assert($expected eq $resp_msg,
          test_msg("Expected response message '$expected', got '$resp_msg'"));
      }

      eval { $client->login($setup->{user}, 'foo') };
      unless ($@) {
        die("Logged in unexpectedly");
      }

      my $resp_code = $client->response_code();
      my $resp_msg = $client->response_msg(0);

      # Perl's Net::Cmd module uses a very non-standard 599 code to
      # indicate that the connection is closed, depending on version.
      $self->assert($resp_code == 421 || $resp_code == 599,
        test_msg("Expected response code 421 or 599, got $resp_code"));
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
