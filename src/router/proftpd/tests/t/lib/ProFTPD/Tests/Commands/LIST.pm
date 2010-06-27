package ProFTPD::Tests::Commands::LIST;

use lib qw(t/lib);
use base qw(Test::Unit::TestCase ProFTPD::TestSuite::Child);
use strict;

use Cwd;
use File::Path qw(mkpath rmtree);
use File::Spec;
use IO::Handle;

use ProFTPD::TestSuite::FTP;
use ProFTPD::TestSuite::Utils qw(:auth :config :features :running :test :testsuite);

$| = 1;

my $order = 0;

my $TESTS = {
  list_ok_raw_active => {
    order => ++$order,
    test_class => [qw(forking)],
  },

  list_ok_raw_passive => {
    order => ++$order,
    test_class => [qw(forking)],
  },

  list_ok_file => {
    order => ++$order,
    test_class => [qw(forking)],
  },

  list_ok_dir => {
    order => ++$order,
    test_class => [qw(forking)],
  },

  list_ok_no_path => {
    order => ++$order,
    test_class => [qw(forking)],
  },

  list_ok_glob => {
    order => ++$order,
    test_class => [qw(forking)],
  },

  list_fails_login_required => {
    order => ++$order,
    test_class => [qw(forking)],
  },

  list_fails_enoent => {
    order => ++$order,
    test_class => [qw(forking)],
  },

  list_fails_enoent_glob => {
    order => ++$order,
    test_class => [qw(forking)],
  },

  list_fails_eperm => {
    order => ++$order,
    test_class => [qw(forking)],
  },

  list_bug2821 => {
    order => ++$order,
    test_class => [qw(bug forking slow)],
  },

  list_opt_C => {
    order => ++$order,
    test_class => [qw(bug forking)],
  },

  list_nonascii_chars_bug3032 => {
    order => ++$order,
    test_class => [qw(bug forking)],
  },

  list_leading_whitespace_bug3268 => {
    order => ++$order,
    test_class => [qw(bug forking)],
  },

  list_leading_whitespace_with_opts_bug3268 => {
    order => ++$order,
    test_class => [qw(bug forking)],
  },

  list_leading_whitespace_with_strict_opts_bug3268 => {
    order => ++$order,
    test_class => [qw(bug forking)],
  },

  list_symlink_bug3254 => {
    order => ++$order,
    test_class => [qw(bug forking rootprivs)],
  },

  list_showsymlinks_on => {
    order => ++$order,
    test_class => [qw(forking)],
  },

  list_glob_bug2367 => {
    order => ++$order,
    test_class => [qw(bug forking)],
  },

  list_glob_legit_dir_name => {
    order => ++$order,
    test_class => [qw(forking)],
  },

  # XXX Plenty of other tests needed: params, maxfiles, maxdirs, depth, etc
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

sub list_ok_raw_active {
  my $self = shift;
  my $tmpdir = $self->{tmpdir};

  my $config_file = "$tmpdir/cmds.conf";
  my $pid_file = File::Spec->rel2abs("$tmpdir/cmds.pid");
  my $scoreboard_file = File::Spec->rel2abs("$tmpdir/cmds.scoreboard");

  my $log_file = File::Spec->rel2abs('tests.log');

  my $auth_user_file = File::Spec->rel2abs("$tmpdir/cmds.passwd");
  my $auth_group_file = File::Spec->rel2abs("$tmpdir/cmds.group");

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

    IfModules => {
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

      $client->login($user, $passwd);

      my $conn = $client->list_raw();
      unless ($conn) {
        die("Failed to LIST: " . $client->response_code() . " " .
          $client->response_msg());
      }

      my $buf;
      $conn->read($buf, 8192);
      $conn->close();

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
        'cmds.conf' => 1,
        'cmds.group' => 1,
        'cmds.passwd' => 1,
        'cmds.pid' => 1,
        'cmds.scoreboard' => 1,
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

sub list_ok_raw_passive {
  my $self = shift;
  my $tmpdir = $self->{tmpdir};

  my $config_file = "$tmpdir/cmds.conf";
  my $pid_file = File::Spec->rel2abs("$tmpdir/cmds.pid");
  my $scoreboard_file = File::Spec->rel2abs("$tmpdir/cmds.scoreboard");

  my $log_file = File::Spec->rel2abs('tests.log');

  my $auth_user_file = File::Spec->rel2abs("$tmpdir/cmds.passwd");
  my $auth_group_file = File::Spec->rel2abs("$tmpdir/cmds.group");

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

    IfModules => {
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
      my $client = ProFTPD::TestSuite::FTP->new('127.0.0.1', $port, 1);

      $client->login($user, $passwd);

      my $conn = $client->list_raw();
      unless ($conn) {
        die("Failed to LIST: " . $client->response_code() . " " .
          $client->response_msg());
      }

      my $buf;
      $conn->read($buf, 8192);
      $conn->close();

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
        'cmds.conf' => 1,
        'cmds.group' => 1,
        'cmds.passwd' => 1,
        'cmds.pid' => 1,
        'cmds.scoreboard' => 1,
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

sub list_ok_file {
  my $self = shift;
  my $tmpdir = $self->{tmpdir};

  my $config_file = "$tmpdir/cmds.conf";
  my $pid_file = File::Spec->rel2abs("$tmpdir/cmds.pid");
  my $scoreboard_file = File::Spec->rel2abs("$tmpdir/cmds.scoreboard");

  my $log_file = File::Spec->rel2abs('tests.log');

  my $auth_user_file = File::Spec->rel2abs("$tmpdir/cmds.passwd");
  my $auth_group_file = File::Spec->rel2abs("$tmpdir/cmds.group");

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

  my $test_file = File::Spec->rel2abs($config_file);

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

      $client->login($user, $passwd);

      my $conn = $client->list_raw($test_file);
      unless ($conn) {
        die("LIST failed: " . $client->response_code() . " " .
          $client->response_msg());
      }

      my $buf;
      $conn->read($buf, 8192);
      $conn->close();

      my $res = {};
      my $lines = [split(/\n/, $buf)];
      foreach my $line (@$lines) {
        if ($line =~ /^\S+\s+\d+\s+\S+\s+\S+\s+.*?\s+(\S+)$/) {
          $res->{$1} = 1;
        }
      }

      my $count = scalar(keys(%$res));
      unless ($count == 1) {
        die("LIST returned wrong number of entries (expected 1, got $count)");
      }

      unless (defined($res->{$test_file})) {
        die("LIST failed to return $test_file");
      }
    };

    if ($@) {
      $ex = $@;
    }

    $wfh->print("done\n");
    $wfh->flush();

  } else {
    eval { server_wait($config_file, $rfh, 2) };
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

sub list_ok_dir {
  my $self = shift;
  my $tmpdir = $self->{tmpdir};

  my $config_file = "$tmpdir/cmds.conf";
  my $pid_file = File::Spec->rel2abs("$tmpdir/cmds.pid");
  my $scoreboard_file = File::Spec->rel2abs("$tmpdir/cmds.scoreboard");

  my $log_file = File::Spec->rel2abs('tests.log');

  my $auth_user_file = File::Spec->rel2abs("$tmpdir/cmds.passwd");
  my $auth_group_file = File::Spec->rel2abs("$tmpdir/cmds.group");

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

    IfModules => {
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

      $client->login($user, $passwd);

      my $conn = $client->list_raw($home_dir);
      unless ($conn) {
        die("LIST failed: " . $client->response_code() . " " .
          $client->response_msg());
      }

      my $buf;
      $conn->read($buf, 8192);
      $conn->close();

      my $res = {};
      my $lines = [split(/\n/, $buf)];
      foreach my $line (@$lines) {
        if ($line =~ /^\S+\s+\d+\s+\S+\s+\S+\s+.*?\s+(\S+)$/) {
          $res->{$1} = 1;
        }
      }

      my $count = scalar(keys(%$res));
      unless ($count == 5) {
        die("LIST returned wrong number of entries (expected 5, got $count)");
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

sub list_ok_no_path {
  my $self = shift;
  my $tmpdir = $self->{tmpdir};

  my $config_file = "$tmpdir/cmds.conf";
  my $pid_file = File::Spec->rel2abs("$tmpdir/cmds.pid");
  my $scoreboard_file = File::Spec->rel2abs("$tmpdir/cmds.scoreboard");

  my $log_file = File::Spec->rel2abs('tests.log');

  my $auth_user_file = File::Spec->rel2abs("$tmpdir/cmds.passwd");
  my $auth_group_file = File::Spec->rel2abs("$tmpdir/cmds.group");

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

    IfModules => {
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

      $client->login($user, $passwd);

      my $conn = $client->list_raw();
      unless ($conn) {
        die("LIST failed: " . $client->response_code() . " " .
          $client->response_msg());
      }

      my $buf;
      $conn->read($buf, 8192);
      $conn->close();

      my $res = {};
      my $lines = [split(/\n/, $buf)];
      foreach my $line (@$lines) {
        if ($line =~ /^\S+\s+\d+\s+\S+\s+\S+\s+.*?\s+(\S+)$/) {
          $res->{$1} = 1;
        }
      }

      my $count = scalar(keys(%$res));
      unless ($count == 5) {
        die("LIST returned wrong number of entries (expected 5, got $count)");
      }
    };

    if ($@) {
      $ex = $@;
    }

    print $wfh "done\n";

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

sub list_ok_glob {
  my $self = shift;
  my $tmpdir = $self->{tmpdir};

  my $config_file = "$tmpdir/cmds.conf";
  my $pid_file = File::Spec->rel2abs("$tmpdir/cmds.pid");
  my $scoreboard_file = File::Spec->rel2abs("$tmpdir/cmds.scoreboard");

  my $log_file = File::Spec->rel2abs('tests.log');

  my $auth_user_file = File::Spec->rel2abs("$tmpdir/cmds.passwd");
  my $auth_group_file = File::Spec->rel2abs("$tmpdir/cmds.group");

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

    IfModules => {
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

      $client->login($user, $passwd);

      my $conn = $client->list_raw("*.conf");
      unless ($conn) {
        die("LIST failed: " . $client->response_code() . " " .
          $client->response_msg());
      }

      my $buf;
      $conn->read($buf, 8192);
      $conn->close();

      my $res = {};
      my $lines = [split(/\n/, $buf)];
      foreach my $line (@$lines) {
        if ($line =~ /^\S+\s+\d+\s+\S+\s+\S+\s+.*?\s+(\S+)$/) {
          $res->{$1} = 1;
        }
      }

      my $count = scalar(keys(%$res));
      unless ($count == 1) {
        die("LIST returned wrong number of entries (expected 1, got $count)");
      }

      my $test_file = 'cmds.conf';
      unless (defined($res->{$test_file})) {
        die("LIST failed to return $test_file");
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

sub list_fails_login_required {
  my $self = shift;
  my $tmpdir = $self->{tmpdir};

  my $config_file = "$tmpdir/cmds.conf";
  my $pid_file = File::Spec->rel2abs("$tmpdir/cmds.pid");
  my $scoreboard_file = File::Spec->rel2abs("$tmpdir/cmds.scoreboard");

  my $log_file = File::Spec->rel2abs('tests.log');

  my $config = {
    PidFile => $pid_file,
    ScoreboardFile => $scoreboard_file,
    SystemLog => $log_file,

    IfModules => {
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
      eval { ($resp_code, $resp_msg) = $client->list() };
      unless ($@) {
        die("LIST succeeded unexpectedly ($resp_code $resp_msg)");

      } else {
        $resp_code = $client->response_code();
        $resp_msg = $client->response_msg();
      }

      my $expected;

      $expected = 530;
      $self->assert($expected == $resp_code,
        test_msg("Expected $expected, got $resp_code"));

      $expected = "Please login with USER and PASS";
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

sub list_fails_enoent {
  my $self = shift;
  my $tmpdir = $self->{tmpdir};

  my $config_file = "$tmpdir/cmds.conf";
  my $pid_file = File::Spec->rel2abs("$tmpdir/cmds.pid");
  my $scoreboard_file = File::Spec->rel2abs("$tmpdir/cmds.scoreboard");

  my $log_file = File::Spec->rel2abs('tests.log');

  my $auth_user_file = File::Spec->rel2abs("$tmpdir/cmds.passwd");
  my $auth_group_file = File::Spec->rel2abs("$tmpdir/cmds.group");

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
  auth_group_write($auth_group_file, 'ftpd', $uid, $user);

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

      $client->login($user, $passwd);

      my ($resp_code, $resp_msg);
      $client->port();

      my $test_file = 'foo/bar/baz';

      eval { ($resp_code, $resp_msg) = $client->list($test_file) };
      unless ($@) {
        die("LIST succeeded unexpectedly ($resp_code $resp_msg)");

      } else {
        $resp_code = $client->response_code();
        $resp_msg = $client->response_msg();
      }

      my $expected;

      $expected = 450;
      $self->assert($expected == $resp_code,
        test_msg("Expected $expected, got $resp_code"));

      $expected = "$test_file: No such file or directory";
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

sub list_fails_enoent_glob {
  my $self = shift;
  my $tmpdir = $self->{tmpdir};

  my $config_file = "$tmpdir/cmds.conf";
  my $pid_file = File::Spec->rel2abs("$tmpdir/cmds.pid");
  my $scoreboard_file = File::Spec->rel2abs("$tmpdir/cmds.scoreboard");

  my $log_file = File::Spec->rel2abs('tests.log');

  my $auth_user_file = File::Spec->rel2abs("$tmpdir/cmds.passwd");
  my $auth_group_file = File::Spec->rel2abs("$tmpdir/cmds.group");

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

    IfModules => {
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

      $client->login($user, $passwd);
      $client->port();

      my $test_glob = '*foo';

      my ($resp_code, $resp_msg);
      ($resp_code, $resp_msg) = $client->list($test_glob);

      # Unlike NLST, for LIST proftpd returns 226, and simply sends an
      # empty list.

      my $expected;

      $expected = 226;
      $self->assert($expected == $resp_code,
        test_msg("Expected $expected, got $resp_code"));

      $expected = "Transfer complete";
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

sub list_fails_eperm {
  my $self = shift;
  my $tmpdir = $self->{tmpdir};

  my $config_file = "$tmpdir/cmds.conf";
  my $pid_file = File::Spec->rel2abs("$tmpdir/cmds.pid");
  my $scoreboard_file = File::Spec->rel2abs("$tmpdir/cmds.scoreboard");

  my $log_file = File::Spec->rel2abs('tests.log');

  my $auth_user_file = File::Spec->rel2abs("$tmpdir/cmds.passwd");
  my $auth_group_file = File::Spec->rel2abs("$tmpdir/cmds.group");

  my $user = 'proftpd';
  my $passwd = 'test';
  my $home_dir = File::Spec->rel2abs($tmpdir);
  my $uid = 500;
  my $gid = 500;

  my $sub_dir = File::Spec->rel2abs("$tmpdir/foo");
  mkpath($sub_dir);

  my $test_file = File::Spec->rel2abs("$tmpdir/foo/bar");
  if (open(my $fh, "> $test_file")) {
    close($fh);

  } else {
    die("Can't open $test_file: $!");
  }

  # Make sure that, if we're running as root, that the home directory has
  # permissions/privs set for the account we create
  if ($< == 0) {
    unless (chmod(0755, $home_dir, $sub_dir)) {
      die("Can't set perms on $home_dir, $sub_dir to 0755: $!");
    }

    unless (chown($uid, $gid, $home_dir, $sub_dir)) {
      die("Can't set owner of $home_dir, $sub_dir to $uid/$gid: $!");
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

    IfModules => {
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

      $client->login($user, $passwd);
      $client->port();

      chmod(0660, $sub_dir);

      my ($resp_code, $resp_msg);
      eval { ($resp_code, $resp_msg) = $client->list($test_file) };
      unless ($@) {
        die("LIST succeeded unexpectedly ($resp_code $resp_msg)");

      } else {
        $resp_code = $client->response_code();
        $resp_msg = $client->response_msg();
      }

      my $expected;

      $expected = 450;
      $self->assert($expected == $resp_code,
        test_msg("Expected $expected, got $resp_code"));

      $expected = "$test_file: Permission denied";
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

sub list_bug2821 {
  my $self = shift;
  my $tmpdir = $self->{tmpdir};
  
  my $config_file = "$tmpdir/cmds.conf";
  my $pid_file = File::Spec->rel2abs("$tmpdir/cmds.pid");
  my $scoreboard_file = File::Spec->rel2abs("$tmpdir/cmds.scoreboard");

  my $log_file = File::Spec->rel2abs('tests.log');

  my $auth_user_file = File::Spec->rel2abs("$tmpdir/cmds.passwd");
  my $auth_group_file = File::Spec->rel2abs("$tmpdir/cmds.group");

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

  # For this test, we need to create MANY (i.e. 110K) files in the
  # home directory.
  my $test_file_prefix = File::Spec->rel2abs("$tmpdir/test_");

  my $count = 110000;
  print STDOUT "# Creating $count files in $tmpdir\n";
  for (my $i = 1; $i <= $count; $i++) {
    my $test_file = 'test_' . sprintf("%07s", $i);
    my $test_path = "$home_dir/$test_file";

    if (open(my $fh, "> $test_path")) {
      close($fh);

    } else {
      die("Can't open $test_path: $!");
    }

    if ($i % 10000 == 0) {
      print STDOUT "# Created file $test_file\n";
    }
  }

  auth_user_write($auth_user_file, $user, $passwd, $uid, $gid, $home_dir,
    '/bin/bash');
  auth_group_write($auth_group_file, 'ftpd', $gid, $user);

  my $timeout = 900;

  my $config = {
    PidFile => $pid_file,
    ScoreboardFile => $scoreboard_file,
    SystemLog => $log_file,
    TraceLog => $log_file,
    Trace => 'DEFAULT:10 fsio:0 lock:0',

    AuthUserFile => $auth_user_file,
    AuthGroupFile => $auth_group_file,

    TimeoutIdle => $timeout + 15,
    TimeoutNoTransfer => $timeout + 15,

    IfModules => {
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

      $client->login($user, $passwd);

      # One of the symptoms of Bug#2821 is that a LIST (or NLST) glob
      # which matches more than 99999 files will not receive the full
      # list.
      #
      # The issue turned out to be the MAX_RESULTS (now
      # PR_TUNABLE_GLOBBING_MAX_MATCHES) limit in lib/glibc-glob.c.
      # So now, we deliberate make sure that the number of files returned
      # is less than the number of potential matches.

      my $conn = $client->list_raw("-C test_*");
      unless ($conn) {
        die("LIST failed: " . $client->response_code() . " " .
          $client->response_msg());
      }

      my $buf;
      my $tmp;
      while ($conn->read($tmp, 32768, $timeout)) {
        $buf .= $tmp;
      }
      $conn->close();
      $client->quit();

      my $res = {};

      # Preallocate memory for the expected hash size, to help speed up the
      # evaluation of the results.
      keys(%$res) = $count;

      my $lines = [split(/\n/, $buf)];
      foreach my $line (@$lines) {
        if ($line =~ /^\S+\s+\d+\s+\S+\s+\S+\s+.*?\s+(\S+)$/) {
          $res->{$line} = 1;
        }
      }

      my $list_count = scalar(keys(%$res));
      if ($list_count >= $count) {
        die("LIST returned wrong number of entries (expected less than $count, got $list_count); check the PR_TUNABLE_GLOBBING_MAX_MATCHES value");
      }

    };

    if ($@) {
      $ex = $@;
    }

    $wfh->print("done\n");
    $wfh->flush();

  } else {
    eval { server_wait($config_file, $rfh, $timeout * 2) };
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

sub list_opt_C {
  my $self = shift;
  my $tmpdir = $self->{tmpdir};

  my $config_file = "$tmpdir/cmds.conf";
  my $pid_file = File::Spec->rel2abs("$tmpdir/cmds.pid");
  my $scoreboard_file = File::Spec->rel2abs("$tmpdir/cmds.scoreboard");

  my $log_file = File::Spec->rel2abs('tests.log');

  my $auth_user_file = File::Spec->rel2abs("$tmpdir/cmds.passwd");
  my $auth_group_file = File::Spec->rel2abs("$tmpdir/cmds.group");

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

  # For this test, we need to create about 100 files in home directory.
  my $test_file_prefix = File::Spec->rel2abs($tmpdir);

  my $count = 100;
  print STDOUT "# Creating $count files in $tmpdir\n";
  for (my $i = 1; $i <= $count; $i++) {
    my $test_file = sprintf("%04s", $i);
    my $test_path = "$home_dir/$test_file";

    if (open(my $fh, "> $test_path")) {
      close($fh);

    } else {
      die("Can't open $test_path: $!");
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

    IfModules => {
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

      $client->login($user, $passwd);

      my $conn = $client->list_raw("-C 0*");
      unless ($conn) {
        die("LIST failed: " . $client->response_code() . " " .
          $client->response_msg());
      }

      my $buf;
      $conn->read($buf, 32768);
      $conn->close();

      my $res = {};
      my $lines = [split(/\n/, $buf)];
      foreach my $line (@$lines) {
        if ($line =~ /^(\S+)\s+(\S+)\s+(\S+)\s+(\S+)\s+(\S+)\s+(\S+)\s+(\S+)\s+(\S+)\s+(\S+)$/) {
          $res->{$1} = 1;
          $res->{$2} = 1;
          $res->{$3} = 1;
          $res->{$4} = 1;
          $res->{$5} = 1;
          $res->{$6} = 1;
          $res->{$7} = 1;
          $res->{$8} = 1;
          $res->{$9} = 1;

        } elsif ($line =~ /^(\S+)\s+(\S+)\s+(\S+)\s+(\S+)\s+(\S+)\s+(\S+)\s+(\S+)\s+(\S+)$/) {
          $res->{$1} = 1;
          $res->{$2} = 1;
          $res->{$3} = 1;
          $res->{$4} = 1;
          $res->{$5} = 1;
          $res->{$6} = 1;
          $res->{$7} = 1;
          $res->{$8} = 1;
        }
      }

      my $list_count = scalar(keys(%$res));
      unless ($list_count == $count) {
        die("LIST returned wrong number of entries (expected $count, got $list_count)");
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

sub list_nonascii_chars_bug3032 {
  my $self = shift;
  my $tmpdir = $self->{tmpdir};

  if (feature_have_feature_enabled('nls')) {
    # This test is only valid if NLS support is not enabled.
    return;
  }

  my $config_file = "$tmpdir/cmds.conf";
  my $pid_file = File::Spec->rel2abs("$tmpdir/cmds.pid");
  my $scoreboard_file = File::Spec->rel2abs("$tmpdir/cmds.scoreboard");

  my $log_file = File::Spec->rel2abs('tests.log');

  my $auth_user_file = File::Spec->rel2abs("$tmpdir/cmds.passwd");
  my $auth_group_file = File::Spec->rel2abs("$tmpdir/cmds.group");

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

  my $test_file = File::Spec->rel2abs("$tmpdir/" . sprintf("test\b"));
  if (open(my $fh, "> $test_file")) {
    close($fh);

  } else {
    die("Can't open $test_file: $!");
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

    IfModules => {
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

      $client->login($user, $passwd);

      my $conn = $client->list_raw("-B test*");
      unless ($conn) {
        die("LIST failed: " . $client->response_code() . " " .
          $client->response_msg());
      }

      my $buf;
      $conn->read($buf, 32768);
      $conn->close();

      my $res = {};
      my $lines = [split(/\n/, $buf)];
      foreach my $line (@$lines) {
        if ($line =~ /^\S+\s+\d+\s+\S+\s+\S+\s+.*?\s+(\S+)$/) {
          $res->{$1} = 1;
        }
      }

      my $expected = {
        'test\010' => 1,
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

sub list_leading_whitespace_bug3268 {
  my $self = shift;
  my $tmpdir = $self->{tmpdir};

  my $config_file = "$tmpdir/cmds.conf";
  my $pid_file = File::Spec->rel2abs("$tmpdir/cmds.pid");
  my $scoreboard_file = File::Spec->rel2abs("$tmpdir/cmds.scoreboard");

  my $log_file = File::Spec->rel2abs('tests.log');

  my $auth_user_file = File::Spec->rel2abs("$tmpdir/cmds.passwd");
  my $auth_group_file = File::Spec->rel2abs("$tmpdir/cmds.group");

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

  my $test_file = File::Spec->rel2abs("$tmpdir/ test.txt");
  if (open(my $fh, "> $test_file")) {
    close($fh);

  } else {
    die("Can't open '$test_file': $!");
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
      $client->login($user, $passwd);

      my $conn = $client->list_raw(' test.txt');
      unless ($conn) {
        die("Failed to LIST: " . $client->response_code() . " " .
          $client->response_msg());
      }

      my $buf;
      $conn->read($buf, 8192);
      $conn->close();

      my $resp_code = $client->response_code();
      my $resp_msg = $client->response_msg();

      my $expected;
   
      $expected = 226;
      $self->assert($expected == $resp_code,
        test_msg("Expected $expected, got $resp_code"));

      $expected = 'Transfer complete';
      $self->assert($expected eq $resp_msg,
        test_msg("Expected '$expected', got '$resp_msg'"));

      # We have to be careful of the fact that readdir returns directory
      # entries in an unordered fashion.
      my $res = {};
      my $lines = [split(/\n/, $buf)];
      foreach my $line (@$lines) {
        if ($line =~ /^\S+\s+\d+\s+\S+\s+\S+\s+\d+\s+\S+\s+\d+\s+\S+ (.*?)$/) {
          $res->{$1} = 1;
        }
      }

      $expected = {
        ' test.txt' => 1,
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

sub list_leading_whitespace_with_opts_bug3268 {
  my $self = shift;
  my $tmpdir = $self->{tmpdir};

  my $config_file = "$tmpdir/cmds.conf";
  my $pid_file = File::Spec->rel2abs("$tmpdir/cmds.pid");
  my $scoreboard_file = File::Spec->rel2abs("$tmpdir/cmds.scoreboard");

  my $log_file = File::Spec->rel2abs('tests.log');

  my $auth_user_file = File::Spec->rel2abs("$tmpdir/cmds.passwd");
  my $auth_group_file = File::Spec->rel2abs("$tmpdir/cmds.group");

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

  my $test_file = File::Spec->rel2abs("$tmpdir/ test.txt");
  if (open(my $fh, "> $test_file")) {
    close($fh);

  } else {
    die("Can't open '$test_file': $!");
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
      $client->login($user, $passwd);

      my $conn = $client->list_raw('-a -b  test.txt');
      unless ($conn) {
        die("Failed to LIST: " . $client->response_code() . " " .
          $client->response_msg());
      }

      my $buf;
      $conn->read($buf, 8192);
      $conn->close();

      my $resp_code = $client->response_code();
      my $resp_msg = $client->response_msg();

      my $expected;
   
      $expected = 226;
      $self->assert($expected == $resp_code,
        test_msg("Expected $expected, got $resp_code"));

      $expected = 'Transfer complete';
      $self->assert($expected eq $resp_msg,
        test_msg("Expected '$expected', got '$resp_msg'"));

      # We have to be careful of the fact that readdir returns directory
      # entries in an unordered fashion.
      my $res = {};
      my $lines = [split(/\n/, $buf)];
      foreach my $line (@$lines) {
        if ($line =~ /^\S+\s+\d+\s+\S+\s+\S+\s+\d+\s+\S+\s+\d+\s+\S+ (.*?)$/) {
          $res->{$1} = 1;
        }
      }

      $expected = {
        ' test.txt' => 1,
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

sub list_leading_whitespace_with_strict_opts_bug3268 {
  my $self = shift;
  my $tmpdir = $self->{tmpdir};

  my $config_file = "$tmpdir/cmds.conf";
  my $pid_file = File::Spec->rel2abs("$tmpdir/cmds.pid");
  my $scoreboard_file = File::Spec->rel2abs("$tmpdir/cmds.scoreboard");

  my $log_file = File::Spec->rel2abs('tests.log');

  my $auth_user_file = File::Spec->rel2abs("$tmpdir/cmds.passwd");
  my $auth_group_file = File::Spec->rel2abs("$tmpdir/cmds.group");

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

  my $test_file = File::Spec->rel2abs("$tmpdir/ test.txt");
  if (open(my $fh, "> $test_file")) {
    close($fh);

  } else {
    die("Can't open '$test_file': $!");
  }

  my $config = {
    PidFile => $pid_file,
    ScoreboardFile => $scoreboard_file,
    SystemLog => $log_file,

    AuthUserFile => $auth_user_file,
    AuthGroupFile => $auth_group_file,

    ListOptions => '"-a +R" strict',

    IfModules => {
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
      $client->login($user, $passwd);

      my $conn = $client->list_raw('-a  -b  test.txt');
      unless ($conn) {
        die("Failed to LIST: " . $client->response_code() . " " .
          $client->response_msg());
      }

      my $buf;
      $conn->read($buf, 8192);
      $conn->close();

      my $resp_code = $client->response_code();
      my $resp_msg = $client->response_msg();

      my $expected;
   
      $expected = 226;
      $self->assert($expected == $resp_code,
        test_msg("Expected $expected, got $resp_code"));

      $expected = 'Transfer complete';
      $self->assert($expected eq $resp_msg,
        test_msg("Expected '$expected', got '$resp_msg'"));

      # We have to be careful of the fact that readdir returns directory
      # entries in an unordered fashion.
      my $res = {};
      my $lines = [split(/\n/, $buf)];
      foreach my $line (@$lines) {
        if ($line =~ /^\S+\s+\d+\s+\S+\s+\S+\s+\d+\s+\S+\s+\d+\s+\S+ (.*?)$/) {
          $res->{$1} = 1;
        }
      }

      $expected = {
        ' test.txt' => 1,
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

sub list_symlink_bug3254 {
  my $self = shift;
  my $tmpdir = $self->{tmpdir};

  my $config_file = "$tmpdir/cmds.conf";
  my $pid_file = File::Spec->rel2abs("$tmpdir/cmds.pid");
  my $scoreboard_file = File::Spec->rel2abs("$tmpdir/cmds.scoreboard");

  my $log_file = File::Spec->rel2abs('tests.log');

  my $auth_user_file = File::Spec->rel2abs("$tmpdir/cmds.passwd");
  my $auth_group_file = File::Spec->rel2abs("$tmpdir/cmds.group");

  my $user = 'proftpd';
  my $passwd = 'test';
  my $home_dir = File::Spec->rel2abs($tmpdir);
  my $uid = 500;
  my $gid = 500;

  my $foo_dir = File::Spec->rel2abs("$tmpdir/foo");
  my $bar_dir = File::Spec->rel2abs("$tmpdir/bar");
  my $baz_dir = File::Spec->rel2abs("$tmpdir/bar/baz");
  my $quxx_dir = File::Spec->rel2abs("$tmpdir/bar/quxx");
  mkpath([$foo_dir, $bar_dir, $baz_dir, $quxx_dir]);

  # Change to the 'foo' directory in order to create a relative path in the
  # symlink we need

  my $cwd = getcwd();
  unless (chdir("$foo_dir")) {
    die("Can't chdir to $foo_dir: $!");
  }

  unless (symlink('../bar/baz', 'baz')) {
    die("Can't symlink '../bar/baz' to 'baz': $!");
  }

  unless (chdir($cwd)) {
    die("Can't chdir to $cwd: $!");
  }

  my $ftpaccess_file = File::Spec->rel2abs("$foo_dir/.ftpaccess");
  if (open(my $fh, "> $ftpaccess_file")) {
    print $fh <<EOL;
<Limit ALL>
  AllowUser $user
  DenyAll
</Limit>
EOL
    unless (close($fh)) {
      die("Can't write $ftpaccess_file: $!");
    }

  } else {
    die("Can't open $ftpaccess_file: $!");
  }

  $ftpaccess_file = File::Spec->rel2abs("$bar_dir/.ftpaccess");
  if (open(my $fh, "> $ftpaccess_file")) {
    print $fh <<EOL;
<Limit ALL>
  AllowUser bar
  DenyAll
</Limit>
EOL
    unless (close($fh)) {
      die("Can't write $ftpaccess_file: $!");
    }

  } else {
    die("Can't open $ftpaccess_file: $!");
  }

  $ftpaccess_file = File::Spec->rel2abs("$baz_dir/.ftpaccess");
  if (open(my $fh, "> $ftpaccess_file")) {
    print $fh <<EOL;
<Limit ALL>
  AllowUser bar,$user
  DenyAll
</Limit>
EOL
    unless (close($fh)) {
      die("Can't write $ftpaccess_file: $!");
    }

  } else {
    die("Can't open $ftpaccess_file: $!");
  }

  $ftpaccess_file = File::Spec->rel2abs("$quxx_dir/.ftpaccess");
  if (open(my $fh, "> $ftpaccess_file")) {
    print $fh <<EOL;
<Limit ALL>
  AllowUser bar,$user,
  DenyAll
</Limit>
EOL
    unless (close($fh)) {
      die("Can't write $ftpaccess_file: $!");
    }

  } else {
    die("Can't open $ftpaccess_file: $!");
  }

  # Make sure that, if we're running as root, that the home directory has
  # permissions/privs set for the account we create
  if ($< == 0) {
    unless (chmod(0755, $home_dir, $foo_dir, $bar_dir, $baz_dir, $quxx_dir)) {
      die("Can't set perms on dirs to 0755: $!");
    }

    unless (chown($uid, $gid, $home_dir, $foo_dir, $bar_dir, $baz_dir, $quxx_dir)) {
      die("Can't set owner of dirs to $uid/$gid: $!");
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

    AllowOverride => 'on',
    DefaultRoot => '~',
    ShowSymlinks => 'off',

    IfModules => {
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

      $client->login($user, $passwd);

      $client->cwd("foo/baz");
      my $conn = $client->list_raw();
      unless ($conn) {
        die("Failed to LIST: " . $client->response_code() . " " .
          $client->response_msg());
      }

      my $buf;
      $conn->read($buf, 8192);
      $conn->close();

      my $resp_code = $client->response_code();
      my $resp_msg = $client->response_msg();

      my $expected;

      $expected = 226;
      $self->assert($expected == $resp_code,
        test_msg("Expected $expected, got $resp_code"));

      $expected = 'Transfer complete';
      $self->assert($expected eq $resp_msg,
        test_msg("Expected '$expected', got '$resp_msg'"));

      $client->quote('CWD', '..');

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

sub list_showsymlinks_on {
  my $self = shift;
  my $tmpdir = $self->{tmpdir};

  my $config_file = "$tmpdir/cmds.conf";
  my $pid_file = File::Spec->rel2abs("$tmpdir/cmds.pid");
  my $scoreboard_file = File::Spec->rel2abs("$tmpdir/cmds.scoreboard");

  my $log_file = File::Spec->rel2abs('tests.log');

  my $auth_user_file = File::Spec->rel2abs("$tmpdir/cmds.passwd");
  my $auth_group_file = File::Spec->rel2abs("$tmpdir/cmds.group");

  my $user = 'proftpd';
  my $passwd = 'test';
  my $home_dir = File::Spec->rel2abs($tmpdir);
  my $uid = 500;
  my $gid = 500;

  my $foo_dir = File::Spec->rel2abs("$tmpdir/foo");
  my $bar_dir = File::Spec->rel2abs("$tmpdir/bar");
  my $baz_dir = File::Spec->rel2abs("$tmpdir/bar/baz");
  mkpath([$foo_dir, $bar_dir, $baz_dir]);

  # Change to the 'foo' directory in order to create a relative path in the
  # symlink we need

  my $cwd = getcwd();
  unless (chdir("$foo_dir")) {
    die("Can't chdir to $foo_dir: $!");
  }

  unless (symlink('../bar/baz', 'baz')) {
    die("Can't symlink '../bar/baz' to 'baz': $!");
  }

  unless (chdir($cwd)) {
    die("Can't chdir to $cwd: $!");
  }

  # Make sure that, if we're running as root, that the home directory has
  # permissions/privs set for the account we create
  if ($< == 0) {
    unless (chmod(0755, $home_dir, $foo_dir, $bar_dir, $baz_dir)) {
      die("Can't set perms on dirs to 0755: $!");
    }

    unless (chown($uid, $gid, $home_dir, $foo_dir, $bar_dir, $baz_dir)) {
      die("Can't set owner of dirs to $uid/$gid: $!");
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
    ShowSymlinks => 'on',

    IfModules => {
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

      $client->login($user, $passwd);

      $client->cwd("foo");
      my $conn = $client->list_raw('-l');
      unless ($conn) {
        die("Failed to LIST: " . $client->response_code() . " " .
          $client->response_msg());
      }

      my $buf;
      $conn->read($buf, 8192);
      $conn->close();

      my $resp_code = $client->response_code();
      my $resp_msg = $client->response_msg();

      my $expected;

      $expected = 226;
      $self->assert($expected == $resp_code,
        test_msg("Expected $expected, got $resp_code"));

      $expected = 'Transfer complete';
      $self->assert($expected eq $resp_msg,
        test_msg("Expected '$expected', got '$resp_msg'"));

      chomp($buf);

      $expected = '^l\S+\s+\d+\s+\S+\s+\S+\s+\d+\s+\S+\s+\d+\s+\S+\s+(.*?)$';
      $self->assert(qr/$expected/, $buf,
        test_msg("Expected '$expected', got '$buf'"));

      $buf =~ /$expected/;
      my $finfo = $1;

      $expected = 'baz -> ../bar/baz';
      $self->assert($expected eq $finfo,
        test_msg("Expected '$expected', got '$finfo'"));

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

sub list_glob_bug2367 {
  my $self = shift;
  my $tmpdir = $self->{tmpdir};

  my $config_file = "$tmpdir/cmds.conf";
  my $pid_file = File::Spec->rel2abs("$tmpdir/cmds.pid");
  my $scoreboard_file = File::Spec->rel2abs("$tmpdir/cmds.scoreboard");

  my $log_file = File::Spec->rel2abs('tests.log');

  my $auth_user_file = File::Spec->rel2abs("$tmpdir/cmds.passwd");
  my $auth_group_file = File::Spec->rel2abs("$tmpdir/cmds.group");

  my $user = 'proftpd';
  my $passwd = 'test';
  my $home_dir = File::Spec->rel2abs($tmpdir);
  my $uid = 500;
  my $gid = 500;

  my $test_dir = File::Spec->rel2abs("$home_dir/foo");
  mkpath($test_dir);

  # Make sure that, if we're running as root, that the home directory has
  # permissions/privs set for the account we create
  if ($< == 0) {
    unless (chmod(0755, $home_dir, $test_dir)) {
      die("Can't set perms on $home_dir to 0755: $!");
    }

    unless (chown($uid, $gid, $home_dir, $test_dir)) {
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

    IfModules => {
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

      $client->login($user, $passwd);
      $client->cwd($test_dir);

      my $conn = $client->list_raw("*.*");
      unless ($conn) {
        die("LIST failed: " . $client->response_code() . " " .
          $client->response_msg());
      }

      my $buf;
      $conn->read($buf, 8192);
      $conn->close();

      my $res = {};
      my $lines = [split(/\n/, $buf)];
      foreach my $line (@$lines) {
        if ($line =~ /^\S+\s+\d+\s+\S+\s+\S+\s+.*?\s+(\S+)$/) {
          $res->{$1} = 1;
        }
      }

      my $count = scalar(keys(%$res));
      unless ($count == 0) {
        die("LIST returned wrong number of entries (expected 0, got $count)");
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

sub list_glob_legit_dir_name {
  my $self = shift;
  my $tmpdir = $self->{tmpdir};

  my $config_file = "$tmpdir/cmds.conf";
  my $pid_file = File::Spec->rel2abs("$tmpdir/cmds.pid");
  my $scoreboard_file = File::Spec->rel2abs("$tmpdir/cmds.scoreboard");

  my $log_file = File::Spec->rel2abs('tests.log');

  my $auth_user_file = File::Spec->rel2abs("$tmpdir/cmds.passwd");
  my $auth_group_file = File::Spec->rel2abs("$tmpdir/cmds.group");

  my $user = 'proftpd';
  my $passwd = 'test';
  my $home_dir = File::Spec->rel2abs($tmpdir);
  my $uid = 500;
  my $gid = 500;

  my $test_dir = File::Spec->rel2abs("$home_dir/\[foo bar");
  mkpath($test_dir);

  # Make sure that, if we're running as root, that the home directory has
  # permissions/privs set for the account we create
  if ($< == 0) {
    unless (chmod(0755, $home_dir, $test_dir)) {
      die("Can't set perms on $home_dir to 0755: $!");
    }

    unless (chown($uid, $gid, $home_dir, $test_dir)) {
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

    IfModules => {
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

      $client->login($user, $passwd);
      $client->cwd($test_dir);

      my $conn = $client->list_raw($test_dir);
      unless ($conn) {
        die("LIST failed: " . $client->response_code() . " " .
          $client->response_msg());
      }

      my $buf;
      $conn->read($buf, 8192);
      $conn->close();

      my $res = {};
      my $lines = [split(/\n/, $buf)];
      foreach my $line (@$lines) {
        if ($line =~ /^\S+\s+\d+\s+\S+\s+\S+\s+.*?\s+(\S+)$/) {
          $res->{$1} = 1;
        }
      }

      my $count = scalar(keys(%$res));
      unless ($count == 0) {
        die("LIST returned wrong number of entries (expected 0, got $count)");
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
