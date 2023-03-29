// Copyright 2017, 2022 The Godror Authors
//
//
// SPDX-License-Identifier: UPL-1.0 OR Apache-2.0

package godror

/*
#include "dpiImpl.h"
*/
import "C"
import (
	"bufio"
	"errors"
	"fmt"
	"io"
	"runtime"
	"strings"
	"sync"
	"unicode/utf8"
	"unsafe"
)

// Lob is for reading/writing a LOB.
type Lob struct {
	io.Reader
	IsClob bool
}

var _ = (io.Reader)((*Lob)(nil))
var _ = (io.ReaderAt)((*Lob)(nil))

// Hijack the underlying lob reader/writer, and
// return a DirectLob for reading/writing the lob directly.
//
// After this, the Lob is unusable!
func (lob *Lob) Hijack() (*DirectLob, error) {
	if lob == nil || lob.Reader == nil {
		return nil, errors.New("lob is nil")
	}
	lr, ok := lob.Reader.(*dpiLobReader)
	if !ok {
		return nil, fmt.Errorf("Lob.Reader is %T, not *dpiLobReader", lob.Reader)
	}
	lob.Reader = nil
	return &DirectLob{drv: lr.drv, dpiLob: lr.dpiLob}, nil
}

// WriteTo writes data to w until there's no more data to write or when an error occurs.
// The return value n is the number of bytes written. Any error encountered during the write is also returned.
//
// Will use Lob.Reader.WriteTo, if Lob.Reader implements io.WriterTo.
func (lob *Lob) WriteTo(w io.Writer) (n int64, err error) {
	if wt, ok := lob.Reader.(io.WriterTo); ok {
		return wt.WriteTo(w)
	}
	return io.CopyBuffer(w, lob.Reader, make([]byte, 1<<20))
}

// NewBufferedReader returns a new bufio.Reader with the given size (or 1M if 0).
func (lob *Lob) NewBufferedReader(size int) *bufio.Reader {
	if size <= 0 {
		size = 1 << 20
	}
	return bufio.NewReaderSize(lob.Reader, size)
}

// Size exposes the underlying Reader's Size method, if it is supported.
func (lob *Lob) Size() (int64, error) {
	if lr, ok := lob.Reader.(interface{ Size() (int64, error) }); ok {
		return lr.Size()
	}
	return 0, ErrNotSupported
}

// ReadAt exposes the underlying Reader's ReadAt method, if it is supported.
func (lob *Lob) ReadAt(p []byte, off int64) (int, error) {
	if lr, ok := lob.Reader.(io.ReaderAt); ok {
		return lr.ReadAt(p, off)
	}
	return 0, ErrNotSupported
}

// Scan assigns a value from a database driver.
//
// The src value will be of one of the following types:
//
//    int64
//    float64
//    bool
//    []byte
//    string
//    time.Time
//    nil - for NULL values
//
// An error should be returned if the value cannot be stored
// without loss of information.
func (dlr *dpiLobReader) Scan(src interface{}) error {
	b, ok := src.([]byte)
	if !ok {
		return fmt.Errorf("cannot convert LOB to %T", src)
	}
	_ = b
	return nil
}

var _ = io.ReadCloser((*dpiLobReader)(nil))
var _ = io.ReaderAt((*dpiLobReader)(nil))

type dpiLobReader struct {
	*drv
	dpiLob              *C.dpiLob
	buf                 []byte
	offset, sizePlusOne C.uint64_t
	mu                  sync.Mutex
	chunkSize           C.uint32_t
	bufR, bufW          int
	finished            bool
	IsClob              bool
}

// WriteTo writes data to w until there's no more data to write or when an error occurs.
// The return value n is the number of bytes written. Any error encountered during the write is also returned.
//
// Uses efficient, multiple-of-LOB-chunk-size buffered reads.
func (dlr *dpiLobReader) WriteTo(w io.Writer) (n int64, err error) {
	size := dlr.ChunkSize()
	const minBufferSize = 1 << 20
	if size <= 0 {
		size = minBufferSize
	} else {
		for size < minBufferSize/2 { // at most 1M
			size *= 2
		}
	}
	return io.CopyBuffer(
		w,
		// Mask WriteTo method
		io.Reader(struct {
			io.Reader
		}{dlr}),
		make([]byte, size))
}

// ChunkSize returns the LOB's native chunk size. Reads/writes with a multiply of this size is the most performant.
func (dlr *dpiLobReader) ChunkSize() int {
	dlr.mu.Lock()
	defer dlr.mu.Unlock()
	if dlr.chunkSize != 0 {
		return int(dlr.chunkSize)
	}
	if err := dlr.checkExec(func() C.int {
		return C.dpiLob_getChunkSize(dlr.dpiLob, &dlr.chunkSize)
	}); err != nil {
		dlr.chunkSize = 0
		return -1
	}
	return int(dlr.chunkSize)
}

// Read from LOB. It does buffer the reading internally against short buffers (io.ReadAll).
func (dlr *dpiLobReader) Read(p []byte) (int, error) {
	dlr.mu.Lock()
	defer dlr.mu.Unlock()
	logger := getLogger()
	if logger != nil {
		logger.Log("msg", "Read", "bufR", dlr.bufR, "bufW", dlr.bufW, "buf", cap(dlr.buf))
	}
	if dlr.buf == nil {
		if dlr.chunkSize == 0 {
			if err := dlr.checkExec(func() C.int {
				return C.dpiLob_getChunkSize(dlr.dpiLob, &dlr.chunkSize)
			}); err != nil {
				return 0, fmt.Errorf("getChunkSize: %w", err)
			}
		}
		// If the dest buffer is big enough, avoid copying.
		if ulen := C.uint64_t(len(p)); ulen >= C.uint64_t(dlr.chunkSize) || dlr.sizePlusOne != 0 && ulen+1 >= dlr.sizePlusOne {
			if logger != nil {
				logger.Log("msg", "direct read", "p", len(p), "chunkSize", dlr.chunkSize)
			}
			return dlr.read(p)
		}
		cs := int(dlr.chunkSize)
		dlr.buf = make([]byte, (maxI(len(p), 1<<20-cs)/cs+1)*cs)
		dlr.bufR, dlr.bufW = 0, 0
	} else if dlr.bufW != 0 && cap(dlr.buf) != 0 {
		var n int
		if dlr.bufR < dlr.bufW {
			n = copy(p, dlr.buf[dlr.bufR:dlr.bufW])
			dlr.bufR += n
		}
		if dlr.bufR == dlr.bufW {
			dlr.bufR, dlr.bufW = 0, 0
		}
		if n != 0 {
			return n, nil
		}
	}
	var err error
	// We only read into dlr.buf when it's empty, dlr.bufR == dlr.bufW == 0
	dlr.bufW, err = dlr.read(dlr.buf)
	if logger != nil {
		logger.Log("msg", "dlr.read", "bufR", dlr.bufR, "bufW", dlr.bufW, "chunkSize", dlr.chunkSize, "error", err)
	}
	dlr.bufR = copy(p, dlr.buf[:dlr.bufW])
	if err == io.EOF && dlr.bufW != dlr.bufR {
		err = nil
	}
	return dlr.bufR, err
}

var ErrCLOB = errors.New("CLOB is not supported")

// Size returns the LOB's size. It returns ErrCLOB for CLOB,
// (works only for BLOBs), as Oracle reports CLOB size in runes, not in bytes!
func (dlr *dpiLobReader) Size() (int64, error) {
	dlr.mu.Lock()
	err := dlr.getSize()
	size := dlr.sizePlusOne - 1
	isClob := dlr.IsClob
	dlr.mu.Unlock()
	if err == nil && isClob {
		err = ErrCLOB
	}
	return int64(size), err
}
func (dlr *dpiLobReader) getSize() error {
	if dlr.sizePlusOne != 0 {
		return nil
	}
	var err error
	runtime.LockOSThread()
	if err = dlr.checkExecNoLOT(func() C.int {
		return C.dpiLob_getSize(dlr.dpiLob, &dlr.sizePlusOne)
	}); err != nil {
		err = fmt.Errorf("getSize: %w", err)
		C.dpiLob_close(dlr.dpiLob)
		dlr.dpiLob = nil
	}
	runtime.UnlockOSThread()
	dlr.sizePlusOne++
	return err
}

// read does the real LOB reading.
func (dlr *dpiLobReader) read(p []byte) (int, error) {
	if dlr == nil {
		return 0, errors.New("read on nil dpiLobReader")
	}
	logger := getLogger()
	if logger != nil {
		logger.Log("msg", "LOB Read", "dlr", fmt.Sprintf("%p", dlr), "offset", dlr.offset, "size", dlr.sizePlusOne, "finished", dlr.finished, "clob", dlr.IsClob)
	}
	if dlr.finished || dlr.dpiLob == nil {
		return 0, io.EOF
	}
	if len(p) < 1 || dlr.IsClob && len(p) < 4 {
		return 0, io.ErrShortBuffer
	}
	runtime.LockOSThread()
	defer runtime.UnlockOSThread()
	// For CLOB, sizePlusOne and offset counts the CHARACTERS!
	// See https://oracle.github.io/odpi/doc/public_functions/dpiLob.html dpiLob_readBytes
	if dlr.sizePlusOne == 0 { // first read
		// never read size before
		if err := dlr.getSize(); err != nil {
			var coder interface{ Code() int }
			if errors.As(err, &coder) && coder.Code() == 22922 || strings.Contains(err.Error(), "invalid dpiLob handle") {
				return 0, io.EOF
			}
			return 0, err
		}

		var lobType C.dpiOracleTypeNum
		if err := dlr.checkExecNoLOT(func() C.int {
			return C.dpiLob_getType(dlr.dpiLob, &lobType)
		}); err == nil &&
			(2017 <= lobType && lobType <= 2019) {
			dlr.IsClob = lobType == 2017 || lobType == 2018 // CLOB and NCLOB
		}
	}
	n := C.uint64_t(len(p))
	amount := n
	if dlr.IsClob {
		amount /= 4 // dpiLob_readBytes' amount is the number of CHARACTERS for CLOBs.
	}
	// fmt.Printf("%p.Read offset=%d sizePlusOne=%d n=%d\n", dlr.dpiLob, dlr.offset, dlr.sizePlusOne, n)
	if logger != nil {
		logger.Log("msg", "Read", "offset", dlr.offset, "sizePlusOne", dlr.sizePlusOne, "n", n, "amount", amount)
	}
	if !dlr.IsClob && dlr.offset+1 >= dlr.sizePlusOne {
		if logger != nil {
			logger.Log("msg", "LOB reached end", "offset", dlr.offset, "size", dlr.sizePlusOne)
		}
		return 0, io.EOF
	}
	if err := dlr.drv.checkExecNoLOT(func() C.int {
		return C.dpiLob_readBytes(dlr.dpiLob, dlr.offset+1, amount, (*C.char)(unsafe.Pointer(&p[0])), &n)
	}); err != nil {
		if logger != nil {
			logger.Log("msg", "readBytes", "error", err)
		}
		C.dpiLob_close(dlr.dpiLob)
		dlr.dpiLob = nil
		if logger != nil {
			logger.Log("msg", "LOB read", "error", err)
		}
		var codeErr interface{ Code() int }
		if dlr.finished = errors.As(err, &codeErr) && codeErr.Code() == 1403; dlr.finished {
			dlr.offset += n
			return int(n), io.EOF
		}
		return int(n), fmt.Errorf("dpiLob_readbytes(lob=%p offset=%d n=%d): %w", dlr.dpiLob, dlr.offset, len(p), err)
	}
	if logger != nil {
		logger.Log("msg", "read", "n", n)
	}
	if dlr.IsClob {
		dlr.offset += C.uint64_t(utf8.RuneCount(p[:n]))
	} else {
		dlr.offset += n
	}
	var err error
	if amount != 0 && n == 0 || !dlr.IsClob && dlr.offset+1 >= dlr.sizePlusOne {
		C.dpiLob_close(dlr.dpiLob)
		dlr.dpiLob = nil
		dlr.finished = true
		err = io.EOF
	}
	if logger != nil {
		logger.Log("msg", "LOB", "n", n, "offset", dlr.offset, "size", dlr.sizePlusOne, "finished", dlr.finished, "clob", dlr.IsClob, "error", err)
	}
	return int(n), err
}

// ReadAt reads at the specified offset (in bytes).
// Works only for BLOBs!
func (dlr *dpiLobReader) ReadAt(p []byte, off int64) (int, error) {
	dlr.mu.Lock()
	defer dlr.mu.Unlock()
	if dlr.IsClob {
		return 0, ErrCLOB
	}
	n := C.uint64_t(len(p))
	err := dlr.checkExec(func() C.int {
		return C.dpiLob_readBytes(dlr.dpiLob, C.uint64_t(off+1), n, (*C.char)(unsafe.Pointer(&p[0])), &n)
	})
	if err != nil {
		err = fmt.Errorf("readBytes at %d for %d: %w", off, n, dlr.getError())
	}
	return int(n), err
}
func (dlr *dpiLobReader) Close() error {
	if dlr == nil || dlr.dpiLob == nil {
		return nil
	}
	lob := dlr.dpiLob
	dlr.dpiLob = nil
	return closeLob(dlr, lob)
}

type dpiLobWriter struct {
	*drv
	dpiLob *C.dpiLob
	offset C.uint64_t
	opened bool
	isClob bool
}

func (dlw *dpiLobWriter) Write(p []byte) (int, error) {
	runtime.LockOSThread()
	defer runtime.UnlockOSThread()

	lob := dlw.dpiLob
	if !dlw.opened {
		// fmt.Printf("open %p\n", lob)
		if err := dlw.drv.checkExecNoLOT(func() C.int {
			return C.dpiLob_openResource(lob)
		}); err != nil {
			return 0, fmt.Errorf("openResources(%p): %w", lob, err)
		}
		dlw.opened = true
	}

	n := C.uint64_t(len(p))
	if err := dlw.drv.checkExecNoLOT(func() C.int {
		return C.dpiLob_writeBytes(lob, dlw.offset+1, (*C.char)(unsafe.Pointer(&p[0])), n)
	}); err != nil {
		err = fmt.Errorf("writeBytes(%p, offset=%d, data=%d): %w", lob, dlw.offset, n, err)
		dlw.dpiLob = nil
		closeLob(dlw, lob)
		return 0, err
	}
	// fmt.Printf("written %q into %p@%d\n", p[:n], lob, dlw.offset)
	dlw.offset += n

	return int(n), nil
}

func (dlw *dpiLobWriter) Close() error {
	if dlw == nil || dlw.dpiLob == nil {
		return nil
	}
	lob := dlw.dpiLob
	dlw.dpiLob = nil
	return closeLob(dlw, lob)
}

func closeLob(d interface{ getError() error }, lob *C.dpiLob) error {
	if lob == nil {
		return nil
	}

	runtime.LockOSThread()
	defer runtime.UnlockOSThread()

	var isOpen C.int
	if C.dpiLob_getIsResourceOpen(lob, &isOpen) != C.DPI_FAILURE && isOpen == 1 {
		if C.dpiLob_closeResource(lob) == C.DPI_FAILURE {
			if err := d.getError(); err != nil {
				var codeErr interface{ Code() int }
				if errors.As(err, &codeErr) && codeErr.Code() != 22289 { // cannot perform %s operation on an unopened file or LOB
					return fmt.Errorf("closeResource(%p): %w", lob, err)
				}
			}
		}
	}
	C.dpiLob_release(lob)
	return nil
}

// DirectLob holds a Lob and allows direct (Read/WriteAt, not streaming Read/Write) operations on it.
type DirectLob struct {
	drv            *drv
	dpiLob         *C.dpiLob
	opened, isClob bool
}

var _ = io.ReaderAt((*DirectLob)(nil))
var _ = io.WriterAt((*DirectLob)(nil))

// NewTempLob returns a temporary LOB as DirectLob.
func (c *conn) NewTempLob(isClob bool) (*DirectLob, error) {
	typ := C.uint(C.DPI_ORACLE_TYPE_BLOB)
	if isClob {
		typ = C.DPI_ORACLE_TYPE_CLOB
	}
	lob := DirectLob{drv: c.drv, isClob: isClob}
	if err := c.checkExec(func() C.int { return C.dpiConn_newTempLob(c.dpiConn, typ, &lob.dpiLob) }); err != nil {
		return nil, fmt.Errorf("newTempLob: %w", err)
	}
	return &lob, nil
}

// Close the Lob.
func (dl *DirectLob) Close() error {
	if !dl.opened {
		return nil
	}
	lob := dl.dpiLob
	dl.opened, dl.dpiLob = false, nil
	return closeLob(dl.drv, lob)
}

// Size returns the size of the LOB.
//
// WARNING: for historical reasons, Oracle stores CLOBs and NCLOBs using the UTF-16 encoding,
// regardless of what encoding is otherwise in use by the database.
// The number of characters, however, is defined by the number of UCS-2 codepoints.
// For this reason, if a character requires more than one UCS-2 codepoint,
// the size returned will be inaccurate and care must be taken to account for the difference!
func (dl *DirectLob) Size() (int64, error) {
	var n C.uint64_t
	if dl.dpiLob == nil {
		return 0, nil
	}
	if err := dl.drv.checkExec(func() C.int {
		return C.dpiLob_getSize(dl.dpiLob, &n)
	}); err != nil {
		var coder interface{ Code() int }
		if errors.As(err, &coder) && coder.Code() == 22922 || strings.Contains(err.Error(), "invalid dpiLob handle") {
			return 0, nil
		}
		return int64(n), fmt.Errorf("getSize: %w", err)
	}
	return int64(n), nil
}

// Trim the LOB to the given size.
func (dl *DirectLob) Trim(size int64) error {
	if err := dl.drv.checkExec(func() C.int {
		return C.dpiLob_trim(dl.dpiLob, C.uint64_t(size))
	}); err != nil {
		return fmt.Errorf("trim: %w", err)
	}
	return nil
}

// Set the contents of the LOB to the given byte slice.
// The LOB is cleared first.
func (dl *DirectLob) Set(p []byte) error {
	if err := dl.drv.checkExec(func() C.int {
		return C.dpiLob_setFromBytes(dl.dpiLob, (*C.char)(unsafe.Pointer(&p[0])), C.uint64_t(len(p)))
	}); err != nil {
		return fmt.Errorf("setFromBytes: %w", err)
	}
	return nil
}

// ReadAt reads at most len(p) bytes into p at offset.
//
// CLOB's offset must be in amount of characters, and does not work reliably!
//
// WARNING: for historical reasons, Oracle stores CLOBs and NCLOBs using the UTF-16 encoding,
// regardless of what encoding is otherwise in use by the database.
// The number of characters, however, is defined by the number of UCS-2 codepoints.
// For this reason, if a character requires more than one UCS-2 codepoint,
// the size returned will be inaccurate and care must be taken to account for the difference!
func (dl *DirectLob) ReadAt(p []byte, offset int64) (int, error) {
	n := C.uint64_t(len(p))
	if dl.dpiLob == nil {
		return 0, io.EOF
	}
	amount := n
	if dl.isClob {
		amount /= 4
		if amount == 0 {
			return 0, io.ErrShortBuffer
		}
	}
	if err := dl.drv.checkExec(func() C.int {
		return C.dpiLob_readBytes(dl.dpiLob, C.uint64_t(offset)+1, n, (*C.char)(unsafe.Pointer(&p[0])), &n)
	}); err != nil {
		return int(n), fmt.Errorf("readBytes: %w", err)
	}
	return int(n), nil
}

// WriteAt writes p starting at offset.
func (dl *DirectLob) WriteAt(p []byte, offset int64) (int, error) {
	runtime.LockOSThread()
	defer runtime.UnlockOSThread()

	if !dl.opened {
		// fmt.Printf("open %p\n", lob)
		if err := dl.drv.checkExecNoLOT(func() C.int {
			return C.dpiLob_openResource(dl.dpiLob)
		}); err != nil {
			return 0, fmt.Errorf("openResources(%p): %w", dl.dpiLob, err)
		}
		dl.opened = true
	}

	n := C.uint64_t(len(p))
	if err := dl.drv.checkExecNoLOT(func() C.int {
		return C.dpiLob_writeBytes(dl.dpiLob, C.uint64_t(offset)+1, (*C.char)(unsafe.Pointer(&p[0])), n)
	}); err != nil {
		return int(n), fmt.Errorf("writeBytes: %w", err)
	}
	return int(n), nil
}

// GetFileName Return directory alias and file name for a BFILE type LOB.
func (dl *DirectLob) GetFileName() (dir, file string, err error) {
	var directoryAliasLength, fileNameLength C.uint32_t
	var directoryAlias, fileName *C.char
	if err := dl.drv.checkExec(func() C.int {
		return C.dpiLob_getDirectoryAndFileName(dl.dpiLob,
			&directoryAlias,
			&directoryAliasLength,
			&fileName,
			&fileNameLength,
		)
	}); err != nil {
		return dir, file, fmt.Errorf("GetFileName: %w", err)
	}
	dir = C.GoStringN(directoryAlias, C.int(directoryAliasLength))
	file = C.GoStringN(fileName, C.int(fileNameLength))
	return dir, file, nil
}

func maxI(a, b int) int {
	if a < b {
		return b
	}
	return a
}
