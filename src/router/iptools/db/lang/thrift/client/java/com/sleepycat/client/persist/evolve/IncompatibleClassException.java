/*-
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 2002, 2017 Oracle and/or its affiliates.  All rights reserved.
 *
 * $Id$
 */

package com.sleepycat.client.persist.evolve;

/**
 * A class has been changed incompatibly and no mutation has been configured to
 * handle the change or a new class version number has not been assigned.
 *
 * @see com.sleepycat.client.persist.EntityStore#EntityStore EntityStore.EntityStore
 * @see com.sleepycat.client.persist.model.Entity#version
 * @see com.sleepycat.client.persist.model.Persistent#version
 *
 * @see com.sleepycat.client.persist.evolve Class Evolution
 * @author Mark Hayes
 */
public class IncompatibleClassException extends RuntimeException {

    private static final long serialVersionUID = 2103957824L;

    public IncompatibleClassException(String message) {
        super(message);
    }
}
