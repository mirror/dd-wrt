package ProFTPD::Tests::Commands::FEAT;

use lib qw(t/lib);
use base qw(ProFTPD::TestSuite::Child);
use strict;

use File::Spec;
use IO::Handle;

use ProFTPD::TestSuite::FTP;
use ProFTPD::TestSuite::Utils qw(:auth :config :features :running :test :testsuite);

$| = 1;

my $order = 0;

my $TESTS = {
  feat_ok => {
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

sub feat_ok {
  my $self = shift;
  my $tmpdir = $self->{tmpdir};
  my $setup = test_setup($tmpdir, 'cmds');

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

  # By default, we expect to see 12 lines in the FEAT response
  my $expected_nfeat = 12;

  my $have_nls = feature_have_feature_enabled('nls');
  if ($have_nls) {
    $expected_nfeat += 2;
  }

  my $have_site_misc = feature_have_module_compiled('mod_site_misc.c');
  if ($have_site_misc) {
    # For the following SITE commands added by mod_site_misc: MKDIR,
    #  RMDIR, UTIME, and SYMLINK.
    $expected_nfeat += 4;
  }

  my $have_copy = feature_have_module_compiled('mod_copy.c');
  if ($have_copy) {
    # For the following SITE commands added by mod_copy: COPY.
    $expected_nfeat += 1;
  }

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

  # Fork child
  $self->handle_sigchld();
  defined(my $pid = fork()) or die("Can't fork: $!");
  if ($pid) {
    eval {
      my $client = ProFTPD::TestSuite::FTP->new('127.0.0.1', $port);

      $client->feat();
      my $resp_code = $client->response_code();
      my $resp_msgs = $client->response_msgs();

      my $expected = 211;
      $self->assert($expected == $resp_code,
        test_msg("Expected response code $expected, got $resp_code"));

      my $nfeat = scalar(@$resp_msgs);
      $self->assert($expected_nfeat == $nfeat,
        test_msg("Expected $expected_nfeat features, got $nfeat"));

      my $feats = { 
        'Features:' => 1,
        ' EPRT' => 1,
        ' EPSV' => 1,
        ' HOST' => 1,
        ' MDTM' => 1,
        ' MFMT' => 1,
        ' TVFS' => 1,
        ' MFF modify;UNIX.group;UNIX.mode;' => 1,
        ' MLST modify*;perm*;size*;type*;unique*;UNIX.group*;UNIX.mode*;UNIX.owner*;' => 1,
        ' REST STREAM' => 1,
        ' SIZE' => 1,
        'End' => 1,
      };

      if ($have_nls) {
        $feats->{' UTF8'} = 1;

        # One of the following will appear in the FEAT list, depending
        # on the underlying platform.
        $feats->{' LANG en_US'} = 1;
        $feats->{' LANG en-US'} = 1;
        $feats->{' LANG en-US*'} = 1;
        $feats->{' LANG en_US.UTF-8'} = 1;
        $feats->{' LANG en-US.UTF-8'} = 1;
        $feats->{' LANG en-US.UTF-8*'} = 1;
      }

      if ($have_site_misc) {
        $feats->{' SITE MKDIR'} = 1;
        $feats->{' SITE RMDIR'} = 1;
        $feats->{' SITE SYMLINK'} = 1;
        $feats->{' SITE UTIME'} = 1;
      }

      if ($have_copy) {
        $feats->{' SITE COPY'} = 1;
      }

      for (my $i = 0; $i < $nfeat; $i++) {
        $self->assert(defined($feats->{$resp_msgs->[$i]}), ,
          test_msg("Unexpected FEAT '$resp_msgs->[$i]'"));
      }
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
