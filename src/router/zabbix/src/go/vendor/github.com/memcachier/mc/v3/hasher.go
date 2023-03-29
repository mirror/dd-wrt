package mc

//

import (
	"hash"
	"hash/fnv"
)

type hasher interface {
	update(servers []*server)
	getServerIndex(key string) (uint, error)
}

type moduloHasher struct {
	nServers uint
	h32      hash.Hash32
}

func NewModuloHasher() hasher {
	var h hasher = &moduloHasher{h32: fnv.New32a()}
	return h
}

func (h *moduloHasher) update(servers []*server) {
	h.nServers = uint(len(servers))
}

func (h *moduloHasher) getServerIndex(key string) (uint, error) {
	if h.nServers < 1 {
		return 0, &Error{StatusNetworkError, "No server available", nil}
	}

	h.h32.Write([]byte(key))
	defer h.h32.Reset()

	return uint(h.h32.Sum32()) % h.nServers, nil
}
