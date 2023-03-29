// Copyright 2017, 2021 The Godror Authors
//
//
// SPDX-License-Identifier: UPL-1.0 OR Apache-2.0

package godror

import (
	"context"
	"io"
	"sync"

	"github.com/go-logfmt/logfmt"
	"github.com/go-logr/logr"
)

var globalLogger = &swapLogger{}

func SetLogger(logger logr.Logger) {
	globalLogger.Swap(logger)
}
func SetLog(f func(...interface{}) error) {
	globalLogger.Swap(logr.New(LogFunc(f)))
}

// NewLogfmtLogger returns a Logger, and that logs using logfmt, to the given io.Writer.
func NewLogfmtLogger(w io.Writer) logr.Logger {
	enc := logfmt.NewEncoder(w)
	return logr.New(LogFunc(func(keyvals ...interface{}) error {
		firstErr := enc.EncodeKeyvals(keyvals...)
		if err := enc.EndRecord(); err != nil && firstErr == nil {
			return err
		}
		return firstErr
	}))
}

type Logger interface {
	Log(keyvals ...interface{}) error
}
type LogFunc func(keyvals ...interface{}) error

func (f LogFunc) Log(keyvals ...interface{}) error { return f(keyvals...) }
func (f LogFunc) Init(_ logr.RuntimeInfo)          {}
func (f LogFunc) Enabled(_ int) bool               { return f != nil }
func (f LogFunc) Info(level int, msg string, keyvals ...interface{}) {
	f(append(append(make([]interface{}, 0, 4+len(keyvals)), "lvl", level, "msg", msg), keyvals...)...)
}
func (f LogFunc) Error(err error, msg string, keyvals ...interface{}) {
	f(append(append(make([]interface{}, 0, 4+len(keyvals)), "msg", msg, "error", err), keyvals...)...)
}
func (f LogFunc) WithValues(plusKeyVals ...interface{}) logr.LogSink {
	return LogFunc(func(keyvals ...interface{}) error {
		return f(append(plusKeyVals, keyvals...)...)
	})
}
func (f LogFunc) WithName(name string) logr.LogSink {
	return f.WithValues("name", name)
}

type swapLogger struct {
	mu     sync.RWMutex
	logger logr.Logger
	isSet  bool
}

func (sw *swapLogger) IsSet() bool {
	if sw == nil {
		return false
	}
	sw.mu.RLock()
	isset := sw.isSet
	sw.mu.RUnlock()
	return isset
}

func (sw *swapLogger) Swap(logger logr.Logger) {
	sw.mu.Lock()
	sw.logger = logger
	sw.isSet = true
	sw.mu.Unlock()
}
func (sw *swapLogger) Info(msg string, keyvals ...interface{}) {
	sw.logger.Info(msg, keyvals...)
}
func (sw *swapLogger) Error(err error, msg string, keyvals ...interface{}) {
	sw.logger.Error(err, msg, keyvals...)
}
func (sw *swapLogger) Log(keyvals ...interface{}) error {
	sw.mu.RLock()
	logger := sw.logger
	sw.mu.RUnlock()
	if !logger.Enabled() {
		return nil
	}
	if len(keyvals) >= 2 {
		if msgKey, ok := keyvals[0].(string); ok && msgKey == "msg" {
			if msg, ok := keyvals[1].(string); ok {
				logger.Info(msg, keyvals[2:]...)
				return nil
			}
		}
	}
	logger.Info("", keyvals...)
	return nil
}

type logCtxKey struct{}

func getLogger() Logger {
	if globalLogger.IsSet() {
		return globalLogger
	}
	return nil
}
func ctxGetLog(ctx context.Context) Logger {
	if ctx != nil {
		if lgr, ok := ctx.Value(logCtxKey{}).(Logger); ok {
			return lgr
		}
	}
	return getLogger()
}

// ContextWithLogger returns a context with the given logger.
func ContextWithLogger(ctx context.Context, logger Logger) context.Context {
	return context.WithValue(ctx, logCtxKey{}, logger)
}

// ContextWithLog returns a context with the given log function.
func ContextWithLog(ctx context.Context, logF func(...interface{}) error) context.Context {
	return context.WithValue(ctx, logCtxKey{}, LogFunc(logF))
}
