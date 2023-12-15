[![Circle CI](https://circleci.com/gh/lsof-org/lsof.svg?style=svg)](https://circleci.com/gh/lsof-org/lsof)
[![Cirrus CI](https://img.shields.io/cirrus/github/lsof-org/lsof)](https://cirrus-ci.com/github/lsof-org/lsof)
[![builds.sr.ht status](https://builds.sr.ht/~jiegec/lsof.svg)](https://builds.sr.ht/~jiegec/lsof?)
[![Read the Docs](https://readthedocs.org/projects/lsof/badge/?version=latest)](https://lsof.readthedocs.io/en/latest/)

# lsof

[lsof](https://en.wikipedia.org/wiki/Lsof) is a command listing open files.

## How it works:

```
$ cat > /tmp/LOG &
cat > /tmp/LOG &
[1] 18083
$ lsof -p 18083
lsof -p 18083
COMMAND   PID   USER   FD   TYPE DEVICE  SIZE/OFF     NODE NAME
cat     18083 yamato  cwd    DIR   0,44      1580 43460784 /tmp/lsof
cat     18083 yamato  rtd    DIR  253,2      4096        2 /
cat     18083 yamato  txt    REG  253,2     47432   678364 /usr/bin/cat
cat     18083 yamato  mem    REG  253,2 111950656   681778 /usr/lib/locale/locale-archive
cat     18083 yamato  mem    REG  253,2   2119256   679775 /usr/lib64/libc-2.27.so
cat     18083 yamato  mem    REG  253,2    187632   655943 /usr/lib64/ld-2.27.so
cat     18083 yamato  mem    REG  253,2     26370   662532 /usr/lib64/gconv/gconv-modules.cache
cat     18083 yamato  mem    REG  253,2      3316  1578981 /usr/lib/locale/en_US.utf8/LC_TIME
cat     18083 yamato    0u   CHR  136,3       0t0        6 /dev/pts/3
cat     18083 yamato    1w   REG   0,44         0 54550934 /tmp/LOG
cat     18083 yamato    2u   CHR  136,3       0t0        6 /dev/pts/3
```

Read the documentation at [lsof.readthedocs.io](https://lsof.readthedocs.io/)

# lsof-org at GitHub

The lsof-org team at GitHub takes over the maintainership of lsof originally
developed and maintained by Vic Abell. This repository is for maintaining the
final source tree of lsof inherited from Vic. "legacy" branch keeps the original
source tree. We will not introduce any changes to the "legacy" branch. This
branch is just for reference.

"master" branch is used for maintenance. Bug fixes and enhancements go to
"master" branch.

lsof had supported many OSes. A term "dialect" represents code for supporting
OSes. Because of limited resources, we will maintain the part of them. The
current status of maintenance is as follows:

<dl>
<dt>FreeBSD</dt>
<dd>fully maintained and tested on Cirrus CI</dd>
<dt>Linux</dt>
<dd>fully maintained, and tested on Circle CI</dd>
<dt>Darwin</dt>
<dd>fully maintained, and tested on Circle CI</dd>
<dt>NetBSD</dt>
<dd>fully maintained, and tested on SourceHut CI</dd>
<dt>OpenBSD</dt>
<dd>fully maintained, and tested on SourceHut CI</dd>
<dt>Solaris/OpenIndiana</dt>
<dd>fully maintained</dd>
</dl>

If you are interested in maintaining a dialect, let us know via the issue
tracker of GitHub (https://github.com/lsof-org/lsof/issues). If we cannot find a
volunteer for a dialect, we will remove the dialect.

Many texts in the source tree still refers purdue.edu as the home of lsof
development. It should be https://github.com/lsof-org/lsof, the new home. The
updating is in progress.

We ran another repository, lsof-org/"lsof-linux" derived from lsof-4.91 that was
also released by Vic. The repository is no more used; all the changes made in
the repository are now in lsof-org/"lsof" repository.

The lsof-org team at GitHub
