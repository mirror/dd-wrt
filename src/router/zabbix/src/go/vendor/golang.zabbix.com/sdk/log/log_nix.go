//go:build !windows
// +build !windows

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

import (
	"fmt"
	"log/syslog"
	"os"

	"golang.zabbix.com/sdk/errs"
)

var syslogWriter *syslog.Writer //nolint:gochecknoglobals

func createSyslog() error {
	var err error

	syslogWriter, err = syslog.New(syslog.LOG_WARNING|syslog.LOG_DAEMON, "zabbix_agent2")
	if err != nil {
		return errs.Wrap(err, "failed to create syslog writer")
	}

	return nil
}

func procSysLog(format string, args []any, level int) {
	var err error

	switch level {
	case Info:
		err = syslogWriter.Info(fmt.Sprintf(format, args...))
	case Crit:
		err = syslogWriter.Crit(fmt.Sprintf(format, args...))
	case Err:
		err = syslogWriter.Err(fmt.Sprintf(format, args...))
	case Warning:
		err = syslogWriter.Warning(fmt.Sprintf(format, args...))
	case Debug, Trace:
		err = syslogWriter.Debug(fmt.Sprintf(format, args...))
	}

	if err != nil {
		fmt.Fprintf(os.Stderr, "failed to write to syslog: %s", err.Error())
	}
}
