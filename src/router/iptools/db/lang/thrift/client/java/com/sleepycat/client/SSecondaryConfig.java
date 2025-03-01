/*-
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 2002, 2017 Oracle and/or its affiliates.  All rights reserved.
 *
 * $Id$
 */

package com.sleepycat.client;

import com.sleepycat.thrift.TDatabaseConfig;
import com.sleepycat.thrift.TFKDeleteAction;
import com.sleepycat.thrift.TSecondaryDatabaseConfig;

import java.util.Objects;

/**
 * The configuration properties of a SSecondaryDatabase extend those of a
 * primary SDatabase. The secondary database configuration is specified when
 * calling {@link SEnvironment#openSecondaryDatabase}.
 * <p>
 * To create a configuration object with default attributes:
 * <pre>
 *  SecondaryConfig config = new SecondaryConfig();
 * </pre>
 * To set custom attributes:
 * <pre>
 *  SSecondaryConfig config = new SSecondaryConfig();
 *  config.setAllowCreate(true);
 *  config.setSortedDuplicates(true);
 *  config.setKeyCreator(new MyKeyCreator());
 * </pre>
 */
public class SSecondaryConfig extends SDatabaseConfig {
    /** The internal wrapper object. */
    private final SecondaryConfigWrapper wrapper;

    /** The foreign database. */
    private SDatabase foreign;

    /** Whether automatic population of the secondary is allowed. */
    private boolean allowPopulate = false;

    /** The secondary key creator. */
    private SSecondaryKeyCreator keyCreator;

    /** The secondary multiple-key creator. */
    private SSecondaryMultiKeyCreator multiKeyCreator;

    /** The foreign key nullifier. */
    private SForeignKeyNullifier keyNullifier;

    /** The multi-valued foreign key nullifier. */
    private SForeignMultiKeyNullifier multiKeyNullifier;

    /**
     * Creates an instance with the system's default settings.
     */
    public SSecondaryConfig() {
        super();
        this.wrapper =
                new SecondaryConfigWrapper(new TSecondaryDatabaseConfig());
        this.wrapper.getThriftObj().setDbConfig(super.getThriftObj());
    }

    /**
     * Copy constructor.
     *
     * @param config the configuration to be copied
     */
    SSecondaryConfig(SSecondaryConfig config) {
        this(config, config.getThriftObj().deepCopy());
    }

    /**
     * Copy constructor with specified TDatabaseConfig which is retrieved from
     * the server.
     *
     * @param config the configuration to be copied
     * @param dbConfig the base database configuration retrieved from the
     * server
     */
    SSecondaryConfig(SSecondaryConfig config, TDatabaseConfig dbConfig) {
        super(dbConfig);
        this.wrapper = new SecondaryConfigWrapper(
                config.wrapper.getThriftObj().deepCopy());
        this.wrapper.getThriftObj().setDbConfig(super.getThriftObj());
        this.allowPopulate = config.allowPopulate;
        this.foreign = config.foreign;
        this.keyCreator = config.keyCreator;
        this.multiKeyCreator = config.multiKeyCreator;
        this.keyNullifier = config.keyNullifier;
        this.multiKeyNullifier = config.multiKeyNullifier;
    }

    static TSecondaryDatabaseConfig nullSafeGet(SSecondaryConfig config) {
        return config == null ? null : config.wrapper.getThriftObj();
    }

    @Override
    public SSecondaryConfig cloneConfig() {
        return new SSecondaryConfig(this);
    }

    /**
     * Returns whether automatic population of the secondary is allowed. If
     * {@link #setAllowPopulate(boolean)} has not been called, this method
     * returns false.
     *
     * @return whether automatic population of the secondary is allowed.
     * @see #setAllowPopulate(boolean)
     */
    public boolean getAllowPopulate() {
        return this.allowPopulate;
    }

    /**
     * Specifies whether automatic population of the secondary is allowed.
     * <p>
     * If automatic population is allowed, when the secondary database is
     * opened it is checked to see if it is empty. If it is empty, the primary
     * database is read in its entirety and keys are added to the secondary
     * database using the information read from the primary.
     * <p>
     * If this property is set to true, the population of the secondary will be
     * done within the explicit or auto-commit transaction that is used to open
     * the database.
     *
     * @param allowPopulate whether automatic population of the secondary is
     * allowed.
     * @return this
     */
    public SSecondaryConfig setAllowPopulate(boolean allowPopulate) {
        this.allowPopulate = allowPopulate;
        return this;
    }

    /**
     * Return whether the secondary key is immutable.
     *
     * @return whether the secondary key is immutable
     * @see #setImmutableSecondaryKey
     */
    public boolean getImmutableSecondaryKey() {
        return (boolean) this.wrapper.getField(
                TSecondaryDatabaseConfig._Fields.IMMUTABLE_SECONDARY_KEY);
    }

    /**
     * Specify whether the secondary key is immutable.
     * <p>
     * Specifying that a secondary key is immutable can be used to optimize
     * updates when the secondary key in a primary record will never be changed
     * after that primary record is inserted.
     * <p>
     * Be sure to set this property to true only if the secondary key in the
     * primary record is never changed.  If this rule is violated, the
     * secondary index will become corrupted, that is, it will become out of
     * sync with the primary.
     *
     * @param immutableSecondaryKey whether the secondary key is immutable
     * @return this
     */
    public SSecondaryConfig setImmutableSecondaryKey(
            final boolean immutableSecondaryKey) {
        this.wrapper.getThriftObj()
                .setImmutableSecondaryKey(immutableSecondaryKey);
        return this;
    }

    /**
     * Return the database used to check the foreign key integrity constraint,
     * or null if no foreign key constraint will be checked.
     *
     * @return the foreign key database, or null
     * @see #setForeignKeyDatabase
     */
    public SDatabase getForeignKeyDatabase() {
        return this.foreign;
    }

    /**
     * Define a foreign key integrity constraint for a given foreign key
     * database.
     * <p>
     * If this property is non-null, a record must be present in the
     * specified foreign database for every record in the secondary database,
     * where the secondary key value is equal to the foreign database key
     * value. Whenever a record is to be added to the secondary database, the
     * secondary key is used as a lookup key in the foreign database.
     * <p>
     * The foreign database must not have duplicates allowed.</p>
     *
     * @param foreignDb the database used to check the foreign key
     * integrity constraint, or null if no foreign key constraint should be
     * checked.
     * @return this
     */
    public SSecondaryConfig setForeignKeyDatabase(SDatabase foreignDb) {
        this.foreign = foreignDb;
        if (foreignDb == null) {
            this.wrapper.getThriftObj().unsetForeignDb();
        } else {
            this.wrapper.getThriftObj().setForeignDb(foreignDb.getThriftObj());
        }
        return this;
    }

    /**
     * Return the action taken when a referenced record in the foreign key
     * database is deleted.
     *
     * @return the action taken when a referenced record in the foreign key
     * database is deleted.
     * @see #setForeignKeyDeleteAction
     */
    public SForeignKeyDeleteAction getForeignKeyDeleteAction() {
        TFKDeleteAction action = (TFKDeleteAction) this.wrapper.getField(
                TSecondaryDatabaseConfig._Fields.FOREIGN_KEY_DELETE_ACTION);
        return SForeignKeyDeleteAction.toBdb(action);
    }

    /**
     * Specify the action taken when a referenced record in the foreign key
     * database is deleted.
     * <p>
     * This property is ignored if the foreign key database property is null.
     *
     * @param action the action taken when a referenced record in the foreign
     * key database is deleted.
     * @return this
     * @see SForeignKeyDeleteAction
     */
    public SSecondaryConfig setForeignKeyDeleteAction(
            SForeignKeyDeleteAction action) {
        this.wrapper.getThriftObj().setForeignKeyDeleteAction(
                SForeignKeyDeleteAction.toThrift(action));
        return this;
    }

    /**
     * Return the user-supplied object used for creating single-valued
     * secondary keys.
     *
     * @return the user-supplied object used for creating single-valued
     * secondary keys
     * @see #setKeyCreator
     */
    public SSecondaryKeyCreator getKeyCreator() {
        return this.keyCreator;
    }

    /**
     * Specify the user-supplied object used for creating single-valued
     * secondary keys.
     * <p>
     * Unless the primary database is read-only, a key creator is required
     * when opening a secondary database.  Either a SKeyCreator or
     * SMultiKeyCreator must be specified, but both may not be specified.
     * <p>
     *
     * @param keyCreator the user-supplied object used for creating
     * single-valued secondary keys.
     * @return this
     */
    public SSecondaryConfig setKeyCreator(SSecondaryKeyCreator keyCreator) {
        this.keyCreator = keyCreator;
        return this;
    }

    /**
     * Return the user-supplied object used for creating multi-valued
     * secondary keys.
     *
     * @return the user-supplied object used for creating multi-valued secondary
     * keys
     * @see #setMultiKeyCreator
     */
    public SSecondaryMultiKeyCreator getMultiKeyCreator() {
        return this.multiKeyCreator;
    }

    /**
     * Specify the user-supplied object used for creating multi-valued
     * secondary keys.
     * <p>
     * Unless the primary database is read-only, a key creator is required
     * when opening a secondary database.  Either a SKeyCreator or
     * SMultiKeyCreator must be specified, but both may not be specified.
     *
     * @param multiKeyCreator the user-supplied object used for creating
     * multi-valued secondary keys
     */
    public void setMultiKeyCreator(
            final SSecondaryMultiKeyCreator multiKeyCreator) {
        this.multiKeyCreator = multiKeyCreator;
    }

    /**
     * Returns the user-supplied object used for setting single-valued foreign
     * keys to null.
     *
     * @return the user-supplied object used for setting single-valued foreign
     * keys to null.
     * @see #setForeignKeyNullifier
     */
    public SForeignKeyNullifier getForeignKeyNullifier() {
        return this.keyNullifier;
    }

    /**
     * Specifies the user-supplied object used for setting single-valued
     * foreign keys to null.
     * <p>
     * This method may <em>not</em> be used along with {@link
     * #setMultiKeyCreator}.  When using a multi-key creator, use {@link
     * #setForeignMultiKeyNullifier} instead.</p>
     * <p>
     * If the foreign key database property is non-null and the foreign key
     * delete action is <code>NULLIFY</code>, this property is required to be
     * non-null; otherwise, this property is ignored.</p>
     * <p>
     * <em>WARNING:</em> Key nullifier instances are shared by multiple
     * threads and key nullifier methods are called without any special
     * synchronization.  Therefore, key creators must be thread safe.  In
     * general no shared state should be used and any caching of computed
     * values must be done with proper synchronization.</p>
     *
     * @param keyNullifier the user-supplied object used for setting
     * single-valued foreign keys to null.
     * @see SForeignKeyNullifier
     * @see SForeignKeyDeleteAction#NULLIFY
     * @see #setForeignKeyDatabase
     */
    public void setForeignKeyNullifier(SForeignKeyNullifier keyNullifier) {
        this.keyNullifier = keyNullifier;
    }

    /**
     * Returns the user-supplied object used for setting multi-valued foreign
     * keys to null.
     *
     * @return the user-supplied object used for setting multi-valued foreign
     * keys to null.
     * @see #setForeignMultiKeyNullifier
     */
    public SForeignMultiKeyNullifier getForeignMultiKeyNullifier() {
        return this.multiKeyNullifier;
    }

    /**
     * Specifies the user-supplied object used for setting multi-valued foreign
     * keys to null.
     * <p>
     * If the foreign key database property is non-null and the foreign key
     * delete action is <code>NULLIFY</code>, this property is required to be
     * non-null; otherwise, this property is ignored.</p>
     * <p>
     * <em>WARNING:</em> Key nullifier instances are shared by multiple
     * threads and key nullifier methods are called without any special
     * synchronization.  Therefore, key creators must be thread safe.  In
     * general no shared state should be used and any caching of computed
     * values must be done with proper synchronization.</p>
     *
     * @param multiKeyNullifier the user-supplied object used for
     * setting multi-valued foreign keys to null.
     * @see SForeignMultiKeyNullifier
     * @see SForeignKeyDeleteAction#NULLIFY
     * @see #setForeignKeyDatabase
     */
    public void setForeignMultiKeyNullifier(
            SForeignMultiKeyNullifier multiKeyNullifier) {
        this.multiKeyNullifier = multiKeyNullifier;
    }

    private static class SecondaryConfigWrapper extends
            ThriftWrapper<TSecondaryDatabaseConfig, TSecondaryDatabaseConfig._Fields> {
        SecondaryConfigWrapper(TDatabaseConfig dbConfig) {
            super(new TSecondaryDatabaseConfig().setDbConfig(dbConfig));
        }

        SecondaryConfigWrapper(TSecondaryDatabaseConfig sdbConfig) {
            super(sdbConfig);
        }
    }
}
