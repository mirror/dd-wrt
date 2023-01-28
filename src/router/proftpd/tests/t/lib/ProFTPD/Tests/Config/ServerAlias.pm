package ProFTPD::Tests::Config::ServerAlias;

use lib qw(t/lib);
use base qw(ProFTPD::TestSuite::Child);
use strict;

use File::Spec;
use IO::Handle;
use IO::Socket::INET;
use Sys::Hostname;

use ProFTPD::TestSuite::FTP;
use ProFTPD::TestSuite::Utils qw(:auth :config :running :test :testsuite);

$| = 1;

my $order = 0;

my $TESTS = {
  serveralias_same_ports_issue1369 => {
    test_class => [qw(bug forking)],
  },

  # Wildcard, with first-match-wins
};

sub new {
  return shift()->SUPER::new(@_);
}

sub list_tests {
  return testsuite_get_runnable_tests($TESTS);
}

sub serveralias_same_ports_issue1369 {
  my $self = shift;
  my $tmpdir = $self->{tmpdir};
  my $setup = test_setup($tmpdir, 'config');

  my $config = {
    PidFile => $setup->{pid_file},
    ScoreboardFile => $setup->{scoreboard_file},
    SystemLog => $setup->{log_file},
    TraceLog => $setup->{log_file},
    Trace => 'binding:30',

    AuthUserFile => $setup->{auth_user_file},
    AuthGroupFile => $setup->{auth_group_file},
    AuthOrder => 'mod_auth_file.c',

    Port => '0',
    ServerName => 'zero.castaglia.org',
    SocketBindTight => 'on',

    IfModules => {
      'mod_delay.c' => {
        DelayEngine => 'off',
      },
    },
  };

  my ($port, $config_user, $config_group) = config_write($setup->{config_file},
    $config);

  if (open(my $fh, ">> $setup->{config_file}")) {
    print $fh <<EOC;
<VirtualHost 127.0.0.1>
  Port $port
  ServerAlias first.castaglia.org
  ServerName first.castaglia.org

  AuthUserFile $setup->{auth_user_file}
  AuthGroupFile $setup->{auth_group_file}
  AuthOrder mod_auth_file.c
  TransferLog none
  WtmpLog off
</VirtualHost>

<VirtualHost 127.0.0.1>
  Port $port
  ServerAlias second.castaglia.org
  ServerName second.castaglia.org

  AuthUserFile $setup->{auth_user_file}
  AuthGroupFile $setup->{auth_group_file}
  AuthOrder mod_auth_file.c
  TransferLog none
  WtmpLog off
</VirtualHost>

<VirtualHost 127.0.0.1>
  Port $port
  ServerAlias *.castaglia.org
  ServerName match.castaglia.org

  AuthUserFile $setup->{auth_user_file}
  AuthGroupFile $setup->{auth_group_file}
  AuthOrder mod_auth_file.c
  TransferLog none
  WtmpLog off
</VirtualHost>

<VirtualHost 127.0.0.1>
  Port $port
  ServerAlias *
  ServerName wildcard

  AuthUserFile $setup->{auth_user_file}
  AuthGroupFile $setup->{auth_group_file}
  AuthOrder mod_auth_file.c
  TransferLog none
  WtmpLog off
</VirtualHost>
EOC
    unless (close($fh)) {
      die("Can't write $setup->{config_file}: $!");
    }

  } else {
    die("Can't open $setup->{config_file}: $!");
  }

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
      # first.castaglia.org (default)
      my $client = ProFTPD::TestSuite::FTP->new('127.0.0.1', $port, 0);
      my $resp_code = $client->response_code();
      my $resp_msg = $client->response_msg();
      $client->login($setup->{user}, $setup->{passwd});
      $client->quit();

      my $expected = 'first.castaglia.org';
      $self->assert(qr/$expected/, $resp_msg,
        test_msg("Expected '$expected', got '$resp_msg'"));

      # second.castaglia.org (explicit HOST)
      $client = ProFTPD::TestSuite::FTP->new('127.0.0.1', $port, 0);
      $client->host('second.castaglia.org');
      $resp_code = $client->response_code();
      $resp_msg = $client->response_msg();
      $client->login($setup->{user}, $setup->{passwd});
      $client->quit();

      $expected = 'second.castaglia.org';
      $self->assert(qr/$expected/, $resp_msg,
        test_msg("Expected '$expected', got '$resp_msg'"));

      # match.castaglia.org (matched HOST)
      $client = ProFTPD::TestSuite::FTP->new('127.0.0.1', $port, 0);
      $client->host('third.castaglia.org');
      $resp_code = $client->response_code();
      $resp_msg = $client->response_msg();
      $client->login($setup->{user}, $setup->{passwd});
      $client->quit();

      $expected = 'match.castaglia.org';
      $self->assert(qr/$expected/, $resp_msg,
        test_msg("Expected '$expected', got '$resp_msg'"));

      # wildcard (catchall)
      $client = ProFTPD::TestSuite::FTP->new('127.0.0.1', $port, 0);
      $client->host('example.com');
      $resp_code = $client->response_code();
      $resp_msg = $client->response_msg();
      $client->login($setup->{user}, $setup->{passwd});
      $client->quit();

      $expected = 'wildcard';
      $self->assert(qr/$expected/, $resp_msg,
        test_msg("Expected '$expected', got '$resp_msg'"));
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
