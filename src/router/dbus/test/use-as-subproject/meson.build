# Copyright 2023 Collabora Ltd.
# Copyright (c) 2023 SUSE LLC
# SPDX-License-Identifier: MIT

project(
    'use-libdbus-as-subproject',
    'c',
    version : '0',
    meson_version : '>=0.49.0'
)
add_project_arguments('-D_GNU_SOURCE', language : 'c')

configure_file(
    copy : true,
    input : 'dummy-config.h.in',
    output : 'config.h',
)

libdbus_dep = dependency(
    'dbus-1',
    required: true,
    fallback: ['dbus', 'libdbus_dep'],
    default_options: [
      'default_library=static',
      'intrusive_tests=false',
      'message_bus=false',
      'modular_tests=disabled',
      'tools=false',
    ],
)

executable(
    'use-libdbus',
    'use-libdbus.c',
    dependencies : [
        libdbus_dep
    ]
)
