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

package plugin

import "golang.zabbix.com/sdk/log"

type Accessor interface {
	Init(name string)
	Name() string
	MaxCapacity() int
	SetMaxCapacity(capacity int)
	HandleTimeout() bool
	SetHandleTimeout(enable bool)
	IsExternal() bool
}

type Base struct {
	log.Logger
	name          string
	maxCapacity   int
	external      bool
	handleTimeout bool
}

// SystemOptions contains System Options of plugin.
//
// Deprecated: SystemOptions exists for historical compatibility with older Zabbix agent 2 versions. Currently System
// options are parsed and removed in Zabix agent 2.
type SystemOptions struct {
	Path                     string `conf:"optional"`
	Capacity                 string `conf:"optional"`
	ForceActiveChecksOnStart string `conf:"optional"`
}

// Init initializes base structure with name and logger.
func (b *Base) Init(name string) {
	b.Logger = log.New(name)
	b.name = name
}

// Name returns base structures name.
func (b *Base) Name() string {
	return b.name
}

// MaxCapacity returns base structures max capacity.
func (b *Base) MaxCapacity() int {
	return b.maxCapacity
}

// SetMaxCapacity sets max capacity plugin can have.
func (b *Base) SetMaxCapacity(capacity int) {
	b.maxCapacity = capacity
}

// IsExternal returns whether plugin is set to be external or not.
func (b *Base) IsExternal() bool {
	return b.external
}

// SetExternal sets external bool to provided value.
func (b *Base) SetExternal(isExternal bool) {
	b.external = isExternal
}

// HandleTimeout returns handle timeout.
func (b *Base) HandleTimeout() bool {
	return b.handleTimeout
}

// SetHandleTimeout sets handle timeout.
func (b *Base) SetHandleTimeout(handleTimeout bool) {
	b.handleTimeout = handleTimeout
}
