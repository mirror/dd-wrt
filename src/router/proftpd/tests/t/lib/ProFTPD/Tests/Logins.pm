package ProFTPD::Tests::Logins;

use lib qw(t/lib);
use base qw(Test::Unit::TestCase ProFTPD::TestSuite::Child);
use strict;

use File::Path qw(mkpath rmtree);
use File::Spec;
use IO::Handle;

use ProFTPD::TestSuite::FTP;
use ProFTPD::TestSuite::Utils qw(:config :running :testsuite);

$| = 1;

my $order = 0;

my $TESTS = {
  login_plaintext_fails => {
    order => ++$order,
    test_class => [qw(forking)],
  },

  login_anonymous_ok => {
    order => ++$order,
    test_class => [qw(forking rootprivs)],
  },

  login_anonymous_fails => {
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
};

sub login_plaintext_fails {
  my $self = shift;
  my $tmpdir = $self->{tmpdir};

  my $config_file = "$tmpdir/login.conf";
  my $pid_file = File::Spec->rel2abs("$tmpdir/login.pid");
  my $scoreboard_file = File::Spec->rel2abs("$tmpdir/login.scoreboard");
  my $log_file = File::Spec->rel2abs("login.log");

  my $config = {
    PidFile => $pid_file,
    ScoreboardFile => $scoreboard_file,
    SystemLog => $log_file,

    IfModules => {
      'mod_delay.c' => {
        DelayEngine => 'off',
      },
    },
  };

  my ($port, $user, $group) = config_write($config_file, $config);

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

      # In parent process, login to server using a plaintext password which
      # should NOT work.
      eval { $client->login('daemon', '*') };
      unless ($@) {
        die("Logged in unexpectedly");
      }
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

sub login_anonymous_ok {
  my $self = shift;
  my $tmpdir = $self->{tmpdir};

  my $config_file = "$tmpdir/login.conf";
  my $pid_file = File::Spec->rel2abs("$tmpdir/login.pid");
  my $scoreboard_file = File::Spec->rel2abs("$tmpdir/login.scoreboard");
  my $log_file = File::Spec->rel2abs('login.log');

  my $anon_dir = File::Spec->rel2abs($tmpdir);

  my ($config_user, $config_group) = config_get_identity();

  my $config = {
    PidFile => $pid_file,
    ScoreboardFile => $scoreboard_file,
    SystemLog => $log_file,

    IfModules => {
      'mod_delay.c' => {
        DelayEngine => 'off',
      },
    },

    Anonymous => {
      $anon_dir => {
        User => $config_user,
        Group => $config_group,
        UserAlias => "anonymous $config_user",
        RequireValidShell => 'off',
      },
    },
  };

  my ($port, $user, $group) = config_write($config_file, $config);

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

      # In parent process, login anonymously to server using a plaintext
      # password which SHOULD work.
      $client->login('anonymous', 'ftp@nospam.org');
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

sub login_anonymous_fails {
  my $self = shift;
  my $tmpdir = $self->{tmpdir};

  my $config_file = "$tmpdir/login.conf";
  my $pid_file = File::Spec->rel2abs("$tmpdir/login.pid");
  my $scoreboard_file = File::Spec->rel2abs("$tmpdir/login.scoreboard");
  my $log_file = File::Spec->rel2abs('login.log');

  my $anon_dir = File::Spec->rel2abs($tmpdir);

  my ($user, $group) = config_get_identity();

  my $config = {
    PidFile => $pid_file,
    ScoreboardFile => $scoreboard_file,
    SystemLog => $log_file,

    IfModules => {
      'mod_delay.c' => {
        DelayEngine => 'off',
      },
    },

    Anonymous => {
      $anon_dir => {
        User => $user,
        Group => $group,
        UserAlias => "anonymous $user",
        RequireValidShell => 'off',
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

      my $perms = (stat($anon_dir))[2];
      chmod(0660, $anon_dir);

      eval { $client->login('anonymous', 'ftp@nospam.org') };
      unless ($@) {
        die("Unexpectedly logged in anonymously");
      }

      chmod($perms, $anon_dir);
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
