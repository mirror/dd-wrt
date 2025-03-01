/*-
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 2002, 2017 Oracle and/or its affiliates.  All rights reserved.
 *
 * $Id$
 */

package com.sleepycat.client;

import com.sleepycat.thrift.BdbService;
import com.sleepycat.thrift.TGetResult;
import com.sleepycat.thrift.TJoinCursor;
import com.sleepycat.thrift.TJoinCursorGetConfig;
import com.sleepycat.thrift.TKeyData;
import com.sleepycat.thrift.TOperationStatus;

/**
 * A specialized join cursor for use in performing equality or natural joins on
 * secondary indices.
 * <p>
 * A join cursor is returned when calling {@link SDatabase#join}.
 * <p>
 * To open a join cursor using two secondary cursors:
 * <pre>
 *  STransaction txn = ...
 *  SDatabase primaryDb = ...
 *  SSecondaryDatabase secondaryDb1 = ...
 *  SSecondaryDatabase secondaryDb2 = ...
 *
 *  SSecondaryCursor cursor1 = null;
 *  SSecondaryCursor cursor2 = null;
 *  SJoinCursor joinCursor = null;
 *  try {
 *      SDatabaseEntry key = new SDatabaseEntry();
 *      SDatabaseEntry data = new SDatabaseEntry();
 *
 *      cursor1 = secondaryDb1.openSecondaryCursor(txn, null);
 *      cursor2 = secondaryDb2.openSecondaryCursor(txn, null);
 *
 *      key.setData(...); // initialize key for secondary index 1
 *      SOperationStatus status1 =
 *          cursor1.getSearchKey(key, data, SLockMode.DEFAULT);
 *      key.setData(...); // initialize key for secondary index 2
 *      OperationStatus status2 =
 *          cursor2.getSearchKey(key, data, SLockMode.DEFAULT);
 *
 *      if (status1 == OperationStatus.SUCCESS &amp;&amp;
 *              status2 == OperationStatus.SUCCESS) {
 *          SSecondaryCursor[] cursors = {cursor1, cursor2};
 *          joinCursor = primaryDb.join(cursors, null);
 *
 *          while (true) {
 *              SOperationStatus joinStatus = joinCursor.getNext(key, data,
 *                  SLockMode.DEFAULT);
 *              if (joinStatus == OperationStatus.SUCCESS) {
 *                  // Do something with the key and data.
 *              } else {
 *                  break;
 *              }
 *          }
 *      }
 *  } finally {
 *      if (cursor1 != null) {
 *          cursor1.close();
 *      }
 *      if (cursor2 != null) {
 *          cursor2.close();
 *      }
 *      if (joinCursor != null) {
 *          joinCursor.close();
 *      }
 * }
 * </pre>
 */
public class SJoinCursor implements RemoteCallHelper, AutoCloseable {
    /** The remote join cursor handle. */
    private final TJoinCursor joinCursor;

    /** The join cursor configuration. */
    private final SJoinConfig config;

    /** The enclosing primary database. */
    private final SDatabase database;

    /** The remote service client. */
    private final BdbService.Client client;

    SJoinCursor(TJoinCursor joinCursor, SJoinConfig config, SDatabase database,
            BdbService.Client client) {
        this.joinCursor = joinCursor;
        this.config = config;
        this.database = database;
        this.client = client;
    }

    /**
     * Closes the cursors that have been opened by this join cursor.
     * <p>
     * The cursors passed to {@link SDatabase#join} are not closed by this
     * method, and should be closed by the caller.
     *
     * @throws SDatabaseException if any error occurs
     */
    public void close() throws SDatabaseException {
        remoteCall(() -> {
            this.client.closeJoinCursorHandle(this.joinCursor);
            return null;
        });
    }

    /**
     * Returns this object's configuration.
     *
     * @return this object's configuration
     */
    public SJoinConfig getConfig() {
        return this.config;
    }

    /**
     * Returns the primary database handle associated with this cursor.
     *
     * @return the primary database handle associated with this cursor
     */
    public SDatabase getDatabase() {
        return this.database;
    }

    /**
     * Returns the next primary key resulting from the join operation.
     * <p>
     * An entry is returned by the join cursor for each primary key/data pair
     * having all secondary key values that were specified using the array of
     * secondary cursors passed to {@link SDatabase#join}.
     *
     * @param key the primary key returned as output
     * @param lockMode the locking attributes; if null, default attributes are
     * used
     * @return {@link SOperationStatus#NOTFOUND} if no matching key/data pair is
     * found; {@link SOperationStatus#KEYEMPTY} if the database is a Queue or
     * Recno database and the specified key exists, but was never explicitly
     * created by the application or was later deleted; otherwise, {@link
     * SOperationStatus#SUCCESS}.
     * @throws SDatabaseException if any error occurs
     */
    public SOperationStatus getNext(SDatabaseEntry key, SLockMode lockMode)
            throws SDatabaseException {
        return getNext(key, null, lockMode);
    }

    /**
     * Returns the next primary key and data resulting from the join operation.
     * <p>
     * An entry is returned by the join cursor for each primary key/data pair
     * having all secondary key values that were specified using the array of
     * secondary cursors passed to {@link SDatabase#join}.
     *
     * @param key the primary key returned as output
     * @param data the primary data returned as output
     * @param lockMode the locking attributes; if null, default attributes are
     * used
     * @return {@link SOperationStatus#NOTFOUND} if no matching key/data pair is
     * found; {@link SOperationStatus#KEYEMPTY} if the database is a Queue or
     * Recno database and the specified key exists, but was never explicitly
     * created by the application or was later deleted; otherwise, {@link
     * SOperationStatus#SUCCESS}.
     * @throws SDatabaseException if any error occurs
     */
    public SOperationStatus getNext(SDatabaseEntry key, SDatabaseEntry data,
            SLockMode lockMode) throws SDatabaseException {
        return remoteCall(() -> {
            TJoinCursorGetConfig config = new TJoinCursorGetConfig();
            setConfig(config, lockMode);

            TKeyData pair = new TKeyData().setKey(key.getThriftObj());
            if (data != null) {
                pair.setData(data.getThriftObj());
            }
            config.setPair(pair);

            TGetResult result =
                    this.client.joinCursorGet(this.joinCursor, config);

            if (result.status == TOperationStatus.SUCCESS) {
                TKeyData resultPair = result.pairs.get(0);
                key.setDataFromTDbt(resultPair.key);
                if (data != null) {
                    data.setDataFromTDbt(resultPair.data);
                }
            }

            return SOperationStatus.toBdb(result.status);
        });
    }

    private TJoinCursorGetConfig setConfig(TJoinCursorGetConfig config,
            SLockMode lockMode) {
        if (lockMode == SLockMode.RMW) {
            config.setRmw(true);
        } else if (lockMode == SLockMode.READ_UNCOMMITTED) {
            config.setReadUncommitted(true);
        }
        return config;
    }
}
