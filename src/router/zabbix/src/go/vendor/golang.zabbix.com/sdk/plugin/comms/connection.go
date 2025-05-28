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
	"bytes"
	"encoding/binary"
	"encoding/json"
	"net"

	"golang.zabbix.com/sdk/errs"
)

const (
	// JSONType specifies that the data transmitted is encoded in JSON format.
	JSONType      = uint32(1)
	headerTypeLen = 4
	headerDataLen = 4
)

// Read reads a single loadable plugin and agent communications request.
func Read(conn net.Conn) (Common, []byte, error) {
	reqByteType := make([]byte, headerTypeLen) //nolint:makezero
	reqByteLen := make([]byte, headerDataLen)  //nolint:makezero

	err := readData(conn, reqByteType)
	if err != nil {
		return Common{}, nil, errs.Wrap(err, "failed to read type header")
	}

	if JSONType != binary.LittleEndian.Uint32(reqByteType) {
		return Common{}, nil, errs.Errorf("only json data type (%d) is supported", JSONType)
	}

	err = readData(conn, reqByteLen)
	if err != nil {
		return Common{}, nil, errs.Wrap(err, "failed to read data header")
	}

	data := make([]byte, int32(binary.LittleEndian.Uint32(reqByteLen))) //nolint:makezero

	err = readData(conn, data)
	if err != nil {
		return Common{}, nil, errs.Wrap(err, "failed to read data body")
	}

	var c Common

	err = json.Unmarshal(data, &c)
	if err != nil {
		return Common{}, nil, errs.Wrap(err, "failed to unmarshal data body")
	}

	return c, data, nil
}

// Write writes a single loadable plugin and agent communications request.
// Marshals `in` to json and writes it to the provided `conn`.
func Write(conn net.Conn, in any) error {
	reqBytes, err := json.Marshal(in)
	if err != nil {
		return errs.Wrap(err, "failed to marshal request")
	}

	buf := new(bytes.Buffer)

	err = binary.Write(buf, binary.LittleEndian, JSONType)
	if err != nil {
		return errs.Wrap(err, "failed to write type header")
	}

	err = binary.Write(buf, binary.LittleEndian, uint32(len(reqBytes)))
	if err != nil {
		return errs.Wrap(err, "failed to write data header")
	}

	_, err = buf.Write(reqBytes)
	if err != nil {
		return errs.Wrap(err, "failed to encode data body")
	}

	_, err = conn.Write(buf.Bytes())
	if err != nil {
		return errs.Wrap(err, "failed to write data body")
	}

	return nil
}

func readData(conn net.Conn, b []byte) error {
	var offset int

	for offset != len(b) {
		n, err := conn.Read(b[offset:])
		if err != nil {
			return err //nolint:wrapcheck
		}

		offset += n
	}

	return nil
}
