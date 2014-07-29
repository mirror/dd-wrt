package ProFTPD::Tests::Contrib::ftpasswd;

use lib qw(t/lib);
use base qw(ProFTPD::TestSuite::Child);
use strict;

use File::Path qw(mkpath rmtree);
use File::Spec;
use IO::Handle;

use ProFTPD::TestSuite::FTP;
use ProFTPD::TestSuite::Utils qw(:auth :config :running :test :testsuite);

$| = 1;

my $order = 0;

my $TESTS = {
  ftpasswd_append_user_bug3867 => {
    order => ++$order,
    test_class => [qw(bug forking)],
  },

  ftpasswd_lock_unlock_bug3994 => {
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

sub get_ftpasswd_bin {
  my $ftpasswd_bin;

  if ($ENV{PROFTPD_TEST_PATH}) {
    $ftpasswd_bin = File::Spec->rel2abs("$ENV{PROFTPD_TEST_PATH}/../contrib/ftpasswd");

  } else {
    $ftpasswd_bin = File::Spec->rel2abs('../contrib/ftpasswd');
  }

  return $ftpasswd_bin;
}

sub ftpasswd_append_user_bug3867 {
  my $self = shift;
  my $tmpdir = $self->{tmpdir};

  my $log_file = test_get_logfile();

  my $passwd_file = File::Spec->rel2abs("$tmpdir/ftpd.passwd");

  my $user1 = 'proftpd1';
  my $passwd1 = 'test1';
  my $home_dir1 = File::Spec->rel2abs($tmpdir);
  my $uid1 = 500;
  my $gid1 = 500;

=pod
$ ftpasswd --passwd --name=test@mail.com --uid=3000
--gid=3000 --home=/home/test --shell=/bin/false
--file=/home/astocker/proftpd.passwd
ftpasswd: using alternate file: /home/astocker/proftpd.passwd
ftpasswd: creating passwd entry for user test@mail.com

Password:
Re-type password:

ftpasswd: entry created
$ ls -l /home/astocker/proftpd.passwd
-r--r--r-- 1 astocker gmu 80 Jan 11 17:26 /home/astocker/proftpd.passwd
=cut

  my $ftpasswd = get_ftpasswd_bin();
  my $cmd = "$ftpasswd --passwd --stdin --file=$passwd_file --name=$user1 --uid=$uid1 --gid=$gid1 --home=$home_dir1 --shell=/bin/false >>$log_file 2>&1";

  if ($ENV{TEST_VERBOSE}) {
    print STDERR "Executing ftpasswd: $cmd\n";
  }

  eval { 
    if (open(my $cmdh, "| $cmd")) {
      print $cmdh "$passwd1\n";
      unless (close($cmdh)) {
        die("ftpasswd command failed when adding passwd entry for user '$user1'");
      }

      # Make sure the permissions on the passwd file are as expected
      my $file_mode = sprintf("%lo", (stat($passwd_file))[2] & 07777);
      my $expected = '440';
      $self->assert($expected eq $file_mode,
        test_msg("Expected perms '$expected', got '$file_mode'"));

    } else {
      die("Can't execute '$cmd': $!");
    }
  };
  if ($@) {
    my $ex = $@;
    test_append_logfile($log_file, $ex);
    unlink($log_file);

    die($ex);
  }

  my $user2 = 'proftpd2';
  my $passwd2 = 'test2';
  my $home_dir2 = File::Spec->rel2abs($tmpdir);
  my $uid2 = 501;
  my $gid2 = 501;

=pod
$ ftpasswd --passwd --name=test2@mail.com --uid=3000 
--gid=3000 --home=/home/test --shell=/bin/false 
--file=/home/astocker/proftpd.passwd
ftpasswd: using alternate file: /home/astocker/proftpd.passwd
ftpasswd: creating passwd entry for user test2@mail.com

Password:
Re-type password:

ftpasswd: entry created
ftpasswd: unable to open /home/astocker/proftpd.passwd: Permission denied
=cut

  $cmd = "$ftpasswd --passwd --stdin --file=$passwd_file --name=$user2 --uid=$uid2 --gid=$gid2 --home=$home_dir2 --shell=/bin/false >>$log_file 2>&1";

  if ($ENV{TEST_VERBOSE}) {
    print STDERR "Executing ftpasswd: $cmd\n";
  }

  eval {
    if (open(my $cmdh, "| $cmd")) {
      print $cmdh "$passwd2\n";
      unless (close($cmdh)) {
        die("ftpasswd command failed when adding passwd entry for user '$user2'");
      }

      # Make sure the permissions on the passwd file are as expected
      my $file_mode = sprintf("%lo", (stat($passwd_file))[2] & 07777);
      my $expected = '440';
      $self->assert($expected eq $file_mode,
        test_msg("Expected perms '$expected', got '$file_mode'"));

    } else {
      die("Can't execute '$cmd': $!");
    }
  };
  if ($@) {
    my $ex = $@;
    test_append_logfile($log_file, $ex);
    unlink($log_file);

    die($ex);
  }

  unlink($log_file);
}

sub ftpasswd_lock_unlock_bug3994 {
  my $self = shift;
  my $tmpdir = $self->{tmpdir};
  my $setup = test_setup($tmpdir, 'contrib');

  my $ftpasswd = get_ftpasswd_bin();

  my $config = {
    PidFile => $setup->{pid_file},
    ScoreboardFile => $setup->{scoreboard_file},
    SystemLog => $setup->{log_file},

    AuthUserFile => $setup->{auth_user_file},
    AuthGroupFile => $setup->{auth_group_file},

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

  $self->handle_sigchld();

  # Lock the account
  my $cmd = "$ftpasswd --passwd --file=$setup->{auth_user_file} --name=$setup->{user} --lock >> $setup->{log_file} 2>&1";

  if ($ENV{TEST_VERBOSE}) {
    print STDERR "Executing ftpasswd: $cmd\n";
  }

  `$cmd`;

  # Fork child
  defined(my $pid = fork()) or die("Can't fork: $!");
  if ($pid) {
    eval {
      # Try to login; should fail
      my $client = ProFTPD::TestSuite::FTP->new('127.0.0.1', $port, 1);
      eval { $client->login($setup->{user}, $setup->{passwd}) };
      unless ($@) {
        die("Login succeeded unexpectedly");
      }

      my $resp_code = $client->response_code();
      my $resp_msg = $client->response_msg();
      $client->quit();

      my $expected = 530;
      $self->assert($expected == $resp_code,
        test_msg("Expected response code $expected, got $resp_code"));

      $expected = 'Login incorrect.';
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

  # Unlock the account
  $cmd = "$ftpasswd --passwd --file=$setup->{auth_user_file} --name=$setup->{user} --unlock >> $setup->{log_file} 2>&1";

  if ($ENV{TEST_VERBOSE}) {
    print STDERR "Executing ftpasswd: $cmd\n";
  }

  `$cmd`;

  # Fork child
  defined($pid = fork()) or die("Can't fork: $!");
  if ($pid) {
    eval {
      my $client = ProFTPD::TestSuite::FTP->new('127.0.0.1', $port, 1);
      $client->login($setup->{user}, $setup->{passwd});
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
