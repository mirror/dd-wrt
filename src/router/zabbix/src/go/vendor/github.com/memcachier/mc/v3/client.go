// Package mc is a memcache client for Go supporting binary protocol and SASL
// authentication.
package mc

import (
	"fmt"
	"strings"
	"time"
)

// Protocol:
// Contains the actual memcache commands a user cares about.
// We document the protocol a little with each command, you can find the
// official documentation at:
// * https://github.com/memcached/memcached/blob/master/doc/protocol-binary.xml
// * https://github.com/memcached/memcached/blob/master/doc/protocol.txt
// * http://code.google.com/p/memcached/wiki/SASLAuthProtocol
// * http://tools.ietf.org/html/rfc4422 (SASL)
// However, sadly none of these are 100% accurate and you have to look at the
// memcached source code to find any missing cases or mismatches.
//
// * Protocol uses standard network order for bytes (big-endian)

// Command Variants:
// One quick note on memcache commands, many of them support the following
// variants [R, Q, K, KQ], e.g., GET can be GETK or GETQ...
// * <cmd>K : Include the KEY in the response.
// * <cmd>Q : Quiet version of a command. This means if the key doesn't exist,
//            no response is sent.
// * R<cmd> : Ranged version of the command. Not actually implemented by
//            memcached, just there for future extension if needed. So we
//            ignore in this client.

// Multi-Get:
// Simply implemented using GETQ. It's used for 'pipelining' requests, where the
// client sends many GETQ's without checking the response until the very end
// (batching). The memcached server doesn't do anything special, it sends
// response straight away, so it relies on the clients socket queuing up the
// responses on its buffer.

// Response:
// In addition to the key, value & extras we always get back in a response the
// status, CAS and opaque (although a user of a memcache client probably never
// cares about opaque, only we, the implementer of a memcache client, may care
// as it can be used for matching request with responses...)

// Expiration.
// * In seconds - when value of int is less than or equal
//   to: 60 * 60 * 24 * 30 (e.g., seconds in a month).
// * As a UNIX timestamp - otherwise.
//
// * Memcached accounts for time passing with a single global counter updated
//   once a second, so it therefore has an error margin of 1 second (as you
//   may do a set with expiration 0.5 seconds before it does a global time
//   update, and that 0.5 seconds will expire your key by 1 whole second).
// * Error margin is always under time, not over. E.g., a expiration of 4
//   seconds will actually expire somewhere in the range of (3,4) seconds.

// Client represents a memcached client that is connected to a list of servers
type Client struct {
	servers []*server
	config  *Config
}

// NewMC creates a new client with the default configuration. For the default
// configuration see DefaultConfig.
func NewMC(servers, username, password string) *Client {
	return NewMCwithConfig(servers, username, password, DefaultConfig())
}

// NewMCwithConfig creates a new client for a given configuration
func NewMCwithConfig(servers, username, password string, config *Config) *Client {
	return newMockableMC(servers, username, password, config, newServerConn)
}

// newMockableMC creates a new client for testing that allows to mock the server
// connection
func newMockableMC(servers, username, password string, config *Config, newMcConn connGen) *Client {
	client := &Client{config: config}

	s := func(r rune) bool {
		return r == ',' || r == ';' || r == ' '
	}
	serverList := strings.FieldsFunc(servers, s)
	for _, addr := range serverList {
		client.servers = append(client.servers,
			newServer(addr, username, password, config, newMcConn))
	}

	client.config.Hasher.update(client.servers)

	return client
}

func (c *Client) perform(m *msg) error {
	// failover on error
	for {
		s, err := c.getServer(m.key)
		if err != nil {
			return err
		}
		err = s.perform(m)
		if err != nil && err.(*Error).Status == StatusNetworkError && c.config.Failover {
			// Failover on network errors
			if s.changeAlive(false) {
				go c.wakeUp(s)
			}
			continue
		}
		return err
	}
	return nil
}

func (c *Client) wakeUp(s *server) {
	time.Sleep(c.config.DownRetryDelay)
	s.changeAlive(true)
}

func (c *Client) getServer(key string) (*server, error) {
	idx, err := c.config.Hasher.getServerIndex(key)
	if err != nil {
		return nil, err
	}
	nServers := uint(len(c.servers))
	for i := uint(0); i < nServers; i++ {
		s := c.servers[(idx+i)%nServers]
		if s.isAlive {
			return s, nil
		}
	}
	return nil, &Error{StatusNetworkError, "All server currently dead", nil}
}

// Get retrieves a value from the cache.
func (c *Client) Get(key string) (val string, flags uint32, cas uint64, err error) {
	// Variants: [R] Get [Q, K, KQ]
	// Request : MUST key; MUST NOT value, extras
	// Response: MAY key, value, extras ([0..3] flags)
	return c.getCAS(key, 0)
}

// getCAS retrieves a value in the cache but only if the CAS specified matches
// the CAS argument.
//
// NOTE: GET doesn't actually care about CAS, but we want this internally for
// testing purposes, to be able to test that a memcache server obeys the proper
// semantics of ignoring CAS with GETs.
func (c *Client) getCAS(key string, ocas uint64) (val string, flags uint32, cas uint64, err error) {
	m := &msg{
		header: header{
			Op:  opGet,
			CAS: uint64(ocas),
		},
		oextras: []interface{}{&flags},
		key:     key,
	}

	err = c.perform(m)
	return m.val, flags, m.CAS, err
}

// GAT (get and touch) retrieves the value associated with the key and updates
// its expiration time.
func (c *Client) GAT(key string, exp uint32) (val string, flags uint32, cas uint64, err error) {
	// Variants: GAT [Q, K, KQ]
	// Request : MUST key, extras; MUST NOT value
	// Response: MAY key, value, extras ([0..3] flags)
	m := &msg{
		header: header{
			Op: opGAT,
		},
		iextras: []interface{}{exp},
		oextras: []interface{}{&flags},
		key:     key,
	}

	err = c.perform(m)
	return m.val, flags, m.CAS, err
}

// Touch updates the expiration time on a key/value pair in the cache.
func (c *Client) Touch(key string, exp uint32) (cas uint64, err error) {
	// Variants: Touch
	// Request : MUST key, extras; MUST NOT value
	// Response: MUST NOT key, value, extras
	m := &msg{
		header: header{
			Op: opTouch,
		},
		iextras: []interface{}{exp},
		key:     key,
	}

	err = c.perform(m)
	return m.CAS, err
}

// Set sets a key/value pair in the cache.
func (c *Client) Set(key, val string, flags, exp uint32, ocas uint64) (cas uint64, err error) {
	// Variants: [R] Set [Q]
	return c.setGeneric(opSet, key, val, ocas, flags, exp)
}

// Replace replaces an existing key/value in the cache. Fails if key doesn't
// already exist in cache.
func (c *Client) Replace(key, val string, flags, exp uint32, ocas uint64) (cas uint64, err error) {
	// Variants: Replace [Q]
	return c.setGeneric(opReplace, key, val, ocas, flags, exp)
}

// Add adds a new key/value to the cache. Fails if the key already exists in the
// cache.
func (c *Client) Add(key, val string, flags, exp uint32) (cas uint64, err error) {
	// Variants: Add [Q]
	return c.setGeneric(opAdd, key, val, 0, flags, exp)
}

// Set/Add/Replace a key/value pair in the cache.
func (c *Client) setGeneric(op opCode, key, val string, ocas uint64, flags, exp uint32) (cas uint64, err error) {
	// Request : MUST key, value, extras ([0..3] flags, [4..7] expiration)
	// Response: MUST NOT key, value, extras
	// CAS: If a CAS is specified (non-zero), all sets only succeed if the key
	//      exists and has the CAS specified. Otherwise, an error is returned.
	m := &msg{
		header: header{
			Op:  op,
			CAS: ocas,
		},
		iextras: []interface{}{flags, exp},
		key:     key,
		val:     val,
	}

	err = c.perform(m)
	return m.CAS, err
}

// Incr increments a value in the cache. The value must be an unsigned 64bit
// integer stored as an ASCII string. It will wrap when incremented outside the
// range.
func (c *Client) Incr(key string, delta, init uint64, exp uint32, ocas uint64) (n, cas uint64, err error) {
	return c.incrdecr(opIncrement, key, delta, init, exp, ocas)
}

// Decr decrements a value in the cache. The value must be an unsigned 64bit
// integer stored as an ASCII string. It can't be decremented below 0.
func (c *Client) Decr(key string, delta, init uint64, exp uint32, ocas uint64) (n, cas uint64, err error) {
	return c.incrdecr(opDecrement, key, delta, init, exp, ocas)
}

// Incr/Decr a key/value pair in the cache.
func (c *Client) incrdecr(op opCode, key string, delta, init uint64, exp uint32, ocas uint64) (n, cas uint64, err error) {
	// Variants: [R] Incr [Q], [R] Decr [Q]
	// Request : MUST key, extras; MUST NOT value
	//   Extras: [ 0.. 7] Amount to add/sub
	//           [ 8..15] Initial value for counter (if key doesn't exist)
	//           [16..20] Expiration
	// Response: MUST value; MUST NOT key, extras

	// * response value is 64 bit unsigned binary number.
	// * if the key doesn't exist and the expiration is all 1's (0xffffffff) then
	//   the operation will fail with NOT_FOUND.
	m := &msg{
		header: header{
			Op:  op,
			CAS: ocas,
		},
		iextras: []interface{}{delta, init, exp},
		key:     key,
	}

	err = c.perform(m)
	if err != nil {
		return
	}
	// value is returned as an unsigned 64bit integer (i.e., not as a string)
	return readInt(m.val), m.CAS, nil
}

// Convert string stored to an uint64 (where no actual byte changes are needed).
func readInt(b string) uint64 {
	switch len(b) {
	case 8: // 64 bit
		return uint64(uint64(b[7]) | uint64(b[6])<<8 | uint64(b[5])<<16 | uint64(b[4])<<24 |
			uint64(b[3])<<32 | uint64(b[2])<<40 | uint64(b[1])<<48 | uint64(b[0])<<56)
	}

	panic(fmt.Sprintf("mc: don't know how to parse string with %d bytes", len(b)))
}

// Append appends the value to the existing value for the key specified. An
// error is thrown if the key doesn't exist.
func (c *Client) Append(key, val string, ocas uint64) (cas uint64, err error) {
	// Variants: [R] Append [Q]
	// Request : MUST key, value; MUST NOT extras
	// Response: MUST NOT key, value, extras
	m := &msg{
		header: header{
			Op:  opAppend,
			CAS: ocas,
		},
		key: key,
		val: val,
	}

	err = c.perform(m)
	return m.CAS, err
}

// Prepend prepends the value to the existing value for the key specified. An
// error is thrown if the key doesn't exist.
func (c *Client) Prepend(key, val string, ocas uint64) (cas uint64, err error) {
	// Variants: [R] Append [Q]
	// Request : MUST key, value; MUST NOT extras
	// Response: MUST NOT key, value, extras
	m := &msg{
		header: header{
			Op:  opPrepend,
			CAS: ocas,
		},
		key: key,
		val: val,
	}

	err = c.perform(m)
	return m.CAS, err
}

// Del deletes a key/value from the cache.
func (c *Client) Del(key string) (err error) {
	return c.DelCAS(key, 0)
}

// DelCAS deletes a key/value from the cache but only if the CAS specified
// matches the CAS in the cache.
func (c *Client) DelCAS(key string, cas uint64) (err error) {
	// Variants: [R] Del [Q]
	// Request : MUST key; MUST NOT value, extras
	// Response: MUST NOT key, value, extras
	m := &msg{
		header: header{
			Op:  opDelete,
			CAS: cas,
		},
		key: key,
	}

	return c.perform(m)
}

// Flush flushes the cache, that is, invalidate all keys. Note, this doesn't
// typically free memory on a memcache server (doing so compromises the O(1)
// nature of memcache). Instead nearly all servers do lazy expiration, where
// they don't free memory but won't return any keys to you that have expired.
func (c *Client) Flush(when uint32) (err error) {
	// Variants: Flush [Q]
	// Request : MUST NOT key, value; MAY extras ([0..3] expiration)
	// Response: MUST NOT key, value, extras

	// optional expiration means that the flush won't become active until that
	// point in time, hence why the argument is called 'when' as that is more
	// descriptive of its function.
	m := &msg{
		header: header{
			Op: opFlush,
		},
		iextras: []interface{}{when},
	}

	for _, s := range c.servers {
		if s.isAlive {
			var ms msg = *m
			err = s.perform(&ms)
		}
	}
	return err // retrns err from last perform but maybe should handle differently
}

// NoOp sends a No-Op message to the memcache server. This can be used as a
// heartbeat for the server to check it's functioning fine still.
func (c *Client) NoOp() (err error) {
	// Variants: NoOp
	// Request : MUST NOT key, value, extras
	// Response: MUST NOT key, value, extras
	m := &msg{
		header: header{
			Op: opNoop,
		},
	}

	for _, s := range c.servers {
		if s.isAlive {
			var ms msg = *m
			err = s.perform(&ms)
		}
	}
	return err // retrns err from last perform but maybe should handle differently
}

// Version gets the version of the memcached server connected to.
func (c *Client) Version() (vers map[string]string, err error) {
	// Variants: Version
	// Request : MUST NOT key, value, extras
	// Response: MUST NOT key, extras; MUST value

	// value is the version as a string in form "X.Y.Z"
	m := &msg{
		header: header{
			Op: opVersion,
		},
	}

	vers = make(map[string]string)
	for _, s := range c.servers {
		if s.isAlive {
			var ms msg = *m
			err = s.perform(&ms)
			if err == nil {
				vers[s.address] = ms.val
			}
		}
	}

	return
}

// Quit closes the connection with memcached server (nicely).
func (c *Client) Quit() {
	// Variants: Quit [Q]
	// Request : MUST NOT key, value, extras
	// Response: MUST NOT key, value, extras
	m := &msg{
		header: header{
			Op: opQuit,
		},
	}

	for _, s := range c.servers {
		var ms msg = *m
		s.quit(&ms)
	}
}

// StatsWithKey returns some statistics about the memcached server. It supports
// sending across a key to the server to select which statistics should be
// returned.
func (c *Client) StatsWithKey(key string) (map[string]McStats, error) {
	// Variants: Stats
	// Request : MAY HAVE key, MUST NOT value, extra
	// Response: Serries of responses that MUST HAVE key, value; followed by one
	//           response that MUST NOT have key, value. ALL MUST NOT extras.
	m := &msg{
		header: header{
			Op: opStat,
		},
		key: key,
	}

	allStats := make(map[string]McStats)
	for _, s := range c.servers {
		if s.isAlive {
			stats, err := s.performStats(m)
			if err != nil {
				return nil, err
			}
			allStats[s.address] = stats
		}
	}

	return allStats, nil
}

// Stats returns some statistics about the memcached server.
func (c *Client) Stats() (stats map[string]McStats, err error) {
	return c.StatsWithKey("")
}

// StatsReset resets the statistics stored at the memcached server.
func (c *Client) StatsReset() (err error) {
	_, err = c.StatsWithKey("reset")
	return err
}
