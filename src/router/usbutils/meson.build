# SPDX-License-Identifier: GPL-2.0-only
# Copyright (c) 2024 Greg Kroah-Hartman <gregkh@linuxfoundation.org>

project(
  'usbutils',
  'c',
  version: '019',
  license: ['GPL-2.0-only', 'GPL-2.0-or-later','GPL-2.0-or-later'],
  meson_version : '>=0.60.0',
  default_options : ['c_std=gnu99', 'warning_level=2', 'prefix=/usr']
)

pkg = import('pkgconfig')

# Compiler stuff
cc = meson.get_compiler('c')

add_project_arguments(
  cc.get_supported_arguments([
    '-Wbad-function-cast',
    # should be removed and the code fixed
    '-Wno-cast-align',
    '-Wchar-subscripts',
    '-Wempty-body',
    '-Wformat',
    '-Wformat=2',
    '-Wformat-nonliteral',
    '-Wformat-security',
    '-Wformat-y2k',
    '-Winit-self',
    '-Winline',
    '-Wint-conversion',
    '-Wmissing-declarations',
    '-Wmissing-format-attribute',
    '-Wmissing-include-dirs',
    '-Wmissing-prototypes',
    '-Wmissing-noreturn',
    '-Wnested-externs',
    '-Wold-style-definition',
    '-Wold-style-declaration',
    '-Wpointer-arith',
    '-Wredundant-decls',
    '-Wshadow',
    '-Wsign-compare',
    #'-Wsuggest-attribute=const',
    #'-Wsuggest-attribute=noreturn',
    #'-Wsuggest-attribute=malloc',
    #'-Wsuggest-attribute=returns_nonnull',
    '-Wstrict-prototypes',
    '-Wswitch',
    '-Wtype-limits',
    '-Wundef',
    '-Wuninitialized',
    '-Wunused',
    '-Wunused-variable',
    '-Wvla',
    '-Wwrite-strings',
    '-fdiagnostics-color=auto',
    # warnings disabled on purpose
    '-Wno-unused-parameter',
    '-Wno-unused-function',
    '-Wno-deprecated-declarations',
  ]),
  language: 'c',
)

# Configuration information
config = configuration_data()
config.set_quoted('PACKAGE_NAME', meson.project_name())
config.set_quoted('VERSION', meson.project_version())
config_h = configure_file(output: 'config.h', configuration: config)

add_project_arguments('-include', 'config.h', language : 'c')


#####################
# man page generation
#####################
install_man(['man/lsusb.8', 'man/lsusb.py.1', 'man/usb-devices.1', 'man/usbhid-dump.8'])


##########################
# lsusb build instructions
##########################
lsusb_sources = [
  'desc-defs.c',
  'desc-defs.h',
  'desc-dump.c',
  'desc-dump.h',
  'lsusb-t.c',
  'lsusb.c',
  'lsusb.h',
  'names.c',
  'names.h',
  'sysfs.c',
  'sysfs.h',
  'usb-spec.h',
  'usbmisc.c',
  'usbmisc.h',
  'ccan/check_type/check_type.h',
  'ccan/config.h',
  'ccan/container_of/container_of.h',
  'ccan/str/str_debug.h',
  'ccan/str/str.h',
  'ccan/list/list.h',
]

libudev = dependency('libudev', version: '>= 196')
libusb = dependency('libusb-1.0', version: '>= 1.0.22')

executable('lsusb', lsusb_sources, dependencies: [libusb, libudev], install: true)

################################
# usbhid-dump build instructions
################################
usbhid_sources = [
  'usbhid-dump/dev.c',
  'usbhid-dump/dev.h',
  'usbhid-dump/dev_list.c',
  'usbhid-dump/dev_list.h',
  'usbhid-dump/iface.c',
  'usbhid-dump/iface.h',
  'usbhid-dump/iface_list.c',
  'usbhid-dump/iface_list.h',
  'usbhid-dump/misc.h',
  'usbhid-dump/usbhid-dump.c',
]

executable('usbhid-dump', usbhid_sources, dependencies: libusb, install: true)

##############################
# usbreset build instructions
##############################
usbreset_sources = [
  'usbreset.c'
]

# By default, usbreset does not get installed as it could cause problems, it's
# in the repo for those that wish to try it out.
executable('usbreset', usbreset_sources, install: false)


################################
# usb-devices build instructions
################################
usb_devices_sources = [
  'usb-devices'
]
# Hack as this is not something to compile, but rather we can just copy it.
install_data(usb_devices_sources, install_dir: get_option('bindir'), install_mode: 'rwxr-xr-x')


#############################
# lsusb.py build instructions
#############################
# Also a hack, like was done for usb-devices, as this is "just" a script and
# doesn't need to be compiled.
install_data(files('lsusb.py'), install_dir: get_option('bindir'), install_mode: 'rwxr-xr-x')
