#!/usr/bin/perl -w
use strict;

# Test that ns filecaps work.

my $pid = fork;
if (!$pid) {
    # child
    print "child\n";
    exit 0;
}

# parent
my $res = waitpid($pid, 0);
print "$res returned $?\n";
exit 0;

my $notes = <<EOT
Email notes from Chritian and Serge:
> > Is there some simple way/documentation-how to launch a namespace? I'd like
> > to augment progs/quicktest.sh with some code to do that before releasing
> > this. I'm running kernel 4.17.19 so I think my kernel should be new enough
> 
> add
> 
> <your-username>:100000:1000000000
> 
> to /etc/subuid and /etc/subgid. Create the files if the don't exist.
> Make sure that you have shadow installed with newuidmap and newuidmap
> and the the setuid bit set. Then do:
> 
> 1. create new user namespace
> unshare -U --setgroups=allow
> echo $$?
> 
> 2. switch to another shell and execute
> 
> newuidmap <pid-taken-from-above> 0 100000 1000000
> newgidmap <pid-taken-from-above> 0 100000 1000000
> 
> 3. Switch to the unshare shell
> 3.1 Verify that the mappings are correct
> 
> cat /proc/self/uid_map
> and
> cat /proc/self/gid_map
> 
> both should show:
> 
> 0 100000 1000000
> 
> 3.2 Set a namespaced file capability from inside that namespace
> 
> 4. Switch to another shell and do:
> 
> nsenter --target <pid-taken-from-above> --setuid 1000 --setgid 1000
> 
> and execute the binary with the namespaced file capability set. It
> should work.

...  or just "lxc-usernsexec -m b:0:100000:1000000 yourprog"
EOT
