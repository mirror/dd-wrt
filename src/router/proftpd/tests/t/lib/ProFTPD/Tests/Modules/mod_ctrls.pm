package ProFTPD::Tests::Modules::mod_ctrls;

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
  ctrls_lsctrl_ok => {
    order => ++$order,
    test_class => [qw(forking)],
  },

  ctrls_lsctrl_system_user_ok => {
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

sub ftpdctl {
  my $sock_file = shift;
  my $ctrl_cmd = shift;

  my $ftpdctl_bin;
  if ($ENV{PROFTPD_TEST_PATH}) {
    $ftpdctl_bin = "$ENV{PROFTPD_TEST_PATH}/ftpdctl";

  } else {
    $ftpdctl_bin = '../ftpdctl';
  }

  my $cmd = "$ftpdctl_bin -s $sock_file $ctrl_cmd";

  if ($ENV{TEST_VERBOSE}) {
    print STDERR "Executing ftpdctl: $cmd\n";
  }

  my @lines = `$cmd`;
  return \@lines;
}

sub ctrls_lsctrl_ok {
  my $self = shift;
  my $tmpdir = $self->{tmpdir};

  my $config_file = "$tmpdir/ctrls.conf";
  my $pid_file = File::Spec->rel2abs("$tmpdir/ctrls.pid");
  my $scoreboard_file = File::Spec->rel2abs("$tmpdir/ctrls.scoreboard");

  my $log_file = File::Spec->rel2abs('tests.log');

  my $ctrls_sock = File::Spec->rel2abs("$tmpdir/ctrls.sock");

  my ($user, $group) = config_get_identity();

  my $config = {
    PidFile => $pid_file,
    ScoreboardFile => $scoreboard_file,
    SystemLog => $log_file,

    IfModules => {
      'mod_ctrls.c' => {
        ControlsEngine => 'on',
        ControlsLog => $log_file,
        ControlsSocket => $ctrls_sock,
        ControlsACLs => "all allow user root,$user",
        ControlsSocketACL => "allow user root,$user",
      },

      'mod_delay.c' => {
        DelayEngine => 'off',
      },
    },
  };

  my ($port, $config_user, $config_group) = config_write($config_file, $config);

  my $ex;

  # Start server
  server_start($config_file);

  sleep(1);

  eval {
    my $lines = ftpdctl($ctrls_sock, 'lsctrl');
    $lines = [grep { /mod_ctrls\.c/ } @$lines];

    my $expected = 4;

    my $matches = scalar(@$lines);
    $self->assert($expected == $matches,
      test_msg("Expected $expected, got $matches"));

    my $actions = '';
    foreach my $line (@$lines) {
      if ($line =~ /^ftpdctl: (\S+) \S+$/) {
        $actions .= "$1 ";
      }
    }

    $expected = 'help insctrl lsctrl rmctrl ';
    $self->assert($expected eq $actions,
      test_msg("Expected '$expected', got '$actions'"));
  }; 

  if ($@) {
    $ex = $@;
  }

  server_stop($pid_file);

  if ($ex) {
    die($ex);
  }

  unlink($log_file);
}

sub ctrls_lsctrl_system_user_ok {
  my $self = shift;
  my $tmpdir = $self->{tmpdir};

  my $config_file = "$tmpdir/ctrls.conf";
  my $pid_file = File::Spec->rel2abs("$tmpdir/ctrls.pid");
  my $scoreboard_file = File::Spec->rel2abs("$tmpdir/ctrls.scoreboard");

  my $log_file = File::Spec->rel2abs('tests.log');

  my $auth_user_file = File::Spec->rel2abs("$tmpdir/ctrls.passwd");
  my $auth_group_file = File::Spec->rel2abs("$tmpdir/ctrls.group");

  my $user = 'proftpd';
  my $passwd = 'test';
  my $home_dir = File::Spec->rel2abs($tmpdir);
  my $uid = 500;
  my $gid = 500;

  auth_user_write($auth_user_file, $user, $passwd, $uid, $gid, $home_dir,
    '/bin/bash');
  auth_group_write($auth_group_file, 'ftpd', $gid, $user);

  my $ctrls_sock = File::Spec->rel2abs("$tmpdir/ctrls.sock");

  my ($sys_user, $sys_group) = config_get_identity();

  my $config = {
    PidFile => $pid_file,
    ScoreboardFile => $scoreboard_file,
    SystemLog => $log_file,
    TraceLog => $log_file,
    Trace => 'DEFAULT:10',

    AuthUserFile => $auth_user_file,
    AuthGroupFile => $auth_group_file,
    AuthOrder => 'mod_auth_file.c',

    IfModules => {
      'mod_ctrls.c' => {
        ControlsEngine => 'on',
        ControlsLog => $log_file,
        ControlsSocket => $ctrls_sock,
        ControlsACLs => "all allow user root,$sys_user",
        ControlsSocketACL => "allow user root,$sys_user",
      },

      'mod_delay.c' => {
        DelayEngine => 'off',
      },
    },
  };

  my ($port, $config_user, $config_group) = config_write($config_file, $config);

  my $ex;

  # Start server
  server_start($config_file);

  sleep(1);

  eval {
    my $lines = ftpdctl($ctrls_sock, 'lsctrl');
    $lines = [grep { /mod_ctrls\.c/ } @$lines];

    my $expected = 4;

    my $matches = scalar(@$lines);
    $self->assert($expected == $matches,
      test_msg("Expected $expected, got $matches"));

    my $actions = '';
    foreach my $line (@$lines) {
      if ($line =~ /^ftpdctl: (\S+) \S+$/) {
        $actions .= "$1 ";
      }
    }

    $expected = 'help insctrl lsctrl rmctrl ';
    $self->assert($expected eq $actions,
      test_msg("Expected '$expected', got '$actions'"));
  }; 

  if ($@) {
    $ex = $@;
  }

  server_stop($pid_file);

  if ($ex) {
    die($ex);
  }

  unlink($log_file);
}

1;
