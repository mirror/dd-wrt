# This file lists HTTP headers whose field-value is a comma-separated
# list of values.
#
# As per RFC 2616, these headers can appear multiple times in a message.
# Such multiple instances can be combined into a single header by appending
# each subsequent field-value to the first, each separated by a comma.
#
# This file is formatted so that it can be included in the main pound
# configuration file in order to enable combining of all these headers.
#
CombineHeaders
	# Standard headers
	"Accept"
	"Accept-Charset"
	"Accept-Encoding"
	"Accept-Language"
	"Accept-Ranges"
	"Allow"
	"Cache-Control"
	"Connection"
	"Content-Encoding"
	"Content-Language"
	"Cookie"
	"Forwarded"
	"Expect"
	"If-Match"
	"If-None-Match"
	"Pragma"
	"Proxy-Authenticate"
	"Range"
	"TE"
	"Trailer"
	"Transfer-Encoding"
	"Upgrade"
	"Vary"
	"Via"
	"Warning"
	"WWW-Authenticate"

	# De-facto standard
	"X-Forwarded-For"
End
