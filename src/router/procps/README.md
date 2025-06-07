[![pipeline status](https://gitlab.com/procps-ng/procps/badges/master/pipeline.svg)](https://gitlab.com/procps-ng/procps/-/commits/master)
[![Latest Release](https://gitlab.com/procps-ng/procps/-/badges/release.svg)](https://gitlab.com/procps-ng/procps/-/releases)

procps
======

procps is a set of command line and full-screen utilities that provide
information out of the pseudo-filesystem most commonly located at /proc.
This filesystem provides a simple interface to kernel data structures.
The utilities provided by procps focus on the structures that describe
processess running on the system.

The following utilities are provided by procps:
* *free* - Report the amounts of free and used memory in the system
* *hugetop* - Report hugepage usage of processes and the system as a whole
* *kill* - Send a signal to a process based on PID
* *pgrep* - List processes based on name or other attributes
* *pkill* - Send a signal to a process based on name or other attributes
* *pmap* - Report the memory map of a process
* *ps* - Report process information including PID and resource usage
* *pwdx* - Report the current working directory of a process
* *skill* - Obsolete version of pgrep/pkill
* *slabtop* - Display kernel slab cache information in real time
* *snice* - Renice a process
* *sysctl* - Read or write kernel parameters at run-time
* *tload* - Graphical representation of system load average
* *top* - Dynamic real-time view of running processes
* *uptime* - Display how long the system has been running
* *vmstat* - Report virtual memory statistics
* *w* - Report logged in users and what they are doing
* *watch* - Execute a program periodically, showing output fullscreen

## Reporting Bugs
There are three ways to submit bugs or feature requests:

1. Your Linux distribution's bug reporter. If you are using Linux distribution
packages your first port of call is their bug tracker. This is because each
distribution has their own patches and way of dealing with bugs. Also bug
reporting often does not need any subscription to websites.
2. GitLab Issues - To the left of this page is the issue tracker. You can report
bugs here.
3. Email list - We have an email list (see below) where you can report bugs.
A shortcoming of this method is that such bug reports often get lost and cannot
be tracked. This is especially problematic when the report conerns something
that will take time to resolve.

If you need to report bugs, there is more details on the
[Bug Reporting](https://gitlab.com/procps-ng/procps/-/blob/master/doc/bugs.md)
page.

## Email List
The email list for the developers and users of procps is found at
http://www.freelists.org/archive/procps/
This email list discusses the development of procps and is used by distributions
to also forward or discuss bugs.
