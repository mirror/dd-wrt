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

package log

import "fmt"

var _ Logger = (*loggerImpl)(nil)

// Logger is the interface defines basic logging methods.
type Logger interface {
	Infof(format string, args ...any)
	Critf(format string, args ...any)
	Errf(format string, args ...any)
	Warningf(format string, args ...any)
	Debugf(format string, args ...any)
	Tracef(format string, args ...any)
}

type loggerImpl struct {
	prefix string
}

// New creates a new logger.
func New(module string) Logger { //nolint:ireturn
	var prefix string

	if module != "" {
		prefix = fmt.Sprintf("[%s] ", module)
	}

	return &loggerImpl{prefix: prefix}
}

// Infof logs a message with the Info level.
func (l *loggerImpl) Infof(format string, args ...any) {
	Infof(l.prefix+format, args...)
}

// Critf logs a message with the Critical level.
func (l *loggerImpl) Critf(format string, args ...any) {
	Critf(l.prefix+format, args...)
}

// Errf logs a message with the Error level.
func (l *loggerImpl) Errf(format string, args ...any) {
	Errf(l.prefix+format, args...)
}

// Warningf logs a message with the Warning level.
func (l *loggerImpl) Warningf(format string, args ...any) {
	Warningf(l.prefix+format, args...)
}

// Debugf logs a message with the Debug level.
func (l *loggerImpl) Debugf(format string, args ...any) {
	Debugf(l.prefix+format, args...)
}

// Tracef logs a message with the Trace level.
func (l *loggerImpl) Tracef(format string, args ...any) {
	Tracef(l.prefix+format, args...)
}
