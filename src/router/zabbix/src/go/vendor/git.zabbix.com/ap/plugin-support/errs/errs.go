/*
** Zabbix
** Copyright 2001-2024 Zabbix SIA
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

package errs

import (
	"fmt"
	"strings"
	"unicode"
)

var (
	_ error    = (*stringError)(nil)
	_ rawError = (*stringError)(nil)

	_ error    = (*messageWrappedError)(nil)
	_ rawError = (*messageWrappedError)(nil)

	_ error    = (*constantWrappedError)(nil)
	_ rawError = (*constantWrappedError)(nil)
)

// rawError defines an internal interface for retriving raw error message
// strings.
type rawError interface {
	rawError() string
}

// messageWrappedError defines a basic error with a message.
type stringError struct {
	msg string
}

// messageWrappedError defines a error wrapped with a message.
type messageWrappedError struct {
	err error
	msg string
}

// constantWrappedError defines a error wrapped with an error constant.
type constantWrappedError struct {
	err      error
	constant error
}

// New creates a new ZabbixError
func New(msg string) error {
	return &stringError{
		msg: msg,
	}
}

// Errorf creates a new Zabbix compliant error string (see zabbixErrorString
// for more info) with a given format.
func Errorf(format string, args ...any) error {
	return New(fmt.Sprintf(format, args...))
}

// Wrap creates a new Zabbix error (see zabbixErrorString funcion) that wraps
// the given error with a given message.
func Wrap(err error, msg string) error {
	return &messageWrappedError{
		err: err,
		msg: msg,
	}
}

// Wrap creates a new Zabbix error (see zabbixErrorString funcion) that wraps
// the given error with a given format message.
func Wrapf(err error, format string, args ...any) error {
	return Wrap(err, fmt.Sprintf(format, args...))
}

// WrapConst wraps an error with an error constant. errors.Is and errors.As
// matches both the error and constant. Requires go version to be 1.20 or above
// as it's basically an errors.Join that was introduced in 1.20. For more info
// view constantWrappedError.Unwrap().
func WrapConst(err, constant error) error {
	return &constantWrappedError{
		err:      err,
		constant: constant,
	}
}

// Error implements the error interface for stringError, returning
// Zabbix compliant error string (see zabbixErrorString for more info).
func (e *stringError) Error() string {
	return zabbixErrorString(e.rawError())
}

// Error implements the error interface for messageWrappedError, returning
// Zabbix compliant error string (see zabbixErrorString for more info).
func (e *messageWrappedError) Error() string {
	return zabbixErrorString(e.rawError())
}

// Error implements the error interface for constantWrappedError, returning
// Zabbix compliant error string (see zabbixErrorString for more info).
func (e *constantWrappedError) Error() string {
	return zabbixErrorString(e.rawError())
}

// Unwrap returns the wrapped error allowing errors.Is and errors.As to work.
func (e *messageWrappedError) Unwrap() error {
	return e.err
}

// Unwrap returns an slice of errors that contains both the wrapped error and
// the error constant. Unwrap methods on error implementations allow errors.As
// and errors.Is to work for complex error types. The ability for Unwrap method
// to return slice was introduced in go 1.20 with errors.Join function.
func (e *constantWrappedError) Unwrap() []error {
	return []error{e.err, e.constant}
}

// rawError implements the internal rawError interface for unprocessed error
// string retrieval.
func (e *stringError) rawError() string {
	return e.msg
}

// rawError implements the internal rawError interface for unprocessed error
// string retrieval.
func (e *messageWrappedError) rawError() string {
	errStr := "nil error"

	if e.err != nil {
		errStr = e.err.Error()

		rErr, ok := e.err.(rawError)
		if ok {
			errStr = rErr.rawError()
		}
	}

	return fmt.Sprintf("%s: %s", e.msg, errStr)
}

// rawError implements the internal rawError interface for unprocessed error
// string retrieval.
func (e *constantWrappedError) rawError() string {
	errStr := "nil error"
	constantStr := "nil error"

	if e.err != nil {
		errStr = e.err.Error()

		rErr, ok := e.err.(rawError)
		if ok {
			errStr = rErr.rawError()
		}
	}

	if e.constant != nil {
		constantStr = e.constant.Error()

		rErr, ok := e.constant.(rawError)
		if ok {
			constantStr = rErr.rawError()
		}
	}

	return fmt.Sprintf("%s: %s", constantStr, errStr)
}

// zabbixErrorString creates a new string in accordance with Zabbix
// requirements:
// - the first letter must be capitalized;
// - an error text should be trailed by a dot.
func zabbixErrorString(s string) string {
	s = strings.TrimSpace(s)

	if s == "" {
		return ""
	}

	r := []rune(s)
	r[0] = unicode.ToUpper(r[0])
	s = string(r)

	if strings.HasSuffix(s, ".") {
		return s
	}

	return s + "."
}
