package ProFTPD::Tests::Modules::mod_quotatab_file;

use lib qw(t/lib);
use base qw(Test::Unit::TestCase ProFTPD::TestSuite::Child);
use strict;

use File::Copy;
use File::Path qw(mkpath rmtree);
use File::Spec;
use IO::Handle;

use ProFTPD::TestSuite::FTP;
use ProFTPD::TestSuite::Utils qw(:auth :config :running :test :testsuite);

$| = 1;

my $order = 0;

my $TESTS = {
  quotatab_file_single_suppl_group => {
    order => ++$order,
    test_class => [qw(bug forking)],
  },

};

sub new {
  return shift()->SUPER::new(@_);
}

sub list_tests {
#  return testsuite_get_runnable_tests($TESTS);
  return qw(
    quotatab_file_single_suppl_group
  );
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

  # Make copies of the original tables into our scratch directory
  my ($src_file, $dst_file);

  $src_file = File::Spec->rel2abs('t/etc/modules/mod_quotatab_file/ftpquota.limittab');
  $dst_file = File::Spec->rel2abs("$self->{tmpdir}/ftpquota.limittab");

  unless (copy($src_file, $dst_file)) {
    die("Can't copy $src_file to $dst_file: $!");
  }

  $src_file = File::Spec->rel2abs('t/etc/modules/mod_quotatab_file/ftpquota.tallytab');
  $dst_file = File::Spec->rel2abs("$self->{tmpdir}/ftpquota.tallytab");

  unless (copy($src_file, $dst_file)) {
    die("Can't copy $src_file to $dst_file: $!");
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

sub get_tally {
  my $tally_file = shift;
  my $name = shift;
  my $quota_type = shift;

  my $ftpquota_bin;
  if ($ENV{PROFTPD_TEST_PATH}) {
    $ftpquota_bin = "$ENV{PROFTPD_TEST_PATH}/contrib/ftpquota";

  } else {
    $ftpquota_bin = '../contrib/ftpquota';
  }

  my $cmd = "perl $ftpquota_bin --show-records --table-path=$tally_file --type=tally";

  if ($ENV{TEST_VERBOSE}) {
    print STDERR "Executing perl: $cmd\n";
  }

  my @res = `$cmd`;

  my ($bytes_in_used, $bytes_out_used, $bytes_xfer_used, $files_in_used, $files_out_used, $files_xfer_used);

  foreach my $line (@res) {
    chomp($line);

    if ($line =~ /Uploaded bytes:\s+(\S+)$/) {
      $bytes_in_used = $1;

    } elsif ($line =~ /Downloaded bytes:\s+(\S+)$/) {
      $bytes_out_used = $1;

    } elsif ($line =~ /Transferred bytes:\s+(\S+)$/) {
      $bytes_xfer_used = $1;

    } elsif ($line =~ /Uploaded files:\s+(\S+)$/) {
      $files_in_used = $1;

    } elsif ($line =~ /Downloaded files:\s+(\S+)$/) {
      $files_out_used = $1;

    } elsif ($line =~ /Transferred files:\s+(\S+)$/) {
      $files_xfer_used = $1;
    }
  }

  return ($quota_type, $bytes_in_used, $bytes_out_used, $bytes_xfer_used, $files_in_used, $files_out_used, $files_xfer_used);
}

sub quotatab_file_single_suppl_group {
  my $self = shift;
  my $tmpdir = $self->{tmpdir};

  my $config_file = "$tmpdir/quotatab.conf";
  my $pid_file = File::Spec->rel2abs("$tmpdir/quotatab.pid");
  my $scoreboard_file = File::Spec->rel2abs("$tmpdir/quotatab.scoreboard");

  my $log_file = File::Spec->rel2abs('tests.log');

  my $auth_user_file = File::Spec->rel2abs("$tmpdir/quotatab.passwd");
  my $auth_group_file = File::Spec->rel2abs("$tmpdir/quotatab.group");

  my $user = 'proftpd';
  my $group = 'ftpd';
  my $passwd = 'test';
  my $home_dir = File::Spec->rel2abs($tmpdir);
  mkpath($home_dir);

  my $uid = 500;
  my $gid = 500;

  # Make sure that, if we're running as root, that the home directories has
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

auth_group_write($auth_group_file, 'test1', $gid+2, $user);

  # Make sure that the group for whom there is a limit is NOT the user's
  # primary group, but IS the user's only supplemental group.
  auth_group_write($auth_group_file, 'test', $gid+1, $user);

  my $limit_file = File::Spec->rel2abs("$tmpdir/ftpquota.limittab");
  my $tally_file = File::Spec->rel2abs("$tmpdir/ftpquota.tallytab");

  my $config = {
    PidFile => $pid_file,
    ScoreboardFile => $scoreboard_file,
    SystemLog => $log_file,

    AuthUserFile => $auth_user_file,
    AuthGroupFile => $auth_group_file,

    DefaultChdir => '~',

    IfModules => {
      'mod_delay.c' => {
        DelayEngine => 'off',
      },

      'mod_quotatab_file.c' => {
        QuotaEngine => 'on',
        QuotaLog => $log_file,
        QuotaLimitTable => "file:$limit_file",
        QuotaTallyTable => "file:$tally_file",
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

      $client->login($user, $passwd);

      my $conn = $client->stor_raw('test.txt');
      unless ($conn) {
        die("Failed to STOR test.txt: " . $client->response_code() . " " .
          $client->response_msg());
      }

      my $buf = "Hello, World\n";
      $conn->write($buf, length($buf));
      $conn->close();

      my $resp_code = $client->response_code();
      my $resp_msg = $client->response_msg();

      my $expected;

      $expected = 226;
      $self->assert($expected == $resp_code,
        test_msg("Expected $expected, got $resp_code"));

      $expected = "Transfer complete";
      $self->assert($expected eq $resp_msg,
        test_msg("Expected '$expected', got '$resp_msg'"));

      $client->quit();
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

  my ($quota_type, $bytes_in_used, $bytes_out_used, $bytes_xfer_used, $files_in_used, $files_out_used, $files_xfer_used) = get_tally($tally_file, 'test', 'group');

  my $expected;

  $expected = 'group';
  $self->assert($expected eq $quota_type,
    test_msg("Expected '$expected', got '$quota_type'"));

  $expected = '^(13.0+|13)$';
  $self->assert(qr/$expected/, $bytes_in_used,
    test_msg("Expected $expected, got $bytes_in_used"));

  $expected = '^(0.0+|0)$';
  $self->assert(qr/$expected/, $bytes_out_used,
    test_msg("Expected $expected, got $bytes_out_used"));

  $expected = '^(0.0+|0)$';
  $self->assert(qr/$expected/, $bytes_xfer_used,
    test_msg("Expected $expected, got $bytes_xfer_used"));

  $expected = 0;
  $self->assert($expected == $files_in_used,
    test_msg("Expected $expected, got $files_in_used"));

  $expected = 0;
  $self->assert($expected == $files_out_used,
    test_msg("Expected $expected, got $files_out_used"));

  $expected = 0;
  $self->assert($expected == $files_xfer_used,
    test_msg("Expected $expected, got $files_xfer_used"));

  if ($ex) {
    die($ex);
  }

  unlink($log_file);
}

1;
