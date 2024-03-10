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

package tlsconfig

import (
	"crypto/tls"
	"crypto/x509"
	"errors"
	"fmt"
	"os"

	"git.zabbix.com/ap/plugin-support/errs"
	"git.zabbix.com/ap/plugin-support/uri"
)

// Details holds required data for TLS connections
type Details struct {
	SessionName        string
	TlsConnect         string
	TlsCaFile          string
	TlsCertFile        string
	TlsKeyFile         string
	RawUri             string
	AllowedConnections map[string]bool
}

// Validate checks if set TlsConnect type is allowed
// if checkCA is true checks if TLSCAFile is set (does not validate the file)
// if checkCertFile is true checks if TlsCertFile is set (does not validate the file)
// if checkKeyFile is true checks if TlsKeyFile is set (does not validate the file)
func (d *Details) Validate(checkCA, checkCertFile, checkKeyFile bool) error {
	if len(d.AllowedConnections) == 0 {
		return errors.New("no connection types allowed")
	}

	if d.TlsConnect == "" {
		return errors.New("connection type must be set")
	}

	if !d.AllowedConnections[d.TlsConnect] {
		return fmt.Errorf("connection type %s not allowed", d.TlsConnect)
	}

	if checkCA && d.TlsCaFile == "" {
		return fmt.Errorf("TLS CA file must be set with connection type %s", d.TlsConnect)
	}

	if checkCertFile && d.TlsCertFile == "" {
		return fmt.Errorf("TLS certificate file must be set with connection type %s", d.TlsConnect)
	}

	if checkKeyFile && d.TlsKeyFile == "" {
		return fmt.Errorf("TLS key file must be set with connection type %s", d.TlsConnect)
	}

	return nil
}

// GetTLSConfig creates tls.Config from details
func (d *Details) GetTLSConfig(skipVerify bool) (*tls.Config, error) {
	rootCertPool := x509.NewCertPool()
	pem, err := os.ReadFile(d.TlsCaFile)
	if err != nil {
		return nil, errs.Wrap(err, "failed to read TLS CA file")
	}

	if ok := rootCertPool.AppendCertsFromPEM(pem); !ok {
		return nil, errs.New("failed to append PEM")
	}

	clientCerts, err := d.LoadCertificates()
	if err != nil {
		return nil, err
	}

	if skipVerify {
		return &tls.Config{RootCAs: rootCertPool, Certificates: clientCerts, InsecureSkipVerify: skipVerify}, nil
	}

	url, err := uri.New(d.RawUri, nil)
	if err != nil {
		return nil, errs.Wrap(err, "failed to parse uri")
	}

	return &tls.Config{
		RootCAs: rootCertPool, Certificates: clientCerts, InsecureSkipVerify: skipVerify, ServerName: url.Host(),
	}, nil
}

// LoadCertificates combines cert and key files
func (d *Details) LoadCertificates() ([]tls.Certificate, error) {
	if d.TlsCertFile == "" || d.TlsKeyFile == "" {
		return nil, nil
	}

	var Certs []tls.Certificate

	certs, err := tls.LoadX509KeyPair(d.TlsCertFile, d.TlsKeyFile)
	if err != nil {
		return nil, errs.Wrap(err, "failed to load tls cert and/or key file")
	}

	Certs = []tls.Certificate{certs}

	return Certs, nil
}

func VerifyPeerCertificateFunc(
	dnsName string, rootCAPool *x509.CertPool,
) func(certificates [][]byte, _ [][]*x509.Certificate) error {
	return func(certificates [][]byte, _ [][]*x509.Certificate) error {
		if len(certificates) == 0 {
			return errs.New("no TLS certificates found")
		}

		certs := make([]*x509.Certificate, 0, len(certificates))

		for _, c := range certificates {
			cert, err := x509.ParseCertificate(c)
			if err != nil {
				return errs.New("no TLS certificates found")
			}

			certs = append(certs, cert)
		}

		opts := x509.VerifyOptions{
			DNSName:       dnsName,
			Roots:         rootCAPool,
			Intermediates: x509.NewCertPool(),
		}

		for i, cert := range certs {
			if i == 0 {
				continue
			}

			opts.Intermediates.AddCert(cert)
		}

		_, err := certs[0].Verify(opts)
		if err != nil {
			return errs.Wrap(err, "failed to verify certificate")
		}

		return nil
	}
}

// CreateConfig (deprecated) creates tls.Config from details
func CreateConfig(details Details, skipVerify bool) (*tls.Config, error) {
	rootCertPool := x509.NewCertPool()
	pem, err := os.ReadFile(details.TlsCaFile)
	if err != nil {
		return nil, err
	}

	if ok := rootCertPool.AppendCertsFromPEM(pem); !ok {
		return nil, errs.New("failed to append PEM")
	}

	clientCerts := make([]tls.Certificate, 0, 1)
	certs, err := tls.LoadX509KeyPair(details.TlsCertFile, details.TlsKeyFile)
	if err != nil {
		return nil, err
	}

	clientCerts = append(clientCerts, certs)

	if skipVerify {
		return &tls.Config{RootCAs: rootCertPool, Certificates: clientCerts, InsecureSkipVerify: skipVerify}, nil
	}

	url, err := uri.New(details.RawUri, nil)
	if err != nil {
		return nil, err
	}

	return &tls.Config{
		RootCAs: rootCertPool, Certificates: clientCerts, InsecureSkipVerify: skipVerify, ServerName: url.Host(),
	}, nil
}

// NewDetails creates new Details struct with no validation
func NewDetails(session, dbConnect, caFile, certFile, keyFile, uri string, connectionTypes ...string) Details {
	connTypes := make(map[string]bool)
	for _, v := range connectionTypes {
		connTypes[v] = true
	}

	return Details{session, dbConnect, caFile, certFile, keyFile, uri, connTypes}
}

// CreateDetails (deprecated) creates details with validation
func CreateDetails(session, dbConnect, caFile, certFile, keyFile, uri string) (Details, error) {
	if dbConnect != "" && dbConnect != "required" {
		if err := validateSetTLSFiles(caFile, certFile, keyFile); err != nil {
			return Details{}, fmt.Errorf("%s uri %s, with session %s", err.Error(), uri, session)
		}
	} else {
		if err := validateUnsetTLSFiles(caFile, certFile, keyFile); err != nil {
			return Details{}, fmt.Errorf("%s uri %s, with session %s", err.Error(), uri, session)
		}
	}

	return Details{session, dbConnect, caFile, certFile, keyFile, uri, nil}, nil
}

func validateSetTLSFiles(caFile, certFile, keyFile string) error {
	if caFile == "" {
		return errors.New("missing TLS CA file")
	}

	if certFile == "" {
		return errors.New("missing TLS certificate file")
	}

	if keyFile == "" {
		return errors.New("missing TLS key file")
	}

	return nil
}

func validateUnsetTLSFiles(caFile, certFile, keyFile string) error {
	errTmpl := "TLS %s file must not be set, when unencrypted connection or TLS without identity checks is configured"

	if caFile != "" {
		return fmt.Errorf(errTmpl, "CA")
	}

	if certFile != "" {
		return fmt.Errorf(errTmpl, "certificate")
	}

	if keyFile != "" {
		return fmt.Errorf(errTmpl, "key")
	}

	return nil
}
