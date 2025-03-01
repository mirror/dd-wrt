/*-
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 2002, 2017 Oracle and/or its affiliates.  All rights reserved.
 *
 * $Id$
 */

package com.sleepycat.server.handle;

import java.io.File;
import java.io.IOException;

/**
 * A EnvironmentKey uniquely identifies an environment.
 */
public class EnvironmentKey extends FileKey {

    public EnvironmentKey(File envHome) throws IOException {
        super(envHome);
    }

}
