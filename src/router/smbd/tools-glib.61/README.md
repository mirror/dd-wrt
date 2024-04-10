# ksmbd-tools

[![Build Status](https://app.travis-ci.com/cifsd-team/ksmbd-tools.svg?branch=master)](https://app.travis-ci.com/cifsd-team/ksmbd-tools)
[![License](https://img.shields.io/badge/License-GPL_v2-blue.svg)](https://www.gnu.org/licenses/old-licenses/gpl-2.0.en.html)

[ksmbd-tools](https://github.com/cifsd-team/ksmbd-tools)
is a collection of userspace utilities for
[the ksmbd kernel server](https://www.kernel.org/doc/html/latest/filesystems/cifs/ksmbd.html)  
merged to mainline in the Linux 5.15 release.

## Table of Contents

- [Building and Installing](#building-and-installing)
- [Usage](#usage)
- [Packages](#packages)

## Building and Installing

You should first check if your distribution has a package for ksmbd-tools,  
and if that is the case, consider installing it from the package manager.  
Otherwise, follow these instructions to build it yourself. Either the GNU  
Autotools or Meson build system can be used.

Dependencies for Debian and its derivatives: `git` `gcc` `pkgconf` `autoconf`  
`automake` `libtool` `make` `meson` `ninja-build` `gawk` `libnl-3-dev`  
`libnl-genl-3-dev` `libglib2.0-dev`

Dependencies for RHEL and its derivatives: `git` `gcc` `pkgconf` `autoconf`  
`automake` `libtool` `make` `meson` `ninja-build` `gawk` `libnl3-devel`  
`glib2-devel`

Example build and install:
```sh
git clone https://github.com/cifsd-team/ksmbd-tools.git
cd ksmbd-tools

# autotools build

./autogen.sh
./configure --with-rundir=/run

make
sudo make install

# meson build

mkdir build
cd build
meson -Drundir=/run ..

ninja
sudo ninja install
```

By default, the utilities are in `/usr/local/sbin` and the files they use by  
default are under `/usr/local/etc` in the `ksmbd` directory.

If you would like to install ksmbd-tools under `/usr`, where it may conflict  
with ksmbd-tools installed using the package manager, give `--prefix=/usr`  
and `--sysconfdir=/etc` as options to `configure` or `meson`. In that case,  
the utilities are in `/usr/sbin` and the files they use by default are under  
`/etc` in the `ksmbd` directory.

It is likely that you should give `--with-rundir` or `-Drundir` as an option  
to `configure` or `meson`, respectively. This is due to it being likely that  
your system does not mount a tmpfs filesystem at the directory given by the  
default value. Common choices are `/run`, `/var/run`, or `/tmp`. ksmbd-tools  
uses the directory for per-process modifiable data, namely the `ksmbd.lock`  
file holding the PID of the `ksmbd.mountd` manager process. If your autoconf  
supports it, you may instead choose to give `--runstatedir` to `configure`.

If you have systemd and it meets at least the minimum version required, the  
build will install the `ksmbd.service` unit file. The unit file supports the  
usual unit commands and handles loading of the kernel module. Note that the  
location of the unit file may conflict with ksmbd-tools installed using the  
package manager. You can bypass the version check and choose the unit file  
directory yourself by giving `--with-systemdsystemunitdir=DIR` or  
`-Dsystemdsystemunitdir=DIR` as an option to either `configure` or `meson`,  
respectively.

## Usage

Manual pages:
```sh
man 8 ksmbd.addshare
man 8 ksmbd.adduser
man 8 ksmbd.control
man 8 ksmbd.mountd
man 5 ksmbd.conf
man 5 ksmbdpwd.db
```

Example session:
```sh
# If you built and installed ksmbd-tools yourself using autoconf defaults,
# the utilities are in `/usr/local/sbin',
# the default user database is `/usr/local/etc/ksmbd/ksmbdpwd.db', and
# the default configuration file is `/usr/local/etc/ksmbd/ksmbd.conf'.

# Otherwise it is likely that,
# the utilities are in `/usr/sbin',
# the default user database is `/etc/ksmbd/ksmbdpwd.db', and
# the default configuration file is `/etc/ksmbd/ksmbd.conf'.

# Create the share path directory.
# The share stores files in this directory using its underlying filesystem.
mkdir -vp $HOME/MyShare

# Add a share to the default configuration file.
# Note that `ksmbd.addshare' does not do variable expansion.
# Without `--add', `ksmbd.addshare' will update `MyShare' if it exists.
sudo ksmbd.addshare --add \
                    --option "path = $HOME/MyShare" \
                    --option 'read only = no' \
                    MyShare

# The default configuration file now has a new section for `MyShare'.
#
# [MyShare]
#         ; share parameters
#         path = /home/tester/MyShare
#         read only = no
#
# Each share has its own section with share parameters that apply to it.
# A share parameter given in `[global]' changes its default value.
# `[global]' also has global parameters which are not share specific.

# You can interactively update a share by omitting `--option'.
# Without `--update', `ksmbd.addshare' will add `MyShare' if it does not exist.
sudo ksmbd.addshare --update MyShare

# Add a user to the default user database.
# You will be prompted for a password.
sudo ksmbd.adduser --add MyUser

# There is no system user called `MyUser' so it has to be mapped to one.
# We can force all users accessing the share to map to a system user and group.

# Update share parameters of a share in the default configuration file.
sudo ksmbd.addshare --update \
                    --option "force user = $USER" \
                    --option "force group = $USER" \
                    MyShare

# The default configuration file now has the updated share parameters.
#
# [MyShare]
#         ; share parameters
#         force group = tester
#         force user = tester
#         path = /home/tester/MyShare
#         read only = no
#

# Add the kernel module.
sudo modprobe ksmbd

# Start the user and kernel mode daemons.
# All interfaces are listened to by default.
sudo ksmbd.mountd

# Mount the new share with cifs-utils and authenticate as the new user.
# You will be prompted for the password given previously with `ksmbd.adduser'.
sudo mount -o user=MyUser //127.0.0.1/MyShare /mnt

# You can now access the share at `/mnt'.
sudo touch /mnt/new_file_from_cifs_utils

# Unmount the share.
sudo umount /mnt

# Update the password of a user in the default user database.
# `--password' can be used to give the password instead of prompting.
sudo ksmbd.adduser --update --password MyNewPassword MyUser

# Delete a user from the default user database.
sudo ksmbd.adduser --delete MyUser

# The utilities notify ksmbd.mountd of changes by sending it the SIGHUP signal.
# This can be done manually when changes are made without using the utilities.
sudo ksmbd.control --reload

# Toggle ksmbd debug printing of the `smb' component.
sudo ksmbd.control --debug smb

# Some changes require restarting the user and kernel mode daemons.
# Modifying any global parameter is one example of such a change.
# Restarting means starting `ksmbd.mountd' after shutting the daemons down.

# Shutdown the user and kernel mode daemons.
sudo ksmbd.control --shutdown

# Remove the kernel module.
sudo modprobe -r ksmbd
```

## Packages

The following packaging status tracker is provided by
[the Repology project](https://repology.org)
.

[![Packaging status](https://repology.org/badge/vertical-allrepos/ksmbd-tools.svg)](https://repology.org/project/ksmbd-tools/versions)
