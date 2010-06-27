package ProFTPD::Tests::Modules::mod_shaper;

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
  shaper_sighup => {
    order => ++$order,
    test_class => [qw(forking)],
  },

  shaper_queue_dos => {
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

sub shaper_restart {
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

sub shaper_sighup {
  my $self = shift;
  my $tmpdir = $self->{tmpdir};

  my $config_file = "$tmpdir/shaper.conf";
  my $pid_file = File::Spec->rel2abs("$tmpdir/shaper.pid");
  my $scoreboard_file = File::Spec->rel2abs("$tmpdir/shaper.scoreboard");

  my $log_file = File::Spec->rel2abs('tests.log');

  my $shaper_tab = File::Spec->rel2abs("$tmpdir/shaper.tab");

  my $auth_user_file = File::Spec->rel2abs("$tmpdir/shaper.passwd");
  my $auth_group_file = File::Spec->rel2abs("$tmpdir/shaper.group");

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

  my $config = {
    PidFile => $pid_file,
    ScoreboardFile => $scoreboard_file,
    SystemLog => $log_file,

    AuthUserFile => $auth_user_file,
    AuthGroupFile => $auth_group_file,

    IfModules => {
      'mod_shaper.c' => {
        ShaperEngine => 'on',
        ShaperLog => $log_file,
        ShaperTable => $shaper_tab,
        ShaperAll => 'downrate 1500 uprate 1500',
      },

      'mod_delay.c' => {
        DelayEngine => 'off',
      },
    },
  };

  my ($port, $config_user, $config_group) = config_write($config_file, $config);

  # First, start the server.
  server_start($config_file);

  # Give it a second to start up, then send the SIGHUP signal
  sleep(2);
  shaper_restart($pid_file);

  # Finally, stop the server
  server_stop($pid_file);
 
  unlink($log_file);
}

sub shaper_queue_dos {
  my $self = shift;
  my $tmpdir = $self->{tmpdir};

  my $config_file = "$tmpdir/shaper.conf";
  my $pid_file = File::Spec->rel2abs("$tmpdir/shaper.pid");
  my $scoreboard_file = File::Spec->rel2abs("$tmpdir/shaper.scoreboard");

  my $log_file = File::Spec->rel2abs('tests.log');

  my $shaper_tab = File::Spec->rel2abs("$tmpdir/shaper.tab");

  my $auth_user_file = File::Spec->rel2abs("$tmpdir/shaper.passwd");
  my $auth_group_file = File::Spec->rel2abs("$tmpdir/shaper.group");

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

  my $test_count = 3000;
  my $test_timeout = 180;

  my $config = {
    PidFile => $pid_file,
    ScoreboardFile => $scoreboard_file,
    SystemLog => $log_file,

    AuthUserFile => $auth_user_file,
    AuthGroupFile => $auth_group_file,
    TimeoutIdle => $test_timeout + 10,

    IfModules => {
      'mod_shaper.c' => {
        ShaperEngine => 'on',
        ShaperLog => $log_file,
        ShaperTable => $shaper_tab,
        ShaperAll => 'downrate 1500 uprate 1500',
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
      # XXX Test how many connections it takes to fill up the msg queue used
      # by mod_shaper.

      my $clients = [];
      for (my $i = 0; $i < $test_count; $i++) {
        my $client = ProFTPD::TestSuite::FTP->new('127.0.0.1', $port);
        $client->login($user, $passwd);
        push(@$clients, $client);
      }

      foreach my $client (@$clients) {
        $client->quit();
      }

      $clients = undef;
    };

    if ($@) {
      $ex = $@;
    }

    $wfh->print("done\n");
    $wfh->flush();

  } else {
    eval { server_wait($config_file, $rfh, $test_timeout) };
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
