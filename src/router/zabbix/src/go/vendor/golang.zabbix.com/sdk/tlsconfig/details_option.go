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

// Option is a function that configures a Details struct.
type Option func(*Details)

// WithTLSConnect sets the TLSConnect value.
func WithTLSConnect(tlsConnect TLSConnectionType) Option {
	return func(d *Details) {
		d.TLSConnect = tlsConnect
	}
}

// WithTLSCaFile sets the TLSCaFile path.
func WithTLSCaFile(caFile string) Option {
	return func(d *Details) {
		d.TLSCaFile = caFile
	}
}

// WithTLSCertFile sets the TLSCertFile path.
func WithTLSCertFile(certFile string) Option {
	return func(d *Details) {
		d.TLSCertFile = certFile
	}
}

// WithTLSKeyFile sets the TLSKeyFile path.
func WithTLSKeyFile(keyFile string) Option {
	return func(d *Details) {
		d.TLSKeyFile = keyFile
	}
}

// WithAllowedConnections sets the allowed connection types.
func WithAllowedConnections(connectionTypes ...string) Option {
	return func(d *Details) {
		for _, v := range connectionTypes {
			d.AllowedConnections[v] = true
		}
	}
}
