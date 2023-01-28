package ProFTPD::Tests::Commands::HELP;

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
  help_ok => {
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

sub help_ok {
  my $self = shift;
  my $tmpdir = $self->{tmpdir};
  my $setup = test_setup($tmpdir, 'cmds');

  my $config = {
    PidFile => $setup->{pid_file},
    ScoreboardFile => $setup->{scoreboard_file},
    SystemLog => $setup->{log_file},

    AuthUserFile => $setup->{auth_user_file},
    AuthGroupFile => $setup->{auth_group_file},
    AuthOrder => 'mod_auth_file.c',

    IfModules => {
      'mod_delay.c' => {
        DelayEngine => 'off',
      },
    },
  };

  my ($port, $config_user, $config_group) = config_write($setup->{config_file},
    $config);

  my $auth_helps = [
    ' NOOP    FEAT    OPTS    HOST    CLNT    AUTH*   CCC*    CONF*   ',
    ' ENC*    MIC*    PBSZ*   PROT*   TYPE    STRU    MODE    RETR    ',
  ];

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
      # Allow for server startup
      sleep(2);

      my $client = ProFTPD::TestSuite::FTP->new('127.0.0.1', $port);

      $client->help();
      my $resp_code = $client->response_code();
      my $resp_msgs = $client->response_msgs();

      my $expected = 214;
      $self->assert($expected == $resp_code,
        test_msg("Expected response code $expected, got $resp_code"));

      $expected = 9;
      my $nhelp = scalar(@$resp_msgs);
      $self->assert($expected == $nhelp,
        test_msg("Expected $expected lines, got $nhelp"));

      my $helps = [(
        'The following commands are recognized (* =>\'s unimplemented):',
        ' CWD     XCWD    CDUP    XCUP    SMNT*   QUIT    PORT    PASV    ',
        ' EPRT    EPSV    ALLO    RNFR    RNTO    DELE    MDTM    RMD     ',
        ' XRMD    MKD     XMKD    PWD     XPWD    SIZE    SYST    HELP    ',
        @$auth_helps,
        ' STOR    STOU    APPE    REST    ABOR    RANG    USER    PASS    ',
        ' ACCT*   REIN*   LIST    NLST    STAT    SITE    MLSD    MLST    ',
        'Direct comments to root@127.0.0.1',
      )];

      for (my $i = 0; $i < $nhelp; $i++) {
        $expected = $helps->[$i];
        $self->assert($expected eq $resp_msgs->[$i],
          test_msg("Expected '$expected', got '$resp_msgs->[$i]'"));
      }

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
