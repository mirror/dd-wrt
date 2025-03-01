/*-
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 2002, 2017 Oracle and/or its affiliates.  All rights reserved.
 *
 * $Id$
 */

package com.sleepycat.server.handle;

import com.sleepycat.db.DatabaseException;
import com.sleepycat.db.JoinCursor;

import java.util.LinkedList;
import java.util.List;

/**
 * A JoinCursorDescriptor is a HandleDescriptor for a JoinCursor.
 */
public class JoinCursorDescriptor extends HandleDescriptor<JoinCursor> {

    /**
     * Create a JoinCursorDescriptor.
     *
     * @param cursor the join cursor
     * @param primaryDb the primary database descriptor
     * @param secCursors the list of secondary cursors
     */
    public JoinCursorDescriptor(JoinCursor cursor, DatabaseDescriptor primaryDb,
            List<CursorDescriptor> secCursors) {
        super(cursor, null, toArray(primaryDb, secCursors));
    }

    private static HandleDescriptor[] toArray(DatabaseDescriptor pdb,
            List<CursorDescriptor> secCursors) {
        List<HandleDescriptor> ret = new LinkedList<>();
        ret.add(pdb);
        ret.addAll(secCursors);
        return ret.toArray(new HandleDescriptor[ret.size()]);
    }

    @Override
    public ResourceKey[] resourceOwners() {
        return new ResourceKey[0];
    }

    @Override
    protected void closeBdbHandle() throws DatabaseException {
        getHandle().close();
    }
}
