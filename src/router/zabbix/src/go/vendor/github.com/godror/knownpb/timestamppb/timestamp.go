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

	"github.com/VictoriaMetrics/easyproto"
	"github.com/godror/knownpb/internal"
	"google.golang.org/protobuf/types/known/timestamppb"
)

var (
	_ = json.Marshaler((*Timestamp)(nil))
	_ = json.Unmarshaler((*Timestamp)(nil))
	_ = encoding.TextMarshaler((*Timestamp)(nil))
	_ = encoding.TextUnmarshaler((*Timestamp)(nil))
	_ = xml.Marshaler((*Timestamp)(nil))
	_ = xml.Unmarshaler((*Timestamp)(nil))
	// _ = proto.Message((*Timestamp)(nil))
	_ = sql.Scanner((*Timestamp)(nil))
	_ = driver.Valuer((*Timestamp)(nil))
)

// go : generate go get -tool github.com/planetscale/vtprotobuf/cmd/protoc-gen-go-vtproto@latest
//go:generate sh -c "mkdir -p $(go env GOPATH)/src/google/protobuf; curl -sS -L -m 30 -o $(go env GOPATH)/src/google/protobuf/timestamp.proto https://github.com/protocolbuffers/protobuf/raw/main/src/google/protobuf/timestamp.proto"
// go : generate sh -c "protoc -I$(go env GOPATH)/src --go-vtproto_out=$(go env GOPATH)/src --go-vtproto_opt=features=marshal+unmarshal+size $(go env GOPATH)/src/google/protobuf/timestamp.proto && cp -a $(go env GOPATH)/src/google.golang.org/protobuf/types/known/timestamppb/timestamp_vtproto.pb.go timestamp_vtproto.go"

// Timestamp is a wrapped timestamppb.Timestamp with proper JSON and XML marshaling (as string date).
type Timestamp struct {
	// Represents seconds of UTC time since Unix epoch
	// 1970-01-01T00:00:00Z. Must be from 0001-01-01T00:00:00Z to
	// 9999-12-31T23:59:59Z inclusive.
	Seconds int64 `protobuf:"varint,1,opt,name=seconds,proto3" json:"seconds,omitempty"`
	// Non-negative fractions of a second at nanosecond resolution. Negative
	// second values with fractions must still have non-negative nanos values
	// that count forward in time. Must be from 0 to 999,999,999
	// inclusive.
	Nanos int32 `protobuf:"varint,2,opt,name=nanos,proto3" json:"nanos,omitempty"`
}

// Now constructs a new Timestamp from the current time.
func Now() *Timestamp {
	return New(time.Now())
}

// New constructs a new Timestamp from the provided time.Time.
func New(t time.Time) *Timestamp {
	return &Timestamp{
		Seconds: int64(t.Unix()),
		Nanos:   int32(t.Nanosecond()),
	}
}

// AsTime converts x to a time.Time.
func (x *Timestamp) AsTime() time.Time {
	if x.IsZero() {
		return time.Time{}
	}
	return time.Unix(int64(x.Seconds), int64(x.Nanos)).UTC()
}

// IsValid reports whether the timestamp is valid.
// It is equivalent to CheckValid == nil.
func (x *Timestamp) IsValid() bool {
	return x.check() == 0
}

// CheckValid returns an error if the timestamp is invalid.
// In particular, it checks whether the value represents a date that is
// in the range of 0001-01-01T00:00:00Z to 9999-12-31T23:59:59Z inclusive.
// An error is reported for a nil Timestamp.
func (x *Timestamp) CheckValid() error {
	switch x.check() {
	case invalidNil:
		return fmt.Errorf("invalid nil Timestamp")
	case invalidUnderflow:
		return fmt.Errorf("timestamp (%v) before 0001-01-01", x)
	case invalidOverflow:
		return fmt.Errorf("timestamp (%v) after 9999-12-31", x)
	case invalidNanos:
		return fmt.Errorf("timestamp (%v) has out-of-range nanos", x)
	default:
		return nil
	}
}

const (
	_ = iota
	invalidNil
	invalidUnderflow
	invalidOverflow
	invalidNanos
)

func (x *Timestamp) check() uint {
	const minTimestamp = -62135596800  // Seconds between 1970-01-01T00:00:00Z and 0001-01-01T00:00:00Z, inclusive
	const maxTimestamp = +253402300799 // Seconds between 1970-01-01T00:00:00Z and 9999-12-31T23:59:59Z, inclusive
	secs := x.Seconds
	nanos := x.Nanos
	switch {
	case x == nil:
		return invalidNil
	case secs < minTimestamp:
		return invalidUnderflow
	case secs > maxTimestamp:
		return invalidOverflow
	case nanos < 0 || nanos >= 1e9:
		return invalidNanos
	default:
		return 0
	}
}

func (x *Timestamp) Reset() { *x = Timestamp{} }

func (*Timestamp) ProtoMessage() {}

func (ts *Timestamp) Format(layout string) string {
	if ts == nil {
		return ""
	}
	t := ts.AsTime()
	if t.IsZero() {
		return ""
	}
	return t.Format(layout)
}
func (ts *Timestamp) AppendFormat(b []byte, layout string) []byte {
	if ts == nil {
		return nil
	}
	t := ts.AsTime()
	if t.IsZero() {
		return nil
	}
	return t.AppendFormat(b, layout)
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
	*ts = *New(t)
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
func (ts *Timestamp) Proto() {}

// func (ts *Timestamp) ProtoReflect() protoreflect.Message { return ts.AsTimestamp().ProtoReflect() }

// MarshalProtobuf marshals ts into protobuf message, appends this message to dst and returns the result.
//
// This function doesn't allocate memory on repeated calls.
func (ts *Timestamp) MarshalProtobuf(dst []byte) []byte {
	m := mp.Get()
	ts.marshalProtobuf(m.MessageMarshaler())
	dst = m.Marshal(dst)
	mp.Put(m)
	return dst
}

func (ts *Timestamp) marshalProtobuf(mm *easyproto.MessageMarshaler) {
	mm.AppendInt64(1, ts.Seconds)
	mm.AppendInt32(2, ts.Nanos)
}

var mp easyproto.MarshalerPool

// UnmarshalProtobuf unmarshals ts from protobuf message at src.
func (ts *Timestamp) UnmarshalProtobuf(src []byte) (err error) {
	// Set default Timestamp values
	ts.Reset()

	// Parse Timestamp message at src
	var fc easyproto.FieldContext
	for len(src) > 0 {
		src, err = fc.NextField(src)
		if err != nil {
			return fmt.Errorf("cannot read next field in Timestamp message")
		}
		switch fc.FieldNum {
		case 1:
			secs, ok := fc.Int64()
			if !ok {
				return fmt.Errorf("cannot read Timestamp seconds")
			}
			// name refers to src. This means that the name changes when src changes.
			// Make a copy with strings.Clone(name) if needed.
			ts.Seconds = secs
		case 2:
			nanos, ok := fc.Int32()
			if !ok {
				return fmt.Errorf("cannot read Timestamp nanos")
			}
			ts.Nanos = nanos
		}
	}
	return nil
}
