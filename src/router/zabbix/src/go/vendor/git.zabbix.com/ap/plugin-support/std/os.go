/*
** Zabbix
** Copyright 2001-2022 Zabbix SIA
**
** Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated
** documentation files (the "Software"), to deal in the Software without restriction, including without limitation the
** rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to
** permit persons to whom the Software is furnished to do so, subject to the following conditions:
**
** The above copyright notice and this permission notice shall be included in all copies or substantial portions
** of the Software.
**
** THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
** WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
** COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
** TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
** SOFTWARE.
**/

// package std is used to create wrappers for standard Go functions to support
// mocking in tests where necessary
package std

import (
	"io"
	"os"
)

// File interface is used to mock os.File structure
type File interface {
	io.Reader
	io.Closer
}

// Os interface is used to mock os package
type Os interface {
	Open(name string) (File, error)
	Stat(name string) (os.FileInfo, error)
	IsExist(err error) bool
}

// A FileMode represents a file's mode and permission bits.
type FileMode uint32

// wrappers for standard os functionality

type sysOs struct {
}

func (o *sysOs) Open(name string) (File, error) {
	return os.Open(name)
}

func (o *sysOs) Stat(name string) (os.FileInfo, error) {
	return os.Stat(name)
}

func (o *sysOs) IsExist(err error) bool {
	return os.IsExist(err)
}

// NewOs returns Os interface that forwards supported methods to os package.
func NewOs() Os {
	return &sysOs{}
}
