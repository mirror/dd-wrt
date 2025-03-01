/*-
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 2002, 2017 Oracle and/or its affiliates.  All rights reserved.
 *
 * $Id$
 */

package com.sleepycat.client.util;

import java.io.File;
import java.io.FileNotFoundException;

import com.sleepycat.client.util.test.TestBase;

import com.sleepycat.client.SDatabaseException;
import com.sleepycat.client.SEnvironment;
import com.sleepycat.client.SEnvironmentConfig;

public class DualTestCase extends TestBase {

    private boolean setUpInvoked = false;

    public DualTestCase() {
        super();
    }

    @Override
    public void setUp()
        throws Exception {

        setUpInvoked = true;
        super.setUp();
    }

    @Override
    public void tearDown()
        throws Exception {

        if (!setUpInvoked) {
            throw new IllegalStateException
                ("tearDown was invoked without a corresponding setUp() call");
        }
        super.tearDown();
    }

    protected void close(SEnvironment env)
        throws SDatabaseException {

        env.close();
    }

    public static boolean isReplicatedTest(Class<?> testCaseClass) {
        return false;
    }
}
