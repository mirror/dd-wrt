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
	"fmt"
	"strings"
)

var _ Flager = (*Flags)(nil)

// Flager describes an methods for a single flag.
type Flager interface {
	Usage() string
	Register(fs *flag.FlagSet)
}

// Flag describes base information about a flag.
type Flag struct {
	Name        string
	Shorthand   string
	Description string
}

// Flags is a collection of multiple flags.
type Flags []Flager

// Usage returns the usage string of a flag.
func (f *Flag) Usage() string {
	return fmt.Sprintf(
		"  %-40s%s",
		fmt.Sprintf("-%s --%s", f.Shorthand, f.Name),
		f.Description,
	)
}

// Register registers all flags in a flag set.
func (flags Flags) Register(fs *flag.FlagSet) {
	for _, f := range flags {
		f.Register(fs)
	}
}

// Usage returns the usage string of a string flag.
func (flags Flags) Usage() string {
	usages := make([]string, 0, len(flags))

	for _, f := range flags {
		usages = append(usages, f.Usage())
	}

	return strings.Join(usages, "\n")
}
