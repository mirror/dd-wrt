// Copyright 2020, 2025 The Godror Authors
//
// SPDX-License-Identifier: UPL-1.0 OR Apache-2.0

package godror

import (
	"runtime/debug"
	"strconv"
)

// https://github.com/oracle/odpi/archive/refs/heads/main.zip
//go:generate bash -c "echo 5.5.1>odpi-version; set -x; curl -L https://github.com/oracle/odpi/archive/refs/tags/v$(cat odpi-version).tar.gz | tar xzvf - odpi-$(cat odpi-version)/{embed,include,src,CONTRIBUTING.md,README.md,LICENSE.txt} && cp -a odpi/embed/require.go odpi-$(cat odpi-version)/embed/ && cp -a odpi/include/require.go odpi-$(cat odpi-version)/include/ && cp -a odpi/src/require.go odpi-$(cat odpi-version)/src/ && rm -rf odpi && mv odpi-$(cat odpi-version) odpi; rm -f odpi-{,v}version; git status --porcelain -- odpi/*/*.go | sed -n -e '/^ D / { s/^ D //;p;}' | xargs -r git checkout -- "
// go : generate bash -c "echo main>odpi-version; set -x; curl -L https://github.com/oracle/odpi/archive/refs/heads/main.tar.gz | tar xzvf - odpi-$(cat odpi-version)/{embed,include,src,CONTRIBUTING.md,README.md,LICENSE.txt} && cp -a odpi/embed/require.go odpi-$(cat odpi-version)/embed/ && cp -a odpi/include/require.go odpi-$(cat odpi-version)/include/ && cp -a odpi/src/require.go odpi-$(cat odpi-version)/src/ && rm -rf odpi && mv odpi-$(cat odpi-version) odpi; rm -f odpi-{,v}version; git status --porcelain -- odpi/*/*.go | sed -n -e '/^ D / { s/^ D //;p;}' | xargs -r git checkout -- "

// Version of this driver
func init() {
	if info, ok := debug.ReadBuildInfo(); ok && info != nil {
		for _, m := range info.Deps {
			if m == nil || m.Path != "github.com/godror/godror" {
				continue
			}
			for m.Replace != nil {
				m = m.Replace
			}
			if m.Version != "" {
				Version = m.Version + "+ODPI-" + odpiVersion
			}
			break
		}
	}
}

var (
	godrorVersion = "v0.48.0"

	odpiVersion = strconv.Itoa(DpiMajorVersion) +
		"." + strconv.Itoa(DpiMinorVersion) +
		"." + strconv.Itoa(DpiPatchLevel)

	Version = godrorVersion + "+ODPI-" + odpiVersion
)
