#!/usr/bin/perl

#
# SPDX-License-Identifier: GPL-2.0-or-later
#
# Copyright (C) 2019 Samsung Electronics Co., Ltd.
#

use strict;

sub tweak_sysctl()
{
	`sudo sysctl kernel.hardlockup_panic=0`;
	`sudo sysctl kernel.hung_task_panic=0`;
	`sudo sysctl kernel.panic=128`;
	`sudo sysctl kernel.panic_on_io_nmi=0`;
	`sudo sysctl kernel.panic_on_oops=0`;
	`sudo sysctl kernel.panic_on_rcu_stall=0`;
	`sudo sysctl kernel.panic_on_unrecovered_nmi=0`;
	`sudo sysctl kernel.panic_on_warn=0`;
	`sudo sysctl kernel.softlockup_panic=0`;
	`sudo sysctl kernel.unknown_nmi_panic=0`;
}

sub execute($$)
{
	my $cmd = shift;
	my $timeout = shift;
	my $output = "Timeout";
	my $status = 1;

	$timeout = 8 * 60 if (!defined $timeout);

	tweak_sysctl();

	eval {
		local $SIG{ALRM} = sub {
			print "TIMEOUT:\n";
			system("top -n 1"), print "top\n";
			system("free"), print "free\n";
			system("dmesg"), print "dmesg\n";
			die "Timeout\n";
		};

		print "Executing $cmd with timeout $timeout\n";

		alarm $timeout;
		$output = `$cmd`;
		$status = $?;
		alarm 0;
		print $output."\n";
		print "Finished: status $status\n";
	};

	if ($@) {
		die unless $@ eq "Timeout\n";
	}
}

if (! defined $ARGV[0]) {
	print "Usage:\n\t./.travis_cmd_wrapper.pl command [timeout seconds]\n";
	exit 1;
}

execute($ARGV[0], $ARGV[1]);
