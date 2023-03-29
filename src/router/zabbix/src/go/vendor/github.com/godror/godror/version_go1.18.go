//go:build go1.18
// +build go1.18

// Copyright 2020 The Godror Authors
//
// SPDX-License-Identifier: UPL-1.0 OR Apache-2.0

package godror

import (
	"runtime/debug"
)

// Version of this driver
func init() {
	if info, ok := debug.ReadBuildInfo(); ok {
		for _, m := range info.Deps {
			if m == nil || m.Path != "github.com/godror/godror" {
				continue
			}
			for m.Replace != nil {
				m = m.Replace
			}
			if m.Version != "" {
				Version = m.Version
			}
			break
		}
	}
}
