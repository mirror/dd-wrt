package ProFTPD::Tests::Modules::mod_ifsession;

use lib qw(t/lib);
use base qw(ProFTPD::TestSuite::Child);
use strict;

use File::Path qw(mkpath);
use File::Spec;
use IO::Handle;
use POSIX qw(:fcntl_h);

use ProFTPD::TestSuite::FTP;
use ProFTPD::TestSuite::Utils qw(:auth :config :running :test :testsuite);

$| = 1;

my $order = 0;

my $TESTS = {
  ifuser_allowoverwrite => {
    order => ++$order,
    test_class => [qw(forking)],
  },

  ifgroup_dir_allow_mkd_bug3467 => {
    order => ++$order,
    test_class => [qw(bug forking rootprivs)],
  },

  ifuser_dir_allow_mkd_bug3467 => {
    order => ++$order,
    test_class => [qw(bug forking)],
  },

  ifclass_dir_allow_mkd_bug3467 => {
    order => ++$order,
    test_class => [qw(bug forking)],
  },

  ifuser_login_deny_by_ip => {
    order => ++$order,
    test_class => [qw(forking)],
  },

  ifgroup_ifclass_login_allowclass_bug3547 => {
    order => ++$order,
    test_class => [qw(bug forking)],
  },

  ifuser_regex_login_deny_by_ip_bug3625 => {
    order => ++$order,
    test_class => [qw(bug forking)],
  },

  ifauthenticated_bug3629 => {
    order => ++$order,
    test_class => [qw(bug forking)],
  },

  ifgroup_displaylogin_bug3882 => {
    order => ++$order,
    test_class => [qw(bug forking rootprivs)],
  },

  ifgroup_dir_allow_stor_bug3881 => {
    order => ++$order,
    test_class => [qw(bug forking rootprivs)],
  },

  ifgroup_dir_allow_stor_bug3881_sftp => {
    order => ++$order,
    test_class => [qw(bug forking mod_sftp rootprivs)],
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
  $self->SUPER::set_up(@_);

  # Make sure that mod_sftp does not complain about permissions on the hostkey
  # files.

  my $rsa_host_key = File::Spec->rel2abs("t/etc/modules/mod_sftp/ssh_host_rsa_key");
  my $dsa_host_key = File::Spec->rel2abs("t/etc/modules/mod_sftp/ssh_host_dsa_key");

  unless (chmod(0400, $rsa_host_key, $dsa_host_key)) {
    die("Can't set perms on $rsa_host_key, $dsa_host_key: $!");
  }
}

sub ifuser_allowoverwrite { 
  my $self = shift;
  my $tmpdir = $self->{tmpdir};

  my $config_file = "$tmpdir/ifsess.conf";
  my $pid_file = File::Spec->rel2abs("$tmpdir/ifsess.pid");
  my $scoreboard_file = File::Spec->rel2abs("$tmpdir/ifsess.scoreboard");

  my $log_file = test_get_logfile();

  my $auth_user_file = File::Spec->rel2abs("$tmpdir/ifsess.passwd");
  my $auth_group_file = File::Spec->rel2abs("$tmpdir/ifsess.group");

  my $test_file = File::Spec->rel2abs($config_file);

  my $user = 'proftpd';
  my $passwd = 'test';
  my $group = 'ftpd';
  my $home_dir = File::Spec->rel2abs($tmpdir);
  my $uid = 500;
  my $gid = 500;

  my $ftpaccess_file = File::Spec->rel2abs("$tmpdir/.ftpaccess");
  if (open(my $fh, "> $ftpaccess_file")) {
    print $fh "AllowOverwrite on\n";

    unless (close($fh)) {
      die("Can't write $ftpaccess_file: $!");
    }

  } else {
    die("Can't open $ftpaccess_file: $!");
  }

  my $test_file = File::Spec->rel2abs("$tmpdir/test.txt");
  if (open(my $fh, "> $test_file")) {
    close($fh);

  } else {
    die("Can't open $test_file: $!");
  }

  # Make sure that, if we're running as root, that the home directory has
  # permissions/privs set for the account we create
  if ($< == 0) {
    unless (chmod(0755, $home_dir, $test_file)) {
      die("Can't set perms on $home_dir to 0755: $!");
    }

    unless (chown($uid, $gid, $home_dir, $test_file)) {
      die("Can't set owner of $home_dir to $uid/$gid: $!");
    }
  }

  auth_user_write($auth_user_file, $user, $passwd, $uid, $gid, $home_dir,
    '/bin/bash');
  auth_group_write($auth_group_file, $group, $gid, $user);

  my $config = {
    PidFile => $pid_file,
    ScoreboardFile => $scoreboard_file,
    SystemLog => $log_file,

    AllowOverride => 'on',
    AuthUserFile => $auth_user_file,
    AuthGroupFile => $auth_group_file,

    IfModules => {
      'mod_delay.c' => {
        DelayEngine => 'off',
      },
    },
  };

  my ($port, $config_user, $config_group) = config_write($config_file, $config);

  # Append the mod_ifsession config to the end of the config file
  if (open(my $fh, ">> $config_file")) {
    print $fh <<EOC;
<IfModule mod_ifsession.c>
  <IfUser $user>
    AllowOverride on
  </IfUser>

  <IfUser !$user>
    AllowOverride off
  </IfUser>
</IfModule>
EOC
    unless (close($fh)) {
      die("Can't write $config_file: $!");
    }

  } else {
    die("Can't open $config_file: $!");
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
      my $client = ProFTPD::TestSuite::FTP->new('127.0.0.1', $port);
      $client->login($user, $passwd);

      my $conn = $client->stor_raw('test.txt');
      unless ($conn) {
        die("Failed to STOR test.txt: " . $client->response_code() . " " .
          $client->response_msg());
      }

      eval { $conn->close() };

      my $resp_code = $client->response_code();
      my $resp_msg = $client->response_msg();
      $self->assert_transfer_ok($resp_code, $resp_msg);

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
    test_append_logfile($log_file, $ex);
    unlink($log_file);

    die($ex);
  }

  unlink($log_file);
}

sub ifgroup_dir_allow_mkd_bug3467 { 
  my $self = shift;
  my $tmpdir = $self->{tmpdir};

  my $config_file = "$tmpdir/ifsess.conf";
  my $pid_file = File::Spec->rel2abs("$tmpdir/ifsess.pid");
  my $scoreboard_file = File::Spec->rel2abs("$tmpdir/ifsess.scoreboard");

  my $log_file = test_get_logfile();

  my $auth_user_file = File::Spec->rel2abs("$tmpdir/ifsess.passwd");
  my $auth_group_file = File::Spec->rel2abs("$tmpdir/ifsess.group");

  my $test_file = File::Spec->rel2abs($config_file);

  my $user = 'proftpd';
  my $passwd = 'test';
  my $group = 'ftpd'; 
  my $home_dir = File::Spec->rel2abs($tmpdir);
  my $uid = 500;
  my $gid = 500;

  my $user_dir = File::Spec->rel2abs("$home_dir/$user");
  mkpath($user_dir);

  my $ftpaccess_file = File::Spec->rel2abs("$user_dir/.ftpaccess");
  if (open(my $fh, "> $ftpaccess_file")) {
    print $fh "Umask 022\n";

    unless (close($fh)) {
      die("Can't write $ftpaccess_file: $!");
    }

  } else {
    die("Can't open $ftpaccess_file: $!");
  }

  # Make sure that, if we're running as root, that the home directory has
  # permissions/privs set for the account we create
  if ($< == 0) {
    unless (chmod(0755, $home_dir, $user_dir)) {
      die("Can't set perms on $home_dir to 0755: $!");
    }

    unless (chown($uid, $gid, $home_dir, $user_dir)) {
      die("Can't set owner of $home_dir to $uid/$gid: $!");
    }
  }

  auth_user_write($auth_user_file, $user, $passwd, $uid, $gid, $home_dir,
    '/bin/bash');
  auth_group_write($auth_group_file, $group, $gid, $user);

  my $config = {
    PidFile => $pid_file,
    ScoreboardFile => $scoreboard_file,
    SystemLog => $log_file,

    AllowOverride => 'on',
    AuthUserFile => $auth_user_file,
    AuthGroupFile => $auth_group_file,

    DefaultRoot => "~ $group",

    IfModules => {
      'mod_delay.c' => {
        DelayEngine => 'off',
      },
    },
  };

  my ($port, $config_user, $config_group) = config_write($config_file, $config);

  # Append the mod_ifsession config to the end of the config file
  if (open(my $fh, ">> $config_file")) {
    print $fh <<EOC;
<Directory />
  <Limit ALL>
    DenyAll
  </Limit>
</Directory>

<IfGroup $group>
  <Directory ~>
    <Limit ALL>
      AllowAll
    </Limit>
  </Directory>
</IfGroup>
EOC
    unless (close($fh)) {
      die("Can't write $config_file: $!");
    }

  } else {
    die("Can't open $config_file: $!");
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
      my $client = ProFTPD::TestSuite::FTP->new('127.0.0.1', $port);
      $client->login($user, $passwd);

      $client->pwd(); 
      $client->mkd('foo');
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
    test_append_logfile($log_file, $ex);
    unlink($log_file);

    die($ex);
  }

  unlink($log_file);
}

sub ifuser_dir_allow_mkd_bug3467 { 
  my $self = shift;
  my $tmpdir = $self->{tmpdir};

  my $config_file = "$tmpdir/ifsess.conf";
  my $pid_file = File::Spec->rel2abs("$tmpdir/ifsess.pid");
  my $scoreboard_file = File::Spec->rel2abs("$tmpdir/ifsess.scoreboard");

  my $log_file = test_get_logfile();

  my $auth_user_file = File::Spec->rel2abs("$tmpdir/ifsess.passwd");
  my $auth_group_file = File::Spec->rel2abs("$tmpdir/ifsess.group");

  my $test_file = File::Spec->rel2abs($config_file);

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

  my $config = {
    PidFile => $pid_file,
    ScoreboardFile => $scoreboard_file,
    SystemLog => $log_file,
    TraceLog => $log_file,
    Trace => 'directory:20',

    AuthUserFile => $auth_user_file,
    AuthGroupFile => $auth_group_file,

    IfModules => {
      'mod_delay.c' => {
        DelayEngine => 'off',
      },
    },
  };

  my ($port, $config_user, $config_group) = config_write($config_file, $config);

  # Append the mod_ifsession config to the end of the config file
  if (open(my $fh, ">> $config_file")) {
    my $limit_dir = $home_dir;
    if ($^O eq 'darwin') {
      # MacOSX hack
      $limit_dir = ('/private' . $home_dir);
    }

    print $fh <<EOC;
<Directory />
  <Limit ALL>
    DenyAll
  </Limit>
</Directory>

<IfUser $user>
  <Directory $limit_dir>
    <Limit MKD>
      AllowAll
    </Limit>
  </Directory>
</IfUser>
EOC
    unless (close($fh)) {
      die("Can't write $config_file: $!");
    }

  } else {
    die("Can't open $config_file: $!");
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
      my $client = ProFTPD::TestSuite::FTP->new('127.0.0.1', $port);
      $client->login($user, $passwd);

      $client->mkd('foo');
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
    test_append_logfile($log_file, $ex);
    unlink($log_file);

    die($ex);
  }

  unlink($log_file);
}

sub ifclass_dir_allow_mkd_bug3467 { 
  my $self = shift;
  my $tmpdir = $self->{tmpdir};

  my $config_file = "$tmpdir/ifsess.conf";
  my $pid_file = File::Spec->rel2abs("$tmpdir/ifsess.pid");
  my $scoreboard_file = File::Spec->rel2abs("$tmpdir/ifsess.scoreboard");

  my $log_file = test_get_logfile();

  my $auth_user_file = File::Spec->rel2abs("$tmpdir/ifsess.passwd");
  my $auth_group_file = File::Spec->rel2abs("$tmpdir/ifsess.group");

  my $test_file = File::Spec->rel2abs($config_file);

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

  my $config = {
    PidFile => $pid_file,
    ScoreboardFile => $scoreboard_file,
    SystemLog => $log_file,
    TraceLog => $log_file,
    Trace => 'directory:20',

    AuthUserFile => $auth_user_file,
    AuthGroupFile => $auth_group_file,
    
    IfModules => {
      'mod_delay.c' => {
        DelayEngine => 'off',
      },
    },
  };

  my ($port, $config_user, $config_group) = config_write($config_file, $config);

  # Append the mod_ifsession config to the end of the config file
  if (open(my $fh, ">> $config_file")) {
    my $limit_dir = $home_dir;
    if ($^O eq 'darwin') {
      $limit_dir = ('/private' . $home_dir);
    }

    print $fh <<EOC;
<Class local>
  From 127.0.0.1
</Class>

<Directory />
  <Limit ALL>
    DenyAll
  </Limit>
</Directory>

<IfClass local>
  <Directory $limit_dir>
    <Limit MKD>
      AllowAll
    </Limit>
  </Directory>
</IfClass>
EOC
    unless (close($fh)) {
      die("Can't write $config_file: $!");
    }

  } else {
    die("Can't open $config_file: $!");
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
      my $client = ProFTPD::TestSuite::FTP->new('127.0.0.1', $port);
      $client->login($user, $passwd);

      $client->mkd('foo');
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
    test_append_logfile($log_file, $ex);
    unlink($log_file);

    die($ex);
  }

  unlink($log_file);
}

sub ifuser_login_deny_by_ip  {
  my $self = shift;
  my $tmpdir = $self->{tmpdir};

  my $config_file = "$tmpdir/ifsess.conf";
  my $pid_file = File::Spec->rel2abs("$tmpdir/ifsess.pid");
  my $scoreboard_file = File::Spec->rel2abs("$tmpdir/ifsess.scoreboard");

  my $log_file = test_get_logfile();

  my $auth_user_file = File::Spec->rel2abs("$tmpdir/ifsess.passwd");
  my $auth_group_file = File::Spec->rel2abs("$tmpdir/ifsess.group");

  my $test_file = File::Spec->rel2abs($config_file);

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

  my $config = {
    PidFile => $pid_file,
    ScoreboardFile => $scoreboard_file,
    SystemLog => $log_file,

    AuthUserFile => $auth_user_file,
    AuthGroupFile => $auth_group_file,

    IfModules => {
      'mod_delay.c' => {
        DelayEngine => 'off',
      },
    },
  };

  my ($port, $config_user, $config_group) = config_write($config_file, $config);

  # Inspired by:
  #
  #  http://forums.proftpd.org/smf/index.php/topic,5153.0.html
  #
  # Append the mod_ifsession config to the end of the config file
  if (open(my $fh, ">> $config_file")) {
    print $fh <<EOC;
<IfModule mod_ifsession.c>
  <IfUser $user>
    <Limit LOGIN>
      Deny from 127.0.0.1
    </Limit>
  </IfUser>
</IfModule>
EOC
    unless (close($fh)) {
      die("Can't write $config_file: $!");
    }

  } else {
    die("Can't open $config_file: $!");
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
      my $client = ProFTPD::TestSuite::FTP->new('127.0.0.1', $port);
      eval { $client->login($user, $passwd) };
      unless ($@) {
        die("Login succeeded unexpectedly");
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
    test_append_logfile($log_file, $ex);
    unlink($log_file);

    die($ex);
  }

  unlink($log_file);
}

sub ifgroup_ifclass_login_allowclass_bug3547 {
  my $self = shift;
  my $tmpdir = $self->{tmpdir};

  my $config_file = "$tmpdir/ifsess.conf";
  my $pid_file = File::Spec->rel2abs("$tmpdir/ifsess.pid");
  my $scoreboard_file = File::Spec->rel2abs("$tmpdir/ifsess.scoreboard");

  my $log_file = test_get_logfile();

  my $auth_user_file = File::Spec->rel2abs("$tmpdir/ifsess.passwd");
  my $auth_group_file = File::Spec->rel2abs("$tmpdir/ifsess.group");

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

  my $test_group = 'test';
  my $test_gid = 501;

  auth_user_write($auth_user_file, $user, $passwd, $uid, $gid, $home_dir,
    '/bin/bash');
  auth_group_write($auth_group_file, $group, $gid, $user);
  auth_group_write($auth_group_file, $test_group, $test_gid, $user);

  my $config = {
    PidFile => $pid_file,
    ScoreboardFile => $scoreboard_file,
    SystemLog => $log_file,
    TraceLog => $log_file,
    Trace => 'ifsession:20',

    AuthUserFile => $auth_user_file,
    AuthGroupFile => $auth_group_file,

    IfModules => {
      'mod_delay.c' => {
        DelayEngine => 'off',
      },
    },
  };

  my ($port, $config_user, $config_group) = config_write($config_file, $config);

  if (open(my $fh, ">> $config_file")) {
    print $fh <<EOC;
<IfModule mod_ifsession.c>
  <Class local>
    From 172.16.0.0/12
    From 192.168.0.0/16
  </Class>

  <Class test>
    From 127.0.0.1
  </Class>

  <IfGroup $test_group>
    <Limit LOGIN>
      AllowClass test
      AllowClass local
      DenyAll
    </Limit>
  </IfGroup>

  <IfGroup !$test_group>
    <Limit LOGIN>
      AllowClass local
      DenyAll
    </Limit>
  </IfGroup>
</IfModule>
EOC
    unless (close($fh)) {
      die("Can't write $config_file: $!");
    }

  } else {
    die("Can't open $config_file: $!");
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
      my $client = ProFTPD::TestSuite::FTP->new('127.0.0.1', $port);
      $client->login($user, $passwd);
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
    test_append_logfile($log_file, $ex);
    unlink($log_file);

    die($ex);
  }

  unlink($log_file);
}

sub ifuser_regex_login_deny_by_ip_bug3625  {
  my $self = shift;
  my $tmpdir = $self->{tmpdir};

  my $config_file = "$tmpdir/ifsess.conf";
  my $pid_file = File::Spec->rel2abs("$tmpdir/ifsess.pid");
  my $scoreboard_file = File::Spec->rel2abs("$tmpdir/ifsess.scoreboard");

  my $log_file = test_get_logfile();

  my $auth_user_file = File::Spec->rel2abs("$tmpdir/ifsess.passwd");
  my $auth_group_file = File::Spec->rel2abs("$tmpdir/ifsess.group");

  my $test_file = File::Spec->rel2abs($config_file);

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

  my $config = {
    PidFile => $pid_file,
    ScoreboardFile => $scoreboard_file,
    SystemLog => $log_file,

    AuthUserFile => $auth_user_file,
    AuthGroupFile => $auth_group_file,

    IfModules => {
      'mod_delay.c' => {
        DelayEngine => 'off',
      },
    },
  };

  my ($port, $config_user, $config_group) = config_write($config_file, $config);

  if (open(my $fh, ">> $config_file")) {
    print $fh <<EOC;
<IfModule mod_ifsession.c>
  <IfUser regex $user>
    <Limit LOGIN>
      Deny from 127.0.0.1
    </Limit>
  </IfUser>
</IfModule>
EOC
    unless (close($fh)) {
      die("Can't write $config_file: $!");
    }

  } else {
    die("Can't open $config_file: $!");
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
      my $client = ProFTPD::TestSuite::FTP->new('127.0.0.1', $port);
      eval { $client->login($user, $passwd) };
      unless ($@) {
        die("Login succeeded unexpectedly");
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
    test_append_logfile($log_file, $ex);
    unlink($log_file);

    die($ex);
  }

  unlink($log_file);
}

sub ifauthenticated_bug3629 { 
  my $self = shift;
  my $tmpdir = $self->{tmpdir};

  my $config_file = "$tmpdir/ifsess.conf";
  my $pid_file = File::Spec->rel2abs("$tmpdir/ifsess.pid");
  my $scoreboard_file = File::Spec->rel2abs("$tmpdir/ifsess.scoreboard");

  my $log_file = test_get_logfile();

  my $auth_user_file = File::Spec->rel2abs("$tmpdir/ifsess.passwd");
  my $auth_group_file = File::Spec->rel2abs("$tmpdir/ifsess.group");

  my $test_file = File::Spec->rel2abs($config_file);

  my $user = 'proftpd';
  my $passwd = 'test';
  my $group = 'ftpd';
  my $home_dir = File::Spec->rel2abs($tmpdir);
  my $uid = 500;
  my $gid = 500;

  my $test_file = File::Spec->rel2abs("$tmpdir/test.txt");
  if (open(my $fh, "> $test_file")) {
    close($fh);

  } else {
    die("Can't open $test_file: $!");
  }

  # Make sure that, if we're running as root, that the home directory has
  # permissions/privs set for the account we create
  if ($< == 0) {
    unless (chmod(0755, $home_dir, $test_file)) {
      die("Can't set perms on $home_dir to 0755: $!");
    }

    unless (chown($uid, $gid, $home_dir, $test_file)) {
      die("Can't set owner of $home_dir to $uid/$gid: $!");
    }
  }

  auth_user_write($auth_user_file, $user, $passwd, $uid, $gid, $home_dir,
    '/bin/bash');
  auth_group_write($auth_group_file, $group, $gid, $user);

  my $config = {
    PidFile => $pid_file,
    ScoreboardFile => $scoreboard_file,
    SystemLog => $log_file,
    TraceLog => $log_file,
    Trace => 'directory:20',

    AuthUserFile => $auth_user_file,
    AuthGroupFile => $auth_group_file,

    AllowOverwrite => 'on',

    IfModules => {
      'mod_delay.c' => {
        DelayEngine => 'off',
      },
    },
  };

  my ($port, $config_user, $config_group) = config_write($config_file, $config);

  # Append the mod_ifsession config to the end of the config file
  if (open(my $fh, ">> $config_file")) {
    my $limit_dir = $home_dir;
    if ($^O eq 'darwin') {
      # MacOSX hack 
      $limit_dir = ('/private' . $home_dir);
    }

    print $fh <<EOC;
<Directory />
  <Limit WRITE>
    DenyAll
  </Limit>
</Directory>

<IfModule mod_ifsession.c>
  <IfAuthenticated>
    <Directory $limit_dir>
      <Limit WRITE>
        AllowAll
      </Limit>
    </Directory>
  </IfAuthenticated>
</IfModule>
EOC
    unless (close($fh)) {
      die("Can't write $config_file: $!");
    }

  } else {
    die("Can't open $config_file: $!");
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
      my $client = ProFTPD::TestSuite::FTP->new('127.0.0.1', $port);
      $client->login($user, $passwd);

      my $conn = $client->stor_raw('test.txt');
      unless ($conn) {
        die("Failed to STOR test.txt: " . $client->response_code() . " " .
          $client->response_msg());
      }

      eval { $conn->close() };

      my $resp_code = $client->response_code();
      my $resp_msg = $client->response_msg();
      $self->assert_transfer_ok($resp_code, $resp_msg);

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
    test_append_logfile($log_file, $ex);
    unlink($log_file);

    die($ex);
  }

  unlink($log_file);
}

sub ifgroup_displaylogin_bug3882 {
  my $self = shift;
  my $tmpdir = $self->{tmpdir};

  my $config_file = "$tmpdir/ifsess.conf";
  my $pid_file = File::Spec->rel2abs("$tmpdir/ifsess.pid");
  my $scoreboard_file = File::Spec->rel2abs("$tmpdir/ifsess.scoreboard");

  my $log_file = test_get_logfile();

  my $auth_user_file = File::Spec->rel2abs("$tmpdir/ifsess.passwd");
  my $auth_group_file = File::Spec->rel2abs("$tmpdir/ifsess.group");

  my $test_file = File::Spec->rel2abs($config_file);

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

  my $login_file = File::Spec->rel2abs("$tmpdir/login.txt");
  if (open(my $fh, "> $login_file")) {
    print $fh "Hello user!\n";

    unless (close($fh)) {
      die("Unable to write $login_file: $!");
    }

  } else {
    die("Unable to open $login_file: $!");
  }

  auth_user_write($auth_user_file, $user, $passwd, $uid, $gid, $home_dir,
    '/bin/bash');
  auth_group_write($auth_group_file, $group, $gid, $user);

  my $config = {
    PidFile => $pid_file,
    ScoreboardFile => $scoreboard_file,
    SystemLog => $log_file,

    AuthUserFile => $auth_user_file,
    AuthGroupFile => $auth_group_file,
    DefaultRoot => '~',

    IfModules => {
      'mod_delay.c' => {
        DelayEngine => 'off',
      },
    },
  };

  my ($port, $config_user, $config_group) = config_write($config_file, $config);

  # Append the mod_ifsession config to the end of the config file
  if (open(my $fh, ">> $config_file")) {
    print $fh <<EOC;
<IfModule mod_ifsession.c>
  <IfGroup $group>
    DisplayLogin $login_file
  </IfGroup>
</IfModule>
EOC
    unless (close($fh)) {
      die("Can't write $config_file: $!");
    }

  } else {
    die("Can't open $config_file: $!");
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
      my $client = ProFTPD::TestSuite::FTP->new('127.0.0.1', $port);
      $client->login($user, $passwd);

      my $resp_code = $client->response_code();
      my $resp_msg = $client->response_msg(0);

      my $expected;

      $expected = 230;
      $self->assert($expected == $resp_code,
        test_msg("Expected response code $expected, got $resp_code"));

      $expected = "Hello user!";
      $self->assert($expected eq $resp_msg,
        test_msg("Expected response message '$expected', got '$resp_msg'"));

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
    test_append_logfile($log_file, $ex);
    unlink($log_file);

    die($ex);
  }

  unlink($log_file);
}

sub ifgroup_dir_allow_stor_bug3881 { 
  my $self = shift;
  my $tmpdir = $self->{tmpdir};

  my $config_file = "$tmpdir/ifsess.conf";
  my $pid_file = File::Spec->rel2abs("$tmpdir/ifsess.pid");
  my $scoreboard_file = File::Spec->rel2abs("$tmpdir/ifsess.scoreboard");

  my $log_file = test_get_logfile();

  my $auth_user_file = File::Spec->rel2abs("$tmpdir/ifsess.passwd");
  my $auth_group_file = File::Spec->rel2abs("$tmpdir/ifsess.group");

  my $test_file = File::Spec->rel2abs($config_file);

  my $user = 'proftpd';
  my $passwd = 'test';
  my $group = 'ftpd'; 
  my $home_dir = File::Spec->rel2abs("$tmpdir/users/$user");
  mkpath($home_dir);
  my $uid = 500;
  my $gid = 500;

  my $sub_dir = File::Spec->rel2abs("$home_dir/test.d");
  mkpath($sub_dir);

  if ($^O eq 'darwin') {
    $home_dir = ('/private' . $home_dir);
  }

  # Make sure that, if we're running as root, that the home directory has
  # permissions/privs set for the account we create
  if ($< == 0) {
    unless (chmod(0755, $home_dir, $sub_dir)) {
      die("Can't set perms on $home_dir to 0755: $!");
    }

    unless (chown($uid, $gid, $home_dir, $sub_dir)) {
      die("Can't set owner of $home_dir to $uid/$gid: $!");
    }
  }

  auth_user_write($auth_user_file, $user, $passwd, $uid, $gid, $home_dir,
    '/bin/bash');
  auth_group_write($auth_group_file, $group, $gid, $user);

  my $config = {
    PidFile => $pid_file,
    ScoreboardFile => $scoreboard_file,
    SystemLog => $log_file,
    TraceLog => $log_file,
    Trace => 'directory:20 fileperms:10 ifsession:20',

    AuthUserFile => $auth_user_file,
    AuthGroupFile => $auth_group_file,
    DefaultRoot => '~',

    IfModules => {
      'mod_delay.c' => {
        DelayEngine => 'off',
      },
    },
  };

  my ($port, $config_user, $config_group) = config_write($config_file, $config);

  # Append the mod_ifsession config to the end of the config file
  if (open(my $fh, ">> $config_file")) {
    my ($volume, $parent_dir, $file) = File::Spec->splitpath($home_dir);
    my $limit_dir = File::Spec->rel2abs("$parent_dir/*/test.d");
    if ($^O eq 'darwin') {
      # MacOSX hack
      $limit_dir = ('/private' . $limit_dir);
    }

    # XXX Hack, for now:
    $limit_dir = '~/test.d';

    print $fh <<EOC;
<Directory ~>
  <Limit WRITE>
    DenyAll
  </Limit>
</Directory>

<IfGroup $group>
  <Directory $limit_dir>
    <Limit MKD XMKD RMD XRMD>
      DenyAll
    </Limit>

    <Limit WRITE>
      AllowAll
    </Limit>
  </Directory>
</IfGroup>
EOC
    unless (close($fh)) {
      die("Can't write $config_file: $!");
    }

  } else {
    die("Can't open $config_file: $!");
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
      my $client = ProFTPD::TestSuite::FTP->new('127.0.0.1', $port);
      $client->login($user, $passwd);
      $client->cwd('test.d');

      my $path = 'test.txt';
      my $conn = $client->stor_raw($path);
      unless ($conn) {
        die("Can't STOR $path: " . $client->response_code() . " " .
          $client->response_msg());
      }

      my $buf = "Hello, World!\n";
      $conn->write($buf, length($buf), 25);
      eval { $conn->close() };

      my $resp_code = $client->response_code();
      my $resp_msg = $client->response_msg();
      $self->assert_transfer_ok($resp_code, $resp_msg);

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
    test_append_logfile($log_file, $ex);
    unlink($log_file);

    die($ex);
  }

  unlink($log_file);
}

sub ifgroup_dir_allow_stor_bug3881_sftp { 
  my $self = shift;
  my $tmpdir = $self->{tmpdir};

  my $config_file = "$tmpdir/ifsess.conf";
  my $pid_file = File::Spec->rel2abs("$tmpdir/ifsess.pid");
  my $scoreboard_file = File::Spec->rel2abs("$tmpdir/ifsess.scoreboard");

  my $log_file = test_get_logfile();

  my $auth_user_file = File::Spec->rel2abs("$tmpdir/ifsess.passwd");
  my $auth_group_file = File::Spec->rel2abs("$tmpdir/ifsess.group");

  my $test_file = File::Spec->rel2abs($config_file);

  my $user = 'proftpd';
  my $passwd = 'test';
  my $group = 'ftpd'; 
  my $home_dir = File::Spec->rel2abs("$tmpdir/users/$user");
  mkpath($home_dir);
  my $uid = 500;
  my $gid = 500;

  my $sub_dir = File::Spec->rel2abs("$home_dir/test.d");
  mkpath($sub_dir);

  if ($^O eq 'darwin') {
    $home_dir = ('/private' . $home_dir);
  }

  # Make sure that, if we're running as root, that the home directory has
  # permissions/privs set for the account we create
  if ($< == 0) {
    unless (chmod(0755, $home_dir, $sub_dir)) {
      die("Can't set perms on $home_dir to 0755: $!");
    }

    unless (chown($uid, $gid, $home_dir, $sub_dir)) {
      die("Can't set owner of $home_dir to $uid/$gid: $!");
    }
  }

  auth_user_write($auth_user_file, $user, $passwd, $uid, $gid, $home_dir,
    '/bin/bash');
  auth_group_write($auth_group_file, $group, $gid, $user);

  my $rsa_host_key = File::Spec->rel2abs("t/etc/modules/mod_sftp/ssh_host_rsa_key");
  my $dsa_host_key = File::Spec->rel2abs("t/etc/modules/mod_sftp/ssh_host_dsa_key");

  my $config = {
    PidFile => $pid_file,
    ScoreboardFile => $scoreboard_file,
    SystemLog => $log_file,
    TraceLog => $log_file,
    Trace => 'directory:20 fileperms:10 ifsession:20',

    AuthUserFile => $auth_user_file,
    AuthGroupFile => $auth_group_file,
    DefaultRoot => '~',

    IfModules => {
      'mod_delay.c' => {
        DelayEngine => 'off',
      },

      'mod_sftp.c' => [
        "SFTPEngine on",
        "SFTPLog $log_file",
        "SFTPHostKey $rsa_host_key",
        "SFTPHostKey $dsa_host_key",
      ],
    },
  };

  my ($port, $config_user, $config_group) = config_write($config_file, $config);

  # Append the mod_ifsession config to the end of the config file
  if (open(my $fh, ">> $config_file")) {
    my ($volume, $parent_dir, $file) = File::Spec->splitpath($home_dir);
    my $limit_dir = File::Spec->rel2abs("$parent_dir/*/test.d");
    if ($^O eq 'darwin') {
      # MacOSX hack
      $limit_dir = ('/private' . $limit_dir);
    }
 
    # XXX Hack, for now:
    $limit_dir = '~/test.d';

    print $fh <<EOC;
<Directory ~>
  <Limit WRITE>
    DenyAll
  </Limit>
</Directory>

<IfGroup $group>
  <Directory $limit_dir>
    <Limit MKD XMKD RMD XRMD>
      DenyAll
    </Limit>

    <Limit WRITE>
      AllowAll
    </Limit>
  </Directory>
</IfGroup>
EOC
    unless (close($fh)) {
      die("Can't write $config_file: $!");
    }

  } else {
    die("Can't open $config_file: $!");
  }

  # Open pipes, for use between the parent and child processes.  Specifically,
  # the child will indicate when it's done with its test by writing a message
  # to the parent.
  my ($rfh, $wfh);
  unless (pipe($rfh, $wfh)) {
    die("Can't open pipe: $!");
  }

  require Net::SSH2;

  my $ex;

  # Ignore SIGPIPE
  local $SIG{PIPE} = sub { };

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

      unless ($ssh2->auth_password($user, $passwd)) {
        my ($err_code, $err_name, $err_str) = $ssh2->error();
        die("Can't login to SSH2 server: [$err_name] ($err_code) $err_str");
      }

      my $sftp = $ssh2->sftp();
      unless ($sftp) {
        my ($err_code, $err_name, $err_str) = $ssh2->error();
        die("Can't use SFTP on SSH2 server: [$err_name] ($err_code) $err_str");
      }

      my $path = 'test.d/test.txt';
      my $fh = $sftp->open($path, O_WRONLY|O_CREAT|O_TRUNC, 0644);
      unless ($fh) {
        my ($err_code, $err_name) = $sftp->error();
        die("Can't open $path: [$err_name] ($err_code)");
      }

      my $count = 20;
      for (my $i = 0; $i < $count; $i++) {
        print $fh "ABCD" x 8192;
      }

      # To issue the FXP_CLOSE, we have to explicitly destroy the filehandle
      $fh = undef;

      $sftp = undef;
      $ssh2->disconnect();
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
    test_append_logfile($log_file, $ex);
    unlink($log_file);

    die($ex);
  }

  unlink($log_file);
}

1;
