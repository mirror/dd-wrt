// Copyright 2019, 2022 The Godror Authors
//
//
// SPDX-License-Identifier: UPL-1.0 OR Apache-2.0

package dsn

import (
	"context"
	"database/sql"
	"database/sql/driver"
	"encoding"
	"errors"
	"fmt"
	"io"
	"net/url"
	"sort"
	"strconv"
	"strings"
	"sync"
	"time"

	"github.com/go-logfmt/logfmt"
	"github.com/godror/godror/slog"
)

const (
	// DefaultPoolMinSessions specifies the default value for minSessions for pool creation.
	DefaultPoolMinSessions = 1
	// DefaultPoolMaxSessions specifies the default value for maxSessions for pool creation.
	DefaultPoolMaxSessions = 1000
	// DefaultSessionIncrement specifies the default value for increment for pool creation.
	DefaultSessionIncrement = 1
	// DefaultPoolIncrement is a deprecated name for DefaultSessionIncrement.
	DefaultPoolIncrement = DefaultSessionIncrement
	// DefaultConnectionClass is empty, which allows to use the poolMinSessions created as part of session pool creation for non-DRCP. For DRCP, connectionClass needs to be explicitly mentioned.
	DefaultConnectionClass = ""
	// NoConnectionPoolingConnectionClass is a special connection class name to indicate no connection pooling.
	// It is the same as setting standaloneConnection=1
	NoConnectionPoolingConnectionClass = "NO-CONNECTION-POOLING"
	// DefaultSessionTimeout is the seconds before idle pool sessions get evicted
	DefaultSessionTimeout = 5 * time.Minute
	// DefaultWaitTimeout is the milliseconds to wait for a session to become available
	DefaultWaitTimeout = 30 * time.Second
	// DefaultMaxLifeTime is the maximum time in seconds till a pooled session may exist
	DefaultMaxLifeTime = 1 * time.Hour
	// DefaultStandaloneConnection holds the default for standaloneConnection.
	DefaultStandaloneConnection = true
	// DefaultNoBreakOnContextCancel holds the default for noBreakOnContext
	DefaultNoBreakOnContextCancel = false
)

type CommonSimpleParams struct {
	Timezone                *time.Location
	ConfigDir, LibDir       string
	Username, ConnectString string
	Token, PrivateKey       string
	Password                Password
	Charset                 string
	// StmtCacheSize of 0 means the default, -1 to disable the stmt cache completely
	StmtCacheSize int
	// true: OnInit will be called only by the new session / false: OnInit will called by new or pooled connection
	InitOnNewConn                               bool
	EnableEvents, NoTZCheck, PerSessionTimezone bool
	NoBreakOnContextCancel                      bool
}

// CommonParams holds the common parameters for pooled or standalone connections.
//
// For details, see https://oracle.github.io/odpi/doc/structs/dpiCommonCreateParams.html#dpicommoncreateparams
type CommonParams struct {
	// Logger is the per-pool or per-connection logger.
	// The default nil logger does not log.
	Logger *slog.Logger
	// OnInit is executed on session init. Overrides AlterSession and OnInitStmts!
	OnInit func(context.Context, driver.ConnPrepareContext) error `json:"-"`
	// OnInitStmts are executed on session init, iff OnInit is nil.
	OnInitStmts []string
	// AlterSession key-values are set with "ALTER SESSION SET key=value" on session init, iff OnInit is nil.
	AlterSession [][2]string
	CommonSimpleParams
}

func (P CommonParams) String() string {
	return P.CommonSimpleParams.String()
}

var cacheCPSMu sync.RWMutex
var cacheCPS = make(map[CommonSimpleParams]string)

// String returns the string representation of CommonParams.
func (P CommonSimpleParams) String() string {
	cacheCPSMu.RLock()
	s, ok := cacheCPS[P]
	cacheCPSMu.RUnlock()
	if ok {
		return s
	}

	q := acquireParamsArray(8)
	defer releaseParamsArray(q)
	q.Add("user", P.Username)
	q.Add("password", P.Password.String())
	q.Add("connectString", P.ConnectString)
	if P.ConfigDir != "" {
		q.Add("configDir", P.ConfigDir)
	}
	if P.LibDir != "" {
		q.Add("libDir", P.LibDir)
	}
	s = ""
	tz := P.Timezone
	if tz != nil {
		if tz == time.Local {
			s = "local"
		} else {
			s = tz.String()
		}
	}
	if s != "" {
		q.Add("timezone", s)
	}
	if P.EnableEvents {
		q.Add("enableEvents", "1")
	}
	if P.NoTZCheck {
		q.Add("noTimezoneCheck", "1")
	}
	if P.PerSessionTimezone {
		q.Add("perSessionTimezone", "1")
	}
	if P.StmtCacheSize != 0 {
		q.Add("stmtCacheSize", strconv.Itoa(int(P.StmtCacheSize)))
	}
	if P.Charset != "" {
		q.Add("charset", P.Charset)
	}
	if P.InitOnNewConn {
		q.Add("initOnNewConnection", "1")
	}
	if P.NoBreakOnContextCancel {
		q.Add("noBreakOnContextCancel", "1")
	}

	s = q.String()
	cacheCPSMu.Lock()
	cacheCPS[P] = s
	cacheCPSMu.Unlock()
	return s
}

// ConnParams holds the connection-specific parameters.
//
// For details, see https://oracle.github.io/odpi/doc/structs/dpiConnCreateParams.html#dpiconncreateparams
type ConnParams struct {
	NewPassword                   Password
	ConnClass                     string
	ShardingKey, SuperShardingKey []interface{}
	AdminRole                     AdminRole
	IsPrelim                      bool
}

// String returns the string representation of the ConnParams.
func (P ConnParams) String() string {
	q := acquireParamsArray(8)
	defer releaseParamsArray(q)
	if P.ConnClass != "" {
		q.Add("connectionClass", P.ConnClass)
	}
	if !P.NewPassword.IsZero() {
		q.Add("newPassword", P.NewPassword.String())
	}
	if P.AdminRole != "" {
		q.Add("adminRole", P.AdminRole.String())
	}
	for _, v := range P.ShardingKey {
		q.Add("shardingKey", fmt.Sprintf("%v", v))
	}
	for _, v := range P.SuperShardingKey {
		q.Add("superShardingKey", fmt.Sprintf("%v", v))
	}
	return q.String()
}

// AccessToken Data for Token Authentication.
type AccessToken struct {
	Token      string
	PrivateKey string
}

// PoolParams holds the configuration of the Oracle Session Pool.
//
// For details, see https://oracle.github.io/odpi/doc/structs/dpiPoolCreateParams.html#dpipoolcreateparams
//
// For details, see https://oracle.github.io/odpi/doc/structs/dpiPoolCreateParams.html#dpipoolcreateparams
type PoolParams struct {
	TokenCBCtx                                 context.Context                           `json:"-"`
	TokenCB                                    func(context.Context, *AccessToken) error `json:"-"`
	MinSessions, MaxSessions, SessionIncrement int
	MaxSessionsPerShard                        int
	WaitTimeout, MaxLifeTime, SessionTimeout   time.Duration
	PingInterval                               time.Duration
	Heterogeneous, ExternalAuth                sql.NullBool
}

// String returns the string representation of PoolParams.
func (P PoolParams) String() string {
	q := acquireParamsArray(8)
	defer releaseParamsArray(q)
	q.Add("poolMinSessions", strconv.Itoa(P.MinSessions))
	if P.MaxSessions != 0 {
		q.Add("poolMaxSessions", strconv.Itoa(P.MaxSessions))
	}
	if P.MaxSessionsPerShard != 0 {
		q.Add("poolMaxSessionsPerShard", strconv.Itoa(P.MaxSessionsPerShard))
	}
	q.Add("poolIncrement", strconv.Itoa(P.SessionIncrement))
	if P.Heterogeneous.Valid {
		q.Add("heterogeneousPool", b2s(P.Heterogeneous.Bool))
	}
	if P.WaitTimeout != 0 {
		q.Add("poolWaitTimeout", P.WaitTimeout.String())
	}
	if P.MaxLifeTime != 0 {
		q.Add("poolSessionMaxLifetime", P.MaxLifeTime.String())
	}
	if P.SessionTimeout != 0 {
		q.Add("poolSessionTimeout", P.SessionTimeout.String())
	}
	if P.ExternalAuth.Valid {
		q.Add("externalAuth", b2s(P.ExternalAuth.Bool))
	}
	if P.PingInterval != 0 {
		q.Add("pingInterval", P.PingInterval.String())
	}
	return q.String()
}

// ConnectionParams holds the params for a connection (pool).
// You can use ConnectionParams{...}.StringWithPassword()
// as a connection string in sql.Open.
type ConnectionParams struct {
	CommonParams
	ConnParams
	PoolParams
	// ConnParams.NewPassword is used iff StandaloneConnection is true!
	StandaloneConnection sql.NullBool
}

// IsStandalone returns whether the connection should be standalone, not pooled.
func (P ConnectionParams) IsStandalone() bool {
	return P.StandaloneConnection.Valid && P.StandaloneConnection.Bool ||
		(!P.StandaloneConnection.Valid && DefaultStandaloneConnection) ||
		P.ConnClass == NoConnectionPoolingConnectionClass ||
		P.AdminRole != NoRole
}

func (P *ConnectionParams) comb() {
	if (!P.StandaloneConnection.Valid || !P.StandaloneConnection.Bool) &&
		P.ConnClass == NoConnectionPoolingConnectionClass {
		P.StandaloneConnection = Bool(true)
	}
	if P.IsPrelim || P.StandaloneConnection.Valid && P.StandaloneConnection.Bool {
		// Prelim: the shared memory may not exist when Oracle is shut down.
		P.ConnClass = ""
		P.Heterogeneous = Bool(false)
	}
	if !P.IsStandalone() {
		P.NewPassword.Reset()
		// only enable external authentication if we are dealing with a
		// homogeneous pool and no user name/password has been specified
		if P.Username == "" && P.Password.IsZero() &&
			!P.ExternalAuth.Valid &&
			!(P.Heterogeneous.Valid && P.Heterogeneous.Bool) {
			P.ExternalAuth = Bool(true)
		}
	}
}

// SetSessionParamOnInit adds an "ALTER SESSION k=v" to the OnInit task list.
func (P *ConnectionParams) SetSessionParamOnInit(k, v string) {
	P.AlterSession = append(P.AlterSession, [2]string{k, v})
}

// String returns the string representation of ConnectionParams.
// The password is replaced with a "***" string!
func (P ConnectionParams) String() string {
	return P.string(true, false)
}

// StringNoClass returns the string representation of ConnectionParams, without class info.
// The password is replaced with a "***" string!
func (P ConnectionParams) StringNoClass() string {
	return P.string(false, false)
}

// StringWithPassword returns the string representation of ConnectionParams (as String() does),
// but does NOT obfuscate the password, just prints it as is.
func (P ConnectionParams) StringWithPassword() string {
	return P.string(true, true)
}

func (P ConnectionParams) string(class, withPassword bool) string {
	q := acquireParamsArray(32)
	defer releaseParamsArray(q)
	q.Add("connectString", P.ConnectString)
	s := P.ConnClass
	if !class {
		s = ""
	}
	q.Add("connectionClass", s)

	q.Add("user", P.Username)
	if withPassword {
		q.Add("password", P.Password.Secret())
		q.Add("newPassword", P.NewPassword.Secret())
		if P.Token != "" {
			q.Add("token", P.Token)
		}
		if P.PrivateKey != "" {
			q.Add("privateKey", P.PrivateKey)
		}
	} else {
		q.Add("password", P.Password.String())
		if !P.NewPassword.IsZero() {
			q.Add("newPassword", P.NewPassword.String())
		}
	}
	s = ""
	if tz := P.Timezone; tz != nil {
		if tz == time.Local {
			s = "local"
		} else {
			s = tz.String()
		}
	}
	if s != "" {
		q.Add("timezone", s)
	}
	if P.NoTZCheck {
		q.Add("noTimezoneCheck", "1")
	}
	if P.PerSessionTimezone {
		q.Add("perSessionTimezone", "1")
	}
	if P.StmtCacheSize != 0 {
		q.Add("stmtCacheSize", strconv.Itoa(int(P.StmtCacheSize)))
	}
	if P.Charset != "" {
		q.Add("charset", P.Charset)
	}
	q.Add("poolMinSessions", strconv.Itoa(P.MinSessions))
	if P.MaxSessions != 0 {
		q.Add("poolMaxSessions", strconv.Itoa(P.MaxSessions))
	}
	if P.MaxSessionsPerShard != 0 {
		q.Add("poolMasSessionsPerShard", strconv.Itoa(P.MaxSessionsPerShard))
	}
	q.Add("poolIncrement", strconv.Itoa(P.SessionIncrement))
	if P.AdminRole != "" {
		q.Add("adminRole", P.AdminRole.String())
	}
	if P.StandaloneConnection.Valid {
		q.Add("standaloneConnection", b2s(P.StandaloneConnection.Bool))
	}
	if P.EnableEvents {
		q.Add("enableEvents", "1")
	}
	if P.Heterogeneous.Valid {
		q.Add("heterogeneousPool", b2s(P.Heterogeneous.Bool))
	}
	if P.ExternalAuth.Valid {
		q.Add("externalAuth", b2s(P.ExternalAuth.Bool))
	}
	if P.IsPrelim {
		q.Add("prelim", "1")
	}
	if P.WaitTimeout != 0 {
		q.Add("poolWaitTimeout", P.WaitTimeout.String())
	}
	if P.MaxLifeTime != 0 {
		q.Add("poolSessionMaxLifetime", P.MaxLifeTime.String())
	}
	if P.SessionTimeout != 0 {
		q.Add("poolSessionTimeout", P.SessionTimeout.String())
	}
	if P.PingInterval != 0 {
		q.Add("pingInterval", P.PingInterval.String())
	}
	as := acquireParamsArray(1)
	defer releaseParamsArray(as)
	for _, kv := range P.AlterSession {
		as.Reset()
		as.Add(kv[0], kv[1])
		q.Add("alterSession", strings.TrimSpace(as.String()))
	}
	if P.InitOnNewConn {
		q.Add("initOnNewConnection", "1")
	}
	if P.NoBreakOnContextCancel {
		q.Add("noBreakOnContextCancel", "1")
	}
	q.Values["onInit"] = P.OnInitStmts
	if P.ConfigDir != "" {
		q.Add("configDir", P.ConfigDir)
	}
	if P.LibDir != "" {
		q.Add("libDir", P.LibDir)
	}
	//return quoteRunes(P.Username, "/@") + "/" + quoteRunes(password, "@") + "@" + P.CommonParams.ConnectString + "\n" + q.String()

	return q.String()
}

// Parse parses the given connection string into a struct.
//
// For examples, see [../doc/connection.md](../doc/connection.md)
func Parse(dataSourceName string) (ConnectionParams, error) {
	P := ConnectionParams{
		// StandaloneConnection: DefaultStandaloneConnection,
		//CommonParams: CommonParams{ Timezone: time.Local, },
		ConnParams: ConnParams{
			ConnClass: DefaultConnectionClass,
		},
		PoolParams: PoolParams{
			MinSessions:      DefaultPoolMinSessions,
			MaxSessions:      DefaultPoolMaxSessions,
			SessionIncrement: DefaultPoolIncrement,
			MaxLifeTime:      DefaultMaxLifeTime,
			WaitTimeout:      DefaultWaitTimeout,
			SessionTimeout:   DefaultSessionTimeout,
		},
	}

	var paramsString string
	dataSourceName = strings.TrimSpace(dataSourceName)
	var q url.Values

	//fmt.Printf("dsn=%q\n", dataSourceName)
	if strings.HasPrefix(dataSourceName, "oracle://") {
		// URL
		u, err := url.Parse(dataSourceName)
		if err != nil {
			return P, fmt.Errorf("%s: %w", dataSourceName, err)
		}
		if usr := u.User; usr != nil {
			P.Username = usr.Username()
			passw, _ := usr.Password()
			P.Password.Set(passw)
		}
		P.ConnectString = u.Hostname()
		// IPv6 literal address brackets are removed by u.Hostname,
		// so we have to put them back
		if strings.HasPrefix(u.Host, "[") && (len(P.ConnectString) <= 1 || !strings.Contains(P.ConnectString[1:], "]")) {
			P.ConnectString = "[" + P.ConnectString + "]"
		}
		if u.Port() != "" {
			P.ConnectString += ":" + u.Port()
		}
		if u.Path != "" && u.Path != "/" {
			P.ConnectString += u.Path
		}
		//fmt.Printf("URL=%s cs=%q host=%q port=%q path=%q\n", u, P.ConnectString, u.Host, u.Port(), u.Path)
		q = u.Query()
	} else if strings.IndexByte(dataSourceName, '\n') >= 0 || // multi-line, or
		strings.Contains(dataSourceName, "connectString=") { // contains connectString
		// This should be a proper logfmt-encoded parameter string, with connectString
		paramsString, dataSourceName = dataSourceName, ""
	} else {
		// Not URL, not logfmt-ed - an old styled DSN
		// Old, or Easy Connect, or anything
		var passw string
		P.Username, passw, dataSourceName = parseUserPassw(dataSourceName)
		P.Password.Set(passw)
		//fmt.Printf("dsn=%q\n", dataSourceName)
		uSid := strings.ToUpper(dataSourceName)
		//fmt.Printf("dataSourceName=%q SID=%q\n", dataSourceName, uSid)
		if strings.Contains(uSid, " AS ") {
			for _, role := range adminRoles {
				if s := role.String(); strings.HasSuffix(uSid, " AS "+s) {
					P.AdminRole = role
					dataSourceName = dataSourceName[:len(dataSourceName)-len(s)]
					break
				}
			}
		}
		P.ConnectString = dataSourceName
	}

	//fmt.Printf("csa=%q\n", P.ConnectString)

	if paramsString != "" {
		if q == nil {
			pa := acquireParamsArray(32)
			defer releaseParamsArray(pa)
			q = pa.Values
		}
		// Parse the logfmt-formatted parameters string
		d := logfmt.NewDecoder(strings.NewReader(paramsString))
		for d.ScanRecord() {
			for d.ScanKeyval() {
				switch key, value := string(d.Key()), string(d.Value()); key {
				case "connectString":
					P.ConnectString = value
				case "user":
					P.Username = value
				case "password":
					P.Password.Set(value)
				case "charset":
					P.Charset = value
				case "token":
					P.Token = value
				case "privateKey":
					P.PrivateKey = value
				case "alterSession", "onInit", "shardingKey", "superShardingKey":
					q.Add(key, value)
				default:
					q.Set(key, value)
				}
			}
		}
		// fmt.Printf("Parse q=%#v\n", q)
		if err := d.Err(); err != nil {
			return P, fmt.Errorf("parsing parameters %q: %w", paramsString, err)
		}
	}
	//fmt.Printf("cs0=%q\n", P.ConnectString)

	// Override everything from the parameters,
	// which can come from the URL values or the logfmt-formatted parameters string.
	if vv, ok := q["connectionClass"]; ok {
		P.ConnClass = vv[0]
	}
	var sysDBA, sysOper, sysASM bool
	boolTasks := []struct {
		Dest *bool
		Key  string
	}{
		{&sysDBA, "sysdba"},
		{&sysOper, "sysoper"},
		{&sysASM, "sysasm"},
		{&P.IsPrelim, "prelim"},

		{&P.EnableEvents, "enableEvents"},

		{&P.NoTZCheck, "noTimezoneCheck"},
		{&P.PerSessionTimezone, "perSessionTimezone"},
		{&P.InitOnNewConn, "initOnNewConnection"},
		{&P.NoBreakOnContextCancel, "noBreakOnContextCancel"},
	}
	if ar := q.Get("adminRole"); len(ar) > 3 && strings.EqualFold(ar[:3], "SYS") {
		P.AdminRole = AdminRole(strings.ToUpper(ar))
		boolTasks = boolTasks[3:] // skip parsing
	}
	for _, task := range boolTasks {
		s := q.Get(task.Key)
		if s == "" {
			continue
		}
		var err error
		if *task.Dest, err = strconv.ParseBool(s); err != nil {
			return P, fmt.Errorf("%s=%q: %w", task.Key, s, err)
		}
	}
	if P.AdminRole == "" {
		if sysDBA {
			P.AdminRole = SysDBA
		} else if sysOper {
			P.AdminRole = SysOPER
		} else if sysASM {
			P.AdminRole = SysASM
		}
	}

	for _, task := range []struct {
		Dest *sql.NullBool
		Key  string
	}{
		{&P.Heterogeneous, "heterogeneousPool"},
		{&P.ExternalAuth, "externalAuth"},
		{&P.StandaloneConnection, "standaloneConnection"},
	} {
		s := q.Get(task.Key)
		if s == "" {
			continue
		}
		b, err := strconv.ParseBool(s)
		if err != nil {
			return P, fmt.Errorf("%s=%q: %w", task.Key, s, err)
		}
		*task.Dest = Bool(b)
	}
	if !P.StandaloneConnection.Valid && P.Heterogeneous.Valid {
		P.StandaloneConnection = Bool(!P.Heterogeneous.Bool)
	}
	// fmt.Println("parse", P.StringWithPassword())

	if tz := q.Get("timezone"); tz != "" {
		if strings.EqualFold(tz, "local") {
			P.Timezone = time.Local
		} else if strings.Contains(tz, "/") {
			ptz, err := time.LoadLocation(tz)
			if err != nil {
				return P, fmt.Errorf("%s: %w", tz, err)
			}
			P.Timezone = ptz
		} else if off, err := ParseTZ(tz); err == nil {
			if off == 0 {
				P.Timezone = time.UTC
			} else {
				P.Timezone = time.FixedZone(tz, off)
			}
		} else {
			return P, fmt.Errorf("%s: %w", tz, err)
		}
		if P.Timezone == nil {
			P.Timezone = time.UTC
		}
		//} else if P.Timezone == nil {
		//P.Timezone = time.Local
	}
	for _, task := range []struct {
		Dest *int
		Key  string
	}{
		{&P.MinSessions, "poolMinSessions"},
		{&P.MaxSessions, "poolMaxSessions"},
		{&P.MaxSessionsPerShard, "poolMasSessionsPerShard"},
		{&P.SessionIncrement, "poolIncrement"},
		{&P.SessionIncrement, "sessionIncrement"},
		{&P.StmtCacheSize, "stmtCacheSize"},
	} {
		s := q.Get(task.Key)
		if s == "" {
			continue
		}
		var err error
		*task.Dest, err = strconv.Atoi(s)
		if err != nil {
			return P, fmt.Errorf("%s: %w", task.Key+"="+s, err)
		}
	}

	for _, task := range []struct {
		Dest *time.Duration
		Key  string
	}{
		{&P.SessionTimeout, "poolSessionTimeout"},
		{&P.WaitTimeout, "poolWaitTimeout"},
		{&P.MaxLifeTime, "poolSessionMaxLifetime"},
		{&P.PingInterval, "pingInterval"},
	} {
		s := q.Get(task.Key)
		if s == "" {
			continue
		}
		var err error
		*task.Dest, err = time.ParseDuration(s)
		if err != nil {
			if !strings.Contains(err.Error(), "time: missing unit in duration") {
				return P, fmt.Errorf("%s: %w", task.Key+"="+s, err)
			}
			i, err := strconv.Atoi(s)
			if err != nil {
				return P, fmt.Errorf("%s: %w", task.Key+"="+s, err)
			}
			base := time.Second
			if task.Key == "poolWaitTimeout" {
				base = time.Millisecond
			}
			*task.Dest = time.Duration(i) * base
		}
	}
	if P.MinSessions > P.MaxSessions {
		P.MinSessions = P.MaxSessions
	}
	if P.MinSessions == P.MaxSessions {
		P.SessionIncrement = 0
	} else if P.SessionIncrement < 1 {
		P.SessionIncrement = 1
	}
	for _, s := range q["alterSession"] {
		d := logfmt.NewDecoder(strings.NewReader(s))
		for d.ScanRecord() {
			for d.ScanKeyval() {
				P.AlterSession = append(P.AlterSession, [2]string{string(d.Key()), string(d.Value())})
			}
		}
		if err := d.Err(); err != nil {
			return P, fmt.Errorf("%q: %w", s, err)
		}
	}
	P.OnInitStmts = q["onInit"]
	P.ShardingKey = strToIntf(q["shardingKey"])
	P.SuperShardingKey = strToIntf(q["superShardingKey"])

	P.NewPassword.Set(q.Get("newPassword"))
	P.ConfigDir = q.Get("configDir")
	P.LibDir = q.Get("libDir")

	// fmt.Printf("cs1=%q\n", P)
	P.comb()

	// fmt.Printf("cs2=%q\n", P)

	return P, nil
}

// Password is printed obfuscated with String, use Secret to reveal the secret.
type Password struct {
	secret string
}

// NewPassword creates a new Password, containing the given secret.
func NewPassword(secret string) Password {
	var P Password
	P.Set(secret)
	return P
}

const obfuscatedPassword = "SECRET-***"

// String returns the secret obfuscated irreversibly.
func (P Password) String() string { return obfuscatedPassword }

// Secret reveals the real password.
func (P Password) Secret() string { return P.secret }

// IsZero returns whether the password is emtpy.
func (P Password) IsZero() bool { return P.secret == "" }

// Len returns the length of the  password.
func (P Password) Len() int { return len(P.secret) }

// Reset the password.
func (P *Password) Reset() { P.secret = "" }

// Set the password.
func (P *Password) Set(secret string) {
	P.secret = secret
}

// LogValue implements slog.LogValuer.
func (P Password) LogValue() slog.Value { return slog.StringValue(obfuscatedPassword) }

var ErrCannotMarshal = errors.New("cannot be marshaled")

func (P *Password) MarshalText() ([]byte, error) { return nil, ErrCannotMarshal }

// nosemgrep
func (P *Password) MarshalJSON() ([]byte, error)   { return nil, ErrCannotMarshal }
func (P *Password) MarshalBinary() ([]byte, error) { return nil, ErrCannotMarshal }

var _ encoding.TextMarshaler = ((*Password)(nil))
var _ encoding.BinaryMarshaler = ((*Password)(nil))

// CopyFrom another password.
func (P *Password) CopyFrom(Q Password) { *P = Q }

// ParamsArray is an url.Values for holding parameters,
// and logfmt-formatting them with the String() method.
type paramsArray struct {
	url.Values
}

var paramsArrayPool8 = sync.Pool{New: func() any { return newParamsArray(8) }}
var paramsArrayPool32 = sync.Pool{New: func() any { return newParamsArray(32) }}

func acquireParamsArray(cap int) *paramsArray {
	if cap <= 8 {
		return paramsArrayPool8.Get().(*paramsArray)
	}
	return paramsArrayPool32.Get().(*paramsArray)
}
func releaseParamsArray(p *paramsArray) {
	length := len(p.Values)
	for k := range p.Values {
		delete(p.Values, k)
	}
	if length < 32 {
		paramsArrayPool8.Put(p)
	} else {
		paramsArrayPool32.Put(p)
	}
}

// newParamsArray returns a new paramsArray with the given capacity of parameters.
//
// You can use this to build a dataSourceName for godror.
func newParamsArray(cap int) *paramsArray { return &paramsArray{Values: make(url.Values, cap)} }

// WriteTo the given writer, logfmt-encoded,
// starting with username, password, connectString,
// then the rest sorted alphabetically.
func (p *paramsArray) WriteTo(w io.Writer) (int64, error) {
	firstKeys := make([]string, 0, len(p.Values))
	keys := make([]string, 0, len(p.Values))
	for k := range p.Values {
		if k == "password" || k == "user" || k == "connectString" {
			firstKeys = append(firstKeys, k)
		} else {
			keys = append(keys, k)
		}
	}
	sort.Strings(firstKeys)
	// reverse
	for i, j := 0, len(firstKeys)-1; i < j; i, j = i+1, j-1 {
		firstKeys[i], firstKeys[j] = firstKeys[j], firstKeys[i]
	}
	sort.Strings(keys)

	cw := &countingWriter{W: w}
	enc := logfmt.NewEncoder(cw)
	var firstErr error
	var prev, act int64
	for _, k := range append(firstKeys, keys...) {
		for _, v := range p.Values[k] {
			if act > 72 {
				act = 0
				if err := enc.EndRecord(); err != nil {
					if firstErr == nil {
						firstErr = err
					}
					break
				}
				prev = cw.N
			}
			if err := enc.EncodeKeyval(k, v); err != nil {
				if firstErr == nil {
					firstErr = err
				}
				break
			}
			act = cw.N - prev
		}
	}
	return cw.N, firstErr
}

var paSB = sync.Pool{New: func() any { return new(strings.Builder) }}

// String returns the values in the params array, logfmt-formatted,
// starting with username, password, connectString, then the rest sorted alphabetically.
func (p *paramsArray) String() string {
	buf := paSB.Get().(*strings.Builder)
	defer func() { buf.Reset(); paSB.Put(buf) }()
	var n int
	for k, vv := range p.Values {
		for _, v := range vv {
			n += len(k) + 1 + len(v) + 1
		}
	}
	buf.Grow(n)
	if _, err := p.WriteTo(buf); err != nil {
		fmt.Fprintf(buf, "\tERROR: %+v", err)
	}
	return buf.String()
}
func (p *paramsArray) Reset() {
	for k := range p.Values {
		delete(p.Values, k)
	}
}

var unquoteSB = sync.Pool{New: func() any { return new(strings.Builder) }}

// unquote replaces quoted ("\\n") with the quoted.
func unquote(s string) string {
	if !strings.ContainsRune(s, '\\') {
		return s
	}
	buf := unquoteSB.Get().(*strings.Builder)
	defer func() { buf.Reset(); unquoteSB.Put(buf) }()
	buf.Grow(len(s))
	var quoted bool
	for _, r := range s {
		if r == '\\' {
			if !quoted {
				quoted = true
			}
			continue
		}
		if !quoted {
			buf.WriteRune(r)
			continue
		}
		quoted = false
		switch r {
		case 'n':
			buf.WriteByte('\n')
		case 'r':
			buf.WriteByte('\r')
		case 't':
			buf.WriteByte('\t')
		default:
			buf.WriteRune(r)
		}
	}
	return buf.String()
}

// splitQuoted splits the string at sep, treating "\" as a quoting char.
func splitQuoted(s string, sep rune) []string {
	var off int
	sepLen := len(string([]rune{sep}))
	for {
		i := strings.IndexRune(s[off:], sep)
		if i < 0 {
			return []string{s}
		}
		off += i
		if off == 0 || s[off-1] != '\\' {
			return []string{s[:off], s[off+sepLen:]}
		}
		off += sepLen
	}
}

// parseUserPassw splits of the username/password@ from the connectString.
func parseUserPassw(dataSourceName string) (user, passw, connectString string) {
	if i := strings.Index(dataSourceName, "://"); i >= 0 &&
		strings.IndexFunc(
			dataSourceName[:i],
			func(r rune) bool { return !('a' <= r && r <= 'z' || 'A' <= r && r <= 'Z' || '0' <= r && r <= '9') },
		) < 0 {
		return "", "", dataSourceName
	}
	ups := splitQuoted(dataSourceName, '@')
	var extra string
	if len(ups) == 1 { // user/pass, no '@'
		if i := strings.Index(strings.ToUpper(ups[0]), " AS SYS"); i >= 0 {
			ups[0], extra = ups[0][:i], ups[0][i:]
		}
	}
	//fmt.Printf("ups=%v extra=%q\n", ups, extra)
	userpass := splitQuoted(ups[0], '/')
	//fmt.Printf("ups=%q\nuserpass=%q\n", ups, userpass)
	if len(ups) == 1 && len(userpass) == 1 {
		return "", "", dataSourceName + unquote(extra)
	}

	user = unquote(userpass[0])
	if len(userpass) > 1 {
		passw = unquote(userpass[1])
	}
	if len(ups) == 1 {
		return user, passw, unquote(extra)
	}
	return user, passw, unquote(ups[1] + extra)
}

// ParseTZ parses timezone specification ("Europe/Budapest" or "+01:00") and returns the offset in seconds.
func ParseTZ(s string) (int, error) {
	s = strings.TrimSpace(s)
	if s == "" {
		return 0, io.EOF
	}
	if s == "Z" || s == "UTC" {
		return 0, nil
	}
	var tz int
	var ok bool
	if i := strings.IndexByte(s, ':'); i >= 0 {
		u64, err := strconv.ParseUint(s[i+1:], 10, 6)
		if err != nil {
			return tz, fmt.Errorf("%s: %w", s, err)
		}
		tz = int(u64 * 60)
		s = s[:i]
		ok = true
	}
	if !ok {
		if i := strings.IndexByte(s, '/'); i >= 0 {
			targetLoc, err := time.LoadLocation(s)
			if err != nil {
				return tz, fmt.Errorf("%s: %w", s, err)
			}
			if targetLoc == nil {
				targetLoc = time.UTC
			}

			_, localOffset := time.Now().In(targetLoc).Zone()

			tz = localOffset
			return tz, nil
		}
	}
	i64, err := strconv.ParseInt(s, 10, 5)
	if err != nil {
		return tz, fmt.Errorf("%s: %w", s, err)
	}
	if i64 < 0 {
		tz = -tz
	}
	tz += int(i64 * 3600)
	return tz, nil
}

// AppendLogfmt appends the key=val logfmt-formatted.
func AppendLogfmt(w io.Writer, key, value interface{}) error {
	e := logfmt.NewEncoder(w)
	err := e.EncodeKeyval(key, value)
	if endErr := e.EndRecord(); endErr != nil && err == nil {
		err = endErr
	}
	return err
}

func strToIntf(ss []string) []interface{} {
	n := len(ss)
	if n == 0 {
		return nil
	}
	intf := make([]interface{}, n)
	for i, s := range ss {
		intf[i] = s
	}
	return intf
}

type countingWriter struct {
	W io.Writer
	N int64
}

func (cw *countingWriter) Write(p []byte) (int, error) {
	n, err := cw.W.Write(p)
	cw.N += int64(n)
	return n, err
}

func b2s(b bool) string {
	if b {
		return "1"
	}
	return "0"
}

// Bool is an sql.NullBool helper
func Bool(b bool) sql.NullBool { return sql.NullBool{Valid: true, Bool: b} }

type AdminRole string

func (r AdminRole) String() string { return string(r) }

const (
	NoRole    = AdminRole("")
	SysDBA    = AdminRole("SYSDBA")
	SysOPER   = AdminRole("SYSOPER")
	SysBACKUP = AdminRole("SYSBACKUP")
	SysDG     = AdminRole("SYSDG")
	SysKM     = AdminRole("SYSKM")
	SysRAC    = AdminRole("SYSRAC")
	SysASM    = AdminRole("SYSASM")
)

var adminRoles = []AdminRole{
	SysDBA, SysOPER, SysBACKUP, SysDG, SysKM, SysRAC, SysASM,
}
