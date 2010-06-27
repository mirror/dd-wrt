package ProFTPD::Tests::Modules::mod_sql_sqlite;

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
  sql_bug2045 => {
    order => ++$order,
    test_class => [qw(bug forking)],
  },

  sql_bug2922 => {
    order => ++$order,
    test_class => [qw(bug forking rootprivs)],
  },

  sql_bug3116 => {
    order => ++$order,
    test_class => [qw(bug forking)],
  },

  sql_bug3124 => {
    order => ++$order,
    test_class => [qw(bug forking)],
  },

  sql_sqlite_bug3126 => {
    order => ++$order,
    test_class => [qw(bug forking)],
  },

  sql_user_where_clause_ok => {
    order => ++$order,
    test_class => [qw(forking)],
  },

  sql_bug3149 => {
    order => ++$order,
    test_class => [qw(bug forking)],
  },

  sql_sqllog => {
    order => ++$order,
    test_class => [qw(forking)],
  },

  sql_sqlite_sqllog_with_chroot => {
    order => ++$order,
    test_class => [qw(forking rootprivs)],
  },

  sql_custom_user_info => {
    order => ++$order,
    test_class => [qw(forking)],
  },

  sql_userset_bug2434 => {
    order => ++$order,
    test_class => [qw(bug forking rootprivs)],
  },

  sql_usersetfast_bug2434 => {
    order => ++$order,
    test_class => [qw(bug forking rootprivs)],
  },

  sql_custom_group_info_bug3043 => {
    order => ++$order,
    test_class => [qw(bug forking)],
  },

  sql_groupset_bug3043 => {
    order => ++$order,
    test_class => [qw(bug forking rootprivs)],
  },

  sql_groupsetfast_bug3043 => {
    order => ++$order,
    test_class => [qw(bug forking rootprivs)],
  },

  sql_sqllog_var_w => {
    order => ++$order,
    test_class => [qw(forking)],
  },

  sql_openssl_auth_type_md5 => {
    order => ++$order,
    test_class => [qw(feature_openssl forking)],
  },

  sql_openssl_auth_type_sha1 => {
    order => ++$order,
    test_class => [qw(feature_openssl forking)],
  },

  sql_sqllog_var_S => {
    order => ++$order,
    test_class => [qw(forking)],
  },

  sql_sqllog_var_S_err => {
    order => ++$order,
    test_class => [qw(forking)],
  },

  sql_negative_cache_bug3282 => {
    order => ++$order,
    test_class => [qw(bug forking)],
  },

  sql_sqllog_var_T_rnfr => {
    order => ++$order,
    test_class => [qw(bug forking)],
  },

  sql_sqllog_var_T_retr => {
    order => ++$order,
    test_class => [qw(bug forking)],
  },

  sql_sqllog_exit => {
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

sub sql_bug2045 {
  my $self = shift;
  my $tmpdir = $self->{tmpdir};

  my $config_file = "$tmpdir/sqlite.conf";
  my $pid_file = File::Spec->rel2abs("$tmpdir/sqlite.pid");
  my $scoreboard_file = File::Spec->rel2abs("$tmpdir/sqlite.scoreboard");

  my $log_file = File::Spec->rel2abs('tests.log');

  my $user = 'proftpd';
  my $passwd = 'test';
  my $home_dir = File::Spec->rel2abs($tmpdir);
  my $uid = 500;
  my $gid = 500;

  my $db_file = File::Spec->rel2abs("$tmpdir/proftpd.db");

  # Build up sqlite3 command to create users, groups tables and populate them
  my $db_script = File::Spec->rel2abs("$tmpdir/proftpd.sql");

  if (open(my $fh, "> $db_script")) {
    print $fh <<EOS;
CREATE TABLE users (
  userid TEXT,
  passwd TEXT,
  uid INTEGER,
  gid INTEGER,
  homedir TEXT, 
  shell TEXT,
  lastdir TEXT
);
INSERT INTO users (userid, passwd, uid, gid, homedir, shell) VALUES ('$user', '$passwd', 500, 500, '$home_dir', '/bin/bash');

CREATE TABLE groups (
  groupname TEXT,
  gid INTEGER,
  members TEXT
);
INSERT INTO groups (groupname, gid, members) VALUES ('ftpd', 500, '$user');
EOS

    unless (close($fh)) {
      die("Can't write $db_script: $!");
    }

  } else {
    die("Can't open $db_script: $!");
  }

  my $cmd = "sqlite3 $db_file < $db_script";

  if ($ENV{TEST_VERBOSE}) {
    print STDERR "Executing sqlite3: $cmd\n";
  }

  my @output = `$cmd`;
  if (scalar(@output) &&
      $ENV{TEST_VERBOSE}) {
    print STDERR "Output: ", join('', @output), "\n";
  }

  my $config = {
    PidFile => $pid_file,
    ScoreboardFile => $scoreboard_file,
    SystemLog => $log_file,

    IfModules => {
      'mod_delay.c' => {
        DelayEngine => 'off',
      },

      'mod_sql.c' => {
        SQLAuthTypes => 'plaintext',
        SQLBackend => 'sqlite3',
        SQLConnectInfo => $db_file,
        SQLLogFile => $log_file,
        SQLNamedQuery => 'lastdir SELECT "lastdir FROM users WHERE userid = \'%u\'"',
        SQLShowInfo => 'PASS "230" "\"%{lastdir}\" was the last directory"',
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

      my $resp_msgs = $client->response_msgs();
      my $nmsgs = scalar(@$resp_msgs);

      my $expected;

      $expected = 1;
      $self->assert($expected == $nmsgs,
        test_msg("Expected $expected, got $nmsgs")); 

      $expected = "User proftpd logged in";
      $self->assert($expected eq $resp_msgs->[0],
        test_msg("Expected '$expected', got '$resp_msgs->[0]'"));

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

sub sql_bug2922 {
  my $self = shift;
  my $tmpdir = $self->{tmpdir};

  my $config_file = "$tmpdir/sqlite.conf";
  my $pid_file = File::Spec->rel2abs("$tmpdir/sqlite.pid");
  my $scoreboard_file = File::Spec->rel2abs("$tmpdir/sqlite.scoreboard");

  my $log_file = File::Spec->rel2abs('tests.log');

  # Bug#2922 occurred because mod_sql would "authenticate" the plaintext
  # password (if configured to do so) of a user whose info did NOT come from
  # mod_sql.

  my $db_file = File::Spec->rel2abs("$tmpdir/proftpd.db");

  # Build up sqlite3 command to create users, groups tables and populate them
  my $db_script = File::Spec->rel2abs("$tmpdir/proftpd.sql");

  if (open(my $fh, "> $db_script")) {
    print $fh <<EOS;
CREATE TABLE users (
  userid TEXT,
  passwd TEXT,
  uid INTEGER,
  gid INTEGER,
  homedir TEXT, 
  shell TEXT
);

CREATE TABLE groups (
  groupname TEXT,
  gid INTEGER,
  members TEXT
);
EOS

    unless (close($fh)) {
      die("Can't write $db_script: $!");
    }

  } else {
    die("Can't open $db_script: $!");
  }

  my $cmd = "sqlite3 $db_file < $db_script";

  if ($ENV{TEST_VERBOSE}) {
    print STDERR "Executing sqlite3: $cmd\n";
  }

  my @output = `$cmd`;
  if (scalar(@output) &&
      $ENV{TEST_VERBOSE}) {
    print STDERR "Output: ", join('', @output), "\n";
  }

  my $config = {
    PidFile => $pid_file,
    ScoreboardFile => $scoreboard_file,
    SystemLog => $log_file,

    MaxLoginAttempts => 100,

    IfModules => {
      'mod_delay.c' => {
        DelayEngine => 'off',
      },

      'mod_sql.c' => {
        SQLAuthTypes => 'plaintext',
        SQLBackend => 'sqlite3',
        SQLConnectInfo => $db_file,
        SQLLogFile => $log_file,
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
      # Bug#2922 would allow a login which should NOT happen.  So if
      # *any* of the following logins succeed, we have a problem.

      my $accounts = [
        { user => 'bin', passwd => '*' },
        { user => 'bin', passwd => 'x' },
        { user => 'daemon', passwd => '*' },
        { user => 'daemon', passwd => 'x' },
        { user => 'mysql', passwd => '*' },
        { user => 'mysql', passwd => 'x' },
      ];

      foreach my $account (@$accounts) {
        my $client = ProFTPD::TestSuite::FTP->new('127.0.0.1', $port);
        my $user = $account->{user};
        my $passwd = $account->{passwd};

        eval { $client->login($user, $passwd) };
        unless ($@) {
          die("Logged in unexpectedly");
        }
      }
    };

    if ($@) {
      $ex = $@;
    }

    $wfh->print("done\n");
    $wfh->flush();

  } else {
    eval { server_wait($config_file, $rfh, 30) };
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

sub sql_bug3116 {
  my $self = shift;
  my $tmpdir = $self->{tmpdir};

  my $config_file = "$tmpdir/sqlite.conf";
  my $pid_file = File::Spec->rel2abs("$tmpdir/sqlite.pid");
  my $scoreboard_file = File::Spec->rel2abs("$tmpdir/sqlite.scoreboard");

  my $log_file = File::Spec->rel2abs('tests.log');

  # Bug#3116 occurred because mod_sql was treating percent signs in user
  # (and group) names as variables to be substituted.

  my $user = 'proftpd%proftpd.org';
  my $passwd = 'test';
  my $home_dir = File::Spec->rel2abs($tmpdir);

  my $db_file = File::Spec->rel2abs("$tmpdir/proftpd.db");

  # Build up sqlite3 command to create users, groups tables and populate them
  my $db_script = File::Spec->rel2abs("$tmpdir/proftpd.sql");

  if (open(my $fh, "> $db_script")) {
    print $fh <<EOS;
CREATE TABLE users (
  userid TEXT,
  passwd TEXT,
  uid INTEGER,
  gid INTEGER,
  homedir TEXT, 
  shell TEXT
);
INSERT INTO users (userid, passwd, uid, gid, homedir, shell) VALUES ('$user', '$passwd', 500, 500, '$home_dir', '/bin/bash');

CREATE TABLE groups (
  groupname TEXT,
  gid INTEGER,
  members TEXT
);
INSERT INTO groups (groupname, gid, members) VALUES ('ftpd', 500, '$user');
INSERT INTO groups (groupname, gid, members) VALUES ('ftpadm', 501, '$user');
EOS

    unless (close($fh)) {
      die("Can't write $db_script: $!");
    }

  } else {
    die("Can't open $db_script: $!");
  }

  my $cmd = "sqlite3 $db_file < $db_script";

  if ($ENV{TEST_VERBOSE}) {
    print STDERR "Executing sqlite3: $cmd\n";
  }

  my @output = `$cmd`;
  if (scalar(@output) &&
      $ENV{TEST_VERBOSE}) {
    print STDERR "Output: ", join('', @output), "\n";
  }

  my $config = {
    PidFile => $pid_file,
    ScoreboardFile => $scoreboard_file,
    SystemLog => $log_file,

    IfModules => {
      'mod_delay.c' => {
        DelayEngine => 'off',
      },

      'mod_sql.c' => {
        SQLAuthTypes => 'plaintext',
        SQLBackend => 'sqlite3',
        SQLConnectInfo => $db_file,
        SQLLogFile => $log_file,
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

      # Bug#3116 would prevent a login from occurring, so we only need to
      # attempt a login.
      $client->login($user, $passwd, 5);
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

sub sql_bug3124 {
  my $self = shift;
  my $tmpdir = $self->{tmpdir};

  my $config_file = "$tmpdir/sqlite.conf";
  my $pid_file = File::Spec->rel2abs("$tmpdir/sqlite.pid");
  my $scoreboard_file = File::Spec->rel2abs("$tmpdir/sqlite.scoreboard");

  my $log_file = File::Spec->rel2abs('tests.log');

  my $user = 'proftpd';
  my $passwd = 'test';
  my $home_dir = File::Spec->rel2abs($tmpdir);

  my $db_file = File::Spec->rel2abs("$tmpdir/proftpd.db");

  # Build up sqlite3 command to create users, groups tables and populate them
  my $db_script = File::Spec->rel2abs("$tmpdir/proftpd.sql");

  if (open(my $fh, "> $db_script")) {
    print $fh <<EOS;
CREATE TABLE users (
  userid TEXT,
  passwd TEXT,
  uid INTEGER,
  gid INTEGER,
  homedir TEXT, 
  shell TEXT
);
INSERT INTO users (userid, passwd, uid, gid, homedir, shell) VALUES ('$user', '$passwd', 500, 500, '$home_dir', '/bin/bash');

CREATE TABLE groups (
  groupname TEXT,
  gid INTEGER,
  members TEXT
);
EOS

    unless (close($fh)) {
      die("Can't write $db_script: $!");
    }

  } else {
    die("Can't open $db_script: $!");
  }

  my $cmd = "sqlite3 $db_file < $db_script";

  if ($ENV{TEST_VERBOSE}) {
    print STDERR "Executing sqlite3: $cmd\n";
  }

  my @output = `$cmd`;
  if (scalar(@output) &&
      $ENV{TEST_VERBOSE}) {
    print STDERR "Output: ", join('', @output), "\n";
  }

  # Bug#3124 occurred when SQLNegativeCache was on, and there was no group
  # info; it would cause a segfault.

  my $config = {
    PidFile => $pid_file,
    ScoreboardFile => $scoreboard_file,
    SystemLog => $log_file,

    IfModules => {
      'mod_delay.c' => {
        DelayEngine => 'off',
      },

      'mod_sql.c' => {
        SQLAuthTypes => 'plaintext',
        SQLBackend => 'sqlite3',
        SQLConnectInfo => $db_file,
        SQLLogFile => $log_file,
        SQLNegativeCache => 'on',
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

      # Bug#3124 would prevent a login from occurring, so we only need to
      # attempt a login.
      $client->login($user, $passwd, 5);
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

sub sql_sqlite_bug3126 {
  my $self = shift;
  my $tmpdir = $self->{tmpdir};

  my $config_file = "$tmpdir/sqlite.conf";
  my $pid_file = File::Spec->rel2abs("$tmpdir/sqlite.pid");
  my $scoreboard_file = File::Spec->rel2abs("$tmpdir/sqlite.scoreboard");

  my $log_file = File::Spec->rel2abs('tests.log');

  my $user = 'proftpd';
  my $passwd = 'test';
  my $home_dir = File::Spec->rel2abs($tmpdir);

  my $db_file = File::Spec->rel2abs("$tmpdir/proftpd.db");

  # Build up sqlite3 command to create users, groups tables and populate them
  my $db_script = File::Spec->rel2abs("$tmpdir/proftpd.sql");

  if (open(my $fh, "> $db_script")) {
    print $fh <<EOS;
CREATE TABLE users (
  userid TEXT,
  passwd TEXT,
  uid INTEGER,
  gid INTEGER,
  homedir TEXT, 
  shell TEXT
);
INSERT INTO users (userid, passwd, uid, gid, homedir, shell) VALUES ('$user', '$passwd', 500, 500, '$home_dir', '/bin/bash');

CREATE TABLE groups (
  groupname TEXT,
  gid INTEGER,
  members TEXT
);
INSERT INTO groups (groupname, gid, members) VALUES ('ftpd', 500, '$user');
INSERT INTO groups (groupname, gid, members) VALUES ('ftpadm', 501, '$user');
EOS

    unless (close($fh)) {
      die("Can't write $db_script: $!");
    }

  } else {
    die("Can't open $db_script: $!");
  }

  my $cmd = "sqlite3 $db_file < $db_script";

  if ($ENV{TEST_VERBOSE}) {
    print STDERR "Executing sqlite3: $cmd\n";
  }

  my @output = `$cmd`;
  if (scalar(@output) &&
      $ENV{TEST_VERBOSE}) {
    print STDERR "Output: ", join('', @output), "\n";
  }

  my $config = {
    PidFile => $pid_file,
    ScoreboardFile => $scoreboard_file,
    SystemLog => $log_file,

    IfModules => {
      'mod_delay.c' => {
        DelayEngine => 'off',
      },

      'mod_sql.c' => {
        SQLAuthTypes => 'plaintext',
        SQLBackend => 'sqlite3',
        SQLConnectInfo => $db_file,
        SQLLogFile => $log_file,
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

      # Bug#3126 would prevent a login from occurring, so we only need to
      # attempt a login.
      $client->login($user, $passwd, 5);
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

sub sql_user_where_clause_ok {
  my $self = shift;
  my $tmpdir = $self->{tmpdir};

  my $config_file = "$tmpdir/sqlite.conf";
  my $pid_file = File::Spec->rel2abs("$tmpdir/sqlite.pid");
  my $scoreboard_file = File::Spec->rel2abs("$tmpdir/sqlite.scoreboard");

  my $log_file = File::Spec->rel2abs('tests.log');

  my $user1 = 'proftpd';
  my $user2 = 'proftpd2';
  my $passwd = 'test';
  my $home_dir = File::Spec->rel2abs($tmpdir);

  my $db_file = File::Spec->rel2abs("$tmpdir/proftpd.db");

  # Build up sqlite3 command to create users, groups tables and populate them
  my $db_script = File::Spec->rel2abs("$tmpdir/proftpd.sql");

  if (open(my $fh, "> $db_script")) {
    print $fh <<EOS;
CREATE TABLE users (
  userid TEXT,
  passwd TEXT,
  uid INTEGER,
  gid INTEGER,
  homedir TEXT, 
  shell TEXT,
  allowed TEXT
);
INSERT INTO users (userid, passwd, uid, gid, homedir, shell, allowed) VALUES ('$user1', '$passwd', 500, 500, '$home_dir', '/bin/bash', 'false');
INSERT INTO users (userid, passwd, uid, gid, homedir, shell, allowed) VALUES ('$user2', '$passwd', 500, 500, '$home_dir', '/bin/bash', 'true');

CREATE TABLE groups (
  groupname TEXT,
  gid INTEGER,
  members TEXT
);
INSERT INTO groups (groupname, gid, members) VALUES ('ftpd', 500, '$user1,$user2');
EOS

    unless (close($fh)) {
      die("Can't write $db_script: $!");
    }

  } else {
    die("Can't open $db_script: $!");
  }

  my $cmd = "sqlite3 $db_file < $db_script";

  if ($ENV{TEST_VERBOSE}) {
    print STDERR "Executing sqlite3: $cmd\n";
  }

  my @output = `$cmd`;
  if (scalar(@output) &&
      $ENV{TEST_VERBOSE}) {
    print STDERR "Output: ", join('', @output), "\n";
  }

  my $config = {
    PidFile => $pid_file,
    ScoreboardFile => $scoreboard_file,
    SystemLog => $log_file,

    IfModules => {
      'mod_delay.c' => {
        DelayEngine => 'off',
      },

      'mod_sql.c' => {
        SQLAuthTypes => 'plaintext',
        SQLBackend => 'sqlite3',
        SQLConnectInfo => $db_file,
        SQLLogFile => $log_file,
        SQLUserWhereClause => '"allowed = \'true\'"',
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

      # This account should not be allowed to login based on the
      # SQLUserWhereCluase...
      eval { $client->login($user1, $passwd) };
      unless ($@) {
        die("Login for user '$user1' succeeded unexpectedly");
      }

      # ...but this one should succeed.
      $client->login($user2, $passwd);
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

sub sql_bug3149 {
  my $self = shift;
  my $tmpdir = $self->{tmpdir};

  my $config_file = "$tmpdir/sqlite.conf";
  my $pid_file = File::Spec->rel2abs("$tmpdir/sqlite.pid");
  my $scoreboard_file = File::Spec->rel2abs("$tmpdir/sqlite.scoreboard");

  my $log_file = File::Spec->rel2abs('tests.log');

  my $user1 = 'proftpd';
  my $user2 = 'proftpd2';
  my $passwd = 'test';
  my $home_dir = File::Spec->rel2abs($tmpdir);

  my $db_file = File::Spec->rel2abs("$tmpdir/proftpd.db");

  # Build up sqlite3 command to create users, groups tables and populate them
  my $db_script = File::Spec->rel2abs("$tmpdir/proftpd.sql");

  if (open(my $fh, "> $db_script")) {
    print $fh <<EOS;
CREATE TABLE users (
  userid TEXT,
  passwd TEXT,
  uid INTEGER,
  gid INTEGER,
  homedir TEXT, 
  shell TEXT,
  host TEXT
);
INSERT INTO users (userid, passwd, uid, gid, homedir, shell, host) VALUES ('$user1', '$passwd', 500, 500, '$home_dir', '/bin/bash', 'remotehost');
INSERT INTO users (userid, passwd, uid, gid, homedir, shell, host) VALUES ('$user2', '$passwd', 500, 500, '$home_dir', '/bin/bash', '127.0.0.1');

CREATE TABLE groups (
  groupname TEXT,
  gid INTEGER,
  members TEXT
);
INSERT INTO groups (groupname, gid, members) VALUES ('ftpd', 500, '$user1,$user2');
EOS

    unless (close($fh)) {
      die("Can't write $db_script: $!");
    }

  } else {
    die("Can't open $db_script: $!");
  }

  my $cmd = "sqlite3 $db_file < $db_script";

  if ($ENV{TEST_VERBOSE}) {
    print STDERR "Executing sqlite3: $cmd\n";
  }

  my @output = `$cmd`;
  if (scalar(@output) &&
      $ENV{TEST_VERBOSE}) {
    print STDERR "Output: ", join('', @output), "\n";
  }

  my $config = {
    PidFile => $pid_file,
    ScoreboardFile => $scoreboard_file,
    SystemLog => $log_file,

    IfModules => {
      'mod_delay.c' => {
        DelayEngine => 'off',
      },

      # Bug#3149 occurred because mod_sql's resolve_short_tag() function
      # was not able to resolve %V properly (it was deferencing a bad pointer).
      'mod_sql.c' => {
        SQLAuthTypes => 'plaintext',
        SQLBackend => 'sqlite3',
        SQLConnectInfo => $db_file,
        SQLLogFile => $log_file,
        SQLUserWhereClause => '"host=\'%V\'"',
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

      # This account should not be allowed to login based on the
      # SQLUserWhereCluase...
      eval { $client->login($user1, $passwd) };
      unless ($@) {
        die("Login for user '$user1' succeeded unexpectedly");
      }

      # ...but this one should succeed.
      $client->login($user2, $passwd);
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

sub get_sessions {
  my $db_file = shift;
  my $where = shift;

  my $sql = "SELECT user, ip_addr, timestamp FROM ftpsessions";
  if ($where) {
    $sql .= " WHERE $where";
  }

  my $cmd = "sqlite3 $db_file \"$sql\"";

  if ($ENV{TEST_VERBOSE}) {
    print STDERR "Executing sqlite3: $cmd\n";
  }

  my $res = join('', `$cmd`);
  chomp($res);

  # The default sqlite3 delimiter is '|'
  return split(/\|/, $res);
}

sub sql_sqllog {
  my $self = shift;
  my $tmpdir = $self->{tmpdir};

  my $config_file = "$tmpdir/sqlite.conf";
  my $pid_file = File::Spec->rel2abs("$tmpdir/sqlite.pid");
  my $scoreboard_file = File::Spec->rel2abs("$tmpdir/sqlite.scoreboard");

  my $log_file = File::Spec->rel2abs('tests.log');

  my $auth_user_file = File::Spec->rel2abs("$tmpdir/sqlite.passwd");
  my $auth_group_file = File::Spec->rel2abs("$tmpdir/sqlite.group");

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

  my $db_file = File::Spec->rel2abs("$tmpdir/proftpd.db");

  # Build up sqlite3 command to create users, groups tables and populate them
  my $db_script = File::Spec->rel2abs("$tmpdir/proftpd.sql");

  if (open(my $fh, "> $db_script")) {
    print $fh <<EOS;
CREATE TABLE ftpsessions (
  user TEXT,
  ip_addr TEXT,
  timestamp TEXT
);
EOS

    unless (close($fh)) {
      die("Can't write $db_script: $!");
    }

  } else {
    die("Can't open $db_script: $!");
  }

  my $cmd = "sqlite3 $db_file < $db_script";

  if ($ENV{TEST_VERBOSE}) {
    print STDERR "Executing sqlite3: $cmd\n";
  }

  my @output = `$cmd`;
  if (scalar(@output) &&
      $ENV{TEST_VERBOSE}) {
    print STDERR "Output: ", join('', @output), "\n";
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

      'mod_sql.c' => {
        SQLEngine => 'log',
        SQLBackend => 'sqlite3',
        SQLConnectInfo => $db_file,
        SQLLogFile => $log_file,
        SQLNamedQuery => 'session_start FREEFORM "INSERT INTO ftpsessions (user, ip_addr, timestamp) VALUES (\'%u\', \'%L\', \'%{time:%Y-%m-%d %H:%M:%S}\')"',
        SQLLog => 'PASS session_start',
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

  my ($login, $ip_addr, $timestamp) = get_sessions($db_file,
    "user = \'$user\'");

  my $expected;

  $expected = $user;
  $self->assert($expected eq $login,
    test_msg("Expected '$expected', got '$login'"));

  $expected = '127.0.0.1';
  $self->assert($expected eq $ip_addr,
    test_msg("Expected '$expected', got '$ip_addr'"));

  $expected = '\d{4}\-\d{2}\-\d{2} \d{2}:\d{2}:\d{2}';
  $self->assert(qr/$expected/, $timestamp,
    test_msg("Expected '$expected', got '$timestamp'"));

  unlink($log_file);
}

sub sql_sqlite_sqllog_with_chroot {
  my $self = shift;
  my $tmpdir = $self->{tmpdir};

  my $config_file = "$tmpdir/sqlite.conf";
  my $pid_file = File::Spec->rel2abs("$tmpdir/sqlite.pid");
  my $scoreboard_file = File::Spec->rel2abs("$tmpdir/sqlite.scoreboard");

  my $log_file = File::Spec->rel2abs('tests.log');

  my $auth_user_file = File::Spec->rel2abs("$tmpdir/sqlite.passwd");
  my $auth_group_file = File::Spec->rel2abs("$tmpdir/sqlite.group");

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

  my $tmp_dir = File::Spec->rel2abs("$tmpdir/tmp");
  mkpath($tmp_dir);
  if ($< == 0) {
    unless (chmod(0777, $tmp_dir)) {
      die("Can't set perms on $tmp_dir to 0777: $!");
    }
  }

  auth_user_write($auth_user_file, $user, $passwd, $uid, $gid, $home_dir,
    '/bin/bash');
  auth_group_write($auth_group_file, 'ftpd', $gid, $user);

  my $db_file = File::Spec->rel2abs("$tmpdir/proftpd.db");

  # Build up sqlite3 command to create users, groups tables and populate them
  my $db_script = File::Spec->rel2abs("$tmpdir/proftpd.sql");

  if (open(my $fh, "> $db_script")) {
    print $fh <<EOS;
CREATE TABLE ftpsessions (
  user TEXT,
  ip_addr TEXT,
  timestamp TEXT
);
EOS

    unless (close($fh)) {
      die("Can't write $db_script: $!");
    }

  } else {
    die("Can't open $db_script: $!");
  }

  my $cmd = "sqlite3 $db_file < $db_script";

  if ($ENV{TEST_VERBOSE}) {
    print STDERR "Executing sqlite3: $cmd\n";
  }

  my @output = `$cmd`;
  if (scalar(@output) &&
      $ENV{TEST_VERBOSE}) {
    print STDERR "Output: ", join('', @output), "\n";
  }

  my $config = {
    PidFile => $pid_file,
    ScoreboardFile => $scoreboard_file,
    SystemLog => $log_file,

    AuthUserFile => $auth_user_file,
    AuthGroupFile => $auth_group_file,
    DefaultRoot => '~',

    IfModules => {
      'mod_delay.c' => {
        DelayEngine => 'off',
      },

      'mod_sql.c' => {
        SQLEngine => 'log',
        SQLBackend => 'sqlite3',
        SQLConnectInfo => $db_file,
        SQLLogFile => $log_file,
        SQLNamedQuery => 'session_start FREEFORM "INSERT INTO ftpsessions (user, ip_addr, timestamp) VALUES (\'%u\', \'%L\', \'%{time:%Y-%m-%d %H:%M:%S}\')"',
        SQLLog => 'PASS session_start',
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

  my ($login, $ip_addr, $timestamp) = get_sessions($db_file,
    "user = \'$user\'");

  my $expected;

  $expected = $user;
  $self->assert($expected eq $login,
    test_msg("Expected '$expected', got '$login'"));

  $expected = '127.0.0.1';
  $self->assert($expected eq $ip_addr,
    test_msg("Expected '$expected', got '$ip_addr'"));

  $expected = '\d{4}\-\d{2}\-\d{2} \d{2}:\d{2}:\d{2}';
  $self->assert(qr/$expected/, $timestamp,
    test_msg("Expected '$expected', got '$timestamp'"));

  unlink($log_file);
}

sub sql_custom_user_info {
  my $self = shift;
  my $tmpdir = $self->{tmpdir};

  my $config_file = "$tmpdir/sqlite.conf";
  my $pid_file = File::Spec->rel2abs("$tmpdir/sqlite.pid");
  my $scoreboard_file = File::Spec->rel2abs("$tmpdir/sqlite.scoreboard");

  my $log_file = File::Spec->rel2abs('tests.log');

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

  my $db_file = File::Spec->rel2abs("$tmpdir/proftpd.db");

  # Build up sqlite3 command to create users, groups tables and populate them
  my $db_script = File::Spec->rel2abs("$tmpdir/proftpd.sql");

  if (open(my $fh, "> $db_script")) {
    print $fh <<EOS;
CREATE TABLE ftpusers (
  userid TEXT,
  passwd TEXT,
  uid INTEGER,
  gid INTEGER,
  homedir TEXT, 
  shell TEXT,
  lastdir TEXT
);
INSERT INTO ftpusers (userid, passwd, uid, gid, homedir, shell) VALUES ('$user', '$passwd', 500, 500, '$home_dir', '/bin/bash');

CREATE TABLE groups (
  groupname TEXT,
  gid INTEGER,
  members TEXT
);
INSERT INTO groups (groupname, gid, members) VALUES ('ftpd', 500, '$user');
EOS

    unless (close($fh)) {
      die("Can't write $db_script: $!");
    }

  } else {
    die("Can't open $db_script: $!");
  }

  my $cmd = "sqlite3 $db_file < $db_script";

  if ($ENV{TEST_VERBOSE}) {
    print STDERR "Executing sqlite3: $cmd\n";
  }

  my @output = `$cmd`;
  if (scalar(@output) &&
      $ENV{TEST_VERBOSE}) {
    print STDERR "Output: ", join('', @output), "\n";
  }

  my $config = {
    PidFile => $pid_file,
    ScoreboardFile => $scoreboard_file,
    SystemLog => $log_file,

    IfModules => {
      'mod_delay.c' => {
        DelayEngine => 'off',
      },

      'mod_sql.c' => {
        SQLAuthTypes => 'plaintext',
        SQLBackend => 'sqlite3',
        SQLConnectInfo => $db_file,
        SQLLogFile => $log_file,
        SQLNamedQuery => 'get-sql-user SELECT "userid, passwd, uid, gid, homedir, shell FROM ftpusers WHERE userid = \'%U\'"',
        SQLUserInfo => 'custom:/get-sql-user',
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

      my $resp_msgs = $client->response_msgs();
      my $nmsgs = scalar(@$resp_msgs);

      my $expected;

      $expected = 1;
      $self->assert($expected == $nmsgs,
        test_msg("Expected $expected, got $nmsgs")); 

      $expected = "User proftpd logged in";
      $self->assert($expected eq $resp_msgs->[0],
        test_msg("Expected '$expected', got '$resp_msgs->[0]'"));

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

sub sql_userset_bug2434 {
  my $self = shift;
  my $tmpdir = $self->{tmpdir};

  my $config_file = "$tmpdir/sqlite.conf";
  my $pid_file = File::Spec->rel2abs("$tmpdir/sqlite.pid");
  my $scoreboard_file = File::Spec->rel2abs("$tmpdir/sqlite.scoreboard");

  my $log_file = File::Spec->rel2abs('tests.log');

  my $user = 'proftpd';
  my $passwd = 'test';
  my $home_dir = File::Spec->rel2abs($tmpdir);
  my $uid = 500;
  my $gid = 500;

  my $db_file = File::Spec->rel2abs("$tmpdir/proftpd.db");

  # Build up sqlite3 command to create users, groups tables and populate them
  my $db_script = File::Spec->rel2abs("$tmpdir/proftpd.sql");

  if (open(my $fh, "> $db_script")) {
    print $fh <<EOS;
CREATE TABLE ftpusers (
  userid TEXT,
  passwd TEXT,
  uid INTEGER,
  gid INTEGER,
  homedir TEXT, 
  shell TEXT,
  lastdir TEXT
);
INSERT INTO ftpusers (userid, passwd, uid, gid, homedir, shell) VALUES ('foo', '$passwd', 500, 500, '$home_dir', '/bin/bash');
INSERT INTO ftpusers (userid, passwd, uid, gid, homedir, shell) VALUES ('bar', '$passwd', 500, 500, '$home_dir', '/bin/bash');
INSERT INTO ftpusers (userid, passwd, uid, gid, homedir, shell) VALUES ('$user', '$passwd', 500, 500, '$home_dir', '/bin/bash');

CREATE TABLE groups (
  groupname TEXT,
  gid INTEGER,
  members TEXT
);
INSERT INTO groups (groupname, gid, members) VALUES ('ftpd', 500, '$user');
EOS

    unless (close($fh)) {
      die("Can't write $db_script: $!");
    }

  } else {
    die("Can't open $db_script: $!");
  }

  my $cmd = "sqlite3 $db_file < $db_script";

  if ($ENV{TEST_VERBOSE}) {
    print STDERR "Executing sqlite3: $cmd\n";
  }

  my @output = `$cmd`;
  if (scalar(@output) &&
      $ENV{TEST_VERBOSE}) {
    print STDERR "Output: ", join('', @output), "\n";
  }

  my $config = {
    PidFile => $pid_file,
    ScoreboardFile => $scoreboard_file,
    SystemLog => $log_file,

    # This bug requires that a chroot occur in order to be tickled.
    DefaultRoot => '~',

    IfModules => {
      'mod_delay.c' => {
        DelayEngine => 'off',
      },

      'mod_sql.c' => [
        'SQLAuthenticate users userset groups',
        'SQLAuthTypes plaintext',
        'SQLBackend sqlite3',
        "SQLConnectInfo $db_file",
        "SQLLogFile $log_file",
        'SQLNamedQuery get-all-users SELECT "userid FROM ftpusers"',
        'SQLNamedQuery get-user-by-name SELECT "userid, passwd, uid, gid, homedir, shell FROM ftpusers WHERE userid = \'%U\'"',
        'SQLNamedQuery get-user-by-id SELECT "userid, passwd, uid, gid, homedir, shell FROM ftpusers WHERE uid = %{0}"',
        'SQLUserInfo custom:/get-user-by-name/get-user-by-id/get-all-users',
      ],
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

      my $resp_msgs = $client->response_msgs();
      my $nmsgs = scalar(@$resp_msgs);

      my $expected;

      $expected = 1;
      $self->assert($expected == $nmsgs,
        test_msg("Expected $expected, got $nmsgs")); 

      $expected = "User proftpd logged in";
      $self->assert($expected eq $resp_msgs->[0],
        test_msg("Expected '$expected', got '$resp_msgs->[0]'"));

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

sub sql_usersetfast_bug2434 {
  my $self = shift;
  my $tmpdir = $self->{tmpdir};

  my $config_file = "$tmpdir/sqlite.conf";
  my $pid_file = File::Spec->rel2abs("$tmpdir/sqlite.pid");
  my $scoreboard_file = File::Spec->rel2abs("$tmpdir/sqlite.scoreboard");

  my $log_file = File::Spec->rel2abs('tests.log');

  my $user = 'proftpd';
  my $passwd = 'test';
  my $home_dir = File::Spec->rel2abs($tmpdir);
  my $uid = 500;
  my $gid = 500;

  my $db_file = File::Spec->rel2abs("$tmpdir/proftpd.db");

  # Build up sqlite3 command to create users, groups tables and populate them
  my $db_script = File::Spec->rel2abs("$tmpdir/proftpd.sql");

  if (open(my $fh, "> $db_script")) {
    print $fh <<EOS;
CREATE TABLE ftpusers (
  userid TEXT,
  passwd TEXT,
  uid INTEGER,
  gid INTEGER,
  homedir TEXT, 
  shell TEXT,
  lastdir TEXT
);
INSERT INTO ftpusers (userid, passwd, uid, gid, homedir, shell) VALUES ('foo', '$passwd', 500, 500, '$home_dir', '/bin/bash');
INSERT INTO ftpusers (userid, passwd, uid, gid, homedir, shell) VALUES ('bar', '$passwd', 500, 500, '$home_dir', '/bin/bash');
INSERT INTO ftpusers (userid, passwd, uid, gid, homedir, shell) VALUES ('$user', '$passwd', 500, 500, '$home_dir', '/bin/bash');

CREATE TABLE groups (
  groupname TEXT,
  gid INTEGER,
  members TEXT
);
INSERT INTO groups (groupname, gid, members) VALUES ('ftpd', 500, '$user');
EOS

    unless (close($fh)) {
      die("Can't write $db_script: $!");
    }

  } else {
    die("Can't open $db_script: $!");
  }

  my $cmd = "sqlite3 $db_file < $db_script";

  if ($ENV{TEST_VERBOSE}) {
    print STDERR "Executing sqlite3: $cmd\n";
  }

  my @output = `$cmd`;
  if (scalar(@output) &&
      $ENV{TEST_VERBOSE}) {
    print STDERR "Output: ", join('', @output), "\n";
  }

  my $config = {
    PidFile => $pid_file,
    ScoreboardFile => $scoreboard_file,
    SystemLog => $log_file,

    # This bug requires that a chroot occur in order to be tickled.
    DefaultRoot => '~',

    IfModules => {
      'mod_delay.c' => {
        DelayEngine => 'off',
      },

      'mod_sql.c' => [
        'SQLAuthenticate users usersetfast groups',
        'SQLAuthTypes plaintext',
        'SQLBackend sqlite3',
        "SQLConnectInfo $db_file",
        "SQLLogFile $log_file",
        'SQLNamedQuery get-all-usernames SELECT "userid FROM ftpusers"',
        'SQLNamedQuery get-all-users SELECT "userid, passwd, uid, gid, homedir, shell FROM ftpusers"',
        'SQLNamedQuery get-user-info SELECT "userid, passwd, uid, gid, homedir, shell FROM ftpusers WHERE userid = \'%U\'"',
        'SQLUserInfo custom:/get-user-info/get-all-usernames/get-all-users',
      ],
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

      my $resp_msgs = $client->response_msgs();
      my $nmsgs = scalar(@$resp_msgs);

      my $expected;

      $expected = 1;
      $self->assert($expected == $nmsgs,
        test_msg("Expected $expected, got $nmsgs")); 

      $expected = "User proftpd logged in";
      $self->assert($expected eq $resp_msgs->[0],
        test_msg("Expected '$expected', got '$resp_msgs->[0]'"));

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

sub sql_custom_group_info_bug3043 {
  my $self = shift;
  my $tmpdir = $self->{tmpdir};

  my $config_file = "$tmpdir/sqlite.conf";
  my $pid_file = File::Spec->rel2abs("$tmpdir/sqlite.pid");
  my $scoreboard_file = File::Spec->rel2abs("$tmpdir/sqlite.scoreboard");

  my $log_file = File::Spec->rel2abs('tests.log');

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

  my $db_file = File::Spec->rel2abs("$tmpdir/proftpd.db");

  # Build up sqlite3 command to create users, groups tables and populate them
  my $db_script = File::Spec->rel2abs("$tmpdir/proftpd.sql");

  if (open(my $fh, "> $db_script")) {
    print $fh <<EOS;
CREATE TABLE users (
  userid TEXT,
  passwd TEXT,
  uid INTEGER,
  gid INTEGER,
  homedir TEXT, 
  shell TEXT
);
INSERT INTO users (userid, passwd, uid, gid, homedir, shell) VALUES ('$user', '$passwd', 500, 500, '$home_dir', '/bin/bash');

CREATE TABLE ftpgroups (
  groupname TEXT,
  gid INTEGER,
  members TEXT
);
INSERT INTO ftpgroups (groupname, gid, members) VALUES ('ftpd', 500, '$user');
EOS

    unless (close($fh)) {
      die("Can't write $db_script: $!");
    }

  } else {
    die("Can't open $db_script: $!");
  }

  my $cmd = "sqlite3 $db_file < $db_script";

  if ($ENV{TEST_VERBOSE}) {
    print STDERR "Executing sqlite3: $cmd\n";
  }

  my @output = `$cmd`;
  if (scalar(@output) &&
      $ENV{TEST_VERBOSE}) {
    print STDERR "Output: ", join('', @output), "\n";
  }

  my $config = {
    PidFile => $pid_file,
    ScoreboardFile => $scoreboard_file,
    SystemLog => $log_file,

    IfModules => {
      'mod_delay.c' => {
        DelayEngine => 'off',
      },

      'mod_sql.c' => [
        'SQLMinID 0',
        'SQLAuthenticate users groups',
        'SQLAuthTypes plaintext',
        'SQLBackend sqlite3',
        "SQLConnectInfo $db_file",
        "SQLLogFile $log_file",
        'SQLNamedQuery get-group-by-name SELECT "groupname, gid, members FROM ftpgroups WHERE groupname = \'%{0}\'"',
        'SQLNamedQuery get-group-by-id SELECT "groupname, gid, members FROM ftpgroups WHERE gid = %{0}"',
        'SQLNamedQuery get-members-by-user SELECT "members FROM ftpgroups WHERE (members LIKE \'%%,%{0},%%\' OR members LIKE \'%{0},%%\' OR members LIKE \'%%,%{0}\')"',
        'SQLGroupInfo custom:/get-group-by-name/get-group-by-id/get-members-by-user',
      ],
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

      my $resp_msgs = $client->response_msgs();
      my $nmsgs = scalar(@$resp_msgs);

      my $expected;

      $expected = 1;
      $self->assert($expected == $nmsgs,
        test_msg("Expected $expected, got $nmsgs")); 

      $expected = "User proftpd logged in";
      $self->assert($expected eq $resp_msgs->[0],
        test_msg("Expected '$expected', got '$resp_msgs->[0]'"));

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

sub sql_groupset_bug3043 {
  my $self = shift;
  my $tmpdir = $self->{tmpdir};

  my $config_file = "$tmpdir/sqlite.conf";
  my $pid_file = File::Spec->rel2abs("$tmpdir/sqlite.pid");
  my $scoreboard_file = File::Spec->rel2abs("$tmpdir/sqlite.scoreboard");

  my $log_file = File::Spec->rel2abs('tests.log');

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

  my $db_file = File::Spec->rel2abs("$tmpdir/proftpd.db");

  # Build up sqlite3 command to create users, groups tables and populate them
  my $db_script = File::Spec->rel2abs("$tmpdir/proftpd.sql");

  if (open(my $fh, "> $db_script")) {
    print $fh <<EOS;
CREATE TABLE users (
  userid TEXT,
  passwd TEXT,
  uid INTEGER,
  gid INTEGER,
  homedir TEXT, 
  shell TEXT
);
INSERT INTO users (userid, passwd, uid, gid, homedir, shell) VALUES ('$user', '$passwd', 500, 500, '$home_dir', '/bin/bash');

CREATE TABLE ftpgroups (
  groupname TEXT,
  gid INTEGER,
  members TEXT
);
INSERT INTO ftpgroups (groupname, gid, members) VALUES ('ftpd', 500, '$user');
EOS

    unless (close($fh)) {
      die("Can't write $db_script: $!");
    }

  } else {
    die("Can't open $db_script: $!");
  }

  my $cmd = "sqlite3 $db_file < $db_script";

  if ($ENV{TEST_VERBOSE}) {
    print STDERR "Executing sqlite3: $cmd\n";
  }

  my @output = `$cmd`;
  if (scalar(@output) &&
      $ENV{TEST_VERBOSE}) {
    print STDERR "Output: ", join('', @output), "\n";
  }

  my $config = {
    PidFile => $pid_file,
    ScoreboardFile => $scoreboard_file,
    SystemLog => $log_file,

    IfModules => {
      'mod_delay.c' => {
        DelayEngine => 'off',
      },

      'mod_sql.c' => [
        'SQLMinID 0',
        'SQLAuthenticate users groups groupset',
        'SQLAuthTypes plaintext',
        'SQLBackend sqlite3',
        "SQLConnectInfo $db_file",
        "SQLLogFile $log_file",
        'SQLNamedQuery get-group-by-name SELECT "groupname, gid, members FROM ftpgroups WHERE groupname = \'%{0}\'"',
        'SQLNamedQuery get-group-by-id SELECT "groupname, gid, members FROM ftpgroups WHERE gid = %{0}"',
        'SQLNamedQuery get-members-by-user SELECT "members FROM ftpgroups WHERE (members LIKE \'%%,%{0},%%\' OR members LIKE \'%{0},%%\' OR members LIKE \'%%,%{0}\')"',
        'SQLNamedQuery get-all-groupnames SELECT "groupname FROM ftpgroups"',
        'SQLGroupInfo custom:/get-group-by-name/get-group-by-id/get-members-by-user/get-all-groupnames',
      ],
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

      my $resp_msgs = $client->response_msgs();
      my $nmsgs = scalar(@$resp_msgs);

      my $expected;

      $expected = 1;
      $self->assert($expected == $nmsgs,
        test_msg("Expected $expected, got $nmsgs")); 

      $expected = "User proftpd logged in";
      $self->assert($expected eq $resp_msgs->[0],
        test_msg("Expected '$expected', got '$resp_msgs->[0]'"));

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

sub sql_groupsetfast_bug3043 {
  my $self = shift;
  my $tmpdir = $self->{tmpdir};

  my $config_file = "$tmpdir/sqlite.conf";
  my $pid_file = File::Spec->rel2abs("$tmpdir/sqlite.pid");
  my $scoreboard_file = File::Spec->rel2abs("$tmpdir/sqlite.scoreboard");

  my $log_file = File::Spec->rel2abs('tests.log');

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

  my $db_file = File::Spec->rel2abs("$tmpdir/proftpd.db");

  # Build up sqlite3 command to create users, groups tables and populate them
  my $db_script = File::Spec->rel2abs("$tmpdir/proftpd.sql");

  if (open(my $fh, "> $db_script")) {
    print $fh <<EOS;
CREATE TABLE users (
  userid TEXT,
  passwd TEXT,
  uid INTEGER,
  gid INTEGER,
  homedir TEXT, 
  shell TEXT
);
INSERT INTO users (userid, passwd, uid, gid, homedir, shell) VALUES ('$user', '$passwd', 500, 500, '$home_dir', '/bin/bash');

CREATE TABLE ftpgroups (
  groupname TEXT,
  gid INTEGER,
  members TEXT
);
INSERT INTO ftpgroups (groupname, gid, members) VALUES ('ftpd', 500, '$user');
EOS

    unless (close($fh)) {
      die("Can't write $db_script: $!");
    }

  } else {
    die("Can't open $db_script: $!");
  }

  my $cmd = "sqlite3 $db_file < $db_script";

  if ($ENV{TEST_VERBOSE}) {
    print STDERR "Executing sqlite3: $cmd\n";
  }

  my @output = `$cmd`;
  if (scalar(@output) &&
      $ENV{TEST_VERBOSE}) {
    print STDERR "Output: ", join('', @output), "\n";
  }

  my $config = {
    PidFile => $pid_file,
    ScoreboardFile => $scoreboard_file,
    SystemLog => $log_file,

    IfModules => {
      'mod_delay.c' => {
        DelayEngine => 'off',
      },

      'mod_sql.c' => [
        'SQLMinID 0',
        'SQLAuthenticate users groups groupsetfast',
        'SQLAuthTypes plaintext',
        'SQLBackend sqlite3',
        "SQLConnectInfo $db_file",
        "SQLLogFile $log_file",
        'SQLNamedQuery get-group-by-name SELECT "groupname, gid, members FROM ftpgroups WHERE groupname = \'%{0}\'"',
        'SQLNamedQuery get-group-by-id SELECT "groupname, gid, members FROM ftpgroups WHERE gid = %{0}"',
        'SQLNamedQuery get-group-by-member SELECT "groupname, gid, members FROM ftpgroups WHERE (members LIKE \'%%,%{0},%%\' OR members LIKE \'%{0},%%\' OR members LIKE \'%%,%{0}\')"',
        'SQLNamedQuery get-all-groupnames SELECT "groupname FROM ftpgroups"',
        'SQLNamedQuery get-all-groups SELECT "groupname, gid, members FROM ftpgroups"',
        'SQLGroupInfo custom:/get-group-by-name/get-group-by-id/get-group-by-member/get-all-groupnames/get-all-groups',
      ],
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

      my $resp_msgs = $client->response_msgs();
      my $nmsgs = scalar(@$resp_msgs);

      my $expected;

      $expected = 1;
      $self->assert($expected == $nmsgs,
        test_msg("Expected $expected, got $nmsgs")); 

      $expected = "User proftpd logged in";
      $self->assert($expected eq $resp_msgs->[0],
        test_msg("Expected '$expected', got '$resp_msgs->[0]'"));

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

sub get_renames {
  my $db_file = shift;
  my $where = shift;

  my $sql = "SELECT user, ip_addr, rename_from FROM ftpsessions";
  if ($where) {
    $sql .= " WHERE $where";
  }

  my $cmd = "sqlite3 $db_file \"$sql\"";

  if ($ENV{TEST_VERBOSE}) {
    print STDERR "Executing sqlite3: $cmd\n";
  }

  my $res = join('', `$cmd`);
  chomp($res);

  # The default sqlite3 delimiter is '|'
  return split(/\|/, $res);
}

sub sql_sqllog_var_w {
  my $self = shift;
  my $tmpdir = $self->{tmpdir};

  my $config_file = "$tmpdir/sqlite.conf";
  my $pid_file = File::Spec->rel2abs("$tmpdir/sqlite.pid");
  my $scoreboard_file = File::Spec->rel2abs("$tmpdir/sqlite.scoreboard");

  my $log_file = File::Spec->rel2abs('tests.log');

  my $auth_user_file = File::Spec->rel2abs("$tmpdir/sqlite.passwd");
  my $auth_group_file = File::Spec->rel2abs("$tmpdir/sqlite.group");

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

  my $db_file = File::Spec->rel2abs("$tmpdir/proftpd.db");

  # Build up sqlite3 command to create users, groups tables and populate them
  my $db_script = File::Spec->rel2abs("$tmpdir/proftpd.sql");

  if (open(my $fh, "> $db_script")) {
    print $fh <<EOS;
CREATE TABLE ftpsessions (
  user TEXT,
  ip_addr TEXT,
  rename_from TEXT
);
EOS

    unless (close($fh)) {
      die("Can't write $db_script: $!");
    }

  } else {
    die("Can't open $db_script: $!");
  }

  my $cmd = "sqlite3 $db_file < $db_script";

  if ($ENV{TEST_VERBOSE}) {
    print STDERR "Executing sqlite3: $cmd\n";
  }

  my @output = `$cmd`;
  if (scalar(@output) &&
      $ENV{TEST_VERBOSE}) {
    print STDERR "Output: ", join('', @output), "\n";
  }

  my $src_file = File::Spec->rel2abs("$tmpdir/test.txt");
  if (open(my $fh, "> $src_file")) {
    close($fh);

  } else {
    die("Can't open $src_file: $!");
  }

  my $dst_file = File::Spec->rel2abs("$tmpdir/foo.txt");

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

      'mod_sql.c' => {
        SQLEngine => 'log',
        SQLBackend => 'sqlite3',
        SQLConnectInfo => $db_file,
        SQLLogFile => $log_file,
        SQLNamedQuery => 'rename FREEFORM "INSERT INTO ftpsessions (user, ip_addr, rename_from) VALUES (\'%u\', \'%L\', \'%w\')"',
        SQLLog => 'RNTO rename',
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
      $client->rnfr('test.txt');
      $client->rnto('foo.txt');
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

  my ($login, $ip_addr, $rnfr_path) = get_renames($db_file, "user = \'$user\'");

  my $expected;

  $expected = $user;
  $self->assert($expected eq $login,
    test_msg("Expected '$expected', got '$login'"));

  $expected = '127.0.0.1';
  $self->assert($expected eq $ip_addr,
    test_msg("Expected '$expected', got '$ip_addr'"));

  $expected = $src_file;
  $self->assert($expected eq $rnfr_path,
    test_msg("Expected '$expected', got '$rnfr_path'"));

  unlink($log_file);
}

sub sql_openssl_auth_type_md5 {
  my $self = shift;
  my $tmpdir = $self->{tmpdir};

  my $config_file = "$tmpdir/sqlite.conf";
  my $pid_file = File::Spec->rel2abs("$tmpdir/sqlite.pid");
  my $scoreboard_file = File::Spec->rel2abs("$tmpdir/sqlite.scoreboard");

  my $log_file = File::Spec->rel2abs('tests.log');

  my $user = 'proftpd';
  my $passwd = '{md5}X03MO1qnZdYdgyfeuILPmQ==';
  my $home_dir = File::Spec->rel2abs($tmpdir);
  my $uid = 500;
  my $gid = 500;

  my $db_file = File::Spec->rel2abs("$tmpdir/proftpd.db");

  # Build up sqlite3 command to create users, groups tables and populate them
  my $db_script = File::Spec->rel2abs("$tmpdir/proftpd.sql");

  if (open(my $fh, "> $db_script")) {
    print $fh <<EOS;
CREATE TABLE users (
  userid TEXT,
  passwd TEXT,
  uid INTEGER,
  gid INTEGER,
  homedir TEXT, 
  shell TEXT,
  lastdir TEXT
);
INSERT INTO users (userid, passwd, uid, gid, homedir, shell) VALUES ('$user', '$passwd', 500, 500, '$home_dir', '/bin/bash');

CREATE TABLE groups (
  groupname TEXT,
  gid INTEGER,
  members TEXT
);
INSERT INTO groups (groupname, gid, members) VALUES ('ftpd', 500, '$user');
EOS

    unless (close($fh)) {
      die("Can't write $db_script: $!");
    }

  } else {
    die("Can't open $db_script: $!");
  }

  my $cmd = "sqlite3 $db_file < $db_script";

  if ($ENV{TEST_VERBOSE}) {
    print STDERR "Executing sqlite3: $cmd\n";
  }

  my @output = `$cmd`;
  if (scalar(@output) &&
      $ENV{TEST_VERBOSE}) {
    print STDERR "Output: ", join('', @output), "\n";
  }

  my $config = {
    PidFile => $pid_file,
    ScoreboardFile => $scoreboard_file,
    SystemLog => $log_file,

    IfModules => {
      'mod_delay.c' => {
        DelayEngine => 'off',
      },

      'mod_sql.c' => {
        SQLAuthTypes => 'OpenSSL',
        SQLBackend => 'sqlite3',
        SQLConnectInfo => $db_file,
        SQLLogFile => $log_file,
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

      $client->login($user, 'password');

      my $resp_msgs = $client->response_msgs();
      my $nmsgs = scalar(@$resp_msgs);

      my $expected;

      $expected = 1;
      $self->assert($expected == $nmsgs,
        test_msg("Expected $expected, got $nmsgs")); 

      $expected = "User proftpd logged in";
      $self->assert($expected eq $resp_msgs->[0],
        test_msg("Expected '$expected', got '$resp_msgs->[0]'"));

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

sub sql_openssl_auth_type_sha1 {
  my $self = shift;
  my $tmpdir = $self->{tmpdir};

  my $config_file = "$tmpdir/sqlite.conf";
  my $pid_file = File::Spec->rel2abs("$tmpdir/sqlite.pid");
  my $scoreboard_file = File::Spec->rel2abs("$tmpdir/sqlite.scoreboard");

  my $log_file = File::Spec->rel2abs('tests.log');

  my $user = 'proftpd';
  my $passwd = '{sha1}W6ph5Mm5Pz8GgiULbPgzG37mj9g=';
  my $home_dir = File::Spec->rel2abs($tmpdir);
  my $uid = 500;
  my $gid = 500;

  my $db_file = File::Spec->rel2abs("$tmpdir/proftpd.db");

  # Build up sqlite3 command to create users, groups tables and populate them
  my $db_script = File::Spec->rel2abs("$tmpdir/proftpd.sql");

  if (open(my $fh, "> $db_script")) {
    print $fh <<EOS;
CREATE TABLE users (
  userid TEXT,
  passwd TEXT,
  uid INTEGER,
  gid INTEGER,
  homedir TEXT, 
  shell TEXT,
  lastdir TEXT
);
INSERT INTO users (userid, passwd, uid, gid, homedir, shell) VALUES ('$user', '$passwd', 500, 500, '$home_dir', '/bin/bash');

CREATE TABLE groups (
  groupname TEXT,
  gid INTEGER,
  members TEXT
);
INSERT INTO groups (groupname, gid, members) VALUES ('ftpd', 500, '$user');
EOS

    unless (close($fh)) {
      die("Can't write $db_script: $!");
    }

  } else {
    die("Can't open $db_script: $!");
  }

  my $cmd = "sqlite3 $db_file < $db_script";

  if ($ENV{TEST_VERBOSE}) {
    print STDERR "Executing sqlite3: $cmd\n";
  }

  my @output = `$cmd`;
  if (scalar(@output) &&
      $ENV{TEST_VERBOSE}) {
    print STDERR "Output: ", join('', @output), "\n";
  }

  my $config = {
    PidFile => $pid_file,
    ScoreboardFile => $scoreboard_file,
    SystemLog => $log_file,

    IfModules => {
      'mod_delay.c' => {
        DelayEngine => 'off',
      },

      'mod_sql.c' => {
        SQLAuthTypes => 'OpenSSL',
        SQLBackend => 'sqlite3',
        SQLConnectInfo => $db_file,
        SQLLogFile => $log_file,
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

      $client->login($user, 'password');

      my $resp_msgs = $client->response_msgs();
      my $nmsgs = scalar(@$resp_msgs);

      my $expected;

      $expected = 1;
      $self->assert($expected == $nmsgs,
        test_msg("Expected $expected, got $nmsgs")); 

      $expected = "User proftpd logged in";
      $self->assert($expected eq $resp_msgs->[0],
        test_msg("Expected '$expected', got '$resp_msgs->[0]'"));

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

sub get_resp_mesgs {
  my $db_file = shift;
  my $where = shift;

  my $sql = "SELECT user, ip_addr, resp_mesg FROM ftpsessions";
  if ($where) {
    $sql .= " WHERE $where";
  }

  my $cmd = "sqlite3 $db_file \"$sql\"";

  if ($ENV{TEST_VERBOSE}) {
    print STDERR "Executing sqlite3: $cmd\n";
  }

  my $res = join('', `$cmd`);
  chomp($res);

  # The default sqlite3 delimiter is '|'
  return split(/\|/, $res);
}

sub sql_sqllog_var_S {
  my $self = shift;
  my $tmpdir = $self->{tmpdir};

  my $config_file = "$tmpdir/sqlite.conf";
  my $pid_file = File::Spec->rel2abs("$tmpdir/sqlite.pid");
  my $scoreboard_file = File::Spec->rel2abs("$tmpdir/sqlite.scoreboard");

  my $log_file = File::Spec->rel2abs('tests.log');

  my $auth_user_file = File::Spec->rel2abs("$tmpdir/sqlite.passwd");
  my $auth_group_file = File::Spec->rel2abs("$tmpdir/sqlite.group");

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

  my $db_file = File::Spec->rel2abs("$tmpdir/proftpd.db");

  # Build up sqlite3 command to create users, groups tables and populate them
  my $db_script = File::Spec->rel2abs("$tmpdir/proftpd.sql");

  if (open(my $fh, "> $db_script")) {
    print $fh <<EOS;
CREATE TABLE ftpsessions (
  user TEXT,
  ip_addr TEXT,
  resp_mesg TEXT
);
EOS

    unless (close($fh)) {
      die("Can't write $db_script: $!");
    }

  } else {
    die("Can't open $db_script: $!");
  }

  my $cmd = "sqlite3 $db_file < $db_script";

  if ($ENV{TEST_VERBOSE}) {
    print STDERR "Executing sqlite3: $cmd\n";
  }

  my @output = `$cmd`;
  if (scalar(@output) &&
      $ENV{TEST_VERBOSE}) {
    print STDERR "Output: ", join('', @output), "\n";
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

      'mod_sql.c' => {
        SQLEngine => 'log',
        SQLBackend => 'sqlite3',
        SQLConnectInfo => $db_file,
        SQLLogFile => $log_file,
        SQLNamedQuery => 'info FREEFORM "INSERT INTO ftpsessions (user, ip_addr, resp_mesg) VALUES (\'%u\', \'%L\', \'%S\')"',
        SQLLog => 'LIST info',
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

  my ($login, $ip_addr, $resp_mesg) = get_resp_mesgs($db_file,
    "user = \'$user\'");

  my $expected;

  $expected = $user;
  $self->assert($expected eq $login,
    test_msg("Expected '$expected', got '$login'"));

  $expected = '127.0.0.1';
  $self->assert($expected eq $ip_addr,
    test_msg("Expected '$expected', got '$ip_addr'"));

  $expected = 'Transfer complete';
  $self->assert($expected eq $resp_mesg,
    test_msg("Expected '$expected', got '$resp_mesg'"));

  unlink($log_file);
}

sub sql_sqllog_var_S_err {
  my $self = shift;
  my $tmpdir = $self->{tmpdir};

  my $config_file = "$tmpdir/sqlite.conf";
  my $pid_file = File::Spec->rel2abs("$tmpdir/sqlite.pid");
  my $scoreboard_file = File::Spec->rel2abs("$tmpdir/sqlite.scoreboard");

  my $log_file = File::Spec->rel2abs('tests.log');

  my $auth_user_file = File::Spec->rel2abs("$tmpdir/sqlite.passwd");
  my $auth_group_file = File::Spec->rel2abs("$tmpdir/sqlite.group");

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

  my $db_file = File::Spec->rel2abs("$tmpdir/proftpd.db");

  # Build up sqlite3 command to create users, groups tables and populate them
  my $db_script = File::Spec->rel2abs("$tmpdir/proftpd.sql");

  if (open(my $fh, "> $db_script")) {
    print $fh <<EOS;
CREATE TABLE ftpsessions (
  user TEXT,
  ip_addr TEXT,
  resp_mesg TEXT
);
EOS

    unless (close($fh)) {
      die("Can't write $db_script: $!");
    }

  } else {
    die("Can't open $db_script: $!");
  }

  my $cmd = "sqlite3 $db_file < $db_script";

  if ($ENV{TEST_VERBOSE}) {
    print STDERR "Executing sqlite3: $cmd\n";
  }

  my @output = `$cmd`;
  if (scalar(@output) &&
      $ENV{TEST_VERBOSE}) {
    print STDERR "Output: ", join('', @output), "\n";
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

      'mod_sql.c' => {
        SQLEngine => 'log',
        SQLBackend => 'sqlite3',
        SQLConnectInfo => $db_file,
        SQLLogFile => $log_file,
        SQLNamedQuery => 'info FREEFORM "INSERT INTO ftpsessions (user, ip_addr, resp_mesg) VALUES (\'%u\', \'%L\', \'%S\')"',
        SQLLog => 'ERR_* info',
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
      eval { $client->quote('CLNT', 'NcFTP 3.2.2 linux-x86_64-glibc2.3') };
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

  my ($login, $ip_addr, $resp_mesg) = get_resp_mesgs($db_file,
    "user = \'$user\'");

  my $expected;

  $expected = $user;
  $self->assert($expected eq $login,
    test_msg("Expected '$expected', got '$login'"));

  $expected = '127.0.0.1';
  $self->assert($expected eq $ip_addr,
    test_msg("Expected '$expected', got '$ip_addr'"));

  $expected = 'CLNT not understood';
  $self->assert($expected eq $resp_mesg,
    test_msg("Expected '$expected', got '$resp_mesg'"));

  unlink($log_file);
}

sub sql_negative_cache_bug3282 {
  my $self = shift;
  my $tmpdir = $self->{tmpdir};

  my $config_file = "$tmpdir/sqlite.conf";
  my $pid_file = File::Spec->rel2abs("$tmpdir/sqlite.pid");
  my $scoreboard_file = File::Spec->rel2abs("$tmpdir/sqlite.scoreboard");

  my $log_file = File::Spec->rel2abs('tests.log');

  my $user = 'proftpd';
  my $passwd = 'test';
  my $home_dir = File::Spec->rel2abs($tmpdir);
  my $uid = 500;
  my $gid = 500;

  my $db_file = File::Spec->rel2abs("$tmpdir/proftpd.db");

  # Build up sqlite3 command to create users, groups tables and populate them
  my $db_script = File::Spec->rel2abs("$tmpdir/proftpd.sql");

  if (open(my $fh, "> $db_script")) {
    print $fh <<EOS;
CREATE TABLE users (
  userid TEXT,
  passwd TEXT,
  uid INTEGER,
  gid INTEGER,
  homedir TEXT, 
  shell TEXT,
  lastdir TEXT
);
INSERT INTO users (userid, passwd, uid, gid, homedir, shell) VALUES ('$user', '$passwd', 500, 500, '$home_dir', '/bin/bash');

CREATE TABLE groups (
  groupname TEXT,
  gid INTEGER,
  members TEXT
);
INSERT INTO groups (groupname, gid, members) VALUES ('ftpd', 500, '$user');
EOS

    unless (close($fh)) {
      die("Can't write $db_script: $!");
    }

  } else {
    die("Can't open $db_script: $!");
  }

  my $cmd = "sqlite3 $db_file < $db_script";

  if ($ENV{TEST_VERBOSE}) {
    print STDERR "Executing sqlite3: $cmd\n";
  }

  my @output = `$cmd`;
  if (scalar(@output) &&
      $ENV{TEST_VERBOSE}) {
    print STDERR "Output: ", join('', @output), "\n";
  }

  my $config = {
    PidFile => $pid_file,
    ScoreboardFile => $scoreboard_file,
    SystemLog => $log_file,
    TraceLog => $log_file,
    Trace => 'auth:10',

    IfModules => {
      'mod_delay.c' => {
        DelayEngine => 'off',
      },

      'mod_sql.c' => {
        SQLAuthTypes => 'plaintext',
        SQLBackend => 'sqlite3',
        SQLConnectInfo => $db_file,
        SQLLogFile => $log_file,
        SQLNegativeCache => 'on',
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

      eval { $client->login('foo', 'foo') };
      unless ($@) {
        die("Login succeeded unexpectedly");
      }

      $client->login($user, $passwd);

      my $expected;

      my $resp_msgs = $client->response_msgs();
      my $nmsgs = scalar(@$resp_msgs);

      $expected = 1;
      $self->assert($expected == $nmsgs,
        test_msg("Expected $expected, got $nmsgs")); 

      $expected = "User proftpd logged in";
      $self->assert($expected eq $resp_msgs->[0],
        test_msg("Expected '$expected', got '$resp_msgs->[0]'"));
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

sub sql_sqllog_var_T_rnfr {
  my $self = shift;
  my $tmpdir = $self->{tmpdir};

  my $config_file = "$tmpdir/sqlite.conf";
  my $pid_file = File::Spec->rel2abs("$tmpdir/sqlite.pid");
  my $scoreboard_file = File::Spec->rel2abs("$tmpdir/sqlite.scoreboard");

  my $log_file = File::Spec->rel2abs('tests.log');

  my $auth_user_file = File::Spec->rel2abs("$tmpdir/sqlite.passwd");
  my $auth_group_file = File::Spec->rel2abs("$tmpdir/sqlite.group");

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

  my $db_file = File::Spec->rel2abs("$tmpdir/proftpd.db");

  # Build up sqlite3 command to create users, groups tables and populate them
  my $db_script = File::Spec->rel2abs("$tmpdir/proftpd.sql");

  if (open(my $fh, "> $db_script")) {
    print $fh <<EOS;
CREATE TABLE ftpsessions (
  user TEXT,
  ip_addr TEXT,
  rename_from TEXT,
  rename_time FLOAT
);
EOS

    unless (close($fh)) {
      die("Can't write $db_script: $!");
    }

  } else {
    die("Can't open $db_script: $!");
  }

  my $cmd = "sqlite3 $db_file < $db_script";

  if ($ENV{TEST_VERBOSE}) {
    print STDERR "Executing sqlite3: $cmd\n";
  }

  my @output = `$cmd`;
  if (scalar(@output) &&
      $ENV{TEST_VERBOSE}) {
    print STDERR "Output: ", join('', @output), "\n";
  }

  my $src_file = File::Spec->rel2abs("$tmpdir/test.txt");
  if (open(my $fh, "> $src_file")) {
    close($fh);

  } else {
    die("Can't open $src_file: $!");
  }

  my $dst_file = File::Spec->rel2abs("$tmpdir/foo.txt");

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

      'mod_sql.c' => {
        SQLEngine => 'log',
        SQLBackend => 'sqlite3',
        SQLConnectInfo => $db_file,
        SQLLogFile => $log_file,
        SQLNamedQuery => 'rename FREEFORM "INSERT INTO ftpsessions (user, ip_addr, rename_from, rename_time) VALUES (\'%u\', \'%L\', \'%w\', %T)"',
        SQLLog => 'RNTO rename',
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
      $client->rnfr('test.txt');
      $client->rnto('foo.txt');
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

  my $query = "SELECT user, ip_addr, rename_from, rename_time FROM ftpsessions WHERE user = \'$user\'";
  $cmd = "sqlite3 $db_file \"$query\"";

  if ($ENV{TEST_VERBOSE}) {
    print STDERR "Executing sqlite3: $cmd\n";
  }

  my $res = join('', `$cmd`);
  chomp($res);

  my ($login, $ip_addr, $rnfr_path, $rename_time) = split(/\|/, $res);

  my $expected;

  $expected = $user;
  $self->assert($expected eq $login,
    test_msg("Expected '$expected', got '$login'"));

  $expected = '127.0.0.1';
  $self->assert($expected eq $ip_addr,
    test_msg("Expected '$expected', got '$ip_addr'"));

  $expected = $src_file;
  $self->assert($expected eq $rnfr_path,
    test_msg("Expected '$expected', got '$rnfr_path'"));

  $expected = '0.0';
  $self->assert($expected eq $rename_time,
    test_msg("Expected '$expected', got '$rename_time'"));

  unlink($log_file);
}

sub sql_sqllog_var_T_retr {
  my $self = shift;
  my $tmpdir = $self->{tmpdir};

  my $config_file = "$tmpdir/sqlite.conf";
  my $pid_file = File::Spec->rel2abs("$tmpdir/sqlite.pid");
  my $scoreboard_file = File::Spec->rel2abs("$tmpdir/sqlite.scoreboard");

  my $log_file = File::Spec->rel2abs('tests.log');

  my $auth_user_file = File::Spec->rel2abs("$tmpdir/sqlite.passwd");
  my $auth_group_file = File::Spec->rel2abs("$tmpdir/sqlite.group");

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

  my $db_file = File::Spec->rel2abs("$tmpdir/proftpd.db");

  # Build up sqlite3 command to create users, groups tables and populate them
  my $db_script = File::Spec->rel2abs("$tmpdir/proftpd.sql");

  if (open(my $fh, "> $db_script")) {
    print $fh <<EOS;
CREATE TABLE ftpsessions (
  user TEXT,
  ip_addr TEXT,
  download_time FLOAT
);
EOS

    unless (close($fh)) {
      die("Can't write $db_script: $!");
    }

  } else {
    die("Can't open $db_script: $!");
  }

  my $cmd = "sqlite3 $db_file < $db_script";

  if ($ENV{TEST_VERBOSE}) {
    print STDERR "Executing sqlite3: $cmd\n";
  }

  my @output = `$cmd`;
  if (scalar(@output) &&
      $ENV{TEST_VERBOSE}) {
    print STDERR "Output: ", join('', @output), "\n";
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

      'mod_sql.c' => {
        SQLEngine => 'log',
        SQLBackend => 'sqlite3',
        SQLConnectInfo => $db_file,
        SQLLogFile => $log_file,
        SQLNamedQuery => 'download FREEFORM "INSERT INTO ftpsessions (user, ip_addr, download_time) VALUES (\'%u\', \'%L\', %T)"',
        SQLLog => 'RETR download',
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

      my $conn = $client->retr_raw('sqlite.conf');
      unless ($conn) {
        die("RETR sqlite.conf failed: " . $client->response_code() . " " .
          $client->response_msg());
      }

      my $buf;
      while ($conn->read($buf, 8192)) {
      }
      $conn->close();

      my $resp_code = $client->response_code();
      my $resp_mesg = $client->response_msg();

      my $expected;

      $expected = 226;
      $self->assert($expected == $resp_code,
        test_msg("Expected $expected, got $resp_code"));

      $expected = 'Transfer complete';
      $self->assert($expected eq $resp_mesg,
        test_msg("Expected '$expected', got '$resp_mesg'"));

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

  my $query = "SELECT user, ip_addr, download_time FROM ftpsessions WHERE user = \'$user\'";
  $cmd = "sqlite3 $db_file \"$query\"";

  if ($ENV{TEST_VERBOSE}) {
    print STDERR "Executing sqlite3: $cmd\n";
  }

  my $res = join('', `$cmd`);
  chomp($res);

  my ($login, $ip_addr, $download_time) = split(/\|/, $res);

  my $expected;

  $expected = $user;
  $self->assert($expected eq $login,
    test_msg("Expected '$expected', got '$login'"));

  $expected = '127.0.0.1';
  $self->assert($expected eq $ip_addr,
    test_msg("Expected '$expected', got '$ip_addr'"));

  $self->assert($download_time > 0.0,
    test_msg("Expected > 0.0, got $download_time"));

  unlink($log_file);
}

sub sql_sqllog_exit {
  my $self = shift;
  my $tmpdir = $self->{tmpdir};

  my $config_file = "$tmpdir/sqlite.conf";
  my $pid_file = File::Spec->rel2abs("$tmpdir/sqlite.pid");
  my $scoreboard_file = File::Spec->rel2abs("$tmpdir/sqlite.scoreboard");

  my $log_file = File::Spec->rel2abs('tests.log');

  my $auth_user_file = File::Spec->rel2abs("$tmpdir/sqlite.passwd");
  my $auth_group_file = File::Spec->rel2abs("$tmpdir/sqlite.group");

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

  my $db_file = File::Spec->rel2abs("$tmpdir/proftpd.db");

  # Build up sqlite3 command to create users, groups tables and populate them
  my $db_script = File::Spec->rel2abs("$tmpdir/proftpd.sql");

  if (open(my $fh, "> $db_script")) {
    print $fh <<EOS;
CREATE TABLE ftpsessions (
  user TEXT,
  ip_addr TEXT
);
EOS

    unless (close($fh)) {
      die("Can't write $db_script: $!");
    }

  } else {
    die("Can't open $db_script: $!");
  }

  my $cmd = "sqlite3 $db_file < $db_script";

  if ($ENV{TEST_VERBOSE}) {
    print STDERR "Executing sqlite3: $cmd\n";
  }

  my @output = `$cmd`;
  if (scalar(@output) &&
      $ENV{TEST_VERBOSE}) {
    print STDERR "Output: ", join('', @output), "\n";
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

      'mod_sql.c' => {
        SQLEngine => 'log',
        SQLBackend => 'sqlite3',
        SQLConnectInfo => $db_file,
        SQLLogFile => $log_file,
        SQLNamedQuery => 'logout FREEFORM "INSERT INTO ftpsessions (user, ip_addr) VALUES (\'%u\', \'%L\')"',
        SQLLog => 'EXIT logout',
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

  my $query = "SELECT user, ip_addr FROM ftpsessions WHERE user = \'$user\'";
  $cmd = "sqlite3 $db_file \"$query\"";

  if ($ENV{TEST_VERBOSE}) {
    print STDERR "Executing sqlite3: $cmd\n";
  }

  my $res = join('', `$cmd`);
  chomp($res);

  my ($login, $ip_addr) = split(/\|/, $res);

  my $expected;

  $expected = $user;
  $self->assert($expected eq $login,
    test_msg("Expected '$expected', got '$login'"));

  $expected = '127.0.0.1';
  $self->assert($expected eq $ip_addr,
    test_msg("Expected '$expected', got '$ip_addr'"));

  unlink($log_file);
}

1;
