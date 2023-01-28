package ProFTPD::Tests::Modules::mod_facts;

use lib qw(t/lib);
use base qw(ProFTPD::TestSuite::Child);
use strict;

use File::Copy;
use File::Path qw(mkpath);
use File::Spec;
use IO::Handle;

use ProFTPD::TestSuite::FTP;
use ProFTPD::TestSuite::Utils qw(:auth :config :running :test :testsuite);

$| = 1;

my $order = 0;

my $TESTS = {
  facts_advertise_off => {
    order => ++$order,
    test_class => [qw(forking)],
  },

  facts_default_issue1367 => {
    order => ++$order,
    test_class => [qw(bug forking)],
  },

  facts_ownername_with_space_issue1367 => {
    order => ++$order,
    test_class => [qw(bug forking)],
  },

  facts_groupname_with_space_issue1367 => {
    order => ++$order,
    test_class => [qw(bug forking)],
  },

  opts_mlst_invalid => {
    order => ++$order,
    test_class => [qw(forking)],
  },

  opts_mlst_clear => {
    order => ++$order,
    test_class => [qw(forking)],
  },

  opts_mlst_set => {
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

sub facts_advertise_off {
  my $self = shift;
  my $tmpdir = $self->{tmpdir};
  my $setup = test_setup($tmpdir, 'facts');

  my $config = {
    PidFile => $setup->{pid_file},
    ScoreboardFile => $setup->{scoreboard_file},
    SystemLog => $setup->{log_file},
    TraceLog => $setup->{log_file},
    Trace => 'facts:20',

    AuthUserFile => $setup->{auth_user_file},
    AuthGroupFile => $setup->{auth_group_file},
    AuthOrder => 'mod_auth_file.c',

    IfModules => {
      'mod_delay.c' => {
        DelayEngine => 'off',
      },

      'mod_facts.c' => {
        FactsAdvertise => 'off',
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

      my $conn = $client->mlsd_raw();
      unless ($conn) {
        die("Failed to MLSD: " . $client->response_code() . ' ' .
          $client->response_msg());
      }

      my $buf;
      $conn->read($buf, 8192, 10);
      eval { $conn->close() };

      if ($ENV{TEST_VERBOSE}) {
        print STDERR "# Response:\n$buf\n";
      }

      my $resp_code = $client->response_code();
      my $resp_msg = $client->response_msg();
      $self->assert_transfer_ok($resp_code, $resp_msg);

      $client->quit();

      # We have to be careful of the fact that readdir returns directory
      # entries in an unordered fashion.
      my $res = {};
      my $lines = [split(/\r\n/, $buf)];
      $self->assert(scalar(@$lines) > 1,
        test_msg("Expected several MLSD lines, got " . scalar(@$lines)));

      foreach my $line (@$lines) {
        if ($line =~ /^modify=\S+;perm=\S+;type=\S+;unique=\S+;UNIX\.group=\d+;UNIX\.groupname=\S+;UNIX\.mode=\d+;UNIX\.owner=\d+;UNIX\.ownername=\S+; (.*?)$/) {
          $res->{$1} = 1;
        }
      }

      # Even though we have "FactsAdvertise off", the MLSD results should
      # still have the default facts enabled.
      my $expected = {
        '.' => 1,
        '..' => 1,
        'facts.conf' => 1,
        'facts.group' => 1,
        'facts.passwd' => 1,
        'facts.pid' => 1,
        'facts.scoreboard' => 1,
        'facts.scoreboard.lck' => 1,
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
        die("Unexpected name '$mismatch' appeared in MLSD data")
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

sub facts_default_issue1367 {
  my $self = shift;
  my $tmpdir = $self->{tmpdir};
  my $setup = test_setup($tmpdir, 'facts');

  my $config = {
    PidFile => $setup->{pid_file},
    ScoreboardFile => $setup->{scoreboard_file},
    SystemLog => $setup->{log_file},
    TraceLog => $setup->{log_file},
    Trace => 'facts:20',

    AuthUserFile => $setup->{auth_user_file},
    AuthGroupFile => $setup->{auth_group_file},
    AuthOrder => 'mod_auth_file.c',

    IfModules => {
      'mod_delay.c' => {
        DelayEngine => 'off',
      },

      'mod_facts.c' => {
        FactsDefault => 'perm size unique UNIX.groupname UNIX.ownername',
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

      my $conn = $client->mlsd_raw();
      unless ($conn) {
        die("Failed to MLSD: " . $client->response_code() . ' ' .
          $client->response_msg());
      }

      my $buf;
      $conn->read($buf, 8192, 10);
      eval { $conn->close() };

      if ($ENV{TEST_VERBOSE}) {
        print STDERR "# Response:\n$buf\n";
      }

      my $resp_code = $client->response_code();
      my $resp_msg = $client->response_msg();
      $self->assert_transfer_ok($resp_code, $resp_msg);

      $client->quit();

      # We have to be careful of the fact that readdir returns directory
      # entries in an unordered fashion.
      my $res = {};
      my $lines = [split(/\r\n/, $buf)];
      $self->assert(scalar(@$lines) > 1,
        test_msg("Expected several MLSD lines, got " . scalar(@$lines)));

      foreach my $line (@$lines) {
        # Directory entries will not have the "size" fact.

        if ($line =~ /^perm=\S+;(size=\d+;)?unique=\S+;UNIX\.groupname=\S+;UNIX\.ownername=\S+; (.*?)$/) {
          $res->{$2} = 1;
        }
      }

      my $expected = {
        '.' => 1,
        '..' => 1,
        'facts.conf' => 1,
        'facts.group' => 1,
        'facts.passwd' => 1,
        'facts.pid' => 1,
        'facts.scoreboard' => 1,
        'facts.scoreboard.lck' => 1,
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
        die("Unexpected name '$mismatch' appeared in MLSD data")
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

sub facts_ownername_with_space_issue1367 {
  my $self = shift;
  my $tmpdir = $self->{tmpdir};

  # To reproduce the behavior reported in Issue #1367, we deliberately
  # use a username that includes spaces.  The current mitigation is that
  # mod_facts replaces all whitespace characters with '_' in such cases.

  my $username = 'My ProFTPD User';
  my $setup = test_setup($tmpdir, 'facts', $username);

  my $expected_ownername = $username;
  $expected_ownername =~ s/\s/_/g;

  my $config = {
    PidFile => $setup->{pid_file},
    ScoreboardFile => $setup->{scoreboard_file},
    SystemLog => $setup->{log_file},
    TraceLog => $setup->{log_file},
    Trace => 'facts:20',

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
      $client->type('binary');

      my $conn = $client->mlsd_raw();
      unless ($conn) {
        die("Failed to MLSD: " . $client->response_code() . ' ' .
          $client->response_msg());
      }

      my $buf;
      $conn->read($buf, 8192, 10);
      eval { $conn->close() };

      if ($ENV{TEST_VERBOSE}) {
        print STDERR "# Response:\n$buf\n";
      }

      my $resp_code = $client->response_code();
      my $resp_msg = $client->response_msg();
      $self->assert_transfer_ok($resp_code, $resp_msg);

      $client->quit();

      # We have to be careful of the fact that readdir returns directory
      # entries in an unordered fashion.
      my $res = {};
      my $lines = [split(/\r\n/, $buf)];
      $self->assert(scalar(@$lines) > 1,
        test_msg("Expected several MLSD lines, got " . scalar(@$lines)));

      foreach my $line (@$lines) {
        if ($line =~ /^modify=\S+;(size=\d+;)?perm=\S+;type=\S+;unique=\S+;UNIX\.group=\d+;UNIX\.groupname=\S+;UNIX\.mode=\d+;UNIX\.owner=\d+;UNIX\.ownername=(.*?); (.*?)$/) {
          $res->{$3} = 1;

          my $facts_ownername = $2;
          $self->assert($facts_ownername eq $expected_ownername,
            test_msg("Expected UNIX.ownername '$expected_ownername', got '$facts_ownername' in '$line'"));
        }
      }

      my $expected = {
        '.' => 1,
        '..' => 1,
        'facts.conf' => 1,
        'facts.group' => 1,
        'facts.passwd' => 1,
        'facts.pid' => 1,
        'facts.scoreboard' => 1,
        'facts.scoreboard.lck' => 1,
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
        die("Unexpected name '$mismatch' appeared in MLSD data")
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

sub facts_groupname_with_space_issue1367 {
  my $self = shift;
  my $tmpdir = $self->{tmpdir};

  # To reproduce the behavior reported in Issue #1367, we deliberately
  # use a group name that includes spaces.  The current mitigation is that
  # mod_facts replaces all whitespace characters with '_' in such cases.

  my $groupname = 'My ProFTPD Group';
  my $setup = test_setup($tmpdir, 'facts', undef, undef, $groupname);

  my $expected_groupname = $groupname;
  $expected_groupname =~ s/\s/_/g;

  my $config = {
    PidFile => $setup->{pid_file},
    ScoreboardFile => $setup->{scoreboard_file},
    SystemLog => $setup->{log_file},
    TraceLog => $setup->{log_file},
    Trace => 'facts:20',

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
      $client->type('binary');

      my $conn = $client->mlsd_raw();
      unless ($conn) {
        die("Failed to MLSD: " . $client->response_code() . ' ' .
          $client->response_msg());
      }

      my $buf;
      $conn->read($buf, 8192, 10);
      eval { $conn->close() };

      if ($ENV{TEST_VERBOSE}) {
        print STDERR "# Response:\n$buf\n";
      }

      my $resp_code = $client->response_code();
      my $resp_msg = $client->response_msg();
      $self->assert_transfer_ok($resp_code, $resp_msg);

      $client->quit();

      # We have to be careful of the fact that readdir returns directory
      # entries in an unordered fashion.
      my $res = {};
      my $lines = [split(/\r\n/, $buf)];
      $self->assert(scalar(@$lines) > 1,
        test_msg("Expected several MLSD lines, got " . scalar(@$lines)));

      foreach my $line (@$lines) {
        if ($line =~ /^modify=\S+;(size=\d+;)?perm=\S+;type=\S+;unique=\S+;UNIX\.group=\d+;UNIX\.groupname=(.*?);UNIX\.mode=\d+;UNIX\.owner=\d+;UNIX\.ownername=.*?; (.*?)$/) {
          $res->{$3} = 1;

          my $facts_groupname = $2;
          $self->assert($facts_groupname eq $expected_groupname,
            test_msg("Expected UNIX.groupname '$expected_groupname', got '$facts_groupname' in '$line'"));
        }
      }

      my $expected = {
        '.' => 1,
        '..' => 1,
        'facts.conf' => 1,
        'facts.group' => 1,
        'facts.passwd' => 1,
        'facts.pid' => 1,
        'facts.scoreboard' => 1,
        'facts.scoreboard.lck' => 1,
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
        die("Unexpected name '$mismatch' appeared in MLSD data")
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

sub opts_mlst_invalid {
  my $self = shift;
  my $tmpdir = $self->{tmpdir};
  my $setup = test_setup($tmpdir, 'facts');

  my $config = {
    PidFile => $setup->{pid_file},
    ScoreboardFile => $setup->{scoreboard_file},
    SystemLog => $setup->{log_file},
    TraceLog => $setup->{log_file},
    Trace => 'DEFAULT:10 facts:20',

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

      my $opts_args = 'MLST FOO BAR';

      eval { $client->opts($opts_args) };
      unless ($@) {
        die("OPTS command succeeded unexpectedly");
      }

      my $resp_code = $client->response_code();
      my $resp_msg = $client->response_msg();

      my $expected = 501;
      $self->assert($expected == $resp_code,
        test_msg("Expected response code $expected, got $resp_code"));

      $expected = "'OPTS MLST' not understood";
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

sub opts_mlst_clear {
  my $self = shift;
  my $tmpdir = $self->{tmpdir};
  my $setup = test_setup($tmpdir, 'facts');

  my $config = {
    PidFile => $setup->{pid_file},
    ScoreboardFile => $setup->{scoreboard_file},
    SystemLog => $setup->{log_file},
    TraceLog => $setup->{log_file},
    Trace => 'DEFAULT:10 facts:20',

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

      # First, discover the current MLST options via FEAT.
      $client->feat();
      my $resp_code = $client->response_code();
      my $resp_msgs = $client->response_msgs();

      my $expected = 211;
      $self->assert($expected == $resp_code,
        test_msg("Expected response code $expected, got $resp_code"));

      my $current_mlst_facts;

      my $nfeat = scalar(@$resp_msgs);
      for (my $i = 0; $i < $nfeat; $i++) {
        my $feat = $resp_msgs->[$i];

        if ($feat =~ / MLST (.*)/) {
          $current_mlst_facts = $1;
          last;
        }
      }

      $self->assert(defined($current_mlst_facts),
        test_msg("Expected to find MLST facts via FEAT"));

      my $opts_args = 'MLST';
      my $resp_msg;
      ($resp_code, $resp_msg) = $client->opts($opts_args);

      $expected = 200;
      $self->assert($expected == $resp_code,
        test_msg("Expected response code $expected, got $resp_code"));

      $expected = 'MLST OPTS';
      $self->assert($expected eq $resp_msg,
        test_msg("Expected response message '$expected', got '$resp_msg'"));

      # Now, read the set MLST options again via FEAT.
      $client->feat();
      $resp_code = $client->response_code();
      $resp_msgs = $client->response_msgs();

      $expected = 211;
      $self->assert($expected == $resp_code,
        test_msg("Expected response code $expected, got $resp_code"));

      my $new_mlst_facts;

      $nfeat = scalar(@$resp_msgs);
      for (my $i = 0; $i < $nfeat; $i++) {
        my $feat = $resp_msgs->[$i];

        if ($feat =~ / MLST (.*)/) {
          $new_mlst_facts = $1;
          last;
        }
      }

      # In our new facts, we don't expect to see any '*', which indicate
      # which, of the supported facts, are enabled.
      $self->assert($new_mlst_facts !~ /\*/,
        test_msg("Expected to see no enabled MLST facts via FEAT, saw '$new_mlst_facts'"));

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

sub opts_mlst_set {
  my $self = shift;
  my $tmpdir = $self->{tmpdir};
  my $setup = test_setup($tmpdir, 'facts');

  my $config = {
    PidFile => $setup->{pid_file},
    ScoreboardFile => $setup->{scoreboard_file},
    SystemLog => $setup->{log_file},
    TraceLog => $setup->{log_file},
    Trace => 'DEFAULT:10 facts:20',

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

      # First, discover the current MLST options via FEAT.
      $client->feat();
      my $resp_code = $client->response_code();
      my $resp_msgs = $client->response_msgs();

      my $expected = 211;
      $self->assert($expected == $resp_code,
        test_msg("Expected response code $expected, got $resp_code"));

      my $current_mlst_facts;

      my $nfeat = scalar(@$resp_msgs);
      for (my $i = 0; $i < $nfeat; $i++) {
        my $feat = $resp_msgs->[$i];

        if ($feat =~ / MLST (.*)/) {
          $current_mlst_facts = $1;
          last;
        }
      }

      $self->assert(defined($current_mlst_facts),
        test_msg("Expected to find MLST facts via FEAT"));

      my $opts_args = 'MLST modify;size;type;';
      my $resp_msg;
      ($resp_code, $resp_msg) = $client->opts($opts_args);

      $expected = 200;
      $self->assert($expected == $resp_code,
        test_msg("Expected response code $expected, got $resp_code"));

      $expected = 'MLST OPTS modify;size;type;';
      $self->assert($expected eq $resp_msg,
        test_msg("Expected response message '$expected', got '$resp_msg'"));

      # Now, read the set MLST options again via FEAT.
      $client->feat();
      $resp_code = $client->response_code();
      $resp_msgs = $client->response_msgs();

      $expected = 211;
      $self->assert($expected == $resp_code,
        test_msg("Expected response code $expected, got $resp_code"));

      my $new_mlst_facts;

      $nfeat = scalar(@$resp_msgs);
      for (my $i = 0; $i < $nfeat; $i++) {
        my $feat = $resp_msgs->[$i];

        if ($feat =~ / MLST (.*)/) {
          $new_mlst_facts = $1;
          last;
        }
      }

      $self->assert($new_mlst_facts =~ /modify\*;.*;size\*;type\*;/,
        test_msg("Expected to see some enabled MLST facts via FEAT, saw '$new_mlst_facts'"));

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
