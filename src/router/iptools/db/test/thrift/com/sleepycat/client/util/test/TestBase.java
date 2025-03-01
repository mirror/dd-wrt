/*-
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 2002, 2017 Oracle and/or its affiliates.  All rights reserved.
 *
 */

package com.sleepycat.client.util.test;

import java.io.File;
import java.io.IOException;
import java.nio.file.Path;
import java.nio.file.Paths;

import com.sleepycat.client.ClientTestBase;
import com.sleepycat.client.SDatabaseException;
import com.sleepycat.client.SEnvironment;
import com.sleepycat.client.SEnvironmentConfig;
import org.junit.After;
import org.junit.Before;
import org.junit.Rule;
import org.junit.rules.TestRule;
import org.junit.rules.TestWatcher;
import org.junit.runner.Description;

/**
 * The base class for all JE unit tests. 
 */
public abstract class TestBase extends ClientTestBase {

    private static final boolean copySucceeded =
        Boolean.getBoolean("test.copySucceeded");

    /*
     * Need to provide a customized name suffix for those tests which are 
     * Parameterized.
     *
     * This is because we need to provide a unique directory name for those 
     * failed tests. Parameterized class would reuse test cases, so class name 
     * plus the test method is not unique. User should set the customName
     * in the constructor of a Parameterized test. 
     */
    protected String customName;
    
    /**
     * The rule we use to control every test case, the core of this rule is 
     * copy the testing environment, files, sub directories to another place
     * for future investigation, if any of test failed. But we do have a limit
     * to control how many times we copy because of disk space. So once the 
     * failure counter exceed limit, it won't copy the environment any more.
     */
    @Rule
    public TestRule watchman = new TestWatcher() {

        /* Copy Environments when the test failed. */
        @Override
        protected void failed(Throwable t, Description desc) {
            doCopy(desc);
        }
        
        @Override
        protected void succeeded(Description desc){
            if (copySucceeded) {
                doCopy(desc);
            }
        }

        private void doCopy(Description desc) {
            String dirName = makeFileName(desc);
            try {
                copyEnvironments(dirName);
            } catch (Exception e) {
                throw new RuntimeException
                    ("can't copy env dir to " + dirName  + " after failure", e);
            }
        }
    };
    
    @Before
    public void setUp() 
        throws Exception {
        
        SharedTestUtils.cleanUpTestDir(SharedTestUtils.getTestDir());

        super.setUp();
    }
    
    @After
    public void tearDown() throws Exception {
        super.tearDown();
    }

    @Override
    protected Path createTestRoot() throws Exception {
        return Paths.get(".");
    }

    @Override
    protected boolean cleanTestRootAfterTests() {
        return false;
    }

    protected SEnvironment create(File envHome, SEnvironmentConfig envConfig)
            throws SDatabaseException {

        SEnvironment env = null;
        try {
            env = connection.openEnvironment(envHome.getPath(), envConfig);
        } catch (IOException e) {
            throw new RuntimeException(e);
        }
        return env;
    }

    /**
     *  Copy the testing directory to other place. 
     */
    private void copyEnvironments(String path) throws Exception{
        
        File failureDir = SharedTestUtils.getFailureCopyDir();
        if (failureDir.list().length < SharedTestUtils.getCopyLimit()) {
            SharedTestUtils.copyDir(SharedTestUtils.getTestDir(),
                                    new File(failureDir, path));
        }
    }
    
    /**
     * Get failure copy directory name. 
     */
    private String makeFileName(Description desc) {
        String name = desc.getClassName() + "-" + desc.getMethodName();
        if (customName != null) {
            name = name + "-" + customName;
        }
        return name;
    }
}
