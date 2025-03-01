/*
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 2002, 2017 Oracle and/or its affiliates.  All rights reserved.
 *
 * $Id$
 */

package com.sleepycat.client.persist;

import com.sleepycat.client.SDatabaseException;

/**
 * Thrown by the {@link EntityStore} constructor when the {@link
 * StoreConfig#setExclusiveCreate ExclusiveCreate} configuration parameter is
 * true and the store's internal catalog database already exists.
 *
 * @author Mark Hayes
 */
public class StoreExistsException extends SDatabaseException {

    private static final long serialVersionUID = 1;

    public StoreExistsException(String message) {
        super(message);
    }
}
