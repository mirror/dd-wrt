/*
 * Copyright (C) 1996-2024 The Squid Software Foundation and contributors
 *
 * Squid software is distributed under GPLv2+ license and includes
 * contributions from numerous individuals and organizations.
 * Please see the COPYING and CONTRIBUTORS files for details.
 */

#ifndef SQUID_SRC_LOADABLEMODULE_H
#define SQUID_SRC_LOADABLEMODULE_H

#include "SquidString.h"

// wrapper for dlopen(3), libltdl, and friends
class LoadableModule
{
public:
    LoadableModule(const String &aName);
    ~LoadableModule();           // unloads if loaded

    bool loaded() const;
    const String &name() const { return theName; }
    const String &error() const { return theError; }

    void load(); // throws Texc
    void unload(); // throws Texc

protected:
    String theName;
    String theError;
    void *theHandle;

private:
    void *openModule();
    bool closeModule();
    const char *errorMsg();
};

#endif /* SQUID_SRC_LOADABLEMODULE_H */

