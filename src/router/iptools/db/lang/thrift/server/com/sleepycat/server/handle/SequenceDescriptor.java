/*-
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 2002, 2017 Oracle and/or its affiliates.  All rights reserved.
 *
 * $Id$
 */

package com.sleepycat.server.handle;

import com.sleepycat.db.DatabaseException;
import com.sleepycat.db.Sequence;

/**
 * A SequenceDescriptor is a HandleDescriptor for a Sequence.
 */
public class SequenceDescriptor extends HandleDescriptor<Sequence> {
    /**
     * Create a SequenceDescriptor.
     * @param seq the BDB sequence handle
     * @param key the resource key
     * @param db the parent database
     */
    public SequenceDescriptor(Sequence seq, SequenceKey key,
            DatabaseDescriptor db) {
        super(seq, key, db);
    }

    @Override
    protected void closeBdbHandle() throws DatabaseException {
        getHandle().close();
    }
}
