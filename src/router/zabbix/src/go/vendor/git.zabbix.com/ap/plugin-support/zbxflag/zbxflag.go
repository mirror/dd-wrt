/*
** Zabbix
** Copyright 2001-2023 Zabbix SIA
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

var (
	_ Flager = (*StringFlag)(nil)
	_ Flager = (*BoolFlag)(nil)
	_ Flager = (*Flags)(nil)
)

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

// StringFlag describes a string flag.
type StringFlag struct {
	Flag
	Default string
	Dest    *string
}

// BoolFlag describes a boolean flag.
type BoolFlag struct {
	Flag
	Default bool
	Dest    *bool
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

// Register registers the flag on a flag set.
func (f *StringFlag) Register(fs *flag.FlagSet) {
	fs.StringVar(f.Dest, f.Name, f.Default, "")
	fs.StringVar(f.Dest, f.Shorthand, f.Default, "")
}

// Register registers the flag on a flag set.
func (f *BoolFlag) Register(fs *flag.FlagSet) {
	fs.BoolVar(f.Dest, f.Name, f.Default, "")
	fs.BoolVar(f.Dest, f.Shorthand, f.Default, "")
}

// Register registers all flags in a flag set.
func (flags Flags) Register(fs *flag.FlagSet) {
	for _, flag := range flags {
		flag.Register(fs)
	}
}

// Usage returns the usage string of a string flag.
func (flags Flags) Usage() string {
	usages := make([]string, 0, len(flags))

	for _, flag := range flags {
		usages = append(usages, flag.Usage())
	}

	return strings.Join(usages, "\n")
}
