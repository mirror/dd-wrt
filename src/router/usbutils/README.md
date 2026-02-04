<!---
SPDX-License-Identifier: GPL-2.0-or-later
Copyright (c) 2018 Greg Kroah-Hartman <gregkh@linuxfoundation.org>
-->
# usbutils

This is a collection of USB tools for use on Linux and BSD systems to
query what type of USB devices are connected to the system.  This is to
be run on a USB host (i.e. a machine you plug USB devices into), not on
a USB device (i.e. a device you plug into a USB host.)

## Building and installing

Note, usbutils depends on libusb and libudev, be sure that libraries are
properly installed first.

To work with the "raw" repo, after cloning it just do:

	./autogen.sh

which will build everything and place the binaries into the `build/`
subdirectory.

usbutils uses meson to build, so if you wish to just build by hand you can do:

	meson setup build
	cd build/
	meson compile

## Source location

The source for usbutils can be found in many places, depending on the git
hosting site you prefer.  Here are the primary locations, in order of
preference by the maintainer:

    https://git.kernel.org/pub/scm/linux/kernel/git/gregkh/usbutils.git/
    https://git.sr.ht/~gregkh/usbutils
    https://github.com/gregkh/usbutils

## Contributing

If you have patches or suggestions, you can submit them either via email
[to the maintainer] or [to the linux-usb mailing list] or via a [pull request].

Please note that commits must include a `Signed-off-by` trailer, indicating that
you comply with the [Developer Certificate of Origin v1.1].

In addition, when adding new files or contributing to existing ones, ensure
that the SPDX tags `SPDX-FileCopyrightText` and `SPDX-License-Identifier` are
available and are kept up-to date.  You can learn more and do that via
[reuse-tool].

[to the maintainer]: mailto:gregkh@linuxfoundation.org
[to the linux-usb mailing list]: mailto:linux-usb@vger.kernel.org
[pull request]: https://github.com/gregkh/usbutils/pulls
[Developer Certificate of Origin v1.1]: https://developercertificate.org/
[reuse-tool]: https://github.com/fsfe/reuse-tool
