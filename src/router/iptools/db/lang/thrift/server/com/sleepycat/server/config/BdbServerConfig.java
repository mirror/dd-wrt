/*-
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 2002, 2017 Oracle and/or its affiliates.  All rights reserved.
 *
 * $Id$
 */

package com.sleepycat.server.config;

import com.sleepycat.server.util.FileUtils;
import org.apache.thrift.TProcessorFactory;
import org.apache.thrift.protocol.TCompactProtocol;
import org.apache.thrift.server.THsHaServer;
import org.apache.thrift.server.TServer;
import org.apache.thrift.server.TThreadPoolServer;
import org.apache.thrift.transport.TNonblockingServerSocket;
import org.apache.thrift.transport.TSSLTransportFactory;
import org.apache.thrift.transport.TServerSocket;
import org.apache.thrift.transport.TTransportException;

import java.io.IOException;
import java.net.InetAddress;
import java.net.UnknownHostException;
import java.nio.file.attribute.PosixFilePermission;
import java.security.AccessControlException;
import java.util.Properties;

/**
 * Configurations for BdbServer.
 */
public class BdbServerConfig {
    /* Property keys. */
    private static final String PORT = "port";
    private static final String WORKERS = "workers";

    private static final String SSL_HOST = "ssl.host";

    private static final String KEY_STORE = "ssl.keyStore";
    private static final String KEY_STORE_PWD = "ssl.keyStore.password";
    private static final String KEY_STORE_TYPE = "ssl.keyStore.type";
    private static final String KEY_STORE_MANAGER = "ssl.keyStore.manager";

    private static final String TRUST_STORE = "ssl.trustStore";
    private static final String TRUST_STORE_PWD = "ssl.trustStore.password";
    private static final String TRUST_STORE_TYPE = "ssl.trustStore.type";
    private static final String TRUST_STORE_MANAGER = "ssl.trustStore.manager";

    /** The configuration properties. */
    private PropertyHelper properties;

    /**
     * Create a BdbServerConfig.
     *
     * @param properties the server configuration properties
     */
    public BdbServerConfig(Properties properties) {
        this.properties = new PropertyHelper(properties);
    }

    /**
     * Create a TServer from this configuration.
     *
     * @param processor the processor factory used by the new server
     * @return a TServer
     */
    public TServer createServer(TProcessorFactory processor)
            throws TTransportException, UnknownHostException {
        if (isSslEnabled()) {
            return createSslServer(processor);
        } else {
            return createHsHaServer(processor);
        }
    }

    private TServer createSslServer(TProcessorFactory processor)
            throws TTransportException, UnknownHostException {
        TSSLTransportFactory.TSSLTransportParameters parameters =
                new TSSLTransportFactory.TSSLTransportParameters();

        if (!this.properties.getString(KEY_STORE, "").isEmpty()) {
            configureKeyStore(parameters);
        }
        if (!this.properties.getString(TRUST_STORE, "").isEmpty()) {
            configureTrustStore(parameters);
        }

        TServerSocket socket = TSSLTransportFactory
                .getServerSocket(getPort(), 0, InetAddress.getByName(
                        this.properties.getString(SSL_HOST, null)), parameters);
        TThreadPoolServer.Args args = new TThreadPoolServer.Args(socket);
        args.maxWorkerThreads(getWorkers()).minWorkerThreads(getWorkers());
        args.protocolFactory(new TCompactProtocol.Factory());
        args.processorFactory(processor);

        return new TThreadPoolServer(args);
    }

    private void configureKeyStore(
            TSSLTransportFactory.TSSLTransportParameters parameters) {
        String keyStore = this.properties.getString(KEY_STORE, "");
        String keyPass = this.properties.getString(KEY_STORE_PWD, "");
        String keyStoreType = this.properties.getString(KEY_STORE_TYPE, null);
        String keyManagerType =
                this.properties.getString(KEY_STORE_MANAGER, null);
        checkKeyStorePermission(keyStore);
        parameters.setKeyStore(keyStore, keyPass, keyManagerType, keyStoreType);
    }

    private void configureTrustStore(
            TSSLTransportFactory.TSSLTransportParameters parameters) {
        String trustStore = this.properties.getString(TRUST_STORE, "");
        String trustPass = this.properties.getString(TRUST_STORE_PWD, "");
        String trustStoreType =
                this.properties.getString(TRUST_STORE_TYPE, null);
        String trustManagerType =
                this.properties.getString(TRUST_STORE_MANAGER, null);
        checkKeyStorePermission(trustStore);
        parameters.setTrustStore(trustStore, trustPass, trustManagerType,
                trustStoreType);
    }

    private void checkKeyStorePermission(String keyStoreFile)
            throws AccessControlException {
        try {
            if (FileUtils.hasAnyPermission(keyStoreFile,
                    PosixFilePermission.OTHERS_READ,
                    PosixFilePermission.OTHERS_WRITE,
                    PosixFilePermission.OTHERS_EXECUTE)) {
                throw new AccessControlException(
                        "The key store file '" + keyStoreFile +
                                "' must not be globally accessible.");
            }
        } catch (IOException e) {
            // Ignored. The error will be caught later when starting the server.
        }
    }

    private TServer createHsHaServer(TProcessorFactory processor)
            throws TTransportException {
        TNonblockingServerSocket socket =
                new TNonblockingServerSocket(getPort());

        THsHaServer.Args args = new THsHaServer.Args(socket);
        args.workerThreads(getWorkers()).processorFactory(processor);
        args.protocolFactory(new TCompactProtocol.Factory());

        return new THsHaServer(args);
    }

    private boolean isSslEnabled() {
        return !this.properties.getString(SSL_HOST, "").isEmpty();
    }

    public int getPort() {
        return this.properties.getInt(PORT, "8080", 0, 65535);
    }

    private int getWorkers() {
        return this.properties.getInt(WORKERS, "20", 1, Integer.MAX_VALUE);
    }
}
