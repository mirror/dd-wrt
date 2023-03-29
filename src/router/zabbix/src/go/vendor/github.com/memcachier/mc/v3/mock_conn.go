package mc

// Mocks the connection between the client and memcached servers.

import (
	"strconv"
	"strings"
)

// mockConn is a mock connection.
type mockConn struct {
	serverId   string
	successMod int
	counter    int
	backupMsg  msg
}

// newMockConn creates a new mockConn which allows for a certain failure pattern
// which is configured via the address parameter. The failure pattern consists
// of an integer that defines which requests will succeed, e.g. 2 means every
// second request succeeds. The address has the following structure:
// <id>-<failure-pattern>:<port> (port gets automatically inserted).
func newMockConn(address, scheme, username, password string, config *Config) mcConn {
	id_mod := strings.Split(strings.Split(address, ":")[0], "-")
	id := id_mod[0]
	mod := 1
	if len(id_mod) > 1 {
		var err error
		mod, err = strconv.Atoi(id_mod[1])
		if err != nil {
			mod = 1
		}
	}

	mockConn := &mockConn{
		serverId:   id,
		successMod: mod,
	}
	return mockConn
}

func (mc *mockConn) perform(m *msg) error {
	mc.counter++
	if mc.counter%mc.successMod == 0 {
		m.val = m.val + m.key + "," + mc.serverId + "," + strconv.Itoa(mc.counter)
		return nil
	}
	return &Error{StatusNetworkError, "Mock network error", nil}
}

func (mc *mockConn) performStats(m *msg) (McStats, error) {
	return nil, nil
}

func (mc *mockConn) quit(m *msg) {
}

func (mc *mockConn) backup(m *msg) {
	backupMsg(m, &mc.backupMsg)
}

func (mc *mockConn) restore(m *msg) {
	restoreMsg(m, &mc.backupMsg)
}
