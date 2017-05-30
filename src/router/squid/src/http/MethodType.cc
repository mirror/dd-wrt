/*
 * Auto-Generated File. Changes will be destroyed.
 */
#include "squid.h"
#include "sbuf/SBuf.h"
#include "http/MethodType.h"
namespace Http
{

const SBuf MethodType_sb[] = {
	SBuf("NONE"),
	SBuf("GET"),
	SBuf("POST"),
	SBuf("PUT"),
	SBuf("HEAD"),
	SBuf("CONNECT"),
	SBuf("TRACE"),
	SBuf("OPTIONS"),
	SBuf("DELETE"),
#if NO_SPECIAL_HANDLING
	SBuf("LINK"),
	SBuf("UNLINK"),
#endif
	SBuf("CHECKOUT"),
	SBuf("CHECKIN"),
	SBuf("UNCHECKOUT"),
	SBuf("MKWORKSPACE"),
	SBuf("VERSION-CONTROL"),
	SBuf("REPORT"),
	SBuf("UPDATE"),
	SBuf("LABEL"),
	SBuf("MERGE"),
	SBuf("BASELINE-CONTROL"),
	SBuf("MKACTIVITY"),
#if NO_SPECIAL_HANDLING
	SBuf("ORDERPATCH"),
	SBuf("ACL"),
	SBuf("MKREDIRECTREF"),
	SBuf("UPDATEREDIRECTREF"),
	SBuf("MKCALENDAR"),
#endif
	SBuf("PROPFIND"),
	SBuf("PROPPATCH"),
	SBuf("MKCOL"),
	SBuf("COPY"),
	SBuf("MOVE"),
	SBuf("LOCK"),
	SBuf("UNLOCK"),
	SBuf("SEARCH"),
#if NO_SPECIAL_HANDLING
	SBuf("PATCH"),
	SBuf("BIND"),
	SBuf("REBIND"),
	SBuf("UNBIND"),
#endif
	SBuf("PRI"),
	SBuf("PURGE"),
	SBuf("OTHER"),
	SBuf("ENUM_END")
};
}; // namespace Http
