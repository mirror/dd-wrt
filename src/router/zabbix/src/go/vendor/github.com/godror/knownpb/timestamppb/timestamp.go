// Copyright 2019, 2021 Tamás Gulácsi
//
// SPDX-License-Identifier: Apache-2.0

package timestamppb

import (
	"bufio"
	"bytes"
	"database/sql"
	"database/sql/driver"
	"encoding"
	"encoding/json"
	"encoding/xml"
	"fmt"
	"strings"
	"time"

	"github.com/godror/knownpb/internal"
	"google.golang.org/protobuf/proto"
	"google.golang.org/protobuf/reflect/protoreflect"
	"google.golang.org/protobuf/types/known/timestamppb"
)

var (
	_ = json.Marshaler((*Timestamp)(nil))
	_ = json.Unmarshaler((*Timestamp)(nil))
	_ = encoding.TextMarshaler((*Timestamp)(nil))
	_ = encoding.TextUnmarshaler((*Timestamp)(nil))
	_ = xml.Marshaler((*Timestamp)(nil))
	_ = xml.Unmarshaler((*Timestamp)(nil))
	_ = proto.Message((*Timestamp)(nil))
	_ = sql.Scanner((*Timestamp)(nil))
	_ = driver.Valuer((*Timestamp)(nil))
)

// Timestamp is a wrapped timestamppb.Timestamp with proper JSON and XML marshaling (as string date).
type Timestamp struct {
	timestamppb.Timestamp
}

func New(t time.Time) *Timestamp {
	return &Timestamp{*timestamppb.New(t)}
}
func (ts *Timestamp) AsTimestamp() *timestamppb.Timestamp {
	if ts == nil {
		return nil
	}
	return &ts.Timestamp
}
func (ts *Timestamp) Format(layout string) string {
	if ts == nil {
		return ""
	}
	t := ts.Timestamp.AsTime()
	if t.IsZero() {
		return ""
	}
	return t.Format(layout)
}
func (ts *Timestamp) AppendFormat(b []byte, layout string) []byte {
	if ts == nil {
		return nil
	}
	t := ts.Timestamp.AsTime()
	if t.IsZero() {
		return nil
	}
	return t.AppendFormat(b, layout)
}
func (ts *Timestamp) Reset() {
	if t := ts.AsTimestamp(); t != nil {
		t.Reset()
	}
}
func (ts *Timestamp) Scan(src interface{}) error {
	if src == nil {
		ts.Reset()
		return nil
	}
	switch x := src.(type) {
	case time.Time:
		if x.IsZero() {
			ts.Reset()
		} else {
			*ts = *New(x)
		}
	case *time.Time:
		if x == nil || x.IsZero() {
			ts.Reset()
		} else {
			*ts = *New(*x)
		}
	case sql.NullTime:
		if !x.Valid {
			ts.Reset()
		} else {
			*ts = *New(x.Time)
		}
	case *timestamppb.Timestamp:
		if x == nil {
			ts.Reset()
		} else {
			*ts = *New(x.AsTime())
		}
	case *Timestamp:
		*ts = *New(x.AsTime())
	default:
		return fmt.Errorf("cannot scan %T to DateTime", src)
	}
	return nil
}
func (ts *Timestamp) Value() (driver.Value, error) {
	if ts == nil || ts.IsZero() {
		return time.Time{}, nil
	}
	return ts.AsTime(), nil
}

func (ts *Timestamp) MarshalXML(enc *xml.Encoder, start xml.StartElement) error {
	if ts != nil {
		t := ts.AsTime()
		if !t.IsZero() {
			return enc.EncodeElement(t.In(time.Local).Format(time.RFC3339), start)
		}
	}
	start.Attr = append(start.Attr,
		xml.Attr{Name: xml.Name{Space: "http://www.w3.org/2001/XMLSchema-instance", Local: "nil"}, Value: "true"})

	bw := internal.GetXMLEncoderWriter(enc)
	bw.Flush()
	old := *bw
	var buf bytes.Buffer
	*bw = *bufio.NewWriter(&buf)
	if err := enc.EncodeElement("", start); err != nil {
		return err
	}
	b := bytes.ReplaceAll(bytes.ReplaceAll(bytes.ReplaceAll(bytes.ReplaceAll(
		buf.Bytes(),
		[]byte("_XMLSchema-instance:"), []byte("xsi:")),
		[]byte("xmlns:_XMLSchema-instance="), []byte("xmlns:xsi=")),
		[]byte("XMLSchema-instance:"), []byte("xsi:")),
		[]byte("xmlns:XMLSchema-instance="), []byte("xmlns:xsi="))
	*bw = old
	if _, err := bw.Write(b); err != nil {
		return err
	}
	return bw.Flush()
}
func (ts *Timestamp) UnmarshalXML(dec *xml.Decoder, st xml.StartElement) error {
	var s string
	if err := dec.DecodeElement(&s, &st); err != nil {
		return err
	}
	return ts.UnmarshalText([]byte(s))
}

func (ts *Timestamp) IsZero() (zero bool) {
	return ts == nil || ts.Seconds == 0 && ts.Nanos == 0
}
func (ts *Timestamp) AsTime() time.Time {
	if ts.IsZero() {
		return time.Time{}
	}
	return ts.AsTimestamp().AsTime()
}
func (ts *Timestamp) MarshalJSON() ([]byte, error) {
	if ts != nil {
		t := ts.AsTime()
		if !t.IsZero() {
			return t.In(time.Local).MarshalJSON()
		}
	}
	return []byte(`""`), nil
}
func (ts *Timestamp) UnmarshalJSON(data []byte) error {
	// Ignore null, like in the main JSON package.
	data = bytes.TrimSpace(data)
	if len(data) == 0 || bytes.Equal(data, []byte(`""`)) || bytes.Equal(data, []byte("null")) {
		ts.Reset()
		return nil
	}
	return ts.UnmarshalText(data)
}

// MarshalText implements the encoding.TextMarshaler interface.
// The time is formatted in RFC 3339 format, with sub-second precision added if present.
func (ts *Timestamp) MarshalText() ([]byte, error) {
	if ts != nil {
		t := ts.AsTime()
		if !t.IsZero() {
			return ts.AsTime().In(time.Local).MarshalText()
		}
	}
	return nil, nil
}

// UnmarshalText implements the encoding.TextUnmarshaler interface.
// The time is expected to be in RFC 3339 format.
func (ts *Timestamp) UnmarshalText(data []byte) error {
	data = bytes.Trim(data, " \"")
	n := len(data)
	if n == 0 {
		ts.Reset()
		//log.Println("time=")
		return nil
	}
	layout := time.RFC3339
	if bytes.IndexByte(data, '.') >= 19 {
		layout = time.RFC3339Nano
	}
	if n < 10 {
		layout = "20060102"
	} else {
		if n > len(layout) {
			data = data[:len(layout)]
		} else if n < 4 {
			layout = layout[:4]
		} else {
			for _, i := range []int{4, 7, 10} {
				if n <= i {
					break
				}
				if data[i] != layout[i] {
					data[i] = layout[i]
				}
			}
			if bytes.IndexByte(data, '.') < 0 {
				layout = layout[:n]
			} else if _, err := time.ParseInLocation(layout, string(data), time.Local); err != nil && strings.HasSuffix(err.Error(), `"" as "Z07:00"`) {
				layout = strings.TrimSuffix(layout, "Z07:00")
			}
		}
	}
	// Fractional seconds are handled implicitly by Parse.
	t, err := time.ParseInLocation(layout, string(data), time.Local)
	//log.Printf("s=%q time=%v err=%+v", data, dt.Time, err)
	if err != nil {
		return fmt.Errorf("ParseInLocation(%q, %q): %w", layout, string(data), err)
	}
	*(ts.AsTimestamp()) = *timestamppb.New(t)
	return nil
}

func (ts *Timestamp) String() string {
	if ts != nil {
		t := ts.AsTime()
		if !t.IsZero() {
			return ts.AsTime().In(time.Local).Format(time.RFC3339)
		}
	}
	return ""
}
func (ts *Timestamp) Proto()                             {}
func (ts *Timestamp) ProtoReflect() protoreflect.Message { return ts.AsTimestamp().ProtoReflect() }
