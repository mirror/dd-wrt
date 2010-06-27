package ProFTPD::Tests::Modules::mod_sftp_sql;

use lib qw(t/lib);
use base qw(Test::Unit::TestCase ProFTPD::TestSuite::Child);
use strict;

use Digest::MD5;
use File::Copy;
use File::Path qw(mkpath rmtree);
use File::Spec;
use IO::Handle;
use POSIX qw(:fcntl_h);

use ProFTPD::TestSuite::Utils qw(:auth :config :features :running :test :testsuite);

$| = 1;

my $order = 0;

my $TESTS = {
  ssh2_auth_publickey_rsa_sql => {
    order => ++$order,
    test_class => [qw(forking ssh2)],
  },

  ssh2_auth_publickey_dsa_sql => {
    order => ++$order,
    test_class => [qw(forking ssh2)],
  },

  ssh2_auth_publickey_rsa_dsa_sql => {
    order => ++$order,
    test_class => [qw(forking ssh2)],
  },

  ssh2_auth_publickey_rsa_sql_rfc4716 => {
    order => ++$order,
    test_class => [qw(forking ssh2)],
  },

  ssh2_auth_publickey_dsa_sql_rfc4716 => {
    order => ++$order,
    test_class => [qw(forking ssh2)],
  },

  ssh2_auth_publickey_rsa_dsa_sql_rfc4716 => {
    order => ++$order,
    test_class => [qw(forking ssh2)],
  },

};

sub new {
  return shift()->SUPER::new(@_);
}

sub list_tests {
  # Check for the required Perl modules:
  #
  #  Net-SSH2
  #  Net-SSH2-SFTP

  my $required = [qw(
    Net::SSH2
    Net::SSH2::SFTP
  )];

  foreach my $req (@$required) {
    eval "use $req";
    if ($@) {
      print STDERR "\nWARNING:\n + Module '$req' not found, skipping all tests\n";

      if ($ENV{TEST_VERBOSE}) {
        print STDERR "Unable to load $req: $@\n";
      }

      return qw(testsuite_empty_test);
    }
  }

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

  # Make sure that mod_sftp does not complain about permissions on the hostkey
  # files.

  my $rsa_host_key = File::Spec->rel2abs('t/etc/modules/mod_sftp/ssh_host_rsa_key');
  my $dsa_host_key = File::Spec->rel2abs('t/etc/modules/mod_sftp/ssh_host_dsa_key');

  unless (chmod(0400, $rsa_host_key, $dsa_host_key)) {
    die("Can't set perms on $rsa_host_key, $dsa_host_key: $!");
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

sub ssh2_auth_publickey_rsa_sql {
  my $self = shift;
  my $tmpdir = $self->{tmpdir};

  my $config_file = "$tmpdir/sftp.conf";
  my $pid_file = File::Spec->rel2abs("$tmpdir/sftp.pid");
  my $scoreboard_file = File::Spec->rel2abs("$tmpdir/sftp.scoreboard");

  my $log_file = File::Spec->rel2abs('tests.log');

  my $auth_user_file = File::Spec->rel2abs("$tmpdir/sftp.passwd");
  my $auth_group_file = File::Spec->rel2abs("$tmpdir/sftp.group");

  my $user = 'proftpd';
  my $passwd = 'test';
  my $home_dir = File::Spec->rel2abs($tmpdir);
  my $uid = 500;
  my $gid = 500;

  my $db_file = File::Spec->rel2abs("$tmpdir/sftp.db");

  my $rsa_data = 'AAAAB3NzaC1yc2EAAAABIwAAAQEAzJ1CLwnVP9mUa8uyM+XBzxLxsRvGz4cS59aPTgdw7jGx1jCvC9ya400x7ej5Q4ubwlAAPblXzG5GYv2ROmYQ1DIjrhmR/61tDKUvAAZIgtvLZ00ydqqpq5lG4ubVJ4gW6sxbPfq/X12kV1gxGsFLUJCgoYInZGyIONrnvmQjFIfIx+mQXaK84uO6w0CT6KhRWgonajMrlO6P8O7qr80rFmOZsBNIMooyYrGTaMyxVsQK2SY+VKbXWFC+2HMmef62n+02ohAOBKtOsSOn8HE2wi7yMA0g8jRTd8kZcWBIkAhizPvl8pqG1F0DCmLn00rhPkByq2pv4VBo953gK7f1AQ==';

  my $db_script = File::Spec->rel2abs("$tmpdir/sftp.sql");

  my $fh;
  if (open($fh, "> $db_script")) {
    print $fh <<EOS;
CREATE TABLE sftpuserkeys (
  name TEXT NOT NULL,
  key BLOB NOT NULL
);

INSERT INTO sftpuserkeys (name, key) VALUES ('$user', '$rsa_data');
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

  my $rsa_host_key = File::Spec->rel2abs('t/etc/modules/mod_sftp/ssh_host_rsa_key');
  my $dsa_host_key = File::Spec->rel2abs('t/etc/modules/mod_sftp/ssh_host_dsa_key');

  my $rsa_priv_key = File::Spec->rel2abs('t/etc/modules/mod_sftp/test_rsa_key');  my $rsa_pub_key = File::Spec->rel2abs('t/etc/modules/mod_sftp/test_rsa_key.pub');

  my $config = {
    PidFile => $pid_file,
    ScoreboardFile => $scoreboard_file,
    SystemLog => $log_file,
    TraceLog => $log_file,
    Trace => 'DEFAULT:10 ssh2:20 sftp:20 scp:20',

    AuthUserFile => $auth_user_file,
    AuthGroupFile => $auth_group_file,

    IfModules => {
      'mod_delay.c' => {
        DelayEngine => 'off',
      },

      'mod_sql_sqlite.c' => {
        SQLAuthenticate => 'off',
        SQLConnectInfo => $db_file,
        SQLLogFile => $log_file,
        SQLNamedQuery => 'get-user-authorized-keys SELECT "key FROM sftpuserkeys WHERE name = \'%{0}\'"',
      },

      'mod_sftp.c' => [
        "SFTPEngine on",
        "SFTPLog $log_file",
        "SFTPHostKey $rsa_host_key",
        "SFTPHostKey $dsa_host_key",
        "SFTPAuthorizedUserKeys sql:/get-user-authorized-keys",
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

  require Net::SSH2;

  my $ex;

  # Fork child
  $self->handle_sigchld();
  defined(my $pid = fork()) or die("Can't fork: $!");
  if ($pid) {
    eval {
      my $ssh2 = Net::SSH2->new();

      sleep(1);

      unless ($ssh2->connect('127.0.0.1', $port)) {
        my ($err_code, $err_name, $err_str) = $ssh2->error();
        die("Can't connect to SSH2 server: [$err_name] ($err_code) $err_str");
      }

      unless ($ssh2->auth_publickey($user, $rsa_pub_key, $rsa_priv_key)) {
        my ($err_code, $err_name, $err_str) = $ssh2->error();
        die("RSA publickey authentication failed: [$err_name] ($err_code) $err_str");
      }

      $ssh2->disconnect();
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

sub ssh2_auth_publickey_dsa_sql {
  my $self = shift;
  my $tmpdir = $self->{tmpdir};

  my $config_file = "$tmpdir/sftp.conf";
  my $pid_file = File::Spec->rel2abs("$tmpdir/sftp.pid");
  my $scoreboard_file = File::Spec->rel2abs("$tmpdir/sftp.scoreboard");

  my $log_file = File::Spec->rel2abs('tests.log');

  my $auth_user_file = File::Spec->rel2abs("$tmpdir/sftp.passwd");
  my $auth_group_file = File::Spec->rel2abs("$tmpdir/sftp.group");

  my $user = 'proftpd';
  my $passwd = 'test';
  my $home_dir = File::Spec->rel2abs($tmpdir);
  my $uid = 500;
  my $gid = 500;

  my $db_file = File::Spec->rel2abs("$tmpdir/sftp.db");

  my $dsa_data = 'AAAAB3NzaC1kc3MAAACBAJWsBFKoQ1HdluDnVacaPm+PDPuXAMxwpplFYafjDCye5Y2FZEjSA8GlLAsEk3XCw6JK2+ciE0I/XXn3+3cAkrdjDKQDRn8IcAANatgLSSjxS9XN4oNzKtrjSs3hOhv5ADhwKMcIhwSSf0cVRH9sGjQF3IiJBVnQjz6i//v+97otAAAAFQDqbZr3aGXlY0Q3UYS3XRNV8ZdWLwAAAIBVZ00AvPF6Kh9GUvXdUAEtNfe1JIThzC4rSc2Fyz3/mQA+GLUHtEq1ZLpO5D4PCjxHcR7N1tIarqR1f0GGjDGvaPocPNtMpaVxCsLWTCnua5fhIgZ3H24mSzd/j2TSDqIeV0DWWiWsFOTF+UzbxSDAinfDjLGV0XQ1jRhgoHS3hAAAAIAfONFEnSPWVLEl0Hv9yAozdWBmkyDijkk0zWWGh17z/Ef4ZRo5bwjsOQCx3c3XswdjDacoqUjep8EMs8MI4LpOx8SybGOGu8xyW3ceT+rdP1/S62JxBeL7SaIwCH0O9sPqeh44l8OHIKVh6lmyUr1KQFOnNJ/pRF0JutY1UDEUMQ==';

  my $db_script = File::Spec->rel2abs("$tmpdir/sftp.sql");

  my $fh;
  if (open($fh, "> $db_script")) {
    print $fh <<EOS;
CREATE TABLE sftpuserkeys (
  name TEXT NOT NULL,
  key BLOB NOT NULL
);

INSERT INTO sftpuserkeys (name, key) VALUES ('$user', '$dsa_data');
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

  my $rsa_host_key = File::Spec->rel2abs('t/etc/modules/mod_sftp/ssh_host_rsa_key');
  my $dsa_host_key = File::Spec->rel2abs('t/etc/modules/mod_sftp/ssh_host_dsa_key');

  my $dsa_priv_key = File::Spec->rel2abs('t/etc/modules/mod_sftp/test_dsa_key');
  my $dsa_pub_key = File::Spec->rel2abs('t/etc/modules/mod_sftp/test_dsa_key.pub');

  my $config = {
    PidFile => $pid_file,
    ScoreboardFile => $scoreboard_file,
    SystemLog => $log_file,
    TraceLog => $log_file,
    Trace => 'DEFAULT:10 ssh2:20 sftp:20 scp:20',

    AuthUserFile => $auth_user_file,
    AuthGroupFile => $auth_group_file,

    IfModules => {
      'mod_delay.c' => {
        DelayEngine => 'off',
      },

      'mod_sql_sqlite.c' => {
        SQLAuthenticate => 'off',
        SQLConnectInfo => $db_file,
        SQLLogFile => $log_file,
        SQLNamedQuery => 'get-user-authorized-keys SELECT "key FROM sftpuserkeys WHERE name = \'%{0}\'"',
      },

      'mod_sftp.c' => [
        "SFTPEngine on",
        "SFTPLog $log_file",
        "SFTPHostKey $rsa_host_key",
        "SFTPHostKey $dsa_host_key",
        "SFTPAuthorizedUserKeys sql:/get-user-authorized-keys",
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

  require Net::SSH2;

  my $ex;

  # Fork child
  $self->handle_sigchld();
  defined(my $pid = fork()) or die("Can't fork: $!");
  if ($pid) {
    eval {
      my $ssh2 = Net::SSH2->new();

      sleep(1);

      unless ($ssh2->connect('127.0.0.1', $port)) {
        my ($err_code, $err_name, $err_str) = $ssh2->error();
        die("Can't connect to SSH2 server: [$err_name] ($err_code) $err_str");
      }

      unless ($ssh2->auth_publickey($user, $dsa_pub_key, $dsa_priv_key)) {
        my ($err_code, $err_name, $err_str) = $ssh2->error();
        die("DSA publickey authentication failed: [$err_name] ($err_code) $err_str");
      }

      $ssh2->disconnect();
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

sub ssh2_auth_publickey_rsa_dsa_sql {
  my $self = shift;
  my $tmpdir = $self->{tmpdir};

  my $config_file = "$tmpdir/sftp.conf";
  my $pid_file = File::Spec->rel2abs("$tmpdir/sftp.pid");
  my $scoreboard_file = File::Spec->rel2abs("$tmpdir/sftp.scoreboard");

  my $log_file = File::Spec->rel2abs('tests.log');

  my $auth_user_file = File::Spec->rel2abs("$tmpdir/sftp.passwd");
  my $auth_group_file = File::Spec->rel2abs("$tmpdir/sftp.group");

  my $user = 'proftpd';
  my $passwd = 'test';
  my $home_dir = File::Spec->rel2abs($tmpdir);
  my $uid = 500;
  my $gid = 500;

  my $db_file = File::Spec->rel2abs("$tmpdir/sftp.db");

  my $rsa_data = 'AAAAB3NzaC1yc2EAAAABIwAAAQEAzJ1CLwnVP9mUa8uyM+XBzxLxsRvGz4cS59aPTgdw7jGx1jCvC9ya400x7ej5Q4ubwlAAPblXzG5GYv2ROmYQ1DIjrhmR/61tDKUvAAZIgtvLZ00ydqqpq5lG4ubVJ4gW6sxbPfq/X12kV1gxGsFLUJCgoYInZGyIONrnvmQjFIfIx+mQXaK84uO6w0CT6KhRWgonajMrlO6P8O7qr80rFmOZsBNIMooyYrGTaMyxVsQK2SY+VKbXWFC+2HMmef62n+02ohAOBKtOsSOn8HE2wi7yMA0g8jRTd8kZcWBIkAhizPvl8pqG1F0DCmLn00rhPkByq2pv4VBo953gK7f1AQ==';
  my $dsa_data = 'AAAAB3NzaC1kc3MAAACBAJWsBFKoQ1HdluDnVacaPm+PDPuXAMxwpplFYafjDCye5Y2FZEjSA8GlLAsEk3XCw6JK2+ciE0I/XXn3+3cAkrdjDKQDRn8IcAANatgLSSjxS9XN4oNzKtrjSs3hOhv5ADhwKMcIhwSSf0cVRH9sGjQF3IiJBVnQjz6i//v+97otAAAAFQDqbZr3aGXlY0Q3UYS3XRNV8ZdWLwAAAIBVZ00AvPF6Kh9GUvXdUAEtNfe1JIThzC4rSc2Fyz3/mQA+GLUHtEq1ZLpO5D4PCjxHcR7N1tIarqR1f0GGjDGvaPocPNtMpaVxCsLWTCnua5fhIgZ3H24mSzd/j2TSDqIeV0DWWiWsFOTF+UzbxSDAinfDjLGV0XQ1jRhgoHS3hAAAAIAfONFEnSPWVLEl0Hv9yAozdWBmkyDijkk0zWWGh17z/Ef4ZRo5bwjsOQCx3c3XswdjDacoqUjep8EMs8MI4LpOx8SybGOGu8xyW3ceT+rdP1/S62JxBeL7SaIwCH0O9sPqeh44l8OHIKVh6lmyUr1KQFOnNJ/pRF0JutY1UDEUMQ==';

  my $db_script = File::Spec->rel2abs("$tmpdir/sftp.sql");

  my $fh;
  if (open($fh, "> $db_script")) {
    print $fh <<EOS;
CREATE TABLE sftpuserkeys (
  name TEXT NOT NULL,
  key BLOB NOT NULL
);

INSERT INTO sftpuserkeys (name, key) VALUES ('$user', '$rsa_data');
INSERT INTO sftpuserkeys (name, key) VALUES ('$user', '$dsa_data');
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

  my $rsa_host_key = File::Spec->rel2abs('t/etc/modules/mod_sftp/ssh_host_rsa_key');
  my $dsa_host_key = File::Spec->rel2abs('t/etc/modules/mod_sftp/ssh_host_dsa_key');

  my $dsa_priv_key = File::Spec->rel2abs('t/etc/modules/mod_sftp/test_dsa_key');
  my $dsa_pub_key = File::Spec->rel2abs('t/etc/modules/mod_sftp/test_dsa_key.pub');

  my $config = {
    PidFile => $pid_file,
    ScoreboardFile => $scoreboard_file,
    SystemLog => $log_file,
    TraceLog => $log_file,
    Trace => 'DEFAULT:10 ssh2:20 sftp:20 scp:20',

    AuthUserFile => $auth_user_file,
    AuthGroupFile => $auth_group_file,

    IfModules => {
      'mod_delay.c' => {
        DelayEngine => 'off',
      },

      'mod_sql_sqlite.c' => {
        SQLAuthenticate => 'off',
        SQLConnectInfo => $db_file,
        SQLLogFile => $log_file,
        SQLNamedQuery => 'get-user-authorized-keys SELECT "key FROM sftpuserkeys WHERE name = \'%{0}\'"',
      },

      'mod_sftp.c' => [
        "SFTPEngine on",
        "SFTPLog $log_file",
        "SFTPHostKey $rsa_host_key",
        "SFTPHostKey $dsa_host_key",
        "SFTPAuthorizedUserKeys sql:/get-user-authorized-keys",
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

  require Net::SSH2;

  my $ex;

  # Fork child
  $self->handle_sigchld();
  defined(my $pid = fork()) or die("Can't fork: $!");
  if ($pid) {
    eval {
      my $ssh2 = Net::SSH2->new();

      sleep(1);

      unless ($ssh2->connect('127.0.0.1', $port)) {
        my ($err_code, $err_name, $err_str) = $ssh2->error();
        die("Can't connect to SSH2 server: [$err_name] ($err_code) $err_str");
      }

      unless ($ssh2->auth_publickey($user, $dsa_pub_key, $dsa_priv_key)) {
        my ($err_code, $err_name, $err_str) = $ssh2->error();
        die("DSA publickey authentication failed: [$err_name] ($err_code) $err_str");
      }

      $ssh2->disconnect();
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

sub ssh2_auth_publickey_rsa_sql_rfc4716 {
  my $self = shift;
  my $tmpdir = $self->{tmpdir};

  my $config_file = "$tmpdir/sftp.conf";
  my $pid_file = File::Spec->rel2abs("$tmpdir/sftp.pid");
  my $scoreboard_file = File::Spec->rel2abs("$tmpdir/sftp.scoreboard");

  my $log_file = File::Spec->rel2abs('tests.log');

  my $auth_user_file = File::Spec->rel2abs("$tmpdir/sftp.passwd");
  my $auth_group_file = File::Spec->rel2abs("$tmpdir/sftp.group");

  my $user = 'proftpd';
  my $passwd = 'test';
  my $home_dir = File::Spec->rel2abs($tmpdir);
  my $uid = 500;
  my $gid = 500;

  my $db_file = File::Spec->rel2abs("$tmpdir/sftp.db");

  my $rsa_rfc4716_data = '---- BEGIN SSH2 PUBLIC KEY ----
Comment: "2048-bit RSA, converted from OpenSSH by tj@familiar"
AAAAB3NzaC1yc2EAAAABIwAAAQEAzJ1CLwnVP9mUa8uyM+XBzxLxsRvGz4cS59aPTgdw7j
Gx1jCvC9ya400x7ej5Q4ubwlAAPblXzG5GYv2ROmYQ1DIjrhmR/61tDKUvAAZIgtvLZ00y
dqqpq5lG4ubVJ4gW6sxbPfq/X12kV1gxGsFLUJCgoYInZGyIONrnvmQjFIfIx+mQXaK84u
O6w0CT6KhRWgonajMrlO6P8O7qr80rFmOZsBNIMooyYrGTaMyxVsQK2SY+VKbXWFC+2HMm
ef62n+02ohAOBKtOsSOn8HE2wi7yMA0g8jRTd8kZcWBIkAhizPvl8pqG1F0DCmLn00rhPk
Byq2pv4VBo953gK7f1AQ==
---- END SSH2 PUBLIC KEY ----';

  my $db_script = File::Spec->rel2abs("$tmpdir/sftp.sql");

  my $fh;
  if (open($fh, "> $db_script")) {
    print $fh <<EOS;
CREATE TABLE sftpuserkeys (
  name TEXT NOT NULL,
  key BLOB NOT NULL
);

INSERT INTO sftpuserkeys (name, key) VALUES ('$user', '$rsa_rfc4716_data');
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

  my $rsa_host_key = File::Spec->rel2abs('t/etc/modules/mod_sftp/ssh_host_rsa_key');
  my $dsa_host_key = File::Spec->rel2abs('t/etc/modules/mod_sftp/ssh_host_dsa_key');

  my $rsa_priv_key = File::Spec->rel2abs('t/etc/modules/mod_sftp/test_rsa_key');  my $rsa_pub_key = File::Spec->rel2abs('t/etc/modules/mod_sftp/test_rsa_key.pub');

  my $config = {
    PidFile => $pid_file,
    ScoreboardFile => $scoreboard_file,
    SystemLog => $log_file,
    TraceLog => $log_file,
    Trace => 'DEFAULT:10 ssh2:20 sftp:20 scp:20',

    AuthUserFile => $auth_user_file,
    AuthGroupFile => $auth_group_file,

    IfModules => {
      'mod_delay.c' => {
        DelayEngine => 'off',
      },

      'mod_sql_sqlite.c' => {
        SQLAuthenticate => 'off',
        SQLConnectInfo => $db_file,
        SQLLogFile => $log_file,
        SQLNamedQuery => 'get-user-authorized-keys SELECT "key FROM sftpuserkeys WHERE name = \'%{0}\'"',
      },

      'mod_sftp.c' => [
        "SFTPEngine on",
        "SFTPLog $log_file",
        "SFTPHostKey $rsa_host_key",
        "SFTPHostKey $dsa_host_key",
        "SFTPAuthorizedUserKeys sql:/get-user-authorized-keys",
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

  require Net::SSH2;

  my $ex;

  # Fork child
  $self->handle_sigchld();
  defined(my $pid = fork()) or die("Can't fork: $!");
  if ($pid) {
    eval {
      my $ssh2 = Net::SSH2->new();

      sleep(1);

      unless ($ssh2->connect('127.0.0.1', $port)) {
        my ($err_code, $err_name, $err_str) = $ssh2->error();
        die("Can't connect to SSH2 server: [$err_name] ($err_code) $err_str");
      }

      unless ($ssh2->auth_publickey($user, $rsa_pub_key, $rsa_priv_key)) {
        my ($err_code, $err_name, $err_str) = $ssh2->error();
        die("RSA publickey authentication failed: [$err_name] ($err_code) $err_str");
      }

      $ssh2->disconnect();
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

sub ssh2_auth_publickey_dsa_sql_rfc4716 {
  my $self = shift;
  my $tmpdir = $self->{tmpdir};

  my $config_file = "$tmpdir/sftp.conf";
  my $pid_file = File::Spec->rel2abs("$tmpdir/sftp.pid");
  my $scoreboard_file = File::Spec->rel2abs("$tmpdir/sftp.scoreboard");

  my $log_file = File::Spec->rel2abs('tests.log');

  my $auth_user_file = File::Spec->rel2abs("$tmpdir/sftp.passwd");
  my $auth_group_file = File::Spec->rel2abs("$tmpdir/sftp.group");

  my $user = 'proftpd';
  my $passwd = 'test';
  my $home_dir = File::Spec->rel2abs($tmpdir);
  my $uid = 500;
  my $gid = 500;

  my $db_file = File::Spec->rel2abs("$tmpdir/sftp.db");

  my $dsa_rfc4716_data = '
---- BEGIN SSH2 PUBLIC KEY ----
Comment: "1024-bit DSA, converted from OpenSSH by tj@familiar"
AAAAB3NzaC1kc3MAAACBAJWsBFKoQ1HdluDnVacaPm+PDPuXAMxwpplFYafjDCye5Y2FZE
jSA8GlLAsEk3XCw6JK2+ciE0I/XXn3+3cAkrdjDKQDRn8IcAANatgLSSjxS9XN4oNzKtrj
Ss3hOhv5ADhwKMcIhwSSf0cVRH9sGjQF3IiJBVnQjz6i//v+97otAAAAFQDqbZr3aGXlY0
Q3UYS3XRNV8ZdWLwAAAIBVZ00AvPF6Kh9GUvXdUAEtNfe1JIThzC4rSc2Fyz3/mQA+GLUH
tEq1ZLpO5D4PCjxHcR7N1tIarqR1f0GGjDGvaPocPNtMpaVxCsLWTCnua5fhIgZ3H24mSz
d/j2TSDqIeV0DWWiWsFOTF+UzbxSDAinfDjLGV0XQ1jRhgoHS3hAAAAIAfONFEnSPWVLEl
0Hv9yAozdWBmkyDijkk0zWWGh17z/Ef4ZRo5bwjsOQCx3c3XswdjDacoqUjep8EMs8MI4L
pOx8SybGOGu8xyW3ceT+rdP1/S62JxBeL7SaIwCH0O9sPqeh44l8OHIKVh6lmyUr1KQFOn
NJ/pRF0JutY1UDEUMQ==
---- END SSH2 PUBLIC KEY ----
';

  my $db_script = File::Spec->rel2abs("$tmpdir/sftp.sql");

  my $fh;
  if (open($fh, "> $db_script")) {
    print $fh <<EOS;
CREATE TABLE sftpuserkeys (
  name TEXT NOT NULL,
  key BLOB NOT NULL
);

INSERT INTO sftpuserkeys (name, key) VALUES ('$user', '$dsa_rfc4716_data');
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

  my $rsa_host_key = File::Spec->rel2abs('t/etc/modules/mod_sftp/ssh_host_rsa_key');
  my $dsa_host_key = File::Spec->rel2abs('t/etc/modules/mod_sftp/ssh_host_dsa_key');

  my $dsa_priv_key = File::Spec->rel2abs('t/etc/modules/mod_sftp/test_dsa_key');
  my $dsa_pub_key = File::Spec->rel2abs('t/etc/modules/mod_sftp/test_dsa_key.pub');

  my $config = {
    PidFile => $pid_file,
    ScoreboardFile => $scoreboard_file,
    SystemLog => $log_file,
    TraceLog => $log_file,
    Trace => 'DEFAULT:10 ssh2:20 sftp:20 scp:20',

    AuthUserFile => $auth_user_file,
    AuthGroupFile => $auth_group_file,

    IfModules => {
      'mod_delay.c' => {
        DelayEngine => 'off',
      },

      'mod_sql_sqlite.c' => {
        SQLAuthenticate => 'off',
        SQLConnectInfo => $db_file,
        SQLLogFile => $log_file,
        SQLNamedQuery => 'get-user-authorized-keys SELECT "key FROM sftpuserkeys WHERE name = \'%{0}\'"',
      },

      'mod_sftp.c' => [
        "SFTPEngine on",
        "SFTPLog $log_file",
        "SFTPHostKey $rsa_host_key",
        "SFTPHostKey $dsa_host_key",
        "SFTPAuthorizedUserKeys sql:/get-user-authorized-keys",
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

  require Net::SSH2;

  my $ex;

  # Fork child
  $self->handle_sigchld();
  defined(my $pid = fork()) or die("Can't fork: $!");
  if ($pid) {
    eval {
      my $ssh2 = Net::SSH2->new();

      sleep(1);

      unless ($ssh2->connect('127.0.0.1', $port)) {
        my ($err_code, $err_name, $err_str) = $ssh2->error();
        die("Can't connect to SSH2 server: [$err_name] ($err_code) $err_str");
      }

      unless ($ssh2->auth_publickey($user, $dsa_pub_key, $dsa_priv_key)) {
        my ($err_code, $err_name, $err_str) = $ssh2->error();
        die("DSA publickey authentication failed: [$err_name] ($err_code) $err_str");
      }

      $ssh2->disconnect();
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

sub ssh2_auth_publickey_rsa_dsa_sql_rfc4716 {
  my $self = shift;
  my $tmpdir = $self->{tmpdir};

  my $config_file = "$tmpdir/sftp.conf";
  my $pid_file = File::Spec->rel2abs("$tmpdir/sftp.pid");
  my $scoreboard_file = File::Spec->rel2abs("$tmpdir/sftp.scoreboard");

  my $log_file = File::Spec->rel2abs('tests.log');

  my $auth_user_file = File::Spec->rel2abs("$tmpdir/sftp.passwd");
  my $auth_group_file = File::Spec->rel2abs("$tmpdir/sftp.group");

  my $user = 'proftpd';
  my $passwd = 'test';
  my $home_dir = File::Spec->rel2abs($tmpdir);
  my $uid = 500;
  my $gid = 500;

  my $db_file = File::Spec->rel2abs("$tmpdir/sftp.db");

  my $rsa_rfc4716_data = '---- BEGIN SSH2 PUBLIC KEY ----
Comment: "2048-bit RSA, converted from OpenSSH by tj@familiar"
AAAAB3NzaC1yc2EAAAABIwAAAQEAzJ1CLwnVP9mUa8uyM+XBzxLxsRvGz4cS59aPTgdw7j
Gx1jCvC9ya400x7ej5Q4ubwlAAPblXzG5GYv2ROmYQ1DIjrhmR/61tDKUvAAZIgtvLZ00y
dqqpq5lG4ubVJ4gW6sxbPfq/X12kV1gxGsFLUJCgoYInZGyIONrnvmQjFIfIx+mQXaK84u
O6w0CT6KhRWgonajMrlO6P8O7qr80rFmOZsBNIMooyYrGTaMyxVsQK2SY+VKbXWFC+2HMm
ef62n+02ohAOBKtOsSOn8HE2wi7yMA0g8jRTd8kZcWBIkAhizPvl8pqG1F0DCmLn00rhPk
Byq2pv4VBo953gK7f1AQ==
---- END SSH2 PUBLIC KEY ----';
  my $dsa_rfc4716_data = '
---- BEGIN SSH2 PUBLIC KEY ----
Comment: "1024-bit DSA, converted from OpenSSH by tj@familiar"
AAAAB3NzaC1kc3MAAACBAJWsBFKoQ1HdluDnVacaPm+PDPuXAMxwpplFYafjDCye5Y2FZE
jSA8GlLAsEk3XCw6JK2+ciE0I/XXn3+3cAkrdjDKQDRn8IcAANatgLSSjxS9XN4oNzKtrj
Ss3hOhv5ADhwKMcIhwSSf0cVRH9sGjQF3IiJBVnQjz6i//v+97otAAAAFQDqbZr3aGXlY0
Q3UYS3XRNV8ZdWLwAAAIBVZ00AvPF6Kh9GUvXdUAEtNfe1JIThzC4rSc2Fyz3/mQA+GLUH
tEq1ZLpO5D4PCjxHcR7N1tIarqR1f0GGjDGvaPocPNtMpaVxCsLWTCnua5fhIgZ3H24mSz
d/j2TSDqIeV0DWWiWsFOTF+UzbxSDAinfDjLGV0XQ1jRhgoHS3hAAAAIAfONFEnSPWVLEl
0Hv9yAozdWBmkyDijkk0zWWGh17z/Ef4ZRo5bwjsOQCx3c3XswdjDacoqUjep8EMs8MI4L
pOx8SybGOGu8xyW3ceT+rdP1/S62JxBeL7SaIwCH0O9sPqeh44l8OHIKVh6lmyUr1KQFOn
NJ/pRF0JutY1UDEUMQ==
---- END SSH2 PUBLIC KEY ----
';

  my $db_script = File::Spec->rel2abs("$tmpdir/sftp.sql");

  my $fh;
  if (open($fh, "> $db_script")) {
    print $fh <<EOS;
CREATE TABLE sftpuserkeys (
  name TEXT NOT NULL,
  key BLOB NOT NULL
);

INSERT INTO sftpuserkeys (name, key) VALUES ('$user', '$rsa_rfc4716_data');
INSERT INTO sftpuserkeys (name, key) VALUES ('$user', '$dsa_rfc4716_data');
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

  my $rsa_host_key = File::Spec->rel2abs('t/etc/modules/mod_sftp/ssh_host_rsa_key');
  my $dsa_host_key = File::Spec->rel2abs('t/etc/modules/mod_sftp/ssh_host_dsa_key');

  my $dsa_priv_key = File::Spec->rel2abs('t/etc/modules/mod_sftp/test_dsa_key');
  my $dsa_pub_key = File::Spec->rel2abs('t/etc/modules/mod_sftp/test_dsa_key.pub');

  my $config = {
    PidFile => $pid_file,
    ScoreboardFile => $scoreboard_file,
    SystemLog => $log_file,
    TraceLog => $log_file,
    Trace => 'DEFAULT:10 ssh2:20 sftp:20 scp:20',

    AuthUserFile => $auth_user_file,
    AuthGroupFile => $auth_group_file,

    IfModules => {
      'mod_delay.c' => {
        DelayEngine => 'off',
      },

      'mod_sql_sqlite.c' => {
        SQLAuthenticate => 'off',
        SQLConnectInfo => $db_file,
        SQLLogFile => $log_file,
        SQLNamedQuery => 'get-user-authorized-keys SELECT "key FROM sftpuserkeys WHERE name = \'%{0}\'"',
      },

      'mod_sftp.c' => [
        "SFTPEngine on",
        "SFTPLog $log_file",
        "SFTPHostKey $rsa_host_key",
        "SFTPHostKey $dsa_host_key",
        "SFTPAuthorizedUserKeys sql:/get-user-authorized-keys",
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

  require Net::SSH2;

  my $ex;

  # Fork child
  $self->handle_sigchld();
  defined(my $pid = fork()) or die("Can't fork: $!");
  if ($pid) {
    eval {
      my $ssh2 = Net::SSH2->new();

      sleep(1);

      unless ($ssh2->connect('127.0.0.1', $port)) {
        my ($err_code, $err_name, $err_str) = $ssh2->error();
        die("Can't connect to SSH2 server: [$err_name] ($err_code) $err_str");
      }

      unless ($ssh2->auth_publickey($user, $dsa_pub_key, $dsa_priv_key)) {
        my ($err_code, $err_name, $err_str) = $ssh2->error();
        die("DSA publickey authentication failed: [$err_name] ($err_code) $err_str");
      }

      $ssh2->disconnect();
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
