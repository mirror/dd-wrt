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

package comms

import (
	"golang.zabbix.com/sdk/errs"
)

const (
	// URI is the configuration field for the URI.
	URI ConfigSetting = "URI"
	// User is the configuration field for the user.
	User ConfigSetting = "User"
	// Password is the configuration field for the password.
	Password ConfigSetting = "Password"
	// TLSConnect is the configuration field for the tls connection type.
	TLSConnect ConfigSetting = "TLSConnect"
	// TLSCAFile is the configuration field for the CA file.
	TLSCAFile ConfigSetting = "TLSCAFile"
	// TLSCertFile is the configuration field for the certificate file.
	TLSCertFile ConfigSetting = "TLSCertFile"
	// TLSKeyFile is the configuration field for the key file.
	TLSKeyFile ConfigSetting = "TLSKeyFile"
	// TLSCRLFile is the configuration field for the CRL file.
	TLSCRLFile ConfigSetting = "TLSCRLFile"
)

//nolint:gochecknoglobals //this is a constant map which is in a form of variable.
var tlsConfigSettingsSet = map[ConfigSetting]struct{}{
	TLSConnect:  {},
	TLSCAFile:   {},
	TLSCertFile: {},
	TLSKeyFile:  {},
	TLSCRLFile:  {},
}

// ConfigSetting represents a single TLS configuration field name.
type ConfigSetting string

// NewTLSConfigSetting validates and creates a new ConfigSetting from a string.
// It returns an error if the provided string is not a valid setting.
func NewTLSConfigSetting(config string) (ConfigSetting, error) {
	setting := ConfigSetting(config)

	if _, ok := tlsConfigSettingsSet[setting]; !ok {
		return "", errs.New("invalid TLS config setting: " + config)
	}

	return setting, nil
}

// AllTLSConfigSettings returns a slice of all valid ConfigSetting values.
func AllTLSConfigSettings() []ConfigSetting {
	allSettings := make([]ConfigSetting, 0, len(tlsConfigSettingsSet))

	for setting := range tlsConfigSettingsSet {
		allSettings = append(allSettings, setting)
	}

	return allSettings
}
