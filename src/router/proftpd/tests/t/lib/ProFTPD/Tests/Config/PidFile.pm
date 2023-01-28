package ProFTPD::Tests::Config::PidFile;

use lib qw(t/lib);
use base qw(ProFTPD::TestSuite::Child);
use strict;

use File::Spec;
use IO::Handle;

use ProFTPD::TestSuite::FTP;
use ProFTPD::TestSuite::Utils qw(:auth :config :running :test :testsuite);

$| = 1;

my $order = 0;

my $TESTS = {
  pidfile_world_writable_issue1018 => {
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

sub pidfile_world_writable_issue1018 {
  my $self = shift;
  my $tmpdir = $self->{tmpdir};
  my $setup = test_setup($tmpdir, 'config');

  my $config = {
    PidFile => $setup->{pid_file},
    ScoreboardFile => $setup->{scoreboard_file},
    SystemLog => $setup->{log_file},

    AuthUserFile => $setup->{auth_user_file},
    AuthGroupFile => $setup->{auth_group_file},
    AuthOrder => 'mod_auth_file.c',

    Umask => '0000',

    IfModules => {
      'mod_delay.c' => {
        DelayEngine => 'off',
      },
    },
  };

  my ($port, $config_user, $config_group) = config_write($setup->{config_file},
    $config);

  server_start($setup->{config_file}, $setup->{pid_file});

  my $ex;

  eval {
    my $mode = (stat($setup->{pid_file}))[2];
    my $mode_text = sprintf("%04o", $mode & 07777);
    if ($ENV{TEST_VERBOSE}) {
      print STDERR "# permissions: $mode_text\n";
    }
    $self->assert(($mode & 002) == 0,
      test_msg("Mode $mode_text is unexpectedly world-writable"));
  };
  if ($@) {
    $ex = $@;
  }

  server_stop($setup->{pid_file});
  test_cleanup($setup->{log_file}, $ex);
}

1;
