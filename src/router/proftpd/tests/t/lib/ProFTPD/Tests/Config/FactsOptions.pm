package ProFTPD::Tests::Config::FactsOptions;

use lib qw(t/lib);
use base qw(ProFTPD::TestSuite::Child);
use strict;

use Cwd;
use File::Path qw(mkpath);
use File::Spec;
use IO::Handle;

use ProFTPD::TestSuite::FTP;
use ProFTPD::TestSuite::Utils qw(:auth :config :running :test :testsuite);

$| = 1;

my $order = 0;

my $TESTS = {
  # XXX All of these tests use relative symlink paths.  Also need
  # tests which use absolute symlink paths, in both chrooted and non-chrooted
  # flavors.

  factsoptions_use_slink_mlsd_rel_symlinked_file => {
    order => ++$order,
    test_class => [qw(forking)],
  },

  factsoptions_use_slink_mlsd_rel_symlinked_dir => {
    order => ++$order,
    test_class => [qw(forking)],
  },

  factsoptions_use_slink_mlst_rel_symlinked_file => {
    order => ++$order,
    test_class => [qw(forking)],
  },

  factsoptions_use_slink_mlst_rel_symlinked_file_chrooted => {
    order => ++$order,
    test_class => [qw(forking rootprivs)],
  },

  factsoptions_use_slink_mlst_rel_symlinked_dir => {
    order => ++$order,
    test_class => [qw(forking)],
  },

  factsoptions_use_slink_mlst_rel_symlinked_dir_chrooted => {
    order => ++$order,
    test_class => [qw(forking rootprivs)],
  },

};

sub new {
  return shift()->SUPER::new(@_);
}

sub list_tests {
  return testsuite_get_runnable_tests($TESTS);
}

sub factsoptions_use_slink_mlsd_rel_symlinked_file {
  my $self = shift;
  my $tmpdir = $self->{tmpdir};
  my $setup = test_setup($tmpdir, 'config');

  my $foo_dir = File::Spec->rel2abs("$tmpdir/foo");
  mkpath($foo_dir);

  my $test_file = File::Spec->rel2abs("$tmpdir/foo/test.txt");
  if (open(my $fh, "> $test_file")) {
    print $fh "Hello, World!\n";
    unless (close($fh)) {
      die("Can't write $test_file: $!");
    }

  } else {
    die("Can't open $test_file: $!");
  }

  # Change to the 'foo' directory in order to create a relative path in the
  # symlink we need

  my $cwd = getcwd();
  unless (chdir("$foo_dir")) {
    die("Can't chdir to $foo_dir: $!");
  }

  unless (symlink('test.txt', 'test.lnk')) {
    die("Can't symlink 'test.txt' to 'test.lnk': $!");
  }

  unless (chdir($cwd)) {
    die("Can't chdir to $cwd: $!");
  }

  # Make sure that, if we're running as root, that the home directory has
  # permissions/privs set for the account we create
  if ($< == 0) {
    unless (chmod(0755, $foo_dir)) {
      die("Can't set perms on $foo_dir to 0755: $!");
    }

    unless (chown($setup->{uid}, $setup->{gid}, $foo_dir)) {
      die("Can't set owner of $foo_dir to $setup->{uid}/$setup->{gid}: $!");
    }
  }

  my $config = {
    PidFile => $setup->{pid_file},
    ScoreboardFile => $setup->{scoreboard_file},
    SystemLog => $setup->{log_file},

    AuthUserFile => $setup->{auth_user_file},
    AuthGroupFile => $setup->{auth_group_file},
    AuthOrder => 'mod_auth_file.c',

    ShowSymlinks => 'on',
    FactsOptions => 'UseSlink',

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

      my $conn = $client->mlsd_raw('foo');
      unless ($conn) {
        die("MLSD failed: " . $client->response_code() . " " .
          $client->response_msg());
      }

      my $buf;
      $conn->read($buf, 8192, 25);
      eval { $conn->close() };

      my $res = {};
      my $lines = [split(/\n/, $buf)];
      foreach my $line (@$lines) {
        if ($line =~ /^modify=\S+;perm=\S+;type=(\S+);unique=(\S+);UNIX\.group=\d+;UNIX\.groupname=\S+;UNIX\.mode=\d+;UNIX\.owner=\d+;UNIX\.ownername=\S+; (.*?)$/) {
          $res->{$3} = { type => $1, unique => $2 };
        }
      }

      my $count = scalar(keys(%$res));
      unless ($count == 4) {
        die("MLSD returned wrong number of entries (expected 4, got $count)");
      }

      # test.lnk is a symlink to test.txt.  According to RFC3659, the unique
      # fact for both of these should thus be the same, since they are the
      # same underlying object.

      my $expected = $res->{'test.txt'}->{unique};
      my $got = $res->{'test.lnk'}->{unique};
      $self->assert($expected eq $got,
        test_msg("Expected '$expected', got '$got'"));

      # Since ShowSymlinks is on, the type for test.lnk should indicate that
      # it's a symlink.  And since the UseSlink FactOption is used, the
      # type should include the target path.
      $expected = 'OS.unix=slink:(.*)?'; 
      $got = $res->{'test.lnk'}->{type};
      $self->assert(qr/$expected/i, $got,
        test_msg("Expected '$expected', got '$got'"));

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

sub factsoptions_use_slink_mlsd_rel_symlinked_dir {
  my $self = shift;
  my $tmpdir = $self->{tmpdir};
  my $setup = test_setup($tmpdir, 'config');

  my $foo_dir = File::Spec->rel2abs("$tmpdir/foo");
  mkpath($foo_dir);

  my $test_dir = File::Spec->rel2abs("$tmpdir/foo/test.d");
  mkpath($test_dir);

  # Change to the 'foo' directory in order to create a relative path in the
  # symlink we need

  my $cwd = getcwd();
  unless (chdir("$foo_dir")) {
    die("Can't chdir to $foo_dir: $!");
  }

  unless (symlink('test.d', 'test.lnk')) {
    die("Can't symlink 'test.d' to 'test.lnk': $!");
  }

  unless (chdir($cwd)) {
    die("Can't chdir to $cwd: $!");
  }

  # Make sure that, if we're running as root, that the home directory has
  # permissions/privs set for the account we create
  if ($< == 0) {
    unless (chmod(0755, $foo_dir)) {
      die("Can't set perms on $foo_dir to 0755: $!");
    }

    unless (chown($setup->{uid}, $setup->{gid}, $foo_dir)) {
      die("Can't set owner of $foo_dir to $setup->{uid}/$setup->{gid}: $!");
    }
  }

  my $config = {
    PidFile => $setup->{pid_file},
    ScoreboardFile => $setup->{scoreboard_file},
    SystemLog => $setup->{log_file},

    AuthUserFile => $setup->{auth_user_file},
    AuthGroupFile => $setup->{auth_group_file},
    AuthOrder => 'mod_auth_file.c',

    ShowSymlinks => 'on',
    FactsOptions => 'UseSlink',

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

      my $conn = $client->mlsd_raw('foo');
      unless ($conn) {
        die("MLSD failed: " . $client->response_code() . " " .
          $client->response_msg());
      }

      my $buf;
      $conn->read($buf, 8192, 25);
      eval { $conn->close() };

      my $res = {};
      my $lines = [split(/\n/, $buf)];
      foreach my $line (@$lines) {
        if ($line =~ /^modify=\S+;perm=\S+;type=(\S+);unique=(\S+);UNIX\.group=\d+;UNIX\.groupname=\S+;UNIX\.mode=\d+;UNIX\.owner=\d+;UNIX\.ownername=\S+; (.*?)$/) {
          $res->{$3} = { type => $1, unique => $2 };
        }
      }

      my $count = scalar(keys(%$res));
      unless ($count == 4) {
        die("MLSD returned wrong number of entries (expected 4, got $count)");
      }

      # test.lnk is a symlink to test.d.  According to RFC3659, the unique
      # fact for both of these should thus be the same, since they are the
      # same underlying object.

      my $expected = $res->{'test.d'}->{unique};
      my $got = $res->{'test.lnk'}->{unique};
      $self->assert($expected eq $got,
        test_msg("Expected '$expected', got '$got'"));

      # Since ShowSymlinks is on, the type for test.lnk should indicate that
      # it's a symlink.  And since the UseSlink FactOption is used, the
      # type should include the target path.
      $expected = 'OS.unix=slink:(.*)?'; 
      $got = $res->{'test.lnk'}->{type};
      $self->assert(qr/$expected/i, $got,
        test_msg("Expected '$expected', got '$got'"));

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

sub factsoptions_use_slink_mlst_rel_symlinked_file {
  my $self = shift;
  my $tmpdir = $self->{tmpdir};
  my $setup = test_setup($tmpdir, 'config');

  my $foo_dir = File::Spec->rel2abs("$tmpdir/foo");
  mkpath($foo_dir);

  my $test_file = File::Spec->rel2abs("$tmpdir/foo/test.txt");
  if (open(my $fh, "> $test_file")) {
    print $fh "Hello, World!\n";
    unless (close($fh)) {
      die("Can't write $test_file: $!");
    }

  } else {
    die("Can't open $test_file: $!");
  }

  my $test_symlink = File::Spec->rel2abs("$tmpdir/foo/test.lnk");

  # Change to the 'foo' directory in order to create a relative path in the
  # symlink we need

  my $cwd = getcwd();
  unless (chdir("$foo_dir")) {
    die("Can't chdir to $foo_dir: $!");
  }

  unless (symlink('test.txt', 'test.lnk')) {
    die("Can't symlink 'test.txt' to 'test.lnk': $!");
  }

  unless (chdir($cwd)) {
    die("Can't chdir to $cwd: $!");
  }

  # Make sure that, if we're running as root, that the home directory has
  # permissions/privs set for the account we create
  if ($< == 0) {
    unless (chmod(0755, $foo_dir)) {
      die("Can't set perms on $foo_dir to 0755: $!");
    }

    unless (chown($setup->{uid}, $setup->{gid}, $foo_dir)) {
      die("Can't set owner of $foo_dir to $setup->{uid}/$setup->{gid}: $!");
    }
  }

  my $config = {
    PidFile => $setup->{pid_file},
    ScoreboardFile => $setup->{scoreboard_file},
    SystemLog => $setup->{log_file},

    AuthUserFile => $setup->{auth_user_file},
    AuthGroupFile => $setup->{auth_group_file},
    AuthOrder => 'mod_auth_file.c',

    ShowSymlinks => 'on',
    FactsOptions => 'UseSlink',

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

      my ($resp_code, $resp_msg) = $client->mlst('foo/test.lnk');

      my $expected = 250;
      $self->assert($expected == $resp_code,
        test_msg("Expected response code $expected, got $resp_code"));

      if ($^O eq 'darwin') {
        # MacOSX-specific hack, due to how it handles tmp files
        $test_symlink = ('/private' . $test_symlink);
      }

      $expected = 'modify=\d+;perm=adfr(w)?;size=\d+;type=OS\.unix=slink:(\S+);unique=\S+;UNIX\.group=\d+;UNIX\.groupname=\S+;UNIX\.mode=\d+;UNIX\.owner=\d+;UNIX\.ownername=\S+; ' . $test_symlink . '$';
      $self->assert(qr/$expected/, $resp_msg,
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

sub factsoptions_use_slink_mlst_rel_symlinked_file_chrooted {
  my $self = shift;
  my $tmpdir = $self->{tmpdir};
  my $setup = test_setup($tmpdir, 'config');

  my $foo_dir = File::Spec->rel2abs("$tmpdir/foo");
  mkpath($foo_dir);

  my $test_file = File::Spec->rel2abs("$tmpdir/foo/test.txt");
  if (open(my $fh, "> $test_file")) {
    print $fh "Hello, World!\n";
    unless (close($fh)) {
      die("Can't write $test_file: $!");
    }

  } else {
    die("Can't open $test_file: $!");
  }

  # Since we're chrooting the session, we don't expect to see the real
  # absolute path.
  my $test_symlink = "/foo/test.lnk";

  # Change to the 'foo' directory in order to create a relative path in the
  # symlink we need

  my $cwd = getcwd();
  unless (chdir("$foo_dir")) {
    die("Can't chdir to $foo_dir: $!");
  }

  unless (symlink('test.txt', 'test.lnk')) {
    die("Can't symlink 'test.txt' to 'test.lnk': $!");
  }

  unless (chdir($cwd)) {
    die("Can't chdir to $cwd: $!");
  }

  # Make sure that, if we're running as root, that the home directory has
  # permissions/privs set for the account we create
  if ($< == 0) {
    unless (chmod(0755, $foo_dir)) {
      die("Can't set perms on $foo_dir to 0755: $!");
    }

    unless (chown($setup->{uid}, $setup->{gid}, $foo_dir)) {
      die("Can't set owner of $foo_dir to $setup->{uid}/$setup->{gid}: $!");
    }
  }

  my $config = {
    PidFile => $setup->{pid_file},
    ScoreboardFile => $setup->{scoreboard_file},
    SystemLog => $setup->{log_file},

    AuthUserFile => $setup->{auth_user_file},
    AuthGroupFile => $setup->{auth_group_file},
    AuthOrder => 'mod_auth_file.c',

    ShowSymlinks => 'on',
    FactsOptions => 'UseSlink',

    DefaultRoot => '~',

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

      my ($resp_code, $resp_msg) = $client->mlst('foo/test.lnk');

      my $expected = 250;
      $self->assert($expected == $resp_code,
        test_msg("Expected response code $expected, got $resp_code"));

      $expected = 'modify=\d+;perm=adfr(w)?;size=\d+;type=OS\.unix=slink:(\S+);unique=\S+;UNIX\.group=\d+;UNIX\.groupname=\S+;UNIX\.mode=\d+;UNIX\.owner=\d+;UNIX\.ownername=\S+; ' . $test_symlink . '$';
      $self->assert(qr/$expected/, $resp_msg,
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

sub factsoptions_use_slink_mlst_rel_symlinked_dir {
  my $self = shift;
  my $tmpdir = $self->{tmpdir};
  my $setup = test_setup($tmpdir, 'config');

  my $foo_dir = File::Spec->rel2abs("$tmpdir/foo");
  mkpath($foo_dir);

  my $test_dir = File::Spec->rel2abs("$tmpdir/foo/test.d");
  mkpath($test_dir);

  my $test_symlink = File::Spec->rel2abs("$tmpdir/foo/test.lnk");

  # Change to the 'foo' directory in order to create a relative path in the
  # symlink we need

  my $cwd = getcwd();
  unless (chdir("$foo_dir")) {
    die("Can't chdir to $foo_dir: $!");
  }

  unless (symlink('test.d', 'test.lnk')) {
    die("Can't symlink 'test.d' to 'test.lnk': $!");
  }

  unless (chdir($cwd)) {
    die("Can't chdir to $cwd: $!");
  }

  # Make sure that, if we're running as root, that the home directory has
  # permissions/privs set for the account we create
  if ($< == 0) {
    unless (chmod(0755, $foo_dir)) {
      die("Can't set perms on $foo_dir to 0755: $!");
    }

    unless (chown($setup->{uid}, $setup->{gid}, $foo_dir)) {
      die("Can't set owner of $foo_dir to $setup->{uid}/$setup->{gid}: $!");
    }
  }

  my $config = {
    PidFile => $setup->{pid_file},
    ScoreboardFile => $setup->{scoreboard_file},
    SystemLog => $setup->{log_file},

    AuthUserFile => $setup->{auth_user_file},
    AuthGroupFile => $setup->{auth_group_file},
    AuthOrder => 'mod_auth_file.c',

    ShowSymlinks => 'on',
    FactsOptions => 'UseSlink',

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

      my ($resp_code, $resp_msg) = $client->mlst('foo/test.lnk');

      my $expected = 250;
      $self->assert($expected == $resp_code,
        test_msg("Expected response code $expected, got $resp_code"));

      if ($^O eq 'darwin') {
        # MacOSX-specific hack, due to how it handles tmp files
        $test_symlink = ('/private' . $test_symlink);
      }

      $expected = 'modify=\d+;perm=adfr(w)?;size=\d+;type=OS\.unix=slink:(\S+);unique=\S+;UNIX\.group=\d+;UNIX\.groupname=\S+;UNIX\.mode=\d+;UNIX\.owner=\d+;UNIX\.ownername=\S+; ' . $test_symlink . '$';
      $self->assert(qr/$expected/, $resp_msg,
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

sub factsoptions_use_slink_mlst_rel_symlinked_dir_chrooted {
  my $self = shift;
  my $tmpdir = $self->{tmpdir};
  my $setup = test_setup($tmpdir, 'config');

  my $foo_dir = File::Spec->rel2abs("$tmpdir/foo");
  mkpath($foo_dir);

  my $test_dir = File::Spec->rel2abs("$tmpdir/foo/test.d");
  mkpath($test_dir);

  # Since we're chrooting the session, we don't expect to see the real
  # absolute path
  my $test_symlink = "/foo/test.lnk";

  # Change to the 'foo' directory in order to create a relative path in the
  # symlink we need

  my $cwd = getcwd();
  unless (chdir("$foo_dir")) {
    die("Can't chdir to $foo_dir: $!");
  }

  unless (symlink('test.d', 'test.lnk')) {
    die("Can't symlink 'test.d' to 'test.lnk': $!");
  }

  unless (chdir($cwd)) {
    die("Can't chdir to $cwd: $!");
  }

  # Make sure that, if we're running as root, that the home directory has
  # permissions/privs set for the account we create
  if ($< == 0) {
    unless (chmod(0755, $foo_dir)) {
      die("Can't set perms on $foo_dir to 0755: $!");
    }

    unless (chown($setup->{uid}, $setup->{gid}, $foo_dir)) {
      die("Can't set owner of $foo_dir to $setup->{uid}/$setup->{gid}: $!");
    }
  }

  my $config = {
    PidFile => $setup->{pid_file},
    ScoreboardFile => $setup->{scoreboard_file},
    SystemLog => $setup->{log_file},

    AuthUserFile => $setup->{auth_user_file},
    AuthGroupFile => $setup->{auth_group_file},
    AuthOrder => 'mod_auth_file.c',

    ShowSymlinks => 'on',
    FactsOptions => 'UseSlink',

    DefaultRoot => '~',

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

      my ($resp_code, $resp_msg) = $client->mlst('foo/test.lnk');

      my $expected = 250;
      $self->assert($expected == $resp_code,
        test_msg("Expected response code $expected, got $resp_code"));

      $expected = 'modify=\d+;perm=adfr(w)?;size=\d+;type=OS\.unix=slink:(\S+);unique=\S+;UNIX\.group=\d+;UNIX\.groupname=\S+;UNIX\.mode=\d+;UNIX\.owner=\d+;UNIX\.ownername=\S+; ' . $test_symlink . '$';
      $self->assert(qr/$expected/, $resp_msg,
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

1;
