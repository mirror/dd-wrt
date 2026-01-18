// Copyright 2024 The Godror Authors
//
//
// SPDX-License-Identifier: UPL-1.0 OR Apache-2.0

package godror

/*
#include <stdint.h>
#include "dpiImpl.h"
#include "token.h"

*/
import "C"

import (
	"context"
	"fmt"
	"runtime/cgo"
	"unsafe"

	"github.com/godror/godror/dsn"
	"github.com/godror/godror/slog"
)

// AccessToken Callback information.
type accessTokenCB struct {
	ctx      context.Context
	callback func(context.Context, *dsn.AccessToken) error
}

// tokenCallbackHandler is the callback for C code on token expiry.
//
//export TokenCallbackHandler
func TokenCallbackHandler(handle C.uintptr_t, accessToken *C.dpiAccessToken) {
	h := cgo.Handle(handle)
	tokenCB := h.Value().(accessTokenCB)
	ctx := tokenCB.ctx
	if ctx == nil {
		ctx = context.TODO()
	}
	logger := getLogger(ctx)
	if logger != nil && logger.Enabled(ctx, slog.LevelDebug) {
		logger.Debug("TokenCallbackHandler", "tokenCB", fmt.Sprintf("%p", tokenCB.callback), "accessToken", accessToken)
	}

	// Call user function which provides the new token and privateKey.
	var refreshAccessToken dsn.AccessToken
	if err := tokenCB.callback(ctx, &refreshAccessToken); err != nil {
		if logger != nil && logger.Enabled(ctx, slog.LevelDebug) {
			logger.Debug("tokenCB.callback", "callback", fmt.Sprintf("%p", tokenCB.callback),
				"tokenCB", tokenCB)
		}
		return
	}
	token := refreshAccessToken.Token
	privateKey := refreshAccessToken.PrivateKey
	if token != "" {
		accessToken.token = C.CString(token)
		accessToken.tokenLength = C.uint32_t(len(token))
		if privateKey != "" {
			accessToken.privateKey = C.CString(privateKey)
			accessToken.privateKeyLength = C.uint32_t(len(privateKey))
		}
	}
}

// RegisterTokenCallback will populate the callback and context.
// The void* datatype context is obtained by wrapping the cgo.Handle.
func RegisterTokenCallback(poolCreateParams *C.dpiPoolCreateParams,
	tokenGenFn func(context.Context, *dsn.AccessToken) error,
	tokenCtx context.Context) unsafe.Pointer {

	// typedef int (*dpiAccessTokenCallback)(void* context, dpiAccessToken *accessToken);
	poolCreateParams.accessTokenCallback = C.dpiAccessTokenCallback(C.godrorTokenCallbackHandlerDebug)
	tokenCB := accessTokenCB{callback: tokenGenFn, ctx: tokenCtx}
	h := cgo.NewHandle(tokenCB)
	wctx := C.godrorWrapHandle(C.uintptr_t(h))
	poolCreateParams.accessTokenCallbackContext = unsafe.Pointer(wctx)
	return unsafe.Pointer(wctx)
}

// UnRegisterTokenCallback will free the token callback data registered
// during pool creation.
func UnRegisterTokenCallback(ptr unsafe.Pointer) {
	if ptr == nil {
		return
	}
	ctx := (*C.godrorHwrap)(ptr)
	h := cgo.Handle(ctx.handle)
	if ctx.token != nil {
		C.free(unsafe.Pointer(ctx.token))
	}
	if ctx.privateKey != nil {
		C.free(unsafe.Pointer(ctx.privateKey))
	}
	h.Delete()
	C.free(ptr)
}
