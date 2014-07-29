package ProFTPD::Tests::Config::AllowForeignAddress;

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
  fxp_denied => {
    order => ++$order,
    test_class => [qw(forking)],
  },

  fxp_allowed => {
    order => ++$order,
    test_class => [qw(forking)],
  },

  # tls_fxp_allowed
};

sub new {
  return shift()->SUPER::new(@_);
}

sub list_tests {
  return testsuite_get_runnable_tests($TESTS);
}

sub fxp_denied {
  my $self = shift;
  my $tmpdir = $self->{tmpdir};

  my $config_file = "$tmpdir/cmds.conf";
  my $pid_file = File::Spec->rel2abs("$tmpdir/cmds.pid");
  my $scoreboard_file = File::Spec->rel2abs("$tmpdir/cmds.scoreboard");

  my $log_file = test_get_logfile();

  my $auth_user_file = File::Spec->rel2abs("$tmpdir/cmds.passwd");
  my $auth_group_file = File::Spec->rel2abs("$tmpdir/cmds.group");

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

  my $src_file = File::Spec->rel2abs("$tmpdir/src.txt");
  if (open(my $fh, "> $src_file")) {
    print $fh "Hello, FXP World!\n";

    unless (close($fh)) {
      die("Can't write $src_file: $!");
    }

  } else {
    die("Can't open $src_file: $!");
  }

  my $dst_file = File::Spec->rel2abs("$tmpdir/dst.txt");

  my $config = {
    PidFile => $pid_file,
    ScoreboardFile => $scoreboard_file,
    SystemLog => $log_file,

    AuthUserFile => $auth_user_file,
    AuthGroupFile => $auth_group_file,

    AllowForeignAddress => 'off',

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
      my $client1 = ProFTPD::TestSuite::FTP->new('127.0.0.1', $port, 1);
      $client1->login($user, $passwd);

      my $client2 = ProFTPD::TestSuite::FTP->new('127.0.0.1', $port);
      $client2->login($user, $passwd);

      # Get the PASV address from the first connection, and give it
      # to the second connection as a PORT command.
      my ($resp_code, $resp_msg) = $client1->pasv();

      my $expected = 227;
      $self->assert($expected == $resp_code,
        test_msg("Expected response code $expected, got $resp_code"));

      $expected = '^Entering Passive Mode \(\d+,\d+,\d+,\d+,\d+,\d+\)';
      $self->assert(qr/$expected/, $resp_msg,
        test_msg("Expected response message '$expected', got '$resp_msg'"));

      # Deliberately provide an IP address that does not match our source
      # IP address, to trigger the AllowForeignAddress restriction.
      my $port_addr = '1,2,3,4,192,6';
      eval { $client2->port($port_addr) };
      unless ($@) {
        die("PORT $port_addr succeeded unexpectedly");
      }

      $resp_code = $client2->response_code();
      $resp_msg = $client2->response_msg();

      $expected = 500;
      $self->assert($expected == $resp_code,
        test_msg("Expected response code $expected, got $resp_code"));

      $expected = 'Illegal PORT command';
      $self->assert($expected eq $resp_msg,
        test_msg("Expected response message '$expected', got '$resp_msg'"));

      $client1->quit();
      $client2->quit();

      $self->assert(!-f $dst_file,
        test_msg("File $dst_file exists unexpectedly"));
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

  eval {
    if (open(my $fh, "< $log_file")) {
      my $ok = 0;

      while (my $line = <$fh>) {
        chomp($line);

        if ($line =~ /Refused PORT .*? \(address mismatch\)/) {
          $ok = 1;
          last;
        }
      }

      close($fh);

      $self->assert($ok, test_msg("Did not see expected log messages"));

    } else {
      die("Can't read $log_file: $!");
    }
  };
  if ($@) {
    $ex = $@;
  }

  if ($ex) {
    test_append_logfile($log_file, $ex);
    unlink($log_file);

    die($ex);
  }

  unlink($log_file);
}

sub fxp_allowed {
  my $self = shift;
  my $tmpdir = $self->{tmpdir};

  my $config_file = "$tmpdir/cmds.conf";
  my $pid_file = File::Spec->rel2abs("$tmpdir/cmds.pid");
  my $scoreboard_file = File::Spec->rel2abs("$tmpdir/cmds.scoreboard");

  my $log_file = test_get_logfile();

  my $auth_user_file = File::Spec->rel2abs("$tmpdir/cmds.passwd");
  my $auth_group_file = File::Spec->rel2abs("$tmpdir/cmds.group");

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

  my $src_file = File::Spec->rel2abs("$tmpdir/src.txt");
  if (open(my $fh, "> $src_file")) {
    print $fh "Hello, FXP World!\n";

    unless (close($fh)) {
      die("Can't write $src_file: $!");
    }

  } else {
    die("Can't open $src_file: $!");
  }

  my $dst_file = File::Spec->rel2abs("$tmpdir/dst.txt");

  my $config = {
    PidFile => $pid_file,
    ScoreboardFile => $scoreboard_file,
    SystemLog => $log_file,

    AuthUserFile => $auth_user_file,
    AuthGroupFile => $auth_group_file,

    AllowForeignAddress => 'on',

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
      my $client1 = ProFTPD::TestSuite::FTP->new('127.0.0.1', $port, 1);
      $client1->login($user, $passwd);

      my $client2 = ProFTPD::TestSuite::FTP->new('127.0.0.1', $port);
      $client2->login($user, $passwd);

      # Get the PASV address from the first connection, and give it
      # to the second connection as a PORT command.
      my ($resp_code, $resp_msg) = $client1->pasv();

      my $expected = 227;
      $self->assert($expected == $resp_code,
        test_msg("Expected response code $expected, got $resp_code"));

      $expected = '^Entering Passive Mode \(\d+,\d+,\d+,\d+,\d+,\d+\)';
      $self->assert(qr/$expected/, $resp_msg,
        test_msg("Expected response message '$expected', got '$resp_msg'"));

      # This will actually work, since both our connections are
      # from 127.0.0.1, which means we shouldn't run afoul of the
      # AllowForeignAddress limit.
      $resp_msg =~ /'^Entering Passive Mode \((\d+,\d+,\d+,\d+,\d+,\d+\))/;
      my $port_addr = $1;

      ($resp_code, $resp_msg) = $client2->port($port_addr);

      $expected = 200;
      $self->assert($expected == $resp_code,
        test_msg("Expected response code $expected, got $resp_code"));

      $expected = 'PORT command successful';
      $self->assert($expected eq $resp_msg,
        test_msg("Expected response message '$expected', got '$resp_msg'"));

      my $tmpfile = 'tmpfile.bin';
      ($resp_code, $resp_msg) = $client1->stor($src_file, $tmpfile);
      $self->assert_transfer_ok($resp_code, $resp_msg);

      ($resp_code, $resp_msg) = $client2->retr($tmpfile, $dst_file);
      $self->assert_transfer_ok($resp_code, $resp_msg);

      $client1->quit();
      $client2->quit();

      $self->assert(-f $dst_file,
        test_msg("File $dst_file does not exist as expected"));
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
    test_append_logfile($log_file, $ex);
    unlink($log_file);

    die($ex);
  }

  unlink($log_file);
}

1;
