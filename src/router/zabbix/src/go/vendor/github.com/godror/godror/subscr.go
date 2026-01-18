// Copyright 2017, 2020 The Godror Authors
//
//
// SPDX-License-Identifier: UPL-1.0 OR Apache-2.0

package godror

/*
#include <stdlib.h>
#include <stdio.h>
#include "dpiImpl.h"

void CallbackSubscrDebug(void *context, dpiSubscrMessage *message);

*/
import "C"

import (
	"context"
	"errors"
	"fmt"
	"log"
	"runtime"
	"strings"
	"sync"
	"unsafe"
)

// SubscriptionOption is for setting various parameters of the Subscription.
type SubscriptionOption func(*subscriptionParams)

// SubscrHostPort is a SubscriptionOption that sets tha IPAddress and Port to the specified values.
//
// The address is on which the subscription listens to receive notifications,
// for a server-initiated connection.
//
// The address can be an IPv4 address in dotted decimal format such as 192.1.2.34
// or an IPv6 address in hexadecimal format such as 2001:0db8:0000:0000:0217:f2ff:fe4b:4ced.
//
// By default (address is the empty string), an IP address will be selected by the Oracle client.
//
// The port number on which to receive notifications, for a server-initiated connection.
//
// The default value of 0 means that a port number will be selected by the Oracle client.
func SubscrHostPort(address string, port uint32) SubscriptionOption {
	return func(p *subscriptionParams) {
		p.IPAddress, p.Port = address, port
	}
}

// SubscrClientInitiated sets whether the subscription is client-initated.
func SubscrClientInitiated(b bool) SubscriptionOption {
	return func(p *subscriptionParams) {
		p.ClientInitiated = b
	}
}

// subscrParams are parameters for a new Subscription.
type subscriptionParams struct {
	// IPAddress on which the subscription listens to receive notifications,
	// for a server-initiated connection.
	//
	// The IP address can be an IPv4 address in dotted decimal format such as 192.1.2.34
	// or an IPv6 address in hexadecimal format such as 2001:0db8:0000:0000:0217:f2ff:fe4b:4ced.
	//
	// By default, an IP address will be selected by the Oracle client.
	IPAddress string

	// Port number on which to receive notifications, for a server-initiated connection.
	// The default value of 0 means that a port number will be selected by the Oracle client.
	Port uint32

	// ClientInitiated specifies whether a client or a server initiated connection should be created.
	//
	// This feature is only available when Oracle Client 19.4
	// and Oracle Database 19.4 or higher are being used.
	ClientInitiated bool
}

// Cannot pass *Subscription to C, so pass an uint64 that points to this map entry
var (
	subscriptionsMu sync.Mutex
	subscriptions   = make(map[uint64]*Subscription)
	subscriptionsID uint64
)

// CallbackSubscr is the callback for C code on subscription event.
//
//export CallbackSubscr
func CallbackSubscr(ctx unsafe.Pointer, message *C.dpiSubscrMessage) {
	log.Printf("CB %p %+v", ctx, message)
	if ctx == nil {
		return
	}
	subscriptionsMu.Lock()
	subscr := subscriptions[*((*uint64)(ctx))]
	subscriptionsMu.Unlock()

	getRows := func(rws *C.dpiSubscrMessageRow, rwsNum C.uint32_t) []RowEvent {
		if rwsNum == 0 {
			return nil
		}
		//cRws := (*((*[maxArraySize]C.dpiSubscrMessageRow)(unsafe.Pointer(rws))))[:int(rwsNum)]
		cRws := unsafe.Slice(rws, rwsNum)
		rows := make([]RowEvent, len(cRws))
		for i, row := range cRws {
			rows[i] = RowEvent{
				Operation: Operation(row.operation),
				Rowid:     C.GoStringN(row.rowid, C.int(row.rowidLength)),
			}
			//C.free(unsafe.Pointer(row.rowid))
		}
		return rows
	}
	getTables := func(tbls *C.dpiSubscrMessageTable, tblsNum C.uint32_t) []TableEvent {
		if tblsNum == 0 {
			return nil
		}
		//cTbls := (*((*[maxArraySize]C.dpiSubscrMessageTable)(unsafe.Pointer(tbls))))[:int(tblsNum)]
		cTbls := unsafe.Slice(tbls, tblsNum)
		tables := make([]TableEvent, len(cTbls))
		for i, tbl := range cTbls {
			tables[i] = TableEvent{
				Operation: Operation(tbl.operation),
				Name:      C.GoStringN(tbl.name, C.int(tbl.nameLength)),
				Rows:      getRows(tbl.rows, tbl.numRows),
			}
			//C.free(unsafe.Pointer(tbl.name))
		}
		return tables
	}
	getQueries := func(qrys *C.dpiSubscrMessageQuery, qrysNum C.uint32_t) []QueryEvent {
		if qrysNum == 0 {
			return nil
		}
		//cQrys := (*((*[maxArraySize]C.dpiSubscrMessageQuery)(unsafe.Pointer(qrys))))[:int(qrysNum)]
		cQrys := unsafe.Slice(qrys, qrysNum)
		queries := make([]QueryEvent, len(cQrys))
		for i, qry := range cQrys {
			queries[i] = QueryEvent{
				ID:        uint64(qry.id),
				Operation: Operation(qry.operation),
				Tables:    getTables(qry.tables, qry.numTables),
			}
		}
		return queries
	}
	var err error
	if message.errorInfo != nil {
		err = fromErrorInfo(*message.errorInfo)
	}

	evt := Event{
		Err:     err,
		Type:    EventType(message.eventType),
		DB:      C.GoStringN(message.dbName, C.int(message.dbNameLength)),
		Tables:  getTables(message.tables, message.numTables),
		Queries: getQueries(message.queries, message.numQueries),
	}
	subscr.callback(evt)
}

// Event for a subscription.
type Event struct {
	Err     error
	DB      string
	Tables  []TableEvent
	Queries []QueryEvent
	Type    EventType
}

// QueryEvent is an event of a Query.
type QueryEvent struct {
	Tables []TableEvent
	ID     uint64
	Operation
}

// TableEvent is for a Table-related event.
type TableEvent struct {
	Name string
	Rows []RowEvent
	Operation
}

// RowEvent is for row-related event.
type RowEvent struct {
	Rowid string
	Operation
}

// Subscription for events in the DB.
type Subscription struct {
	conn      *conn
	dpiSubscr *C.dpiSubscr
	callback  func(Event)
	ID        uint64
}

// NewSubscription creates a new Subscription in the DB.
//
// Make sure your user has CHANGE NOTIFICATION privilege!
//
// This code is EXPERIMENTAL yet!
func (c *conn) NewSubscription(name string, cb func(Event), options ...SubscriptionOption) (*Subscription, error) {
	if !c.params.EnableEvents {
		return nil, errors.New("subscription must be allowed by specifying \"enableEvents=1\" in the connection parameters")
	}
	var p subscriptionParams
	for _, o := range options {
		o(&p)
	}
	subscr := Subscription{conn: c, callback: cb}
	params := (*C.dpiSubscrCreateParams)(C.malloc(C.sizeof_dpiSubscrCreateParams))
	defer func() { C.free(unsafe.Pointer(params)) }()
	C.dpiContext_initSubscrCreateParams(c.drv.dpiContext, params)
	params.subscrNamespace = C.DPI_SUBSCR_NAMESPACE_DBCHANGE
	params.protocol = C.DPI_SUBSCR_PROTO_CALLBACK
	params.qos = C.DPI_SUBSCR_QOS_BEST_EFFORT | C.DPI_SUBSCR_QOS_QUERY | C.DPI_SUBSCR_QOS_ROWIDS
	params.operations = C.DPI_OPCODE_ALL_OPS
	if name != "" || p.IPAddress != "" {
		if name != "" {
			params.name = C.CString(name)
			params.nameLength = C.uint32_t(len(name))
		}
		if p.IPAddress != "" {
			params.ipAddress = C.CString(p.IPAddress)
			params.ipAddressLength = C.uint32_t(len(p.IPAddress))
		}
		defer func() {
			if params.name != nil {
				C.free(unsafe.Pointer(params.name))
			}
			if params.ipAddress != nil {
				C.free(unsafe.Pointer(params.ipAddress))
			}
		}()
	}
	if p.Port != 0 {
		params.portNumber = C.uint32_t(p.Port)
	}
	if p.ClientInitiated {
		params.clientInitiated = C.int(1)
	}
	// typedef void (*dpiSubscrCallback)(void* context, dpiSubscrMessage *message);
	params.callback = C.dpiSubscrCallback(C.CallbackSubscrDebug)
	// cannot pass &subscr to C, so pass indirectly
	subscriptionsMu.Lock()
	subscriptionsID++
	subscr.ID = subscriptionsID
	subscriptions[subscr.ID] = &subscr
	subscriptionsMu.Unlock()
	subscrID := (*C.uint64_t)(C.malloc(8))
	*subscrID = C.uint64_t(subscriptionsID)
	params.callbackContext = unsafe.Pointer(subscrID)

	dpiSubscr := (*C.dpiSubscr)(C.malloc(C.sizeof_void))

	if err := c.checkExec(func() C.int {
		return C.dpiConn_subscribe(c.dpiConn, params, (**C.dpiSubscr)(unsafe.Pointer(&dpiSubscr)))
	}); err != nil {
		C.free(unsafe.Pointer(dpiSubscr))
		err = fmt.Errorf("newSubscription: %w", err)
		if strings.Contains(errors.Unwrap(err).Error(), "DPI-1065:") {
			err = fmt.Errorf("specify \"enableEvents=1\" connection parameter on connection to be able to use subscriptions: %w", err)
		}
		return nil, err
	}
	subscr.dpiSubscr = dpiSubscr
	return &subscr, nil
}

// Register a query for Change Notification.
//
// This code is EXPERIMENTAL yet!
func (s *Subscription) Register(qry string, params ...interface{}) error {
	runtime.LockOSThread()
	defer runtime.UnlockOSThread()

	cQry := C.CString(qry)
	defer C.free(unsafe.Pointer(cQry))

	var dpiStmt *C.dpiStmt
	if C.dpiSubscr_prepareStmt(s.dpiSubscr, cQry, C.uint32_t(len(qry)), &dpiStmt) == C.DPI_FAILURE {
		return fmt.Errorf("prepareStmt[%p]: %w", s.dpiSubscr, s.conn.getError())
	}
	defer func() { C.dpiStmt_release(dpiStmt) }()

	mode := C.dpiExecMode(C.DPI_MODE_EXEC_DEFAULT)
	var qCols C.uint32_t
	if C.dpiStmt_execute(dpiStmt, mode, &qCols) == C.DPI_FAILURE {
		return fmt.Errorf("executeStmt: %w", s.conn.getError())
	}
	var queryID C.uint64_t
	if C.dpiStmt_getSubscrQueryId(dpiStmt, &queryID) == C.DPI_FAILURE {
		return fmt.Errorf("getSubscrQueryId: %w", s.conn.getError())
	}
	logger := getLogger(context.TODO())
	if logger != nil {
		logger.Debug("subscribed", "query", qry, "id", queryID)
	}

	return nil
}

// Close the subscription.
//
// This code is EXPERIMENTAL yet!
func (s *Subscription) Close() error {
	subscriptionsMu.Lock()
	delete(subscriptions, s.ID)
	subscriptionsMu.Unlock()
	dpiSubscr := s.dpiSubscr
	conn := s.conn
	s.conn = nil
	s.dpiSubscr = nil
	s.callback = nil
	if dpiSubscr == nil || conn == nil || conn.dpiConn == nil {
		return nil
	}
	if err := conn.checkExec(func() C.int { return C.dpiConn_unsubscribe(conn.dpiConn, dpiSubscr) }); err != nil {
		return fmt.Errorf("close: %w", err)
	}
	return nil
}

// EventType is the type of an event.
type EventType C.dpiEventType

// Events that can be watched.
const (
	EvtStartup     = EventType(C.DPI_EVENT_STARTUP)
	EvtShutdown    = EventType(C.DPI_EVENT_SHUTDOWN)
	EvtShutdownAny = EventType(C.DPI_EVENT_SHUTDOWN_ANY)
	EvtDereg       = EventType(C.DPI_EVENT_DEREG)
	EvtObjChange   = EventType(C.DPI_EVENT_OBJCHANGE)
	EvtQueryChange = EventType(C.DPI_EVENT_QUERYCHANGE)
	EvtAQ          = EventType(C.DPI_EVENT_AQ)
)

// Operation in the DB.
type Operation C.dpiOpCode

const (
	// OpAll Indicates that notifications should be sent for all operations on the table or query.
	OpAll = Operation(C.DPI_OPCODE_ALL_OPS)
	// OpAllRows Indicates that all rows have been changed in the table or query (or too many rows were changed or row information was not requested).
	OpAllRows = Operation(C.DPI_OPCODE_ALL_ROWS)
	// OpInsert Indicates that an insert operation has taken place in the table or query.
	OpInsert = Operation(C.DPI_OPCODE_INSERT)
	// OpUpdate Indicates that an update operation has taken place in the table or query.
	OpUpdate = Operation(C.DPI_OPCODE_UPDATE)
	// OpDelete Indicates that a delete operation has taken place in the table or query.
	OpDelete = Operation(C.DPI_OPCODE_DELETE)
	// OpAlter Indicates that the registered table or query has been altered.
	OpAlter = Operation(C.DPI_OPCODE_ALTER)
	// OpDrop Indicates that the registered table or query has been dropped.
	OpDrop = Operation(C.DPI_OPCODE_DROP)
	// OpUnknown An unknown operation has taken place.
	OpUnknown = Operation(C.DPI_OPCODE_UNKNOWN)
)
