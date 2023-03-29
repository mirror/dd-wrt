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

package log

import "fmt"

type Logger interface {
	Tracef(format string, args ...interface{})
	Debugf(format string, args ...interface{})
	Warningf(format string, args ...interface{})
	Infof(format string, args ...interface{})
	Errf(format string, args ...interface{})
	Critf(format string, args ...interface{})
}

type loggerImpl struct {
	prefix string
}

func New(module string) Logger {
	var prefix string
	if module != "" {
		prefix = fmt.Sprintf("[%s] ", module)
	}
	return &loggerImpl{prefix: prefix}
}

func (l *loggerImpl) Critf(format string, args ...interface{}) {
	Critf(l.prefix+format, args...)
}

func (l *loggerImpl) Infof(format string, args ...interface{}) {
	Infof(l.prefix+format, args...)
}

func (l *loggerImpl) Warningf(format string, args ...interface{}) {
	Warningf(l.prefix+format, args...)
}

func (l *loggerImpl) Tracef(format string, args ...interface{}) {
	Tracef(l.prefix+format, args...)
}

func (l *loggerImpl) Debugf(format string, args ...interface{}) {
	Debugf(l.prefix+format, args...)
}

func (l *loggerImpl) Errf(format string, args ...interface{}) {
	Errf(l.prefix+format, args...)
}
