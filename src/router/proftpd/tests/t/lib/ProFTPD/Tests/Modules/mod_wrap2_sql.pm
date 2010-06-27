package ProFTPD::Tests::Modules::mod_wrap2_sql;

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
  wrap2_allow_msg => {
    order => ++$order,
    test_class => [qw(forking)],
  },

  wrap2_deny_msg => {
    order => ++$order,
    test_class => [qw(forking)],
  },

  wrap2_engine => {
    order => ++$order,
    test_class => [qw(forking)],
  },

  wrap2_sql_allow_table => {
    order => ++$order,
    test_class => [qw(forking)],
  },

  wrap2_sql_allow_table_multi_rows_multi_entries => {
    order => ++$order,
    test_class => [qw(bug forking)],
  },

  wrap2_sql_allow_table_all => {
    order => ++$order,
    test_class => [qw(bug forking)],
  },

  wrap2_sql_deny_table_ip_addr => {
    order => ++$order,
    test_class => [qw(forking)],
  },

  wrap2_sql_deny_table_dns_name => {
    order => ++$order,
    test_class => [qw(forking)],
  },

  wrap2_sql_user_tables => {
    order => ++$order,
    test_class => [qw(forking)],
  },

  wrap2_sql_group_tables => {
    order => ++$order,
    test_class => [qw(forking)],
  },

  wrap2_sql_bug3215 => {
    order => ++$order,
    test_class => [qw(bug forking)],
  },

  wrap2_bug3341 => {
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
};

sub wrap2_allow_msg {
  my $self = shift;
  my $tmpdir = $self->{tmpdir};

  my $config_file = "$tmpdir/wrap2.conf";
  my $pid_file = File::Spec->rel2abs("$tmpdir/wrap2.pid");
  my $scoreboard_file = File::Spec->rel2abs("$tmpdir/wrap2.scoreboard");

  my $log_file = File::Spec->rel2abs('tests.log');

  my $auth_user_file = File::Spec->rel2abs("$tmpdir/wrap2.passwd");
  my $auth_group_file = File::Spec->rel2abs("$tmpdir/wrap2.group");

  my $db_file = File::Spec->rel2abs("$tmpdir/wrap2.db");

  # Build up sqlite3 command to create allow, deny tables and populate them
  my $db_script = File::Spec->rel2abs("$tmpdir/wrap2.sql");

  my $fh;
  if (open($fh, "> $db_script")) {
    print $fh <<EOS;
CREATE TABLE ftpallow (
  name TEXT,
  allowed TEXT
);

CREATE TABLE ftpdeny (
  name TEXT,
  denied TEXT
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

  unlink($db_script);

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

  my $timeout_idle = 30;

  my $config = {
    PidFile => $pid_file,
    ScoreboardFile => $scoreboard_file,
    SystemLog => $log_file,

    AuthUserFile => $auth_user_file,
    AuthGroupFile => $auth_group_file,
    TimeoutIdle => $timeout_idle,

    IfModules => {
      'mod_delay.c' => {
        DelayEngine => 'off',
      },

      'mod_sql_sqlite.c' => [
        'SQLAuthenticate off',
        "SQLConnectInfo $db_file",
        'SQLNamedQuery get-allowed-clients SELECT "allowed FROM ftpallow WHERE name = \'%{0}\'"',
        'SQLNamedQuery get-denied-clients SELECT "denied FROM ftpdeny WHERE name = \'%{0}\'"',
        "SQLLogFile $log_file",
      ],

      'mod_wrap2_sql.c' => {
        WrapEngine => 'on',
        WrapAllowMsg => '"User %u allowed by access rules"',
        WrapTables => "sql:/get-allowed-clients sql:/get-denied-clients",
        WrapLog => $log_file,
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

      ($resp_code, $resp_msg) = $client->login($user, $passwd);

      my $expected;

      $expected = 230;
      $self->assert($expected == $resp_code,
        test_msg("Expected $expected, got $resp_code"));

      $expected = "User $user allowed by access rules";
      $self->assert($expected eq $resp_msg,
        test_msg("Expected '$expected', got '$resp_msg'"));
    };

    if ($@) {
      $ex = $@;
    }

    $wfh->print("done\n");
    $wfh->flush();

  } else {
    eval { server_wait($config_file, $rfh, $timeout_idle + 2) };
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

sub wrap2_deny_msg {
  my $self = shift;
  my $tmpdir = $self->{tmpdir};

  my $config_file = "$tmpdir/wrap2.conf";
  my $pid_file = File::Spec->rel2abs("$tmpdir/wrap2.pid");
  my $scoreboard_file = File::Spec->rel2abs("$tmpdir/wrap2.scoreboard");

  my $log_file = File::Spec->rel2abs('tests.log');

  my $auth_user_file = File::Spec->rel2abs("$tmpdir/wrap2.passwd");
  my $auth_group_file = File::Spec->rel2abs("$tmpdir/wrap2.group");

  my $db_file = File::Spec->rel2abs("$tmpdir/wrap2.db");

  # Build up sqlite3 command to create allow, deny tables and populate them
  my $db_script = File::Spec->rel2abs("$tmpdir/wrap2.sql");

  my $fh;
  if (open($fh, "> $db_script")) {
    print $fh <<EOS;
CREATE TABLE ftpallow (
  name TEXT,
  allowed TEXT
);

CREATE TABLE ftpdeny (
  name TEXT,
  denied TEXT
);

INSERT INTO ftpdeny (name, denied) VALUES ('', 'ALL');
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

  unlink($db_script);

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

  my $timeout_idle = 30;

  my $config = {
    PidFile => $pid_file,
    ScoreboardFile => $scoreboard_file,
    SystemLog => $log_file,

    AuthUserFile => $auth_user_file,
    AuthGroupFile => $auth_group_file,
    TimeoutIdle => $timeout_idle,

    IfModules => {
      'mod_delay.c' => {
        DelayEngine => 'off',
      },

      'mod_sql_sqlite.c' => [
        'SQLAuthenticate off',
        "SQLConnectInfo $db_file",
        'SQLNamedQuery get-allowed-clients SELECT "allowed FROM ftpallow WHERE name = \'%{0}\'"',
        'SQLNamedQuery get-denied-clients SELECT "denied FROM ftpdeny WHERE name = \'%{0}\'"',
        "SQLLogFile $log_file",
      ],

      'mod_wrap2_sql.c' => {
        WrapEngine => 'on',
        WrapDenyMsg => '"User %u rejected by access rules"',
        WrapTables => "sql:/get-allowed-clients sql:/get-denied-clients",
        WrapLog => $log_file,
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

      eval { $client->login($user, $passwd) };
      unless ($@) {
        die("Login succeeded unexpectedly");

      } else {
        $resp_code = $client->response_code();
        $resp_msg = $client->response_msg();
      }

      my $expected;

      $expected = 530;
      $self->assert($expected == $resp_code,
        test_msg("Expected $expected, got $resp_code"));

      $expected = "User $user rejected by access rules";
      $self->assert($expected eq $resp_msg,
        test_msg("Expected '$expected', got '$resp_msg'"));
    };

    if ($@) {
      $ex = $@;
    }

    $wfh->print("done\n");
    $wfh->flush();

  } else {
    eval { server_wait($config_file, $rfh, $timeout_idle + 2) };
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

sub wrap2_engine {
  my $self = shift;
  my $tmpdir = $self->{tmpdir};

  my $config_file = "$tmpdir/wrap2.conf";
  my $pid_file = File::Spec->rel2abs("$tmpdir/wrap2.pid");
  my $scoreboard_file = File::Spec->rel2abs("$tmpdir/wrap2.scoreboard");

  my $log_file = File::Spec->rel2abs('tests.log');

  my $auth_user_file = File::Spec->rel2abs("$tmpdir/wrap2.passwd");
  my $auth_group_file = File::Spec->rel2abs("$tmpdir/wrap2.group");

  my $db_file = File::Spec->rel2abs("$tmpdir/wrap2.db");

  # Build up sqlite3 command to create allow, deny tables and populate them
  my $db_script = File::Spec->rel2abs("$tmpdir/wrap2.sql");

  my $fh;
  if (open($fh, "> $db_script")) {
    print $fh <<EOS;
CREATE TABLE ftpallow (
  name TEXT,
  allowed TEXT
);

CREATE TABLE ftpdeny (
  name TEXT,
  denied TEXT
);

INSERT INTO ftpdeny (name, denied) VALUES ('', 'ALL');
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

  unlink($db_script);

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

  my $timeout_idle = 30;

  my $config = {
    PidFile => $pid_file,
    ScoreboardFile => $scoreboard_file,
    SystemLog => $log_file,

    AuthUserFile => $auth_user_file,
    AuthGroupFile => $auth_group_file,
    TimeoutIdle => $timeout_idle,

    IfModules => {
      'mod_delay.c' => {
        DelayEngine => 'off',
      },

      'mod_sql_sqlite.c' => [
        'SQLAuthenticate off',
        "SQLConnectInfo $db_file",
        'SQLNamedQuery get-allowed-clients SELECT "allowed FROM ftpallow WHERE name = \'%{0}\'"',
        'SQLNamedQuery get-denied-clients SELECT "denied FROM ftpdeny WHERE name = \'%{0}\'"',
        "SQLLogFile $log_file",
      ],

      'mod_wrap2_sql.c' => {
        WrapEngine => 'off',
        WrapAllowMsg => '"User %u allowed by access rules"',
        WrapDenyMsg => '"User %u rejected by access rules"',
        WrapTables => "sql:/get-allowed-clients sql:/get-denied-clients",
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

      ($resp_code, $resp_msg) = $client->login($user, $passwd);

      my $expected;

      $expected = 230;
      $self->assert($expected == $resp_code,
        test_msg("Expected $expected, got $resp_code"));

      $expected = "User $user logged in";
      $self->assert($expected eq $resp_msg,
        test_msg("Expected '$expected', got '$resp_msg'"));
    };

    if ($@) {
      $ex = $@;
    }

    $wfh->print("done\n");
    $wfh->flush();

  } else {
    eval { server_wait($config_file, $rfh, $timeout_idle + 2) };
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

sub wrap2_sql_allow_table {
  my $self = shift;
  my $tmpdir = $self->{tmpdir};

  my $config_file = "$tmpdir/wrap2.conf";
  my $pid_file = File::Spec->rel2abs("$tmpdir/wrap2.pid");
  my $scoreboard_file = File::Spec->rel2abs("$tmpdir/wrap2.scoreboard");

  my $log_file = File::Spec->rel2abs('tests.log');

  my $auth_user_file = File::Spec->rel2abs("$tmpdir/wrap2.passwd");
  my $auth_group_file = File::Spec->rel2abs("$tmpdir/wrap2.group");

  my $db_file = File::Spec->rel2abs("$tmpdir/wrap2.db");

  # Build up sqlite3 command to create allow, deny tables and populate them
  my $db_script = File::Spec->rel2abs("$tmpdir/wrap2.sql");

  my $fh;
  if (open($fh, "> $db_script")) {
    print $fh <<EOS;
CREATE TABLE ftpallow (
  name TEXT,
  allowed TEXT
);

CREATE TABLE ftpdeny (
  name TEXT,
  denied TEXT
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

  unlink($db_script);

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

  my $timeout_idle = 30;

  my $config = {
    PidFile => $pid_file,
    ScoreboardFile => $scoreboard_file,
    SystemLog => $log_file,

    AuthUserFile => $auth_user_file,
    AuthGroupFile => $auth_group_file,
    TimeoutIdle => $timeout_idle,

    IfModules => {
      'mod_delay.c' => {
        DelayEngine => 'off',
      },

      'mod_sql_sqlite.c' => [
        'SQLAuthenticate off',
        "SQLConnectInfo $db_file",
        'SQLNamedQuery get-allowed-clients SELECT "allowed FROM ftpallow WHERE name = \'%{0}\'"',
        'SQLNamedQuery get-denied-clients SELECT "denied FROM ftpdeny WHERE name = \'%{0}\'"',
        "SQLLogFile $log_file",
      ],

      'mod_wrap2_sql.c' => {
        WrapEngine => 'on',
        WrapTables => "sql:/get-allowed-clients sql:/get-denied-clients",
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

      ($resp_code, $resp_msg) = $client->login($user, $passwd);

      my $expected;

      $expected = 230;
      $self->assert($expected == $resp_code,
        test_msg("Expected $expected, got $resp_code"));

      $expected = "User $user logged in";
      $self->assert($expected eq $resp_msg,
        test_msg("Expected '$expected', got '$resp_msg'"));
    };

    if ($@) {
      $ex = $@;
    }

    $wfh->print("done\n");
    $wfh->flush();

  } else {
    eval { server_wait($config_file, $rfh, $timeout_idle + 2) };
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

  if (open($fh, "> $db_script")) {
    print $fh <<EOS;
INSERT INTO ftpallow (name, allowed) VALUES ('', '127.0.0.1');
EOS
    unless (close($fh)) {
      die("Can't write $db_script: $!");
    }

  } else {
    die("Can't open $db_script: $!");
  }

  $cmd = "sqlite3 $db_file < $db_script";

  if ($ENV{TEST_VERBOSE}) {
    print STDERR "Executing sqlite3: $cmd\n";
  }

  @output = `$cmd`;

  unlink($db_script);

  ($port, $config_user, $config_group) = config_write($config_file, $config);

  # Fork child
  $self->handle_sigchld();
  defined($pid = fork()) or die("Can't fork: $!");
  if ($pid) {
    eval {
      my $client = ProFTPD::TestSuite::FTP->new('127.0.0.1', $port);

      my ($resp_code, $resp_msg);

      ($resp_code, $resp_msg) = $client->login($user, $passwd);

      my $expected;

      $expected = 230;
      $self->assert($expected == $resp_code,
        test_msg("Expected $expected, got $resp_code"));

      $expected = "User $user logged in";
      $self->assert($expected eq $resp_msg,
        test_msg("Expected '$expected', got '$resp_msg'"));
    };

    if ($@) {
      $ex = $@;
    }

    $wfh->print("done\n");
    $wfh->flush();

  } else {
    eval { server_wait($config_file, $rfh, $timeout_idle + 2) };
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

sub wrap2_sql_allow_table_multi_rows_multi_entries {
  my $self = shift;
  my $tmpdir = $self->{tmpdir};

  my $config_file = "$tmpdir/wrap2.conf";
  my $pid_file = File::Spec->rel2abs("$tmpdir/wrap2.pid");
  my $scoreboard_file = File::Spec->rel2abs("$tmpdir/wrap2.scoreboard");

  my $log_file = File::Spec->rel2abs('tests.log');

  my $auth_user_file = File::Spec->rel2abs("$tmpdir/wrap2.passwd");
  my $auth_group_file = File::Spec->rel2abs("$tmpdir/wrap2.group");

  my $user = 'proftpd';
  my $passwd = 'test';
  my $home_dir = File::Spec->rel2abs($tmpdir);
  my $uid = 500;
  my $gid = 500;

  my $db_file = File::Spec->rel2abs("$tmpdir/wrap2.db");

  # Build up sqlite3 command to create allow, deny tables and populate them
  my $db_script = File::Spec->rel2abs("$tmpdir/wrap2.sql");

  my $fh;
  if (open($fh, "> $db_script")) {
    print $fh <<EOS;
CREATE TABLE ftpallow (
  name TEXT,
  allowed TEXT
);

INSERT INTO ftpallow (name, allowed) VALUES ('', '192.168.127.5, 192.168.127.6');
INSERT INTO ftpallow (name, allowed) VALUES ('', '192.168.127.1 192.168.127.2 127.0.0.1');
INSERT INTO ftpallow (name, allowed) VALUES ('', '192.168.127.3,192.168.127.4 127.0.0.1');

CREATE TABLE ftpdeny (
  name TEXT,
  denied TEXT
);

INSERT INTO ftpdeny (name, denied) VALUES ('', 'ALL');
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

  unlink($db_script);

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

  my $timeout_idle = 30;

  my $config = {
    PidFile => $pid_file,
    ScoreboardFile => $scoreboard_file,
    SystemLog => $log_file,

    AuthUserFile => $auth_user_file,
    AuthGroupFile => $auth_group_file,
    TimeoutIdle => $timeout_idle,

    IfModules => {
      'mod_delay.c' => {
        DelayEngine => 'off',
      },

      'mod_sql_sqlite.c' => [
        'SQLAuthenticate off',
        "SQLConnectInfo $db_file",
        "SQLLogFile $log_file",
        'SQLNamedQuery get-allowed-clients SELECT "allowed FROM ftpallow WHERE name = \'%{0}\'"',
        'SQLNamedQuery get-denied-clients SELECT "denied FROM ftpdeny WHERE name = \'%{0}\'"',
      ],

      'mod_wrap2_sql.c' => {
        WrapEngine => 'on',
        WrapLog => $log_file,
        WrapTables => "sql:/get-allowed-clients sql:/get-denied-clients",
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

      ($resp_code, $resp_msg) = $client->login($user, $passwd);

      my $expected;

      $expected = 230;
      $self->assert($expected == $resp_code,
        test_msg("Expected $expected, got $resp_code"));

      $expected = "User $user logged in";
      $self->assert($expected eq $resp_msg,
        test_msg("Expected '$expected', got '$resp_msg'"));
    };

    if ($@) {
      $ex = $@;
    }

    $wfh->print("done\n");
    $wfh->flush();

  } else {
    eval { server_wait($config_file, $rfh, $timeout_idle + 2) };
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

sub wrap2_sql_allow_table_all {
  my $self = shift;
  my $tmpdir = $self->{tmpdir};

  my $config_file = "$tmpdir/wrap2.conf";
  my $pid_file = File::Spec->rel2abs("$tmpdir/wrap2.pid");
  my $scoreboard_file = File::Spec->rel2abs("$tmpdir/wrap2.scoreboard");

  my $log_file = File::Spec->rel2abs('tests.log');

  my $auth_user_file = File::Spec->rel2abs("$tmpdir/wrap2.passwd");
  my $auth_group_file = File::Spec->rel2abs("$tmpdir/wrap2.group");

  my $db_file = File::Spec->rel2abs("$tmpdir/wrap2.db");

  # Build up sqlite3 command to create allow, deny tables and populate them
  my $db_script = File::Spec->rel2abs("$tmpdir/wrap2.sql");

  my $fh;
  if (open($fh, "> $db_script")) {
    print $fh <<EOS;
CREATE TABLE ftpallow (
  name TEXT,
  allowed TEXT
);

INSERT INTO ftpallow (name, allowed) VALUES ('', 'ALL');

CREATE TABLE ftpdeny (
  name TEXT,
  denied TEXT
);

INSERT INTO ftpdeny (name, allowed) VALUES ('', 'ALL');
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

  unlink($db_script);

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

  my $timeout_idle = 30;

  my $config = {
    TraceLog => $log_file,
    Trace => 'dns:10',
    PidFile => $pid_file,
    ScoreboardFile => $scoreboard_file,
    SystemLog => $log_file,

    AuthUserFile => $auth_user_file,
    AuthGroupFile => $auth_group_file,
    TimeoutIdle => $timeout_idle,

    IfModules => {
      'mod_delay.c' => {
        DelayEngine => 'off',
      },

      'mod_sql_sqlite.c' => [
        'SQLAuthenticate off',
        "SQLConnectInfo $db_file",
        'SQLNamedQuery get-allowed-clients SELECT "allowed FROM ftpallow WHERE name = \'%{0}\'"',
        'SQLNamedQuery get-denied-clients SELECT "denied FROM ftpdeny WHERE name = \'%{0}\'"',
        "SQLLogFile $log_file",
      ],

      'mod_wrap2_sql.c' => {
        WrapEngine => 'on',
        WrapTables => "sql:/get-allowed-clients sql:/get-denied-clients",
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

      ($resp_code, $resp_msg) = $client->login($user, $passwd);

      my $expected;

      $expected = 230;
      $self->assert($expected == $resp_code,
        test_msg("Expected $expected, got $resp_code"));

      $expected = "User $user logged in";
      $self->assert($expected eq $resp_msg,
        test_msg("Expected '$expected', got '$resp_msg'"));
    };

    if ($@) {
      $ex = $@;
    }

    $wfh->print("done\n");
    $wfh->flush();

  } else {
    eval { server_wait($config_file, $rfh, $timeout_idle + 2) };
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

sub wrap2_sql_deny_table_ip_addr {
  my $self = shift;
  my $tmpdir = $self->{tmpdir};

  my $config_file = "$tmpdir/wrap2.conf";
  my $pid_file = File::Spec->rel2abs("$tmpdir/wrap2.pid");
  my $scoreboard_file = File::Spec->rel2abs("$tmpdir/wrap2.scoreboard");

  my $log_file = File::Spec->rel2abs('tests.log');

  my $auth_user_file = File::Spec->rel2abs("$tmpdir/wrap2.passwd");
  my $auth_group_file = File::Spec->rel2abs("$tmpdir/wrap2.group");

  my $db_file = File::Spec->rel2abs("$tmpdir/wrap2.db");

  # Build up sqlite3 command to create allow, deny tables and populate them
  my $db_script = File::Spec->rel2abs("$tmpdir/wrap2.sql");

  my $fh;
  if (open($fh, "> $db_script")) {
    print $fh <<EOS;
CREATE TABLE ftpallow (
  name TEXT,
  allowed TEXT
);

CREATE TABLE ftpdeny (
  name TEXT,
  denied TEXT
);

INSERT INTO ftpdeny (name, denied) VALUES ('', 'ALL');
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

  unlink($db_script);

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

  my $timeout_idle = 30;

  my $config = {
    PidFile => $pid_file,
    ScoreboardFile => $scoreboard_file,
    SystemLog => $log_file,

    AuthUserFile => $auth_user_file,
    AuthGroupFile => $auth_group_file,
    TimeoutIdle => $timeout_idle,

    IfModules => {
      'mod_delay.c' => {
        DelayEngine => 'off',
      },

      'mod_sql_sqlite.c' => [
        "SQLAuthenticate off",
        "SQLConnectInfo $db_file",
        'SQLNamedQuery get-allowed-clients SELECT "allowed FROM ftpallow WHERE name = \'%{0}\'"',
        'SQLNamedQuery get-denied-clients SELECT "denied FROM ftpdeny WHERE name = \'%{0}\'"',
        "SQLLogFile $log_file",
      ],

      'mod_wrap2_sql.c' => {
        WrapEngine => 'on',
        WrapTables => "sql:/get-allowed-clients sql:/get-denied-clients",
        WrapLog => $log_file,
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

      eval { $client->login($user, $passwd) };
      unless ($@) {
        die("Login succeeded unexpectedly");

      } else {
        $resp_code = $client->response_code();
        $resp_msg = $client->response_msg();
      }

      my $expected;

      $expected = 530;
      $self->assert($expected == $resp_code,
        test_msg("Expected $expected, got $resp_code"));

      $expected = "Access denied";
      $self->assert($expected eq $resp_msg,
        test_msg("Expected '$expected', got '$resp_msg'"));
    };

    if ($@) {
      $ex = $@;
    }

    $wfh->print("done\n");
    $wfh->flush();

  } else {
    eval { server_wait($config_file, $rfh, $timeout_idle + 2) };
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

  if (open($fh, "> $db_script")) {
    print $fh <<EOS;
DELETE FROM ftpdeny;
INSERT INTO ftpdeny (name, denied) VALUES ('', '127.0.0.1');
EOS
    unless (close($fh)) {
      die("Can't write $db_script: $!");
    }

  } else {
    die("Can't open $db_script: $!");
  }

  $cmd = "sqlite3 $db_file < $db_script";

  if ($ENV{TEST_VERBOSE}) {
    print STDERR "Executing sqlite3: $cmd\n";
  }

  @output = `$cmd`;

  unlink($db_script);

  ($port, $config_user, $config_group) = config_write($config_file, $config);

  # Fork child
  $self->handle_sigchld();
  defined($pid = fork()) or die("Can't fork: $!");
  if ($pid) {
    eval {
      my $client = ProFTPD::TestSuite::FTP->new('127.0.0.1', $port);

      my ($resp_code, $resp_msg);

      eval { $client->login($user, $passwd) };
      unless ($@) {
        die("Login succeeded unexpectedly");

      } else {
        $resp_code = $client->response_code();
        $resp_msg = $client->response_msg();
      }

      my $expected;

      $expected = 530;
      $self->assert($expected == $resp_code,
        test_msg("Expected $expected, got $resp_code"));

      $expected = "Access denied";
      $self->assert($expected eq $resp_msg,
        test_msg("Expected '$expected', got '$resp_msg'"));
    };

    if ($@) {
      $ex = $@;
    }

    $wfh->print("done\n");
    $wfh->flush();

  } else {
    eval { server_wait($config_file, $rfh, $timeout_idle + 2) };
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

sub wrap2_sql_deny_table_dns_name {
  my $self = shift;
  my $tmpdir = $self->{tmpdir};

  my $config_file = "$tmpdir/wrap2.conf";
  my $pid_file = File::Spec->rel2abs("$tmpdir/wrap2.pid");
  my $scoreboard_file = File::Spec->rel2abs("$tmpdir/wrap2.scoreboard");

  my $log_file = File::Spec->rel2abs('tests.log');

  my $auth_user_file = File::Spec->rel2abs("$tmpdir/wrap2.passwd");
  my $auth_group_file = File::Spec->rel2abs("$tmpdir/wrap2.group");

  my $db_file = File::Spec->rel2abs("$tmpdir/wrap2.db");

  # Build up sqlite3 command to create allow, deny tables and populate them
  my $db_script = File::Spec->rel2abs("$tmpdir/wrap2.sql");

  my $fh;
  if (open($fh, "> $db_script")) {
    print $fh <<EOS;
CREATE TABLE ftpallow (
  name TEXT,
  allowed TEXT
);

CREATE TABLE ftpdeny (
  name TEXT,
  denied TEXT
);

INSERT INTO ftpdeny (name, denied) VALUES ('', 'ALL');
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

  unlink($db_script);

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

  my $timeout_idle = 30;

  my $config = {
    PidFile => $pid_file,
    ScoreboardFile => $scoreboard_file,
    SystemLog => $log_file,

    AuthUserFile => $auth_user_file,
    AuthGroupFile => $auth_group_file,
    TimeoutIdle => $timeout_idle,
    UseReverseDNS => 'on',

    IfModules => {
      'mod_delay.c' => {
        DelayEngine => 'off',
      },

      'mod_sql_sqlite.c' => [
        'SQLAuthenticate off',
        "SQLConnectInfo $db_file",
        'SQLNamedQuery get-allowed-clients SELECT "allowed FROM ftpallow WHERE name = \'%{0}\'"',
        'SQLNamedQuery get-denied-clients SELECT "denied FROM ftpdeny WHERE name = \'%{0}\'"',
        "SQLLogFile $log_file",
      ],

      'mod_wrap2_sql.c' => {
        WrapEngine => 'on',
        WrapTables => "sql:/get-allowed-clients sql:/get-denied-clients",
        WrapLog => $log_file,
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

      eval { $client->login($user, $passwd) };
      unless ($@) {
        die("Login succeeded unexpectedly");

      } else {
        $resp_code = $client->response_code();
        $resp_msg = $client->response_msg();
      }

      my $expected;

      $expected = 530;
      $self->assert($expected == $resp_code,
        test_msg("Expected $expected, got $resp_code"));

      $expected = "Access denied";
      $self->assert($expected eq $resp_msg,
        test_msg("Expected '$expected', got '$resp_msg'"));
    };

    if ($@) {
      $ex = $@;
    }

    $wfh->print("done\n");
    $wfh->flush();

  } else {
    eval { server_wait($config_file, $rfh, $timeout_idle + 2) };
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

  if (open($fh, "> $db_script")) {
    print $fh <<EOS;
DELETE FROM ftpdeny;
INSERT INTO ftpdeny (name, denied) VALUES ('', 'localhost');
EOS
    unless (close($fh)) {
      die("Can't write $db_script: $!");
    }

  } else {
    die("Can't open $db_script: $!");
  }

  $cmd = "sqlite3 $db_file < $db_script";

  if ($ENV{TEST_VERBOSE}) {
    print STDERR "Executing sqlite3: $cmd\n";
  }

  @output = `$cmd`;

  unlink($db_script);

  ($port, $config_user, $config_group) = config_write($config_file, $config);

  # Fork child
  $self->handle_sigchld();
  defined($pid = fork()) or die("Can't fork: $!");
  if ($pid) {
    eval {
      my $client = ProFTPD::TestSuite::FTP->new('127.0.0.1', $port);

      my ($resp_code, $resp_msg);

      eval { $client->login($user, $passwd) };
      unless ($@) {
        die("Login succeeded unexpectedly");

      } else {
        $resp_code = $client->response_code();
        $resp_msg = $client->response_msg();
      }

      my $expected;

      $expected = 530;
      $self->assert($expected == $resp_code,
        test_msg("Expected $expected, got $resp_code"));

      $expected = "Access denied";
      $self->assert($expected eq $resp_msg,
        test_msg("Expected '$expected', got '$resp_msg'"));
    };

    if ($@) {
      $ex = $@;
    }

    $wfh->print("done\n");
    $wfh->flush();

  } else {
    eval { server_wait($config_file, $rfh, $timeout_idle + 2) };
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

sub wrap2_sql_user_tables {
  my $self = shift;
  my $tmpdir = $self->{tmpdir};

  my $config_file = "$tmpdir/wrap2.conf";
  my $pid_file = File::Spec->rel2abs("$tmpdir/wrap2.pid");
  my $scoreboard_file = File::Spec->rel2abs("$tmpdir/wrap2.scoreboard");

  my $log_file = File::Spec->rel2abs('tests.log');

  my $auth_user_file = File::Spec->rel2abs("$tmpdir/wrap2.passwd");
  my $auth_group_file = File::Spec->rel2abs("$tmpdir/wrap2.group");

  my $user = 'proftpd';
  my $passwd = 'test';
  my $home_dir = File::Spec->rel2abs($tmpdir);
  my $uid = 500;
  my $gid = 500;

  my $db_file = File::Spec->rel2abs("$tmpdir/wrap2.db");

  # Build up sqlite3 command to create allow, deny tables and populate them
  my $db_script = File::Spec->rel2abs("$tmpdir/wrap2.sql");

  my $fh;
  if (open($fh, "> $db_script")) {
    print $fh <<EOS;
CREATE TABLE ftpallow (
  name TEXT,
  allowed TEXT
);

CREATE TABLE ftpdeny (
  name TEXT,
  denied TEXT
);

INSERT INTO ftpdeny (name, denied) VALUES ('$user', 'ALL');
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

  unlink($db_script);

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

  my $timeout_idle = 30;

  my $config = {
    PidFile => $pid_file,
    ScoreboardFile => $scoreboard_file,
    SystemLog => $log_file,

    AuthUserFile => $auth_user_file,
    AuthGroupFile => $auth_group_file,
    TimeoutIdle => $timeout_idle,

    IfModules => {
      'mod_delay.c' => {
        DelayEngine => 'off',
      },

      'mod_sql_sqlite.c' => [
        'SQLAuthenticate off',
        "SQLConnectInfo $db_file",
        'SQLNamedQuery get-allowed-clients SELECT "allowed FROM ftpallow WHERE name = \'%{0}\'"',
        'SQLNamedQuery get-denied-clients SELECT "denied FROM ftpdeny WHERE name = \'%{0}\'"',
        "SQLLogFile $log_file",
      ],

      'mod_wrap2_sql.c' => {
        WrapEngine => 'on',
        WrapUserTables => "!$user sql:/get-allowed-clients sql:/get-denied-clients",
        WrapLog => $log_file,
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

      ($resp_code, $resp_msg) = $client->login($user, $passwd);

      my $expected;

      $expected = 230;
      $self->assert($expected == $resp_code,
        test_msg("Expected $expected, got $resp_code"));

      $expected = "User $user logged in";
      $self->assert($expected eq $resp_msg,
        test_msg("Expected '$expected', got '$resp_msg'"));
    };

    if ($@) {
      $ex = $@;
    }

    $wfh->print("done\n");
    $wfh->flush();

  } else {
    eval { server_wait($config_file, $rfh, $timeout_idle + 2) };
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

  # Modify the config a little
  $config->{IfModules}->{'mod_wrap2_sql.c'}->{WrapUserTables} = "$user sql:/get-allowed-clients sql:/get-denied-clients";

  ($port, $config_user, $config_group) = config_write($config_file, $config);

  # Fork child
  $self->handle_sigchld();
  defined($pid = fork()) or die("Can't fork: $!");
  if ($pid) {
    eval {
      my $client = ProFTPD::TestSuite::FTP->new('127.0.0.1', $port);

      my ($resp_code, $resp_msg);

      eval { $client->login($user, $passwd) };
      unless ($@) {
        die("Login succeeded unexpectedly");

      } else {
        $resp_code = $client->response_code();
        $resp_msg = $client->response_msg();
      }

      my $expected;

      $expected = 530;
      $self->assert($expected == $resp_code,
        test_msg("Expected $expected, got $resp_code"));

      $expected = "Access denied";
      $self->assert($expected eq $resp_msg,
        test_msg("Expected '$expected', got '$resp_msg'"));
    };

    if ($@) {
      $ex = $@;
    }

    $wfh->print("done\n");
    $wfh->flush();

  } else {
    eval { server_wait($config_file, $rfh, $timeout_idle + 2) };
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

sub wrap2_sql_group_tables {
  my $self = shift;
  my $tmpdir = $self->{tmpdir};

  my $config_file = "$tmpdir/wrap2.conf";
  my $pid_file = File::Spec->rel2abs("$tmpdir/wrap2.pid");
  my $scoreboard_file = File::Spec->rel2abs("$tmpdir/wrap2.scoreboard");

  my $log_file = File::Spec->rel2abs('tests.log');

  my $auth_user_file = File::Spec->rel2abs("$tmpdir/wrap2.passwd");
  my $auth_group_file = File::Spec->rel2abs("$tmpdir/wrap2.group");

  my $user = 'proftpd';
  my $group = 'ftpd';
  my $passwd = 'test';
  my $home_dir = File::Spec->rel2abs($tmpdir);
  my $uid = 500;
  my $gid = 500;

  my $db_file = File::Spec->rel2abs("$tmpdir/wrap2.db");

  # Build up sqlite3 command to create allow, deny tables and populate them
  my $db_script = File::Spec->rel2abs("$tmpdir/wrap2.sql");

  my $fh;
  if (open($fh, "> $db_script")) {
    print $fh <<EOS;
CREATE TABLE ftpallow (
  name TEXT,
  allowed TEXT
);

CREATE TABLE ftpdeny (
  name TEXT,
  denied TEXT
);

INSERT INTO ftpdeny (name, denied) VALUES ('$group', 'ALL');
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

  unlink($db_script);

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

  my $timeout_idle = 30;

  my $config = {
    PidFile => $pid_file,
    ScoreboardFile => $scoreboard_file,
    SystemLog => $log_file,

    AuthUserFile => $auth_user_file,
    AuthGroupFile => $auth_group_file,
    TimeoutIdle => $timeout_idle,

    IfModules => {
      'mod_delay.c' => {
        DelayEngine => 'off',
      },

      'mod_sql_sqlite.c' => [
        'SQLAuthenticate off',
        "SQLConnectInfo $db_file",
        'SQLNamedQuery get-allowed-clients SELECT "allowed FROM ftpallow WHERE name = \'%{0}\'"',
        'SQLNamedQuery get-denied-clients SELECT "denied FROM ftpdeny WHERE name = \'%{0}\'"',
        "SQLLogFile $log_file",
      ],

      'mod_wrap2_sql.c' => {
        WrapEngine => 'on',
        WrapGroupTables => "foo sql:/get-allowed-clients sql:/get-denied-clients",
        WrapLog => $log_file,
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

      ($resp_code, $resp_msg) = $client->login($user, $passwd);

      my $expected;

      $expected = 230;
      $self->assert($expected == $resp_code,
        test_msg("Expected $expected, got $resp_code"));

      $expected = "User $user logged in";
      $self->assert($expected eq $resp_msg,
        test_msg("Expected '$expected', got '$resp_msg'"));
    };

    if ($@) {
      $ex = $@;
    }

    $wfh->print("done\n");
    $wfh->flush();

  } else {
    eval { server_wait($config_file, $rfh, $timeout_idle + 2) };
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

  # Modify the config a little
  $config->{IfModules}->{'mod_wrap2_sql.c'}->{WrapGroupTables} = "ftpd sql:/get-allowed-clients sql:/get-denied-clients";

  ($port, $config_user, $config_group) = config_write($config_file, $config);

  # Fork child
  $self->handle_sigchld();
  defined($pid = fork()) or die("Can't fork: $!");
  if ($pid) {
    eval {
      my $client = ProFTPD::TestSuite::FTP->new('127.0.0.1', $port);

      my ($resp_code, $resp_msg);

      eval { $client->login($user, $passwd) };
      unless ($@) {
        die("Login succeeded unexpectedly");

      } else {
        $resp_code = $client->response_code();
        $resp_msg = $client->response_msg();
      }

      my $expected;

      $expected = 530;
      $self->assert($expected == $resp_code,
        test_msg("Expected $expected, got $resp_code"));

      $expected = "Access denied";
      $self->assert($expected eq $resp_msg,
        test_msg("Expected '$expected', got '$resp_msg'"));
    };

    if ($@) {
      $ex = $@;
    }

    $wfh->print("done\n");
    $wfh->flush();

  } else {
    eval { server_wait($config_file, $rfh, $timeout_idle + 2) };
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

sub wrap2_sql_bug3215 {
  my $self = shift;
  my $tmpdir = $self->{tmpdir};

  my $config_file = "$tmpdir/wrap2.conf";
  my $pid_file = File::Spec->rel2abs("$tmpdir/wrap2.pid");
  my $scoreboard_file = File::Spec->rel2abs("$tmpdir/wrap2.scoreboard");

  my $log_file = File::Spec->rel2abs('tests.log');

  my $auth_user_file = File::Spec->rel2abs("$tmpdir/wrap2.passwd");
  my $auth_group_file = File::Spec->rel2abs("$tmpdir/wrap2.group");

  my $user = 'proftpd';
  my $group = 'ftpd';
  my $passwd = 'test';
  my $home_dir = File::Spec->rel2abs($tmpdir);
  my $uid = 500;
  my $gid = 500;

  my $db_file = File::Spec->rel2abs("$tmpdir/wrap2.db");

  # Build up sqlite3 command to create allow, deny tables and populate them
  my $db_script = File::Spec->rel2abs("$tmpdir/wrap2.sql");

  my $fh;
  if (open($fh, "> $db_script")) {
    print $fh <<EOS;
CREATE TABLE ftpallow (
  allowed TEXT
);

INSERT INTO ftpallow (allowed) VALUES ('192.168.0.1,192.168.0.2 192.168.0.3, 192.168.0.4 127.0.0.1');

CREATE TABLE ftpdeny (
  denied TEXT
);

INSERT INTO ftpdeny (denied) VALUES ('ALL');
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

  unlink($db_script);

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

  my $timeout_idle = 30;

  my $config = {
    PidFile => $pid_file,
    ScoreboardFile => $scoreboard_file,
    SystemLog => $log_file,

    AuthUserFile => $auth_user_file,
    AuthGroupFile => $auth_group_file,
    TimeoutIdle => $timeout_idle,

    IfModules => {
      'mod_delay.c' => {
        DelayEngine => 'off',
      },

      'mod_sql_sqlite.c' => [
        'SQLAuthenticate off',
        "SQLConnectInfo $db_file",
        'SQLNamedQuery get-allowed-clients SELECT "allowed FROM ftpallow"',
        'SQLNamedQuery get-denied-clients SELECT "denied FROM ftpdeny"',
        "SQLLogFile $log_file",
      ],

      'mod_wrap2_sql.c' => {
        WrapEngine => 'on',
        WrapTables => "sql:/get-allowed-clients sql:/get-denied-clients",
        WrapLog => $log_file,
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

      ($resp_code, $resp_msg) = $client->login($user, $passwd);

      my $expected;

      $expected = 230;
      $self->assert($expected == $resp_code,
        test_msg("Expected $expected, got $resp_code"));

      $expected = "User $user logged in";
      $self->assert($expected eq $resp_msg,
        test_msg("Expected '$expected', got '$resp_msg'"));
    };

    if ($@) {
      $ex = $@;
    }

    $wfh->print("done\n");
    $wfh->flush();

  } else {
    eval { server_wait($config_file, $rfh, $timeout_idle + 2) };
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

sub wrap2_bug3341 {
  my $self = shift;
  my $tmpdir = $self->{tmpdir};

  my $config_file = "$tmpdir/wrap2.conf";
  my $pid_file = File::Spec->rel2abs("$tmpdir/wrap2.pid");
  my $scoreboard_file = File::Spec->rel2abs("$tmpdir/wrap2.scoreboard");

  my $log_file = File::Spec->rel2abs('tests.log');

  my $auth_user_file = File::Spec->rel2abs("$tmpdir/wrap2.passwd");
  my $auth_group_file = File::Spec->rel2abs("$tmpdir/wrap2.group");

  my $user = 'proftpd';
  my $group = 'ftpd';
  my $passwd = 'test';
  my $home_dir = File::Spec->rel2abs($tmpdir);
  my $uid = 500;
  my $gid = 500;

  my $db_file = File::Spec->rel2abs("$tmpdir/wrap2.db");

  # Build up sqlite3 command to create allow, deny tables and populate them
  my $db_script = File::Spec->rel2abs("$tmpdir/wrap2.sql");

  my $fh;
  if (open($fh, "> $db_script")) {
    print $fh <<EOS;
CREATE TABLE ftpallow (
  allowed TEXT
);

INSERT INTO ftpallow (allowed) VALUES ('192.168.0.1,192.168.0.2 192.168.0.3, 192.168.0.4 127.0.0.1');

CREATE TABLE ftpdeny (
  denied TEXT
);

INSERT INTO ftpdeny (denied) VALUES ('ALL');
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

  unlink($db_script);

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

  my $timeout_idle = 30;

  my $config = {
    PidFile => $pid_file,
    ScoreboardFile => $scoreboard_file,
    SystemLog => $log_file,

    AuthUserFile => $auth_user_file,
    AuthGroupFile => $auth_group_file,
    TimeoutIdle => $timeout_idle,

    IfModules => {
      'mod_delay.c' => {
        DelayEngine => 'off',
      },

      'mod_sql_sqlite.c' => [
        'SQLAuthenticate off',
        "SQLConnectInfo $db_file",
        'SQLNamedQuery get-allowed-clients SELECT "allowed FROM ftpallow"',
        'SQLNamedQuery get-denied-clients SELECT "denied FROM ftpdeny"',
        "SQLLogFile $log_file",
      ],

      'mod_wrap2_sql.c' => {
        WrapEngine => 'on',
        WrapTables => "sql:/get-allowed-clients sql:/get-denied-clients",
        WrapLog => $log_file,
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

      # As per Bug#3341, we need to send a bad password twice.  The second
      # attempt triggered a segfault in mod_wrap2.
      eval { $client->login($user, 'foo') };
      unless ($@) {
        die("Login succeeded unexpectedly");
      }

      $resp_code = $client->response_code();
      $resp_msg = $client->response_msg(); 

      my $expected;

      $expected = 530;
      $self->assert($expected == $resp_code,
        test_msg("Expected $expected, got $resp_code"));

      $expected = "Login incorrect.";
      $self->assert($expected eq $resp_msg,
        test_msg("Expected '$expected', got '$resp_msg'"));

      # Now try to login again
      eval { $client->login($user, 'foo') };
      unless ($@) {
        die("Login succeeded unexpectedly");
      }

      $resp_code = $client->response_code();
      $resp_msg = $client->response_msg(); 

      $expected = 530;
      $self->assert($expected == $resp_code,
        test_msg("Expected $expected, got $resp_code"));

      $expected = "Login incorrect.";
      $self->assert($expected eq $resp_msg,
        test_msg("Expected '$expected', got '$resp_msg'"));

      ($resp_code, $resp_msg) = $client->login($user, $passwd);

      $expected = 230;
      $self->assert($expected == $resp_code,
        test_msg("Expected $expected, got $resp_code"));

      $expected = "User $user logged in";
      $self->assert($expected eq $resp_msg,
        test_msg("Expected '$expected', got '$resp_msg'"));
    };

    if ($@) {
      $ex = $@;
    }

    $wfh->print("done\n");
    $wfh->flush();

  } else {
    eval { server_wait($config_file, $rfh, $timeout_idle + 2) };
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
