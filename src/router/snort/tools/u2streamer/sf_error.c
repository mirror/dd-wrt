

#include "sf_error.h"
#include <stdio.h>

static const char * const SF_errstrings[] =
{
    "SUCCESS",                      // 0
    "Invalid Argument",
    "Unsupported/Unimplemented",
    "Out of memory",
    "Out of range",
    "Not allowed",
    "No entry",
    "Already exists",
    "Unhandled database error",
    "Syntax error",
    "required User value missing",  // 10
    "required User Role value missing",
    "required TimeSpent value missing",
    "required Comment value missing",
    "Database corrupt due to lack of transactions",
    "required Type value missing",
    "required State value missing",
    "required Summary value missing",
    "Resource busy",
    "No space",
    "General read error",           // 20
    "End of file",
    "Try Again",
    "Partial Read",
    "Not connected",
    "Read Truncated",
    "Closed",
    "Protocol Unsupported",
    "Not supported",
    "Write Error",
    "Partial Write",                // 30
    "Bad Length",
    "Protocol Violation",
    "Peer Error",
    "Not a directory",
    "Mutex error",
    "Invalid mutex",
    "Mutex deadlock avoided",
    "Open failed",
    "Resource locked",
    "SSL Error",                    // 40
    "Invalid license",
    "Invalid license for platform",
    "Corrupt license",
    "No valid ciphers",
    "CRL expired",
    "Does not match",
    "Socket error",
    "Nitro database error",
    "License unavailable/does not have a license",
    "Already has a license",        // 50
    "Corrupt file",
    "Bad magic",
    "Bad linktype",
    "Continue",
    "Invalid Hostname",
    "Couldn't create user - license limit reached", // 56 - SF_EUSER_LIMIT_REACHED
    "Error in deleting file or entry in memory",  // 57 - SF_EDELETE
    "Error manipulating memory"  // 58 - SF_EMEM
};

#define SF_MAX_ERRNUM   (sizeof(SF_errstrings)/sizeof(SF_errstrings[0]))

const char *sf_strerror(int errnum)
{
    if(errnum == -1)
        return "General error";

    if(errnum >= (int)SF_MAX_ERRNUM || errnum < 0)
        return "Unknown Error";

    return SF_errstrings[errnum];
}

