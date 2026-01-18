//go:build go1.21

// Copyright 2022, 2023 Tamás Gulácsi. All rights reserved.
//
// SPDX-License-Identifier: Apache-2.0

package slog

import (
	"io"
	"log/slog"
	"time"
)

type (
	Attr           = slog.Attr
	Handler        = slog.Handler
	HandlerOptions = slog.HandlerOptions
	Level          = slog.Level
	Leveler        = slog.Leveler
	LevelVar       = slog.LevelVar
	Logger         = slog.Logger
	Record         = slog.Record
	Value          = slog.Value
)

const (
	LevelDebug = slog.LevelDebug
	LevelInfo  = slog.LevelInfo
	LevelWarn  = slog.LevelWarn
	LevelError = slog.LevelError
)

func Default() *slog.Logger           { return slog.Default() }
func SetDefault(lgr *slog.Logger)     { slog.SetDefault(lgr) }
func New(h slog.Handler) *slog.Logger { return slog.New(h) }
func NewRecord(t time.Time, lvl slog.Level, s string, p uintptr) slog.Record {
	return slog.NewRecord(t, lvl, s, p)
}

func String(k, v string) slog.Attr                             { return slog.String(k, v) }
func StringValue(value string) slog.Value                      { return slog.StringValue(value) }
func NewJSONHandler(w io.Writer, opts *HandlerOptions) Handler { return slog.NewJSONHandler(w, opts) }
func NewTextHandler(w io.Writer, opts *HandlerOptions) Handler { return slog.NewTextHandler(w, opts) }
