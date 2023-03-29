// Copyright 2014, 2021 Tamás Gulácsi
//
// SPDX-License-Identifier: Apache-2.0

package internal

import (
	"bufio"
	"encoding/xml"
	"reflect"
	"unsafe"
)

func GetXMLEncoderWriter(enc *xml.Encoder) *bufio.Writer {
	rEnc := reflect.ValueOf(enc)
	rP := rEnc.Elem().FieldByName("p").Addr()
	return *(**bufio.Writer)(unsafe.Pointer(rP.Elem().FieldByName("Writer").UnsafeAddr()))
}
