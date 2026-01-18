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
	w := rP.Elem().FieldByName("w")
	if !w.IsValid() { // pre-1.20
		w = rP.Elem().FieldByName("Writer")
	}
	return *(**bufio.Writer)(unsafe.Pointer(w.UnsafeAddr()))
}
