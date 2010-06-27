package ProFTPD::Tests::Modules::mod_exec;

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
  exec_on_connect => {
    order => ++$order,
    test_class => [qw(forking)],
  },

  exec_on_cmd => {
    order => ++$order,
    test_class => [qw(forking)],
  },

  exec_on_cmd_var_total_bytes_xfer => {
    order => ++$order,
    test_class => [qw(forking)],
  },

  exec_on_cmd_var_bytes_xfer => {
    order => ++$order,
    test_class => [qw(forking)],
  },

  exec_on_exit => {
    order => ++$order,
    test_class => [qw(forking)],
  },

  exec_on_error => {
    order => ++$order,
    test_class => [qw(forking)],
  },

  exec_on_restart => {
    order => ++$order,
    test_class => [qw(forking)],
  },

  exec_opt_log_stdout => {
    order => ++$order,
    test_class => [qw(forking)],
  },

  exec_opt_log_stderr => {
    order => ++$order,
    test_class => [qw(forking)],
  },

  exec_opt_send_stdout => {
    order => ++$order,
    test_class => [qw(forking)],
  },

  exec_opt_use_stdin => {
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

sub exec_on_connect {
  my $self = shift;
  my $tmpdir = $self->{tmpdir};

  my $config_file = "$tmpdir/exec.conf";
  my $pid_file = File::Spec->rel2abs("$tmpdir/exec.pid");
  my $scoreboard_file = File::Spec->rel2abs("$tmpdir/exec.scoreboard");

  my $log_file = File::Spec->rel2abs('tests.log');

  my $auth_user_file = File::Spec->rel2abs("$tmpdir/exec.passwd");
  my $auth_group_file = File::Spec->rel2abs("$tmpdir/exec.group");

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

  my $connect_file = File::Spec->rel2abs("$tmpdir/connect.txt");

  my $config = {
    PidFile => $pid_file,
    ScoreboardFile => $scoreboard_file,
    SystemLog => $log_file,

    AuthUserFile => $auth_user_file,
    AuthGroupFile => $auth_group_file,

    IfModules => {
      'mod_exec.c' => {
        ExecEngine => 'on',
        ExecLog => $log_file,
        ExecTimeout => 1,
        ExecOnConnect => "/bin/bash -c \"echo %a > $connect_file\"",
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
      $client->login($user, $passwd);
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

  if (open(my $fh, "< $connect_file")) {
    my $line = <$fh>;
    close($fh);

    chomp($line);

    my $expected = '127.0.0.1';

    $self->assert($expected eq $line,
      test_msg("Expected '$expected', got '$line'"));

  } else {
    die("Can't read $connect_file: $!");
  }

  unlink($log_file);
}

sub exec_on_cmd {
  my $self = shift;
  my $tmpdir = $self->{tmpdir};

  my $config_file = "$tmpdir/exec.conf";
  my $pid_file = File::Spec->rel2abs("$tmpdir/exec.pid");
  my $scoreboard_file = File::Spec->rel2abs("$tmpdir/exec.scoreboard");

  my $log_file = File::Spec->rel2abs('tests.log');

  my $auth_user_file = File::Spec->rel2abs("$tmpdir/exec.passwd");
  my $auth_group_file = File::Spec->rel2abs("$tmpdir/exec.group");

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

  my $cmd_file = File::Spec->rel2abs("$tmpdir/cmd.txt");

  my $config = {
    PidFile => $pid_file,
    ScoreboardFile => $scoreboard_file,
    SystemLog => $log_file,

    AuthUserFile => $auth_user_file,
    AuthGroupFile => $auth_group_file,

    IfModules => {
      'mod_exec.c' => {
        ExecEngine => 'on',
        ExecLog => $log_file,
        ExecTimeout => 1,
        ExecOnCommand => "LIST,NLST /bin/bash -c \"echo %a > $cmd_file\"",
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
      $client->login($user, $passwd);
      $client->list();
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

  if (open(my $fh, "< $cmd_file")) {
    my $line = <$fh>;
    close($fh);

    chomp($line);

    my $expected = '127.0.0.1';

    $self->assert($expected eq $line,
      test_msg("Expected '$expected', got '$line'"));

  } else {
    die("Can't read $cmd_file: $!");
  }

  unlink($log_file);
}

sub exec_on_cmd_var_total_bytes_xfer {
  my $self = shift;
  my $tmpdir = $self->{tmpdir};

  my $config_file = "$tmpdir/exec.conf";
  my $pid_file = File::Spec->rel2abs("$tmpdir/exec.pid");
  my $scoreboard_file = File::Spec->rel2abs("$tmpdir/exec.scoreboard");

  my $log_file = File::Spec->rel2abs('tests.log');

  my $auth_user_file = File::Spec->rel2abs("$tmpdir/exec.passwd");
  my $auth_group_file = File::Spec->rel2abs("$tmpdir/exec.group");

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

  my $cmd_file = File::Spec->rel2abs("$tmpdir/cmd.txt");

  my $config = {
    PidFile => $pid_file,
    ScoreboardFile => $scoreboard_file,
    SystemLog => $log_file,

    AuthUserFile => $auth_user_file,
    AuthGroupFile => $auth_group_file,

    IfModules => {
      'mod_exec.c' => {
        ExecEngine => 'on',
        ExecLog => $log_file,
        ExecTimeout => 1,
        ExecOnCommand => "LIST,NLST /bin/bash -c \"echo %{total_bytes_xfer} > $cmd_file\"",
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
      $client->login($user, $passwd);
      $client->list();
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

  if (open(my $fh, "< $cmd_file")) {
    my $line = <$fh>;
    close($fh);

    chomp($line);

    my $expected = '\d+';

    $self->assert(qr/$expected/, $line,
      test_msg("Expected '$expected', got '$line'"));

  } else {
    die("Can't read $cmd_file: $!");
  }

  unlink($log_file);
}

sub exec_on_cmd_var_bytes_xfer {
  my $self = shift;
  my $tmpdir = $self->{tmpdir};

  my $config_file = "$tmpdir/exec.conf";
  my $pid_file = File::Spec->rel2abs("$tmpdir/exec.pid");
  my $scoreboard_file = File::Spec->rel2abs("$tmpdir/exec.scoreboard");

  my $log_file = File::Spec->rel2abs('tests.log');

  my $auth_user_file = File::Spec->rel2abs("$tmpdir/exec.passwd");
  my $auth_group_file = File::Spec->rel2abs("$tmpdir/exec.group");

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

  my $test_file = File::Spec->rel2abs("$tmpdir/test.txt");
  if (open(my $fh, "> $test_file")) {
    print $fh "ABCD\n" x 128;

    unless (close($fh)) {
      die("Can't write $test_file: $!");
    }

  } else {
    die("Can't open $test_file: $!");
  }

  my $cmd_file = File::Spec->rel2abs("$tmpdir/cmd.txt");

  my $config = {
    PidFile => $pid_file,
    ScoreboardFile => $scoreboard_file,
    SystemLog => $log_file,

    AuthUserFile => $auth_user_file,
    AuthGroupFile => $auth_group_file,

    IfModules => {
      'mod_exec.c' => {
        ExecEngine => 'on',
        ExecLog => $log_file,
        ExecTimeout => 1,
        ExecOnCommand => "RETR /bin/bash -c \"echo %{bytes_xfer} > $cmd_file\"",
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
      $client->login($user, $passwd);
      
      my $conn = $client->retr_raw('test.txt');
      unless ($conn) {
        die("RETR test.txt failed: " . $client->response_code() . " " .
          $client->response_msg());
      }

      my $buf;
      while ($conn->read($buf, 8192)) {
      }
      $conn->close();

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

  if (open(my $fh, "< $cmd_file")) {
    my $line = <$fh>;
    close($fh);

    chomp($line);

    my $expected = '\d+';

    $self->assert(qr/$expected/, $line,
      test_msg("Expected '$expected', got '$line'"));

  } else {
    die("Can't read $cmd_file: $!");
  }

  unlink($log_file);
}

sub exec_on_exit {
  my $self = shift;
  my $tmpdir = $self->{tmpdir};

  my $config_file = "$tmpdir/exec.conf";
  my $pid_file = File::Spec->rel2abs("$tmpdir/exec.pid");
  my $scoreboard_file = File::Spec->rel2abs("$tmpdir/exec.scoreboard");

  my $log_file = File::Spec->rel2abs('tests.log');

  my $auth_user_file = File::Spec->rel2abs("$tmpdir/exec.passwd");
  my $auth_group_file = File::Spec->rel2abs("$tmpdir/exec.group");

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

  my $exit_file = File::Spec->rel2abs("$tmpdir/exit.txt");

  my $config = {
    PidFile => $pid_file,
    ScoreboardFile => $scoreboard_file,
    SystemLog => $log_file,

    AuthUserFile => $auth_user_file,
    AuthGroupFile => $auth_group_file,

    IfModules => {
      'mod_exec.c' => {
        ExecEngine => 'on',
        ExecLog => $log_file,
        ExecTimeout => 1,
        ExecOnExit => "/bin/bash -c \"echo %a > $exit_file\"",
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

  if (open(my $fh, "< $exit_file")) {
    my $line = <$fh>;
    close($fh);

    chomp($line);

    my $expected = '127.0.0.1';

    $self->assert($expected eq $line,
      test_msg("Expected '$expected', got '$line'"));

  } else {
    die("Can't read $exit_file: $!");
  }

  unlink($log_file);
}

sub exec_on_error {
  my $self = shift;
  my $tmpdir = $self->{tmpdir};

  my $config_file = "$tmpdir/exec.conf";
  my $pid_file = File::Spec->rel2abs("$tmpdir/exec.pid");
  my $scoreboard_file = File::Spec->rel2abs("$tmpdir/exec.scoreboard");

  my $log_file = File::Spec->rel2abs('tests.log');

  my $auth_user_file = File::Spec->rel2abs("$tmpdir/exec.passwd");
  my $auth_group_file = File::Spec->rel2abs("$tmpdir/exec.group");

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

  my $cmd_file = File::Spec->rel2abs("$tmpdir/cmd.txt");

  my $config = {
    PidFile => $pid_file,
    ScoreboardFile => $scoreboard_file,
    SystemLog => $log_file,

    AuthUserFile => $auth_user_file,
    AuthGroupFile => $auth_group_file,

    IfModules => {
      'mod_exec.c' => {
        ExecEngine => 'on',
        ExecLog => $log_file,
        ExecTimeout => 1,
        ExecOnError => "STOR /bin/bash -c \"echo %a > $cmd_file\"",
      },

      'mod_delay.c' => {
        DelayEngine => 'off',
      },
    },

    Limit => {
      'STOR' => {
        DenyAll => '',
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

      my $conn = $client->stor_raw('foo.txt');
      if ($conn) {
        die("STOR succeeded unexpectedly");
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

  if (open(my $fh, "< $cmd_file")) {
    my $line = <$fh>;
    close($fh);

    chomp($line);

    my $expected = '127.0.0.1';

    $self->assert($expected eq $line,
      test_msg("Expected '$expected', got '$line'"));

  } else {
    die("Can't read $cmd_file: $!");
  }

  unlink($log_file);
}

sub exec_restart {
  my $pid_file = shift;

  my $pid;
  if (open(my $fh, "< $pid_file")) {
    $pid = <$fh>;
    chomp($pid);
    close($fh);

  } else {
    die("Can't read $pid_file: $!");
  }

  my $cmd = "kill -HUP $pid";

  if ($ENV{TEST_VERBOSE}) {
    print STDERR "Restarting server: $cmd\n";
  }

  my @output = `$cmd`;
  if ($ENV{TEST_VERBOSE}) {
    print STDERR "Output: ", join('', @output), "\n";
  }
}

sub exec_on_restart {
  my $self = shift;
  my $tmpdir = $self->{tmpdir};

  my $config_file = "$tmpdir/exec.conf";
  my $pid_file = File::Spec->rel2abs("$tmpdir/exec.pid");
  my $scoreboard_file = File::Spec->rel2abs("$tmpdir/exec.scoreboard");

  my $log_file = File::Spec->rel2abs('tests.log');

  my $auth_user_file = File::Spec->rel2abs("$tmpdir/exec.passwd");
  my $auth_group_file = File::Spec->rel2abs("$tmpdir/exec.group");

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

  my $restart_file = File::Spec->rel2abs("$tmpdir/restart.txt");

  my $config = {
    PidFile => $pid_file,
    ScoreboardFile => $scoreboard_file,
    SystemLog => $log_file,
    TraceLog => $log_file,
    Trace => 'DEFAULT:10',

    AuthUserFile => $auth_user_file,
    AuthGroupFile => $auth_group_file,

    IfModules => {
      'mod_exec.c' => {
        ExecEngine => 'on',
        ExecLog => $log_file,
        ExecTimeout => 1,
        ExecOnRestart => "/bin/bash -c \"echo restarted > $restart_file\"",
      },

      'mod_delay.c' => {
        DelayEngine => 'off',
      },
    },

  };

  my ($port, $config_user, $config_group) = config_write($config_file, $config);

  # Start the server
  server_start($config_file);

  # Give it a second or two to start up, then send the SIGHUP signal.  Wait
  # for a couple seconds after, to let mod_exec do its thing.
  sleep(2);
  exec_restart($pid_file);
  sleep(2);

  # Stop server
  server_stop($pid_file);

  if (open(my $fh, "< $restart_file")) {
    my $line = <$fh>;
    close($fh);

    chomp($line);

    my $expected = 'restarted';

    $self->assert($expected eq $line,
      test_msg("Expected '$expected', got '$line'"));

  } else {
    die("Can't read $restart_file: $!");
  }

  unlink($log_file);
}

sub exec_opt_log_stdout {
  my $self = shift;
  my $tmpdir = $self->{tmpdir};

  my $config_file = "$tmpdir/exec.conf";
  my $pid_file = File::Spec->rel2abs("$tmpdir/exec.pid");
  my $scoreboard_file = File::Spec->rel2abs("$tmpdir/exec.scoreboard");

  my $log_file = File::Spec->rel2abs('tests.log');

  my $auth_user_file = File::Spec->rel2abs("$tmpdir/exec.passwd");
  my $auth_group_file = File::Spec->rel2abs("$tmpdir/exec.group");

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

  my $script = File::Spec->rel2abs("$tmpdir/exec.pl");
  if (open(my $fh, "> $script")) {
    print $fh <<EOS;
#!/usr/bin/env perl
print STDOUT "\$ARGV[0]\\n";
exit 0;
EOS
    unless (close($fh)) {
      die("Can't write $script: $!");
    }

    unless (chmod(0755, $script)) {
      die("Can't set perms on $script to 0755: $!");
    }

  } else {
    die("Can't open $script: $!");
  }

  my $config = {
    PidFile => $pid_file,
    ScoreboardFile => $scoreboard_file,
    SystemLog => $log_file,

    AuthUserFile => $auth_user_file,
    AuthGroupFile => $auth_group_file,

    IfModules => {
      'mod_exec.c' => {
        ExecEngine => 'on',
        ExecLog => $log_file,
        ExecTimeout => 1,
        ExecOnConnect => "$script addr=%a",
        ExecOptions => 'logStdout',
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

  if (open(my $fh, "< $log_file")) {
    my $line;
    while ($line = <$fh>) {
      unless ($line =~ /stdout from '$script'/) {
        next;
      }

      chomp($line);
      last;
    }

    close($fh);

    $line =~ /stdout from '$script': '(.*?)'/;
    my $stdout = $1;

    my $expected = 'addr=127.0.0.1';

    $self->assert($expected eq $stdout,
      test_msg("Expected '$expected', got '$stdout'"));

  } else {
    die("Can't read $log_file: $!");
  }

  unlink($log_file);
}

sub exec_opt_log_stderr {
  my $self = shift;
  my $tmpdir = $self->{tmpdir};

  my $config_file = "$tmpdir/exec.conf";
  my $pid_file = File::Spec->rel2abs("$tmpdir/exec.pid");
  my $scoreboard_file = File::Spec->rel2abs("$tmpdir/exec.scoreboard");

  my $log_file = File::Spec->rel2abs('tests.log');

  my $auth_user_file = File::Spec->rel2abs("$tmpdir/exec.passwd");
  my $auth_group_file = File::Spec->rel2abs("$tmpdir/exec.group");

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

  my $script = File::Spec->rel2abs("$tmpdir/exec.pl");
  if (open(my $fh, "> $script")) {
    print $fh <<EOS;
#!/usr/bin/env perl
print STDERR "\$ARGV[0]\\n";
exit 0;
EOS
    unless (close($fh)) {
      die("Can't write $script: $!");
    }

    unless (chmod(0755, $script)) {
      die("Can't set perms on $script to 0755: $!");
    }

  } else {
    die("Can't open $script: $!");
  }

  my $config = {
    PidFile => $pid_file,
    ScoreboardFile => $scoreboard_file,
    SystemLog => $log_file,

    AuthUserFile => $auth_user_file,
    AuthGroupFile => $auth_group_file,

    IfModules => {
      'mod_exec.c' => {
        ExecEngine => 'on',
        ExecLog => $log_file,
        ExecTimeout => 1,
        ExecOnConnect => "$script addr=%a",
        ExecOptions => 'logStderr',
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

  if (open(my $fh, "< $log_file")) {
    my $line;
    while ($line = <$fh>) {
      unless ($line =~ /stderr from '$script'/) {
        next;
      }

      chomp($line);
      last;
    }

    close($fh);

    $line =~ /stderr from '$script': '(.*?)'/;
    my $stderr = $1;

    my $expected = 'addr=127.0.0.1';

    $self->assert($expected eq $stderr,
      test_msg("Expected '$expected', got '$stderr'"));

  } else {
    die("Can't read $log_file: $!");
  }

  unlink($log_file);
}

sub exec_opt_send_stdout {
  my $self = shift;
  my $tmpdir = $self->{tmpdir};

  my $config_file = "$tmpdir/exec.conf";
  my $pid_file = File::Spec->rel2abs("$tmpdir/exec.pid");
  my $scoreboard_file = File::Spec->rel2abs("$tmpdir/exec.scoreboard");

  my $log_file = File::Spec->rel2abs('tests.log');

  my $auth_user_file = File::Spec->rel2abs("$tmpdir/exec.passwd");
  my $auth_group_file = File::Spec->rel2abs("$tmpdir/exec.group");

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

  my $script = File::Spec->rel2abs("$tmpdir/exec.pl");
  if (open(my $fh, "> $script")) {
    print $fh <<EOS;
#!/usr/bin/env perl
print STDOUT "\$ARGV[0]\\n";
exit 0;
EOS
    unless (close($fh)) {
      die("Can't write $script: $!");
    }

    unless (chmod(0755, $script)) {
      die("Can't set perms on $script to 0755: $!");
    }

  } else {
    die("Can't open $script: $!");
  }

  my $config = {
    PidFile => $pid_file,
    ScoreboardFile => $scoreboard_file,
    SystemLog => $log_file,

    AuthUserFile => $auth_user_file,
    AuthGroupFile => $auth_group_file,

    IfModules => {
      'mod_exec.c' => {
        ExecEngine => 'on',
        ExecLog => $log_file,
        ExecTimeout => 1,
        ExecOnConnect => "$script addr=%a",
        ExecOptions => 'sendStdout',
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

      my $resp_code = $client->response_code();
      my $resp_msg = $client->response_msg();

      my $expected;

      $expected = 220;
      $self->assert($expected == $resp_code,
        test_msg("Expected $expected, got $resp_code"));

      $expected = "addr=127.0.0.1";
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

sub exec_opt_use_stdin {
  my $self = shift;
  my $tmpdir = $self->{tmpdir};

  my $config_file = "$tmpdir/exec.conf";
  my $pid_file = File::Spec->rel2abs("$tmpdir/exec.pid");
  my $scoreboard_file = File::Spec->rel2abs("$tmpdir/exec.scoreboard");

  my $log_file = File::Spec->rel2abs('tests.log');

  my $auth_user_file = File::Spec->rel2abs("$tmpdir/exec.passwd");
  my $auth_group_file = File::Spec->rel2abs("$tmpdir/exec.group");

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

  my $script = File::Spec->rel2abs("$tmpdir/exec.pl");
  if (open(my $fh, "> $script")) {
    print $fh <<EOS;
#!/usr/bin/env perl
while (my \$line = <STDIN>) {
  chomp(\$line);
  if (\$line eq '.') {
    last;
  }

  if (\$line !~ /^\\d+\$/) {
    print STDOUT "\$line\\n";
  }
}
exit 0;
EOS
    unless (close($fh)) {
      die("Can't write $script: $!");
    }

    unless (chmod(0755, $script)) {
      die("Can't set perms on $script to 0755: $!");
    }

  } else {
    die("Can't open $script: $!");
  }

  my $config = {
    PidFile => $pid_file,
    ScoreboardFile => $scoreboard_file,
    SystemLog => $log_file,

    AuthUserFile => $auth_user_file,
    AuthGroupFile => $auth_group_file,

    IfModules => {
      'mod_exec.c' => {
        ExecEngine => 'on',
        ExecLog => $log_file,
        ExecTimeout => 1,
        ExecOnConnect => "$script 1 2 3 addr=%a",
        ExecOptions => 'logStdout useStdin',
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

  if (open(my $fh, "< $log_file")) {
    while (my $line = <$fh>) {
      unless ($line =~ /stdout from '$script'/) {
        next;
      }

      chomp($line);
      close($fh);

      $line =~ /stdout from '$script': '(.*?)'/;
      my $stdout = $1;

      my $expected = 'addr=127.0.0.1';

      $self->assert($expected eq $stdout,
        test_msg("Expected '$expected', got '$stdout'"));
    }

  } else {
    die("Can't read $log_file: $!");
  }

  unlink($log_file);
}

1;
