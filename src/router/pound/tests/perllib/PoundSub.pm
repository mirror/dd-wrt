# This file is part of pound testsuite
# Copyright (C) 2024-2025 Sergey Poznyakoff
#
# Pound is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 3 of the License, or
# (at your option) any later version.
#
# Pound is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with pound.  If not, see <http://www.gnu.org/licenses/>.

package PoundSub;
use strict;
use warnings;
use Carp;
use POSIX ":sys_wait_h";
eval "require threads";
my $threads_ok = !$@ && ($ENV{POUNDSUB_FORK} // 0) == 0;

my %pidreg;

sub using_threads { return $threads_ok };

sub start {
    my ($class, $code, @args) = @_;

    if (using_threads()) {
	threads->create(sub { &{$code}(@args); })->detach();
    } else {
	my $pid = fork();
	croak "fork: $!" unless defined $pid;
	if ($pid == 0) {
	    local %SIG;
	    $SIG{CHLD} = sub { while (($pid = waitpid(-1, WNOHANG)) > 0) {}; };
	    &{$code}(@args);
	    POSIX::_exit(0);
	}
	$pidreg{$pid} = $pid;
    }
}

sub exit {
    if ($threads_ok) {
	threads->exit();
    } else {
	exit(1);
    }
}

sub stop {
    kill 9, (keys %pidreg);
}

1;
