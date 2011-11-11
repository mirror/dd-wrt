package ProFTPD::Tests::Modules::mod_readme;

use lib qw(t/lib);
use base qw(ProFTPD::TestSuite::Child);
use strict;

use File::Path qw(mkpath);
use File::Spec;
use IO::Handle;

use ProFTPD::TestSuite::FTP;
use ProFTPD::TestSuite::Utils qw(:auth :config :running :test :testsuite);

$| = 1;

my $order = 0;

my $TESTS = {
  readme_login_path => {
    order => ++$order,
    test_class => [qw(forking)],
  },

  readme_login_path_nonexistent => {
    order => ++$order,
    test_class => [qw(forking)],
  },

  readme_login_pattern => {
    order => ++$order,
    test_class => [qw(forking)],
  },

  readme_cwd_pattern => {
    order => ++$order,
    test_class => [qw(forking)],
  },

  readme_cwd_pattern_multiple_matches => {
    order => ++$order,
    test_class => [qw(forking)],
  },

  readme_login_path_displaylogin_bug3605 => {
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

sub readme_login_path {
  my $self = shift;
  my $tmpdir = $self->{tmpdir};

  my $config_file = "$tmpdir/readme.conf";
  my $pid_file = File::Spec->rel2abs("$tmpdir/readme.pid");
  my $scoreboard_file = File::Spec->rel2abs("$tmpdir/readme.scoreboard");

  my $log_file = File::Spec->rel2abs('tests.log');

  my $auth_user_file = File::Spec->rel2abs("$tmpdir/readme.passwd");
  my $auth_group_file = File::Spec->rel2abs("$tmpdir/readme.group");

  my $user = 'proftpd';
  my $passwd = 'test';
  my $group = 'ftpd';
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
  auth_group_write($auth_group_file, $group, $gid, $user);

  my $readme_file = File::Spec->rel2abs("$home_dir/README");
  if (open(my $fh, "> $readme_file")) {
    print $fh "Hello, mod_readme users!\n";

    unless (close($fh)) {
      die("Can't write $readme_file: $!");
    }

  } else {
    die("Can't open $readme_file: $!");
  }

  my $config = {
    PidFile => $pid_file,
    ScoreboardFile => $scoreboard_file,
    SystemLog => $log_file,
    TraceLog => $log_file,
    Trace => 'response:10',

    AuthUserFile => $auth_user_file,
    AuthGroupFile => $auth_group_file,

    IfModules => {
      'mod_delay.c' => {
        DelayEngine => 'off',
      },

      'mod_readme.c' => {
        DisplayReadme => $readme_file,
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

      my $resp_code = $client->response_code();
      my $resp_msg = $client->response_msg(0);

      my $expected;

      $expected = 230;
      $self->assert($expected == $resp_code,
        test_msg("Expected $expected, got $resp_code"));

      $expected = "User $user logged in";
      $self->assert($expected eq $resp_msg,
        test_msg("Expected '$expected', got '$resp_msg'"));
 
      $resp_msg = $client->response_msg(2);

      $expected = "Please read the file (.*?)\/README";
      $self->assert(qr/$expected/, $resp_msg,
        test_msg("Expected '$expected', got '$resp_msg'"));

      $resp_msg = $client->response_msg(3);

      $expected = "it was last modified on (.*?) \- 0 days ago";
      $self->assert(qr/$expected/, $resp_msg,
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

  if ($ex) {
    die($ex);
  }

  unlink($log_file);
}

sub readme_login_path_nonexistent {
  my $self = shift;
  my $tmpdir = $self->{tmpdir};

  my $config_file = "$tmpdir/readme.conf";
  my $pid_file = File::Spec->rel2abs("$tmpdir/readme.pid");
  my $scoreboard_file = File::Spec->rel2abs("$tmpdir/readme.scoreboard");

  my $log_file = File::Spec->rel2abs('tests.log');

  my $auth_user_file = File::Spec->rel2abs("$tmpdir/readme.passwd");
  my $auth_group_file = File::Spec->rel2abs("$tmpdir/readme.group");

  my $user = 'proftpd';
  my $passwd = 'test';
  my $group = 'ftpd';
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
  auth_group_write($auth_group_file, $group, $gid, $user);

  my $readme_file = File::Spec->rel2abs("$home_dir/README");

  my $config = {
    PidFile => $pid_file,
    ScoreboardFile => $scoreboard_file,
    SystemLog => $log_file,
    TraceLog => $log_file,
    Trace => 'response:10',

    AuthUserFile => $auth_user_file,
    AuthGroupFile => $auth_group_file,

    IfModules => {
      'mod_delay.c' => {
        DelayEngine => 'off',
      },

      'mod_readme.c' => {
        DisplayReadme => $readme_file,
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

      my $resp_code = $client->response_code();
      my $resp_msg = $client->response_msg(0);

      my $expected;

      $expected = 230;
      $self->assert($expected == $resp_code,
        test_msg("Expected $expected, got $resp_code"));

      $expected = "User $user logged in";
      $self->assert($expected eq $resp_msg,
        test_msg("Expected '$expected', got '$resp_msg'"));
 
      $resp_msg = $client->response_msg(2);
      $self->assert(!defined($resp_msg),
        test_msg("Expected undefined, got '$resp_msg'"));

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

  if ($ex) {
    die($ex);
  }

  unlink($log_file);
}

sub readme_login_pattern {
  my $self = shift;
  my $tmpdir = $self->{tmpdir};

  my $config_file = "$tmpdir/readme.conf";
  my $pid_file = File::Spec->rel2abs("$tmpdir/readme.pid");
  my $scoreboard_file = File::Spec->rel2abs("$tmpdir/readme.scoreboard");

  my $log_file = File::Spec->rel2abs('tests.log');

  my $auth_user_file = File::Spec->rel2abs("$tmpdir/readme.passwd");
  my $auth_group_file = File::Spec->rel2abs("$tmpdir/readme.group");

  my $user = 'proftpd';
  my $passwd = 'test';
  my $group = 'ftpd';
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
  auth_group_write($auth_group_file, $group, $gid, $user);

  my $readme_file = File::Spec->rel2abs("$home_dir/README");
  if (open(my $fh, "> $readme_file")) {
    print $fh "Hello, mod_readme users!\n";

    unless (close($fh)) {
      die("Can't write $readme_file: $!");
    }

  } else {
    die("Can't open $readme_file: $!");
  }

  my $config = {
    PidFile => $pid_file,
    ScoreboardFile => $scoreboard_file,
    SystemLog => $log_file,
    TraceLog => $log_file,
    Trace => 'response:10',

    AuthUserFile => $auth_user_file,
    AuthGroupFile => $auth_group_file,

    IfModules => {
      'mod_delay.c' => {
        DelayEngine => 'off',
      },

      'mod_readme.c' => {
        DisplayReadme => 'README',
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

      my $resp_code = $client->response_code();
      my $resp_msg = $client->response_msg(0);

      my $expected;

      $expected = 230;
      $self->assert($expected == $resp_code,
        test_msg("Expected $expected, got $resp_code"));

      $expected = "User $user logged in";
      $self->assert($expected eq $resp_msg,
        test_msg("Expected '$expected', got '$resp_msg'"));
 
      $resp_msg = $client->response_msg(2);

      $expected = "Please read the file README";
      $self->assert(qr/$expected/, $resp_msg,
        test_msg("Expected '$expected', got '$resp_msg'"));

      $resp_msg = $client->response_msg(3);

      $expected = "it was last modified on (.*?) \- 0 days ago";
      $self->assert(qr/$expected/, $resp_msg,
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

  if ($ex) {
    die($ex);
  }

  unlink($log_file);
}

sub readme_cwd_pattern {
  my $self = shift;
  my $tmpdir = $self->{tmpdir};

  my $config_file = "$tmpdir/readme.conf";
  my $pid_file = File::Spec->rel2abs("$tmpdir/readme.pid");
  my $scoreboard_file = File::Spec->rel2abs("$tmpdir/readme.scoreboard");

  my $log_file = File::Spec->rel2abs('tests.log');

  my $auth_user_file = File::Spec->rel2abs("$tmpdir/readme.passwd");
  my $auth_group_file = File::Spec->rel2abs("$tmpdir/readme.group");

  my $user = 'proftpd';
  my $passwd = 'test';
  my $group = 'ftpd';
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
  auth_group_write($auth_group_file, $group, $gid, $user);

  my $readme_file = File::Spec->rel2abs("$home_dir/README");
  if (open(my $fh, "> $readme_file")) {
    print $fh "Hello, mod_readme users!\n";

    unless (close($fh)) {
      die("Can't write $readme_file: $!");
    }

  } else {
    die("Can't open $readme_file: $!");
  }

  my $subdir = File::Spec->rel2abs("$home_dir/subdir.d");
  mkpath($subdir);

  $readme_file = File::Spec->rel2abs("$subdir/README");
  if (open(my $fh, "> $readme_file")) {
    print $fh "Welcome to subdir.d\n";

    unless (close($fh)) {
      die("Can't write $readme_file: $!");
    }

  } else {
    die("Can't open $readme_file: $!");
  }

  my $config = {
    PidFile => $pid_file,
    ScoreboardFile => $scoreboard_file,
    SystemLog => $log_file,
    TraceLog => $log_file,
    Trace => 'response:10',

    AuthUserFile => $auth_user_file,
    AuthGroupFile => $auth_group_file,

    IfModules => {
      'mod_delay.c' => {
        DelayEngine => 'off',
      },

      'mod_readme.c' => {
        DisplayReadme => 'README',
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

      my $resp_code = $client->response_code();
      my $resp_msg = $client->response_msg(0);

      my $expected;

      $expected = 230;
      $self->assert($expected == $resp_code,
        test_msg("Expected $expected, got $resp_code"));

      $expected = "User $user logged in";
      $self->assert($expected eq $resp_msg,
        test_msg("Expected '$expected', got '$resp_msg'"));
 
      $resp_msg = $client->response_msg(2);

      $expected = "Please read the file README";
      $self->assert(qr/$expected/, $resp_msg,
        test_msg("Expected '$expected', got '$resp_msg'"));

      $resp_msg = $client->response_msg(3);

      $expected = "it was last modified on (.*?) \- 0 days ago";
      $self->assert(qr/$expected/, $resp_msg,
        test_msg("Expected '$expected', got '$resp_msg'"));

      $client->cwd('subdir.d');

      $resp_code = $client->response_code();
      $resp_msg = $client->response_msg(0);

      $expected = 250;
      $self->assert($expected == $resp_code,
        test_msg("Expected $expected, got $resp_code"));

      $expected = "CWD command successful";
      $self->assert($expected eq $resp_msg,
        test_msg("Expected '$expected', got '$resp_msg'"));

      $resp_msg = $client->response_msg(1);

      $expected = " Please read the file README";
      $self->assert(qr/$expected/, $resp_msg,
        test_msg("Expected '$expected', got '$resp_msg'"));

      $resp_msg = $client->response_msg(2);

      $expected = "it was last modified on (.*?) \- 0 days ago";
      $self->assert(qr/$expected/, $resp_msg,
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

  if ($ex) {
    die($ex);
  }

  unlink($log_file);
}

sub readme_cwd_pattern_multiple_matches {
  my $self = shift;
  my $tmpdir = $self->{tmpdir};

  my $config_file = "$tmpdir/readme.conf";
  my $pid_file = File::Spec->rel2abs("$tmpdir/readme.pid");
  my $scoreboard_file = File::Spec->rel2abs("$tmpdir/readme.scoreboard");

  my $log_file = File::Spec->rel2abs('tests.log');

  my $auth_user_file = File::Spec->rel2abs("$tmpdir/readme.passwd");
  my $auth_group_file = File::Spec->rel2abs("$tmpdir/readme.group");

  my $user = 'proftpd';
  my $passwd = 'test';
  my $group = 'ftpd';
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
  auth_group_write($auth_group_file, $group, $gid, $user);

  my $readme_file = File::Spec->rel2abs("$home_dir/README");
  if (open(my $fh, "> $readme_file")) {
    print $fh "Hello, mod_readme users!\n";

    unless (close($fh)) {
      die("Can't write $readme_file: $!");
    }

  } else {
    die("Can't open $readme_file: $!");
  }

  my $subdir = File::Spec->rel2abs("$home_dir/subdir.d");
  mkpath($subdir);

  $readme_file = File::Spec->rel2abs("$subdir/README");
  if (open(my $fh, "> $readme_file")) {
    print $fh "Welcome to subdir.d\n";

    unless (close($fh)) {
      die("Can't write $readme_file: $!");
    }

  } else {
    die("Can't open $readme_file: $!");
  }

  $readme_file = File::Spec->rel2abs("$subdir/READMETOO");
  if (open(my $fh, "> $readme_file")) {
    print $fh "Welcome to subdir.d from me as well\n";

    unless (close($fh)) {
      die("Can't write $readme_file: $!");
    }

  } else {
    die("Can't open $readme_file: $!");
  }

  my $config = {
    PidFile => $pid_file,
    ScoreboardFile => $scoreboard_file,
    SystemLog => $log_file,
    TraceLog => $log_file,
    Trace => 'response:10',

    AuthUserFile => $auth_user_file,
    AuthGroupFile => $auth_group_file,

    IfModules => {
      'mod_delay.c' => {
        DelayEngine => 'off',
      },

      'mod_readme.c' => {
        DisplayReadme => 'README*',
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

      my $resp_code = $client->response_code();
      my $resp_msg = $client->response_msg(0);

      my $expected;

      $expected = 230;
      $self->assert($expected == $resp_code,
        test_msg("Expected $expected, got $resp_code"));

      $expected = "User $user logged in";
      $self->assert($expected eq $resp_msg,
        test_msg("Expected '$expected', got '$resp_msg'"));
 
      $resp_msg = $client->response_msg(2);

      $expected = "Please read the file README";
      $self->assert(qr/$expected/, $resp_msg,
        test_msg("Expected '$expected', got '$resp_msg'"));

      $resp_msg = $client->response_msg(3);

      $expected = "it was last modified on (.*?) \- 0 days ago";
      $self->assert(qr/$expected/, $resp_msg,
        test_msg("Expected '$expected', got '$resp_msg'"));

      $client->cwd('subdir.d');

      $resp_code = $client->response_code();
      $resp_msg = $client->response_msg(0);

      $expected = 250;
      $self->assert($expected == $resp_code,
        test_msg("Expected $expected, got $resp_code"));

      $expected = "CWD command successful";
      $self->assert($expected eq $resp_msg,
        test_msg("Expected '$expected', got '$resp_msg'"));

      $resp_msg = $client->response_msg(1);

      $expected = " Please read the file README";
      $self->assert(qr/$expected/, $resp_msg,
        test_msg("Expected '$expected', got '$resp_msg'"));

      $resp_msg = $client->response_msg(2);

      $expected = "it was last modified on (.*?) \- 0 days ago";
      $self->assert(qr/$expected/, $resp_msg,
        test_msg("Expected '$expected', got '$resp_msg'"));

      $resp_msg = $client->response_msg(3);

      $expected = " Please read the file READMETOO";
      $self->assert(qr/$expected/, $resp_msg,
        test_msg("Expected '$expected', got '$resp_msg'"));

      $resp_msg = $client->response_msg(4);

      $expected = "it was last modified on (.*?) \- 0 days ago";
      $self->assert(qr/$expected/, $resp_msg,
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

  if ($ex) {
    die($ex);
  }

  unlink($log_file);
}

sub readme_login_path_displaylogin_bug3605 {
  my $self = shift;
  my $tmpdir = $self->{tmpdir};

  my $config_file = "$tmpdir/readme.conf";
  my $pid_file = File::Spec->rel2abs("$tmpdir/readme.pid");
  my $scoreboard_file = File::Spec->rel2abs("$tmpdir/readme.scoreboard");

  my $log_file = File::Spec->rel2abs('tests.log');

  my $auth_user_file = File::Spec->rel2abs("$tmpdir/readme.passwd");
  my $auth_group_file = File::Spec->rel2abs("$tmpdir/readme.group");

  my $user = 'proftpd';
  my $passwd = 'test';
  my $group = 'ftpd';
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
  auth_group_write($auth_group_file, $group, $gid, $user);

  my $login_file = File::Spec->rel2abs("$home_dir/.welcome");
  if (open(my $fh, "> $login_file")) {
    print $fh "\nWelcome to my FTP server, %U.\n";
    print $fh "You are user number %N out of\n";
    print $fh "a maximum of %M simultaneous\n";
    print $fh "users.\n\n";

    unless (close($fh)) {
      die("Can't write $login_file: $!");
    }

  } else {
    die("Can't open $login_file: $!");
  }

  my $readme_file = File::Spec->rel2abs("$home_dir/README");
  if (open(my $fh, "> $readme_file")) {
    print $fh "Hello, mod_readme users!\n";

    unless (close($fh)) {
      die("Can't write $readme_file: $!");
    }

  } else {
    die("Can't open $readme_file: $!");
  }

  my $config = {
    PidFile => $pid_file,
    ScoreboardFile => $scoreboard_file,
    SystemLog => $log_file,
    TraceLog => $log_file,
    Trace => 'response:10',

    AuthUserFile => $auth_user_file,
    AuthGroupFile => $auth_group_file,

    DisplayLogin => $login_file,

    IfModules => {
      'mod_delay.c' => {
        DelayEngine => 'off',
      },

      'mod_readme.c' => {
        DisplayReadme => $readme_file,
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

      my $resp_code = $client->response_code();
      my $resp_msg = $client->response_msg(0);

      my $expected;

      $expected = 230;
      $self->assert($expected == $resp_code,
        test_msg("Expected $expected, got $resp_code"));

      $resp_msg = $client->response_msg(6);

      $expected = " User $user logged in";
      $self->assert($expected eq $resp_msg,
        test_msg("Expected '$expected', got '$resp_msg'"));
 
      $resp_msg = $client->response_msg(8);

      $expected = "Please read the file (.*?)\/README";
      $self->assert(qr/$expected/, $resp_msg,
        test_msg("Expected '$expected', got '$resp_msg'"));

      $resp_msg = $client->response_msg(9);

      $expected = "it was last modified on (.*?) \- 0 days ago";
      $self->assert(qr/$expected/, $resp_msg,
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

  if ($ex) {
    die($ex);
  }

  unlink($log_file);
}

1;
