package ProFTPD::Tests::Config::Directory::Limits;

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

# A fun collection of various <Directory> and <Limit> configurations

my $TESTS = {
  limits_with_glob_then_nonglob_dirs_for_same_path => {
    order => ++$order,
    test_class => [qw(forking)],
  },

  limits_with_nonglob_then_glob_dirs_for_same_path => {
    order => ++$order,
    test_class => [qw(forking)],
  },

  limits_with_glob_denied_delete_bug3146 => {
    order => ++$order,
    test_class => [qw(bug forking)],
  },

  limits_without_glob_denied_delete_bug3146 => {
    order => ++$order,
    test_class => [qw(bug forking)],
  },

  limits_commands_comma_space_delimited_deferred_paths_bug3147 => {
    order => ++$order,
    test_class => [qw(bug forking)],
  },

  limits_commands_comma_delimited_deferred_paths_bug3147 => {
    order => ++$order,
    test_class => [qw(bug forking)],
  },

  limits_commands_no_commas_deferred_paths_bug3147 => {
    order => ++$order,
    test_class => [qw(bug forking)],
  },

  limits_rename_dir_ok_write_denied => {
    order => ++$order,
    test_class => [qw(forking)],
  },

  limits_rename_dir_failed_rnfr_denied => {
    order => ++$order,
    test_class => [qw(forking)],
  },

  limits_one_char_dir_bug3337 => {
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

sub limits_with_glob_then_nonglob_dirs_for_same_path {
  my $self = shift;
  my $tmpdir = $self->{tmpdir};

  my $config_file = "$tmpdir/dir.conf";
  my $pid_file = File::Spec->rel2abs("$tmpdir/dir.pid");
  my $scoreboard_file = File::Spec->rel2abs("$tmpdir/dir.scoreboard");

  my $log_file = File::Spec->rel2abs('tests.log');

  my $auth_user_file = File::Spec->rel2abs("$tmpdir/dir.passwd");
  my $auth_group_file = File::Spec->rel2abs("$tmpdir/dir.group");

  my $user = 'proftpd';
  my $passwd = 'test';
  my $home_dir = File::Spec->rel2abs("$tmpdir/home/users/$user");
  my $uid = 500;
  my $gid = 500;

  mkpath($home_dir);

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

  my $abs_tmp_dir = File::Spec->rel2abs($tmpdir);

  my $config = {
    PidFile => $pid_file,
    ScoreboardFile => $scoreboard_file,
    SystemLog => $log_file,
    TraceLog => $log_file,
    Trace => 'DEFAULT:10 fsio:0 directory:10 lock:0',

    AuthUserFile => $auth_user_file,
    AuthGroupFile => $auth_group_file,

    DefaultChdir => '~',

    IfModules => {
      'mod_delay.c' => {
        DelayEngine => 'off',
      },
    },
  };

  my ($port, $config_user, $config_group) = config_write($config_file, $config);

  # In order to ensure that the <Directory> sections appear in the desired
  # order in the config file, we write them out here.  The config_write()
  # function, whilst useful, uses hashrefs, which means the ordering of the
  # keys is not necessarily predictable.
  if (open(my $fh, ">> $config_file")) {
    print $fh <<EOD;
<Directory $abs_tmp_dir/home/*/proftpd>
  <Limit ALL>
    DenyAll
  </Limit>
</Directory>
<Directory $abs_tmp_dir/home/users/proftpd>
  <Limit ALL>
    AllowAll
  </Limit>
</Directory>
EOD
    close($fh);

  } else {
    die("Can't read $config_file: $!");
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
      $client->login($user, $passwd);

      my ($resp_code, $resp_msg);

      my $conn = $client->stor_raw('test.txt');
      unless ($conn) {
        die("Failed to STOR test.txt: " . $client->response_code() . " " .
          $client->response_msg());
      };

      my $buf = "Hello, World\n";
      $conn->write($buf, length($buf));
      $conn->close();

      $resp_code = $client->response_code();
      $resp_msg = $client->response_msg();

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

sub limits_with_nonglob_then_glob_dirs_for_same_path {
  my $self = shift;
  my $tmpdir = $self->{tmpdir};

  my $config_file = "$tmpdir/dir.conf";
  my $pid_file = File::Spec->rel2abs("$tmpdir/dir.pid");
  my $scoreboard_file = File::Spec->rel2abs("$tmpdir/dir.scoreboard");

  my $log_file = File::Spec->rel2abs('tests.log');

  my $auth_user_file = File::Spec->rel2abs("$tmpdir/dir.passwd");
  my $auth_group_file = File::Spec->rel2abs("$tmpdir/dir.group");

  my $user = 'proftpd';
  my $passwd = 'test';
  my $home_dir = File::Spec->rel2abs("$tmpdir/home/users/$user");
  my $uid = 500;
  my $gid = 500;

  mkpath($home_dir);

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

  my $abs_tmp_dir = File::Spec->rel2abs($tmpdir);

  my $config = {
    PidFile => $pid_file,
    ScoreboardFile => $scoreboard_file,
    SystemLog => $log_file,
    TraceLog => $log_file,
    Trace => 'DEFAULT:10 fsio:0 directory:10 lock:0',

    AuthUserFile => $auth_user_file,
    AuthGroupFile => $auth_group_file,

    DefaultChdir => '~',

    IfModules => {
      'mod_delay.c' => {
        DelayEngine => 'off',
      },
    },
  };

  my ($port, $config_user, $config_group) = config_write($config_file, $config);

  # In order to ensure that the <Directory> sections appear in the desired
  # order in the config file, we write them out here.  The config_write()
  # function, whilst useful, uses hashrefs, which means the ordering of the
  # keys is not necessarily predictable.
  if (open(my $fh, ">> $config_file")) {
    print $fh <<EOD;
<Directory $abs_tmp_dir/home/users/proftpd>
  <Limit ALL>
    AllowAll
  </Limit>
</Directory>
<Directory $abs_tmp_dir/home/*/proftpd>
  <Limit ALL>
    DenyAll
  </Limit>
</Directory>
EOD
    close($fh);

  } else {
    die("Can't read $config_file: $!");
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
      $client->login($user, $passwd);

      my ($resp_code, $resp_msg);

      my $conn = $client->stor_raw('test.txt');
      if ($conn) {
        die("STOR test.txt succeeded unexpectedly");
      };

      $resp_code = $client->response_code();
      $resp_msg = $client->response_msg();

      my $expected;

      $expected = 550;
      $self->assert($expected == $resp_code,
        test_msg("Expected $expected, got $resp_code"));

      $expected = "test.txt: Operation not permitted";
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

sub limits_with_glob_denied_delete_bug3146 {
  my $self = shift;
  my $tmpdir = $self->{tmpdir};

  # This config is from:
  #
  #  http://forums.proftpd.org/smf/index.php/topic,3491.0
  #  http://bugs.proftpd.org/show_bug.cgi?id=3146

  my $config_file = "$tmpdir/dir.conf";
  my $pid_file = File::Spec->rel2abs("$tmpdir/dir.pid");
  my $scoreboard_file = File::Spec->rel2abs("$tmpdir/dir.scoreboard");

  my $log_file = File::Spec->rel2abs('tests.log');

  my $auth_user_file = File::Spec->rel2abs("$tmpdir/dir.passwd");
  my $auth_group_file = File::Spec->rel2abs("$tmpdir/dir.group");

  my $user = 'proftpd';
  my $passwd = 'test';
  my $home_dir = File::Spec->rel2abs("$tmpdir/home/users/$user");
  my $uid = 500;
  my $gid = 500;

  mkpath($home_dir);

  my $test_file = File::Spec->rel2abs("$home_dir/test.txt");
  if (open(my $fh, "> $test_file")) {
    print $fh "Hello, World!\n";

    unless (close($fh)) {
      die("Can't write $test_file: $!");
    }

  } else {
    die("Can't open $test_file: $!");
  }
 
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

  my $abs_tmp_dir = File::Spec->rel2abs($tmpdir);

  my $config = {
    PidFile => $pid_file,
    ScoreboardFile => $scoreboard_file,
    SystemLog => $log_file,
    TraceLog => $log_file,
    Trace => 'DEFAULT:10 fsio:0 directory:10 lock:0',

    AuthUserFile => $auth_user_file,
    AuthGroupFile => $auth_group_file,

    DefaultChdir => '~',

    Directory => {
      "$abs_tmp_dir/home/*/proftpd" => {
        Limit => {
          'ALL' => {
            DenyAll => '',
          },

          'CDUP CWD XCWD XCUP LIST NLST' => {
            AllowAll => '',
          },

          'STOR STOU' => {
            AllowAll => '',
          },
        },
      },
    },

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
      eval { $client->dele('test.txt') };
      unless ($@) {
        die("DELE succeeded unexpectedly");

      } else {
        $resp_code = $client->response_code();
        $resp_msg = $client->response_msg();
      }

      my $expected;

      $expected = 550;
      $self->assert($expected == $resp_code,
        test_msg("Expected $expected, got $resp_code"));

      $expected = "test.txt: Operation not permitted";
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

sub limits_without_glob_denied_delete_bug3146 {
  my $self = shift;
  my $tmpdir = $self->{tmpdir};

  # This config is from:
  #
  #  http://forums.proftpd.org/smf/index.php/topic,3491.0
  #
  # only without the glob pattern, to check for regressions.

  my $config_file = "$tmpdir/dir.conf";
  my $pid_file = File::Spec->rel2abs("$tmpdir/dir.pid");
  my $scoreboard_file = File::Spec->rel2abs("$tmpdir/dir.scoreboard");

  my $log_file = File::Spec->rel2abs('tests.log');

  my $auth_user_file = File::Spec->rel2abs("$tmpdir/dir.passwd");
  my $auth_group_file = File::Spec->rel2abs("$tmpdir/dir.group");

  my $user = 'proftpd';
  my $passwd = 'test';
  my $home_dir = File::Spec->rel2abs("$tmpdir/home/users/$user");
  my $uid = 500;
  my $gid = 500;

  mkpath($home_dir);

  my $test_file = File::Spec->rel2abs("$home_dir/test.txt");
  if (open(my $fh, "> $test_file")) {
    print $fh "Hello, World!\n";

    unless (close($fh)) {
      die("Can't write $test_file: $!");
    }

  } else {
    die("Can't open $test_file: $!");
  }
 
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

  my $abs_tmp_dir = File::Spec->rel2abs($tmpdir);

  my $config = {
    PidFile => $pid_file,
    ScoreboardFile => $scoreboard_file,
    SystemLog => $log_file,
    TraceLog => $log_file,
    Trace => 'DEFAULT:10 fsio:0 directory:10 lock:0',

    AuthUserFile => $auth_user_file,
    AuthGroupFile => $auth_group_file,

    DefaultChdir => '~',

    Directory => {
      "$abs_tmp_dir/home/users/proftpd" => {
        Limit => {
          'ALL' => {
            DenyAll => '',
          },

          'CDUP CWD XCWD XCUP LIST NLST' => {
            AllowAll => '',
          },

          'STOR STOU' => {
            AllowAll => '',
          },
        },
      },
    },

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
      eval { $client->dele('test.txt') };
      unless ($@) {
        die("DELE succeeded unexpectedly");

      } else {
        $resp_code = $client->response_code();
        $resp_msg = $client->response_msg();
      }

      my $expected;

      $expected = 550;
      $self->assert($expected == $resp_code,
        test_msg("Expected $expected, got $resp_code"));

      $expected = "test.txt: Operation not permitted";
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

sub limits_commands_comma_space_delimited_deferred_paths_bug3147 {
  my $self = shift;
  my $tmpdir = $self->{tmpdir};

  # This config is from:
  #
  #  http://forums.proftpd.org/smf/index.php/topic,3648.0
  #  http://bugs.proftpd.org/show_bug.cgi?id=3147

  my $config_file = "$tmpdir/dir.conf";
  my $pid_file = File::Spec->rel2abs("$tmpdir/dir.pid");
  my $scoreboard_file = File::Spec->rel2abs("$tmpdir/dir.scoreboard");

  my $log_file = File::Spec->rel2abs('tests.log');

  my $auth_user_file = File::Spec->rel2abs("$tmpdir/dir.passwd");
  my $auth_group_file = File::Spec->rel2abs("$tmpdir/dir.group");

  my $user = 'proftpd';
  my $passwd = 'test';
  my $home_dir = File::Spec->rel2abs("$tmpdir/home/$user");
  my $uid = 500;
  my $gid = 500;

  mkpath($home_dir);

  my $sub_dir = File::Spec->rel2abs("$tmpdir/home/$user/upload");
  mkpath($sub_dir);

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
    TraceLog => $log_file,
    Trace => 'DEFAULT:10 fsio:0 directory:10 lock:0',

    AuthUserFile => $auth_user_file,
    AuthGroupFile => $auth_group_file,

    DefaultChdir => '~',

    Directory => {
      "~$user" => {
        Limit => {
          'WRITE' => {
            DenyAll => '',
          },
        },
      },

      # Bug#3147 happened because proftpd's parsing of the list of commands
      # in the <Limit> section was not properly handling commas after the
      # command names.

      "~$user/upload" => {
        Limit => {
          'STOR, APPE, MKD, RMD, RNTO, STOU, XMKD, XRMD' => {
            AllowAll => '',
          },
        },
      },
    },

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

      my $conn = $client->stor_raw('upload/test.txt');
      unless ($conn) {
        die("Failed to STOR upload/test.txt: " . $client->response_code() .
          " " . $client->response_msg());
      }

      my $buf = "Hello, World!\n";
      $conn->write($buf, length($buf));
      $conn->close();

      $resp_code = $client->response_code();
      $resp_msg = $client->response_msg();

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

sub limits_commands_comma_delimited_deferred_paths_bug3147 {
  my $self = shift;
  my $tmpdir = $self->{tmpdir};

  # This config is from:
  #
  #  http://forums.proftpd.org/smf/index.php/topic,3648.0
  #  http://bugs.proftpd.org/show_bug.cgi?id=3147

  my $config_file = "$tmpdir/dir.conf";
  my $pid_file = File::Spec->rel2abs("$tmpdir/dir.pid");
  my $scoreboard_file = File::Spec->rel2abs("$tmpdir/dir.scoreboard");

  my $log_file = File::Spec->rel2abs('tests.log');

  my $auth_user_file = File::Spec->rel2abs("$tmpdir/dir.passwd");
  my $auth_group_file = File::Spec->rel2abs("$tmpdir/dir.group");

  my $user = 'proftpd';
  my $passwd = 'test';
  my $home_dir = File::Spec->rel2abs("$tmpdir/home/$user");
  my $uid = 500;
  my $gid = 500;

  mkpath($home_dir);

  my $sub_dir = File::Spec->rel2abs("$tmpdir/home/$user/upload");
  mkpath($sub_dir);

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
    TraceLog => $log_file,
    Trace => 'DEFAULT:10 fsio:0 directory:10 lock:0',

    AuthUserFile => $auth_user_file,
    AuthGroupFile => $auth_group_file,

    DefaultChdir => '~',

    Directory => {
      "~$user" => {
        Limit => {
          'WRITE' => {
            DenyAll => '',
          },
        },
      },

      # Bug#3147 happened because proftpd's parsing of the list of commands
      # in the <Limit> section was not properly handling commas after the
      # command names.

      "~$user/upload" => {
        Limit => {
          'STOR,APPE,MKD,RMD,RNTO,STOU,XMKD,XRMD' => {
            AllowAll => '',
          },
        },
      },
    },

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

      my $conn = $client->stor_raw('upload/test.txt');
      unless ($conn) {
        die("Failed to STOR upload/test.txt: " . $client->response_code() .
          " " . $client->response_msg());
      }

      my $buf = "Hello, World!\n";
      $conn->write($buf, length($buf));
      $conn->close();

      $resp_code = $client->response_code();
      $resp_msg = $client->response_msg();

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

sub limits_commands_no_commas_deferred_paths_bug3147 {
  my $self = shift;
  my $tmpdir = $self->{tmpdir};

  # This config is from:
  #
  #  http://forums.proftpd.org/smf/index.php/topic,3648.0
  #  http://bugs.proftpd.org/show_bug.cgi?id=3147

  my $config_file = "$tmpdir/dir.conf";
  my $pid_file = File::Spec->rel2abs("$tmpdir/dir.pid");
  my $scoreboard_file = File::Spec->rel2abs("$tmpdir/dir.scoreboard");

  my $log_file = File::Spec->rel2abs('tests.log');

  my $auth_user_file = File::Spec->rel2abs("$tmpdir/dir.passwd");
  my $auth_group_file = File::Spec->rel2abs("$tmpdir/dir.group");

  my $user = 'proftpd';
  my $passwd = 'test';
  my $home_dir = File::Spec->rel2abs("$tmpdir/home/$user");
  my $uid = 500;
  my $gid = 500;

  mkpath($home_dir);

  my $sub_dir = File::Spec->rel2abs("$tmpdir/home/$user/upload");
  mkpath($sub_dir);

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
    TraceLog => $log_file,
    Trace => 'DEFAULT:10 fsio:0 directory:10 lock:0',

    AuthUserFile => $auth_user_file,
    AuthGroupFile => $auth_group_file,

    DefaultChdir => '~',

    Directory => {
      "~$user" => {
        Limit => {
          'WRITE' => {
            DenyAll => '',
          },
        },
      },

      # Bug#3147 happened because proftpd's parsing of the list of commands
      # in the <Limit> section was not properly handling commas after the
      # command names.  Make sure the fix for Bug#3147 doesn't break command
      # lists which DO NOT use commas.

      "~$user/upload" => {
        Limit => {
          'STOR APPE MKD RMD RNTO STOU XMKD XRMD' => {
            AllowAll => '',
          },
        },
      },
    },

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

      my $conn = $client->stor_raw('upload/test.txt');
      unless ($conn) {
        die("Failed to STOR upload/test.txt: " . $client->response_code() .
          " " . $client->response_msg());
      }

      my $buf = "Hello, World!\n";
      $conn->write($buf, length($buf));
      $conn->close();

      $resp_code = $client->response_code();
      $resp_msg = $client->response_msg();

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

sub limits_rename_dir_ok_write_denied {
  my $self = shift;
  my $tmpdir = $self->{tmpdir};

  my $config_file = "$tmpdir/dir.conf";
  my $pid_file = File::Spec->rel2abs("$tmpdir/dir.pid");
  my $scoreboard_file = File::Spec->rel2abs("$tmpdir/dir.scoreboard");

  my $log_file = File::Spec->rel2abs('tests.log');

  my $auth_user_file = File::Spec->rel2abs("$tmpdir/dir.passwd");
  my $auth_group_file = File::Spec->rel2abs("$tmpdir/dir.group");

  my $user = 'proftpd';
  my $passwd = 'test';
  my $home_dir = File::Spec->rel2abs("$tmpdir/home/$user");
  my $uid = 500;
  my $gid = 500;

  mkpath($home_dir);

  my $sub_dir = File::Spec->rel2abs("$tmpdir/home/$user/upload");
  mkpath($sub_dir);

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
    TraceLog => $log_file,
    Trace => 'DEFAULT:10 fsio:0 directory:10 lock:0',

    AuthUserFile => $auth_user_file,
    AuthGroupFile => $auth_group_file,

    DefaultChdir => '~',

    Directory => {
      "~$user/*" => {
        Limit => {
          'CWD XCWD RNFR RNTO' => {
            AllowAll => '',
          },

          'ALL' => {
            DenyAll => '',
          },
        },
      },

      "~$user/upload" => {
        Limit => {
          'WRITE' => {
            DenyAll => '',
          },
        },
      },
    },

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

      ($resp_code, $resp_msg) = $client->rnfr('upload');

      my $expected;

      $expected = 350;
      $self->assert($expected == $resp_code,
        test_msg("Expected $expected, got $resp_code"));

      $expected = "File or directory exists, ready for destination name";
      $self->assert($expected eq $resp_msg,
        test_msg("Expected '$expected', got '$resp_msg'"));

      ($resp_code, $resp_msg) = $client->rnto('upload2');

      $expected = 250;
      $self->assert($expected == $resp_code,
        test_msg("Expected $expected, got $resp_code"));

      $expected = "Rename successful";
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

sub limits_rename_dir_failed_rnfr_denied {
  my $self = shift;
  my $tmpdir = $self->{tmpdir};

  my $config_file = "$tmpdir/dir.conf";
  my $pid_file = File::Spec->rel2abs("$tmpdir/dir.pid");
  my $scoreboard_file = File::Spec->rel2abs("$tmpdir/dir.scoreboard");

  my $log_file = File::Spec->rel2abs('tests.log');

  my $auth_user_file = File::Spec->rel2abs("$tmpdir/dir.passwd");
  my $auth_group_file = File::Spec->rel2abs("$tmpdir/dir.group");

  my $user = 'proftpd';
  my $passwd = 'test';
  my $home_dir = File::Spec->rel2abs("$tmpdir/home/$user");
  my $uid = 500;
  my $gid = 500;

  mkpath($home_dir);

  my $sub_dir = File::Spec->rel2abs("$tmpdir/home/$user/upload");
  mkpath($sub_dir);

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
    TraceLog => $log_file,
    Trace => 'DEFAULT:10 fsio:0 directory:10 lock:0',

    AuthUserFile => $auth_user_file,
    AuthGroupFile => $auth_group_file,

    DefaultChdir => '~',

    Directory => {
      "~$user/*" => {
        Limit => {
          'CWD XCWD RNFR RNTO' => {
            AllowAll => '',
          },

          'ALL' => {
            DenyAll => '',
          },
        },
      },

      "~$user/upload" => {
        Limit => {
          'RNFR' => {
            DenyAll => '',
          },
        },
      },
    },

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

      eval { $client->rnfr('upload') };
      unless ($@) {
        die("RNFR upload succeeded unexpectedly");

      } else {
        $resp_code = $client->response_code();
        $resp_msg = $client->response_msg();
      }

      my $expected;

      $expected = 550;
      $self->assert($expected == $resp_code,
        test_msg("Expected $expected, got $resp_code"));

      $expected = "upload: Operation not permitted";
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

sub limits_one_char_dir_bug3337 {
  my $self = shift;
  my $tmpdir = $self->{tmpdir};

  my $config_file = "$tmpdir/dir.conf";
  my $pid_file = File::Spec->rel2abs("$tmpdir/dir.pid");
  my $scoreboard_file = File::Spec->rel2abs("$tmpdir/dir.scoreboard");

  my $log_file = File::Spec->rel2abs('tests.log');

  my $auth_user_file = File::Spec->rel2abs("$tmpdir/dir.passwd");
  my $auth_group_file = File::Spec->rel2abs("$tmpdir/dir.group");

  my $user = 'proftpd';
  my $passwd = 'test';
  my $home_dir = File::Spec->rel2abs($tmpdir);
  my $uid = 500;
  my $gid = 500;

  my $a_dir = File::Spec->rel2abs("$home_dir/a");
  mkpath($a_dir);

  my $ab_dir = File::Spec->rel2abs("$home_dir/ab");
  mkpath($ab_dir);

  # Make sure that, if we're running as root, that the home directory has
  # permissions/privs set for the account we create
  if ($< == 0) {
    unless (chmod(0755, $home_dir, $a_dir, $ab_dir)) {
      die("Can't set perms on $home_dir to 0755: $!");
    }

    unless (chown($uid, $gid, $home_dir, $a_dir, $ab_dir)) {
      die("Can't set owner of $home_dir to $uid/$gid: $!");
    }
  }
 
  auth_user_write($auth_user_file, $user, $passwd, $uid, $gid, $home_dir,
    '/bin/bash');
  auth_group_write($auth_group_file, 'ftpd', $gid, $user);

  # Reported in
  #
  #  http://forums.proftpd.org/smf/index.php/topic,4170.0.html

  my $config = {
    PidFile => $pid_file,
    ScoreboardFile => $scoreboard_file,
    SystemLog => $log_file,
    TraceLog => $log_file,
    Trace => 'DEFAULT:10 fsio:0 directory:10 lock:0',

    AuthUserFile => $auth_user_file,
    AuthGroupFile => $auth_group_file,
    DefaultChdir => '~',

    Directory => {
      "$home_dir" => {
        Limit => {
          WRITE => {
            DenyUser => $user,
          },
        },
      },

      "$a_dir" => {
        Limit => {
          WRITE => {
            AllowUser => $user,
          },
        },
      },

      "$ab_dir" => {
        Limit => {
          WRITE => {
            AllowUser => $user,
          },
        },
      },
    },

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

      $client->cwd('ab');
      my $conn = $client->stor_raw('test.txt');
      unless ($conn) {
        die("STOR test.txt failed: " . $client->response_code() . " " .
          $client->response_msg());
      }

      $conn->close();

      my $resp_code = $client->response_code();
      my $resp_msg = $client->response_msg();

      my $expected;

      $expected = 226;
      $self->assert($expected == $resp_code,
        test_msg("Expected $expected, got $resp_code"));

      $expected = "Transfer complete";
      $self->assert($expected eq $resp_msg,
        test_msg("Expected '$expected', got '$resp_msg'"));

      $client->cwd('../a');
      $conn = $client->stor_raw('test.txt');
      unless ($conn) {
        die("STOR test.txt failed: " . $client->response_code() . " " .
          $client->response_msg());
      }

      $conn->close();

      $resp_code = $client->response_code();
      $resp_msg = $client->response_msg();

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

1;
