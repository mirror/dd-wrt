/*
 * Copyright (C) 1996-2024 The Squid Software Foundation and contributors
 *
 * Squid software is distributed under GPLv2+ license and includes
 * contributions from numerous individuals and organizations.
 * Please see the COPYING and CONTRIBUTORS files for details.
 */

#include "squid.h"
#include "errorpage.h"

#define STUB_API "errorpage.cc"
#include "tests/STUB.h"

err_type errorReservePageId(const char *, const SBuf &) STUB_RETVAL(err_type(0))
void errorAppendEntry(StoreEntry *, ErrorState * ) STUB
bool strHdrAcptLangGetItem(const String &, char *, int, size_t &) STUB_RETVAL(false)
void TemplateFile::loadDefault() STUB
TemplateFile::TemplateFile(char const*, err_type) STUB
bool TemplateFile::loadFor(const HttpRequest *) STUB_RETVAL(false)

std::ostream &operator <<(std::ostream &os, const ErrorState *) STUB_RETVAL(os)

