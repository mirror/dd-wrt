//go:build linux
// +build linux

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
	"bytes"
	"os"
	"syscall"
	"time"

	"golang.org/x/sys/unix"
)

// mocked os functionality

type MockOs interface {
	MockFile(path string, data []byte)
}

type fileTime struct {
	ModTime time.Time
}

type mockOs struct {
	files  map[string][]byte
	ftimes map[string]fileTime
}

type mockFile struct {
	buffer *bytes.Buffer
}

type fileStat struct {
	name    string
	size    int64
	mode    FileMode
	modTime time.Time
	sys     syscall.Stat_t
}

func (o *mockOs) Open(name string) (File, error) {
	if data, ok := o.files[name]; !ok {
		return nil, os.ErrNotExist
	} else {
		return &mockFile{bytes.NewBuffer(data)}, nil
	}
}

func (o *fileStat) IsDir() bool {
	return false
}

func (o *fileStat) Mode() os.FileMode {
	return os.FileMode(o.mode)
}

func (o *fileStat) ModTime() time.Time {
	return o.modTime
}

func (o *fileStat) Name() string {
	return o.name
}

func (o *fileStat) Size() int64 {
	return o.size
}

func (o *fileStat) Sys() interface{} {
	return &o.sys
}

func (o *mockOs) Stat(name string) (os.FileInfo, error) {
	if data, ok := o.files[name]; !ok {
		return nil, os.ErrNotExist
	} else {
		var fs fileStat

		fs.mode = 436
		fs.modTime = o.ftimes[name].ModTime
		fs.name = name
		fs.size = int64(len(data))
		a, err := unix.TimeToTimespec(o.ftimes[name].ModTime)
		if err != nil {
			return nil, err
		}
		fs.sys.Atim.Sec = a.Sec
		fs.sys.Ctim.Sec = a.Sec

		return &fs, nil
	}
}

func (o *mockOs) IsExist(err error) bool {

	if err == nil {
		return false
	}
	if err.Error() == "exists" {
		return true
	}
	return false
}

func (f *mockFile) Close() error {
	return nil
}

func (f *mockFile) Read(p []byte) (n int, err error) {
	return f.buffer.Read(p)
}

// MockFile creates new mock file with the specified path and contents.
func (o *mockOs) MockFile(path string, data []byte) {
	o.files[path] = data
	var ft fileTime
	ft.ModTime = time.Now()
	o.ftimes[path] = ft
}

// NewMockOs returns Os interface that replaces supported os package functionality with mock functions.
func NewMockOs() Os {
	return &mockOs{
		files:  make(map[string][]byte),
		ftimes: make(map[string]fileTime),
	}
}
