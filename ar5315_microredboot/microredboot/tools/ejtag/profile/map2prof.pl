#!/usr/bin/perl
while ($line = <STDIN>) {
    ($addr,$type,$name)=split(/ /, $line);
	print "$addr $name";
}

