/*
** Copyright (C) 2001-2025 Zabbix SIA
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

package zbxflag

import (
	"flag"

	"golang.zabbix.com/sdk/errs"
)

var (
	_ flag.Value = (*StringFlag)(nil)
	_ Flager     = (*StringFlag)(nil)
)

// StringFlag describes a string flag.
type StringFlag struct {
	Flag
	Default string
	Dest    *string
	set     bool
}

// Register registers the flag on a flag set.
func (f *StringFlag) Register(fs *flag.FlagSet) {
	*f.Dest = f.Default

	fs.Var(f, f.Name, "")
	fs.Var(f, f.Shorthand, "")
}

// Set sets the flags value.
func (f *StringFlag) Set(s string) error {
	if f.set {
		return errs.Errorf(
			"argument -%s, --%s already set",
			f.Flag.Shorthand,
			f.Flag.Name,
		)
	}

	*f.Dest = s
	f.set = true

	return nil
}

// String returns flags value as string.
func (f *StringFlag) String() string {
	return *f.Dest
}
