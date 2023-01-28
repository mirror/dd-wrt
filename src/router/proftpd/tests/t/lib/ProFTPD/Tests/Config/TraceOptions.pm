package ProFTPD::Tests::Config::TraceOptions;

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
  traceoptions_conn_ips => {
    order => ++$order,
    test_class => [qw(feature_trace forking)],
  },

  traceoptions_timestamp_millis => {
    order => ++$order,
    test_class => [qw(feature_trace forking)],
  },

  traceoptions_no_timestamp => {
    order => ++$order,
    test_class => [qw(feature_trace forking)],
  },
};

sub new {
  return shift()->SUPER::new(@_);
}

sub list_tests {
  return testsuite_get_runnable_tests($TESTS);
}

sub traceoptions_conn_ips {
  my $self = shift;
  my $tmpdir = $self->{tmpdir};
  my $setup = test_setup($tmpdir, 'trace');

  my $trace_log = File::Spec->rel2abs("$tmpdir/trace.log");

  my $config = {
    PidFile => $setup->{pid_file},
    ScoreboardFile => $setup->{scoreboard_file},
    SystemLog => $setup->{log_file},
    TraceLog => $trace_log,
    Trace => 'DEFAULT:10',
    TraceOptions => '+ConnIPs',

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

  eval {
    if (open(my $fh, "< $trace_log")) {
      my $ok = 0;

      while (my $line = <$fh>) {
        chomp($line);

        my $expected = '\[\d+\]\s+\(client\s+(\S+), server\s+\S+\)\s+<(\S+):(\d+)>: (.*?)$';

        if ($line =~ /$expected/) {
          my $client_ip = $1;
          my $trace_channel = $2;
          my $trace_level = $3;
          my $trace_msg = $4;

          next unless $trace_channel eq 'command';

          my $expected = 'command';
          $self->assert($expected eq $trace_channel,
            test_msg("Expected '$expected', got '$trace_channel'"));

          $expected = 7;
          $self->assert($expected >= $trace_level,
            test_msg("Expected >= $expected, got $trace_level"));

          $expected = '127.0.0.1';
          $self->assert($expected eq $client_ip,
            test_msg("Expected '$expected', got '$client_ip'"));

          $ok = 1;
          last;
        }
      }

      close($fh);

      $self->assert($ok, test_msg("Missing expected TraceLog messages"));

    } else {
      die("Can't read $trace_log: $!");
    }
  };
  if ($@) {
    $ex = $@;
  }

  test_cleanup($setup->{log_file}, $ex);
}

sub traceoptions_timestamp_millis {
  my $self = shift;
  my $tmpdir = $self->{tmpdir};
  my $setup = test_setup($tmpdir, 'trace');

  my $trace_log = File::Spec->rel2abs("$tmpdir/trace.log");

  my $config = {
    PidFile => $setup->{pid_file},
    ScoreboardFile => $setup->{scoreboard_file},
    SystemLog => $setup->{log_file},
    TraceLog => $trace_log,
    Trace => 'DEFAULT:10',
    TraceOptions => '+TimestampMillis',

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

  eval {
    if (open(my $fh, "< $trace_log")) {
      my $ok = 0;

      while (my $line = <$fh>) {
        chomp($line);

        if ($ENV{TEST_VERBOSE}) {
          print STDERR "# $line\n";
        }

        my $expected = '\d+:\d+:\d+,\d+\s+\[\d+\]\s+<(\S+):(\d+)>: (.*?)$';

        if ($line =~ /$expected/) {
          my $trace_channel = $1;
          my $trace_level = $2;
          my $trace_msg = $3;

          next unless $trace_channel eq 'command';

          my $expected = 'command';
          $self->assert($expected eq $trace_channel,
            test_msg("Expected '$expected', got '$trace_channel'"));

          $expected = 7;
          $self->assert($expected >= $trace_level,
            test_msg("Expected >= $expected, got $trace_level"));

          $ok = 1;
          last;
        }
      }

      close($fh);

      $self->assert($ok, test_msg("Missing expected TraceLog messages"));

    } else {
      die("Can't read $trace_log: $!");
    }
  };
  if ($@) {
    $ex = $@;
  }

  test_cleanup($setup->{log_file}, $ex);
}

sub traceoptions_no_timestamp {
  my $self = shift;
  my $tmpdir = $self->{tmpdir};
  my $setup = test_setup($tmpdir, 'trace');

  my $trace_log = File::Spec->rel2abs("$tmpdir/trace.log");

  my $config = {
    PidFile => $setup->{pid_file},
    ScoreboardFile => $setup->{scoreboard_file},
    SystemLog => $setup->{log_file},
    TraceLog => $trace_log,
    Trace => 'DEFAULT:10',
    TraceOptions => '-Timestamp',

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

  eval {
    if (open(my $fh, "< $trace_log")) {
      my $ok = 0;

      while (my $line = <$fh>) {
        chomp($line);

        if ($ENV{TEST_VERBOSE}) {
          print STDERR "# $line\n";
        }

        my $expected = '\[\d+\]\s+<(\S+):(\d+)>: (.*?)$';

        if ($line =~ /$expected/) {
          my $trace_channel = $1;
          my $trace_level = $2;
          my $trace_msg = $3;

          next unless $trace_channel eq 'command';

          my $expected = 'command';
          $self->assert($expected eq $trace_channel,
            test_msg("Expected '$expected', got '$trace_channel'"));

          $expected = 7;
          $self->assert($expected >= $trace_level,
            test_msg("Expected >= $expected, got $trace_level"));

          $ok = 1;
          last;
        }
      }

      close($fh);

      $self->assert($ok, test_msg("Missing expected TraceLog messages"));

    } else {
      die("Can't read $trace_log: $!");
    }
  };
  if ($@) {
    $ex = $@;
  }

  test_cleanup($setup->{log_file}, $ex);
}

1;
