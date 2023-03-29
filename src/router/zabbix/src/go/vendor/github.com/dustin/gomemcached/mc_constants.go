// Memcached binary protocol packet formats and constants.
package gomemcached

import (
	"fmt"
)

const (
	REQ_MAGIC = 0x80
	RES_MAGIC = 0x81
)

type CommandCode uint8

const (
	GET        = CommandCode(0x00)
	SET        = CommandCode(0x01)
	ADD        = CommandCode(0x02)
	REPLACE    = CommandCode(0x03)
	DELETE     = CommandCode(0x04)
	INCREMENT  = CommandCode(0x05)
	DECREMENT  = CommandCode(0x06)
	QUIT       = CommandCode(0x07)
	FLUSH      = CommandCode(0x08)
	GETQ       = CommandCode(0x09)
	NOOP       = CommandCode(0x0a)
	VERSION    = CommandCode(0x0b)
	GETK       = CommandCode(0x0c)
	GETKQ      = CommandCode(0x0d)
	APPEND     = CommandCode(0x0e)
	PREPEND    = CommandCode(0x0f)
	STAT       = CommandCode(0x10)
	SETQ       = CommandCode(0x11)
	ADDQ       = CommandCode(0x12)
	REPLACEQ   = CommandCode(0x13)
	DELETEQ    = CommandCode(0x14)
	INCREMENTQ = CommandCode(0x15)
	DECREMENTQ = CommandCode(0x16)
	QUITQ      = CommandCode(0x17)
	FLUSHQ     = CommandCode(0x18)
	APPENDQ    = CommandCode(0x19)
	PREPENDQ   = CommandCode(0x1a)
	RGET       = CommandCode(0x30)
	RSET       = CommandCode(0x31)
	RSETQ      = CommandCode(0x32)
	RAPPEND    = CommandCode(0x33)
	RAPPENDQ   = CommandCode(0x34)
	RPREPEND   = CommandCode(0x35)
	RPREPENDQ  = CommandCode(0x36)
	RDELETE    = CommandCode(0x37)
	RDELETEQ   = CommandCode(0x38)
	RINCR      = CommandCode(0x39)
	RINCRQ     = CommandCode(0x3a)
	RDECR      = CommandCode(0x3b)
	RDECRQ     = CommandCode(0x3c)

	SASL_LIST_MECHS = CommandCode(0x20)
	SASL_AUTH       = CommandCode(0x21)
	SASL_STEP       = CommandCode(0x22)

	TAP_CONNECT          = CommandCode(0x40) // Client-sent request to initiate Tap feed
	TAP_MUTATION         = CommandCode(0x41) // Notification of a SET/ADD/REPLACE/etc. on the server
	TAP_DELETE           = CommandCode(0x42) // Notification of a DELETE on the server
	TAP_FLUSH            = CommandCode(0x43) // Replicates a flush_all command
	TAP_OPAQUE           = CommandCode(0x44) // Opaque control data from the engine
	TAP_VBUCKET_SET      = CommandCode(0x45) // Sets state of vbucket in receiver (used in takeover)
	TAP_CHECKPOINT_START = CommandCode(0x46) // Notifies start of new checkpoint
	TAP_CHECKPOINT_END   = CommandCode(0x47) // Notifies end of checkpoint

	OBSERVE = CommandCode(0x92)
)

type Status uint16

const (
	SUCCESS         = Status(0x00)
	KEY_ENOENT      = Status(0x01)
	KEY_EEXISTS     = Status(0x02)
	E2BIG           = Status(0x03)
	EINVAL          = Status(0x04)
	NOT_STORED      = Status(0x05)
	DELTA_BADVAL    = Status(0x06)
	NOT_MY_VBUCKET  = Status(0x07)
	UNKNOWN_COMMAND = Status(0x81)
	ENOMEM          = Status(0x82)
	TMPFAIL         = Status(0x86)
)

// An internal representation of an item.
type MCItem struct {
	Cas               uint64
	Flags, Expiration uint32
	Data              []byte
}

// Number of bytes in a binary protocol header.
const HDR_LEN = 24

// Mapping of CommandCode -> name of command (not exhaustive)
var CommandNames map[CommandCode]string

var StatusNames map[Status]string

func init() {
	CommandNames = make(map[CommandCode]string)
	CommandNames[GET] = "GET"
	CommandNames[SET] = "SET"
	CommandNames[ADD] = "ADD"
	CommandNames[REPLACE] = "REPLACE"
	CommandNames[DELETE] = "DELETE"
	CommandNames[INCREMENT] = "INCREMENT"
	CommandNames[DECREMENT] = "DECREMENT"
	CommandNames[QUIT] = "QUIT"
	CommandNames[FLUSH] = "FLUSH"
	CommandNames[GETQ] = "GETQ"
	CommandNames[NOOP] = "NOOP"
	CommandNames[VERSION] = "VERSION"
	CommandNames[GETK] = "GETK"
	CommandNames[GETKQ] = "GETKQ"
	CommandNames[APPEND] = "APPEND"
	CommandNames[PREPEND] = "PREPEND"
	CommandNames[STAT] = "STAT"
	CommandNames[SETQ] = "SETQ"
	CommandNames[ADDQ] = "ADDQ"
	CommandNames[REPLACEQ] = "REPLACEQ"
	CommandNames[DELETEQ] = "DELETEQ"
	CommandNames[INCREMENTQ] = "INCREMENTQ"
	CommandNames[DECREMENTQ] = "DECREMENTQ"
	CommandNames[QUITQ] = "QUITQ"
	CommandNames[FLUSHQ] = "FLUSHQ"
	CommandNames[APPENDQ] = "APPENDQ"
	CommandNames[PREPENDQ] = "PREPENDQ"
	CommandNames[RGET] = "RGET"
	CommandNames[RSET] = "RSET"
	CommandNames[RSETQ] = "RSETQ"
	CommandNames[RAPPEND] = "RAPPEND"
	CommandNames[RAPPENDQ] = "RAPPENDQ"
	CommandNames[RPREPEND] = "RPREPEND"
	CommandNames[RPREPENDQ] = "RPREPENDQ"
	CommandNames[RDELETE] = "RDELETE"
	CommandNames[RDELETEQ] = "RDELETEQ"
	CommandNames[RINCR] = "RINCR"
	CommandNames[RINCRQ] = "RINCRQ"
	CommandNames[RDECR] = "RDECR"
	CommandNames[RDECRQ] = "RDECRQ"

	CommandNames[SASL_LIST_MECHS] = "SASL_LIST_MECHS"
	CommandNames[SASL_AUTH] = "SASL_AUTH"
	CommandNames[SASL_STEP] = "SASL_STEP"

	CommandNames[TAP_CONNECT] = "TAP_CONNECT"
	CommandNames[TAP_MUTATION] = "TAP_MUTATION"
	CommandNames[TAP_DELETE] = "TAP_DELETE"
	CommandNames[TAP_FLUSH] = "TAP_FLUSH"
	CommandNames[TAP_OPAQUE] = "TAP_OPAQUE"
	CommandNames[TAP_VBUCKET_SET] = "TAP_VBUCKET_SET"
	CommandNames[TAP_CHECKPOINT_START] = "TAP_CHECKPOINT_START"
	CommandNames[TAP_CHECKPOINT_END] = "TAP_CHECKPOINT_END"

	StatusNames = make(map[Status]string)
	StatusNames[SUCCESS] = "SUCCESS"
	StatusNames[KEY_ENOENT] = "KEY_ENOENT"
	StatusNames[KEY_EEXISTS] = "KEY_EEXISTS"
	StatusNames[E2BIG] = "E2BIG"
	StatusNames[EINVAL] = "EINVAL"
	StatusNames[NOT_STORED] = "NOT_STORED"
	StatusNames[DELTA_BADVAL] = "DELTA_BADVAL"
	StatusNames[NOT_MY_VBUCKET] = "NOT_MY_VBUCKET"
	StatusNames[UNKNOWN_COMMAND] = "UNKNOWN_COMMAND"
	StatusNames[ENOMEM] = "ENOMEM"
	StatusNames[TMPFAIL] = "TMPFAIL"

}

// String an op code.
func (o CommandCode) String() (rv string) {
	rv = CommandNames[o]
	if rv == "" {
		rv = fmt.Sprintf("0x%02x", int(o))
	}
	return rv
}

// String an op code.
func (s Status) String() (rv string) {
	rv = StatusNames[s]
	if rv == "" {
		rv = fmt.Sprintf("0x%02x", int(s))
	}
	return rv
}

// Return true if a command is a "quiet" command.
func (o CommandCode) IsQuiet() bool {
	switch o {
	case GETQ,
		GETKQ,
		SETQ,
		ADDQ,
		REPLACEQ,
		DELETEQ,
		INCREMENTQ,
		DECREMENTQ,
		QUITQ,
		FLUSHQ,
		APPENDQ,
		PREPENDQ,
		RSETQ,
		RAPPENDQ,
		RPREPENDQ,
		RDELETEQ,
		RINCRQ,
		RDECRQ:
		return true
	}
	return false
}
