//go:build linux && !darwin
// +build linux,!darwin

// Does not work on MacOS (clang): Issues #148. #149

// Copyright 2017, 2020 The Godror Authors
//
//
// SPDX-License-Identifier: UPL-1.0 OR Apache-2.0

package godror

/*
#cgo LDFLAGS: -ldl -lpthread

// https://github.com/godror/godror/issues/139
// https://www.win.tue.nl/~aeb/linux/misc/gcc-semibug.html
__asm__(".symver memcpy,memcpy@GLIBC_2.2.5");
*/
import "C"
