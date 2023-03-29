// Copyright 2017, 2022 The Godror Authors
//
//
// SPDX-License-Identifier: UPL-1.0 OR Apache-2.0

package godror

import (
	"context"
	"database/sql"
	"reflect"
)

const DefaultBatchLimit = 1024

// Batch collects the Added rows and executes in batches, after collecting Limit number of rows.
// The default Limit is DefaultBatchLimit.
type Batch struct {
	Stmt        *sql.Stmt
	values      []interface{}
	rValues     []reflect.Value
	size, Limit int
}

// Add the values. The first call initializes the storage,
// so all the subsequent calls to Add must use the same number of values,
// with the same types.
//
// When the number of added rows reaches Size, Flush is called.
func (b *Batch) Add(ctx context.Context, values ...interface{}) error {
	if b.rValues == nil {
		if b.Limit <= 0 {
			b.Limit = DefaultBatchLimit
		}
		b.rValues = make([]reflect.Value, len(values))
		for i := range values {
			b.rValues[i] = reflect.MakeSlice(reflect.SliceOf(reflect.TypeOf(values[i])), 0, b.Limit)
		}
	}
	for i, v := range values {
		b.rValues[i] = reflect.Append(b.rValues[i], reflect.ValueOf(v))
	}
	b.size++
	if b.size < b.Limit {
		return nil
	}
	return b.Flush(ctx)
}

// Size returns the buffered (unflushed) number of records.
func (b *Batch) Size() int { return b.size }

// Flush executes the statement is and the clears the storage.
func (b *Batch) Flush(ctx context.Context) error {
	if len(b.rValues) == 0 || b.rValues[0].Len() == 0 {
		return nil
	}
	if b.values == nil {
		b.values = make([]interface{}, len(b.rValues))
	}
	logger := ctxGetLog(ctx)
	for i, v := range b.rValues {
		b.values[i] = v.Interface()
		if false && logger != nil {
			logger.Log("msg", "Flush", "i", i, "v", b.values[i])
		}
	}
	if false && logger != nil {
		logger.Log("msg", "Flush", "values", b.values)
	}
	if _, err := b.Stmt.ExecContext(ctx, b.values...); err != nil {
		return err
	}
	for i, v := range b.rValues {
		b.rValues[i] = v.Slice(0, 0)
	}
	b.size = 0
	return nil
}
