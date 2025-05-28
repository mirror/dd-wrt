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

import (
	"sync/atomic"
	"time"

	"golang.zabbix.com/sdk/errs"
)

var UnsupportedMetricError = errs.New("unsupported item key")

// Collector - interface for periodical metric collection
type Collector interface {
	Collect() error
	Period() int
}

// Exporter - interface for exporting collected metrics
type Exporter interface {
	// Export method exports data based on the key 'key' and its parameters 'params'.
	Export(
		key string, params []string, context ContextProvider,
	) (interface{}, error)
}

// Runner - interface for managing background processes
type Runner interface {
	// Start method activates plugin.
	Start()
	// Stop method deactivates plugin.
	Stop()
}

// Watcher - interface for fully custom monitoring
type Watcher interface {
	// Watch method instructs plugin to watch for events based on item configuration.
	Watch(items []*Item, context ContextProvider)
}

// Configurator - interface for plugin configuration in agent conf files
type Configurator interface {
	// Configure method passes global and private plugin configuration after it has been activated.
	Configure(globalOptions *GlobalOptions, privateOptions interface{})
	// Validate method validates private plugin configuration during agent startup.
	Validate(privateOptions interface{}) error
}

type ResultWriter interface {
	Write(result *Result)
	Flush()
	SlotsAvailable() int
	PersistSlotsAvailable() int
}

type Meta struct {
	lastLogsize atomic.Uint64
	mtime       atomic.Int32
	Data        interface{}
}

type ContextProvider interface {
	ClientID() uint64
	ItemID() uint64
	Output() ResultWriter
	Meta() *Meta
	GlobalRegexp() RegexpMatcher
	Timeout() int
	Delay() string
}

type Result struct {
	Itemid uint64
	Value  *string

	// additional windows eventlog fields
	EventSource    *string
	EventID        *int
	EventTimestamp *int
	EventSeverity  *int

	Ts          time.Time
	Error       error
	LastLogsize *uint64
	Mtime       *int
	Persistent  bool
}

type Item struct {
	Itemid      uint64
	Key         string
	Delay       string
	LastLogsize *uint64
	Mtime       *int
	Timeout     int
}

// GlobalOptions are global agent configuration parameters that can be accessed by plugins.
// In most cases it's recommended to allow plugins overriding global configuration parameters
// they are using with plugin specific parameters.
type GlobalOptions struct {
	Timeout  int    `json:"Timeout"`  //nolint:tagliatelle
	SourceIP string `json:"SourceIP"` //nolint:tagliatelle
}

type RegexpMatcher interface {
	Match(value, pattern string, mode int, output_template *string) (match bool, output string)
}

func (m *Meta) SetLastLogsize(value uint64) {
	m.lastLogsize.Store(value)
}

func (m *Meta) LastLogsize() uint64 {
	return m.lastLogsize.Load()
}

func (m *Meta) SetMtime(value int32) {
	m.mtime.Store(value)
}

func (m *Meta) Mtime() int32 {
	return m.mtime.Load()
}
