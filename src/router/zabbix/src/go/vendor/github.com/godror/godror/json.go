package godror

/*
#include <stdlib.h>
#include "dpiImpl.h"

// All CGO functions are used because handling unions, indexing,
// nested callocs for substructures is hard to handle in Go code

void godror_allocate_dpiNode(dpiJsonNode **dpijsonnode) {
	*dpijsonnode = (dpiJsonNode *)(calloc(1, sizeof(dpiJsonNode)));
	dpiDataBuffer *dpijsonDataBuffer = (dpiDataBuffer *)(calloc(1, sizeof(dpiDataBuffer)));
	(*dpijsonnode)->value = dpijsonDataBuffer;
}

void godror_free_dpiNode(dpiJsonNode *node) {
    if(node == NULL) {
        return;
    }
    if (node->value) {
        free(node->value);
        node->value = NULL;
    }
    free(node);
    node = NULL;
}

void godror_setObjectFields(dpiJsonObject * jsonobj, int i, dpiJsonNode **jnode)
{
    *jnode = &(jsonobj->fields[i]);
    jsonobj->fields[i].value = &jsonobj->fieldValues[i];
}

void godror_dpiJsonObject_setKey(dpiJsonNode *dpijsonnode, int index, const char *key, uint32_t keyLength) {
    dpiJsonObject *dpijsonobj = &(dpijsonnode->value->asJsonObject);
    dpijsonobj->fieldNames[index] = calloc((keyLength + 1), sizeof(char));
    memcpy(dpijsonobj->fieldNames[index], key, keyLength);
    dpijsonobj->fieldNames[index][keyLength] = '\0';
    dpijsonobj->fieldNameLengths[index] = keyLength;
}

void godror_dpiasJsonObject(dpiJsonNode *dpijsonnode, dpiJsonObject **dpijsonobj)
{
    *dpijsonobj = &(dpijsonnode->value->asJsonObject);
}

void godror_dpiasJsonArray(dpiJsonNode *dpijsonnode, dpiJsonArray **dpijsonobj)
{
    *dpijsonobj = &(dpijsonnode->value->asJsonArray);
}

void godror_setArrayElements(dpiJsonArray * jsonarr, int i, dpiJsonNode **jnode)
{
    *jnode = &(jsonarr->elements[i]);
    jsonarr->elements[i].value = &jsonarr->elementValues[i];
}

void godror_dpiJson_setDouble(dpiJsonNode *topNode, double value) {
    topNode->oracleTypeNum = DPI_ORACLE_TYPE_NUMBER;
    topNode->nativeTypeNum = DPI_NATIVE_TYPE_DOUBLE;
    topNode->value->asDouble = value;
}

void godror_dpiJson_setInt64(dpiJsonNode *topNode, int64_t value) {
    topNode->oracleTypeNum = DPI_ORACLE_TYPE_NUMBER;
    topNode->nativeTypeNum = DPI_NATIVE_TYPE_INT64;
    topNode->value->asInt64 = value;
}

void godror_dpiJson_setUint64(dpiJsonNode *topNode, uint64_t value) {
    topNode->oracleTypeNum = DPI_ORACLE_TYPE_NUMBER;
    topNode->nativeTypeNum = DPI_NATIVE_TYPE_UINT64;
    topNode->value->asUint64 = value;
}

void godror_dpiJson_setTime(dpiJsonNode *topNode, dpiData *data) {
    topNode->oracleTypeNum = DPI_ORACLE_TYPE_TIMESTAMP;
    topNode->nativeTypeNum = DPI_NATIVE_TYPE_TIMESTAMP;
    topNode->value->asTimestamp = data->value.asTimestamp;
}

void godror_dpiJson_setBytes(dpiJsonNode *topNode, dpiData *data) {
    uint32_t size = data->value.asBytes.length;
    topNode->oracleTypeNum = DPI_ORACLE_TYPE_RAW;
    topNode->nativeTypeNum = DPI_NATIVE_TYPE_BYTES;
    topNode->value->asBytes.ptr = calloc(1, size);
    memcpy(topNode->value->asBytes.ptr, data->value.asBytes.ptr, size);
    topNode->value->asBytes.length = size;
}

void godror_dpiJson_setNumber(dpiJsonNode *topNode, const _GoString_ value) {
    uint32_t length;
    length = _GoStringLen(value);
    topNode->oracleTypeNum = DPI_ORACLE_TYPE_NUMBER;
    topNode->nativeTypeNum = DPI_NATIVE_TYPE_BYTES;
    topNode->value->asBytes.ptr = calloc(1, length);
    memcpy(topNode->value->asBytes.ptr, _GoStringPtr(value), length);
    topNode->value->asBytes.length = length;
}

void godror_dpiJson_setIntervalDS(dpiJsonNode *topNode, dpiData *data) {
    topNode->oracleTypeNum = DPI_ORACLE_TYPE_INTERVAL_DS;
    topNode->nativeTypeNum = DPI_NATIVE_TYPE_INTERVAL_DS;
    topNode->value->asIntervalDS = data->value.asIntervalDS;
}

void godror_dpiJson_setBool(dpiJsonNode *topNode, dpiData *data) {
    topNode->oracleTypeNum = DPI_ORACLE_TYPE_BOOLEAN;
    topNode->nativeTypeNum = DPI_NATIVE_TYPE_BOOLEAN;
    topNode->value->asBoolean = data->value.asBoolean;
}

void godror_dpiJson_setString(dpiJsonNode *topNode, dpiData *data) {
    uint32_t size = data->value.asBytes.length;
    topNode->oracleTypeNum = DPI_ORACLE_TYPE_VARCHAR;
    topNode->nativeTypeNum = DPI_NATIVE_TYPE_BYTES;
    // make a copy before passing to C?
    topNode->value->asBytes.ptr = calloc(1, size);
    memcpy(topNode->value->asBytes.ptr, data->value.asBytes.ptr, size);
    topNode->value->asBytes.length = size;
}

void godror_dpiJsonObject_initialize(dpiJsonNode **dpijsonnode, uint32_t numfields) {
    dpiJsonObject dpijsonobjtmp;
    dpiJsonObject *dpijsonobj = &dpijsonobjtmp;
    (*dpijsonnode)->oracleTypeNum = DPI_ORACLE_TYPE_JSON_OBJECT;
    (*dpijsonnode)->nativeTypeNum = DPI_NATIVE_TYPE_JSON_OBJECT;
    dpijsonobj->fieldNames = (calloc(numfields,  sizeof(char *)));
    dpijsonobj->fields = (dpiJsonNode *)(calloc(numfields, sizeof(dpiJsonNode)));
    dpijsonobj->fieldNameLengths = calloc(numfields, sizeof(uint32_t));
    dpijsonobj->fieldValues = (dpiDataBuffer *)calloc(numfields, sizeof(dpiDataBuffer));
	dpijsonobj->numFields = numfields;
    (*dpijsonnode)->value->asJsonObject = *dpijsonobj;
}

void godror_dpiJsonArray_initialize(dpiJsonNode **dpijsonnode, uint32_t numelem) {
    dpiJsonArray dpijsonarrtmp;
    dpiJsonArray *dpijsonarr = &dpijsonarrtmp;
    (*dpijsonnode)->oracleTypeNum = DPI_ORACLE_TYPE_JSON_ARRAY;
    (*dpijsonnode)->nativeTypeNum = DPI_NATIVE_TYPE_JSON_ARRAY;
    dpijsonarr->elements = calloc(numelem, sizeof(dpiJsonNode));
    dpijsonarr->elementValues = (dpiDataBuffer *)calloc(numelem, sizeof(dpiDataBuffer));
	dpijsonarr->numElements = numelem;
    (*dpijsonnode)->value->asJsonArray = *dpijsonarr;
}

void godror_dpiJsonNodeFree(dpiJsonNode *node)
{
    dpiJsonArray *array;
    dpiJsonObject *obj;
    uint32_t i;

    if (node == NULL)
    {
        return;
    }

    switch (node->nativeTypeNum) {
        case DPI_NATIVE_TYPE_BYTES:
            if(node->value->asBytes.ptr) {
                free(node->value->asBytes.ptr);
                node->value->asBytes.ptr = NULL;
            }
            break;
        case DPI_NATIVE_TYPE_JSON_ARRAY:
            array = &node->value->asJsonArray;
            if (array->elements) {
                for (i = 0; i < array->numElements; i++) {
                    if (array->elements[i].value) {
                        godror_dpiJsonNodeFree(&array->elements[i]);
                    }
                }
                free(array->elements);
                array->elements = NULL;
            }
            if (array->elementValues) {
                free(array->elementValues);
                array->elementValues = NULL;
            }
            break;
        case DPI_NATIVE_TYPE_JSON_OBJECT:
            obj = &node->value->asJsonObject;
            if (obj->fields) {
                for (i = 0; i < obj->numFields; i++) {
                    if (obj->fields[i].value)
                        godror_dpiJsonNodeFree(&obj->fields[i]);
                }
                free(obj->fields);
                obj->fields = NULL;
            }
            if (obj->fieldNames) {
                for (i = 0; i < obj->numFields; i++) {
                    if (obj->fieldNames[i]) {
                        free(obj->fieldNames[i]);
                        obj->fieldNames[i] = NULL;
                    }
                }
                free(obj->fieldNames);
                obj->fieldNames = NULL;
            }
            if (obj->fieldNameLengths) {
                free(obj->fieldNameLengths);
                obj->fieldNameLengths = NULL;
            }
            if (obj->fieldValues) {
                free(obj->fieldValues);
                obj->fieldValues = NULL;
            }
            break;
    }
}

void godror_dpiJsonfreeMem(dpiJsonNode *node) {
    if (node == NULL)
    {
        return;
    }
    godror_dpiJsonNodeFree(node);
    godror_free_dpiNode(node);
}

*/
import "C"

import (
	"encoding/json"
	"errors"
	"fmt"
	"reflect"
	"time"
	"unsafe"
)

// JSONOption provides an option to retrieve scalar values
// from JSON tree.
//
// DPI_JSON_OPT_NUMBER_AS_STRING - returns value stored as NUMBER in DB
// as godror.Number.
// DPI_JSON_OPT_DEAFULT - returns value stored as NUMBER in DB as float64.
type JSONOption uint8

var ErrInvalidJSON = errors.New("invalid JSON Document")
var ErrInvalidType = errors.New("invalid JSON Scalar Type")

const (
	JSONOptDefault        = JSONOption(C.DPI_JSON_OPT_DEFAULT)
	JSONOptNumberAsString = JSONOption(C.DPI_JSON_OPT_NUMBER_AS_STRING)
)

// JSON holds the JSON data to/from Oracle.
// It is like a root node in JSON tree.
type JSON struct {
	dpiJson *C.dpiJson
}

// Get retrieves the data stored in JSON based on option, opts.
func (j JSON) Get(data *Data, opts JSONOption) error {
	var node *C.dpiJsonNode
	if C.dpiJson_getValue(j.dpiJson, C.uint32_t(opts), (**C.dpiJsonNode)(unsafe.Pointer(&node))) == C.DPI_FAILURE {
		return ErrInvalidJSON
	}
	jsonNodeToData(data, node)
	return nil
}

// GetJSONObject retrieves JSONObject from JSON based on option, opts.
// It returns error if JSON doesnt represent object type.
func (j JSON) GetJSONObject(opts JSONOption) (JSONObject, error) {
	var node *C.dpiJsonNode
	var d Data
	if C.dpiJson_getValue(j.dpiJson, C.uint32_t(opts), (**C.dpiJsonNode)(unsafe.Pointer(&node))) == C.DPI_FAILURE {
		return JSONObject{}, ErrInvalidJSON
	}
	jsonNodeToData(&d, node)
	if C.dpiOracleTypeNum(node.oracleTypeNum) != C.DPI_ORACLE_TYPE_JSON_OBJECT {
		return JSONObject{}, ErrInvalidType
	}
	return JSONObject{dpiJsonObject: C.dpiData_getJsonObject(&(d.dpiData))}, nil
}

// GetJSONArray retrieves JSONArray from JSON based on option, opts.
// It returns error if JSON doesnt represent array type.
func (j JSON) GetJSONArray(opts JSONOption) (JSONArray, error) {
	var node *C.dpiJsonNode
	if C.dpiJson_getValue(j.dpiJson, C.uint32_t(opts), (**C.dpiJsonNode)(unsafe.Pointer(&node))) == C.DPI_FAILURE {
		return JSONArray{}, ErrInvalidJSON
	}
	var d Data
	jsonNodeToData(&d, node)
	if C.dpiOracleTypeNum(node.oracleTypeNum) != C.DPI_ORACLE_TYPE_JSON_ARRAY {
		return JSONArray{}, ErrInvalidType
	}
	return JSONArray{dpiJsonArray: C.dpiData_getJsonArray(&(d.dpiData))}, nil
}

// GetJSONScalar retrieves JSONScalar from JSON based on option, opts.
func (j JSON) GetJSONScalar(opts JSONOption) (JSONScalar, error) {
	var node *C.dpiJsonNode
	if C.dpiJson_getValue(j.dpiJson, C.uint32_t(opts), (**C.dpiJsonNode)(unsafe.Pointer(&node))) == C.DPI_FAILURE {
		return JSONScalar{}, ErrInvalidJSON
	}
	return JSONScalar{dpiJsonNode: node}, nil
}

// GetValue converts the native DB type stored in JSON into an interface value.
// The scalar values stored in JSON get converted as below.
//      map[string]interface{}, for JSON object type
//      []interface{}, for JSON arrays
//      godror.Number or float64 based on options for NUMBER
//      bool , for boolean
//      byte[], for RAW
//      time.Duration, for INTERVAL DAY TO SECOND
//      time.Time, for TIMESTAMP
//      string, for VARCHAR2(string)
func (j JSON) GetValue(opts JSONOption) (interface{}, error) {
	jScalar, err := j.GetJSONScalar(opts)
	logger := getLogger()
	if err != nil {
		if logger != nil {
			logger.Log("msg", "JSON.GetValue", "Error", err.Error())
		}
		return nil, err
	}
	val, err := jScalar.GetValue()
	if err != nil {
		if logger != nil {
			logger.Log("msg", "JSON.GetValue", "Error", err.Error())
		}
		return nil, err
	}
	return val, nil
}

// String returns standard JSON formatted string
func (j JSON) String() string {
	// json library is used, it will be removed
	// with direct call to get JSON string from JSON.
	// Returning empty string for error case, fix?

	jScalar, err := j.GetJSONScalar(JSONOptNumberAsString)
	logger := getLogger()
	if err != nil {
		if logger != nil {
			logger.Log("msg", "JSON.String", "Error", err.Error())
		}
		return ""
	}
	jScalarVal, err := jScalar.GetValue()
	if err != nil {
		if logger != nil {
			logger.Log("msg", "JSON.String", "Error", err.Error())
		}
		return ""
	}
	data, err := json.Marshal(jScalarVal)
	if err != nil {
		if logger != nil {
			logger.Log("msg", "JSON.String", "Error", err.Error())
		}
		return ""
	}
	return string(data)
}

// jsonNodeToData gets the data from dpiJsonNode
func jsonNodeToData(data *Data, node *C.dpiJsonNode) {
	if node.value == nil {
		data.dpiData.isNull = 1
		return
	}
	data.dpiData.value = *node.value
	data.NativeTypeNum = node.nativeTypeNum
}

// JSONStringFlags represents the input JSON string format.
// It can be standard JSON format or it can include ORACLE extended types,
// BSON extended types.
type JSONStringFlags uint

// JSONString encapsulates JSON formatted string.
type JSONString struct {
	Flags JSONStringFlags // standard , extended types for OSON, BSON
	Value string          // JSON input
}

// JSONValue indicates the input bind value provided for DB column type JSON.
// Valid inputs: int, int8, int16, int32, int64, uint, uint8, uint16,
// uint32, uint64, float32, float64, string, map, array, string and bool.
//
// for  int, int8, int16, int32, int64, uint, uint8, uint16, uint32,
//      uint64, float32, float64; DB native type NUMBER is used.
// for  string; DB native type VARCHAR2 is used.
// for  time.Time; DB native type TIMESTAMP is used.
// for  time.Duration; DB native type INTERVAL DAY TO SECOND is used.
// for  []byte; DB native type RAW is used.
// for  bool; DB native type boolean is used.
// for  map[string]interface{}; DB type JSON Object is used.
// for  []interface{}; DB native type JSON Array is used.

type JSONValue struct {
	Value interface{}
}

// JSONScalar holds the JSON data to/from Oracle.
// It includes all scalar values such as int, int8, int16, int32, int64,
// uint, uint8, uint16, uint32, uint64, float32, float64, string,
// map, array, string, byte[], time.Time, time.Duration, godror.Number and bool.
type JSONScalar struct {
	dpiJsonNode *C.dpiJsonNode
}

// GetValue converts native DB type stored in JSONScalar to an interface value.
// The scalar value stored in JSONScalar gets converted as below.
//      map[string]interface{}, for JSON object type
//      []interface{}, for JSON arrays
//      godror.Number or float64 based on options for NUMBER
//      bool , for JSON boolean
//      byte[], for JSON RAW
//      time.Duration, for INTERVAL DAY TO SECOND
//      time.Time, for TIMESTAMP
//      string, for VARCHAR2(string)
func (j JSONScalar) GetValue() (val interface{}, err error) {
	var d Data
	jsonNodeToData(&d, j.dpiJsonNode)
	if j.dpiJsonNode.oracleTypeNum == C.DPI_ORACLE_TYPE_NUMBER {
		val = getJSONScalarNumber(d)
	} else if j.dpiJsonNode.oracleTypeNum == C.DPI_ORACLE_TYPE_VARCHAR {
		val, err = getJSONScalarString(d)
	} else {
		val = d.Get()
		if j.dpiJsonNode.oracleTypeNum == C.DPI_ORACLE_TYPE_JSON_OBJECT {
			jobj := val.(JSONObject)
			val, err = jobj.GetValue()
		} else if j.dpiJsonNode.oracleTypeNum == C.DPI_ORACLE_TYPE_JSON_ARRAY {
			jarr := val.(JSONArray)
			val, err = jarr.GetValue()
		} else {
			err = nil
		}
	}
	return
}

// getJSONScalarNumber returns DB NUMBER as godror.Number for option,
// JSONOptNumberAsString and float64 for otpion, JSONOptDefault.
func getJSONScalarNumber(d Data) (val interface{}) {
	b := d.Get()
	if d.NativeTypeNum == C.DPI_NATIVE_TYPE_BYTES {
		val = Number(string(b.([]byte)))
	} else {
		val = b.(float64)
	}
	return
}

//getJSONScalarString converts the byte array of VARCHAR2 to string
func getJSONScalarString(d Data) (string, error) {
	b := d.Get()
	return string(b.([]byte)), nil
}

// JSONArray represents the array input.
type JSONArray struct {
	dpiJsonArray *C.dpiJsonArray
}

// Len returns the number of elements in the JSONArray.
func (j JSONArray) Len() int { return int(j.dpiJsonArray.numElements) }

// GetElement returns the ith element in JSONArray as data.
func (j JSONArray) GetElement(i int) Data {
	elts := jsonArraySlice(j.dpiJsonArray)
	var d Data
	jsonNodeToData(&d, &elts[i])
	return d

}

// Get returns the data array from JSONArray
func (j JSONArray) Get(nodes []Data) []Data {
	elts := jsonArraySlice(j.dpiJsonArray)
	for i := range elts {
		var d Data
		jsonNodeToData(&d, &elts[i])
		nodes = append(nodes, d)
	}
	return nodes
}

// GetValue converts native DB type, array into []interface{}.
func (j JSONArray) GetValue() (nodes []interface{}, err error) {
	elts := jsonArraySlice(j.dpiJsonArray)
	for i := range elts {
		var d Data
		jsonNodeToData(&d, &elts[i])
		if d.NativeTypeNum == C.DPI_NATIVE_TYPE_JSON_OBJECT {

			jsobj := JSONObject{dpiJsonObject: C.dpiData_getJsonObject(&(d.dpiData))}
			m, err := jsobj.GetValue()
			if err != nil {
				return nil, err
			}
			nodes = append(nodes, m)
		} else if d.NativeTypeNum == C.DPI_NATIVE_TYPE_JSON_ARRAY {
			jsarr := JSONArray{dpiJsonArray: C.dpiData_getJsonArray(&(d.dpiData))}
			ua, err := jsarr.GetValue()
			if err != nil {
				return nil, err
			}
			nodes = append(nodes, ua)
		} else {
			if elts[i].oracleTypeNum == C.DPI_ORACLE_TYPE_VARCHAR {
				keyval, err := getJSONScalarString(d)
				if err == nil {
					nodes = append(nodes, keyval)
				} else {
					return nil, err
				}
			} else if elts[i].oracleTypeNum == C.DPI_ORACLE_TYPE_NUMBER {
				nodes = append(nodes, getJSONScalarNumber(d))
			} else {
				nodes = append(nodes, d.Get())
			}
		}
	}
	return nodes, nil
}

func jsonArraySlice(arr *C.dpiJsonArray) []C.dpiJsonNode {
	n := int(arr.numElements)
	return ((*[maxArraySize]C.dpiJsonNode)(unsafe.Pointer(arr.elements)))[:n:n]
}

// JSONObject represents the map input.
type JSONObject struct {
	dpiJsonObject *C.dpiJsonObject
}

// Len returns the number of keys in the JSONObject
func (j JSONObject) Len() int { return int(j.dpiJsonObject.numFields) }

// Get returns the map, map[string]Data from JSONObject
func (j JSONObject) Get() map[string]Data {
	ff := jsonObjectFields(j.dpiJsonObject)
	m := make(map[string]Data, len(ff))
	for _, f := range ff {
		var d Data
		jsonNodeToData(&d, f.Value)
		m[f.Name] = d
	}
	return m
}

// GetValue converts native DB type, array into map[string]interface{}.
func (j JSONObject) GetValue() (m map[string]interface{}, err error) {
	m = make(map[string]interface{})
	for _, f := range jsonObjectFields(j.dpiJsonObject) {
		var d Data
		jsonNodeToData(&d, f.Value)
		if d.NativeTypeNum == C.DPI_NATIVE_TYPE_JSON_OBJECT {
			jsobj := JSONObject{dpiJsonObject: C.dpiData_getJsonObject(&(d.dpiData))}
			um, err := jsobj.GetValue()
			if err != nil {
				return nil, err
			}
			m[f.Name] = um
		} else if d.NativeTypeNum == C.DPI_NATIVE_TYPE_JSON_ARRAY {
			jsobj := JSONArray{dpiJsonArray: C.dpiData_getJsonArray(&(d.dpiData))}
			ua, err := jsobj.GetValue()
			if err != nil {
				return nil, err
			}
			m[f.Name] = ua
		} else if f.Value.oracleTypeNum == C.DPI_ORACLE_TYPE_VARCHAR {
			keyval, err := getJSONScalarString(d)
			if err == nil {
				m[f.Name] = keyval
			} else {
				return nil, err
			}
		} else if f.Value.oracleTypeNum == C.DPI_ORACLE_TYPE_NUMBER {
			m[f.Name] = getJSONScalarNumber(d)
		} else {
			m[f.Name] = d.Get()
		}
	}
	return m, nil
}

// GetInto takes pointer to struct and populates the fields.
// The struct name fields are matched with DB JSON keynames but
// not the struct json tags.
func (j JSONObject) GetInto(v interface{}) {
	rv := reflect.ValueOf(v).Elem()
	for _, f := range jsonObjectFields(j.dpiJsonObject) {
		var d Data
		jsonNodeToData(&d, f.Value)
		rv.FieldByName(f.Name).Set(reflect.ValueOf(d.Get()))
	}
}

type jsonField struct {
	Name  string
	Value *C.dpiJsonNode
}

func jsonObjectFields(obj *C.dpiJsonObject) []jsonField {
	n := int(obj.numFields)
	names := ((*[maxArraySize]*C.char)(unsafe.Pointer(obj.fieldNames)))[:n:n]
	nameLengths := ((*[maxArraySize]C.uint32_t)(unsafe.Pointer(obj.fieldNameLengths)))[:n:n]
	fields := ((*[maxArraySize]C.dpiJsonNode)(unsafe.Pointer(obj.fields)))[:n:n]
	ff := make([]jsonField, n)
	for i := range fields {
		ff[i] = jsonField{
			Name:  C.GoStringN(names[i], C.int(nameLengths[i])),
			Value: &fields[i],
		}
	}
	return ff
}

// populateJSONNode populates dpiJsonNode from user inputs.
// It creates a seperate memory for the new output value, jsonnode.
// memory from user input, in is not shared with jsonnode.
// Caller has to explicitly free using godror_dpiJsonfreeMem
func populateJSONNode(jsonnode *C.dpiJsonNode, in interface{}) error {
	switch x := in.(type) {
	case []interface{}:
		arr, _ := in.([]interface{})
		C.godror_dpiJsonArray_initialize((**C.dpiJsonNode)(unsafe.Pointer(&jsonnode)), C.uint32_t(len(arr)))

		var dpijsonarr *C.dpiJsonArray
		C.godror_dpiasJsonArray(jsonnode, (**C.dpiJsonArray)(unsafe.Pointer(&dpijsonarr)))
		for index, entry := range arr {
			var jsonnodelocal *C.dpiJsonNode
			C.godror_setArrayElements(dpijsonarr, C.int(index), (**C.dpiJsonNode)(unsafe.Pointer(&jsonnodelocal)))
			err := populateJSONNode(jsonnodelocal, entry)
			if err != nil {
				return err
			}
		}
	case map[string]interface{}:
		m, _ := in.(map[string]interface{})
		// Initialize dpiJsonObjectNode
		C.godror_dpiJsonObject_initialize((**C.dpiJsonNode)(unsafe.Pointer(&jsonnode)), C.uint32_t(len(m)))

		var dpijsonobj *C.dpiJsonObject
		C.godror_dpiasJsonObject(jsonnode, (**C.dpiJsonObject)(unsafe.Pointer(&dpijsonobj)))

		var i C.int = 0
		var cKey *C.char

		for k, v := range m {
			cKey = C.CString(k)
			C.godror_dpiJsonObject_setKey(jsonnode, i, cKey, C.uint32_t(len(k)))
			var jsonnodelocal *C.dpiJsonNode
			C.free(unsafe.Pointer(cKey))
			C.godror_setObjectFields(dpijsonobj, i, (**C.dpiJsonNode)(unsafe.Pointer(&jsonnodelocal)))
			err := populateJSONNode(jsonnodelocal, v)
			if err != nil {
				return err
			}
			i = i + 1
		}

	case int:
		C.godror_dpiJson_setInt64(jsonnode, C.int64_t(x))
	case int8:
		C.godror_dpiJson_setInt64(jsonnode, C.int64_t(x))
	case int16:
		C.godror_dpiJson_setInt64(jsonnode, C.int64_t(x))
	case int32:
		C.godror_dpiJson_setInt64(jsonnode, C.int64_t(x))
	case int64:
		C.godror_dpiJson_setInt64(jsonnode, C.int64_t(x))
	case uint:
		C.godror_dpiJson_setUint64(jsonnode, C.uint64_t(x))
	case uint8:
		C.godror_dpiJson_setUint64(jsonnode, C.uint64_t(x))
	case uint16:
		C.godror_dpiJson_setUint64(jsonnode, C.uint64_t(x))
	case uint32:
		C.godror_dpiJson_setUint64(jsonnode, C.uint64_t(x))
	case uint64:
		C.godror_dpiJson_setUint64(jsonnode, C.uint64_t(x))
	case float32:
		C.godror_dpiJson_setDouble(jsonnode, C.double(x))
	case float64:
		C.godror_dpiJson_setDouble(jsonnode, C.double(x))
	case Number:
		C.godror_dpiJson_setNumber(jsonnode, x.String())
	case string:
		data, err := NewData(x)
		if err != nil {
			return err
		}
		C.godror_dpiJson_setString(jsonnode, &(data.dpiData))
	case time.Time:
		data, err := NewData(x)
		if err != nil {
			return err
		}
		C.godror_dpiJson_setTime(jsonnode, &(data.dpiData))
	case time.Duration:
		data, err := NewData(x)
		if err != nil {
			return err
		}
		C.godror_dpiJson_setIntervalDS(jsonnode, &(data.dpiData))
	case []byte:
		data, err := NewData(x)
		if err != nil {
			return err
		}
		C.godror_dpiJson_setBytes(jsonnode, &(data.dpiData))
	case bool:
		data, err := NewData(x)
		if err != nil {
			return err
		}
		C.godror_dpiJson_setBool(jsonnode, &(data.dpiData))
	default:
		return fmt.Errorf("unsupported type %T", in)
	}
	return nil
}

// freedpiJSONNode deallocates the dpiJsonNode
func freedpiJSONNode(node *C.dpiJsonNode) error {
	C.godror_dpiJsonfreeMem(node)
	return nil
}

// allocdpiJSONNode allocates dpiJsonNode from interface value, val
func allocdpiJSONNode(val interface{}, node **C.dpiJsonNode) error {
	C.godror_allocate_dpiNode((**C.dpiJsonNode)(unsafe.Pointer(node)))
	err := populateJSONNode(*node, val)
	if err != nil {
		C.godror_dpiJsonfreeMem(*node)
	}
	return err
}
