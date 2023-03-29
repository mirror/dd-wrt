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

package tlsconfig

import (
	"crypto/tls"
	"crypto/x509"
	"errors"
	"fmt"
	"io/ioutil"

	"git.zabbix.com/ap/plugin-support/uri"
)

type Details struct {
	SessionName string
	TlsConnect  string
	TlsCaFile   string
	TlsCertFile string
	TlsKeyFile  string
	RawUri      string
}

func CreateConfig(details Details, skipVerify bool) (*tls.Config, error) {
	rootCertPool := x509.NewCertPool()
	pem, err := ioutil.ReadFile(details.TlsCaFile)
	if err != nil {
		return nil, err
	}

	if ok := rootCertPool.AppendCertsFromPEM(pem); !ok {
		return nil, errors.New("Failed to append PEM.")
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

	return Details{session, dbConnect, caFile, certFile, keyFile, uri}, nil
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
