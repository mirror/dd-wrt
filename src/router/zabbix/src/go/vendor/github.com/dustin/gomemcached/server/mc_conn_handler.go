// Useful functions for building your own memcached server.
package memcached

import (
	"io"

	"github.com/dustin/gomemcached"
)

type funcHandler func(io.Writer, *gomemcached.MCRequest) *gomemcached.MCResponse

func (fh funcHandler) HandleMessage(w io.Writer, msg *gomemcached.MCRequest) *gomemcached.MCResponse {
	return fh(w, msg)
}

// Request handler for doing server stuff.
type RequestHandler interface {
	// Handle a message from the client.
	// If the message should cause the connection to terminate,
	// the Fatal flag should be set.  If the message requires no
	// response, return nil
	//
	// Most clients should ignore the io.Writer unless they want
	// complete control over the response.
	HandleMessage(io.Writer, *gomemcached.MCRequest) *gomemcached.MCResponse
}

// Convert a request handler function as a RequestHandler.
func FuncHandler(f func(io.Writer, *gomemcached.MCRequest) *gomemcached.MCResponse) RequestHandler {
	return funcHandler(f)
}

func must(err error) {
	if err != nil {
		panic(err)
	}
}

// Handle until the handler returns a fatal message or a read or write
// on the socket fails.
func HandleIO(s io.ReadWriteCloser, handler RequestHandler) error {
	defer func() { must(s.Close()) }()
	var err error
	for err == nil {
		err = HandleMessage(s, s, handler)
	}
	return err
}

// Handle an individual message.
func HandleMessage(r io.Reader, w io.Writer, handler RequestHandler) error {
	req, err := ReadPacket(r)
	if err != nil {
		return err
	}

	res := handler.HandleMessage(w, &req)
	if res == nil {
		// Quiet command
		return nil
	}

	if !res.Fatal {
		res.Opcode = req.Opcode
		res.Opaque = req.Opaque
		_, err = res.Transmit(w)
		if err != nil {
			return err
		}
		return nil
	}

	return io.EOF
}

func ReadPacket(r io.Reader) (rv gomemcached.MCRequest, err error) {
	_, err = rv.Receive(r, nil)
	return
}

func transmitResponse(o io.Writer, res *gomemcached.MCResponse) (int, error) {
	return res.Transmit(o)
}
