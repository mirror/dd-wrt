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
	"crypto/tls"
	"crypto/x509"
	"os"

	"golang.zabbix.com/sdk/errs"
)

// Corrected code with documentation comments.
var (
	// ErrNoConnectionTypesAllowed is returned when no connection types are permitted.
	ErrNoConnectionTypesAllowed = errs.New("no connection types allowed")
	// ErrConnectionTypeNotSet is returned when the connection type is not specified.
	ErrConnectionTypeNotSet = errs.New("connection type must be set")
	// ErrConnectionTypeNotAllowed is returned when the specified connection type is not in the allowed list.
	ErrConnectionTypeNotAllowed = errs.New("connection type not allowed")
	// ErrTLSCaFileNotSet is returned when the TLS CA file is required but not provided.
	ErrTLSCaFileNotSet = errs.New("TLS CA file must be set")
	// ErrTLSCertFileNotSet is returned when the TLS certificate file is required but not provided.
	ErrTLSCertFileNotSet = errs.New("TLS certificate file must be set")
	// ErrTLSKeyFileNotSet is returned when the TLS key file is required but not provided.
	ErrTLSKeyFileNotSet = errs.New("TLS key file must be set")
)

// Details holds required data for TLS connections.
type Details struct {
	SessionName        string
	TLSConnect         TLSConnectionType
	TLSCaFile          string
	TLSCertFile        string
	TLSKeyFile         string
	RawURI             string
	AllowedConnections map[string]bool
}

type certificateBundle struct {
	rootCAs     *x509.CertPool
	clientCerts []tls.Certificate
}

// NewDetails creates new Details struct with no validation.
func NewDetails(session, uri string, opts ...Option) Details {
	// Initialize with default or required values
	details := Details{
		SessionName:        session,
		RawURI:             uri,
		AllowedConnections: make(map[string]bool),
	}

	// Loop through each option and apply it to the details' struct.
	for _, opt := range opts {
		opt(&details)
	}

	return details
}

// Apply adds options to the details after creation.
func (d *Details) Apply(opts ...Option) {
	for _, opt := range opts {
		opt(d)
	}
}

// Validate checks if set TLSConnect type is allowed
// if checkCA is true checks if TLSCAFile is set (does not validate the file)
// if checkCertFile is true checks if TLSCertFile is set (does not validate the file)
// if checkKeyFile is true checks if TLSKeyFile is set (does not validate the file).
// Deprecated: implement this check at plugin Validation() not here.
func (d *Details) Validate(checkCA, checkCertFile, checkKeyFile bool) error {
	if len(d.AllowedConnections) == 0 {
		return ErrNoConnectionTypesAllowed
	}

	if d.TLSConnect == "" {
		return ErrConnectionTypeNotSet
	}

	if !d.AllowedConnections[string(d.TLSConnect)] {
		// Wrap the static error with specific context.
		return errs.Wrapf(ErrConnectionTypeNotAllowed, "connection type %s", d.TLSConnect)
	}

	if checkCA && d.TLSCaFile == "" {
		// Wrap the static error with specific context.
		return errs.Wrapf(ErrTLSCaFileNotSet, "with connection type %s", d.TLSConnect)
	}

	if checkCertFile && d.TLSCertFile == "" {
		// Wrap the static error with specific context.
		return errs.Wrapf(ErrTLSCertFileNotSet, "with connection type %s", d.TLSConnect)
	}

	if checkKeyFile && d.TLSKeyFile == "" {
		// Wrap the static error with specific context.
		return errs.Wrapf(ErrTLSKeyFileNotSet, "with connection type %s", d.TLSConnect)
	}

	return nil
}

// GetTLSConfig creates TLS.Config from details according to the zabbix conventions.
// Returns (nil, nil) when TLS is disabled.
func (d *Details) GetTLSConfig() (*tls.Config, error) {
	switch d.TLSConnect {
	case Disabled:
		return nil, nil //nolint:nilnil // this behavior is not ambiguous.

	case Required:
		return d.buildRequiredTLSConfig()

	case VerifyCA:
		return d.buildVerifyCATLSConfig()

	case VerifyFull:
		return d.buildVerifyFullTLSConfig()

	default:
		return nil, errs.New("unsupported TLS connection type: " + string(d.TLSConnect))
	}
}

func (d *Details) buildRequiredTLSConfig() (*tls.Config, error) {
	certBundle, err := d.loadCertificates()
	if err != nil {
		return nil, err
	}

	return &tls.Config{
		RootCAs:            certBundle.rootCAs,
		Certificates:       certBundle.clientCerts,
		InsecureSkipVerify: true, //nolint:gosec // default TLS verification is skipped due to user settings.
	}, nil
}

func (d *Details) buildVerifyCATLSConfig() (*tls.Config, error) {
	certBundle, err := d.loadCertificates()
	if err != nil {
		return nil, err
	}

	customVerifyFunction := verifyPeerCertificateFunc("", certBundle.rootCAs)

	return &tls.Config{
		RootCAs:               certBundle.rootCAs,
		Certificates:          certBundle.clientCerts,
		InsecureSkipVerify:    true, //nolint:gosec // default TLS verification is skipped to use custom function.
		VerifyPeerCertificate: customVerifyFunction,
	}, nil
}

func (d *Details) buildVerifyFullTLSConfig() (*tls.Config, error) {
	certBundle, err := d.loadCertificates()
	if err != nil {
		return nil, err
	}

	return &tls.Config{ //nolint:gosec // minimum TLS version will be added in future
		RootCAs:      certBundle.rootCAs,
		Certificates: certBundle.clientCerts,
	}, nil
}

// loadCertificates loads both root CA or client certificates in one operation.
func (d *Details) loadCertificates() (*certificateBundle, error) {
	// Load root CAs
	rootCertPool := x509.NewCertPool()

	if d.TLSCaFile != "" {
		pem, err := os.ReadFile(d.TLSCaFile)
		if err != nil {
			return nil, errs.Wrap(err, "failed to read TLS CA file")
		}

		if !rootCertPool.AppendCertsFromPEM(pem) {
			return nil, errs.New("failed to append PEM to root CA pool")
		}
	}

	// Load client certificates
	if d.TLSCertFile == "" || d.TLSKeyFile == "" {
		return &certificateBundle{
			rootCAs: rootCertPool,
		}, nil
	}

	keyPair, err := tls.LoadX509KeyPair(d.TLSCertFile, d.TLSKeyFile)
	if err != nil {
		return nil, errs.Wrap(err, "failed to load TLS keyPair and/or key file")
	}

	clientCerts := []tls.Certificate{keyPair}

	return &certificateBundle{
		rootCAs:     rootCertPool,
		clientCerts: clientCerts,
	}, nil
}
