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

package zbxerr

import (
	"errors"
	"unicode"
)

type ZabbixError struct {
	err   error
	cause error
}

// New creates a new ZabbixError
func New(msg string) ZabbixError {
	return ZabbixError{errors.New(msg), nil}
}

// Wrap creates a new ZabbixError with wrapped cause
func (e ZabbixError) Wrap(cause error) error {
	return ZabbixError{err: e, cause: cause}
}

// Unwrap extracts an original underlying error
func (e ZabbixError) Unwrap() error {
	return e.err
}

// Cause returns a cause of original error
func (e ZabbixError) Cause() error {
	return e.cause
}

// Error stringifies an error according to Zabbix requirements:
// * the first letter must be capitalized;
// * an error text should be trailed by a dot.
func (e ZabbixError) Error() string {
	var msg string

	ucFirst := func(str string) string {
		for i, v := range str {
			return string(unicode.ToUpper(v)) + str[i+1:]
		}

		return ""
	}

	if zbxErr, ok := e.err.(ZabbixError); ok {
		msg = zbxErr.Raw()
	} else {
		msg = e.err.Error()
	}

	if e.cause != nil {
		msg += ": " + e.cause.Error()
	}

	if msg[len(msg)-1:] != "." {
		msg += "."
	}

	return ucFirst(msg)
}

// Raw returns a non-modified error message
func (e ZabbixError) Raw() string {
	return e.err.Error()
}

var (
	ErrorInvalidParams        = New("invalid parameters")
	ErrorTooFewParameters     = New("too few parameters")
	ErrorTooManyParameters    = New("too many parameters")
	ErrorInvalidConfiguration = New("invalid configuration")
	ErrorCannotFetchData      = New("cannot fetch data")
	ErrorCannotUnmarshalJSON  = New("cannot unmarshal JSON")
	ErrorCannotMarshalJSON    = New("cannot marshal JSON")
	ErrorCannotParseResult    = New("cannot parse result")
	ErrorConnectionFailed     = New("connection failed")
	ErrorUnsupportedMetric    = New("unsupported metric")
	ErrorEmptyResult          = New("empty result")
	ErrorUnknownSession       = New("unknown session")
)
