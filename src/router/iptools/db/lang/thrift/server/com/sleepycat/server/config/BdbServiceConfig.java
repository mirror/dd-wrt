/*-
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 2002, 2017 Oracle and/or its affiliates.  All rights reserved.
 *
 * $Id$
 */

package com.sleepycat.server.config;

import java.io.File;
import java.io.IOException;
import java.nio.file.Files;
import java.util.EnumMap;
import java.util.Map;
import java.util.Properties;

/**
 * Configurations for BdbService.
 */
public class BdbServiceConfig {
    /* Property keys. */
    public static final String ROOT_LOG = "root.log";
    public static final String ROOT_BLOB = "root.blob";
    public static final String ROOT_DATA = "root.data";
    public static final String ROOT_HOME = "root.home";

    public static final String HANDLE_TIMEOUT = "handle.timeout";
    public static final String CLEANUP_INTERVAL = "cleanup.interval";

    /** The configuration properties. */
    private PropertyHelper properties;

    /** The root directories. */
    private Map<EnvDirType, File> rootDirs = new EnumMap<>(EnvDirType.class);

    /**
     * Create a BdbServiceConfig.
     *
     * @param properties the service configuration properties
     */
    public BdbServiceConfig(Properties properties) {
        this.properties = new PropertyHelper(properties);
    }

    /**
     * Initialize root directories for various environment and database files.
     *
     * @throws IOException if failed to create any root directory
     */
    public void initRootDirs() throws IOException {
        initDir(ROOT_HOME, ".", EnvDirType.HOME);
        initDir(ROOT_DATA, null, EnvDirType.DATA);
        initDir(ROOT_BLOB, null, EnvDirType.BLOB);
        initDir(ROOT_LOG, null, EnvDirType.LOG);
    }

    private void initDir(String key, String defaultDir, EnvDirType type)
            throws IOException {
        String dirPath = this.properties.getString(key, defaultDir);
        if (dirPath != null) {
            File dir = new File(dirPath);
            Files.createDirectories(dir.toPath());
            this.rootDirs.put(type, dir);
        }
    }

    /**
     * Get all types of root directories.
     *
     * @return all root directories
     */
    public Map<EnvDirType, File> getRootDirs() {
        return this.rootDirs;
    }

    /**
     * Return the timeout value for open handles, in seconds.
     *
     * @return the handle timeout value, in seconds
     */
    public int getHandleTimeoutInSeconds() {
        return this.properties
                .getInt(HANDLE_TIMEOUT, "600", 0, Integer.MAX_VALUE);
    }

    /**
     * Return the interval between two runs of handle cleanup workers, in
     * seconds.
     *
     * @return the cleanup interval, in seconds
     */
    public int getCleanupIntervalInSeconds() {
        return this.properties
                .getInt(CLEANUP_INTERVAL, "300", 1, Integer.MAX_VALUE);
    }
}
