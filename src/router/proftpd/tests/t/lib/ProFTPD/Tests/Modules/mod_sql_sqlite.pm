package ProFTPD::Tests::Modules::mod_sql_sqlite;

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

  sql_user_where_clause_with_vars_ok => {
    order => ++$order,
    test_class => [qw(forking)],
  },

  sql_group_where_clause_ok => {
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

  sql_sqllog_var_w_chrooted => {
    order => ++$order,
    test_class => [qw(forking rootprivs)],
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

  sql_sqllog_var_d_bug3395 => {
    order => ++$order,
    test_class => [qw(bug forking)],
  },

  sql_sqllog_var_d_chroot_bug3395 => {
    order => ++$order,
    test_class => [qw(bug forking rootprivs)],
  },

  sql_sqllog_var_uid_gid_bug3390 => {
    order => ++$order,
    test_class => [qw(bug forking)],
  },

  sql_sqlite_auth_type_backend_bug3511 => {
    order => ++$order,
    test_class => [qw(bug forking)],
  },

  sql_sqllog_pass_ok_var_s_bug3528 => {
    order => ++$order,
    test_class => [qw(bug forking)],
  },

  sql_sqllog_pass_failed_var_s_bug3528 => {
    order => ++$order,
    test_class => [qw(bug forking)],
  },

  sql_sqlshowinfo_pass_bug3423 => {
    order => ++$order,
    test_class => [qw(bug forking)],
  },

  sql_sqlshowinfo_list_bug3423 => {
    order => ++$order,
    test_class => [qw(bug forking)],
  },

  sql_multiple_users_shared_uid_gid => {
    order => ++$order,
    test_class => [qw(bug forking)],
  },

  sql_resolve_tag_bug3536 => {
    order => ++$order,
    test_class => [qw(bug forking inprogress)],
  },

  sql_sqllog_vars_I_O_bug3554 => {
    order => ++$order,
    test_class => [qw(bug forking)],
  },

  sql_sqllog_note_var_unique_id_bug3572 => {
    order => ++$order,
    test_class => [qw(bug forking mod_unique_id)],
  },

  sql_sqllog_note_var_rewrite_bug3572 => {
    order => ++$order,
    test_class => [qw(bug forking mod_rewrite)],
  },

  sql_sqllog_note_sql_user_info => {
    order => ++$order,
    test_class => [qw(forking)],
  },

  sql_sqllog_var_E => {
    order => ++$order,
    test_class => [qw(forking)],
  },

  sql_named_conn_bug3262 => {
    order => ++$order,
    test_class => [qw(bug forking)],
  },

  sql_named_conn_sqllog_exit_bug3645 => {
    order => ++$order,
    test_class => [qw(bug forking rootprivs)],
  },

  sql_sqllog_vars_H_L_matching_server_bug3620 => {
    order => ++$order,
    test_class => [qw(bug forking)],
  },

  sql_sqllog_vars_H_L_default_server_bug3620 => {
    order => ++$order,
    test_class => [qw(bug forking)],
  },

  sql_sqllog_exit_ifuser => {
    order => ++$order,
    test_class => [qw(forking mod_ifsession)],
  },

  sql_sqllog_exit_ifgroup => {
    order => ++$order,
    test_class => [qw(forking mod_ifsession)],
  },

  sql_opt_no_disconnect_on_error_with_extlog_bug3633 => {
    order => ++$order,
    test_class => [qw(bug forking)],
  },

  sql_sqllog_ignore_errors_bad_table_bug3692 => {
    order => ++$order,
    test_class => [qw(bug forking)],
  },

  sql_sqllog_ignore_errors_bad_db_bug3692 => {
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

sub sql_bug2045 {
  my $self = shift;
  my $tmpdir = $self->{tmpdir};

  my $config_file = "$tmpdir/sqlite.conf";
  my $pid_file = File::Spec->rel2abs("$tmpdir/sqlite.pid");
  my $scoreboard_file = File::Spec->rel2abs("$tmpdir/sqlite.scoreboard");

  my $log_file = File::Spec->rel2abs('tests.log');

  my $user = 'proftpd';
  my $passwd = 'test';
  my $group = 'ftpd';
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
INSERT INTO users (userid, passwd, uid, gid, homedir, shell) VALUES ('$user', '$passwd', $uid, $gid, '$home_dir', '/bin/bash');

CREATE TABLE groups (
  groupname TEXT,
  gid INTEGER,
  members TEXT
);
INSERT INTO groups (groupname, gid, members) VALUES ('$group', $gid, '$user');
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

  # Make sure that, if we're running as root, the database file has
  # the permissions/privs set for use by proftpd
  if ($< == 0) {
    unless (chmod(0666, $db_file)) {
      die("Can't set perms on $db_file to 0666: $!");
    }
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

  # Make sure that, if we're running as root, the database file has
  # the permissions/privs set for use by proftpd
  if ($< == 0) {
    unless (chmod(0666, $db_file)) {
      die("Can't set perms on $db_file to 0666: $!");
    }
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

  # Make sure that, if we're running as root, the database file has
  # the permissions/privs set for use by proftpd
  if ($< == 0) {
    unless (chmod(0666, $db_file)) {
      die("Can't set perms on $db_file to 0666: $!");
    }
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

  # Make sure that, if we're running as root, the database file has
  # the permissions/privs set for use by proftpd
  if ($< == 0) {
    unless (chmod(0666, $db_file)) {
      die("Can't set perms on $db_file to 0666: $!");
    }
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

  # Make sure that, if we're running as root, the database file has
  # the permissions/privs set for use by proftpd
  if ($< == 0) {
    unless (chmod(0666, $db_file)) {
      die("Can't set perms on $db_file to 0666: $!");
    }
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

  # Make sure that, if we're running as root, the database file has
  # the permissions/privs set for use by proftpd
  if ($< == 0) {
    unless (chmod(0666, $db_file)) {
      die("Can't set perms on $db_file to 0666: $!");
    }
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
      # SQLUserWhereClause...
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

sub sql_user_where_clause_with_vars_ok {
  my $self = shift;
  my $tmpdir = $self->{tmpdir};

  my $config_file = "$tmpdir/sqlite.conf";
  my $pid_file = File::Spec->rel2abs("$tmpdir/sqlite.pid");
  my $scoreboard_file = File::Spec->rel2abs("$tmpdir/sqlite.scoreboard");

  my $log_file = File::Spec->rel2abs('tests.log');

  my $user1 = 'proftpd';
  my $user2 = 'proftpd2';
  my $passwd = 'test';
  my $group = 'ftpd';
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
  allowed_ip TEXT
);
INSERT INTO users (userid, passwd, uid, gid, homedir, shell, allowed_ip) VALUES ('$user1', '$passwd', $uid, $gid, '$home_dir', '/bin/bash', '1.2.3.4');
INSERT INTO users (userid, passwd, uid, gid, homedir, shell, allowed_ip) VALUES ('$user2', '$passwd', $uid, $gid, '$home_dir', '/bin/bash', '127.0.0.1');

CREATE TABLE groups (
  groupname TEXT,
  gid INTEGER,
  members TEXT
);
INSERT INTO groups (groupname, gid, members) VALUES ('$group', $gid, '$user1,$user2');
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

  # Make sure that, if we're running as root, the database file has
  # the permissions/privs set for use by proftpd
  if ($< == 0) {
    unless (chmod(0666, $db_file)) {
      die("Can't set perms on $db_file to 0666: $!");
    }
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
        SQLUserWhereClause => '"allowed_ip = \'%L\'"',
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
      # SQLUserWhereClause...
      eval { $client->login($user1, $passwd) };
      unless ($@) {
        die("Login for user '$user1' succeeded unexpectedly");
      }

      # ...but this one should succeed.
      $client->login($user2, $passwd);
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

sub sql_group_where_clause_ok {
  my $self = shift;
  my $tmpdir = $self->{tmpdir};

  my $config_file = "$tmpdir/sqlite.conf";
  my $pid_file = File::Spec->rel2abs("$tmpdir/sqlite.pid");
  my $scoreboard_file = File::Spec->rel2abs("$tmpdir/sqlite.scoreboard");

  my $log_file = File::Spec->rel2abs('tests.log');

  my $user = 'proftpd';
  my $passwd = 'test';
  my $group = 'ftpd';
  my $home_dir = File::Spec->rel2abs($tmpdir);
  my $uid = 500;
  my $gid= 500;

  my $acl_group = 'whitelist';

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
INSERT INTO users (userid, passwd, uid, gid, homedir, shell) VALUES ('$user', '$passwd', $uid, $gid, '$home_dir', '/bin/bash');

CREATE TABLE groups (
  groupname TEXT,
  gid INTEGER,
  members TEXT,
  allowed TEXT
);
INSERT INTO groups (groupname, gid, members, allowed) VALUES ('$group', $gid, '$user', 'false');
INSERT INTO groups (groupname, gid, members, allowed) VALUES ('$acl_group', $gid, '$user', 'true');
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

  # Make sure that, if we're running as root, the database file has
  # the permissions/privs set for use by proftpd
  if ($< == 0) {
    unless (chmod(0666, $db_file)) {
      die("Can't set perms on $db_file to 0666: $!");
    }
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
        AuthOrder => 'mod_sql.c',

        SQLAuthenticate => 'users groups',
        SQLAuthTypes => 'plaintext',
        SQLBackend => 'sqlite3',
        SQLConnectInfo => $db_file,
        SQLLogFile => $log_file,
        SQLGroupWhereClause => '"allowed = \'true\'"',
        SQLOptions => 'useNormalizedGroupSchema',
      },
    },
  };

  my ($port, $config_user, $config_group) = config_write($config_file, $config);

  if (open(my $fh, ">> $config_file")) {
    print $fh <<EOC;

<Limit LOGIN>
  AllowGroup $acl_group
  DenyAll
</Limit>
EOC
    unless (close($fh)) {
      die("Can't write $config_file: $!");
    }

  } else {
    die("Can't open $config_file: $!");
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

  # Make sure that, if we're running as root, the database file has
  # the permissions/privs set for use by proftpd
  if ($< == 0) {
    unless (chmod(0666, $db_file)) {
      die("Can't set perms on $db_file to 0666: $!");
    }
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
      # SQLUserWhereClause...
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
  my $group = 'ftpd';
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
  auth_group_write($auth_group_file, $group, $gid, $user);

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

  # Make sure that, if we're running as root, the database file has
  # the permissions/privs set for use by proftpd
  if ($< == 0) {
    unless (chmod(0666, $db_file)) {
      die("Can't set perms on $db_file to 0666: $!");
    }
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

  # Make sure that, if we're running as root, the database file has
  # the permissions/privs set for use by proftpd
  if ($< == 0) {
    unless (chmod(0666, $db_file)) {
      die("Can't set perms on $db_file to 0666: $!");
    }
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
        SQLConnectInfo => "$db_file foo bar PERCONNECTION",
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
  my $group = 'ftpd';
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
INSERT INTO ftpusers (userid, passwd, uid, gid, homedir, shell) VALUES ('$user', '$passwd', $uid, $gid, '$home_dir', '/bin/bash');

CREATE TABLE groups (
  groupname TEXT,
  gid INTEGER,
  members TEXT
);
INSERT INTO groups (groupname, gid, members) VALUES ('$group', $gid, '$user');
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

  # Make sure that, if we're running as root, the database file has
  # the permissions/privs set for use by proftpd
  if ($< == 0) {
    unless (chmod(0666, $db_file)) {
      die("Can't set perms on $db_file to 0666: $!");
    }
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

  # Make sure that, if we're running as root, the database file has
  # the permissions/privs set for use by proftpd
  if ($< == 0) {
    unless (chmod(0666, $db_file)) {
      die("Can't set perms on $db_file to 0666: $!");
    }
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
  my $group = 'ftpd';
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

  # Make sure that, if we're running as root, the database file has
  # the permissions/privs set for use by proftpd
  if ($< == 0) {
    unless (chmod(0666, $db_file)) {
      die("Can't set perms on $db_file to 0666: $!");
    }
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
        'SQLNamedQuery get-user-by-name SELECT "userid, passwd, uid, gid, homedir, shell FROM ftpusers WHERE userid = \'%U\'"',
        'SQLNamedQuery get-user-by-id SELECT "userid, passwd, uid, gid, homedir, shell FROM ftpusers WHERE uid = %{0}"',
        'SQLUserInfo custom:/get-user-by-name/get-user-by-id/get-all-usernames/get-all-users',
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

  # Make sure that, if we're running as root, the database file has
  # the permissions/privs set for use by proftpd
  if ($< == 0) {
    unless (chmod(0666, $db_file)) {
      die("Can't set perms on $db_file to 0666: $!");
    }
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

  # Make sure that, if we're running as root, the database file has
  # the permissions/privs set for use by proftpd
  if ($< == 0) {
    unless (chmod(0666, $db_file)) {
      die("Can't set perms on $db_file to 0666: $!");
    }
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

  # Make sure that, if we're running as root, the database file has
  # the permissions/privs set for use by proftpd
  if ($< == 0) {
    unless (chmod(0666, $db_file)) {
      die("Can't set perms on $db_file to 0666: $!");
    }
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

  # Make sure that, if we're running as root, the database file has
  # the permissions/privs set for use by proftpd
  if ($< == 0) {
    unless (chmod(0666, $db_file)) {
      die("Can't set perms on $db_file to 0666: $!");
    }
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

sub sql_sqllog_var_w_chrooted {
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
  my $group = 'ftpd';
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
  auth_group_write($auth_group_file, $group, $gid, $user);

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

  # Make sure that, if we're running as root, the database file has
  # the permissions/privs set for use by proftpd
  if ($< == 0) {
    unless (chmod(0666, $db_file)) {
      die("Can't set perms on $db_file to 0666: $!");
    }
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
    DefaultRoot => '~',

    IfModules => {
      'mod_delay.c' => {
        DelayEngine => 'off',
      },

      'mod_sql.c' => {
        SQLEngine => 'log',
        SQLBackend => 'sqlite3',
        SQLConnectInfo => "$db_file foo bar PERCONNECTION",
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

  # Make sure that, if we're running as root, the database file has
  # the permissions/privs set for use by proftpd
  if ($< == 0) {
    unless (chmod(0666, $db_file)) {
      die("Can't set perms on $db_file to 0666: $!");
    }
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

  # Make sure that, if we're running as root, the database file has
  # the permissions/privs set for use by proftpd
  if ($< == 0) {
    unless (chmod(0666, $db_file)) {
      die("Can't set perms on $db_file to 0666: $!");
    }
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
  my $cols = shift;

  my $sql = "SELECT $cols FROM ftpsessions";
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

  # Make sure that, if we're running as root, the database file has
  # the permissions/privs set for use by proftpd
  if ($< == 0) {
    unless (chmod(0666, $db_file)) {
      die("Can't set perms on $db_file to 0666: $!");
    }
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
    "user = \'$user\'", "user, ip_addr, resp_mesg");

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

  # Make sure that, if we're running as root, the database file has
  # the permissions/privs set for use by proftpd
  if ($< == 0) {
    unless (chmod(0666, $db_file)) {
      die("Can't set perms on $db_file to 0666: $!");
    }
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
    "user = \'$user\'", "user, ip_addr, resp_mesg");

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

  # Make sure that, if we're running as root, the database file has
  # the permissions/privs set for use by proftpd
  if ($< == 0) {
    unless (chmod(0666, $db_file)) {
      die("Can't set perms on $db_file to 0666: $!");
    }
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

  # Make sure that, if we're running as root, the database file has
  # the permissions/privs set for use by proftpd
  if ($< == 0) {
    unless (chmod(0666, $db_file)) {
      die("Can't set perms on $db_file to 0666: $!");
    }
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

  # Make sure that, if we're running as root, the database file has
  # the permissions/privs set for use by proftpd
  if ($< == 0) {
    unless (chmod(0666, $db_file)) {
      die("Can't set perms on $db_file to 0666: $!");
    }
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
      while ($conn->read($buf, 8192, 30)) {
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

  # Make sure that, if we're running as root, the database file has
  # the permissions/privs set for use by proftpd
  if ($< == 0) {
    unless (chmod(0666, $db_file)) {
      die("Can't set perms on $db_file to 0666: $!");
    }
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

sub get_locations {
  my $db_file = shift;
  my $where = shift;

  my $sql = "SELECT user, ip_addr, dir FROM ftpsessions";
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

sub sql_sqllog_var_d_bug3395 {
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

  my $sub_dir = File::Spec->rel2abs("$tmpdir/foo");
  mkpath($sub_dir);

  # Make sure that, if we're running as root, that the home directory has
  # permissions/privs set for the account we create
  if ($< == 0) {
    unless (chmod(0755, $home_dir, $sub_dir)) {
      die("Can't set perms on $home_dir to 0755: $!");
    }

    unless (chown($uid, $gid, $home_dir, $sub_dir)) {
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
  dir TEXT
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

  # Make sure that, if we're running as root, the database file has
  # the permissions/privs set for use by proftpd
  if ($< == 0) {
    unless (chmod(0666, $db_file)) {
      die("Can't set perms on $db_file to 0666: $!");
    }
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
        SQLNamedQuery => 'location FREEFORM "INSERT INTO ftpsessions (user, ip_addr, dir) VALUES (\'%u\', \'%L\', \'%d\')"',
        SQLLog => 'EXIT location',
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
      $client->cwd('foo');
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

  my ($login, $ip_addr, $dir) = get_locations($db_file, "user = \'$user\'");

  my $expected;

  $expected = $user;
  $self->assert($expected eq $login,
    test_msg("Expected '$expected', got '$login'"));

  $expected = '127.0.0.1';
  $self->assert($expected eq $ip_addr,
    test_msg("Expected '$expected', got '$ip_addr'"));

  $expected = $sub_dir;
  $self->assert($expected eq $dir,
    test_msg("Expected '$expected', got '$dir'"));

  unlink($log_file);
}

sub sql_sqllog_var_d_chroot_bug3395 {
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
  my $group = 'ftpd';
  my $home_dir = File::Spec->rel2abs($tmpdir);
  my $uid = 500;
  my $gid = 500;

  my $sub_dir = File::Spec->rel2abs("$tmpdir/foo");
  mkpath($sub_dir);

  auth_user_write($auth_user_file, $user, $passwd, $uid, $gid, $home_dir,
    '/bin/bash');
  auth_group_write($auth_group_file, $group, $gid, $user);

  my $db_file = File::Spec->rel2abs("$tmpdir/proftpd.db");

  # Build up sqlite3 command to create users, groups tables and populate them
  my $db_script = File::Spec->rel2abs("$tmpdir/proftpd.sql");

  if (open(my $fh, "> $db_script")) {
    print $fh <<EOS;
CREATE TABLE ftpsessions (
  user TEXT,
  ip_addr TEXT,
  dir TEXT
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

  # Make sure that, if we're running as root, that the home directory has
  # permissions/privs set for the account we create
  if ($< == 0) {
    unless (chmod(0755, $home_dir, $sub_dir)) {
      die("Can't set perms on $home_dir to 0755: $!");
    }

    unless (chown($uid, $gid, $home_dir, $sub_dir)) {
      die("Can't set owner of $home_dir to $uid/$gid: $!");
    }

    unless (chmod(0666, $db_file)) {
      die("Can't set perms on $db_file to 0666: $!");
    }
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
        SQLConnectInfo => "$db_file foo bar PERCONNECTION",
        SQLLogFile => $log_file,
        SQLNamedQuery => 'location FREEFORM "INSERT INTO ftpsessions (user, ip_addr, dir) VALUES (\'%u\', \'%L\', \'%d\')"',
        SQLLog => 'EXIT location',
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
      $client->cwd('foo');
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

  my ($login, $ip_addr, $dir) = get_locations($db_file, "user = \'$user\'");

  my $expected;

  $expected = $user;
  $self->assert($expected eq $login,
    test_msg("Expected '$expected', got '$login'"));

  $expected = '127.0.0.1';
  $self->assert($expected eq $ip_addr,
    test_msg("Expected '$expected', got '$ip_addr'"));

  $expected = $sub_dir;
  $self->assert($expected eq $dir,
    test_msg("Expected '$expected', got '$dir'"));

  unlink($log_file);
}

sub get_ids {
  my $db_file = shift;
  my $where = shift;

  my $sql = "SELECT user, ip_addr, uid, gid FROM ftpsessions";
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

sub sql_sqllog_var_uid_gid_bug3390 {
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
  my $group = 'ftpd';
  my $home_dir = File::Spec->rel2abs($tmpdir);
  my $uid = 500;
  my $gid = 500;

  my $sub_dir = File::Spec->rel2abs("$tmpdir/foo");
  mkpath($sub_dir);

  # Make sure that, if we're running as root, that the home directory has
  # permissions/privs set for the account we create
  if ($< == 0) {
    unless (chmod(0755, $home_dir, $sub_dir)) {
      die("Can't set perms on $home_dir to 0755: $!");
    }

    unless (chown($uid, $gid, $home_dir, $sub_dir)) {
      die("Can't set owner of $home_dir to $uid/$gid: $!");
    }
  }

  auth_user_write($auth_user_file, $user, $passwd, $uid, $gid, $home_dir,
    '/bin/bash');
  auth_group_write($auth_group_file, $group, $gid, $user);

  my $db_file = File::Spec->rel2abs("$tmpdir/proftpd.db");

  # Build up sqlite3 command to create users, groups tables and populate them
  my $db_script = File::Spec->rel2abs("$tmpdir/proftpd.sql");

  if (open(my $fh, "> $db_script")) {
    print $fh <<EOS;
CREATE TABLE ftpsessions (
  user TEXT,
  ip_addr TEXT,
  uid NUMBER,
  gid NUMBER
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

  # Make sure that, if we're running as root, the database file has
  # the permissions/privs set for use by proftpd
  if ($< == 0) {
    unless (chmod(0666, $db_file)) {
      die("Can't set perms on $db_file to 0666: $!");
    }
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
        SQLConnectInfo => "$db_file foo bar PERCONNECTION",
        SQLLogFile => $log_file,
        SQLNamedQuery => 'location FREEFORM "INSERT INTO ftpsessions (user, ip_addr, uid, gid) VALUES (\'%u\', \'%L\', %{uid}, %{gid})"',
        SQLLog => 'PASS location',
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
      $client->cwd('foo');
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

  my ($login, $ip_addr, $sql_uid, $sql_gid) = get_ids($db_file, "user = \'$user\'");

  my $expected;

  $expected = $user;
  $self->assert($expected eq $login,
    test_msg("Expected '$expected', got '$login'"));

  $expected = '127.0.0.1';
  $self->assert($expected eq $ip_addr,
    test_msg("Expected '$expected', got '$ip_addr'"));

  $expected = $uid;
  $self->assert($expected == $sql_uid,
    test_msg("Expected $expected, got $sql_uid"));

  $expected = $gid;
  $self->assert($expected == $sql_gid,
    test_msg("Expected $expected, got $sql_gid"));

  unlink($log_file);
}

sub sql_sqlite_auth_type_backend_bug3511 {
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

  # Make sure that, if we're running as root, the database file has
  # the permissions/privs set for use by proftpd
  if ($< == 0) {
    unless (chmod(0666, $db_file)) {
      die("Can't set perms on $db_file to 0666: $!");
    }
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
        SQLAuthTypes => 'backend',
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

      eval { $client->login($user, $passwd) };
      unless ($@) {
        die("Login succeeded unexpectedly");
      }

      my $resp_code = $client->response_code();
      my $resp_msg = $client->response_msg();

      my $expected;

      $expected = 530;
      $self->assert($expected == $resp_code,
        test_msg("Expected $expected, got $resp_code")); 

      $expected = "Login incorrect.";
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

sub sql_sqllog_pass_ok_var_s_bug3528 {
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
  resp_code TEXT,
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

  # Make sure that, if we're running as root, the database file has
  # the permissions/privs set for use by proftpd
  if ($< == 0) {
    unless (chmod(0666, $db_file)) {
      die("Can't set perms on $db_file to 0666: $!");
    }
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
        SQLNamedQuery => 'info FREEFORM "INSERT INTO ftpsessions (user, resp_code, resp_mesg) VALUES (\'%u\', \'%s\', \'%S\')"',
        SQLLog => 'PASS,ERR_PASS info',
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

  my ($login, $resp_code, $resp_mesg) = get_resp_mesgs($db_file,
    undef, "user, resp_code, resp_mesg");

  my $expected;

  $expected = 230;
  $self->assert($expected eq $resp_code,
    test_msg("Expected '$expected', got '$resp_code'"));

  $expected = "User $user logged in";
  $self->assert($expected eq $resp_mesg,
    test_msg("Expected '$expected', got '$resp_mesg'"));

  unlink($log_file);
}

sub sql_sqllog_pass_failed_var_s_bug3528 {
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
  resp_code TEXT,
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

  # Make sure that, if we're running as root, the database file has
  # the permissions/privs set for use by proftpd
  if ($< == 0) {
    unless (chmod(0666, $db_file)) {
      die("Can't set perms on $db_file to 0666: $!");
    }
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
        SQLNamedQuery => 'info FREEFORM "INSERT INTO ftpsessions (user, resp_code, resp_mesg) VALUES (\'%u\', \'%s\', \'%S\')"',
        SQLLog => 'PASS,ERR_PASS info',
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
      eval { $client->login($user, 'foobar') };
      unless ($@) {
        die("Login succeeded unexpectedly");
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

  my ($login, $resp_code, $resp_mesg) = get_resp_mesgs($db_file,
    undef, "user, resp_code, resp_mesg");

  my $expected;

  $expected = 530;
  $self->assert($expected eq $resp_code,
    test_msg("Expected '$expected', got '$resp_code'"));

  $expected = "Login incorrect.";
  $self->assert($expected eq $resp_mesg,
    test_msg("Expected '$expected', got '$resp_mesg'"));

  unlink($log_file);
}

sub sql_sqlshowinfo_pass_bug3423 {
  my $self = shift;
  my $tmpdir = $self->{tmpdir};

  my $config_file = "$tmpdir/sqlite.conf";
  my $pid_file = File::Spec->rel2abs("$tmpdir/sqlite.pid");
  my $scoreboard_file = File::Spec->rel2abs("$tmpdir/sqlite.scoreboard");

  my $log_file = File::Spec->rel2abs('tests.log');

  my $user = 'proftpd';
  my $passwd = 'test';
  my $group = 'ftpd';
  my $home_dir = File::Spec->rel2abs($tmpdir);
  my $uid = 500;
  my $gid = 500;
  my $lastdir = '/path/to/lastdir';

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
INSERT INTO users (userid, passwd, uid, gid, homedir, shell, lastdir) VALUES ('$user', '$passwd', $uid, $gid, '$home_dir', '/bin/bash', '$lastdir');

CREATE TABLE groups (
  groupname TEXT,
  gid INTEGER,
  members TEXT
);
INSERT INTO groups (groupname, gid, members) VALUES ('$group', $gid, '$user');
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

  # Make sure that, if we're running as root, the database file has
  # the permissions/privs set for use by proftpd
  if ($< == 0) {
    unless (chmod(0666, $db_file)) {
      die("Can't set perms on $db_file to 0666: $!");
    }
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
        'AuthOrder mod_sql.c',

        'SQLAuthenticate users groups',
        'SQLAuthTypes plaintext',
        'SQLBackend sqlite3',
        "SQLConnectInfo $db_file",
        "SQLLogFile $log_file",
        'SQLMinID 100',

        'SQLNamedQuery lastdir SELECT "lastdir FROM users WHERE userid = \'%u\'"',


        # Configure the equivalent of a multiline DisplayLogin file
        # using the SQLShowInfo directive

        'SQLShowInfo PASS 230 " "',
        'SQLShowInfo PASS 230 "Greetings, %u"',
        'SQLShowInfo PASS 230 "Last directory: \"%{lastdir}\""',
        'SQLShowInfo PASS 230 "-"',
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

      $expected = 5;
      $self->assert($expected == $nmsgs,
        test_msg("Expected $expected, got $nmsgs")); 

      $expected = " ";
      $self->assert($expected eq $resp_msgs->[0],
        test_msg("Expected '$expected', got '$resp_msgs->[0]'"));

      $expected = " Greetings, $user";
      $self->assert($expected eq $resp_msgs->[1],
        test_msg("Expected '$expected', got '$resp_msgs->[1]'"));

      $expected = " Last directory: \"$lastdir\"";
      $self->assert($expected eq $resp_msgs->[2],
        test_msg("Expected '$expected', got '$resp_msgs->[2]'"));

      $expected = " -";
      $self->assert($expected eq $resp_msgs->[3],
        test_msg("Expected '$expected', got '$resp_msgs->[3]'"));

      $expected = "User $user logged in";
      $self->assert($expected eq $resp_msgs->[4],
        test_msg("Expected '$expected', got '$resp_msgs->[4]'"));

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

sub sql_sqlshowinfo_list_bug3423 {
  my $self = shift;
  my $tmpdir = $self->{tmpdir};

  my $config_file = "$tmpdir/sqlite.conf";
  my $pid_file = File::Spec->rel2abs("$tmpdir/sqlite.pid");
  my $scoreboard_file = File::Spec->rel2abs("$tmpdir/sqlite.scoreboard");

  my $log_file = File::Spec->rel2abs('tests.log');

  my $user = 'proftpd';
  my $passwd = 'test';
  my $group = 'ftpd';
  my $home_dir = File::Spec->rel2abs($tmpdir);
  my $uid = 500;
  my $gid = 500;
  my $lastdir = '/path/to/lastdir';

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
INSERT INTO users (userid, passwd, uid, gid, homedir, shell, lastdir) VALUES ('$user', '$passwd', $uid, $gid, '$home_dir', '/bin/bash', '$lastdir');

CREATE TABLE groups (
  groupname TEXT,
  gid INTEGER,
  members TEXT
);
INSERT INTO groups (groupname, gid, members) VALUES ('$group', $gid, '$user');
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

  # Make sure that, if we're running as root, the database file has
  # the permissions/privs set for use by proftpd
  if ($< == 0) {
    unless (chmod(0666, $db_file)) {
      die("Can't set perms on $db_file to 0666: $!");
    }
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
        'AuthOrder mod_sql.c',

        'SQLAuthenticate users groups',
        'SQLAuthTypes plaintext',
        'SQLBackend sqlite3',
        "SQLConnectInfo $db_file",
        "SQLLogFile $log_file",
        'SQLMinID 100',

        # Configure the equivalent of a multiline DisplayLogin file
        # using the SQLShowInfo directive

        'SQLShowInfo LIST 226 "Get that directory listing OK, %u?"',
        'SQLShowInfo LIST 226 "-"',
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

      my $resp_code = $client->response_code();
      my $resp_msg = $client->response_msg();

      my $expected;

      $expected = 230;
      $self->assert($expected == $resp_code,
        test_msg("Expected $expected, got $resp_code")); 

      $expected = "User $user logged in";
      $self->assert($expected eq $resp_msg,
        test_msg("Expected '$expected', got '$resp_msg'"));

      my $conn = $client->list_raw();
      unless ($conn) {
        die("LIST failed unexpectedly: " . $client->response_code() . " " .
          $client->response_msg());
      }

      my $buf;
      $conn->read($buf, 8192, 25);
      eval { $conn->close() };

      $resp_code = $client->response_code();

      $expected = 226;
      $self->assert($expected == $resp_code,
        test_msg("Expected $expected, got $resp_code")); 

      my $resp_msgs = $client->response_msgs();
      my $nmsgs = scalar(@$resp_msgs);

      $expected = 4;
      $self->assert($expected == $nmsgs,
        test_msg("Expected $expected, got $nmsgs"));

      $expected = "Opening ASCII mode data connection for file list";
      $self->assert($expected eq $resp_msgs->[0],
        test_msg("Expected '$expected', got '$resp_msgs->[0]'"));

      $expected = "Transfer complete";
      $self->assert($expected eq $resp_msgs->[1],
        test_msg("Expected '$expected', got '$resp_msgs->[1]'"));

      $expected = " Get that directory listing OK, $user?";
      $self->assert($expected eq $resp_msgs->[2],
        test_msg("Expected '$expected', got '$resp_msgs->[2]'"));

      $expected = "-";
      $self->assert($expected eq $resp_msgs->[3],
        test_msg("Expected '$expected', got '$resp_msgs->[3]'"));

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

sub sql_multiple_users_shared_uid_gid {
  my $self = shift;
  my $tmpdir = $self->{tmpdir};

  my $config_file = "$tmpdir/sqlite.conf";
  my $pid_file = File::Spec->rel2abs("$tmpdir/sqlite.pid");
  my $scoreboard_file = File::Spec->rel2abs("$tmpdir/sqlite.scoreboard");

  my $log_file = File::Spec->rel2abs('tests.log');

  my $user = 'proftpd';
  my $passwd = 'test';
  my $group = 'ftpd';
  my $home_dir = File::Spec->rel2abs($tmpdir);
  my $uid = 500;
  my $gid = 500;

  my $db_file = File::Spec->rel2abs("$tmpdir/proftpd.db");

  # Build up sqlite3 command to create users, groups tables and populate them
  my $db_script = File::Spec->rel2abs("$tmpdir/proftpd.sql");

  if (open(my $fh, "> $db_script")) {
    print $fh <<EOU;
CREATE TABLE users (
  userid TEXT,
  passwd TEXT,
  uid INTEGER,
  gid INTEGER,
  homedir TEXT, 
  shell TEXT
);
EOU

  for (my $i = 1; $i <= 10; $i++) {
    my $name = $user . $i;

    print $fh "INSERT INTO users (userid, passwd, uid, gid, homedir, shell) VALUES ('$name', '$passwd', $uid, $gid, '$home_dir', '/bin/bash');\n";
  }

    print $fh <<EOG;
CREATE TABLE groups (
  groupname TEXT,
  gid INTEGER,
  members TEXT
);
EOG

  my $group_members = ($user . '1');
  for (my $i = 2; $i <= 5; $i++) {
    my $name = $user . $i;
    $group_members .= ",$name";
  }

  my $group_name = ($group . '1');
  print $fh "INSERT INTO groups (groupname, gid, members) VALUES ('$group_name', $gid, '$group_members');\n";

  $group_members = ($user . '6');
  for (my $i = 7; $i <= 10; $i++) {
    my $name = $user . $i;
    $group_members .= ",$name";
  }

  $group_name = ($group . '2');
  print $fh "INSERT INTO groups (groupname, gid, members) VALUES ('$group_name', $gid, '$group_members');\n";

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

  # Make sure that, if we're running as root, the database file has
  # the permissions/privs set for use by proftpd
  if ($< == 0) {
    unless (chmod(0666, $db_file)) {
      die("Can't set perms on $db_file to 0666: $!");
    }
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
        'AuthOrder mod_sql.c',

        'SQLAuthenticate users groups',
        'SQLAuthTypes plaintext',
        'SQLBackend sqlite3',
        "SQLConnectInfo $db_file",
        "SQLLogFile $log_file",
        'SQLMinID 100',
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

      my $name = $user . '10';
      $client->login($name, $passwd);
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

sub get_cmds {
  my $db_file = shift;
  my $where = shift;

  my $sql = "SELECT user, ip_addr, command, request FROM ftpsessions";
  if ($where) {
    $sql .= " WHERE $where";
  }

  my $cmd = "sqlite3 $db_file \"$sql\"";

  if ($ENV{TEST_VERBOSE}) {
    print STDERR "Executing sqlite3: $cmd\n";
  }

  my $rows = [`$cmd`];
  my $res;

  # Return the last row found, for now
  foreach my $row (@$rows) {
    chomp($row);

    # The default sqlite3 delimiter is '|'
    $res = [split(/\|/, $row)];
  }

  return @$res;
}

sub sql_resolve_tag_bug3536 {
  my $self = shift;
  my $tmpdir = $self->{tmpdir};

  my $config_file = "$tmpdir/sqlite.conf";
  my $pid_file = File::Spec->rel2abs("$tmpdir/sqlite.pid");
  my $scoreboard_file = File::Spec->rel2abs("$tmpdir/sqlite.scoreboard");

  my $log_file = File::Spec->rel2abs('tests.log');

  my $user = 'proftpd';
  my $passwd = 'test';
  my $group = 'ftpd';
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
INSERT INTO users (userid, passwd, uid, gid, homedir, shell) VALUES ('$user', '$passwd', $uid, $gid, '$home_dir', '/bin/bash');

CREATE TABLE groups (
  groupname TEXT,
  gid INTEGER,
  members TEXT
);
INSERT INTO groups (groupname, gid, members) VALUES ('$group', $gid, '$user');

CREATE TABLE ftpsessions (
  user TEXT,
  ip_addr TEXT,
  command TEXT,
  request TEXT
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

  # Make sure that, if we're running as root, the database file has
  # the permissions/privs set for use by proftpd
  if ($< == 0) {
    unless (chmod(0666, $db_file)) {
      die("Can't set perms on $db_file to 0666: $!");
    }
  }

  my $config = {
    PidFile => $pid_file,
    ScoreboardFile => $scoreboard_file,
    SystemLog => $log_file,
    TraceLog => $log_file,
    Trace => 'sql:20',

    IfModules => {
      'mod_delay.c' => {
        DelayEngine => 'off',
      },

      'mod_sql.c' => {
        SQLAuthTypes => 'plaintext',
        SQLBackend => 'sqlite3',
        SQLConnectInfo => $db_file,
        SQLLogFile => $log_file,
        SQLNamedQuery => 'command FREEFORM "INSERT INTO ftpsessions (user, ip_addr, command, request) VALUES (\'%u\', \'%L\', \'%m\', \'%r\')"',
        SQLLog => '* command',
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

      # Ignore errors for these commands
      eval {
        my $name = "AAAAAAAAAA%m%m%mA%m%m%mA%m%mAA%m%m%m%m%mA%m%Z%m%mA%m%mAA%mA%ZAA%m%m%m%m%m%mA%m%ZAAA%m%m%m%m%m%m%m%m%m%m%m%m%m%m%mAa0Aa1Aa2Aa3Aa4Aa5Aa6Aa7Aa8Aa9Ab0Ab1Ab2Ab3Ab4Ab5Ab6Ab7Ab8Ab9Ac0A%%%m%r%m%Z";
        $client->user($name);
      };
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

  my ($login, $ip_addr, $req);
  ($login, $ip_addr, $cmd, $req) = get_cmds($db_file, "user = \'$config_user\'");

  my $expected;

  $expected = $config_user;
  $self->assert($expected eq $login,
    test_msg("Expected '$expected', got '$login'"));

  $expected = '127.0.0.1';
  $self->assert($expected eq $ip_addr,
    test_msg("Expected '$expected', got '$ip_addr'"));

  unlink($log_file);
}

sub get_session_io {
  my $db_file = shift;
  my $where = shift;

  my $sql = "SELECT user, ip_addr, bytes_in, bytes_out FROM ftpsessions";
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

sub sql_sqllog_vars_I_O_bug3554 {
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
  bytes_in NUMBER,
  bytes_out NUMBER
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

  # Make sure that, if we're running as root, the database file has
  # the permissions/privs set for use by proftpd
  if ($< == 0) {
    unless (chmod(0666, $db_file)) {
      die("Can't set perms on $db_file to 0666: $!");
    }
  }

  my $config = {
    PidFile => $pid_file,
    ScoreboardFile => $scoreboard_file,
    SystemLog => $log_file,
    TraceLog => $log_file,
    Trace => 'DEFAULT:10',

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
        SQLNamedQuery => 'session_io FREEFORM "INSERT INTO ftpsessions (user, ip_addr, bytes_in, bytes_out) VALUES (\'%u\', \'%L\', %I, %O)"',
        SQLLog => 'EXIT session_io',
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
      $client->type('ascii');

      my $conn = $client->stor_raw('test.txt');
      unless ($conn) {
        die("STOR test.txt failed: " . $client->response_code() . " " .
          $client->response_msg());
      }

      my $buf = "ABCD\n" x 8;
      $conn->write($buf, length($buf), 30);
      $conn->close();

      my $resp_code = $client->response_code();
      my $resp_msg = $client->response_msg();

      $client->quit();

      my $expected = 226;
      $self->assert($expected == $resp_code,
        test_msg("Expected $expected, got $resp_code"));

      $expected = 'Transfer complete';
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

  my ($login, $ip_addr, $bytes_in, $bytes_out) = get_session_io($db_file,
    "user = \'$user\'");

  my $expected;

  $expected = $user;
  $self->assert($expected eq $login,
    test_msg("Expected '$expected', got '$login'"));

  $expected = '127.0.0.1';
  $self->assert($expected eq $ip_addr,
    test_msg("Expected '$expected', got '$ip_addr'"));

  $expected = 108;
  $self->assert($expected == $bytes_in,
    test_msg("Expected $expected, got $bytes_in"));

  # Why would this number vary so widely?  It's because of the notation
  # used to express the port number in a PASV response.  That port
  # number is ephemeral, chosen by the kernel.

  my $expected_min = 232;
  my $expected_max = 236;
  $self->assert($expected_min <= $bytes_out ||
                $expected_max >= $bytes_out,
    test_msg("Expected $expected_min - $expected_max, got $bytes_out"));

  unlink($log_file);
}

sub get_session_id {
  my $db_file = shift;
  my $where = shift;

  my $sql = "SELECT user, ip_addr, session_id FROM ftpsessions";
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

sub sql_sqllog_note_var_unique_id_bug3572 {
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
  session_id TEXT
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

  # Make sure that, if we're running as root, the database file has
  # the permissions/privs set for use by proftpd
  if ($< == 0) {
    unless (chmod(0666, $db_file)) {
      die("Can't set perms on $db_file to 0666: $!");
    }
  }

  my $config = {
    PidFile => $pid_file,
    ScoreboardFile => $scoreboard_file,
    SystemLog => $log_file,
    TraceLog => $log_file,
    Trace => 'DEFAULT:10',

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
        SQLNamedQuery => 'session_id FREEFORM "INSERT INTO ftpsessions (user, ip_addr, session_id) VALUES (\'%u\', \'%L\', \'%{note:UNIQUE_ID}\')"',
        SQLLog => 'PASS session_id',
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

  my ($login, $ip_addr, $session_id) = get_session_id($db_file,
    "user = \'$user\'");

  my $expected;

  $expected = $user;
  $self->assert($expected eq $login,
    test_msg("Expected '$expected', got '$login'"));

  $expected = '127.0.0.1';
  $self->assert($expected eq $ip_addr,
    test_msg("Expected '$expected', got '$ip_addr'"));

  $self->assert($session_id ne '',
    test_msg("Expected session ID ($session_id), got ''"));

  unlink($log_file);
}

sub sql_sqllog_note_var_rewrite_bug3572 {
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

  my $domain = 'proftpd.org';

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
  session_id TEXT
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

  # Make sure that, if we're running as root, the database file has
  # the permissions/privs set for use by proftpd
  if ($< == 0) {
    unless (chmod(0666, $db_file)) {
      die("Can't set perms on $db_file to 0666: $!");
    }
  }

  my $config = {
    PidFile => $pid_file,
    ScoreboardFile => $scoreboard_file,
    SystemLog => $log_file,
    TraceLog => $log_file,
    Trace => 'rewrite:10 sql:20 table:20',

    AuthUserFile => $auth_user_file,
    AuthGroupFile => $auth_group_file,

    IfModules => {
      'mod_delay.c' => {
        DelayEngine => 'off',
      },

      'mod_rewrite.c' => [
        'RewriteEngine on',
        "RewriteLog $log_file",

        'RewriteCondition %m USER',
        'RewriteRule ^(.*#)?([-_.0-9A-Za-z]+)(@)?(.*)? $2',
      ],

      'mod_sql.c' => {
        SQLEngine => 'log',
        SQLBackend => 'sqlite3',
        SQLConnectInfo => $db_file,
        SQLLogFile => $log_file,
        SQLNamedQuery => 'session_id FREEFORM "INSERT INTO ftpsessions (user, ip_addr, session_id) VALUES (\'%U\', \'%L\', \'%{note:mod_rewrite.$4}\')"',
        SQLLog => 'USER session_id',
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

      my $login = $user . '@' . $domain;
      $client->login($login, $passwd);
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

  my ($login, $ip_addr, $session_id) = get_session_id($db_file,
    "user = \'$user\'");

  my $expected;

  $expected = $user;
  $self->assert($expected eq $login,
    test_msg("Expected '$expected', got '$login'"));

  $expected = '127.0.0.1';
  $self->assert($expected eq $ip_addr,
    test_msg("Expected '$expected', got '$ip_addr'"));

  $expected = $domain;
  $self->assert($expected eq $session_id,
    test_msg("Expected '$expected', got '$session_id'"));

  unlink($log_file);
}

sub get_sql_notes {
  my $db_file = shift;
  my $where = shift;

  my $sql = "SELECT user, primary_group, home, shell FROM ftpnotes";
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

sub sql_sqllog_note_sql_user_info {
  my $self = shift;
  my $tmpdir = $self->{tmpdir};

  my $config_file = "$tmpdir/sqlite.conf";
  my $pid_file = File::Spec->rel2abs("$tmpdir/sqlite.pid");
  my $scoreboard_file = File::Spec->rel2abs("$tmpdir/sqlite.scoreboard");

  my $log_file = File::Spec->rel2abs('tests.log');

  my $user = 'proftpd';
  my $group = 'ftpd',
  my $passwd = 'test';
  my $home_dir = File::Spec->rel2abs($tmpdir);
  my $uid = 500;
  my $gid = 500;
  my $shell = '/bin/bash';

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
  shell TEXT,
  lastdir TEXT
);
INSERT INTO users (userid, passwd, uid, gid, homedir, shell) VALUES ('$user', '$passwd', $uid, $gid, '$home_dir', '$shell');

CREATE TABLE groups (
  groupname TEXT,
  gid INTEGER,
  members TEXT
);
INSERT INTO groups (groupname, gid, members) VALUES ('$group', $gid, '$user');

CREATE TABLE ftpnotes (
  user TEXT,
  primary_group TEXT,
  home TEXT,
  shell TEXT
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

  # Make sure that, if we're running as root, the database file has
  # the permissions/privs set for use by proftpd
  if ($< == 0) {
    unless (chmod(0666, $db_file)) {
      die("Can't set perms on $db_file to 0666: $!");
    }
  }

  my $config = {
    PidFile => $pid_file,
    ScoreboardFile => $scoreboard_file,
    SystemLog => $log_file,
    TraceLog => $log_file,
    Trace => 'DEFAULT:10 sql:20',

    IfModules => {
      'mod_delay.c' => {
        DelayEngine => 'off',
      },

      'mod_sql.c' => {
        SQLAuthTypes => 'plaintext',
        SQLBackend => 'sqlite3',
        SQLConnectInfo => $db_file,
        SQLMinID => 100,
        SQLLogFile => $log_file,
        SQLNamedQuery => 'sql_notes FREEFORM "INSERT INTO ftpnotes (user, primary_group, home, shell) VALUES (\'%u\', \'%{note:primary-group}\', \'%{note:home}\', \'%{note:shell}\')"',
        SQLLog => 'PASS sql_notes',
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

  my ($login, $sql_group, $sql_home, $sql_shell) = get_sql_notes($db_file,
    "user = \'$user\'");

  my $expected;

  $expected = $user;
  $self->assert($expected eq $login,
    test_msg("Expected '$expected', got '$login'"));

  $expected = $group;
  $self->assert($expected eq $sql_group,
    test_msg("Expected '$expected', got '$sql_group'"));

  $expected = $home_dir;
  $self->assert($expected eq $sql_home,
    test_msg("Expected '$expected', got '$sql_home'"));

  $expected = $shell;
  $self->assert($expected eq $sql_shell,
    test_msg("Expected '$expected', got '$sql_shell'"));

  unlink($log_file);
}

sub sql_sqllog_var_E {
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
  my $group = 'ftpd';
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
  auth_group_write($auth_group_file, $group, $gid, $user);

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

  # Make sure that, if we're running as root, the database file has
  # the permissions/privs set for use by proftpd
  if ($< == 0) {
    unless (chmod(0666, $db_file)) {
      die("Can't set perms on $db_file to 0666: $!");
    }
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
        SQLNamedQuery => 'info FREEFORM "INSERT INTO ftpsessions (user, ip_addr, resp_mesg) VALUES (\'%u\', \'%L\', \'%E\')"',
        SQLLog => 'EXIT info',
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
    "user = \'$user\'", "user, ip_addr, resp_mesg");

  my $expected;

  $expected = $user;
  $self->assert($expected eq $login,
    test_msg("Expected '$expected', got '$login'"));

  $expected = '127.0.0.1';
  $self->assert($expected eq $ip_addr,
    test_msg("Expected '$expected', got '$ip_addr'"));

  $expected = 'Quit';
  $self->assert($expected eq $resp_mesg,
    test_msg("Expected '$expected', got '$resp_mesg'"));

  unlink($log_file);
}

sub sql_named_conn_bug3262 {
  my $self = shift;
  my $tmpdir = $self->{tmpdir};

  my $config_file = "$tmpdir/sqlite.conf";
  my $pid_file = File::Spec->rel2abs("$tmpdir/sqlite.pid");
  my $scoreboard_file = File::Spec->rel2abs("$tmpdir/sqlite.scoreboard");

  my $log_file = File::Spec->rel2abs('tests.log');

  my $user = 'proftpd';
  my $passwd = 'test';
  my $group = 'ftpd';
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

  my $userdb_file = File::Spec->rel2abs("$tmpdir/proftpd-users.db");

  # Build up sqlite3 command to create users, groups tables and populate them
  my $db_script = File::Spec->rel2abs("$tmpdir/proftpd-users.sql");

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
INSERT INTO ftpusers (userid, passwd, uid, gid, homedir, shell) VALUES ('$user', '$passwd', $uid, $gid, '$home_dir', '/bin/bash');

CREATE TABLE groups (
  groupname TEXT,
  gid INTEGER,
  members TEXT
);
INSERT INTO groups (groupname, gid, members) VALUES ('$group', $gid, '$user');

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

  my $cmd = "sqlite3 $userdb_file < $db_script";

  if ($ENV{TEST_VERBOSE}) {
    print STDERR "Executing sqlite3: $cmd\n";
  }

  my @output = `$cmd`;
  if (scalar(@output) &&
      $ENV{TEST_VERBOSE}) {
    print STDERR "Output: ", join('', @output), "\n";
  }

  my $logdb_file = File::Spec->rel2abs("$tmpdir/proftpd-log.db");

  # Build up sqlite3 command to create users, groups tables and populate them
  $db_script = File::Spec->rel2abs("$tmpdir/proftpd-log.sql");

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

  $cmd = "sqlite3 $logdb_file < $db_script";

  if ($ENV{TEST_VERBOSE}) {
    print STDERR "Executing sqlite3: $cmd\n";
  }

  @output = `$cmd`;
  if (scalar(@output) &&
      $ENV{TEST_VERBOSE}) {
    print STDERR "Output: ", join('', @output), "\n";
  }

  # Make sure that, if we're running as root, the database file has
  # the permissions/privs set for use by proftpd
  if ($< == 0) {
    unless (chmod(0666, $userdb_file, $logdb_file)) {
      die("Can't set perms on $userdb_file, $logdb_file to 0666: $!");
    }
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
        'SQLAuthTypes plaintext',
        'SQLBackend sqlite3',
        "SQLConnectInfo $userdb_file",
        "SQLNamedConnectInfo logdb sqlite3 $logdb_file foo bar PERSESSION",
        "SQLLogFile $log_file",
        'SQLNamedQuery get-sql-user SELECT "userid, passwd, uid, gid, homedir, shell FROM ftpusers WHERE userid = \'%U\'"',
        'SQLUserInfo custom:/get-sql-user',
        'SQLNamedQuery session_start FREEFORM "INSERT INTO ftpsessions (user, ip_addr, timestamp) VALUES (\'%u\', \'%L\', \'%{time:%Y-%m-%d %H:%M:%S}\')" logdb',
        'SQLLog PASS session_start',
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

  my ($login, $ip_addr, $timestamp) = get_sessions($userdb_file,
    "user = \'$user\'");

  $self->assert(!defined($login),
    test_msg("Expected undef, got '$login'"));

  $self->assert(!defined($ip_addr),
    test_msg("Expected undef, got '$ip_addr'"));

  $self->assert(!defined($timestamp),
    test_msg("Expected undef, got '$timestamp'"));

  ($login, $ip_addr, $timestamp) = get_sessions($logdb_file,
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

sub sql_named_conn_sqllog_exit_bug3645 {
  my $self = shift;
  my $tmpdir = $self->{tmpdir};

  my $config_file = "$tmpdir/sqlite.conf";
  my $pid_file = File::Spec->rel2abs("$tmpdir/sqlite.pid");
  my $scoreboard_file = File::Spec->rel2abs("$tmpdir/sqlite.scoreboard");

  my $log_file = File::Spec->rel2abs('tests.log');

  my $user = 'proftpd';
  my $passwd = 'test';
  my $group = 'ftpd';
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

  my $userdb_file = File::Spec->rel2abs("$tmpdir/proftpd-users.db");

  # Build up sqlite3 command to create users, groups tables and populate them
  my $db_script = File::Spec->rel2abs("$tmpdir/proftpd-users.sql");

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
INSERT INTO users (userid, passwd, uid, gid, homedir, shell) VALUES ('$user', '$passwd', $uid, $gid, '$home_dir', '/bin/bash');

CREATE TABLE groups (
  groupname TEXT,
  gid INTEGER,
  members TEXT
);
INSERT INTO groups (groupname, gid, members) VALUES ('$group', $gid, '$user');
EOS

    unless (close($fh)) {
      die("Can't write $db_script: $!");
    }

  } else {
    die("Can't open $db_script: $!");
  }

  my $cmd = "sqlite3 $userdb_file < $db_script";

  if ($ENV{TEST_VERBOSE}) {
    print STDERR "Executing sqlite3: $cmd\n";
  }

  my @output = `$cmd`;
  if (scalar(@output) &&
      $ENV{TEST_VERBOSE}) {
    print STDERR "Output: ", join('', @output), "\n";
  }

  my $logdb_file = File::Spec->rel2abs("$tmpdir/proftpd-log.db");

  # Build up sqlite3 command to create users, groups tables and populate them
  $db_script = File::Spec->rel2abs("$tmpdir/proftpd-log.sql");

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

  $cmd = "sqlite3 $logdb_file < $db_script";

  if ($ENV{TEST_VERBOSE}) {
    print STDERR "Executing sqlite3: $cmd\n";
  }

  @output = `$cmd`;
  if (scalar(@output) &&
      $ENV{TEST_VERBOSE}) {
    print STDERR "Output: ", join('', @output), "\n";
  }

  # Make sure that, if we're running as root, the database file has
  # the permissions/privs set for use by proftpd
  if ($< == 0) {
    unless (chmod(0666, $userdb_file, $logdb_file)) {
      die("Can't set perms on $userdb_file, $logdb_file to 0666: $!");
    }
  }

  my $config = {
    PidFile => $pid_file,
    ScoreboardFile => $scoreboard_file,
    SystemLog => $log_file,

    DefaultRoot => '~',

    IfModules => {
      'mod_delay.c' => {
        DelayEngine => 'off',
      },

      'mod_sql.c' => [
        'SQLAuthTypes plaintext',
        'SQLBackend sqlite3',
        "SQLConnectInfo $userdb_file",
        "SQLNamedConnectInfo logdb sqlite3 $logdb_file",
        "SQLLogFile $log_file",
        'SQLNamedQuery session_end FREEFORM "INSERT INTO ftpsessions (user, ip_addr, timestamp) VALUES (\'%u\', \'%L\', \'%{time:%Y-%m-%d %H:%M:%S}\')" logdb',
        'SQLLog EXIT session_end',
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

  my ($login, $ip_addr, $timestamp) = get_sessions($userdb_file,
    "user = \'$user\'");

  $self->assert(!defined($login),
    test_msg("Expected undef, got '$login'"));

  $self->assert(!defined($ip_addr),
    test_msg("Expected undef, got '$ip_addr'"));

  $self->assert(!defined($timestamp),
    test_msg("Expected undef, got '$timestamp'"));

  ($login, $ip_addr, $timestamp) = get_sessions($logdb_file,
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

sub sql_sqllog_vars_H_L_matching_server_bug3620 {
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
  my $group = 'ftpd';
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
  auth_group_write($auth_group_file, $group, $gid, $user);

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

  # Make sure that, if we're running as root, the database file has
  # the permissions/privs set for use by proftpd
  if ($< == 0) {
    unless (chmod(0666, $db_file)) {
      die("Can't set perms on $db_file to 0666: $!");
    }
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
        SQLNamedQuery => 'session_start FREEFORM "INSERT INTO ftpsessions (user, ip_addr, rename_from) VALUES (\'%u\', \'%L\', \'%H\')"',
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

  my ($login, $ip_addr, $vhost_addr) = get_renames($db_file, "user = \'$user\'");

  my $expected;

  $expected = $user;
  $self->assert($expected eq $login,
    test_msg("Expected '$expected', got '$login'"));

  $expected = '127.0.0.1';
  $self->assert($expected eq $ip_addr,
    test_msg("Expected '$expected', got '$ip_addr'"));

  $self->assert($expected eq $vhost_addr,
    test_msg("Expected '$expected', got '$vhost_addr'"));

  unlink($log_file);
}

sub sql_sqllog_vars_H_L_default_server_bug3620 {
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
  my $group = 'ftpd';
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
  auth_group_write($auth_group_file, $group, $gid, $user);

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

  # Make sure that, if we're running as root, the database file has
  # the permissions/privs set for use by proftpd
  if ($< == 0) {
    unless (chmod(0666, $db_file)) {
      die("Can't set perms on $db_file to 0666: $!");
    }
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

    DefaultServer => 'off',
    SocketBindTight => 'off',
    Port => '0',

    IfModules => {
      'mod_delay.c' => {
        DelayEngine => 'off',
      },

      'mod_sql.c' => {
        SQLEngine => 'log',
        SQLBackend => 'sqlite3',
        SQLConnectInfo => $db_file,
        SQLLogFile => $log_file,
        SQLNamedQuery => 'session_start FREEFORM "INSERT INTO ftpsessions (user, ip_addr, rename_from) VALUES (\'%u\', \'%L\', \'%H\')"',
        SQLLog => 'PASS session_start',
      },
    },
  };

  my ($port, $config_user, $config_group) = config_write($config_file, $config);

  # NOTE: this real_addr value may need to be tweaked as necessary.  Or
  # maybe find a Perl module which can list the interfaces currently configured
  # for the machine; all we really want is a non-127.0.0.1 address.
  my $real_addr = '192.168.0.101';
  my $real_port = ProFTPD::TestSuite::Utils::get_high_numbered_port();
  my $vhost_addr = '0.0.0.0';

  if (open(my $fh, ">> $config_file")) {
    print $fh <<EOC;

<VirtualHost $vhost_addr>
  ServerName "DefaultServer VHost"
  Port $real_port
  DefaultServer on

  AuthUserFile $auth_user_file
  AuthGroupFile $auth_group_file
  RequireValidShell off
  WtmpLog off

  <IfModule mod_sql.c>
    SQLEngine log
    SQLBackend sqlite3
    SQLConnectInfo $db_file
    SQLLogFile $log_file
    SQLNamedQuery session_start FREEFORM "INSERT INTO ftpsessions (user, ip_addr, rename_from) VALUES (\'%u\', \'%L\', \'%H\')"
    SQLLog PASS session_start
  </IfModule>
</VirtualHost>
EOC
    unless (close($fh)) {
      die("Can't write $config_file: $!");
    }

  } else {
    die("Can't open $config_file: $!");
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
      my $client = ProFTPD::TestSuite::FTP->new($real_addr, $real_port);
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

  my ($login, $ip_addr, $sess_addr) = get_renames($db_file, "user = \'$user\'");

  my $expected;

  $expected = $user;
  $self->assert($expected eq $login,
    test_msg("Expected '$expected', got '$login'"));

  $expected = $real_addr;
  $self->assert($expected eq $ip_addr,
    test_msg("Expected '$expected', got '$ip_addr'"));

  $expected = $vhost_addr;
  $self->assert($expected eq $sess_addr,
    test_msg("Expected '$expected', got '$sess_addr'"));

  unlink($log_file);
}

sub sql_sqllog_exit_ifuser {
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
  my $group = 'ftpd';
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
  auth_group_write($auth_group_file, $group, $gid, $user);

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

  # Make sure that, if we're running as root, the database file has
  # the permissions/privs set for use by proftpd
  if ($< == 0) {
    unless (chmod(0666, $db_file)) {
      die("Can't set perms on $db_file to 0666: $!");
    }
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
      },
    },
  };

  my ($port, $config_user, $config_group) = config_write($config_file, $config);

  if (open(my $fh, ">> $config_file")) {
    print $fh <<EOC;
<IfModule mod_ifsession.c>
  <IfUser regex .*>
    SQLLog EXIT logout
  </IfUser>
</IfModule>
EOC
    unless (close($fh)) {
      die("Can't write $config_file: $!");
    }

  } else {
    die("Can't open $config_file: $!");
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

      # First, just connect and quit, without logging in
      my $client = ProFTPD::TestSuite::FTP->new('127.0.0.1', $port);
      $client->quit();

      # Then connect, login, and quit
      $client = ProFTPD::TestSuite::FTP->new('127.0.0.1', $port);
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

  my $query = "SELECT user, ip_addr FROM ftpsessions";
  $cmd = "sqlite3 $db_file \"$query\"";

  if ($ENV{TEST_VERBOSE}) {
    print STDERR "Executing sqlite3: $cmd\n";
  }

  my @res = `$cmd`;
  my $res = join('', @res);
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

sub sql_sqllog_exit_ifgroup {
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
  my $group = 'ftpd';
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
  auth_group_write($auth_group_file, $group, $gid, $user);

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

  # Make sure that, if we're running as root, the database file has
  # the permissions/privs set for use by proftpd
  if ($< == 0) {
    unless (chmod(0666, $db_file)) {
      die("Can't set perms on $db_file to 0666: $!");
    }
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
      },
    },
  };

  my ($port, $config_user, $config_group) = config_write($config_file, $config);

  if (open(my $fh, ">> $config_file")) {
    print $fh <<EOC;
<IfModule mod_ifsession.c>
  <IfGroup regex .*>
    SQLLog EXIT logout
  </IfGroup>
</IfModule>
EOC
    unless (close($fh)) {
      die("Can't write $config_file: $!");
    }

  } else {
    die("Can't open $config_file: $!");
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

      # First, just connect and quit, without logging in
      my $client = ProFTPD::TestSuite::FTP->new('127.0.0.1', $port);
      $client->quit();

      # Then connect, login, and quit
      $client = ProFTPD::TestSuite::FTP->new('127.0.0.1', $port);
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

  my $query = "SELECT user, ip_addr FROM ftpsessions";
  $cmd = "sqlite3 $db_file \"$query\"";

  if ($ENV{TEST_VERBOSE}) {
    print STDERR "Executing sqlite3: $cmd\n";
  }

  my @res = `$cmd`;
  my $res = join('', @res);
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

sub sql_opt_no_disconnect_on_error_with_extlog_bug3633 {
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
  my $group = 'ftpd';
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
  auth_group_write($auth_group_file, $group, $gid, $user);

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

  # Make sure that, if we're running as root, the database file has
  # the permissions/privs set for use by proftpd
  if ($< == 0) {
    unless (chmod(0666, $db_file)) {
      die("Can't set perms on $db_file to 0666: $!");
    }
  }

  my $test_file = File::Spec->rel2abs("$home_dir/test.txt");
  if (open(my $fh, "> $test_file")) {
    print $fh "Hello, World!\n";
    unless (close($fh)) {
      die("Can't write $test_file: $!");
    }

  } else {
    die("Can't open $test_file: $!");
  }

  my $ext_log = File::Spec->rel2abs("$tmpdir/custom.log");

  my $config = {
    PidFile => $pid_file,
    ScoreboardFile => $scoreboard_file,
    SystemLog => $log_file,
    TraceLog => $log_file,
    Trace => 'DEFAULT:10 sql:20',

    AuthUserFile => $auth_user_file,
    AuthGroupFile => $auth_group_file,

    LogFormat => 'custom "%m %f"',
    ExtendedLog => "$ext_log ALL custom",

    IfModules => {
      'mod_delay.c' => {
        DelayEngine => 'off',
      },

      'mod_sql.c' => {
        SQLEngine => 'log',
        SQLBackend => 'sqlite3',
        SQLConnectInfo => $db_file,
        SQLLogFile => $log_file,
        SQLNamedQuery => 'log_dele FREEFORM "INSERT INTO ftpsessions (user, ip_addr, ts) VALUES (\'%u\', \'%L\', \'%{time:%Y-%m-%d %H:%M:%S}\')"',
        SQLLog => 'DELE log_dele',

        SQLOptions => 'noDisconnectOnError',
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
      $client->dele('test.txt');
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

  # Now, read in the ExtendedLog, and see whether the %f variable was
  # properly written out.
  if (open(my $fh, "< $ext_log")) {
    my $ok = 0;

    while (my $line = <$fh>) {
      chomp($line);

      # We're only interested in the DELE log line
      unless ($line =~ /^DELE (.*)$/i) {
        next;
      }

      my $name = $1;
      my $expected = $test_file;
      $self->assert($expected eq $name,
        test_msg("Expected '$expected', got '$name'"));

      $ok = 1;
    }

    close($fh);

    $self->assert($ok, test_msg("Expected ExtendedLog messages not found"));

  } else {
    die("Can't read $ext_log: $!");
  }

  unlink($log_file);
}

sub sql_sqllog_ignore_errors_bad_table_bug3692 {
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
  my $group = 'ftpd';
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
  auth_group_write($auth_group_file, $group, $gid, $user);

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

  # Make sure that, if we're running as root, the database file has
  # the permissions/privs set for use by proftpd
  if ($< == 0) {
    unless (chmod(0666, $db_file)) {
      die("Can't set perms on $db_file to 0666: $!");
    }
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
        SQLNamedQuery => 'session_start FREEFORM "INSERT INTO ftpsessions (user, ip_addr, ts) VALUES (\'%u\', \'%L\', \'%{time:%Y-%m-%d %H:%M:%S}\')"',
        SQLLog => 'PASS session_start IGNORE_ERRORS',
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

  unlink($log_file);
}

sub sql_sqllog_ignore_errors_bad_db_bug3692 {
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
  my $group = 'ftpd';
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
  auth_group_write($auth_group_file, $group, $gid, $user);

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

  # Make sure that, if we're running as root, the database file has
  # the permissions/privs set for use by proftpd
  if ($< == 0) {
    unless (chmod(0666, $db_file)) {
      die("Can't set perms on $db_file to 0666: $!");
    }
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
        SQLConnectInfo => "/foobar/$db_file",
        SQLLogFile => $log_file,
        SQLNamedQuery => 'session_start FREEFORM "INSERT INTO ftpsessions (user, ip_addr, ts) VALUES (\'%u\', \'%L\', \'%{time:%Y-%m-%d %H:%M:%S}\')"',
        SQLLog => 'PASS session_start IGNORE_ERRORS',
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

  unlink($log_file);
}

1;
