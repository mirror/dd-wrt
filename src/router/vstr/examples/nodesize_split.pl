#! /usr/bin/perl -w

use strict;

use FileHandle;

my $prefix = shift;

# <type> <len> <num> <sz> <xtra> <speed> <mem>

my %fields = (type  => 0, len   => 1, num => 2, sz        => 3,
	      extra => 4, speed => 5, mem => 6, unusedmem => 7);

my $store = {};

sub print_data
{
	my $a   = shift;
	my $key = shift;

	open (OUT, "> $prefix-$key.csv") || die ("open: $!\n");

	splice(@$a, 2, 3);
	splice(@$a, 0, 1);

	while (scalar(@$a) > 4)
	{
		OUT->print(join " ", splice(@$a, 0, 4));
        	OUT->print("\n");
       	}
}

while (<>)
{
	my @vals = split;

	if (scalar(@vals) != 8) { next; }

	my $key = join "_", ($vals[$fields{num}], $vals[$fields{extra}],
                             $vals[$fields{sz}],  $vals[$fields{type}]);
	my $a = $store->{$key};

	if (!defined (@$a))
	{ # First time...
		$a = $store->{$key} = [];

		push(@$a, @vals);
		next;
	}

	# Same as previous...
	push(@$a, @vals[$fields{len}, $fields{speed},
			$fields{mem}, $fields{unusedmem}]);
}

for (keys %$store)
{
	my $a = $store->{$_};

	if (!scalar(@$a)) { next; }

	print_data($a, $_);
}

