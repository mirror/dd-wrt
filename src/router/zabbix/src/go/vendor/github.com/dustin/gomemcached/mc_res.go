package gomemcached

import (
	"encoding/binary"
	"fmt"
	"io"
)

// A memcached response
type MCResponse struct {
	// The command opcode of the command that sent the request
	Opcode CommandCode
	// The status of the response
	Status Status
	// The opaque sent in the request
	Opaque uint32
	// The CAS identifier (if applicable)
	Cas uint64
	// Extras, key, and body for this response
	Extras, Key, Body []byte
	// If true, this represents a fatal condition and we should hang up
	Fatal bool
}

// A debugging string representation of this response
func (res MCResponse) String() string {
	return fmt.Sprintf("{MCResponse status=%v keylen=%d, extralen=%d, bodylen=%d}",
		res.Status, len(res.Key), len(res.Extras), len(res.Body))
}

// Response as an error.
func (res *MCResponse) Error() string {
	return fmt.Sprintf("MCResponse status=%v, opcode=%v, opaque=%v, msg: %s",
		res.Status, res.Opcode, res.Opaque, string(res.Body))
}

func errStatus(e error) Status {
	status := Status(0xffff)
	if res, ok := e.(*MCResponse); ok {
		status = res.Status
	}
	return status
}

// True if this error represents a "not found" response.
func IsNotFound(e error) bool {
	return errStatus(e) == KEY_ENOENT
}

// False if this error isn't believed to be fatal to a connection.
func IsFatal(e error) bool {
	if e == nil {
		return false
	}
	switch errStatus(e) {
	case KEY_ENOENT, KEY_EEXISTS, NOT_STORED, TMPFAIL:
		return false
	}
	return true
}

// Number of bytes this response consumes on the wire.
func (res *MCResponse) Size() int {
	return HDR_LEN + len(res.Extras) + len(res.Key) + len(res.Body)
}

func (res *MCResponse) fillHeaderBytes(data []byte) int {
	pos := 0
	data[pos] = RES_MAGIC
	pos++
	data[pos] = byte(res.Opcode)
	pos++
	binary.BigEndian.PutUint16(data[pos:pos+2],
		uint16(len(res.Key)))
	pos += 2

	// 4
	data[pos] = byte(len(res.Extras))
	pos++
	data[pos] = 0
	pos++
	binary.BigEndian.PutUint16(data[pos:pos+2], uint16(res.Status))
	pos += 2

	// 8
	binary.BigEndian.PutUint32(data[pos:pos+4],
		uint32(len(res.Body)+len(res.Key)+len(res.Extras)))
	pos += 4

	// 12
	binary.BigEndian.PutUint32(data[pos:pos+4], res.Opaque)
	pos += 4

	// 16
	binary.BigEndian.PutUint64(data[pos:pos+8], res.Cas)
	pos += 8

	if len(res.Extras) > 0 {
		copy(data[pos:pos+len(res.Extras)], res.Extras)
		pos += len(res.Extras)
	}

	if len(res.Key) > 0 {
		copy(data[pos:pos+len(res.Key)], res.Key)
		pos += len(res.Key)
	}

	return pos
}

// Get just the header bytes for this response.
func (res *MCResponse) HeaderBytes() []byte {
	data := make([]byte, HDR_LEN+len(res.Extras)+len(res.Key))

	res.fillHeaderBytes(data)

	return data
}

// The actual bytes transmitted for this response.
func (res *MCResponse) Bytes() []byte {
	data := make([]byte, res.Size())

	pos := res.fillHeaderBytes(data)

	copy(data[pos:pos+len(res.Body)], res.Body)

	return data
}

// Send this response message across a writer.
func (res *MCResponse) Transmit(w io.Writer) (n int, err error) {
	if len(res.Body) < 128 {
		n, err = w.Write(res.Bytes())
	} else {
		n, err = w.Write(res.HeaderBytes())
		if err == nil {
			m := 0
			m, err = w.Write(res.Body)
			m += n
		}
	}
	return
}

// Fill this MCResponse with the data from this reader.
func (req *MCResponse) Receive(r io.Reader, hdrBytes []byte) (int, error) {
	if len(hdrBytes) < HDR_LEN {
		hdrBytes = []byte{
			0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0}
	}
	n, err := io.ReadFull(r, hdrBytes)
	if err != nil {
		return n, err
	}

	if hdrBytes[0] != RES_MAGIC && hdrBytes[0] != REQ_MAGIC {
		return n, fmt.Errorf("Bad magic: 0x%02x", hdrBytes[0])
	}

	klen := int(binary.BigEndian.Uint16(hdrBytes[2:4]))
	elen := int(hdrBytes[4])

	req.Opcode = CommandCode(hdrBytes[1])
	req.Status = Status(binary.BigEndian.Uint16(hdrBytes[6:8]))
	req.Opaque = binary.BigEndian.Uint32(hdrBytes[12:16])
	req.Cas = binary.BigEndian.Uint64(hdrBytes[16:24])

	bodyLen := int(binary.BigEndian.Uint32(hdrBytes[8:12])) - (klen + elen)

	buf := make([]byte, klen+elen+bodyLen)
	m, err := io.ReadFull(r, buf)
	if err == nil {
		if elen > 0 {
			req.Extras = buf[0:elen]
		}
		if klen > 0 {
			req.Key = buf[elen : klen+elen]
		}
		if klen+elen > 0 {
			req.Body = buf[klen+elen:]
		}
	}

	return n + m, err
}
