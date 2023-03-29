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

package comms

import (
	"bytes"
	"encoding/binary"
	"encoding/json"
	"fmt"
	"net"

	"git.zabbix.com/ap/plugin-support/zbxerr"
)

const JSONType = uint32(1)
const headerTypeLen = 4
const headerDataLen = 4

func Read(conn net.Conn) (dataType uint32, requestData []byte, err error) {
	reqByteType := make([]byte, headerTypeLen)
	reqByteLen := make([]byte, headerDataLen)

	err = readData(conn, reqByteType)
	if err != nil {
		return dataType, requestData, fmt.Errorf("failed to read type header, %w", err)
	}

	if JSONType != binary.LittleEndian.Uint32(reqByteType) {
		err = zbxerr.New(fmt.Sprintf("only json data type (%d) supported", JSONType))

		return
	}

	err = readData(conn, reqByteLen)
	if err != nil {
		return dataType, requestData, fmt.Errorf("failed to read data header, %w", err)
	}

	data := make([]byte, int32(binary.LittleEndian.Uint32(reqByteLen)))

	err = readData(conn, data)
	if err != nil {
		return dataType, requestData, fmt.Errorf("failed to read data body, %w", err)
	}

	var c Common
	if err = json.Unmarshal(data, &c); err != nil {
		return dataType, requestData, fmt.Errorf("failed to unmarshal data body, %w", err)
	}

	return c.Type, data, nil
}

func readData(conn net.Conn, b []byte) error {
	var offset int

	for offset != len(b) {
		n, err := conn.Read(b[offset:])
		if err != nil {
			return err
		}

		offset += n
	}

	return nil
}

func Write(conn net.Conn, in interface{}) (err error) {
	reqBytes, err := json.Marshal(in)
	if err != nil {
		return
	}

	buf := new(bytes.Buffer)
	err = binary.Write(buf, binary.LittleEndian, JSONType)
	if err != nil {
		return
	}

	err = binary.Write(buf, binary.LittleEndian, uint32(len(reqBytes)))
	if err != nil {
		return
	}

	_, err = buf.Write(reqBytes)
	if err != nil {
		return
	}

	if _, err = conn.Write(buf.Bytes()); err != nil {
		return
	}

	return
}
