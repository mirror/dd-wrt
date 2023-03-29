// Copyright 2017, 2022 The Godror Authors
//
//
// SPDX-License-Identifier: UPL-1.0 OR Apache-2.0

package godror

/*
#include <stdlib.h>
#include "dpiImpl.h"
*/
import "C"
import (
	"bufio"
	"bytes"
	"context"
	"database/sql/driver"
	"encoding/json"
	"errors"
	"fmt"
	"io"
	"reflect"
	"runtime"
	"sort"
	"strings"
	"sync"
	"unsafe"
)

var _ = fmt.Printf

// Object represents a dpiObject.
type Object struct {
	dpiObject *C.dpiObject
	*ObjectType
}

// ErrNoSuchKey is the error for missing key in lookup.
var ErrNoSuchKey = errors.New("no such key")

// GetAttribute gets the i-th attribute into data.
func (O *Object) GetAttribute(data *Data, name string) error {
	if O == nil {
		panic("nil Object")
	}
	attr, ok := O.Attributes[name]
	if !ok {
		return fmt.Errorf("%s: %w", name, ErrNoSuchKey)
	}

	data.reset()
	data.NativeTypeNum = attr.NativeTypeNum
	data.ObjectType = attr.ObjectType
	data.implicitObj = true
	if O.dpiObject == nil {
		data.SetNull()
		return nil
	}
	// the maximum length of that buffer must be supplied
	// in the value.asBytes.length attribute before calling this function.
	if attr.NativeTypeNum == C.DPI_NATIVE_TYPE_BYTES && attr.OracleTypeNum == C.DPI_ORACLE_TYPE_NUMBER {
		var a [39]byte
		C.dpiData_setBytes(&data.dpiData, (*C.char)(unsafe.Pointer(&a[0])), C.uint32_t(len(a)))
	}

	//fmt.Printf("getAttributeValue(%p, %p, %d, %+v)\n", O.dpiObject, attr.dpiObjectAttr, data.NativeTypeNum, data.dpiData)
	if err := O.drv.checkExec(func() C.int {
		return C.dpiObject_getAttributeValue(O.dpiObject, attr.dpiObjectAttr, data.NativeTypeNum, &data.dpiData)
	}); err != nil {
		return fmt.Errorf("getAttributeValue(%q, obj=%s, attr=%+v, typ=%d): %w", name, O.Name, attr.dpiObjectAttr, data.NativeTypeNum, err)
	}
	logger := getLogger()
	if logger != nil {
		logger.Log("msg", "getAttributeValue", "dpiObject", fmt.Sprintf("%p", O.dpiObject),
			attr.Name, fmt.Sprintf("%p", attr.dpiObjectAttr),
			"nativeType", data.NativeTypeNum, "oracleType", attr.OracleTypeNum,
			"data", data.dpiData, "p", fmt.Sprintf("%p", data))
	}
	return nil
}

// SetAttribute sets the named attribute with data.
func (O *Object) SetAttribute(name string, data *Data) error {
	if !strings.Contains(name, `"`) {
		name = strings.ToUpper(name)
	}
	attr, ok := O.Attributes[name]
	if !ok {
		return fmt.Errorf("%s[%q]: %w", O, name, ErrNoSuchKey)
	}
	if data.NativeTypeNum == 0 {
		data.NativeTypeNum = attr.NativeTypeNum
		data.ObjectType = attr.ObjectType
	}
	return O.drv.checkExec(func() C.int {
		return C.dpiObject_setAttributeValue(O.dpiObject, attr.dpiObjectAttr, data.NativeTypeNum, &data.dpiData)
	})
}

// Set is a convenience function to set the named attribute with the given value.
func (O *Object) Set(name string, v interface{}) error {
	if data, ok := v.(*Data); ok {
		return O.SetAttribute(name, data)
	}
	d := scratch.Get()
	defer scratch.Put(d)
	if err := d.Set(v); err != nil {
		return err
	}
	return O.SetAttribute(name, d)
}

// ResetAttributes prepare all attributes for use the object as IN parameter
func (O *Object) ResetAttributes() error {
	runtime.LockOSThread()
	defer runtime.UnlockOSThread()

	data := scratch.Get()
	defer scratch.Put(data)
	for _, attr := range O.Attributes {
		data.reset()
		data.NativeTypeNum = attr.NativeTypeNum
		data.ObjectType = attr.ObjectType
		if attr.NativeTypeNum == C.DPI_NATIVE_TYPE_BYTES && attr.OracleTypeNum == C.DPI_ORACLE_TYPE_NUMBER {
			a := make([]byte, attr.Precision)
			C.dpiData_setBytes(&data.dpiData, (*C.char)(unsafe.Pointer(&a[0])), C.uint32_t(attr.Precision))
		}
		if C.dpiObject_setAttributeValue(O.dpiObject, attr.dpiObjectAttr, data.NativeTypeNum, &data.dpiData) == C.DPI_FAILURE {
			return O.drv.getError()
		}
	}

	return nil
}

// Get scans the named attribute into dest, and returns it.
func (O *Object) Get(name string) (interface{}, error) {
	d := scratch.Get()
	defer scratch.Put(d)
	if err := O.GetAttribute(d, name); err != nil {
		return nil, err
	}
	isObject := d.IsObject()
	ot := O.Attributes[name].ObjectType
	if isObject {
		d.ObjectType = ot
	}
	v := d.Get()
	if !isObject {
		return maybeString(v, ot), nil
	}
	sub := v.(*Object)
	if sub != nil && sub.ObjectType.CollectionOf != nil {
		return &ObjectCollection{Object: sub}, nil
	}
	return sub, nil
}

func maybeString(v interface{}, ot *ObjectType) interface{} {
	switch ot.OracleTypeNum {
	case C.DPI_ORACLE_TYPE_VARCHAR, C.DPI_ORACLE_TYPE_NVARCHAR,
		C.DPI_ORACLE_TYPE_CHAR, C.DPI_ORACLE_TYPE_NCHAR,
		C.DPI_ORACLE_TYPE_NUMBER,
		C.DPI_ORACLE_TYPE_CLOB, C.DPI_ORACLE_TYPE_NCLOB,
		C.DPI_ORACLE_TYPE_LONG_VARCHAR, C.DPI_ORACLE_TYPE_LONG_NVARCHAR:

		if b, ok := v.([]byte); ok {
			return string(b)
		}
	}
	return v
}

// ObjectRef implements userType interface.
func (O *Object) ObjectRef() *Object {
	return O
}

// Collection returns &ObjectCollection{Object: O} iff the Object is a collection.
// Otherwise it returns nil.
func (O *Object) Collection() ObjectCollection {
	if O.ObjectType.CollectionOf == nil {
		return ObjectCollection{}
	}
	return ObjectCollection{Object: O}
}

// Close releases a reference to the object.
func (O *Object) Close() error {
	if O == nil {
		return nil
	}
	obj := O.dpiObject
	O.dpiObject = nil
	if obj == nil {
		return nil
	}
	logger := getLogger()
	if logger != nil {
		logger.Log("msg", "Object.Close", "object", obj)
	}
	if err := O.drv.checkExec(func() C.int { return C.dpiObject_release(obj) }); err != nil {
		return fmt.Errorf("error on close object: %w", err)
	}

	return nil
}

// AsMap is a convenience function that returns the object's attributes as a map[string]interface{}.
// It allocates, so use it as a guide how to implement your own converter function.
//
// If recursive is true, then the embedded objects are converted, too, recursively.
func (O *Object) AsMap(recursive bool) (map[string]interface{}, error) {
	if O == nil || O.dpiObject == nil {
		return nil, nil
	}
	m := make(map[string]interface{}, len(O.ObjectType.Attributes))
	logger := getLogger()
	data := scratch.Get()
	defer scratch.Put(data)
	for a, ot := range O.ObjectType.Attributes {
		if err := O.GetAttribute(data, a); err != nil {
			return m, fmt.Errorf("%q: %w", a, err)
		}
		d := data.Get()
		if d == nil {
			continue
		}
		if !data.IsObject() {
			d = maybeString(d, ot.ObjectType)
		}
		m[a] = d
		if logger != nil {
			logger.Log("msg", "AsMap", "attribute", a, "data", fmt.Sprintf("%#v", d), "type", fmt.Sprintf("%T", d), "recursive", recursive)
		}
		if !recursive {
			continue
		}
		if sub, ok := d.(*Object); ok && sub != nil && sub.ObjectType != nil {
			var err error
			if sub.ObjectType.CollectionOf == nil {
				if m[a], err = sub.AsMap(recursive); err != nil {
					return m, fmt.Errorf("%q.AsMap: %w", a, err)
				}
				continue
			}
			if sub.ObjectType.CollectionOf.Attributes == nil {
				if m[a], err = sub.Collection().AsSlice(nil); err != nil {
					return m, fmt.Errorf("%q.AsSlice: %w", a, err)
				}
			} else if m[a], err = sub.Collection().AsMapSlice(recursive); err != nil {
				return m, fmt.Errorf("%q.AsMapSlice: %w", a, err)
			}
		}
	}
	return m, nil
}

// ToJSON writes the Object as JSON into the io.Writer.
func (O *Object) ToJSON(w io.Writer) error {
	if O == nil || O.ObjectType == nil {
		_, err := io.WriteString(w, "nil")
		return err
	}
	if O.ObjectType.CollectionOf != nil {
		return O.Collection().ToJSON(w)
	}
	bw := bufio.NewWriter(w)
	defer bw.Flush()
	if err := bw.WriteByte('{'); err != nil {
		return err
	}
	data := scratch.Get()
	defer scratch.Put(data)
	var buf bytes.Buffer
	enc := json.NewEncoder(&buf)
	keys := make([]string, 0, len(O.ObjectType.Attributes))
	for a := range O.ObjectType.Attributes {
		keys = append(keys, a)
	}
	sort.Strings(keys)
	for i, a := range keys {
		if err := O.GetAttribute(data, a); err != nil {
			return fmt.Errorf("%q: %w", a, err)
		}
		if i != 0 {
			if err := bw.WriteByte(','); err != nil {
				return err
			}
		}
		fmt.Fprintf(bw, "%q:", a)
		d := data.Get()
		if data.IsObject() {
			if err := d.(*Object).ToJSON(bw); err != nil {
				return fmt.Errorf("%q: %w", a, err)
			}
			continue
		}
		d = maybeString(d, O.ObjectType.Attributes[a].ObjectType)
		buf.Reset()
		if err := enc.Encode(d); err != nil {
			return fmt.Errorf("%q: %#v: %w", a, d, err)
		}
		if _, err := bw.Write(bytes.TrimSpace(buf.Bytes())); err != nil {
			return fmt.Errorf("%q: %w", a, err)
		}
	}
	return bw.WriteByte('}')
}

func (O *Object) String() string {
	if O == nil {
		return ""
	}
	var buf strings.Builder
	_, _ = buf.WriteString(O.ObjectType.String())
	_ = O.ToJSON(&buf)
	return buf.String()
}

// ObjectCollection represents a Collection of Objects - itself an Object, too.
type ObjectCollection struct {
	*Object
}

// ErrNotCollection is returned when the Object is not a collection.
var ErrNotCollection = errors.New("not collection")

// ErrNotExist is returned when the collection's requested element does not exist.
var ErrNotExist = errors.New("not exist")

// AsMapSlice retrieves the collection into a []map[string]interface{}.
// If recursive is true, then all subsequent Objects/ObjectsCollections are translated.
//
// This is horrendously inefficient, use it only as a guide!
func (O ObjectCollection) AsMapSlice(recursive bool) ([]map[string]interface{}, error) {
	length, err := O.Len()
	if err != nil {
		return nil, fmt.Errorf("Len: %w", err)
	}
	m := make([]map[string]interface{}, 0, length)
	for curr, err := O.First(); err == nil; curr, err = O.Next(curr) {
		if v, err := O.Get(curr); err != nil {
			return m, fmt.Errorf("Get(%v): %w", curr, err)
		} else if v == nil {
			m = append(m, nil)
		} else if o, ok := v.(*Object); ok {
			r, err := o.AsMap(recursive)
			if err != nil {
				return m, fmt.Errorf("[%d](%v).AsMap: %w", curr, v, err)
			}
			m = append(m, r)
		}
	}
	return m, nil
}

// FromMap populates the Object starting from a map, according to the Object's Attributes.
func (O *Object) FromMap(recursive bool, m map[string]interface{}) error {
	if O == nil || O.dpiObject == nil {
		return nil
	}
	logger := getLogger()

	for a, ot := range O.ObjectType.Attributes {
		v := m[a]
		if v == nil {
			continue
		}
		if logger != nil {
			logger.Log("msg", "FromMap", "attribute", a, "value", v, "type", fmt.Sprintf("%T", v), "recursive", recursive, "ot", ot.ObjectType)
		}
		if ot.ObjectType.CollectionOf != nil { // Collection case
			if err := func() error {
				coll, err := ot.NewCollection()
				if err != nil {
					return fmt.Errorf("%q.FromMap: %w", a, err)
				}
				defer coll.Close()
				switch v := v.(type) {
				case []map[string]interface{}:
					if err := coll.FromMapSlice(recursive, v); err != nil {
						return fmt.Errorf("%q.FromMapSlice: %w", a, err)
					}
				case []interface{}:
					if ot.IsObject() {
						m := make([]map[string]interface{}, 0, len(v))
						for _, e := range v {
							m = append(m, e.(map[string]interface{}))
						}
						if err := coll.FromMapSlice(recursive, m); err != nil {
							return fmt.Errorf("%q.FromMapSlice: %w", a, err)
						}
					} else {
						data := scratch.Get()
						defer scratch.Put(data)
						for _, e := range v {
							if err := data.Set(e); err != nil {
								return err
							}
							if err := coll.AppendData(data); err != nil {
								return err
							}
						}
					}
				default:
					return fmt.Errorf("%q is a collection, needs []interface{} or []map[string]interface{}, got %T", a, v)
				}
				if err = O.Set(a, coll); err != nil {
					return fmt.Errorf("%q.Set(%v): %w", a, coll, err)
				}
				return nil
			}(); err != nil {
				return err
			}
		} else if ot.ObjectType.Attributes != nil { // Object case
			newO, err := ot.NewObject()
			if err != nil {
				return fmt.Errorf("%q.FromMap: %w", a, err)
			}
			if err := newO.FromMap(recursive, v.(map[string]interface{})); err != nil {
				newO.Close()
				return fmt.Errorf("%q.FromMap: %w", a, err)
			}
			err = O.Set(a, newO)
			newO.Close()
			if err != nil {
				return fmt.Errorf("%q.Set(%v): %w", a, newO.ObjectType, err)
			}
		} else if err := O.Set(a, v); err != nil { // Plain type case
			return err
		}
	}
	return nil
}

func (O *Object) FromJSON(dec *json.Decoder) error {
	tok, err := dec.Token()
	if err != nil {
		if err == io.EOF {
			return nil
		}
		return err
	}
	logger := getLogger()
	wantDelim := tok == json.Delim('{')
	first := true
	for {
		if first && wantDelim || !first {
			first = false
			if tok, err = dec.Token(); err != nil {
				if err == io.EOF {
					break
				}
				return err
			}
		}
		k, ok := tok.(string)
		if !ok {
			return fmt.Errorf("wanted key (string), got %v (%T)", tok, tok)
		}
		k = strings.ToUpper(k)
		a, ok := O.ObjectType.Attributes[k]
		if logger != nil {
			logger.Log("attribute", k, "a", a)
		}
		if !ok {
			return fmt.Errorf("key %q not found", k)
		}
		var v interface{}
		var C func() error
		if a.ObjectType.CollectionOf != nil {
			coll, err := a.ObjectType.NewCollection()
			if err != nil {
				return fmt.Errorf("%q.%s.NewCollection: %w", k, a.ObjectType, err)
			}
			if err = coll.FromJSON(dec); err != nil {
				return fmt.Errorf("%q.FromJSON: %w", k, err)
			}
			v = coll
			C = coll.Close
		} else if a.ObjectType.IsObject() {
			obj, err := a.ObjectType.NewObject()
			if err != nil {
				return fmt.Errorf("%q.%s.NewObject: %w", k, a.ObjectType, err)
			}
			if err = obj.FromJSON(dec); err != nil {
				return fmt.Errorf("%q.FromJSON: %w", k, err)
			}
			v = obj
			C = obj.Close
		} else {
			if tok, err = dec.Token(); err != nil {
				return err
			}
			v = tok
		}
		err = O.Set(k, v)
		if C != nil {
			C()
		}
		if err != nil {
			return fmt.Errorf("%q.Set(%v): %w", k, v, err)
		}
		if !dec.More() {
			break
		}
	}
	if wantDelim {
		_, err = dec.Token()
	}
	return err
}

// FromMapSlice populates the ObjectCollection starting from a slice of map, according to the Collections's Attributes.
func (O ObjectCollection) FromMapSlice(recursive bool, m []map[string]interface{}) error {
	if O.dpiObject == nil {
		return nil
	}
	logger := getLogger()

	for i, o := range m {
		if logger != nil {
			logger.Log("msg", "FromMapSlice", "index", i, "recursive", recursive)
		}
		elt, err := O.ObjectType.CollectionOf.NewObject()
		if err != nil {
			return fmt.Errorf("%d.FromMapSlice: %w", i, err)
		}
		if err := elt.FromMap(recursive, o); err != nil {
			elt.Close()
			return fmt.Errorf("%d.FromMapSlice: %w", i, err)
		}
		err = O.Append(elt)
		elt.Close()
		if err != nil {
			return err
		}
	}
	return nil
}

func (O ObjectCollection) FromJSON(dec *json.Decoder) error {
	tok, err := dec.Token()
	if err != nil {
		if err == io.EOF {
			return nil
		}
		return err
	}
	wantDelim := tok == json.Delim('[')
	for {
		elt, err := O.ObjectType.CollectionOf.NewObject()
		if err != nil {
			return err
		}
		err = elt.FromJSON(dec)
		if err != nil {
			elt.Close()
			return err
		}
		err = O.Append(elt)
		elt.Close()
		if err != nil {
			return err
		}
		if !dec.More() {
			break
		}
	}
	if wantDelim {
		_, err = dec.Token()
	}
	return err
}

// AsSlice retrieves the collection into a slice.
func (O ObjectCollection) AsSlice(dest interface{}) (interface{}, error) {
	var dr reflect.Value
	needsInit := dest == nil
	if !needsInit {
		dr = reflect.ValueOf(dest)
	}
	d := scratch.Get()
	defer scratch.Put(d)
	for i, err := O.First(); err == nil; i, err = O.Next(i) {
		if O.CollectionOf.IsObject() {
			d.ObjectType = O.CollectionOf
		}
		if err = O.GetItem(d, i); err != nil {
			return dest, err
		}
		v := d.Get()
		if !d.IsObject() {
			v = maybeString(v, O.CollectionOf)
		}
		vr := reflect.ValueOf(v)
		if needsInit {
			needsInit = false
			length, lengthErr := O.Len()
			if lengthErr != nil {
				return dr.Interface(), lengthErr
			}
			dr = reflect.MakeSlice(reflect.SliceOf(vr.Type()), 0, length)
		}
		dr = reflect.Append(dr, vr)
	}
	return dr.Interface(), nil
}

// ToJSON writes the ObjectCollection as JSON to the io.Writer.
func (O ObjectCollection) ToJSON(w io.Writer) error {
	var notFirst bool
	bw := bufio.NewWriter(w)
	defer bw.Flush()
	if err := bw.WriteByte('['); err != nil {
		return err
	}
	for curr, err := O.First(); err == nil; curr, err = O.Next(curr) {
		if notFirst {
			if err = bw.WriteByte(','); err != nil {
				return err
			}
		} else {
			notFirst = true
		}
		if v, err := O.Get(curr); err != nil {
			return fmt.Errorf("Get(%v): %w", curr, err)
		} else if v == nil {
			if _, err = bw.WriteString("nil"); err != nil {
				return err
			}
		} else if o, ok := v.(*Object); ok {
			if err = o.ToJSON(bw); err != nil {
				return err
			}
		}
	}
	return bw.WriteByte(']')
}

func (O ObjectCollection) String() string {
	if O.Object == nil {
		return ""
	}
	var buf strings.Builder
	_, _ = buf.WriteString(O.Object.ObjectType.String())
	_ = O.ToJSON(&buf)
	return buf.String()
}

// AppendData to the collection.
func (O ObjectCollection) AppendData(data *Data) error {
	if err := O.drv.checkExec(func() C.int {
		return C.dpiObject_appendElement(O.dpiObject, data.NativeTypeNum, &data.dpiData)
	}); err != nil {
		return fmt.Errorf("append(%d): %w", data.NativeTypeNum, err)
	}
	return nil
}

// Append v to the collection.
func (O ObjectCollection) Append(v interface{}) error {
	if data, ok := v.(*Data); ok {
		return O.AppendData(data)
	}
	d := scratch.Get()
	defer scratch.Put(d)
	if err := d.Set(v); err != nil {
		return err
	}
	return O.AppendData(d)
}

// AppendObject adds an Object to the collection.
func (O ObjectCollection) AppendObject(obj *Object) error {
	d := scratch.Get()
	defer scratch.Put(d)
	d.ObjectType = obj.ObjectType
	d.NativeTypeNum = C.DPI_NATIVE_TYPE_OBJECT
	d.SetObject(obj)
	return O.Append(d)
}

// Delete i-th element of the collection.
func (O ObjectCollection) Delete(i int) error {
	if err := O.drv.checkExec(func() C.int {
		return C.dpiObject_deleteElementByIndex(O.dpiObject, C.int32_t(i))
	}); err != nil {
		return fmt.Errorf("delete(%d): %w", i, err)
	}
	return nil
}

// GetItem gets the i-th element of the collection into data.
func (O ObjectCollection) GetItem(data *Data, i int) error {
	if data == nil {
		panic("data cannot be nil")
	}

	runtime.LockOSThread()
	defer runtime.UnlockOSThread()

	idx := C.int32_t(i)
	var exists C.int
	if C.dpiObject_getElementExistsByIndex(O.dpiObject, idx, &exists) == C.DPI_FAILURE {
		return fmt.Errorf("exists(%d): %w", idx, O.drv.getError())
	}
	if exists == 0 {
		return ErrNotExist
	}
	data.reset()
	data.ObjectType = O.CollectionOf
	if O.CollectionOf != nil {
		data.NativeTypeNum = O.CollectionOf.NativeTypeNum
		data.implicitObj = true
	}
	if C.dpiObject_getElementValueByIndex(O.dpiObject, idx, data.NativeTypeNum, &data.dpiData) == C.DPI_FAILURE {
		return fmt.Errorf("get(%d[%d]): %w", idx, data.NativeTypeNum, O.drv.getError())
	}
	return nil
}

// Get the i-th element of the collection.
func (O ObjectCollection) Get(i int) (interface{}, error) {
	data := scratch.Get()
	defer scratch.Put(data)
	err := O.GetItem(data, i)
	return data.Get(), err
}

// SetItem sets the i-th element of the collection with data.
func (O ObjectCollection) SetItem(i int, data *Data) error {
	if err := O.drv.checkExec(func() C.int {
		return C.dpiObject_setElementValueByIndex(O.dpiObject, C.int32_t(i), data.NativeTypeNum, &data.dpiData)
	}); err != nil {
		return fmt.Errorf("set(%d[%d]): %w", i, data.NativeTypeNum, err)
	}
	return nil
}

// Set the i-th element of the collection with value.
func (O ObjectCollection) Set(i int, v interface{}) error {
	if data, ok := v.(*Data); ok {
		return O.SetItem(i, data)
	}
	d := scratch.Get()
	defer scratch.Put(d)
	if err := d.Set(v); err != nil {
		return err
	}
	return O.SetItem(i, d)
}

// First returns the first element's index of the collection.
func (O ObjectCollection) First() (int, error) {
	var exists C.int
	var idx C.int32_t
	if err := O.drv.checkExec(func() C.int {
		return C.dpiObject_getFirstIndex(O.dpiObject, &idx, &exists)
	}); err != nil {
		return 0, fmt.Errorf("first: %w", err)
	}
	if exists == 1 {
		return int(idx), nil
	}
	return 0, ErrNotExist
}

// Last returns the index of the last element.
func (O ObjectCollection) Last() (int, error) {
	var exists C.int
	var idx C.int32_t
	if err := O.drv.checkExec(func() C.int {
		return C.dpiObject_getLastIndex(O.dpiObject, &idx, &exists)
	}); err != nil {
		return 0, fmt.Errorf("last: %w", err)
	}
	if exists == 1 {
		return int(idx), nil
	}
	return 0, ErrNotExist
}

// Next returns the succeeding index of i.
func (O ObjectCollection) Next(i int) (int, error) {
	var exists C.int
	var idx C.int32_t
	if err := O.drv.checkExec(func() C.int {
		return C.dpiObject_getNextIndex(O.dpiObject, C.int32_t(i), &idx, &exists)
	}); err != nil {
		return 0, fmt.Errorf("next(%d): %w", i, err)
	}
	if exists == 1 {
		return int(idx), nil
	}
	return 0, ErrNotExist
}

// Len returns the length of the collection.
func (O ObjectCollection) Len() (int, error) {
	var size C.int32_t
	if err := O.drv.checkExec(func() C.int { return C.dpiObject_getSize(O.dpiObject, &size) }); err != nil {
		return 0, fmt.Errorf("len: %w", err)
	}
	return int(size), nil
}

// Trim the collection to n.
func (O ObjectCollection) Trim(n int) error {
	return O.drv.checkExec(func() C.int { return C.dpiObject_trim(O.dpiObject, C.uint32_t(n)) })
}

// ObjectType holds type info of an Object.
type ObjectType struct {
	CollectionOf                        *ObjectType
	Attributes                          map[string]ObjectAttribute
	drv                                 *drv
	dpiObjectType                       *C.dpiObjectType
	Schema, Name                        string
	DBSize, ClientSizeInBytes, CharSize int
	mu                                  sync.RWMutex
	OracleTypeNum                       C.dpiOracleTypeNum
	NativeTypeNum                       C.dpiNativeTypeNum
	Precision                           int16
	Scale                               int8
	FsPrecision                         uint8
}

// AttributeNames returns the Attributes' names ordered as on the database (by ObjectAttribute.Sequence).
func (t *ObjectType) AttributeNames() []string {
	names := make([]string, len(t.Attributes))
	for k, v := range t.Attributes {
		names[v.Sequence] = k
	}
	return names
}

func (t *ObjectType) String() string {
	if t.Schema == "" {
		return t.Name
	}
	return t.Schema + "." + t.Name
}

func (t *ObjectType) IsObject() bool { return t != nil && t.NativeTypeNum == C.DPI_NATIVE_TYPE_OBJECT }

// FullName returns the object's name with the schame prepended.
func (t *ObjectType) FullName() string {
	if t.Schema == "" {
		return t.Name
	}
	return t.Schema + "." + t.Name
}

// GetObjectType returns the ObjectType of a name.
//
// The name is uppercased! Because here Oracle seems to be case-sensitive.
// To leave it as is, enclose it in "-s!
func (c *conn) GetObjectType(name string) (*ObjectType, error) {
	if !strings.Contains(name, "\"") {
		name = strings.ToUpper(name)
	}
	c.mu.RLock()
	defer c.mu.RUnlock()
	if c.dpiConn == nil {
		return nil, driver.ErrBadConn
	}
	if t := c.objTypes[name]; t != nil {
		if t.drv != nil {
			return t, nil
		}
		// t is closed
		delete(c.objTypes, name)
	}

	cName := C.CString(name)
	defer func() { C.free(unsafe.Pointer(cName)) }()
	objType := (*C.dpiObjectType)(C.malloc(C.sizeof_void))
	if err := c.checkExec(func() C.int {
		return C.dpiConn_getObjectType(c.dpiConn, cName, C.uint32_t(len(name)), &objType)
	}); err != nil {
		C.free(unsafe.Pointer(objType))
		if strings.Contains(err.Error(), "DPI-1062: unexpected OCI return value 1041 in function dpiConn_getObjectType") {
			err = fmt.Errorf("getObjectType(%q) conn=%p: %+v: %w", name, c.dpiConn, err, driver.ErrBadConn)
			c.closeNotLocking()
			return nil, err
		}
		return nil, fmt.Errorf("getObjectType(%q) conn=%p: %w", name, c.dpiConn, err)
	}
	t := &ObjectType{drv: c.drv, dpiObjectType: objType}
	err := t.init()
	if err != nil {
		return t, err
	}
	c.objTypes[name] = t
	return t, nil
}

// NewObject returns a new Object with ObjectType type.
//
// As with all Objects, you MUST call Close on it when not needed anymore!
func (t *ObjectType) NewObject() (*Object, error) {
	logger := getLogger()
	if logger != nil {
		logger.Log("msg", "NewObject", "name", t.Name)
	}
	obj := (*C.dpiObject)(C.malloc(C.sizeof_void))
	t.mu.RLock()
	err := t.drv.checkExec(func() C.int { return C.dpiObjectType_createObject(t.dpiObjectType, &obj) })
	t.mu.RUnlock()
	if err != nil {
		C.free(unsafe.Pointer(obj))
		return nil, err
	}
	O := &Object{ObjectType: t, dpiObject: obj}
	if true {
		runtime.SetFinalizer(O, func(O *Object) {
			if O == nil || O.dpiObject == nil {
				return
			}
			fmt.Printf("WARN Object %v is not closed\n", O)
			O.Close()
		})
	}
	// https://github.com/oracle/odpi/issues/112#issuecomment-524479532
	return O, O.ResetAttributes()
}

// NewCollection returns a new Collection object with ObjectType type.
// If the ObjectType is not a Collection, it returns ErrNotCollection error.
func (t *ObjectType) NewCollection() (ObjectCollection, error) {
	if t.CollectionOf == nil {
		return ObjectCollection{}, ErrNotCollection
	}
	O, err := t.NewObject()
	if err != nil {
		return ObjectCollection{}, err
	}
	return ObjectCollection{Object: O}, nil
}

// Close releases a reference to the object type.
func (t *ObjectType) Close() error {
	if t == nil {
		return nil
	}
	t.mu.Lock()
	defer t.mu.Unlock()
	attributes, cof, d, conn := t.Attributes, t.CollectionOf, t.dpiObjectType, t.drv
	t.Attributes, t.CollectionOf, t.dpiObjectType, t.drv = nil, nil, nil, nil

	if d == nil {
		return nil
	}

	logger := getLogger()
	if cof != nil {
		if err := cof.Close(); err != nil && logger != nil {
			logger.Log("msg", "ObjectType.Close CollectionOf.Close", "name", t.Name, "collectionOf", cof.Name, "error", err)
		}
	}

	for _, attr := range attributes {
		if err := attr.Close(); err != nil && logger != nil {
			logger.Log("msg", "ObjectType.Close attr.Close", "name", t.Name, "attr", attr.Name, "error", err)
		}
	}

	if logger != nil {
		logger.Log("msg", "ObjectType.Close", "name", t.Name)
	}
	if err := conn.checkExec(func() C.int { return C.dpiObjectType_release(d) }); err != nil {
		return fmt.Errorf("error on close object type: %w", err)
	}

	return nil
}

func wrapObject(c *conn, objectType *C.dpiObjectType, object *C.dpiObject) (*Object, error) {
	if objectType == nil {
		return nil, errors.New("objectType is nil")
	}
	if err := c.checkExec(func() C.int { return C.dpiObject_addRef(object) }); err != nil {
		return nil, err
	}
	o := &Object{
		ObjectType: &ObjectType{dpiObjectType: objectType, drv: c.drv},
		dpiObject:  object,
	}
	return o, o.init()
}

func (t *ObjectType) init() error {
	if t.drv == nil {
		panic("conn is nil")
	}
	if t.Name != "" && t.Attributes != nil {
		return nil
	}

	runtime.LockOSThread()
	defer runtime.UnlockOSThread()

	t.mu.RLock()
	d := t.dpiObjectType
	t.mu.RUnlock()
	if d == nil {
		return nil
	}
	var info C.dpiObjectTypeInfo
	if C.dpiObjectType_getInfo(d, &info) == C.DPI_FAILURE {
		return fmt.Errorf("%v.getInfo: %w", t, t.drv.getError())
	}
	t.Schema = C.GoStringN(info.schema, C.int(info.schemaLength))
	t.Name = C.GoStringN(info.name, C.int(info.nameLength))
	t.CollectionOf = nil

	numAttributes := int(info.numAttributes)
	if info.isCollection == 1 {
		t.CollectionOf = &ObjectType{drv: t.drv}
		if err := t.CollectionOf.fromDataTypeInfo(info.elementTypeInfo); err != nil {
			return err
		}
		if t.CollectionOf.Name == "" {
			t.CollectionOf.Schema = t.Schema
			t.CollectionOf.Name = t.Name
		}
		if t.CollectionOf.dpiObjectType != nil {
			C.dpiObjectType_addRef(t.CollectionOf.dpiObjectType)
		}
	}
	if numAttributes == 0 {
		t.Attributes = map[string]ObjectAttribute{}
		return nil
	}
	t.Attributes = make(map[string]ObjectAttribute, numAttributes)
	attrs := make([]*C.dpiObjectAttr, numAttributes)
	if C.dpiObjectType_getAttributes(d,
		C.uint16_t(len(attrs)),
		(**C.dpiObjectAttr)(unsafe.Pointer(&attrs[0])),
	) == C.DPI_FAILURE {
		return fmt.Errorf("%v.getAttributes: %w", t, t.drv.getError())
	}
	logger := getLogger()
	for i, attr := range attrs {
		var attrInfo C.dpiObjectAttrInfo
		if C.dpiObjectAttr_getInfo(attr, &attrInfo) == C.DPI_FAILURE {
			return fmt.Errorf("%v.attr_getInfo: %w", attr, t.drv.getError())
		}
		if logger != nil {
			logger.Log("i", i, "attrInfo", attrInfo)
		}
		typ := attrInfo.typeInfo
		sub, err := objectTypeFromDataTypeInfo(t.drv, typ)
		if err != nil {
			return err
		}
		objAttr := ObjectAttribute{
			dpiObjectAttr: attr,
			Name:          C.GoStringN(attrInfo.name, C.int(attrInfo.nameLength)),
			ObjectType:    sub,
			Sequence:      uint32(i),
		}
		if sub.dpiObjectType != nil {
			C.dpiObjectType_addRef(sub.dpiObjectType)
		}
		//fmt.Printf("%d=%q. typ=%+v sub=%+v\n", i, objAttr.Name, typ, sub)
		t.Attributes[objAttr.Name] = objAttr
	}

	if false {
		runtime.SetFinalizer(t, func(t *ObjectType) { t.Close() })
	}
	return nil
}

func (t *ObjectType) fromDataTypeInfo(typ C.dpiDataTypeInfo) error {
	t.dpiObjectType = typ.objectType

	t.OracleTypeNum = typ.oracleTypeNum
	t.NativeTypeNum = typ.defaultNativeTypeNum
	t.DBSize = int(typ.dbSizeInBytes)
	t.ClientSizeInBytes = int(typ.clientSizeInBytes)
	t.CharSize = int(typ.sizeInChars)
	t.Precision = int16(typ.precision)
	t.Scale = int8(typ.scale)
	t.FsPrecision = uint8(typ.fsPrecision)
	return t.init()
}
func objectTypeFromDataTypeInfo(d *drv, typ C.dpiDataTypeInfo) (*ObjectType, error) {
	if d == nil {
		panic("drv is nil")
	}
	if typ.oracleTypeNum == 0 {
		panic("typ is nil")
	}
	t := &ObjectType{drv: d}
	err := t.fromDataTypeInfo(typ)
	return t, err
}

// ObjectAttribute is an attribute of an Object.
type ObjectAttribute struct {
	*ObjectType
	dpiObjectAttr *C.dpiObjectAttr
	Name          string
	Sequence      uint32
}

// Close the ObjectAttribute.
func (A ObjectAttribute) Close() error {
	if A.dpiObjectAttr == nil {
		return nil
	}
	logger := getLogger()
	if logger != nil {
		logger.Log("msg", "ObjectAttribute.Close", "name", A.Name)
	}
	if err := A.ObjectType.drv.checkExec(func() C.int { return C.dpiObjectAttr_release(A.dpiObjectAttr) }); err != nil {
		return err
	}
	return A.ObjectType.Close()
}

// GetObjectType returns the ObjectType for the name.
func GetObjectType(ctx context.Context, ex Execer, typeName string) (*ObjectType, error) {
	c, err := getConn(ctx, ex)
	if err != nil {
		return nil, fmt.Errorf("getConn for %s: %w", typeName, err)
	}
	return c.GetObjectType(typeName)
}

var scratch = &dataPool{Pool: sync.Pool{New: func() interface{} { return &Data{} }}}

type dataPool struct{ sync.Pool }

func (dp *dataPool) Get() *Data  { return dp.Pool.Get().(*Data) }
func (dp *dataPool) Put(d *Data) { d.reset(); dp.Pool.Put(d) }
