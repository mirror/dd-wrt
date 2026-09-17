package ProFTPD::Tests::Config::DisplayChdir;

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
  displaychdir_singleline => {
    order => ++$order,
    test_class => [qw(forking)],
  },

  displaychdir_multiline => {
    order => ++$order,
    test_class => [qw(forking)],
  },

  displaychdir_first_chdir => {
    order => ++$order,
    test_class => [qw(bug forking)],
  },

  displaychdir_abs_path_issue1688 => {
    order => ++$order,
    test_class => [qw(bug forking)],
  },

  displaychdir_abs_path_chrooted_issue1688 => {
    order => ++$order,
    test_class => [qw(bug forking rootprivs)],
  },

  displaychdir_first_chdir_abs_path_chrooted_issue1688 => {
    order => ++$order,
    test_class => [qw(bug forking rootprivs)],
  },

};

sub new {
  return shift()->SUPER::new(@_);
}

sub list_tests {
  return testsuite_get_runnable_tests($TESTS);
}

sub displaychdir_singleline {
  my $self = shift;
  my $tmpdir = $self->{tmpdir};
  my $setup = test_setup($tmpdir, 'config');

  my $sub_dir = File::Spec->rel2abs("$setup->{home_dir}/subdir");
  mkpath($sub_dir);

  # Make sure that, if we're running as root, that the test directory has
  # permissions/privs set for the account we create
  if ($< == 0) {
    unless (chmod(0755, $sub_dir)) {
      die("Can't set perms on $sub_dir to 0755: $!");
    }

    unless (chown($setup->{uid}, $setup->{gid}, $sub_dir)) {
      die("Can't set owner of $sub_dir to $setup->{uid}/$setup->{gid}: $!");
    }
  }

  my $display_file = File::Spec->rel2abs("$sub_dir/display.txt");
  if (open(my $fh, "> $display_file")) {
    print $fh "Hello user!\n";

    unless (close($fh)) {
      die("Unable to write $display_file: $!");
    }

  } else {
    die("Unable to open $display_file: $!");
  }

  my $config = {
    PidFile => $setup->{pid_file},
    ScoreboardFile => $setup->{scoreboard_file},
    SystemLog => $setup->{log_file},

    AuthUserFile => $setup->{auth_user_file},
    AuthGroupFile => $setup->{auth_group_file},
    AuthOrder => 'mod_auth_file.c',

    DisplayChdir => 'display.txt',

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

      $client->cwd('subdir');

      my $resp_code = $client->response_code();
      my $resp_msg = $client->response_msg(0);

      my $expected = 250;
      $self->assert($expected == $resp_code,
        test_msg("Expected response code $expected, got $resp_code"));

      $expected = "Hello user!";
      $self->assert($expected eq $resp_msg,
        test_msg("Expected response message '$expected', got '$resp_msg'"));

      $client->quit();

      $resp_code = $client->response_code();
      $resp_msg = $client->response_msg();

      $expected = 221;
      $self->assert($expected == $resp_code,
        test_msg("Expected response code $expected, got $resp_code"));

      $expected = "Goodbye.";
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

sub displaychdir_multiline {
  my $self = shift;
  my $tmpdir = $self->{tmpdir};
  my $setup = test_setup($tmpdir, 'config');

  my $sub_dir = File::Spec->rel2abs("$setup->{home_dir}/subdir");
  mkpath($sub_dir);

  # Make sure that, if we're running as root, that the test directory has
  # permissions/privs set for the account we create
  if ($< == 0) {
    unless (chmod(0755, $sub_dir)) {
      die("Can't set perms on $sub_dir to 0755: $!");
    }

    unless (chown($setup->{uid}, $setup->{gid}, $sub_dir)) {
      die("Can't set owner of $sub_dir to $setup->{uid}/$setup->{gid}: $!");
    }
  }

  my $display_file = File::Spec->rel2abs("$sub_dir/display.txt");
  if (open(my $fh, "> $display_file")) {
    print $fh "Velkommen, Bienvenue\nDoor's open, come on in\nHello user!\n";

    unless (close($fh)) {
      die("Unable to write $display_file: $!");
    }

  } else {
    die("Unable to open $display_file: $!");
  }

  my $config = {
    PidFile => $setup->{pid_file},
    ScoreboardFile => $setup->{scoreboard_file},
    SystemLog => $setup->{log_file},

    AuthUserFile => $setup->{auth_user_file},
    AuthGroupFile => $setup->{auth_group_file},
    AuthOrder => 'mod_auth_file.c',

    DisplayChdir => 'display.txt',

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

      $client->cwd('subdir');

      my $resp_code = $client->response_code();
      my $resp_msg = $client->response_msg(2);

      my $expected = 250;
      $self->assert($expected == $resp_code,
        test_msg("Expected response code $expected, got $resp_code"));

      $expected = " Hello user!";
      $self->assert($expected eq $resp_msg,
        test_msg("Expected response message '$expected', got '$resp_msg'"));

      $client->quit();

      $resp_code = $client->response_code();
      $resp_msg = $client->response_msg();

      $expected = 221;
      $self->assert($expected == $resp_code,
        test_msg("Expected response code $expected, got $resp_code"));

      $expected = "Goodbye.";
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

sub displaychdir_first_chdir {
  my $self = shift;
  my $tmpdir = $self->{tmpdir};
  my $setup = test_setup($tmpdir, 'config');

  my $sub_dir = File::Spec->rel2abs("$setup->{home_dir}/subdir");
  mkpath($sub_dir);

  # Make sure that, if we're running as root, that the test directory has
  # permissions/privs set for the account we create
  if ($< == 0) {
    unless (chmod(0755, $sub_dir)) {
      die("Can't set perms on $sub_dir to 0755: $!");
    }

    unless (chown($setup->{uid}, $setup->{gid}, $sub_dir)) {
      die("Can't set owner of $sub_dir to $setup->{uid}/$setup->{gid}: $!");
    }
  }

  my $display_file = File::Spec->rel2abs("$sub_dir/display.txt");
  if (open(my $fh, "> $display_file")) {
    print $fh "Hello user!\n";

    unless (close($fh)) {
      die("Unable to write $display_file: $!");
    }

  } else {
    die("Unable to open $display_file: $!");
  }

  my $config = {
    PidFile => $setup->{pid_file},
    ScoreboardFile => $setup->{scoreboard_file},
    SystemLog => $setup->{log_file},

    AuthUserFile => $setup->{auth_user_file},
    AuthGroupFile => $setup->{auth_group_file},
    AuthOrder => 'mod_auth_file.c',

    DisplayChdir => "display.txt true",

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

      $client->cwd('subdir');

      my $resp_code = $client->response_code();
      my $resp_msg = $client->response_msg(0);

      my $expected = 250;
      $self->assert($expected == $resp_code,
        test_msg("Expected response code $expected, got $resp_code"));

      $expected = "Hello user!";
      $self->assert($expected eq $resp_msg,
        test_msg("Expected response message '$expected', got '$resp_msg'"));

      $client->cwd('..');
      $client->cwd('subdir');

      $resp_code = $client->response_code();
      $resp_msg = $client->response_msg(0);

      $expected = 250;
      $self->assert($expected == $resp_code,
        test_msg("Expected response code $expected, got $resp_code"));

      $expected = "CWD command successful";
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

sub displaychdir_abs_path_issue1688 {
  my $self = shift;
  my $tmpdir = $self->{tmpdir};
  my $setup = test_setup($tmpdir, 'config');

  my $sub_dir = File::Spec->rel2abs("$setup->{home_dir}/subdir");
  mkpath($sub_dir);

  # Make sure that, if we're running as root, that the test directory has
  # permissions/privs set for the account we create
  if ($< == 0) {
    unless (chmod(0755, $sub_dir)) {
      die("Can't set perms on $sub_dir to 0755: $!");
    }

    unless (chown($setup->{uid}, $setup->{gid}, $sub_dir)) {
      die("Can't set owner of $sub_dir to $setup->{uid}/$setup->{gid}: $!");
    }
  }

  my $display_file = File::Spec->rel2abs("$sub_dir/display.txt");
  if (open(my $fh, "> $display_file")) {
    print $fh "Hello user!\n";

    unless (close($fh)) {
      die("Unable to write $display_file: $!");
    }

  } else {
    die("Unable to open $display_file: $!");
  }

  my $config = {
    PidFile => $setup->{pid_file},
    ScoreboardFile => $setup->{scoreboard_file},
    SystemLog => $setup->{log_file},

    AuthUserFile => $setup->{auth_user_file},
    AuthGroupFile => $setup->{auth_group_file},
    AuthOrder => 'mod_auth_file.c',

    DisplayChdir => $display_file,

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

      $client->cwd('subdir');

      my $resp_code = $client->response_code();
      my $resp_msg = $client->response_msg(0);

      my $expected = 250;
      $self->assert($expected == $resp_code,
        test_msg("Expected response code $expected, got $resp_code"));

      $expected = "Hello user!";
      $self->assert($expected eq $resp_msg,
        test_msg("Expected response message '$expected', got '$resp_msg'"));

      $client->quit();

      $resp_code = $client->response_code();
      $resp_msg = $client->response_msg();

      $expected = 221;
      $self->assert($expected == $resp_code,
        test_msg("Expected response code $expected, got $resp_code"));

      $expected = "Goodbye.";
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

sub displaychdir_abs_path_chrooted_issue1688 {
  my $self = shift;
  my $tmpdir = $self->{tmpdir};
  my $setup = test_setup($tmpdir, 'config');

  my $sub_dir = File::Spec->rel2abs("$setup->{home_dir}/subdir");
  mkpath($sub_dir);

  # Make sure that, if we're running as root, that the test directory has
  # permissions/privs set for the account we create
  if ($< == 0) {
    unless (chmod(0755, $sub_dir)) {
      die("Can't set perms on $sub_dir to 0755: $!");
    }

    unless (chown($setup->{uid}, $setup->{gid}, $sub_dir)) {
      die("Can't set owner of $sub_dir to $setup->{uid}/$setup->{gid}: $!");
    }
  }

  my $display_file = File::Spec->rel2abs("$setup->{home_dir}/display.txt");
  if (open(my $fh, "> $display_file")) {
    print $fh "Hello user!\n";

    unless (close($fh)) {
      die("Unable to write $display_file: $!");
    }

  } else {
    die("Unable to open $display_file: $!");
  }

  my $config = {
    PidFile => $setup->{pid_file},
    ScoreboardFile => $setup->{scoreboard_file},
    SystemLog => $setup->{log_file},

    AuthUserFile => $setup->{auth_user_file},
    AuthGroupFile => $setup->{auth_group_file},
    AuthOrder => 'mod_auth_file.c',

    DefaultRoot => '~',
    DisplayChdir => $display_file,

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

      $client->cwd('subdir');

      my $resp_code = $client->response_code();
      my $resp_msg = $client->response_msg(0);

      my $expected = 250;
      $self->assert($expected == $resp_code,
        test_msg("Expected response code $expected, got $resp_code"));

      $expected = "Hello user!";
      $self->assert($expected eq $resp_msg,
        test_msg("Expected response message '$expected', got '$resp_msg'"));

      $client->quit();

      $resp_code = $client->response_code();
      $resp_msg = $client->response_msg();

      $expected = 221;
      $self->assert($expected == $resp_code,
        test_msg("Expected response code $expected, got $resp_code"));

      $expected = "Goodbye.";
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

sub displaychdir_first_chdir_abs_path_chrooted_issue1688 {
  my $self = shift;
  my $tmpdir = $self->{tmpdir};
  my $setup = test_setup($tmpdir, 'config');

  my $sub_dir = File::Spec->rel2abs("$setup->{home_dir}/subdir");
  mkpath($sub_dir);

  # Make sure that, if we're running as root, that the test directory has
  # permissions/privs set for the account we create
  if ($< == 0) {
    unless (chmod(0755, $sub_dir)) {
      die("Can't set perms on $sub_dir to 0755: $!");
    }

    unless (chown($setup->{uid}, $setup->{gid}, $sub_dir)) {
      die("Can't set owner of $sub_dir to $setup->{uid}/$setup->{gid}: $!");
    }
  }

  my $display_file = File::Spec->rel2abs("$setup->{home_dir}/display.txt");
  if (open(my $fh, "> $display_file")) {
    print $fh "Hello user!\n";

    unless (close($fh)) {
      die("Unable to write $display_file: $!");
    }

  } else {
    die("Unable to open $display_file: $!");
  }

  my $config = {
    PidFile => $setup->{pid_file},
    ScoreboardFile => $setup->{scoreboard_file},
    SystemLog => $setup->{log_file},

    AuthUserFile => $setup->{auth_user_file},
    AuthGroupFile => $setup->{auth_group_file},
    AuthOrder => 'mod_auth_file.c',

    DefaultRoot => '~',
    DisplayChdir => "$display_file true",

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

      $client->cwd('subdir');

      my $resp_code = $client->response_code();
      my $resp_msg = $client->response_msg(0);

      my $expected = 250;
      $self->assert($expected == $resp_code,
        test_msg("Expected response code $expected, got $resp_code"));

      $expected = "Hello user!";
      $self->assert($expected eq $resp_msg,
        test_msg("Expected response message '$expected', got '$resp_msg'"));

      $client->cwd('..');
      $client->cwd('subdir');

      $resp_code = $client->response_code();
      $resp_msg = $client->response_msg(0);

      $expected = 250;
      $self->assert($expected == $resp_code,
        test_msg("Expected response code $expected, got $resp_code"));

      $expected = "CWD command successful";
      $self->assert($expected eq $resp_msg,
        test_msg("Expected response message '$expected', got '$resp_msg'"));

      $client->quit();

      $resp_code = $client->response_code();
      $resp_msg = $client->response_msg();

      $expected = 221;
      $self->assert($expected == $resp_code,
        test_msg("Expected response code $expected, got $resp_code"));

      $expected = "Goodbye.";
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
