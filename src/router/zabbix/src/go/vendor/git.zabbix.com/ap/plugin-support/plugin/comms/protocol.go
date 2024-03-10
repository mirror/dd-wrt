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

package comms

import "git.zabbix.com/ap/plugin-support/plugin"

const ProtocolVersion = "6.4.0"

const NonRequiredID = 0

const (
	Exporter = 1 << iota
	Configurator
	Runner
)

const (
	LogRequestType = iota + 1
	RegisterRequestType
	RegisterResponseType
	StartRequestType
	TerminateRequestType
	ExportRequestType
	ExportResponseType
	ConfigureRequestType
	ValidateRequestType
	ValidateResponseType
	PeriodRequestType
	PeriodResponseType
)

type request int

var toString = map[request]string{
	LogRequestType:       "Log Request",
	RegisterRequestType:  "Register Request",
	RegisterResponseType: "Register Response",
	StartRequestType:     "Start Request",
	TerminateRequestType: "Terminate Request",
	ExportRequestType:    "Export Request",
	ExportResponseType:   "Export Response",
	ConfigureRequestType: "Configure Request",
	ValidateRequestType:  "Validate Request",
	ValidateResponseType: "Validate Response",
	PeriodRequestType:    "Period Request",
	PeriodResponseType:   "Period Response",
}

func GetRequestName(reqType uint32) string {
	return toString[request(reqType)]
}

func ImplementsConfigurator(in uint32) bool {
	return in&Configurator != 0
}

func ImplementsExporter(in uint32) bool {
	return in&Exporter != 0
}

func ImplementsRunner(in uint32) bool {
	return in&Runner != 0
}

type Common struct {
	Id   uint32 `json:"id"`
	Type uint32 `json:"type"`
}

type LogRequest struct {
	Common
	Severity uint32 `json:"severity"`
	Message  string `json:"message"`
}

type RegisterRequest struct {
	Common
	ProtocolVersion string `json:"version"`
}

type RegisterResponse struct {
	Common
	Name       string   `json:"name"`
	Metrics    []string `json:"metrics,omitempty"`
	Interfaces uint32   `json:"interfaces,omitempty"`
	Error      string   `json:"error,omitempty"`
}

type ValidateRequest struct {
	Common
	PrivateOptions interface{} `json:"private_options,omitempty"`
}

type ValidateResponse struct {
	Common
	Error string `json:"error,omitempty"`
}

type StartRequest struct {
	Common
}

type TerminateRequest struct {
	Common
}

type ExportRequest struct {
	Common
	Key    string   `json:"key"`
	Params []string `json:"parameters,omitempty"`
}

type ExportResponse struct {
	Common
	Value interface{} `json:"value,omitempty"`
	Error string      `json:"error,omitempty"`
}

type ConfigureRequest struct {
	Common
	GlobalOptions  *plugin.GlobalOptions `json:"global_options"`
	PrivateOptions interface{}           `json:"private_options,omitempty"`
}
