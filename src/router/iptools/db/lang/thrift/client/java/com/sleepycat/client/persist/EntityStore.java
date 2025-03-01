/*-
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 2002, 2017 Oracle and/or its affiliates.  All rights reserved.
 *
 */

package com.sleepycat.client.persist;

import java.io.Closeable;
import java.util.Set;

import com.sleepycat.client.SDatabase; // for javadoc
import com.sleepycat.client.SDatabaseConfig;
import com.sleepycat.client.SDatabaseException;
import com.sleepycat.client.SEnvironment;
import com.sleepycat.client.SSecondaryConfig;
import com.sleepycat.client.SSequence;
import com.sleepycat.client.SSequenceConfig;
import com.sleepycat.client.STransaction;
import com.sleepycat.client.persist.evolve.EvolveConfig;
import com.sleepycat.client.persist.evolve.EvolveStats;
import com.sleepycat.client.persist.evolve.IncompatibleClassException;
import com.sleepycat.client.persist.evolve.Mutations;
import com.sleepycat.client.persist.impl.Store;
import com.sleepycat.client.persist.model.DeleteAction;
import com.sleepycat.client.persist.model.Entity; // for javadoc
import com.sleepycat.client.persist.model.EntityModel;
import com.sleepycat.client.persist.model.PrimaryKey;
import com.sleepycat.client.persist.model.SecondaryKey;

/**
 * A store for managing persistent entity objects.
 *
 * <p>{@code EntityStore} objects are thread-safe.  Multiple threads may safely
 * call the methods of a shared {@code EntityStore} object.</p>
 *
 * <p>See the <a href="package-summary.html#example">package
 * summary example</a> for an example of using an {@code EntityStore}.</p>
 *
 * <p>Before creating an <code>EntityStore</code> you must create an {@link
 * SEnvironment} object using the Berkeley DB engine API.  The environment may
 * contain any number of entity stores and their associated databases, as well
 * as other databases not associated with an entity store.</p>
 *
 * <p>An entity store is based on an {@link EntityModel}: a data model which
 * defines persistent classes (<em>entity classes</em>), primary keys,
 * secondary keys, and relationships between entities.  A primary index is
 * created for each entity class.  An associated secondary index is created for
 * each secondary key.  The {@link Entity}, {@link PrimaryKey} and {@link
 * SecondaryKey} annotations may be used to define entities and keys.</p>
 *
 * <p>To use an <code>EntityStore</code>, first obtain {@link PrimaryIndex} and
 * {@link SecondaryIndex} objects by calling {@link #getPrimaryIndex
 * getPrimaryIndex} and {@link #getSecondaryIndex getSecondaryIndex}.  Then use
 * these indices to store and access entity records by key.</p>
 *
 * <p>Although not normally needed, you can also use the entity store along
 * with the {@link com.sleepycat.client Base API}.  Methods in the {@link
 * PrimaryIndex} and {@link SecondaryIndex} classes may be used to obtain
 * databases and bindings.  The databases may be used directly for accessing
 * entity records.  The bindings should be called explicitly to translate
 * between {@link com.sleepycat.client.SDatabaseEntry} objects and entity model
 * objects.</p>
 *
 * <p>Each primary and secondary index is associated internally with a {@link
 * SDatabase}.  With any of the above mentioned use cases, methods are provided
 * that may be used for database performance tuning.  The {@link
 * #setPrimaryConfig setPrimaryConfig} and {@link #setSecondaryConfig
 * setSecondaryConfig} methods may be called anytime before a database is
 * opened via {@link #getPrimaryIndex getPrimaryIndex} or {@link
 * #getSecondaryIndex getSecondaryIndex}.  The {@link #setSequenceConfig
 * setSequenceConfig} method may be called anytime before {@link #getSequence
 * getSequence} is called or {@link #getPrimaryIndex getPrimaryIndex} is called
 * for a primary index associated with that sequence.</p>
 *
 *
 * @author Mark Hayes
 */
public class EntityStore
    {

    private Store store;

    /**
     * Opens an entity store in a given environment.
     *
     * @param env an open Berkeley DB SEnvironment.
     *
     * @param storeName the name of the entity store within the given
     * environment.  An empty string is allowed.  Named stores may be used to
     * distinguish multiple sets of persistent entities for the same entity
     * classes in a single environment.  Underlying database names are prefixed
     * with the store name.
     *
     * @param config the entity store configuration, or null to use default
     * configuration properties.
     *
     * @throws StoreExistsException when the {@link
     * StoreConfig#setExclusiveCreate ExclusiveCreate} configuration parameter
     * is true and the store's internal catalog database already exists.
     *
     * @throws StoreNotFoundException when when the {@link
     * StoreConfig#setAllowCreate AllowCreate} configuration parameter is false
     * and the store's internal catalog database does not exist.
     *
     * @throws IncompatibleClassException if an incompatible class change has
     * been made and mutations are not configured for handling the change.  See
     * {@link com.sleepycat.client.persist.evolve Class Evolution} for more
     * information.
     *
     *
     * @throws SDatabaseException the base class for all BDB exceptions.
     */
    public EntityStore(SEnvironment env, String storeName, StoreConfig config)
        throws StoreExistsException,
               StoreNotFoundException,
               IncompatibleClassException,
               SDatabaseException {

        store = new Store(env, storeName, config, false /*rawAccess*/);
    }

    /**
     * Returns the environment associated with this store.
     *
     * @return the environment.
     */
    public SEnvironment getEnvironment() {
        return store.getEnvironment();
    }

    /**
     * Returns a copy of the entity store configuration.
     *
     * @return the config.
     */
    public StoreConfig getConfig() {
        return store.getConfig();
    }

    /**
     * Returns the name of this store.
     *
     * @return the name.
     */
    public String getStoreName() {
        return store.getStoreName();
    }

    

    /**
     * Returns the current entity model for this store.  The current model is
     * derived from the configured entity model and the live entity class
     * definitions.
     *
     * @return the model.
     */
    public EntityModel getModel() {
        return store.getModel();
    }

    /**
     * Returns the set of mutations that were configured when the store was
     * opened, or if none were configured, the set of mutations that were
     * configured and stored previously.
     *
     * @return the mutations.
     */
    public Mutations getMutations() {
        return store.getMutations();
    }

    /**
     * Returns the primary index for a given entity class, opening it if
     * necessary.
     *
     * <p>If they are not already open, the primary and secondary databases for
     * the entity class are created/opened together in a single internal
     * transaction.  When the secondary indices are opened, that can cascade to
     * open other related primary indices.</p>
     *
     * @param primaryKeyClass the class of the entity's primary key field, or
     * the corresponding primitive wrapper class if the primary key field type
     * is a primitive.
     *
     * @param entityClass the entity class for which to open the primary index.
     *
     * @param <PK> the primary key class.
     *
     * @param <E> the entity class.
     *
     * @return the primary index.
     *
     * @throws IllegalArgumentException if the entity class or classes
     * referenced by it are not persistent, or the primary key class does not
     * match the entity's primary key field, or if metadata for the entity or
     * primary key is invalid.
     *
     *
     * @throws SDatabaseException the base class for all BDB exceptions.
     */
    public <PK, E> PrimaryIndex<PK, E>
        getPrimaryIndex(Class<PK> primaryKeyClass, Class<E> entityClass)
        throws SDatabaseException {

        try {
            return store.getPrimaryIndex
                (primaryKeyClass, primaryKeyClass.getName(),
                 entityClass, entityClass.getName());
        } catch (IndexNotAvailableException e) {
            if (!store.attemptRefresh()) {
                throw e;
            }
            return store.getPrimaryIndex
                (primaryKeyClass, primaryKeyClass.getName(),
                 entityClass, entityClass.getName());
        }
    }

    /**
     * Returns a secondary index for a given primary index and secondary key,
     * opening it if necessary.
     *
     * <p><em>NOTE:</em> If the secondary key field is declared in a subclass
     * of the entity class, use {@link #getSubclassIndex} instead.</p>
     *
     * <p>If a {@link SecondaryKey#relatedEntity} is used and the primary index
     * for the related entity is not already open, it will be opened by this
     * method.  That will, in turn, open its secondary indices, which can
     * cascade to open other primary indices.</p>
     *
     * @param primaryIndex the primary index associated with the returned
     * secondary index.  The entity class of the primary index, or one of its
     * superclasses, must contain a secondary key with the given secondary key
     * class and key name.
     *
     * @param keyClass the class of the secondary key field, or the
     * corresponding primitive wrapper class if the secondary key field type is
     * a primitive.
     *
     * @param keyName the name of the secondary key field, or the {@link
     * SecondaryKey#name} if this name annotation property was specified.
     *
     * @param <SK> the secondary key class.
     *
     * @param <PK> the primary key class.
     *
     * @param <E> the entity class.
     *
     * @return the secondary index.
     *
     * @throws IllegalArgumentException if the entity class or one of its
     * superclasses does not contain a key field of the given key class and key
     * name, or if the metadata for the secondary key is invalid.
     *
     *
     * @throws SDatabaseException the base class for all BDB exceptions.
     */
    public <SK, PK, E> SecondaryIndex<SK, PK, E>
        getSecondaryIndex(PrimaryIndex<PK, E> primaryIndex,
                          Class<SK> keyClass,
                          String keyName)
        throws SDatabaseException {

        try {
            return store.getSecondaryIndex
                (primaryIndex, primaryIndex.getEntityClass(),
                 primaryIndex.getEntityClass().getName(),
                 keyClass, keyClass.getName(), keyName);
        } catch (IndexNotAvailableException e) {
            if (!store.attemptRefresh()) {
                throw e;
            }
            return store.getSecondaryIndex
                (primaryIndex, primaryIndex.getEntityClass(),
                 primaryIndex.getEntityClass().getName(),
                 keyClass, keyClass.getName(), keyName);
        }
    }

    /**
     * Returns a secondary index for a secondary key in an entity subclass,
     * opening it if necessary.
     *
     * <p>If a {@link SecondaryKey#relatedEntity} is used and the primary index
     * for the related entity is not already open, it will be opened by this
     * method.  That will, in turn, open its secondary indices, which can
     * cascade to open other primary indices.</p>
     *
     * @param primaryIndex the primary index associated with the returned
     * secondary index.  The entity class of the primary index, or one of its
     * superclasses, must contain a secondary key with the given secondary key
     * class and key name.
     *
     * @param entitySubclass a subclass of the entity class for the primary
     * index.  The entity subclass must contain a secondary key with the given
     * secondary key class and key name.
     *
     * @param keyClass the class of the secondary key field, or the
     * corresponding primitive wrapper class if the secondary key field type is
     * a primitive.
     *
     * @param keyName the name of the secondary key field, or the {@link
     * SecondaryKey#name} if this name annotation property was specified.
     *
     * @param <SK> the secondary key class.
     *
     * @param <PK> the primary key class.
     *
     * @param <E1> the entity class.
     *
     * @param <E2> the entity sub-class.
     *
     * @return the secondary index.
     *
     * @throws IllegalArgumentException if the given entity subclass does not
     * contain a key field of the given key class and key name, or if the
     * metadata for the secondary key is invalid.
     *
     *
     * @throws SDatabaseException the base class for all BDB exceptions.
     */
    public <SK, PK, E1, E2 extends E1> SecondaryIndex<SK, PK, E2>
        getSubclassIndex(PrimaryIndex<PK, E1> primaryIndex,
                         Class<E2> entitySubclass,
                         Class<SK> keyClass,
                         String keyName)
        throws SDatabaseException {

        /* Make subclass metadata available before getting the index. */
        getModel().getClassMetadata(entitySubclass.getName());

        try {
            return store.getSecondaryIndex
                (primaryIndex, entitySubclass,
                 primaryIndex.getEntityClass().getName(),
                 keyClass, keyClass.getName(), keyName);
        } catch (IndexNotAvailableException e) {
            if (!store.attemptRefresh()) {
                throw e;
            }
            return store.getSecondaryIndex
                (primaryIndex, entitySubclass,
                 primaryIndex.getEntityClass().getName(),
                 keyClass, keyClass.getName(), keyName);
        }
    }

    /**
     * Performs conversion of unevolved objects in order to reduce lazy
     * conversion overhead.  Evolution may be performed concurrently with
     * normal access to the store.
     *
     * <p>Conversion is performed one entity class at a time.  An entity class
     * is converted only if it has {@link Mutations} associated with it via
     * {@link StoreConfig#setMutations StoreConfig.setMutations}.</p>
     *
     * <p>Conversion of an entity class is performed by reading each entity,
     * converting it if necessary, and updating it if conversion was performed.
     * When all instances of an entity class are converted, references to the
     * appropriate {@link Mutations} are deleted.  Therefore, if this method is
     * called twice successfully without changing class definitions, the second
     * call will do nothing.</p>
     *
     * @param config the EvolveConfig.
     *
     * @return the EvolveStats.
     *
     *
     * @throws SDatabaseException the base class for all BDB exceptions.
     *
     * @see com.sleepycat.client.persist.evolve Class Evolution
     */
    public EvolveStats evolve(EvolveConfig config)
        throws SDatabaseException {

        return store.evolve(config);
    }

    /**
     * Deletes all instances of this entity class and its (non-entity)
     * subclasses.
     *
     * <p>The primary database for the given entity class will be truncated and
     * all secondary databases will be removed.  The primary and secondary
     * databases associated with the entity class must not be open except by
     * this store, since database truncation/removal is only possible when the
     * database is not open.</p>
     *
     * <p>The primary and secondary databases for the entity class will be
     * closed by this operation and the existing {@link PrimaryIndex} and
     * {@link SecondaryIndex} objects will be invalidated.  To access the
     * indexes, the user must call {@link #getPrimaryIndex} and {@link
     * #getSecondaryIndex} after this operation is complete.</p>
     *
     * <p>Auto-commit is used implicitly if the store is transactional.</p>
     *
     * @param entityClass the entity class whose instances are to be deleted.
     *
     *
     * @throws SDatabaseException the base class for all BDB exceptions.
     */
    public void truncateClass(Class entityClass)
        throws SDatabaseException {

        store.truncateClass(null, entityClass);
    }

    /**
     * Deletes all instances of this entity class and its (non-entity)
     * subclasses.
     *
     * <p>The primary database for the given entity class will be truncated and
     * all secondary databases will be removed.  The primary and secondary
     * databases associated with the entity class must not be open except by
     * this store, since database truncation/removal is only possible when the
     * database is not open.</p>
     *
     * <p>The primary and secondary databases for the entity class will be
     * closed by this operation and the existing {@link PrimaryIndex} and
     * {@link SecondaryIndex} objects will be invalidated.  To access the
     * indexes, the user must call {@link #getPrimaryIndex} and {@link
     * #getSecondaryIndex} after this operation is complete.</p>
     *
     * @param txn the transaction used to protect this operation, null to use
     * auto-commit, or null if the store is non-transactional.
     *
     * @param entityClass the entity class whose instances are to be deleted.
     *
     *
     * @throws SDatabaseException the base class for all BDB exceptions.
     */
    public void truncateClass(STransaction txn, Class entityClass)
        throws SDatabaseException {

        store.truncateClass(txn, entityClass);
    }


    /**
     * Closes the primary and secondary databases for the given entity class
     * that were opened via this store.  The caller must ensure that the
     * primary and secondary indices for the entity class are no longer in
     * use.
     *
     * <p>The primary and secondary databases for the entity class will be
     * closed by this operation and the existing {@link PrimaryIndex} and
     * {@link SecondaryIndex} objects will be invalidated.  To access the
     * indexes, the user must call {@link #getPrimaryIndex} and {@link
     * #getSecondaryIndex} after this operation is complete.</p>
     *
     * @param entityClass the entity class whose databases are to be closed.
     *
     * @throws SDatabaseException the base class for all BDB exceptions.
     */
    public void closeClass(Class entityClass)
        throws SDatabaseException {

        store.closeClass(entityClass);
    }

    /**
     * Closes all databases and sequences that were opened via this store.  The
     * caller must ensure that no databases opened via this store are in use.
     *
     *
     * <p>WARNING: To guard against memory leaks, the application should
     * discard all references to the closed handle.  While BDB makes an effort
     * to discard references from closed objects to the allocated memory for an
     * environment, this behavior is not guaranteed.  The safe course of action
     * for an application is to discard all references to closed BDB
     * objects.</p>
     *
     * @throws SDatabaseException the base class for all BDB exceptions.
     */
    public void close()
        throws SDatabaseException {

        store.close();
    }

    /**
     * Returns a named sequence for using Berkeley DB engine API directly,
     * opening it if necessary.
     *
     * @param name the sequence name, which is normally defined using the
     * {@link PrimaryKey#sequence} annotation property.
     *
     * @return the open sequence for the given sequence name.
     *
     * @throws SDatabaseException the base class for all BDB exceptions.
     */
    public SSequence getSequence(String name)
        throws SDatabaseException {

        return store.getSequence(name);
    }

    /**
     * Returns the default Berkeley DB engine API configuration for a named key
     * sequence.
     *
     * <p>The returned configuration is as follows.  All other properties have
     * default values.</p>
     * <ul>
     * <li>The {@link SSequenceConfig#setInitialValue InitialValue} is one.</li>
     * <li>The {@link SSequenceConfig#setRange Range} minimum is one.</li>
     * <li>The {@link SSequenceConfig#setCacheSize CacheSize} is 100.</li>
     * <li>{@link SSequenceConfig#setAutoCommitNoSync AutoCommitNoSync} is
     * true.</li>
     * <li>{@link SSequenceConfig#setAllowCreate AllowCreate} is set to the
     * inverse of the store {@link StoreConfig#setReadOnly ReadOnly}.
     * setting.</li>
     * </ul>
     *
     * @param name the sequence name, which is normally defined using the
     * {@link PrimaryKey#sequence} annotation property.
     *
     * @return the default configuration for the given sequence name.
     */
    public SSequenceConfig getSequenceConfig(String name) {
        return store.getSequenceConfig(name);
    }

    /**
     * Configures a named key sequence using the Berkeley DB engine API.
     *
     * <p>To be compatible with the entity model and the Direct Persistence
     * Layer, the configuration should be retrieved using {@link
     * #getSequenceConfig getSequenceConfig}, modified, and then passed to this
     * method.  The following configuration properties may not be changed:</p>
     * <ul>
     * <li>{@link SSequenceConfig#setExclusiveCreate ExclusiveCreate}</li>
     * </ul>
     * <p>In addition, {@link SSequenceConfig#setAllowCreate AllowCreate} must be
     * the inverse of {@code ReadOnly}</p>
     *
     * <p>If the range is changed to include the value zero, see {@link
     * PrimaryKey} for restrictions.</p>
     *
     * @param name the sequence name, which is normally defined using the
     * {@link PrimaryKey#sequence} annotation property.
     *
     * @param config the configuration to use for the given sequence name.
     *
     * @throws IllegalArgumentException if the configuration is incompatible
     * with the entity model or the Direct Persistence Layer.
     *
     * @throws IllegalStateException if the sequence has already been opened.
     */
    public void setSequenceConfig(String name, SSequenceConfig config) {
        store.setSequenceConfig(name, config);
    }

    /**
     * Returns the default primary database Berkeley DB engine API
     * configuration for an entity class.
     *
     * <p>The returned configuration is as follows.  All other properties have
     * default values.</p>
     * <ul>
     * <li>{@link SDatabaseConfig#setAllowCreate AllowCreate} is set to the
     * inverse of the store {@link StoreConfig#setReadOnly ReadOnly}.
     * setting.</li>
     * <li>{@link SDatabaseConfig#setReadOnly ReadOnly} is set to match
     * {@link StoreConfig#setReadOnly StoreConfig}.</li>
     * </ul>
     *
     * @param entityClass the entity class identifying the primary database.
     *
     * @return the default configuration for the given entity class.
     */
    public SDatabaseConfig getPrimaryConfig(Class entityClass) {
        return store.getPrimaryConfig(entityClass);
    }

    /**
     * Configures the primary database for an entity class using the Berkeley
     * DB engine API.
     *
     * <p>To be compatible with the entity model and the Direct Persistence
     * Layer, the configuration should be retrieved using {@link
     * #getPrimaryConfig getPrimaryConfig}, modified, and then passed to this
     * method.  The following configuration properties may not be changed:</p>
     * <ul>
     * <li>{@link SDatabaseConfig#setExclusiveCreate ExclusiveCreate}</li>
     * <li>{@link SDatabaseConfig#setSortedDuplicates SortedDuplicates}</li>
     * </ul>
     * <p>In addition, {@link SDatabaseConfig#setAllowCreate AllowCreate} must be
     * the inverse of {@code ReadOnly}</p>
     *
     * @param entityClass the entity class identifying the primary database.
     *
     * @param config the configuration to use for the given entity class.
     *
     * @throws IllegalArgumentException if the configuration is incompatible
     * with the entity model or the Direct Persistence Layer.
     *
     * @throws IllegalStateException if the database has already been opened.
     */
    public void setPrimaryConfig(Class entityClass, SDatabaseConfig config) {
        store.setPrimaryConfig(entityClass, config);
    }

    /**
     * Returns the default secondary database Berkeley DB engine API
     * configuration for an entity class and key name.
     *
     * <p>The returned configuration is as follows.  All other properties have
     * default values.</p>
     * <ul>
     * <li>{@link SDatabaseConfig#setAllowCreate AllowCreate} is set to the
     * inverse of the primary database {@link SDatabaseConfig#setReadOnly
     * ReadOnly} setting.</li>
     * <li>{@link SDatabaseConfig#setReadOnly ReadOnly} is set to match
     * the primary database.</li>
     * <li>{@link SDatabaseConfig#setSortedDuplicates SortedDuplicates} is set
     * according to {@link SecondaryKey#relate}.</li>
     * <li>{@link SSecondaryConfig#setAllowPopulate AllowPopulate} is set to
     * true when a secondary key is added to an existing primary index.</li>
     * <li>{@link SSecondaryConfig#setKeyCreator KeyCreator} or {@link
     * SSecondaryConfig#setMultiKeyCreator MultiKeyCreator} is set to an
     * internal instance.</li>
     * <li>{@link SSecondaryConfig#setForeignMultiKeyNullifier
     * ForeignMultiKeyNullifier} is set to an internal instance if {@link
     * SecondaryKey#onRelatedEntityDelete} is {@link DeleteAction#NULLIFY}.</li>
     * </ul>
     *
     * @param entityClass the entity class containing the given secondary key
     * name.
     *
     * @param keyName the name of the secondary key field, or the {@link
     * SecondaryKey#name} if this name annotation property was specified.
     *
     * @return the default configuration for the given secondary key.
     */
    public SSecondaryConfig getSecondaryConfig(Class entityClass,
                                              String keyName) {
        return store.getSecondaryConfig(entityClass, keyName);
    }

    /**
     * Configures a secondary database for an entity class and key name using
     * the Berkeley DB engine API.
     *
     * <p>To be compatible with the entity model and the Direct Persistence
     * Layer, the configuration should be retrieved using {@link
     * #getSecondaryConfig getSecondaryConfig}, modified, and then passed to
     * this method.  The following configuration properties may not be
     * changed:</p>
     * <ul>
     * <li>{@link SDatabaseConfig#setExclusiveCreate ExclusiveCreate}</li>
     * <li>{@link SDatabaseConfig#setSortedDuplicates SortedDuplicates}</li>
     * <li>{@link SSecondaryConfig#setAllowPopulate AllowPopulate}</li>
     * <li>{@link SSecondaryConfig#setKeyCreator KeyCreator}</li>
     * <li>{@link SSecondaryConfig#setMultiKeyCreator MultiKeyCreator}</li>
     * <li>{@link SSecondaryConfig#setForeignKeyNullifier
     * ForeignKeyNullifier}</li>
     * <li>{@link SSecondaryConfig#setForeignMultiKeyNullifier
     * ForeignMultiKeyNullifier}</li>
     * <li>{@link SSecondaryConfig#setForeignKeyDeleteAction
     * ForeignKeyDeleteAction}</li>
     * <li>{@link SSecondaryConfig#setForeignKeyDatabase
     * ForeignKeyDatabase}</li>
     * </ul>
     * <p>In addition, {@link SDatabaseConfig#setAllowCreate AllowCreate} must be
     * the inverse of {@code ReadOnly}</p>
     *
     * @param entityClass the entity class containing the given secondary key
     * name.
     *
     * @param keyName the name of the secondary key field, or the {@link
     * SecondaryKey#name} if this name annotation property was specified.
     *
     * @param config the configuration to use for the given secondary key.
     *
     * @throws IllegalArgumentException if the configuration is incompatible
     * with the entity model or the Direct Persistence Layer.
     *
     * @throws IllegalStateException if the database has already been opened.
     */
    public void setSecondaryConfig(Class entityClass,
                                   String keyName,
                                   SSecondaryConfig config) {
        store.setSecondaryConfig(entityClass, keyName, config);
    }
}
