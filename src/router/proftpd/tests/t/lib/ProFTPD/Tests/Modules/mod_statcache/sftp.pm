package ProFTPD::Tests::Modules::mod_statcache::sftp;

use lib qw(t/lib);
use base qw(ProFTPD::TestSuite::Child);
use strict;

use Cwd;
use File::Path qw(mkpath);
use File::Spec;
use IO::Handle;
use POSIX qw(:fcntl_h);

use ProFTPD::TestSuite::FTP;
use ProFTPD::TestSuite::Utils qw(:auth :config :running :test :testsuite);

$| = 1;

my $order = 0;

my $TESTS = {
  statcache_sftp_stat_file => {
    order => ++$order,
    test_class => [qw(forking sftp)],
  },

  statcache_sftp_stat_dir => {
    order => ++$order,
    test_class => [qw(forking sftp)],
  },

  statcache_sftp_upload_file => {
    order => ++$order,
    test_class => [qw(forking sftp)],
  },

};

sub new {
  return shift()->SUPER::new(@_);
}

sub list_tests {
  # Check for the required Perl modules:
  #
  #  Net-SSH2
  #  Net-SSH2-SFTP

  my $required = [qw(
    Net::SSH2
    Net::SSH2::SFTP
  )];

  foreach my $req (@$required) {
    eval "use $req";
    if ($@) {
      print STDERR "\nWARNING:\n + Module '$req' not found, skipping all tests\n";

      if ($ENV{TEST_VERBOSE}) {
        print STDERR "Unable to load $req: $@\n";
      }

      return qw(testsuite_empty_test);
    }
  }

  return testsuite_get_runnable_tests($TESTS);
}

sub set_up {
  my $self = shift;
  $self->SUPER::set_up(@_);

  # Make sure that mod_sftp does not complain about permissions on the hostkey
  # files.

  my $rsa_host_key = File::Spec->rel2abs('t/etc/modules/mod_statcache/ssh_host_rsa_key');

  unless (chmod(0400, $rsa_host_key)) {
    die("Can't set perms on $rsa_host_key: $!");
  }
}

sub statcache_sftp_stat_file {
  my $self = shift;
  my $tmpdir = $self->{tmpdir};
  my $setup = test_setup($tmpdir, 'statcache');

  my $test_file = File::Spec->rel2abs("$setup->{home_dir}/test.txt");
  if (open(my $fh, "> $test_file")) {
    print $fh "Hello, World!\n";
    unless (close($fh)) {
      die("Can't write $test_file: $!");
    }

  } else {
    die("Can't open $test_file: $!");
  }

  my $test_filemode = (stat($test_file))[2];
  if ($ENV{TEST_VERBOSE}) {
    print STDERR "# $test_file mode: $test_filemode\n";
  }

  my $test_filesize = (stat($test_file))[7];

  my $statcache_tab = File::Spec->rel2abs("$tmpdir/statcache.tab");

  my $rsa_host_key = File::Spec->rel2abs('t/etc/modules/mod_statcache/ssh_host_rsa_key');

  my $config = {
    PidFile => $setup->{pid_file},
    ScoreboardFile => $setup->{scoreboard_file},
    SystemLog => $setup->{log_file},
    TraceLog => $setup->{log_file},
    Trace => 'fsio:10 sftp:20 statcache:20',

    AuthUserFile => $setup->{auth_user_file},
    AuthGroupFile => $setup->{auth_group_file},
    AuthOrder => 'mod_auth_file.c',

    IfModules => {
      'mod_delay.c' => {
        DelayEngine => 'off',
      },

      'mod_sftp.c' => {
        SFTPEngine => 'on',
        SFTPLog => $setup->{log_file},
        SFTPHostKey => $rsa_host_key,
      },

      'mod_statcache.c' => {
        StatCacheEngine => 'on',
        StatCacheTable => $statcache_tab,
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

  require Net::SSH2;

  my $ex;

  # Fork child
  $self->handle_sigchld();
  defined(my $pid = fork()) or die("Can't fork: $!");
  if ($pid) {
    eval {
      my $ssh2 = Net::SSH2->new();

      sleep(1);

      unless ($ssh2->connect('127.0.0.1', $port)) {
        my ($err_code, $err_name, $err_str) = $ssh2->error();
        die("Can't connect to SSH2 server: [$err_name] ($err_code) $err_str");
      }

      unless ($ssh2->auth_password($setup->{user}, $setup->{passwd})) {
        my ($err_code, $err_name, $err_str) = $ssh2->error();
        die("Can't login to SSH2 server: [$err_name] ($err_code) $err_str");
      }
 
      my $sftp = $ssh2->sftp();
      unless ($sftp) {
        my ($err_code, $err_name, $err_str) = $ssh2->error();
        die("Can't use SFTP on SSH2 server: [$err_name] ($err_code) $err_str");
      }

      my $path = 'test.txt';
      my $attrs = $sftp->stat($path, 1);
      unless ($attrs) {
        my ($err_code, $err_name) = $sftp->error();
        die("STAT $path failed: [$err_name] ($err_code)");
      }

      my $expected = $test_filesize;
      my $file_size = $attrs->{size};
      $self->assert($expected == $file_size,
        test_msg("Expected file size '$expected', got '$file_size'"));

      $expected = $<;
      my $file_uid = $attrs->{uid};
      $self->assert($expected == $file_uid,
        test_msg("Expected file UID '$expected', got '$file_uid'"));

      $expected = $(;
      my $file_gid = $attrs->{gid};
      $self->assert($expected == $file_gid,
        test_msg("Expected file GID '$expected', got '$file_gid'"));

      $expected = $test_filemode;
      my $file_mode = $attrs->{mode};
      $self->assert($expected == $file_mode,
        test_msg("Expected file mode '$expected', got '$file_mode'"));

      # Do the stat again; we'll check the logs to see if mod_statcache
      # did its job.
      $attrs = $sftp->stat($path, 1);
      unless ($attrs) {
        my ($err_code, $err_name) = $sftp->error();
        die("STAT $path failed: [$err_name] ($err_code)");
      }

      $expected = $test_filesize;
      $file_size = $attrs->{size};
      $self->assert($expected == $file_size,
        test_msg("Expected file size '$expected', got '$file_size'"));

      $expected = $<;
      $file_uid = $attrs->{uid};
      $self->assert($expected == $file_uid,
        test_msg("Expected file UID '$expected', got '$file_uid'"));

      $expected = $(;
      $file_gid = $attrs->{gid};
      $self->assert($expected == $file_gid,
        test_msg("Expected file GID '$expected', got '$file_gid'"));

      $expected = $test_filemode;
      $file_mode = $attrs->{mode};
      $self->assert($expected == $file_mode,
        test_msg("Expected file mode '$expected', got '$file_mode'"));

      $sftp = undef;
      $ssh2->disconnect();

      # Now connect again, do another stat, and see if we're still using
      # the cached entry.
      $ssh2 = Net::SSH2->new();
      unless ($ssh2->connect('127.0.0.1', $port)) {
        my ($err_code, $err_name, $err_str) = $ssh2->error();
        die("Can't connect to SSH2 server: [$err_name] ($err_code) $err_str");
      }

      unless ($ssh2->auth_password($setup->{user}, $setup->{passwd})) {
        my ($err_code, $err_name, $err_str) = $ssh2->error();
        die("Can't login to SSH2 server: [$err_name] ($err_code) $err_str");
      }

      $sftp = $ssh2->sftp();
      unless ($sftp) {
        my ($err_code, $err_name, $err_str) = $ssh2->error();
        die("Can't use SFTP on SSH2 server: [$err_name] ($err_code) $err_str");
      }

      $attrs = $sftp->stat($path, 1);
      unless ($attrs) {
        my ($err_code, $err_name) = $sftp->error();
        die("STAT $path failed: [$err_name] ($err_code)");
      }

      $expected = $test_filesize;
      $file_size = $attrs->{size};
      $self->assert($expected == $file_size,
        test_msg("Expected file size '$expected', got '$file_size'"));

      $expected = $<;
      $file_uid = $attrs->{uid};
      $self->assert($expected == $file_uid,
        test_msg("Expected file UID '$expected', got '$file_uid'"));

      $expected = $(;
      $file_gid = $attrs->{gid};
      $self->assert($expected == $file_gid,
        test_msg("Expected file GID '$expected', got '$file_gid'"));

      $expected = $test_filemode;
      $file_mode = $attrs->{mode};
      $self->assert($expected == $file_mode,
        test_msg("Expected file mode '$expected', got '$file_mode'"));

      $sftp = undef;
      $ssh2->disconnect();
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

  eval {
    if (open(my $fh, "< $setup->{log_file}")) {
      my $adding_entry = 0;
      my $cached_stat = 0;
      my $cached_lstat = 0;

      if ($^O eq 'darwin') {
        # MacOSX-specific hack
        $test_file = '/private' . $test_file;
      }

      while (my $line = <$fh>) {
        chomp($line);

        if ($ENV{TEST_VERBOSE}) {
          print STDERR "# line: $line\n";
        }

        if ($line =~ /<statcache:9>/) {
          if ($line =~ /adding entry.*?type file/) {
            $adding_entry++;
            next;
          }
        }

        if ($line =~ /<statcache:11>/) {
          if ($cached_stat == 0 &&
              $line =~ /using cached stat.*?path '$test_file'/) {
            $cached_stat++;
            next;
          }

          if ($cached_lstat == 0 &&
              $line =~ /using cached lstat.*?path '$test_file'/) {
            $cached_lstat++;
            next;
          }
        }

        if ($adding_entry >= 2 &&
            $cached_stat == 1 &&
            $cached_lstat == 1) {
          last;
        }
      }

      close($fh);

      $self->assert($adding_entry >= 2 &&
                    $cached_stat == 1 &&
                    $cached_lstat == 1,
        test_msg("Did not see expected 'statcache' TraceLog messages"));

    } else {
      die("Can't read $setup->{log_file}: $!");
    }
  };
  if ($@) {
    $ex = $@;
  }

  test_cleanup($setup->{log_file}, $ex);
}

sub statcache_sftp_stat_dir {
  my $self = shift;
  my $tmpdir = $self->{tmpdir};
  my $setup = test_setup($tmpdir, 'statcache');

  my $test_dir = File::Spec->rel2abs("$setup->{home_dir}/test.d");
  mkpath($test_dir);

  my $test_filemode = (stat($test_dir))[2];
  if ($ENV{TEST_VERBOSE}) {
    print STDERR "# $test_dir mode: $test_filemode\n";
  }

  my $statcache_tab = File::Spec->rel2abs("$tmpdir/statcache.tab");

  my $rsa_host_key = File::Spec->rel2abs('t/etc/modules/mod_statcache/ssh_host_rsa_key');

  my $config = {
    PidFile => $setup->{pid_file},
    ScoreboardFile => $setup->{scoreboard_file},
    SystemLog => $setup->{log_file},
    TraceLog => $setup->{log_file},
    Trace => 'fsio:10 sftp:20 statcache:20',

    AuthUserFile => $setup->{auth_user_file},
    AuthGroupFile => $setup->{auth_group_file},
    AuthOrder => 'mod_auth_file.c',

    IfModules => {
      'mod_delay.c' => {
        DelayEngine => 'off',
      },

      'mod_sftp.c' => {
        SFTPEngine => 'on',
        SFTPLog => $setup->{log_file},
        SFTPHostKey => $rsa_host_key,
      },

      'mod_statcache.c' => {
        StatCacheEngine => 'on',
        StatCacheTable => $statcache_tab,
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

  require Net::SSH2;

  my $ex;

  # Fork child
  $self->handle_sigchld();
  defined(my $pid = fork()) or die("Can't fork: $!");
  if ($pid) {
    eval {
      my $ssh2 = Net::SSH2->new();

      sleep(1);

      unless ($ssh2->connect('127.0.0.1', $port)) {
        my ($err_code, $err_name, $err_str) = $ssh2->error();
        die("Can't connect to SSH2 server: [$err_name] ($err_code) $err_str");
      }

      unless ($ssh2->auth_password($setup->{user}, $setup->{passwd})) {
        my ($err_code, $err_name, $err_str) = $ssh2->error();
        die("Can't login to SSH2 server: [$err_name] ($err_code) $err_str");
      }
 
      my $sftp = $ssh2->sftp();
      unless ($sftp) {
        my ($err_code, $err_name, $err_str) = $ssh2->error();
        die("Can't use SFTP on SSH2 server: [$err_name] ($err_code) $err_str");
      }

      my $path = 'test.d';
      my $attrs = $sftp->stat($path, 1);
      unless ($attrs) {
        my ($err_code, $err_name) = $sftp->error();
        die("STAT $path failed: [$err_name] ($err_code)");
      }

      my $expected = $<;
      my $file_uid = $attrs->{uid};
      $self->assert($expected == $file_uid,
        test_msg("Expected file UID '$expected', got '$file_uid'"));

      $expected = $(;
      my $file_gid = $attrs->{gid};
      $self->assert($expected == $file_gid,
        test_msg("Expected file GID '$expected', got '$file_gid'"));

      $expected = $test_filemode;
      my $file_mode = $attrs->{mode};
      $self->assert($expected == $file_mode,
        test_msg("Expected file mode '$expected', got '$file_mode'"));

      # Do the stat again; we'll check the logs to see if mod_statcache
      # did its job.
      $attrs = $sftp->stat($path, 1);
      unless ($attrs) {
        my ($err_code, $err_name) = $sftp->error();
        die("STAT $path failed: [$err_name] ($err_code)");
      }

      $expected = $<;
      $file_uid = $attrs->{uid};
      $self->assert($expected == $file_uid,
        test_msg("Expected file UID '$expected', got '$file_uid'"));

      $expected = $(;
      $file_gid = $attrs->{gid};
      $self->assert($expected == $file_gid,
        test_msg("Expected file GID '$expected', got '$file_gid'"));

      $expected = $test_filemode;
      $file_mode = $attrs->{mode};
      $self->assert($expected == $file_mode,
        test_msg("Expected file mode '$expected', got '$file_mode'"));

      $sftp = undef;
      $ssh2->disconnect();

      # Now connect again, do another stat, and see if we're still using
      # the cached entry.
      $ssh2 = Net::SSH2->new();
      unless ($ssh2->connect('127.0.0.1', $port)) {
        my ($err_code, $err_name, $err_str) = $ssh2->error();
        die("Can't connect to SSH2 server: [$err_name] ($err_code) $err_str");
      }

      unless ($ssh2->auth_password($setup->{user}, $setup->{passwd})) {
        my ($err_code, $err_name, $err_str) = $ssh2->error();
        die("Can't login to SSH2 server: [$err_name] ($err_code) $err_str");
      }

      $sftp = $ssh2->sftp();
      unless ($sftp) {
        my ($err_code, $err_name, $err_str) = $ssh2->error();
        die("Can't use SFTP on SSH2 server: [$err_name] ($err_code) $err_str");
      }

      $attrs = $sftp->stat($path, 1);
      unless ($attrs) {
        my ($err_code, $err_name) = $sftp->error();
        die("STAT $path failed: [$err_name] ($err_code)");
      }

      $expected = $<;
      $file_uid = $attrs->{uid};
      $self->assert($expected == $file_uid,
        test_msg("Expected file UID '$expected', got '$file_uid'"));

      $expected = $(;
      $file_gid = $attrs->{gid};
      $self->assert($expected == $file_gid,
        test_msg("Expected file GID '$expected', got '$file_gid'"));

      $expected = $test_filemode;
      $file_mode = $attrs->{mode};
      $self->assert($expected == $file_mode,
        test_msg("Expected file mode '$expected', got '$file_mode'"));

      $sftp = undef;
      $ssh2->disconnect();
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

  eval {
    if (open(my $fh, "< $setup->{log_file}")) {
      my $adding_entry = 0;
      my $cached_stat = 0;
      my $cached_lstat = 0;

      if ($^O eq 'darwin') {
        # MacOSX-specific hack
        $test_dir = '/private' . $test_dir;
      }

      while (my $line = <$fh>) {
        chomp($line);

        if ($ENV{TEST_VERBOSE}) {
          print STDERR "# line: $line\n";
        }

        if ($line =~ /<statcache:9>/) {
          if ($line =~ /adding entry.*?type dir/) {
            $adding_entry++;
            next;
          }
        }

        if ($line =~ /<statcache:11>/) {
          if ($cached_stat == 0 &&
              $line =~ /using cached stat.*?path '$test_dir'/) {
            $cached_stat++;
            next;
          }

          if ($cached_lstat == 0 &&
              $line =~ /using cached lstat.*?path '$test_dir'/) {
            $cached_lstat++;
            next;
          }
        }

        if ($adding_entry >= 2 &&
            $cached_stat == 1 &&
            $cached_lstat == 1) {
          last;
        }
      }

      close($fh);

      $self->assert($adding_entry >= 2 &&
                    $cached_stat == 1 &&
                    $cached_lstat == 1,
        test_msg("Did not see expected 'statcache' TraceLog messages"));

    } else {
      die("Can't read $setup->{log_file}: $!");
    }
  };
  if ($@) {
    $ex = $@;
  }

  test_cleanup($setup->{log_file}, $ex);
}

sub statcache_sftp_upload_file {
  my $self = shift;
  my $tmpdir = $self->{tmpdir};
  my $setup = test_setup($tmpdir, 'statcache');

  my $test_file = File::Spec->rel2abs("$setup->{home_dir}/test.txt");
  my $test_filemode = 33188;
  my $test_filesize = 14;

  my $statcache_tab = File::Spec->rel2abs("$tmpdir/statcache.tab");

  my $rsa_host_key = File::Spec->rel2abs('t/etc/modules/mod_statcache/ssh_host_rsa_key');

  my $config = {
    PidFile => $setup->{pid_file},
    ScoreboardFile => $setup->{scoreboard_file},
    SystemLog => $setup->{log_file},
    TraceLog => $setup->{log_file},
    Trace => 'fsio:10 sftp:20 statcache:20',

    AuthUserFile => $setup->{auth_user_file},
    AuthGroupFile => $setup->{auth_group_file},
    AuthOrder => 'mod_auth_file.c',

    IfModules => {
      'mod_delay.c' => {
        DelayEngine => 'off',
      },

      'mod_sftp.c' => {
        SFTPEngine => 'on',
        SFTPLog => $setup->{log_file},
        SFTPHostKey => $rsa_host_key,
      },

      'mod_statcache.c' => {
        StatCacheEngine => 'on',
        StatCacheTable => $statcache_tab,
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

  require Net::SSH2;

  my $ex;

  # Fork child
  $self->handle_sigchld();
  defined(my $pid = fork()) or die("Can't fork: $!");
  if ($pid) {
    eval {
      my $ssh2 = Net::SSH2->new();

      sleep(1);

      unless ($ssh2->connect('127.0.0.1', $port)) {
        my ($err_code, $err_name, $err_str) = $ssh2->error();
        die("Can't connect to SSH2 server: [$err_name] ($err_code) $err_str");
      }

      unless ($ssh2->auth_password($setup->{user}, $setup->{passwd})) {
        my ($err_code, $err_name, $err_str) = $ssh2->error();
        die("Can't login to SSH2 server: [$err_name] ($err_code) $err_str");
      }
 
      my $sftp = $ssh2->sftp();
      unless ($sftp) {
        my ($err_code, $err_name, $err_str) = $ssh2->error();
        die("Can't use SFTP on SSH2 server: [$err_name] ($err_code) $err_str");
      }

      my $path = 'test.txt';
      my $fh = $sftp->open($path, O_WRONLY|O_CREAT|O_TRUNC, 0644);
      unless ($fh) {
        my ($err_code, $err_name) = $sftp->error();
        die("Can't open test.txt: [$err_name] ($err_code)");
      }

      print $fh "Hello, World!\n";

      # To issue the FXP_CLOSE, we have to explicitly destroy the filehandle
      $fh = undef;

      my $attrs = $sftp->stat($path, 1);
      unless ($attrs) {
        my ($err_code, $err_name) = $sftp->error();
        die("STAT $path failed: [$err_name] ($err_code)");
      }

      my $expected = $test_filesize;
      my $file_size = $attrs->{size};
      $self->assert($expected == $file_size,
        test_msg("Expected file size '$expected', got '$file_size'"));

      $expected = $<;
      my $file_uid = $attrs->{uid};
      $self->assert($expected == $file_uid,
        test_msg("Expected file UID '$expected', got '$file_uid'"));

      $expected = $(;
      my $file_gid = $attrs->{gid};
      $self->assert($expected == $file_gid,
        test_msg("Expected file GID '$expected', got '$file_gid'"));

      $expected = $test_filemode;
      my $file_mode = $attrs->{mode};
      $self->assert($expected == $file_mode,
        test_msg("Expected file mode '$expected', got '$file_mode'"));

      # Do the stat again; we'll check the logs to see if mod_statcache
      # did its job.
      $attrs = $sftp->stat($path, 1);
      unless ($attrs) {
        my ($err_code, $err_name) = $sftp->error();
        die("STAT $path failed: [$err_name] ($err_code)");
      }

      $expected = $test_filesize;
      $file_size = $attrs->{size};
      $self->assert($expected == $file_size,
        test_msg("Expected file size '$expected', got '$file_size'"));

      $expected = $<;
      $file_uid = $attrs->{uid};
      $self->assert($expected == $file_uid,
        test_msg("Expected file UID '$expected', got '$file_uid'"));

      $expected = $(;
      $file_gid = $attrs->{gid};
      $self->assert($expected == $file_gid,
        test_msg("Expected file GID '$expected', got '$file_gid'"));

      $expected = $test_filemode;
      $file_mode = $attrs->{mode};
      $self->assert($expected == $file_mode,
        test_msg("Expected file mode '$expected', got '$file_mode'"));

      $sftp = undef;
      $ssh2->disconnect();

      # Now connect again, do another stat, and see if we're still using
      # the cached entry.
      $ssh2 = Net::SSH2->new();
      unless ($ssh2->connect('127.0.0.1', $port)) {
        my ($err_code, $err_name, $err_str) = $ssh2->error();
        die("Can't connect to SSH2 server: [$err_name] ($err_code) $err_str");
      }

      unless ($ssh2->auth_password($setup->{user}, $setup->{passwd})) {
        my ($err_code, $err_name, $err_str) = $ssh2->error();
        die("Can't login to SSH2 server: [$err_name] ($err_code) $err_str");
      }

      $sftp = $ssh2->sftp();
      unless ($sftp) {
        my ($err_code, $err_name, $err_str) = $ssh2->error();
        die("Can't use SFTP on SSH2 server: [$err_name] ($err_code) $err_str");
      }

      $attrs = $sftp->stat($path, 1);
      unless ($attrs) {
        my ($err_code, $err_name) = $sftp->error();
        die("STAT $path failed: [$err_name] ($err_code)");
      }

      $expected = $test_filesize;
      $file_size = $attrs->{size};
      $self->assert($expected == $file_size,
        test_msg("Expected file size '$expected', got '$file_size'"));

      $expected = $<;
      $file_uid = $attrs->{uid};
      $self->assert($expected == $file_uid,
        test_msg("Expected file UID '$expected', got '$file_uid'"));

      $expected = $(;
      $file_gid = $attrs->{gid};
      $self->assert($expected == $file_gid,
        test_msg("Expected file GID '$expected', got '$file_gid'"));

      $expected = $test_filemode;
      $file_mode = $attrs->{mode};
      $self->assert($expected == $file_mode,
        test_msg("Expected file mode '$expected', got '$file_mode'"));

      $sftp = undef;
      $ssh2->disconnect();
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

  eval {
    if (open(my $fh, "< $setup->{log_file}")) {
      my $adding_entry = 0;
      my $cached_stat = 0;
      my $cached_lstat = 0;

      if ($^O eq 'darwin') {
        # MacOSX-specific hack
        $test_file = '/private' . $test_file;
      }

      while (my $line = <$fh>) {
        chomp($line);

        if ($ENV{TEST_VERBOSE}) {
          print STDERR "# line: $line\n";
        }

        if ($line =~ /<statcache:9>/) {
          if ($line =~ /adding entry.*?type file/) {
            $adding_entry++;
            next;
          }
        }

        if ($line =~ /<statcache:11>/) {
          if ($cached_stat == 0 &&
              $line =~ /using cached stat.*?path '$test_file'/) {
            $cached_stat++;
            next;
          }

          if ($cached_lstat == 0 &&
              $line =~ /using cached lstat.*?path '$test_file'/) {
            $cached_lstat++;
            next;
          }
        }

        if ($adding_entry >= 2 &&
            $cached_stat == 1 &&
            $cached_lstat == 1) {
          last;
        }
      }

      close($fh);

      $self->assert($adding_entry >= 2 &&
                    $cached_stat == 1 &&
                    $cached_lstat == 1,
        test_msg("Did not see expected 'statcache' TraceLog messages"));

    } else {
      die("Can't read $setup->{log_file}: $!");
    }
  };
  if ($@) {
    $ex = $@;
  }

  test_cleanup($setup->{log_file}, $ex);
}

1;
