/*-
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 2002, 2017 Oracle and/or its affiliates.  All rights reserved.
 *
 */

package com.sleepycat.client.persist;

import java.util.ArrayList;
import java.util.Iterator;
import java.util.List;

import com.sleepycat.client.bind.EntityBinding;
import com.sleepycat.client.bind.EntryBinding;
import com.sleepycat.client.SCursor;
import com.sleepycat.client.SCursorConfig;
import com.sleepycat.client.SDatabase;
import com.sleepycat.client.SDatabaseEntry;
import com.sleepycat.client.SDatabaseException;
import com.sleepycat.client.SJoinCursor;
import com.sleepycat.client.SLockMode;
import com.sleepycat.client.SDatabaseException;
import com.sleepycat.client.SOperationStatus;
import com.sleepycat.client.STransaction;

/**
 * Performs an equality join on two or more secondary keys.
 *
 * <p>{@code EntityJoin} objects are thread-safe.  Multiple threads may safely
 * call the methods of a shared {@code EntityJoin} object.</p>
 *
 * <p>An equality join is a match on all entities in a given primary index that
 * have two or more specific secondary key values.  Note that key ranges may
 * not be matched by an equality join, only exact keys are matched.</p>
 *
 * <p>For example:</p>
 * <pre class="code">
 *  // Index declarations -- see <a href="package-summary.html#example">package summary example</a>.
 *  //
 *  {@literal PrimaryIndex<String, Person> personBySsn;}
 *  {@literal SecondaryIndex<String, String, Person> personByParentSsn;}
 *  {@literal SecondaryIndex<Long, String, Person> personByEmployerIds;}
 *  Employer employer = ...;
 *
 *  // Match on all Person objects having parentSsn "111-11-1111" and also
 *  // containing an employerId of employer.id.  In other words, match on all
 *  // of Bob's children that work for a given employer.
 *  //
 *  {@literal EntityJoin<String, Person> join = new EntityJoin(personBySsn);}
 *  join.addCondition(personByParentSsn, "111-11-1111");
 *  join.addCondition(personByEmployerIds, employer.id);
 *
 *  // Perform the join operation by traversing the results with a cursor.
 *  //
 *  {@literal ForwardCursor<Person> results = join.entities();}
 *  try {
 *      for (Person person : results) {
 *          System.out.println(person.ssn + ' ' + person.name);
 *      }
 *  } finally {
 *      results.close();
 *  }</pre>
 *
 * @author Mark Hayes
 */
public class EntityJoin<PK, E> {

    private PrimaryIndex<PK, E> primary;
    private List<Condition> conditions;

    /**
     * Creates a join object for a given primary index.
     *
     * @param index the primary index on which the join will operate.
     */
    public EntityJoin(PrimaryIndex<PK, E> index) {
        primary = index;
        conditions = new ArrayList<Condition>();
    }

    /**
     * Adds a secondary key condition to the equality join.  Only entities
     * having the given key value in the given secondary index will be returned
     * by the join operation.
     *
     * @param index the secondary index containing the given key value.
     *
     * @param key the key value to match during the join.
     *
     * @param <SK> the secondary key class.
     */
    public <SK> void addCondition(SecondaryIndex<SK, PK, E> index, SK key) {

        /* Make key entry. */
        SDatabaseEntry keyEntry = new SDatabaseEntry();
        index.getKeyBinding().objectToEntry(key, keyEntry);

        /* Use keys database if available. */
        SDatabase db = index.getKeysDatabase();
        if (db == null) {
            db = index.getDatabase();
        }

        /* Add condition. */
        conditions.add(new Condition(db, keyEntry));
    }

    /**
     * Opens a cursor that returns the entities qualifying for the join.  The
     * join operation is performed as the returned cursor is accessed.
     *
     * <p>The operations performed with the cursor will not be transaction
     * protected, and {@link SCursorConfig#DEFAULT} is used implicitly.</p>
     *
     * @return the cursor.
     *
     *
     * @throws IllegalStateException if less than two conditions were added.
     *
     * @throws SDatabaseException the base class for all BDB exceptions.
     */
    public ForwardCursor<E> entities()
        throws SDatabaseException {

        return entities(null, null);
    }

    /**
     * Opens a cursor that returns the entities qualifying for the join.  The
     * join operation is performed as the returned cursor is accessed.
     *
     * @param txn the transaction used to protect all operations performed with
     * the cursor, or null if the operations should not be transaction
     * protected.  If the store is non-transactional, null must be specified.
     * For a transactional store the transaction is optional for read-only
     * access and required for read-write access.
     *
     * @param config the cursor configuration that determines the default lock
     * mode used for all cursor operations, or null to implicitly use {@link
     * SCursorConfig#DEFAULT}.
     *
     * @return the cursor.
     *
     *
     * @throws IllegalStateException if less than two conditions were added.
     *
     * @throws SDatabaseException the base class for all BDB exceptions.
     */
    public ForwardCursor<E> entities(STransaction txn, SCursorConfig config)
        throws SDatabaseException {

        return new JoinForwardCursor<E>(txn, config, false);
    }

    /**
     * Opens a cursor that returns the primary keys of entities qualifying for
     * the join.  The join operation is performed as the returned cursor is
     * accessed.
     *
     * <p>The operations performed with the cursor will not be transaction
     * protected, and {@link SCursorConfig#DEFAULT} is used implicitly.</p>
     *
     * @return the cursor.
     *
     *
     * @throws IllegalStateException if less than two conditions were added.
     *
     * @throws SDatabaseException the base class for all BDB exceptions.
     */
    public ForwardCursor<PK> keys()
        throws SDatabaseException {

        return keys(null, null);
    }

    /**
     * Opens a cursor that returns the primary keys of entities qualifying for
     * the join.  The join operation is performed as the returned cursor is
     * accessed.
     *
     * @param txn the transaction used to protect all operations performed with
     * the cursor, or null if the operations should not be transaction
     * protected.  If the store is non-transactional, null must be specified.
     * For a transactional store the transaction is optional for read-only
     * access and required for read-write access.
     *
     * @param config the cursor configuration that determines the default lock
     * mode used for all cursor operations, or null to implicitly use {@link
     * SCursorConfig#DEFAULT}.
     *
     * @return the cursor.
     *
     *
     * @throws IllegalStateException if less than two conditions were added.
     *
     * @throws SDatabaseException the base class for all BDB exceptions.
     */
    public ForwardCursor<PK> keys(STransaction txn, SCursorConfig config)
        throws SDatabaseException {

        return new JoinForwardCursor<PK>(txn, config, true);
    }

    private static class Condition {

        private SDatabase db;
        private SDatabaseEntry key;

        Condition(SDatabase db, SDatabaseEntry key) {
            this.db = db;
            this.key = key;
        }

        SCursor openCursor(STransaction txn, SCursorConfig config)
            throws SDatabaseException {

            SOperationStatus status;
            SCursor cursor = db.openCursor(txn, config);
            try {
                SDatabaseEntry data = BasicIndex.NO_RETURN_ENTRY;
                status = cursor.getSearchKey(key, data, null);
            } catch (SDatabaseException e) {
                try {
                    cursor.close();
                } catch (SDatabaseException ignored) {}
                throw e;
            }
            if (status == SOperationStatus.SUCCESS) {
                return cursor;
            } else {
                cursor.close();
                return null;
            }
        }
    }

    private class JoinForwardCursor<V> implements ForwardCursor<V> {

        private SCursor[] cursors;
        private SJoinCursor joinCursor;
        private boolean doKeys;

        JoinForwardCursor(STransaction txn, SCursorConfig config, boolean doKeys)
            throws SDatabaseException {

            this.doKeys = doKeys;
            try {
                cursors = new SCursor[conditions.size()];
                for (int i = 0; i < cursors.length; i += 1) {
                    Condition cond = conditions.get(i);
                    SCursor cursor = cond.openCursor(txn, config);
                    if (cursor == null) {
                        /* Leave joinCursor null. */
                        doClose(null);
                        return;
                    }
                    cursors[i] = cursor;
                }
                joinCursor = primary.getDatabase().join(cursors, null);
            } catch (SDatabaseException e) {
                /* doClose will throw e. */
                doClose(e);
            }
        }

        public V next()
            throws SDatabaseException {

            return next(null);
        }

        public V next(SLockMode lockMode)
            throws SDatabaseException {

            if (joinCursor == null) {
                return null;
            }
            if (doKeys) {
                SDatabaseEntry key = new SDatabaseEntry();
                SOperationStatus status = joinCursor.getNext(key, lockMode);
                if (status == SOperationStatus.SUCCESS) {
                    EntryBinding binding = primary.getKeyBinding();
                    return (V) binding.entryToObject(key);
                }
            } else {
                SDatabaseEntry key = new SDatabaseEntry();
                SDatabaseEntry data = new SDatabaseEntry();
                SOperationStatus status =
                    joinCursor.getNext(key, data, lockMode);
                if (status == SOperationStatus.SUCCESS) {
                    EntityBinding binding = primary.getEntityBinding();
                    return (V) binding.entryToObject(key, data);
                }
            }
            return null;
        }

        public Iterator<V> iterator() {
            return iterator(null);
        }

        public Iterator<V> iterator(SLockMode lockMode) {
            return new BasicIterator<V>(this, lockMode);
        }

        public void close()
            throws SDatabaseException {

            doClose(null);
        }

        private void doClose(SDatabaseException firstException)
            throws SDatabaseException {

            if (joinCursor != null) {
                try {
                    joinCursor.close();
                    joinCursor = null;
                } catch (SDatabaseException e) {
                    if (firstException == null) {
                        firstException = e;
                    }
                }
            }
            for (int i = 0; i < cursors.length; i += 1) {
                SCursor cursor = cursors[i];
                if (cursor != null) {
                    try {
                        cursor.close();
                        cursors[i] = null;
                    } catch (SDatabaseException e) {
                        if (firstException == null) {
                            firstException = e;
                        }
                    }
                }
            }
            if (firstException != null) {
                throw firstException;
            }
        }
    }
}
