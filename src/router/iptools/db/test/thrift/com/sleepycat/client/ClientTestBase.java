/*-
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 2002, 2017 Oracle and/or its affiliates.  All rights reserved.
 *
 * $Id$
 */

package com.sleepycat.client;

import com.sleepycat.server.BdbServiceHandler;
import com.sleepycat.server.config.BdbServiceConfig;
import com.sleepycat.server.util.FileUtils;
import com.sleepycat.thrift.BdbService;
import org.apache.log4j.BasicConfigurator;
import org.apache.thrift.protocol.TCompactProtocol;
import org.apache.thrift.server.TServer;
import org.apache.thrift.server.TSimpleServer;
import org.junit.After;
import org.junit.Before;
import org.junit.Rule;
import org.junit.rules.ExpectedException;

import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.io.PipedInputStream;
import java.io.PipedOutputStream;
import java.nio.file.Files;
import java.nio.file.Path;
import java.util.Arrays;
import java.util.Properties;
import java.util.concurrent.Executors;
import java.util.concurrent.TimeUnit;

import static org.hamcrest.CoreMatchers.is;
import static org.hamcrest.MatcherAssert.assertThat;
import static org.junit.Assert.fail;

public class ClientTestBase {

    @Rule
    public ExpectedException thrown = ExpectedException.none();

    protected Path testRoot;

    protected BdbServiceHandler handler;

    protected BdbServerConnection connection;

    @Before
    public void setUp() throws Exception {
        testRoot = createTestRoot();
        handler = new BdbServiceHandler();

        PipedInputStream serverIn = new PipedInputStream();
        PipedInputStream clientIn = new PipedInputStream();
        PipedOutputStream serverOut = new PipedOutputStream(clientIn);
        PipedOutputStream clientOut = new PipedOutputStream(serverIn);

        connection = new BdbServerConnection(clientIn, clientOut);
        TServer server = createServer(handler, serverIn, serverOut);

        handler.setServerAndConfig(server, createConfig());

        new Thread(server::serve).start();

        Thread testThread = Thread.currentThread();
        Executors.newScheduledThreadPool(1).schedule(() -> {
            testThread.interrupt();
            connection.close();
        }, 10000, TimeUnit.SECONDS);
    }

    @After
    public void tearDown() throws Exception {
        handler.shutdown();
        if (cleanTestRootAfterTests()) {
            FileUtils.deleteFileTree(testRoot.toFile());
        }
    }

    protected Path createTestRoot() throws Exception {
        return Files.createTempDirectory("BdbClientTest");
    }

    protected boolean cleanTestRootAfterTests() {
        return true;
    }

    private TServer createServer(BdbServiceHandler handler,
            InputStream serverIn, OutputStream serverOut) {
        TServer.Args args = new TServer.Args(
                new IOStreamServerTransport(serverIn, serverOut));
        args.processor(new BdbService.Processor<>(handler));
        args.protocolFactory(new TCompactProtocol.Factory());

        return new TSimpleServer(args);
    }

    private BdbServiceConfig createConfig() throws IOException {
        Properties properties = new Properties();
        properties.setProperty(BdbServiceConfig.ROOT_HOME,
                testRoot.toAbsolutePath().toString());

        BdbServiceConfig config = new BdbServiceConfig(properties);
        config.initRootDirs();

        return config;
    }

    protected void assertClosed(AutoCloseable handle) throws Exception {
        thrown.expect(IllegalArgumentException.class);
        thrown.expectMessage("The handle is closed or expired.");
        handle.close();
    }

    protected SDatabaseEntry entry(String value) {
        return new SDatabaseEntry(value.getBytes());
    }

    protected void assertCursorGet(CursorGetFunc func,
            SDatabaseEntry key, SDatabaseEntry data,
            String expectedKey, String expectedData) throws Exception {
        assertThat(func.apply(key, data, null), is(SOperationStatus.SUCCESS));
        assertThat(new String(key.getData()), is(expectedKey));
        assertThat(new String(data.getData()), is(expectedData));
    }

    @FunctionalInterface
    protected interface CursorGetFunc {
        SOperationStatus apply(SDatabaseEntry key, SDatabaseEntry data,
                SLockMode mode);
    }

    protected void assertDbData(SDatabase db, String[][] expectedData) {
        Arrays.stream(expectedData).forEach(keyData -> {
            try {
                assertDbGet(db::getSearchBoth,
                        entry(keyData[0]), entry(keyData[1]),
                        keyData[0], keyData[1]);
            } catch (Exception e) {
                throw new RuntimeException(e);
            }
        });
    }

    protected void assertDbGet(DbGetFunc func,
            SDatabaseEntry key, SDatabaseEntry data,
            String expectedKey, String expectedData) throws Exception {
        assertThat(func.apply(null, key, data, null),
                is(SOperationStatus.SUCCESS));
        assertThat(new String(key.getData()), is(expectedKey));
        assertThat(new String(data.getData()), is(expectedData));
    }

    @FunctionalInterface
    protected interface DbGetFunc {
        SOperationStatus apply(STransaction txn, SDatabaseEntry key,
                SDatabaseEntry data, SLockMode mode);
    }


}