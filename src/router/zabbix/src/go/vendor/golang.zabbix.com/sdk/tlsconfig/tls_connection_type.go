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

package tlsconfig

import (
	"fmt"
	"strings"

	"golang.zabbix.com/sdk/errs"
)

const (
	// Disabled indicates that no TLS encryption will be used.
	Disabled TLSConnectionType = "disabled"
	// Required indicates a TLS connection that skips server certificate verification.
	Required TLSConnectionType = "required"
	// VerifyCA indicates a TLS connection that verifies the server certificate against a trusted CA.
	VerifyCA TLSConnectionType = "verify_ca"
	// VerifyFull indicates a TLS connection that verifies both the CA and the server's hostname.
	VerifyFull TLSConnectionType = "verify_full"
)

//nolint:gochecknoglobals //this is a constant map which is in a form of variable.
var validConnectionTypes = map[TLSConnectionType]struct{}{
	Disabled:   {},
	Required:   {},
	VerifyCA:   {},
	VerifyFull: {},
}

// TLSConnectionType is used to define what kind of connection will be implemented.
type TLSConnectionType string

// NewTLSConnectionType returns error if such type does not exist.
func NewTLSConnectionType(tlsType string) (TLSConnectionType, error) {
	tlsType = strings.TrimSpace(tlsType)

	if tlsType == "" {
		return Disabled, nil
	}

	// Normalize input for case-insensitive comparison.
	normalizedType := TLSConnectionType(strings.ToLower(tlsType))

	_, ok := validConnectionTypes[normalizedType]
	if !ok {
		// define slice to have consistent order.
		validTypes := []string{
			string(Required),
			string(VerifyCA),
			string(VerifyFull),
		}

		return "", errs.New(fmt.Sprintf("%s connection type is invalid, valid types are: %s",
			tlsType, strings.Join(validTypes, ", ")))
	}

	return normalizedType, nil
}
