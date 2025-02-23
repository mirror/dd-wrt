# dbus release checklist

To make a release of D-Bus, do the following:

 - check out a fresh copy from Git

 - verify that the libtool versioning/library soname in `meson.build` is
   changed if it needs to be, or not changed if not
    - CMake takes the version number from `meson.build` and so should not
      need updating

 - update the file NEWS based on the git history

 - verify that the version number of dbus-specification.xml is
   changed if it needs to be; if changes have been made, update the
   release date in that file

 - update the AUTHORS file with
   `ninja -C ${builddir} maintainer-update-authors`
   if necessary

 - the version number in `meson.build` should have major.minor.micro, even
   if micro is 0, i.e. "1.0.0" and "1.2.0" not "1.0"/"1.2"; the micro
   version should be even for releases, and odd for intermediate snapshots
    - CMake takes the version number from `meson.build` and so should not
      need updating

 - When ready to release, `git commit -a`.  This is the version
   of the tree that corresponds exactly to the released tarball.

 - `meson dist -C ${builddir)`
    (this is the equivalent of Autotools `make distcheck`)

 - if `meson dist` failed, fix it, commit, retry until successful

 - tag the tree with `git tag -s -m 'Released X.Y.Z' dbus-X.Y.Z`
   where X.Y.Z is the version of the release.  If you can't sign
   then simply created an unsigned annotated tag:
   `git tag -a -m 'Released X.Y.Z' dbus-X.Y.Z`.

 - bump the version number up in `meson.build`
   again (so the micro version is odd),
   and commit it.  Make sure you do this *after* tagging the previous
   release! The idea is that git has a newer version number
   than anything released. Similarly, bump the version number of
   dbus-specification.xml and set the release date to "(not finalized)".

 - push your changes and the tag to the central repository with
     `git push origin master dbus-X.Y dbus-X.Y.Z`

 - scp your tarball to freedesktop.org server and copy it to
   `dbus.freedesktop.org:/srv/dbus.freedesktop.org/www/releases/dbus/`.
   This should be possible if you're in group "dbus"

 - Update the online documentation with
     `ninja -C ${builddir} maintainer-upload-docs`.

 - If `doc/busconfig.dtd` and/or `doc/introspect.dtd` have changed, send
   a merge request to `xdg-specs` similar to
   <https://gitlab.freedesktop.org/xdg/xdg-specs/90> to get the new
   version published

 - post to dbus@lists.freedesktop.org announcing the release.

## Making a ".0" stable release

We create a branch for each stable release. The branch name should be
dbus-X.Y which is a branch that has releases versioned X.Y.Z;
changes on a stable branch should be limited to significant bug fixes.

Because we won't make minor changes like keeping up with the latest
deprecations on a stable branch, stable branches should turn off the
gcc warning for deprecated declarations (e.g. see commit 76a68867).

Be extra-careful not to merge master (or any branch based on master) into a
stable branch.

To branch:

    git branch dbus-X.Y

and upload the branch tag to the server:

    git push origin dbus-X.Y

To develop in this branch:

    git checkout dbus-X.Y

After starting a new stable branch:

 - update the wiki page <https://www.freedesktop.org/wiki/Software/dbus> by
   adding the new release under the Download heading
