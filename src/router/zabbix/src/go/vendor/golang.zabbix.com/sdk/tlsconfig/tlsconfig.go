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
	"golang.zabbix.com/sdk/uri"
)

var (
	errTLSCaFileMustNotBeSet   = errs.New("TLS CA file must not be set")
	errTLSCertFileMustNotBeSet = errs.New("TLS certificate file must not be set")
	errTLSKeyFileMustNotBeSet  = errs.New("TLS key file must not be set")
)

func verifyPeerCertificateFunc(
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

// CreateConfig (deprecated) creates tls.Config from details.
// Deprecated: use Details.GetTLSConfig method.
//
//nolint:gocritic
func CreateConfig(details Details, skipVerify bool) (*tls.Config, error) {
	rootCertPool := x509.NewCertPool()

	pem, err := os.ReadFile(details.TLSCaFile)
	if err != nil {
		return nil, errs.Wrap(err, "failed to read TLS CA file")
	}

	ok := rootCertPool.AppendCertsFromPEM(pem)
	if !ok {
		return nil, errs.New("failed to append PEM")
	}

	clientCerts := make([]tls.Certificate, 0, 1)

	certs, err := tls.LoadX509KeyPair(details.TLSCertFile, details.TLSKeyFile)
	if err != nil {
		return nil, errs.Wrap(err, "failed to load key pair")
	}

	clientCerts = append(clientCerts, certs)

	if skipVerify {
		//nolint:gosec
		return &tls.Config{RootCAs: rootCertPool, Certificates: clientCerts, InsecureSkipVerify: skipVerify}, nil
	}

	url, err := uri.New(details.RawURI, nil)
	if err != nil {
		return nil, errs.Wrap(err, "failed to parse URI")
	}

	return &tls.Config{
		//nolint:gosec
		RootCAs: rootCertPool, Certificates: clientCerts, InsecureSkipVerify: skipVerify, ServerName: url.Host(),
	}, nil
}

// CreateDetails (deprecated) creates details with validation.
// Deprecated: use NewDetails.
//
//nolint:revive // deprecated function
func CreateDetails(session, dbConnect, caFile, certFile, keyFile, u string) (Details, error) {
	if dbConnect != "" && dbConnect != "required" {
		err := validateSetTLSFiles(caFile, certFile, keyFile)
		if err != nil {
			return Details{}, errs.Wrap(err, "uri "+u+", with session "+session)
		}
	} else {
		err := validateUnsetTLSFiles(caFile, certFile, keyFile)
		if err != nil {
			return Details{}, errs.Wrap(err, "uri "+u+", with session "+session)
		}
	}

	return Details{
		SessionName:        session,
		TLSConnect:         TLSConnectionType(dbConnect),
		TLSCaFile:          caFile,
		TLSCertFile:        certFile,
		TLSKeyFile:         keyFile,
		RawURI:             u,
		AllowedConnections: nil,
	}, nil
}

func validateSetTLSFiles(caFile, certFile, keyFile string) error {
	if caFile == "" {
		return errs.New("missing TLS CA file")
	}

	if certFile == "" {
		return errs.New("missing TLS certificate file")
	}

	if keyFile == "" {
		return errs.New("missing TLS key file")
	}

	return nil
}

func validateUnsetTLSFiles(caFile, certFile, keyFile string) error {
	const errCtx = "when unencrypted connection or TLS without identity checks is configured"

	if caFile != "" {
		return errs.Wrap(errTLSCaFileMustNotBeSet, errCtx)
	}

	if certFile != "" {
		return errs.Wrap(errTLSCertFileMustNotBeSet, errCtx)
	}

	if keyFile != "" {
		return errs.Wrap(errTLSKeyFileMustNotBeSet, errCtx)
	}

	return nil
}
