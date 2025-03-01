/*-
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 2000, 2017 Oracle and/or its affiliates.  All rights reserved.
 *
 */

package com.sleepycat.client.collections;

import com.sleepycat.client.compat.DbCompat;
import com.sleepycat.client.SCursor;
import com.sleepycat.client.SCursorConfig;
import com.sleepycat.client.SDatabaseException;
import com.sleepycat.client.util.keyrange.KeyRange;
import com.sleepycat.client.util.keyrange.RangeCursor;

class MyRangeCursor extends RangeCursor {

    private DataView view;
    private boolean isRecnoOrQueue;
    private boolean writeCursor;

    MyRangeCursor(KeyRange range,
                  SCursorConfig config,
                  DataView view,
                  boolean writeAllowed)
        throws SDatabaseException {

        super(range, view.dupsRange, view.dupsOrdered,
              openCursor(view, config, writeAllowed));
        this.view = view;
        isRecnoOrQueue = view.recNumAllowed && !view.btreeRecNumDb;
        writeCursor = isWriteCursor(config, writeAllowed);
    }

    /**
     * Returns true if a write cursor is requested by the user via the cursor
     * config, or if this is a writable cursor and the user has not specified a
     * cursor config.  For CDB, a special cursor must be created for writing.
     * See CurrentTransaction.openCursor.
     */
    private static boolean isWriteCursor(SCursorConfig config,
                                         boolean writeAllowed) {
        return DbCompat.getWriteCursor(config) ||
               (config == SCursorConfig.DEFAULT && writeAllowed);
    }

    private static SCursor openCursor(DataView view,
                                     SCursorConfig config,
                                     boolean writeAllowed)
        throws SDatabaseException {

        return view.currentTxn.openCursor
            (view.db, config, isWriteCursor(config, writeAllowed),
             view.useTransaction());
    }

    protected SCursor dupCursor(SCursor cursor, boolean samePosition)
        throws SDatabaseException {

        return view.currentTxn.dupCursor(cursor, writeCursor, samePosition);
    }

    protected void closeCursor(SCursor cursor)
        throws SDatabaseException {

        view.currentTxn.closeCursor(cursor);
    }

    protected boolean checkRecordNumber() {
        return isRecnoOrQueue;
    }
}
