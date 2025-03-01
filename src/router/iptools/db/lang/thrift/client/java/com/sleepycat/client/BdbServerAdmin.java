/*-
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 2002, 2017 Oracle and/or its affiliates.  All rights reserved.
 *
 * $Id$
 */

package com.sleepycat.client;

import com.sleepycat.thrift.BdbService;

import java.io.FileNotFoundException;
import java.io.IOException;
import java.util.Objects;

/**
 * BdbServerAdmin is used to perform administrative operations on a BDB server.
 */
public class BdbServerAdmin implements RemoteCallHelper {
    /** The remote service client. */
    private final BdbService.Client client;

    BdbServerAdmin(BdbService.Client client) {
        this.client = Objects.requireNonNull(client);
    }

    /**
     * Check if the server is reachable and responsive.
     *
     * @throws SDatabaseException if any error occurs
     */
    public void ping() throws SDatabaseException {
        remoteCall(() -> {
            this.client.ping();
            return null;
        });
    }

    /**
     * Return the BDB library release version hosted by the server.
     *
     * @return the BDB library release version
     * @throws SDatabaseException if any error occurs
     */
    public String getServerBdbVersion() throws SDatabaseException {
        return remoteCall(this.client::getBdbVersion);
    }

    /**
     * Request to shutdown the server.
     * <p>
     * After this method. All connections to the server and all objects created
     * from them must be discarded.
     *
     * @throws SDatabaseException if any error occurs
     */
    public void shutdownServer() throws SDatabaseException {
        remoteCall(() -> {
            this.client.shutdown();
            return null;
        });
    }

    /**
     * Close environment handles which are opened for the specified environment
     * and which have not been accessed for at least the specified idle time.
     * <p>
     * Before an environment handle is closed, all handles that depend on it
     * are close first. If a close operation fails, this method continues to
     * close the rest of handles and throw the first exception occurred.
     *
     * @param home the environment's home directory as a relative path
     * @param minIdleInMilli the idle time in milliseconds, environment handles
     * which have not been accessed for at least this time are closed
     * @throws SDatabaseException if any error occurs
     */
    public void closeEnvironmentHandles(String home, long minIdleInMilli)
            throws SDatabaseException {
        remoteCall(() -> {
            this.client.closeEnvironmentHandles(home, minIdleInMilli);
            return null;
        });
    }

    /**
     * Delete all files created for the environment at the specified home
     * directory, and all database files created under the environment.
     * <p>
     * If {@code force} is true, all handles opened for the environment and all
     * handles created from them are closed before files are deleted. If {@code
     * force} is false and there is at least one handle opened for the
     * environment, {@link SResourceInUseException} is thrown.
     *
     * @param home the environment's home directory as a relative path
     * @param force if true, open handles for the environments are closed so
     * that environment files can be deleted; otherwise, this operation fails
     * with a SResourceInUseException if there is at least one open handle for
     * the environment
     * @throws FileNotFoundException if no environment exists at the specified
     * home directory
     * @throws IOException if any I/O error occurs
     * @throws SResourceInUseException if {@code force} is false and there is
     * at least one open handle for the environment
     * @throws SDatabaseException if any error occurs
     */
    public void deleteEnvironmentAndDatabases(String home, boolean force) throws
            IOException, SDatabaseException {
        remoteCallWithIOException(() -> {
            this.client.deleteEnvironmentAndDatabases(home, force);
            return null;
        });
    }

    /**
     * Close database handles which are opened for the specified file and
     * database name under the specified environment and which have not been
     * accessed for at least the specified idle time. If {@code databaseName}
     * is not specified, handles for all databases in the file are closed if
     * they have not been accessed for at least the specified idle time.
     * <p>
     * Before a database handle is closed, all handles that depend on it
     * are close first. If a close operation fails, this method continues to
     * close the rest of handles and throw the first exception occurred.
     *
     * @param envHome the environment home directory as a relative path
     * @param fileName the database file name
     * @param databaseName the database name
     * @param minIdleInMilli the idle time in milliseconds, database handles
     * which have not been accessed for at least this time are closed
     * @throws SDatabaseException if any error occurs
     */
    public void closeDatabaseHandles(String envHome, String fileName,
            String databaseName, long minIdleInMilli)
            throws SDatabaseException {
        remoteCall(() -> {
            this.client.closeDatabaseHandles(envHome, fileName, databaseName,
                    minIdleInMilli);
            return null;
        });
    }

}
