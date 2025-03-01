/*-
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 2002, 2017 Oracle and/or its affiliates.  All rights reserved.
 *
 * $Id$
 */

package com.sleepycat.client;

/**
 * Specifies the attributes of an application invoked checkpoint operation.
 */
public class SCheckpointConfig {

    private boolean force;

    private int kBytes;

    private int minutes;

    /**
     * Create a default SCheckpointConfig.
     */
    public SCheckpointConfig() {
        this(false, 0, 0);
    }

    /**
     * Create a SCheckpointConfig with specified attributes.
     *
     * @param force if a checkpoint is always performed
     * @param kBytes the checkpoint log data threshold, in kilobytes
     * @param minutes the checkpint time threshold, in minutes
     */
    public SCheckpointConfig(boolean force, int kBytes, int minutes) {
        this.force = force;
        this.kBytes = kBytes;
        this.minutes = minutes;
    }

    /**
     * Return the configuration of the checkpoint force option.
     *
     * @return the configuration of the checkpoint force option.
     */
    public boolean getForce() {
        return force;
    }

    /**
     * Configure the checkpoint force option.
     *
     * @param force if set to true, force a checkpoint, even if there has been
     * no activity since the last checkpoint.
     * @return this
     */
    public SCheckpointConfig setForce(boolean force) {
        this.force = force;
        return this;
    }

    /**
     * Return the checkpoint log data threshold, in kilobytes.
     *
     * @return the checkpoint log data threshold, in kilobytes
     */
    public int getKBytes() {
        return kBytes;
    }

    /**
     * Configure the checkpoint log data threshold, in kilobytes.
     *
     * @param kBytes if the {@code kbytes} parameter is non-zero, a checkpoint
     * will be performed if more than {@code kbytes} of log data have been
     * written since the last checkpoint.
     * @return this
     */
    public SCheckpointConfig setKBytes(int kBytes) {
        this.kBytes = kBytes;
        return this;
    }

    /**
     * Return the checkpoint time threshold, in minutes.
     *
     * @return the checkpoint time threshold, in minutes.
     */
    public int getMinutes() {
        return minutes;
    }

    /**
     * Configure the checkpoint time threshold, in minutes.
     *
     * @param minutes if the {@code minutes} parameter is non-zero, a
     * checkpoint is performed if more than min minutes have passed since the
     * last checkpoint.
     * @return this
     */
    public SCheckpointConfig setMinutes(int minutes) {
        this.minutes = minutes;
        return this;
    }
}
