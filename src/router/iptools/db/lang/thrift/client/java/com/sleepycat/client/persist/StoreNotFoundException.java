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
 * StoreConfig#setAllowCreate AllowCreate} configuration parameter is false and
 * the store's internal catalog database does not exist.
 *
 * @author Mark Hayes
 */
public class StoreNotFoundException extends SDatabaseException {

    private static final long serialVersionUID = 1895430616L;

    public StoreNotFoundException(String message) {
        super(message);
    }
}
