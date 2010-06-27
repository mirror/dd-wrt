package ProFTPD::Tests::Signals::HUP;

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
  hup_daemon_ok => {
    order => ++$order,
    test_class => [qw(bug)],
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

sub server_restart {
  my $pid_file = shift;

  my $pid;
  if (open(my $fh, "< $pid_file")) {
    $pid = <$fh>;
    chomp($pid);
    close($fh);

  } else {
    croak("Can't read $pid_file: $!");
  }

  my $cmd = "kill -HUP $pid";

  if ($ENV{TEST_VERBOSE}) {
    print STDERR "Restarting server: $cmd\n";
  }

  my @output = `$cmd`;
}

sub hup_daemon_ok {
  my $self = shift;
  my $tmpdir = $self->{tmpdir};

  my $config_file = "$tmpdir/signals.conf";
  my $pid_file = File::Spec->rel2abs("$tmpdir/signals.pid");
  my $scoreboard_file = File::Spec->rel2abs("$tmpdir/signals.scoreboard");

  my $log_file = File::Spec->rel2abs('tests.log');

  my $config = {
    PidFile => $pid_file,
    ScoreboardFile => $scoreboard_file,
    SystemLog => $log_file,

    ServerIdent => 'on foo',

    IfModules => {
      'mod_delay.c' => {
        DelayEngine => 'off',
      },
    },
  };

  my ($port, $config_user, $config_group) = config_write($config_file, $config);

  # Start server
  server_start($config_file); 

  my $client = ProFTPD::TestSuite::FTP->new('127.0.0.1', $port);

  my ($resp_code, $resp_msg);

  $resp_code = $client->response_code();
  $resp_msg = $client->response_msg();

  my $expected;
    
  $expected = 220;
  $self->assert($expected == $resp_code,
    test_msg("Expected $expected, got $resp_code"));

  $expected = "foo";
  $self->assert($expected eq $resp_msg,
    test_msg("Expected '$expected', got '$resp_msg'"));

  # Now change the config a little, and send the HUP signal
  $config->{ServerIdent} = 'on bar';
  ($port, $config_user, $config_group) = config_write($config_file, $config);
  server_restart($pid_file);

  $client = ProFTPD::TestSuite::FTP->new('127.0.0.1', $port);

  $resp_code = $client->response_code();
  $resp_msg = $client->response_msg();

  $expected = 220;
  $self->assert($expected == $resp_code,
    test_msg("Expected $expected, got $resp_code"));

  $expected = "bar";
  $self->assert($expected eq $resp_msg,
    test_msg("Expected '$expected', got '$resp_msg'"));

  server_stop($pid_file);
  unlink($log_file);
}

1;
