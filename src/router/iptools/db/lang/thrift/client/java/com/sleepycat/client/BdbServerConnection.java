/*-
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 2002, 2017 Oracle and/or its affiliates.  All rights reserved.
 *
 * $Id$
 */

package com.sleepycat.client;

import com.sleepycat.thrift.BdbService;
import com.sleepycat.thrift.TEnvironment;
import com.sleepycat.thrift.TProtocolVersionTestResult;
import com.sleepycat.thrift.dbConstants;
import org.apache.thrift.protocol.TCompactProtocol;
import org.apache.thrift.protocol.TProtocol;
import org.apache.thrift.transport.TFramedTransport;
import org.apache.thrift.transport.TIOStreamTransport;
import org.apache.thrift.transport.TSSLTransportFactory;
import org.apache.thrift.transport.TSocket;
import org.apache.thrift.transport.TTransport;
import org.apache.thrift.transport.TTransportException;

import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.nio.ByteOrder;
import java.util.Objects;

/**
 * A connection with a specific BDB server.
 * <p>
 * BdbServerConnection is not thread-safe. Objects created from a connection
 * use the connection internally, therefore they are not thread-safe either. A
 * connection together with all objects created from it should only be accessed
 * from a single thread. For example, a connection, an environment handle
 * opened from it and a database handle opened from the environment handle, all
 * three objects should be accessed from the same thread. It is an error to
 * access the environment handle and the database handle from different
 * threads.
 */
public class BdbServerConnection implements RemoteCallHelper, AutoCloseable {
    /** The server's host URL. */
    private final String host;

    /** The server's listening port. */
    private final int port;

    /** The server's native byte order. */
    private ByteOrder serverByteOrder;

    /** The client instance. */
    private BdbService.Client client;

    private BdbServerConnection(String host, int port) {
        this.host = Objects.requireNonNull(host, "host is null.");
        this.port = Objects.requireNonNull(port, "port is null.");
    }

    /**
     * For unit test only.
     *
     * @param clientIn the input stream
     * @param clientOut the out stream
     */
    BdbServerConnection(InputStream clientIn, OutputStream clientOut) {
        this.host = "test";
        this.port = 0;
        TProtocol protocol = new TCompactProtocol(
                new TIOStreamTransport(clientIn, clientOut));
        this.client = new BdbService.Client(protocol);
        this.serverByteOrder = ByteOrder.nativeOrder();
    }

    /**
     * Open a new connection to the specified BDB server.
     *
     * @param host the server's host URL
     * @param port the server's listening port
     * @return a new connection
     * @throws BdbConnectionException if an error occurred while connecting to
     * the server
     * @throws SDatabaseException if any error occurs
     */
    public static BdbServerConnection connect(String host, int port)
            throws SDatabaseException {
        return connectSsl(host, port, null, 0);
    }

    /**
     * Open a new SSL connection to the specified BDB server.
     *
     * @param host the server's host URL
     * @param port the server's listening port
     * @param sslConfig the SSL configuration
     * @param timeout the timeout in milliseconds for each remote request, zero
     * as an infinite timeout
     * @return a new connection
     * @throws BdbConnectionException if an error occurred while connecting to
     * the server
     * @throws SDatabaseException if any error occurs
     */
    public static BdbServerConnection connectSsl(String host, int port,
            SslConfig sslConfig, int timeout) throws SDatabaseException {
        try {
            BdbServerConnection connection =
                    new BdbServerConnection(host, port);
            connection.open(sslConfig, timeout);
            return connection;
        } catch (TTransportException e) {
            String msg = "Failed to connect to '" + host + ":" + port + "'";
            throw new BdbConnectionException(msg, e);
        }
    }

    private void open(SslConfig sslConfig, int timeout)
            throws TTransportException {
        TTransport transport;
        if (sslConfig == null) {
            TSocket socket = new TSocket(this.host, this.port);
            transport = new TFramedTransport(socket);
            transport.open();
        } else {
            transport = TSSLTransportFactory.getClientSocket(this.host,
                    this.port, timeout, sslConfig.getParameters());
        }

        TProtocol protocol = new TCompactProtocol(transport);
        this.client = new BdbService.Client(protocol);

        TProtocolVersionTestResult testResult =
                remoteCall(() -> this.client.isProtocolVersionSupported(
                        dbConstants.PROTOCOL_VERSION));
        if (!testResult.isSupported()) {
            throw new BdbConnectionException(
                    "Driver is not compatible with the server. The " +
                            "server requires drivers with protocol version: " +
                            testResult.serverProtocolVersion, null);
        }
        this.serverByteOrder = testResult.isServerBigEndian() ?
                ByteOrder.BIG_ENDIAN : ByteOrder.LITTLE_ENDIAN;
    }

    /**
     * Return a {@link BdbServerAdmin} which can be used to perform
     * administrative operations on the connected server.
     *
     * @return a BdbServerAdmin
     */
    public BdbServerAdmin adminService() {
        return new BdbServerAdmin(this.client);
    }

    /**
     * Return the native byte order of the connected server.
     *
     * @return the native byte order of the connected server
     */
    public ByteOrder getServerByteOrder() {
        return this.serverByteOrder;
    }

    /**
     * Open a remote environment and return an environment handle.
     *
     * @param home the environment's home directory. The directory must
     * specified as a relative path
     * @param config the environment attributes. If null, system default
     * attributes are used
     * @return an environment handle
     * @throws IOException if the environment does not exist or cannot be
     * accessed
     * @throws SResourceInUseException if the environment is being recovered or
     * deleted
     * @throws SDatabaseException if any error occurs
     */
    public SEnvironment openEnvironment(String home, SEnvironmentConfig config)
            throws IOException, SDatabaseException {
        return remoteCallWithIOException(() -> {
            TEnvironment env = this.client
                    .openEnvironment(home, ThriftWrapper.nullSafeGet(config));
            return new SEnvironment(env, home, this, this.client);
        });
    }

    /**
     * Close this connection.
     * <p>
     * After closing this connection, all objects created from this connection
     * must be discarded.
     */
    public void close() {
        this.client.getInputProtocol().getTransport().close();
    }
}
