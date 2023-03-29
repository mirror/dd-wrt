// Copyright 2017, 2020 The Godror Authors
//
//
// SPDX-License-Identifier: UPL-1.0 OR Apache-2.0

package godror

/*
#include "dpiImpl.h"

//int dpiData_getRowidStringValue(dpiData *data, const char **value, uint32_t *valueLength) {
//	return dpiRowid_getStringValue(data->value.asRowid, value, valueLength);
//}
//dpiRowid *dpiData_getRowid(dpiData *data) {
//	return data->value.asRowid;
//}
*/
import "C"
import (
	"bytes"
	"database/sql/driver"
	"fmt"
	"io"
	"math"
	"reflect"
	"runtime"
	"strconv"
	"strings"
	"time"
	"unsafe"
)

var _ = driver.Rows((*rows)(nil))
var _ = driver.RowsColumnTypeDatabaseTypeName((*rows)(nil))
var _ = driver.RowsColumnTypeLength((*rows)(nil))
var _ = driver.RowsColumnTypeNullable((*rows)(nil))
var _ = driver.RowsColumnTypePrecisionScale((*rows)(nil))
var _ = driver.RowsColumnTypeScanType((*rows)(nil))
var _ = driver.RowsNextResultSet((*rows)(nil))

type rows struct {
	err       error
	nextRsErr error
	done      chan struct{}
	*statement
	origSt         *statement
	nextRs         *C.dpiStmt
	data           [][]C.dpiData
	columns        []Column
	vars           []*C.dpiVar
	bufferRowIndex C.uint32_t
	fetched        C.uint32_t
	fromData       bool
}

// Columns returns the names of the columns. The number of
// columns of the result is inferred from the length of the
// slice. If a particular column name isn't known, an empty
// string should be returned for that entry.
func (r *rows) Columns() []string {
	names := make([]string, len(r.columns))
	for i, col := range r.columns {
		names[i] = col.Name
	}
	return names
}

// Close closes the rows iterator.
func (r *rows) Close() error {
	if r == nil {
		return nil
	}
	vars, st, nextRs, done := r.vars, r.statement, r.nextRs, r.done
	r.columns, r.vars, r.data, r.statement, r.nextRs, r.done = nil, nil, nil, nil, nil, nil
	if done != nil {
		close(done)
	}
	fromData := r.fromData
	r.fromData = false
	for _, v := range vars[:cap(vars)] {
		if v != nil {
			C.dpiVar_release(v)
		}
	}
	if nextRs != nil {
		if logger := getLogger(); logger != nil {
			logger.Log("msg", "rows Close", "nextRs", fmt.Sprintf("%p", nextRs))
		}
		C.dpiStmt_release(nextRs)
	}
	if st == nil {
		return nil
	}

	if fromData || st.dpiStmt.refCount < 2 {
		return st.Close()
	}
	C.dpiStmt_release(st.dpiStmt)
	return nil
}

// ColumnTypeLength return the length of the column type if the column is a variable length type.
// If the column is not a variable length type ok should return false.
// If length is not limited other than system limits, it should return math.MaxInt64.
// The following are examples of returned values for various types:
//
// TEXT          (math.MaxInt64, true)
// varchar(10)   (10, true)
// nvarchar(10)  (10, true)
// decimal       (0, false)
// int           (0, false)
// bytea(30)     (30, true)
func (r *rows) ColumnTypeLength(index int) (length int64, ok bool) {
	switch col := r.columns[index]; col.OracleType {
	case C.DPI_ORACLE_TYPE_ROWID, C.DPI_NATIVE_TYPE_ROWID:
		return int64(10), true
	case C.DPI_ORACLE_TYPE_VARCHAR, C.DPI_ORACLE_TYPE_NVARCHAR,
		C.DPI_ORACLE_TYPE_CHAR, C.DPI_ORACLE_TYPE_NCHAR,
		C.DPI_ORACLE_TYPE_LONG_VARCHAR, C.DPI_ORACLE_TYPE_LONG_NVARCHAR,
		C.DPI_NATIVE_TYPE_BYTES:
		return int64(col.Size), true
	case C.DPI_ORACLE_TYPE_CLOB, C.DPI_ORACLE_TYPE_NCLOB,
		C.DPI_ORACLE_TYPE_BLOB,
		C.DPI_ORACLE_TYPE_BFILE,
		C.DPI_NATIVE_TYPE_LOB,
		C.DPI_ORACLE_TYPE_JSON, C.DPI_ORACLE_TYPE_JSON_OBJECT, C.DPI_ORACLE_TYPE_JSON_ARRAY:
		return math.MaxInt64, true
	default:
		return 0, false
	}
}

// ColumnTypeDatabaseTypeName returns the database system type name without the length.
// Type names should be uppercase.
// Examples of returned types: "VARCHAR", "NVARCHAR", "VARCHAR2", "CHAR", "TEXT", "DECIMAL", "SMALLINT", "INT", "BIGINT", "BOOL", "[]BIGINT", "JSONB", "XML", "TIMESTAMP".
func (r *rows) ColumnTypeDatabaseTypeName(index int) string {
	switch r.columns[index].OrigOracleType {
	case C.DPI_ORACLE_TYPE_VARCHAR:
		return "VARCHAR2"
	case C.DPI_ORACLE_TYPE_NVARCHAR:
		return "NVARCHAR2"
	case C.DPI_ORACLE_TYPE_CHAR:
		return "CHAR"
	case C.DPI_ORACLE_TYPE_NCHAR:
		return "NCHAR"
	case C.DPI_ORACLE_TYPE_LONG_VARCHAR, C.DPI_ORACLE_TYPE_LONG_NVARCHAR:
		return "LONG"
	case C.DPI_NATIVE_TYPE_BYTES, C.DPI_ORACLE_TYPE_RAW:
		return "RAW"
	case C.DPI_ORACLE_TYPE_ROWID, C.DPI_NATIVE_TYPE_ROWID:
		return "ROWID"
	case C.DPI_ORACLE_TYPE_LONG_RAW:
		return "LONG RAW"
	case C.DPI_ORACLE_TYPE_NUMBER:
		return "NUMBER"
	case C.DPI_ORACLE_TYPE_NATIVE_FLOAT, C.DPI_NATIVE_TYPE_FLOAT:
		return "FLOAT"
	case C.DPI_ORACLE_TYPE_NATIVE_DOUBLE, C.DPI_NATIVE_TYPE_DOUBLE:
		return "DOUBLE"
	case C.DPI_ORACLE_TYPE_NATIVE_INT, C.DPI_NATIVE_TYPE_INT64:
		return "BINARY_INTEGER"
	case C.DPI_ORACLE_TYPE_NATIVE_UINT, C.DPI_NATIVE_TYPE_UINT64:
		return "BINARY_INTEGER"
	case C.DPI_ORACLE_TYPE_TIMESTAMP, C.DPI_NATIVE_TYPE_TIMESTAMP:
		return "TIMESTAMP"
	case C.DPI_ORACLE_TYPE_TIMESTAMP_TZ:
		return "TIMESTAMP WITH TIME ZONE"
	case C.DPI_ORACLE_TYPE_TIMESTAMP_LTZ:
		return "TIMESTAMP WITH LOCAL TIME ZONE"
	case C.DPI_ORACLE_TYPE_DATE:
		return "DATE"
	case C.DPI_ORACLE_TYPE_INTERVAL_DS, C.DPI_NATIVE_TYPE_INTERVAL_DS:
		return "INTERVAL DAY TO SECOND"
	case C.DPI_ORACLE_TYPE_INTERVAL_YM, C.DPI_NATIVE_TYPE_INTERVAL_YM:
		return "INTERVAL YEAR TO MONTH"
	case C.DPI_ORACLE_TYPE_CLOB:
		return "CLOB"
	case C.DPI_ORACLE_TYPE_NCLOB:
		return "NCLOB"
	case C.DPI_ORACLE_TYPE_BLOB:
		return "BLOB"
	case C.DPI_ORACLE_TYPE_BFILE:
		return "BFILE"
	case C.DPI_ORACLE_TYPE_STMT, C.DPI_NATIVE_TYPE_STMT:
		return "SYS_REFCURSOR"
	case C.DPI_ORACLE_TYPE_BOOLEAN, C.DPI_NATIVE_TYPE_BOOLEAN:
		return "BOOLEAN"
	case C.DPI_ORACLE_TYPE_OBJECT:
		return "OBJECT"
	case C.DPI_ORACLE_TYPE_JSON, C.DPI_ORACLE_TYPE_JSON_OBJECT, C.DPI_ORACLE_TYPE_JSON_ARRAY:
		return "JSON"
	default:
		return fmt.Sprintf("OTHER[%d]", r.columns[index].OracleType)
	}
}

// ColumnTypeNullable. The nullable value should be true if it is known the column may be null, or false if the column is known to be not nullable. If the column nullability is unknown, ok should be false.

func (r *rows) ColumnTypeNullable(index int) (nullable, ok bool) {
	return r.columns[index].Nullable, true
}

// ColumnTypePrecisionScale returns the precision and scale for decimal types.
// If not applicable, ok should be false.
// The following are examples of returned values for various types:
//
// decimal(38, 4)    (38, 4, true)
// int               (0, 0, false)
// decimal           (math.MaxInt64, math.MaxInt64, true)
func (r *rows) ColumnTypePrecisionScale(index int) (precision, scale int64, ok bool) {
	switch col := r.columns[index]; col.OracleType {
	case
		//C.DPI_ORACLE_TYPE_NATIVE_FLOAT, C.DPI_NATIVE_TYPE_FLOAT,
		//C.DPI_ORACLE_TYPE_NATIVE_DOUBLE, C.DPI_NATIVE_TYPE_DOUBLE,
		//C.DPI_ORACLE_TYPE_NATIVE_INT, C.DPI_NATIVE_TYPE_INT64,
		//C.DPI_ORACLE_TYPE_NATIVE_UINT, C.DPI_NATIVE_TYPE_UINT64,
		C.DPI_ORACLE_TYPE_NUMBER:
		return int64(col.Precision), int64(col.Scale), true
	default:
		return 0, 0, false
	}
}

// ColumnTypeScanType returns the value type that can be used to scan types into.
// For example, the database column type "bigint" this should return "reflect.TypeOf(int64(0))".
func (r *rows) ColumnTypeScanType(index int) reflect.Type {
	switch col := r.columns[index]; col.OracleType {
	case C.DPI_NATIVE_TYPE_BYTES, C.DPI_ORACLE_TYPE_RAW,
		C.DPI_ORACLE_TYPE_ROWID, C.DPI_NATIVE_TYPE_ROWID,
		C.DPI_ORACLE_TYPE_LONG_RAW:
		return reflect.TypeOf([]byte(nil))
	case C.DPI_ORACLE_TYPE_NUMBER:
		switch col.NativeType {
		case C.DPI_NATIVE_TYPE_INT64:
			return reflect.TypeOf(int64(0))
		case C.DPI_NATIVE_TYPE_UINT64:
			return reflect.TypeOf(uint64(0))
		//case C.DPI_NATIVE_TYPE_FLOAT:
		//	return reflect.TypeOf(float32(0))
		//case C.DPI_NATIVE_TYPE_DOUBLE:
		//		return reflect.TypeOf(float64(0))
		default:
			return reflect.TypeOf(Number(""))
		}
	case C.DPI_ORACLE_TYPE_NATIVE_FLOAT, C.DPI_NATIVE_TYPE_FLOAT:
		return reflect.TypeOf(float32(0))
	case C.DPI_ORACLE_TYPE_NATIVE_DOUBLE, C.DPI_NATIVE_TYPE_DOUBLE:
		return reflect.TypeOf(float64(0))
	case C.DPI_ORACLE_TYPE_NATIVE_INT, C.DPI_NATIVE_TYPE_INT64:
		return reflect.TypeOf(int64(0))
	case C.DPI_ORACLE_TYPE_NATIVE_UINT, C.DPI_NATIVE_TYPE_UINT64:
		return reflect.TypeOf(uint64(0))
	case C.DPI_ORACLE_TYPE_TIMESTAMP, C.DPI_NATIVE_TYPE_TIMESTAMP,
		C.DPI_ORACLE_TYPE_TIMESTAMP_TZ, C.DPI_ORACLE_TYPE_TIMESTAMP_LTZ,
		C.DPI_ORACLE_TYPE_DATE:
		return reflect.TypeOf(NullTime{})
	case C.DPI_ORACLE_TYPE_INTERVAL_DS, C.DPI_NATIVE_TYPE_INTERVAL_DS:
		return reflect.TypeOf(time.Duration(0))
	case C.DPI_ORACLE_TYPE_CLOB, C.DPI_ORACLE_TYPE_NCLOB:
		return reflect.TypeOf("")
	case C.DPI_ORACLE_TYPE_BLOB, C.DPI_ORACLE_TYPE_BFILE:
		return reflect.TypeOf([]byte(nil))
	case C.DPI_ORACLE_TYPE_STMT, C.DPI_NATIVE_TYPE_STMT:
		return reflect.TypeOf(&statement{})
	case C.DPI_ORACLE_TYPE_BOOLEAN, C.DPI_NATIVE_TYPE_BOOLEAN:
		return reflect.TypeOf(false)
	case C.DPI_ORACLE_TYPE_JSON:
		return reflect.TypeOf(JSON{})
	case C.DPI_ORACLE_TYPE_JSON_OBJECT:
		return reflect.TypeOf(JSONObject{})
	case C.DPI_ORACLE_TYPE_JSON_ARRAY:
		return reflect.TypeOf(JSONArray{})
	default:
		return reflect.TypeOf("")
	}
}

const debugRowsNext = false

// Next is called to populate the next row of data into
// the provided slice. The provided slice will be the same
// size as the Columns() are wide.
//
// Next should return io.EOF when there are no more rows.
//
// As with all Objects, you MUST call Close on the returned Object instances when they're not needed anymore!
func (r *rows) Next(dest []driver.Value) error {
	if r.err != nil {
		return r.err
	}
	if len(dest) != len(r.columns) {
		return fmt.Errorf("column count mismatch: we have %d columns, but given %d destination", len(r.columns), len(dest))
	}
	logger := getLogger()

	runtime.LockOSThread()
	defer runtime.UnlockOSThread()

	if r.fetched == 0 {
		// Start the watchdog only once See issue #113 (https://github.com/godror/godror/issues/113)
		if r.done == nil {
			if ctx := r.statement.ctx; ctx != nil {
				// nil can be present when Next is issued on cursor returned from DB
				if r.err = ctx.Err(); r.err != nil {
					return r.err
				}
				if _, hasDeadline := r.statement.ctx.Deadline(); hasDeadline {
					r.done = make(chan struct{})
					// handle deadline for dpiStmt_fetchRows. context reused from stmt
					if err := r.statement.handleDeadline(ctx, r.done); err != nil {
						return err
					}
				}
			}
		}
		if r.done != nil {
			defer func() {
				if r.err != nil && r.done != nil {
					close(r.done)
					r.done = nil
				}
			}()
		}

		var moreRows C.int
		var start time.Time
		maxRows := C.uint32_t(r.statement.FetchArraySize())
		r.statement.Lock()
		if debugRowsNext {
			fmt.Printf("fetching max=%d\n", maxRows)
			start = time.Now()
		}
		err := r.statement.checkExecNoLOT(func() C.int {
			return C.dpiStmt_fetchRows(r.dpiStmt, maxRows, &r.bufferRowIndex, &r.fetched, &moreRows)
		})
		failed := err != nil
		if debugRowsNext {
			fmt.Printf("failed=%t bri=%d fetched=%d more=%d data=%d cols=%d dur=%s\n", failed, r.bufferRowIndex, r.fetched, moreRows, len(r.data), len(r.columns), time.Since(start))
		}
		r.statement.Unlock()
		if failed {
			if logger != nil {
				logger.Log("msg", "fetch", "error", err)
			}
			_ = r.Close()
			if strings.Contains(err.Error(), "DPI-1039: statement was already closed") {
				r.err = io.EOF
			} else {
				r.err = fmt.Errorf("Next: %w", err)
			}
			return r.err
		}
		if logger != nil {
			logger.Log("msg", "fetched", "bri", r.bufferRowIndex, "fetched", r.fetched, "moreRows", moreRows, "len(data)", len(r.data), "cols", len(r.columns))
		}
		if r.fetched == 0 {
			_ = r.Close()
			r.err = io.EOF
			return r.err
		}
		if r.data == nil {
			r.data = make([][]C.dpiData, len(r.columns))
			for i := range r.columns {
				var n C.uint32_t
				var data *C.dpiData
				if err = r.statement.checkExecNoLOT(func() C.int {
					return C.dpiVar_getReturnedData(r.vars[i], 0, &n, &data)
				}); err != nil {
					return fmt.Errorf("getReturnedData[%d]: %w", i, err)
				}
				r.data[i] = unsafe.Slice(data, n)
				//fmt.Printf("data %d=%+v\n%+v\n", n, data, r.data[i][0])
			}
		}

	}
	//fmt.Printf("data=%#v\n", r.data)

	nullDate := r.statement.NullDate()
	mkNumber := func(s string) interface{} { return Number(s) }
	if r.statement.NumberAsString() {
		mkNumber = func(s string) interface{} { return s }
	}

	//fmt.Printf("bri=%d fetched=%d\n", r.bufferRowIndex, r.fetched)
	//fmt.Printf("data=%#v\n", r.data[0][r.bufferRowIndex])
	//fmt.Printf("VC=%d\n", C.DPI_ORACLE_TYPE_VARCHAR)
	for i, col := range r.columns {
		typ := col.OracleType
		d := &r.data[i][r.bufferRowIndex]
		isNull := d.isNull == 1
		if false && logger != nil {
			logger.Log("msg", "Next", "i", i, "row", r.bufferRowIndex, "typ", typ, "null", isNull) //, "data", fmt.Sprintf("%+v", d), "typ", typ)
		}

		switch typ {
		case C.DPI_ORACLE_TYPE_VARCHAR, C.DPI_ORACLE_TYPE_NVARCHAR,
			C.DPI_ORACLE_TYPE_CHAR, C.DPI_ORACLE_TYPE_NCHAR,
			C.DPI_ORACLE_TYPE_LONG_VARCHAR, C.DPI_ORACLE_TYPE_LONG_NVARCHAR:
			//fmt.Printf("CHAR\n")
			if isNull {
				dest[i] = ""
				continue
			}
			//b := C.dpiData_getBytes(d)
			b := (*C.dpiBytes)(unsafe.Pointer(&d.value))
			if b.length == 0 {
				dest[i] = ""
				continue
			}
			dest[i] = C.GoStringN(b.ptr, C.int(b.length))

		case C.DPI_ORACLE_TYPE_NUMBER:
			if isNull {
				dest[i] = nil
				continue
			}
			switch col.NativeType {
			case C.DPI_NATIVE_TYPE_INT64:
				//dest[i] = int64(C.dpiData_getInt64(d))
				dest[i] = *((*int64)(unsafe.Pointer(&d.value)))
			case C.DPI_NATIVE_TYPE_UINT64:
				//dest[i] = uint64(C.dpiData_getUint64(d))
				dest[i] = *((*uint64)(unsafe.Pointer(&d.value)))
			case C.DPI_NATIVE_TYPE_FLOAT:
				//dest[i] = float32(C.dpiData_getFloat(d))
				//dest[i] = printFloat(float64(C.dpiData_getFloat(d)))
				dest[i] = printFloat(float64(*((*float32)(unsafe.Pointer(&d.value)))))
			case C.DPI_NATIVE_TYPE_DOUBLE:
				//dest[i] = float64(C.dpiData_getDouble(d))
				//dest[i] = printFloat(float64(C.dpiData_getDouble(d)))
				dest[i] = printFloat(*((*float64)(unsafe.Pointer(&d.value))))
			case C.DPI_NATIVE_TYPE_BOOLEAN:
				dest[i] = d.isNull != 0 && C.dpiData_getBool(d) != 0
			default:
				//b := C.dpiData_getBytes(d)
				b := (*C.dpiBytes)(unsafe.Pointer(&d.value))
				s := C.GoStringN(b.ptr, C.int(b.length))

				dest[i] = mkNumber(s)
				if false && logger != nil {
					logger.Log("msg", "b", "i", i, "ptr", b.ptr, "length", b.length, "typ", col.NativeType, "int64", C.dpiData_getInt64(d), "dest", dest[i])
				}
			}
			if false && logger != nil {
				logger.Log("msg", "num", "t", col.NativeType, "i", i, "dest", fmt.Sprintf("%T %+v", dest[i], dest[i]))
			}

		case C.DPI_ORACLE_TYPE_ROWID, C.DPI_NATIVE_TYPE_ROWID:
			if isNull {
				dest[i] = nil
				continue
			}
			// ROWID as returned by OCIRowidToChar
			//cRowid := C.dpiData_getRowid(d)
			cRowid := *((**C.dpiRowid)(unsafe.Pointer(&d.value)))
			var cBuf *C.char
			var cLen C.uint32_t
			if err := r.statement.checkExecNoLOT(func() C.int {
				return C.dpiRowid_getStringValue(cRowid, &cBuf, &cLen)
			}); err != nil {
				return err
			}
			dest[i] = C.GoStringN(cBuf, C.int(cLen))

		case C.DPI_ORACLE_TYPE_RAW, C.DPI_ORACLE_TYPE_LONG_RAW:
			if isNull {
				dest[i] = nil
				continue
			}
			//b := C.dpiData_getBytes(d)
			b := (*C.dpiBytes)(unsafe.Pointer(&d.value))
			if b.length == 0 {
				dest[i] = []byte{}
				continue
			}
			dest[i] = C.GoBytes(unsafe.Pointer(b.ptr), C.int(b.length))
		case C.DPI_ORACLE_TYPE_NATIVE_FLOAT, C.DPI_NATIVE_TYPE_FLOAT:
			if isNull {
				dest[i] = nil
				continue
			}
			//dest[i] = float32(C.dpiData_getFloat(d))
			dest[i] = *((*float32)(unsafe.Pointer(&d.value)))
		case C.DPI_ORACLE_TYPE_NATIVE_DOUBLE, C.DPI_NATIVE_TYPE_DOUBLE:
			if isNull {
				dest[i] = nil
				continue
			}
			//dest[i] = float64(C.dpiData_getDouble(d))
			dest[i] = *((*float64)(unsafe.Pointer(&d.value)))
		case C.DPI_ORACLE_TYPE_NATIVE_INT, C.DPI_NATIVE_TYPE_INT64:
			if isNull {
				dest[i] = nil
				continue
			}
			//dest[i] = int64(C.dpiData_getInt64(d))
			dest[i] = *((*int64)(unsafe.Pointer(&d.value)))
		case C.DPI_ORACLE_TYPE_NATIVE_UINT, C.DPI_NATIVE_TYPE_UINT64:
			if isNull {
				dest[i] = nil
				continue
			}
			//dest[i] = uint64(C.dpiData_getUint64(d))
			dest[i] = *((*uint64)(unsafe.Pointer(&d.value)))
		case C.DPI_ORACLE_TYPE_TIMESTAMP,
			C.DPI_ORACLE_TYPE_TIMESTAMP_TZ, C.DPI_ORACLE_TYPE_TIMESTAMP_LTZ,
			C.DPI_NATIVE_TYPE_TIMESTAMP,
			C.DPI_ORACLE_TYPE_DATE:
			if isNull {
				dest[i] = nullDate
				continue
			}
			//ts := C.dpiData_getTimestamp(d)
			ts := *((*C.dpiTimestamp)(unsafe.Pointer(&d.value)))
			tz := r.conn.Timezone()
			if col.OracleType == C.DPI_ORACLE_TYPE_TIMESTAMP_TZ || col.OracleType == C.DPI_ORACLE_TYPE_TIMESTAMP_LTZ {
				tz = timeZoneFor(ts.tzHourOffset, ts.tzMinuteOffset,
					nil, // just obey to what's included in the data
				)
			}
			if tz == nil {
				if logger != nil {
					logger.Log("msg", "DATE", "i", i, "tz", tz, "params", r.conn.params)
				}
			}
			dest[i] = time.Date(int(ts.year), time.Month(ts.month), int(ts.day), int(ts.hour), int(ts.minute), int(ts.second), int(ts.fsecond), tz)
		case C.DPI_ORACLE_TYPE_INTERVAL_DS, C.DPI_NATIVE_TYPE_INTERVAL_DS:
			if isNull {
				dest[i] = nil
				continue
			}
			var t time.Duration
			dataGetIntervalDS(&t, d)
			dest[i] = t
		case C.DPI_ORACLE_TYPE_INTERVAL_YM, C.DPI_NATIVE_TYPE_INTERVAL_YM:
			if isNull {
				dest[i] = nil
				continue
			}
			//ym := C.dpiData_getIntervalYM(d)
			ym := *((*C.dpiIntervalYM)(unsafe.Pointer(&d.value)))
			dest[i] = strconv.Itoa(int(ym.years)) + "-" + strconv.Itoa(int(ym.months))

		case C.DPI_ORACLE_TYPE_CLOB, C.DPI_ORACLE_TYPE_NCLOB,
			C.DPI_ORACLE_TYPE_BLOB,
			C.DPI_ORACLE_TYPE_BFILE,
			C.DPI_NATIVE_TYPE_LOB:
			isClob := typ == C.DPI_ORACLE_TYPE_CLOB || typ == C.DPI_ORACLE_TYPE_NCLOB
			if isNull {
				if isClob && (r.ClobAsString() || !r.LobAsReader()) {
					dest[i] = ""
				} else {
					dest[i] = nil
				}
				continue
			}
			rdr := &dpiLobReader{dpiLob: C.dpiData_getLOB(d), drv: r.drv, IsClob: isClob}
			if isClob && (r.ClobAsString() || !r.LobAsReader()) {
				sb := stringBuilders.Get()
				_, err := io.Copy(sb, rdr)
				C.dpiLob_close(rdr.dpiLob)
				if err != nil {
					stringBuilders.Put(sb)
					return err
				}
				dest[i] = sb.String()
				stringBuilders.Put(sb)
				continue
			}
			dest[i] = &Lob{Reader: rdr, IsClob: rdr.IsClob}

		case C.DPI_ORACLE_TYPE_STMT, C.DPI_NATIVE_TYPE_STMT:
			if isNull {
				dest[i] = nil
				continue
			}
			st := &statement{conn: r.conn, dpiStmt: C.dpiData_getStmt(d),
				stmtOptions: r.statement.stmtOptions, // inherit parent statement's options
			}
			var colCount C.uint32_t
			if err := r.statement.checkExecNoLOT(func() C.int {
				return C.dpiStmt_getNumQueryColumns(st.dpiStmt, &colCount)
			}); err != nil {
				if logger != nil {
					logger.Log("msg", "Next.getNumQueryColumns", "st", fmt.Sprintf("%p", st.dpiStmt), "error", err)
				}
				//C.dpiStmt_release(st.dpiStmt)
				return fmt.Errorf("getNumQueryColumns: %w", err)
			}
			st.Lock()
			r2, err := st.openRows(int(colCount))
			st.Unlock()
			if err != nil {
				if logger != nil {
					logger.Log("msg", "Next.openRows", "st", fmt.Sprintf("%p", st.dpiStmt), "error", err)
				}
				st.Close()
				return err
			}
			r2.fromData = true
			stmtSetFinalizer(st, "Next")
			dest[i] = r2

		case C.DPI_ORACLE_TYPE_BOOLEAN, C.DPI_NATIVE_TYPE_BOOLEAN:
			if isNull {
				dest[i] = nil
				continue
			}
			//dest[i] = C.dpiData_getBool(d) == 1
			dest[i] = *((*C.int)(unsafe.Pointer(&d.value))) == 1

		case C.DPI_ORACLE_TYPE_OBJECT: //Default type used for named type columns in the database. Data is transferred to/from Oracle in Oracle's internal format.
			if isNull {
				dest[i] = nil
				continue
			}
			o, err := wrapObject(r.conn, col.ObjectType, C.dpiData_getObject(d))
			if err != nil {
				return err
			}
			dest[i] = o

		case C.DPI_ORACLE_TYPE_JSON, C.DPI_NATIVE_TYPE_JSON:
			if isNull {
				dest[i] = nil
				continue
			}
			switch col.NativeType {
			case C.DPI_NATIVE_TYPE_JSON:
				dj := *((**C.dpiJson)(unsafe.Pointer(&(d.value))))
				dest[i] = JSON{dpiJson: dj}
			default:
			}

		case C.DPI_NATIVE_TYPE_JSON_OBJECT:
			if isNull {
				dest[i] = nil
				continue
			}
			dest[i] = JSONObject{dpiJsonObject: ((*C.dpiJsonObject)(unsafe.Pointer(&d.value)))}
		case C.DPI_NATIVE_TYPE_JSON_ARRAY:
			if isNull {
				dest[i] = nil
				continue
			}
			dest[i] = JSONArray{dpiJsonArray: ((*C.dpiJsonArray)(unsafe.Pointer(&d.value)))}

		default:
			return fmt.Errorf("unsupported column type %d", typ)
		}

		//fmt.Printf("dest[%d]=%#v\n", i, dest[i])
	}
	r.bufferRowIndex++
	r.fetched--

	if debugRowsNext && r.fetched < 2 {
		fmt.Printf("bri=%d fetched=%d\n", r.bufferRowIndex, r.fetched)
	}
	if logger != nil {
		logger.Log("msg", "scanned", "row", r.bufferRowIndex, "dest", dest)
	}

	return nil
}

var _ = driver.Rows((*directRow)(nil))

type directRow struct {
	conn   *conn
	query  string
	result []interface{}
	args   []string
}

func (dr *directRow) Columns() []string {
	logger := getLogger()
	if logger != nil {
		logger.Log("directRow", "Columns")
	}
	switch dr.query {
	case getConnection:
		return []string{"CONNECTION"}
	}
	return nil
}

// Close closes the rows iterator.
func (dr *directRow) Close() error {
	dr.conn = nil
	dr.query = ""
	dr.args = nil
	dr.result = nil
	return nil
}

// Next is called to populate the next row of data into
// the provided slice. The provided slice will be the same
// size as the Columns() are wide.
//
// Next should return io.EOF when there are no more rows.
func (dr *directRow) Next(dest []driver.Value) error {
	logger := getLogger()
	if logger != nil {
		logger.Log("directRow", "Next", "query", dr.query, "dest", dest)
	}
	switch dr.query {
	case getConnection:
		*(dest[0].(*interface{})) = dr.result[0]
	}
	return nil
}

func (r *rows) getImplicitResult() {
	if r == nil || r.nextRsErr != nil {
		return
	}
	// use the original statement for the NextResultSet call.
	st := r.origSt
	if st == nil {
		st = r.statement
		r.origSt = st
	}
	if err := r.checkExec(func() C.int { return C.dpiStmt_getImplicitResult(st.dpiStmt, &r.nextRs) }); err != nil {
		r.nextRsErr = fmt.Errorf("getImplicitResult: %w", err)
	}
	C.dpiStmt_addRef(r.nextRs)
}
func (r *rows) HasNextResultSet() bool {
	if r == nil || r.statement == nil || r.conn == nil {
		return false
	}
	if r.nextRs != nil {
		return true
	}
	if cv := r.conn.drv.clientVersion; !(cv.Version > 12 || cv.Version == 12 && cv.Release >= 1) {
		return false
	}
	if sv, err := r.conn.ServerVersion(); !(err == nil &&
		(sv.Version > 12 || sv.Version == 12 && sv.Release >= 1)) {
		return false
	}
	r.getImplicitResult()
	return r.nextRs != nil
}
func (r *rows) NextResultSet() error {
	if !r.HasNextResultSet() {
		if r.nextRsErr != nil {
			return r.nextRsErr
		}
		return fmt.Errorf("getImplicitResult: %w", io.EOF)
	}
	st := &statement{conn: r.conn, dpiStmt: r.nextRs}

	var n C.uint32_t
	logger := getLogger()
	if err := r.checkExec(func() C.int { return C.dpiStmt_getNumQueryColumns(st.dpiStmt, &n) }); err != nil {
		err = fmt.Errorf("getNumQueryColumns: %+v: %w", err, io.EOF)
		if logger != nil {
			logger.Log("msg", "NextResultSet.getNumQueryColumns", "st", fmt.Sprintf("%p", st.dpiStmt), "error", err)
		}
		//C.dpiStmt_release(st.dpiStmt)
		return err
	}
	// keep the originam statement for the succeeding NextResultSet calls.
	st.Lock()
	nr, err := st.openRows(int(n))
	st.Unlock()
	if err != nil {
		if logger != nil {
			logger.Log("msg", "NextResultSet.openRows", "st", fmt.Sprintf("%p", st.dpiStmt), "error", err)
		}
		st.Close()
		return err
	}
	stmtSetFinalizer(st, "NextResultSet")
	nr.origSt = r.origSt
	if nr.origSt == nil {
		nr.origSt = r.statement
	}
	*r = *nr
	return nil
}

func printFloat(f float64) string {
	var a [40]byte
	b := strconv.AppendFloat(a[:0], f, 'f', -1, 64)
	i := bytes.IndexByte(b, '.')
	if i < 0 {
		return string(b)
	}
	for j := i + 1; j < len(b); j++ {
		if b[j] != '0' {
			return string(b)
		}
	}
	return string(b[:i])
}
