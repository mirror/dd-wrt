// Copyright 2020, 2022 The Godror Authors
//
// SPDX-License-Identifier: UPL-1.0 OR Apache-2.0

package godror

// https://github.com/oracle/odpi/archive/refs/heads/main.zip
//go:generate bash -c "echo 4.4.1>odpi-version; set -x; curl -L https://github.com/oracle/odpi/archive/refs/tags/v$(cat odpi-version).tar.gz | tar xzvf - odpi-$(cat odpi-version)/{embed,include,src,CONTRIBUTING.md,README.md,LICENSE.txt} && cp -a odpi/embed/require.go odpi-$(cat odpi-version)/embed/ && cp -a odpi/include/require.go odpi-$(cat odpi-version)/include/ && cp -a odpi/src/require.go odpi-$(cat odpi-version)/src/ && rm -rf odpi && mv odpi-$(cat odpi-version) odpi; rm -f odpi-{,v}version; git status --porcelain -- odpi/*/*.go | sed -n -e '/^ D / { s/^ D //;p;}' | xargs -r git checkout -- "
// go:generate bash -c "echo main>odpi-version; set -x; curl -L https://github.com/oracle/odpi/archive/refs/heads/main.tar.gz | tar xzvf - odpi-$(cat odpi-version)/{embed,include,src,CONTRIBUTING.md,README.md,LICENSE.txt} && cp -a odpi/embed/require.go odpi-$(cat odpi-version)/embed/ && cp -a odpi/include/require.go odpi-$(cat odpi-version)/include/ && cp -a odpi/src/require.go odpi-$(cat odpi-version)/src/ && rm -rf odpi && mv odpi-$(cat odpi-version) odpi; rm -f odpi-{,v}version; git status --porcelain -- odpi/*/*.go | sed -n -e '/^ D / { s/^ D //;p;}' | xargs -r git checkout -- "

// Version of this driver
var Version = "v0.34.0"
