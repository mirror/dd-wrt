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
	"bytes"
	"fmt"
	"log"
	"os"
	"path/filepath"
	"runtime"
	"runtime/debug"
	"sync"

	"golang.zabbix.com/sdk/errs"
)

// Constants define supported log levels and their priorities.
const (
	None    = -1
	Info    = 0
	Crit    = 1
	Err     = 2
	Warning = 3
	Debug   = 4
	Trace   = 5
)

// Constants defines the logging destinations and/or targets.
const (
	Undefined = 0
	System    = 1
	File      = 2
	Console   = 3
)

// MB defines the size of a megabyte.
const MB = 1048576

//nolint:gochecknoglobals
var (
	// DefaultLogger is the default Zabbix agent 2 and Zabbix web service logger.
	DefaultLogger *log.Logger
	logLevel      int
	logStat       logStatus
	logAccess     sync.Mutex
)

type logStatus struct {
	logType     int
	filename    string
	filesize    int64
	f           *os.File
	currentSize int64
}

// CheckLogLevel checks if the log level is enabled.
func CheckLogLevel(level int) bool {
	return level <= logLevel
}

// Level returns the current log level string representation.
func Level() string {
	switch logLevel {
	case None:
		return "none"
	case Info:
		return "info"
	case Crit:
		return "critical"
	case Err:
		return "error"
	case Warning:
		return "warning"
	case Debug:
		return "debug"
	case Trace:
		return "trace"
	default:
		return "unknown"
	}
}

// IncreaseLogLevel increases the log level.
func IncreaseLogLevel() bool {
	if logLevel != Trace {
		logLevel++

		return true
	}

	return false
}

// DecreaseLogLevel decreases the log level.
func DecreaseLogLevel() bool {
	if logLevel != Info {
		logLevel--

		return true
	}

	return false
}

// Open sets a new logger based on the log type and a new log output level.
func Open(logType, level int, filename string, filesize int) error {
	logStat.logType = logType
	logStat.filename = filename
	logStat.filesize = int64(filesize) * MB

	switch logType {
	case System:
		err := createSyslog()
		if err != nil {
			return err
		}
	case Console:
		DefaultLogger = log.New(os.Stdout, "", log.Lmicroseconds|log.Ldate)
	case File:
		var err error

		logStat.f, err = os.OpenFile(filename, os.O_APPEND|os.O_CREATE|os.O_WRONLY, 0o640) //nolint:gosec
		if err != nil {
			return errs.Wrap(err, "failed to open log file")
		}

		DefaultLogger = log.New(logStat.f, "", log.Lmicroseconds|log.Ldate)
	default:
		return errs.Errorf("unknown log type %d", logType)
	}

	logLevel = level

	return nil
}

// Infof logs a message with the Info level.
func Infof(format string, args ...any) {
	procLog(format, args, Info)
}

// Critf logs a message with the Critical level.
func Critf(format string, args ...any) {
	procLog(format, args, Crit)
}

// Errf logs a message with the Error level.
func Errf(format string, args ...any) {
	procLog(format, args, Err)
}

// Warningf logs a message with the Warning level.
func Warningf(format string, args ...any) {
	procLog(format, args, Warning)
}

// Debugf logs a message with the Debug level.
func Debugf(format string, args ...any) {
	procLog(format, args, Debug)
}

// Tracef logs a message with the Trace level.
func Tracef(format string, args ...any) {
	procLog(format, args, Trace)
}

func procLog(format string, args []any, level int) {
	if !CheckLogLevel(level) {
		return
	}

	if logStat.logType == System {
		procSysLog(format, args, level)

		return
	}

	logAccess.Lock()
	defer logAccess.Unlock()

	rotateLog()

	DefaultLogger.Printf(format, args...)
}

// RefreshLogFile exposes rotateLog under lock
// Used by Agent to avoid holding old fds when the logfile
// was rotated by external tool.
func RefreshLogFile() {
	logAccess.Lock()
	defer logAccess.Unlock()

	rotateLog()
}

// disabled all reported issues, because to fix
// them would require a complete rewrite of the logic
//
//nolint:gocyclo,nestif,errcheck,revive,gosec,cyclop
func rotateLog() {
	if logStat.logType == File {
		fstat, err := os.Stat(logStat.filename)
		if err != nil || fstat.Size() == 0 || logStat.currentSize > fstat.Size() {
			logStat.f.Close()

			if logStat.f, err = os.OpenFile(logStat.filename, os.O_CREATE|os.O_WRONLY, 0o640); err != nil {
				logStat.logType = Undefined
				log.Fatalf("Cannot open log file %s", logStat.filename)
			}

			DefaultLogger = log.New(logStat.f, "", log.Lmicroseconds|log.Ldate)
			logStat.currentSize = 0

			return
		}

		if logStat.filesize != 0 {
			var printError string

			logStat.currentSize = fstat.Size()

			if logStat.currentSize > logStat.filesize {
				filenameOld := logStat.filename + ".old"
				logStat.f.Close()
				os.Remove(filenameOld)

				err = os.Rename(logStat.filename, filenameOld)
				if err != nil {
					printError = err.Error()
				}

				logStat.f, err = os.OpenFile(logStat.filename, os.O_CREATE|os.O_WRONLY, 0o640)
				if err != nil {
					errmsg := "Cannot open log file " + logStat.filename
					if printError != "" {
						errmsg = fmt.Sprintf("%s and cannot rename it: %s", errmsg, printError)
					}

					logStat.logType = Undefined

					log.Fatal(errmsg)
				}

				DefaultLogger = log.New(logStat.f, "", log.Lmicroseconds|log.Ldate)

				if printError != "" {
					DefaultLogger.Printf(
						"cannot rename log file \"%s\" to \"%s\":%s\n", logStat.filename, filenameOld, printError,
					)
					DefaultLogger.Printf(
						"Logfile %q size reached configured limit LogFileSize but "+
							"moving it to %q failed. The logfile was truncated.",
						logStat.filename,
						filenameOld,
					)
				}
			}
		}
	}
}

// PanicHook must be called in a defer, on panic writes stack trace to log
// and re-panics.
func PanicHook() {
	r := recover() //revive:disable-line
	if r == nil {
		return
	}

	if logStat.logType == Undefined {
		return
	}

	data := debug.Stack()

	Critf("Critical failure: %v", r)

	var tail int

	for offset, end, num := 0, 0, 1; end != -1; offset, num = offset+end+1, num+1 {
		end = bytes.IndexByte(data[offset:], '\n')

		if end != -1 {
			tail = offset + end
		} else {
			tail = len(data)
		}

		Critf("%s", string(data[offset:tail]))
	}

	panic(r)
}

// Caller returns the name of the function that called the caller function.
func Caller() string {
	var (
		pc     = make([]uintptr, 2)
		n      = runtime.Callers(2, pc)
		frames = runtime.CallersFrames(pc[:n])
	)

	frame, ok := frames.Next()
	if !ok {
		return ""
	}

	return filepath.Base(frame.Func.Name())
}
