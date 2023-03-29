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

package plugin

import "git.zabbix.com/ap/plugin-support/log"

type Accessor interface {
	Init(name string)
	Name() string
	Capacity() int
	SetCapacity(capactity int)
	IsExternal() bool
}

type Base struct {
	log.Logger
	name     string
	capacity int
	external bool
}

func (b *Base) Init(name string) {
	b.Logger = log.New(name)
	b.name = name
	b.capacity = DefaultCapacity
}

func (b *Base) Name() string {
	return b.name
}

func (b *Base) Capacity() int {
	return b.capacity
}

func (b *Base) SetCapacity(capacity int) {
	b.capacity = capacity
}

func (b *Base) IsExternal() bool {
	return b.external
}

func (b *Base) SetExternal(isExternal bool) {
	b.external = isExternal
}

type SystemOptions struct {
	Path     string `conf:"optional"`
	Capacity string `conf:"optional"`
}
