[![Travis CI Linux Build Status](https://travis-ci.org/lsof-org/lsof.svg?branch=master)](https://travis-ci.org/lsof-org/lsof)
[![Coveralls Linux Coverage Status on Travis CI](https://coveralls.io/repos/github/lsof-org/lsof/badge.svg?branch=master)](https://coveralls.io/github/lsof-org/lsof?branch=master)

# lsof
The lsof-org team at GitHub takes over the maintainership of lsof
originally maintained by Vic Abell. This repository is for maintaining
the final source tree of lsof inherited from Vic. "legacy" branch
keeps the original source tree. We will not introduce any changes to
the "legacy" branch. This branch is just for reference.

"master" branch is used for maintenance. Bug fixes and enhancements go
to "master" branch.

lsof has supported many OSes. A term "dialect" represents code for
supporting a OSes. Because of limitted resources, we will maintain the
part of them. The current status of maintaince is as follows:

<dl>
<dt>freebsd</dt>
<dd>partially maintained, but testing on Cirrus CI is temporary disabled</dd>
<dt>linux</dt>
<dd>fully maintained, and tested on Travis CI</dd>
<dt>darwin</dt>
<dd>not maintained, but partially tested on Travis CI</dd>
</dl>

If you are interested in maintaining a dialect, let us know via the
issue tracker of GitHub (https://github.com/lsof-org/lsof).  If
we cannot find a volunteer for a dialect, we will remove the dialect.

Many texts in the source tree still refers purdue.edu as the home of
lsof development. It should be https://github.com/lsof-org/lsof, the
new home. The updating is in progress.

We ran another repository, lsof-org/"lsof-linux" derived from
lsof-4.91 that was also released by Vic. The repository is no more
used; all the changes made in the repository are now in lsof-org/"lsof"
repository.

The lsof-org team at GitHub
