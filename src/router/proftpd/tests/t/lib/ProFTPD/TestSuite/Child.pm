package ProFTPD::TestSuite::Child;

use strict;

use POSIX qw(:sys_wait_h);

my $processes = {};

sub sig_chld {
  my $child;

  while (($child = waitpid(-1, 0)) > 0) {
    $processes->{$child} = ($? >> 8);
  }

  $SIG{CHLD} = \&sig_chld;
}

sub handle_sigchld {
  my $self = shift;
  $SIG{CHLD} = \&sig_chld;
}

sub assert_child_ok {
  my $self = shift;
  my $pid = shift;

  my ($pkg, $file, $lineno, $func, @rest) = caller(1);

  $self->assert($processes->{$pid} == 0,
    "Child test process $pid failed in $func (line $lineno) [see above for possible errors]");
}

1;
