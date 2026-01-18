// Copyright 2017, 2021 The Godror Authors
//
//
// SPDX-License-Identifier: UPL-1.0 OR Apache-2.0

package godror

import (
	"context"
	"sync/atomic"

	"github.com/godror/godror/slog"
)

var globalLogger atomic.Value

// SetLogger sets the global logger.
func SetLogger(logger *slog.Logger) { globalLogger.Store(logger) }

type logCtxKey struct{}

// ContextWithLogger returns a context with the given logger.
func ContextWithLogger(ctx context.Context, logger *slog.Logger) context.Context {
	return context.WithValue(ctx, logCtxKey{}, logger)
}
func getLogger(ctx context.Context) *slog.Logger {
	if ctx != nil && ctx != context.TODO() {
		if lgr, ok := ctx.Value(logCtxKey{}).(*slog.Logger); ok {
			return lgr
		}
		if ctxValue := ctx.Value(paramsCtxKey{}); ctxValue != nil {
			if cc, ok := ctxValue.(commonAndConnParams); ok {
				return cc.Logger
			}
		}
	}
	if lgr, ok := globalLogger.Load().(*slog.Logger); ok {
		return lgr
	}
	return nil
}
