/*-
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 2002, 2017 Oracle and/or its affiliates.  All rights reserved.
 *
 */

package com.sleepycat.client.persist.impl;

import java.util.ArrayList;
import java.util.Comparator;
import java.util.HashMap;
import java.util.HashSet;
import java.util.IdentityHashMap;
import java.util.List;
import java.util.Map;
import java.util.Set;
import java.util.WeakHashMap;

import com.sleepycat.client.bind.EntityBinding;
import com.sleepycat.client.bind.tuple.StringBinding;
import com.sleepycat.client.compat.DbCompat;
import com.sleepycat.client.SCursor;
import com.sleepycat.client.SCursorConfig;
import com.sleepycat.client.SDatabase;
import com.sleepycat.client.SDatabaseConfig;
import com.sleepycat.client.SDatabaseEntry;
import com.sleepycat.client.SDatabaseException;
import com.sleepycat.client.SEnvironment;
import com.sleepycat.client.SForeignKeyDeleteAction;
import com.sleepycat.client.SOperationStatus;
import com.sleepycat.client.SSecondaryConfig;
import com.sleepycat.client.SSecondaryDatabase;
import com.sleepycat.client.SSequence;
import com.sleepycat.client.SSequenceConfig;
import com.sleepycat.client.STransaction;
import com.sleepycat.client.STransactionConfig;
import com.sleepycat.client.persist.DatabaseNamer;
import com.sleepycat.client.persist.IndexNotAvailableException;
import com.sleepycat.client.persist.PrimaryIndex;
import com.sleepycat.client.persist.SecondaryIndex;
import com.sleepycat.client.persist.StoreConfig;
import com.sleepycat.client.persist.StoreExistsException;
import com.sleepycat.client.persist.StoreNotFoundException;
import com.sleepycat.client.persist.evolve.EvolveConfig;
import com.sleepycat.client.persist.evolve.EvolveEvent;
import com.sleepycat.client.persist.evolve.EvolveInternal;
import com.sleepycat.client.persist.evolve.EvolveListener;
import com.sleepycat.client.persist.evolve.EvolveStats;
import com.sleepycat.client.persist.evolve.IncompatibleClassException;
import com.sleepycat.client.persist.evolve.Mutations;
import com.sleepycat.client.persist.model.DeleteAction;
import com.sleepycat.client.persist.model.EntityMetadata;
import com.sleepycat.client.persist.model.EntityModel;
import com.sleepycat.client.persist.model.PrimaryKeyMetadata;
import com.sleepycat.client.persist.model.Relationship;
import com.sleepycat.client.persist.model.SecondaryKeyMetadata;
import com.sleepycat.client.persist.raw.RawObject;
import com.sleepycat.client.util.keyrange.KeyRange;
import com.sleepycat.client.util.RuntimeExceptionWrapper;

/**
 * Base implementation for EntityStore and RawStore.  The methods here
 * correspond directly to those in EntityStore; see EntityStore documentation
 * for details.
 *
 * @author Mark Hayes
 */
public class Store {

    public static final String NAME_SEPARATOR = "#";
    private static final String NAME_PREFIX = "persist" + NAME_SEPARATOR;
    private static final String DB_NAME_PREFIX = "com.sleepycat.client.persist.";
    private static final String CATALOG_DB = DB_NAME_PREFIX + "formats";
    private static final String SEQUENCE_DB = DB_NAME_PREFIX + "sequences";

    private static Map<SEnvironment, Map<String, PersistCatalog>> catalogPool =
        new WeakHashMap<SEnvironment, Map<String, PersistCatalog>>();

    /* For unit testing. */
    private static SyncHook syncHook;
    public static boolean expectFlush;

    private final SEnvironment env;
    private final boolean rawAccess;
    private volatile PersistCatalog catalog;
    private EntityModel model;
    private final StoreConfig storeConfig;
    private final String storeName;
    private final String storePrefix;
    private final Map<String, InternalPrimaryIndex> priIndexMap;
    private final Map<String, InternalSecondaryIndex> secIndexMap;
    private final Map<String, SDatabaseConfig> priConfigMap;
    private final Map<String, SSecondaryConfig> secConfigMap;
    private final Map<String, PersistKeyBinding> keyBindingMap;
    private SDatabase sequenceDb;
    private final Map<String, SSequence> sequenceMap;
    private final Map<String, SSequenceConfig> sequenceConfigMap;
    private final IdentityHashMap<SDatabase, Object> deferredWriteDatabases;
    private final Map<String, Set<String>> inverseRelatedEntityMap;
    private final STransactionConfig autoCommitTxnConfig;
    private final STransactionConfig autoCommitNoWaitTxnConfig;

    public Store(SEnvironment env,
                 String storeName,
                 StoreConfig config,
                 boolean rawAccess)
        throws StoreExistsException,
               StoreNotFoundException,
               IncompatibleClassException,
               SDatabaseException {

        this.env = env;
        this.storeName = storeName;
        this.rawAccess = rawAccess;

        if (env == null || storeName == null) {
            throw new NullPointerException
                ("env and storeName parameters must not be null");
        }

        storeConfig = (config != null) ?
            config.clone() :
            StoreConfig.DEFAULT;

        autoCommitTxnConfig = new STransactionConfig();
        autoCommitNoWaitTxnConfig = new STransactionConfig();
        autoCommitNoWaitTxnConfig.setNoWait(true);

        model = config.getModel();

        storePrefix = NAME_PREFIX + storeName + NAME_SEPARATOR;
        priIndexMap = new HashMap<String, InternalPrimaryIndex>();
        secIndexMap = new HashMap<String, InternalSecondaryIndex>();
        priConfigMap = new HashMap<String, SDatabaseConfig>();
        secConfigMap = new HashMap<String, SSecondaryConfig>();
        keyBindingMap = new HashMap<String, PersistKeyBinding>();
        sequenceMap = new HashMap<String, SSequence>();
        sequenceConfigMap = new HashMap<String, SSequenceConfig>();
        deferredWriteDatabases = new IdentityHashMap<SDatabase, Object>();

        if (rawAccess) {
            /* Open a read-only catalog that uses the stored model. */
            if (model != null) {
                throw new IllegalArgumentException
                    ("A model may not be specified when opening a RawStore");
            }
            SDatabaseConfig dbConfig = new SDatabaseConfig();
            dbConfig.setReadOnly(true);
            catalog = new PersistCatalog
                (env, storePrefix, storePrefix + CATALOG_DB, dbConfig,
                 null /*model*/, config.getMutations(), rawAccess, this);
        } else {
            /* Open the shared catalog that uses the current model. */
            synchronized (catalogPool) {
                Map<String, PersistCatalog> catalogMap = catalogPool.get(env);
                if (catalogMap == null) {
                    catalogMap = new HashMap<String, PersistCatalog>();
                    catalogPool.put(env, catalogMap);
                }
                catalog = catalogMap.get(storeName);
                if (catalog != null) {
                    catalog.openExisting();
                } else {
                    SDatabaseConfig dbConfig = new SDatabaseConfig();
                    dbConfig.setAllowCreate(storeConfig.getAllowCreate());
                    dbConfig.setExclusiveCreate
                        (storeConfig.getExclusiveCreate());
                    dbConfig.setReadOnly(storeConfig.getReadOnly());
                    DbCompat.setTypeBtree(dbConfig);
                    catalog = new PersistCatalog
                        (env, storePrefix, storePrefix + CATALOG_DB, dbConfig,
                         model, config.getMutations(), rawAccess, this);
                    catalogMap.put(storeName, catalog);
                }
            }
        }

        /*
         * If there is no model parameter, use the default or stored model
         * obtained from the catalog.
         */
        model = catalog.getResolvedModel();

        /*
         * For each existing entity with a relatedEntity reference, create an
         * inverse map (back pointer) from the class named in the relatedEntity
         * to the class containing the secondary key.  This is used to open the
         * class containing the secondary key whenever we open the
         * relatedEntity class, to configure foreign key constraints. Note that
         * we do not need to update this map as new primary indexes are
         * created, because opening the new index will setup the foreign key
         * constraints. [#15358]
         */
        inverseRelatedEntityMap = new HashMap<String, Set<String>>();
        List<Format> entityFormats = new ArrayList<Format>();
        catalog.getEntityFormats(entityFormats);
        for (Format entityFormat : entityFormats) {
            EntityMetadata entityMeta = entityFormat.getEntityMetadata();
            for (SecondaryKeyMetadata secKeyMeta :
                 entityMeta.getSecondaryKeys().values()) {
                String relatedClsName = secKeyMeta.getRelatedEntity();
                if (relatedClsName != null) {
                    Set<String> inverseClassNames =
                        inverseRelatedEntityMap.get(relatedClsName);
                    if (inverseClassNames == null) {
                        inverseClassNames = new HashSet<String>();
                        inverseRelatedEntityMap.put
                            (relatedClsName, inverseClassNames);
                    }
                    inverseClassNames.add(entityMeta.getClassName());
                }
            }
        }
    }

    public SEnvironment getEnvironment() {
        return env;
    }

    public StoreConfig getConfig() {
        return storeConfig.clone();
    }

    public String getStoreName() {
        return storeName;
    }


    /**
     * For unit testing.
     */
    public boolean isReplicaUpgradeMode() {
        return catalog.isReplicaUpgradeMode();
    }

    public EntityModel getModel() {
        return model;
    }

    public Mutations getMutations() {
        return catalog.getMutations();
    }

    /**
     * A getPrimaryIndex with extra parameters for opening a raw store.
     * primaryKeyClass and entityClass are used for generic typing; for a raw
     * store, these should always be Object.class and RawObject.class.
     * primaryKeyClassName is used for consistency checking and should be null
     * for a raw store only.  entityClassName is used to identify the store and
     * may not be null.
     */
    public synchronized <PK, E> PrimaryIndex<PK, E>
        getPrimaryIndex(Class<PK> primaryKeyClass,
                        String primaryKeyClassName,
                        Class<E> entityClass,
                        String entityClassName)
        throws SDatabaseException, IndexNotAvailableException {

        assert (rawAccess && entityClass == RawObject.class) ||
              (!rawAccess && entityClass != RawObject.class);
        assert (rawAccess && primaryKeyClassName == null) ||
              (!rawAccess && primaryKeyClassName != null);

        checkOpen();

        InternalPrimaryIndex<PK, E> priIndex =
            priIndexMap.get(entityClassName);
        if (priIndex == null) {

            /* Check metadata. */
            EntityMetadata entityMeta = checkEntityClass(entityClassName);
            PrimaryKeyMetadata priKeyMeta = entityMeta.getPrimaryKey();
            if (primaryKeyClassName == null) {
                primaryKeyClassName = priKeyMeta.getClassName();
            } else {
                String expectClsName =
                    SimpleCatalog.keyClassName(priKeyMeta.getClassName());
                if (!primaryKeyClassName.equals(expectClsName)) {
                    throw new IllegalArgumentException
                        ("Wrong primary key class: " + primaryKeyClassName +
                         " Correct class is: " + expectClsName);
                }
            }

            /* Create bindings. */
            PersistEntityBinding entityBinding =
                new PersistEntityBinding(catalog, entityClassName, rawAccess);
            PersistKeyBinding keyBinding = getKeyBinding(primaryKeyClassName);

            /* If not read-only, get the primary key sequence. */
            String seqName = priKeyMeta.getSequenceName();
            if (!storeConfig.getReadOnly() && seqName != null) {
                entityBinding.keyAssigner = new PersistKeyAssigner
                    (keyBinding, entityBinding, getSequence(seqName));
            }

            /*
             * Use a single transaction for opening the primary DB and its
             * secondaries.  If opening any secondary fails, abort the
             * transaction and undo the changes to the state of the store.
             * Also support undo if the store is non-transactional.
             *
             * Use a no-wait transaction to avoid blocking on a Replica while
             * attempting to open an index that is currently being populated
             * via the replication stream from the Master.
             */
            STransaction txn = null;
            SDatabaseConfig dbConfig = getPrimaryConfig(entityMeta);
            if (dbConfig.getTransactional() &&
                DbCompat.getThreadTransaction(env) == null) {
                txn = env.beginTransaction(null, autoCommitNoWaitTxnConfig);
            }
            PrimaryOpenState priOpenState =
                new PrimaryOpenState(entityClassName);
            final boolean saveAllowCreate = dbConfig.getAllowCreate();
            boolean success = false;
            try {

                /*
                 * The AllowCreate setting is false in read-only / Replica
                 * upgrade mode. In this mode new primaries are not available.
                 * They can be opened later when the upgrade is complete on the
                 * Master, by calling getSecondaryIndex.  [#16655]
                 */
                if (catalog.isReadOnly()) {
                    dbConfig.setAllowCreate(false);
                }

                /*
                 * Open the primary database.  Account for database renaming
                 * by calling getDatabaseClassName.  The dbClassName will be
                 * null if the format has not yet been stored. [#16655].
                 */
                SDatabase db = null;
                final String dbClassName =
                    catalog.getDatabaseClassName(entityClassName);
                if (dbClassName != null) {
                    final String[] fileAndDbNames =
                        parseDbName(storePrefix + dbClassName);
                        db = DbCompat.openDatabase(env, txn, fileAndDbNames[0],
                                                   fileAndDbNames[1],
                                                   dbConfig);
                }
                if (db == null) {
                    throw new IndexNotAvailableException
                        ("PrimaryIndex not yet available on this Replica, " +
                         "entity class: " + entityClassName);
                }

                priOpenState.addDatabase(db);

                /* Create index object. */
                priIndex = new InternalPrimaryIndex(db, primaryKeyClass,
                                                    keyBinding, entityClass,
                                                    entityBinding);

                /* Update index and database maps. */
                priIndexMap.put(entityClassName, priIndex);
                if (DbCompat.getDeferredWrite(dbConfig)) {
                    deferredWriteDatabases.put(db, null);
                }

                /* If not read-only, open all associated secondaries. */
                if (!dbConfig.getReadOnly()) {
                    openSecondaryIndexes(txn, entityMeta, priOpenState);

                    /*
                     * To enable foreign key contraints, also open all primary
                     * indexes referring to this class via a relatedEntity
                     * property in another entity. [#15358]
                     */
                    Set<String> inverseClassNames =
                        inverseRelatedEntityMap.get(entityClassName);
                    if (inverseClassNames != null) {
                        for (String relatedClsName : inverseClassNames) {
                            getRelatedIndex(relatedClsName);
                        }
                    }
                }
                success = true;
            } finally {
                dbConfig.setAllowCreate(saveAllowCreate);
                if (success) {
                    if (txn != null) {
                        txn.commit();
                    }
                } else {
                    if (txn != null) {
                        txn.abort();
                    }
                    priOpenState.undoState();
                }
            }
        }
        return priIndex;
    }

    /**
     * Holds state information about opening a primary index and its secondary
     * indexes.  Used to undo the state of this object if the transaction
     * opening the primary and secondaries aborts.  Also used to close all
     * databases opened during this process for a non-transactional store.
     */
    private class PrimaryOpenState {

        private String entityClassName;
        private IdentityHashMap<SDatabase, Object> databases;
        private Set<String> secNames;

        PrimaryOpenState(String entityClassName) {
            this.entityClassName = entityClassName;
            databases = new IdentityHashMap<SDatabase, Object>();
            secNames = new HashSet<String>();
        }

        /**
         * Save a database that was opening during this operation.
         */
        void addDatabase(SDatabase db) {
            databases.put(db, null);
        }

        /**
         * Save the name of a secondary index that was opening during this
         * operation.
         */
        void addSecondaryName(String secName) {
            secNames.add(secName);
        }

        /**
         * Reset all state information and closes any databases opened, when
         * this operation fails.  This method should be called for both
         * transactional and non-transsactional operation.
         *
         * For transactional operations on JE, we don't strictly need to close
         * the databases since the transaction abort will do that.  However,
         * closing them is harmless on JE, and required for DB core.
         */
        void undoState() {
            for (SDatabase db : databases.keySet()) {
                try {
                    db.close();
                } catch (Exception ignored) {
                }
            }
            priIndexMap.remove(entityClassName);
            for (String secName : secNames) {
                secIndexMap.remove(secName);
            }
            for (SDatabase db : databases.keySet()) {
                deferredWriteDatabases.remove(db);
            }
        }
    }

    /**
     * Opens a primary index related via a foreign key (relatedEntity).
     * Related indexes are not opened in the same transaction used by the
     * caller to open a primary or secondary.  It is OK to leave the related
     * index open when the caller's transaction aborts.  It is only important
     * to open a primary and its secondaries atomically.
     */
    private PrimaryIndex getRelatedIndex(String relatedClsName)
        throws SDatabaseException {

        PrimaryIndex relatedIndex = priIndexMap.get(relatedClsName);
        if (relatedIndex == null) {
            EntityMetadata relatedEntityMeta =
                checkEntityClass(relatedClsName);
            Class relatedKeyCls;
            String relatedKeyClsName;
            Class relatedCls;
            if (rawAccess) {
                relatedCls = RawObject.class;
                relatedKeyCls = Object.class;
                relatedKeyClsName = null;
            } else {
                try {
                    relatedCls = catalog.resolveClass(relatedClsName);
                } catch (ClassNotFoundException e) {
                    throw new IllegalArgumentException
                        ("Related entity class not found: " +
                         relatedClsName);
                }
                relatedKeyClsName = SimpleCatalog.keyClassName
                    (relatedEntityMeta.getPrimaryKey().getClassName());
                relatedKeyCls = catalog.resolveKeyClass(relatedKeyClsName);
            }

            /*
             * Cycles are prevented here by adding primary indexes to the
             * priIndexMap as soon as they are created, before opening related
             * indexes.
             */
            relatedIndex = getPrimaryIndex
                (relatedKeyCls, relatedKeyClsName,
                 relatedCls, relatedClsName);
        }
        return relatedIndex;
    }

    /**
     * A getSecondaryIndex with extra parameters for opening a raw store.
     * keyClassName is used for consistency checking and should be null for a
     * raw store only.
     */
    public synchronized <SK, PK, E1, E2 extends E1> SecondaryIndex<SK, PK, E2>
        getSecondaryIndex(PrimaryIndex<PK, E1> primaryIndex,
                          Class<E2> entityClass,
                          String entityClassName,
                          Class<SK> keyClass,
                          String keyClassName,
                          String keyName)
        throws SDatabaseException, IndexNotAvailableException {

        assert (rawAccess && keyClassName == null) ||
              (!rawAccess && keyClassName != null);

        checkOpen();

        EntityMetadata entityMeta = null;
        SecondaryKeyMetadata secKeyMeta = null;

        /* Validate the subclass for a subclass index. */
        if (entityClass != primaryIndex.getEntityClass()) {
            entityMeta = model.getEntityMetadata(entityClassName);
            assert entityMeta != null;
            secKeyMeta = checkSecKey(entityMeta, keyName);
            String subclassName = entityClass.getName();
            String declaringClassName = secKeyMeta.getDeclaringClassName();
            if (!subclassName.equals(declaringClassName)) {
                throw new IllegalArgumentException
                    ("Key for subclass " + subclassName +
                     " is declared in a different class: " +
                     makeSecName(declaringClassName, keyName));
            }

            /*
             * Get/create the subclass format to ensure it is stored in the
             * catalog, even if no instances of the subclass are stored.
             * [#16399]
             */
            try {
                catalog.getFormat(entityClass,
                                  false /*checkEntitySubclassIndexes*/);
            } catch (RefreshException e) {
                e.refresh();
                try {
                    catalog.getFormat(entityClass,
                                      false /*checkEntitySubclassIndexes*/);
                } catch (RefreshException e2) {
                    throw DbCompat.unexpectedException(e2);
                }
            }
        }

        /*
         * Even though the primary is already open, we can't assume the
         * secondary is open because we don't automatically open all
         * secondaries when the primary is read-only.  Use auto-commit (a null
         * transaction) since we're opening only one database.
         */
        String secName = makeSecName(entityClassName, keyName);
        InternalSecondaryIndex<SK, PK, E2> secIndex = secIndexMap.get(secName);
        if (secIndex == null) {
            if (entityMeta == null) {
                entityMeta = model.getEntityMetadata(entityClassName);
                assert entityMeta != null;
            }
            if (secKeyMeta == null) {
                secKeyMeta = checkSecKey(entityMeta, keyName);
            }

            /* Check metadata. */
            if (keyClassName == null) {
                keyClassName = getSecKeyClass(secKeyMeta);
            } else {
                String expectClsName = getSecKeyClass(secKeyMeta);
                if (!keyClassName.equals(expectClsName)) {
                    throw new IllegalArgumentException
                        ("Wrong secondary key class: " + keyClassName +
                         " Correct class is: " + expectClsName);
                }
            }

            /*
             * Account for database renaming.  The dbClassName or dbKeyName
             * will be null if the format has not yet been stored. [#16655]
             */
            final String dbClassName =
                catalog.getDatabaseClassName(entityClassName);
            final String dbKeyName =
                catalog.getDatabaseKeyName(entityClassName, keyName);
            if (dbClassName != null && dbKeyName != null) {

                /*
                 * Use a no-wait transaction to avoid blocking on a Replica
                 * while attempting to open an index that is currently being
                 * populated via the replication stream from the Master.
                 */
                STransaction txn = null;
                if (getPrimaryConfig(entityMeta).getTransactional() &&
                    DbCompat.getThreadTransaction(env) == null) {
                    txn = env.beginTransaction(null,
                                               autoCommitNoWaitTxnConfig);
                }
                boolean success = false;
                try {

                    /*
                     * The doNotCreate param is true below in read-only /
                     * Replica upgrade mode. In this mode new secondaries are
                     * not available.  They can be opened later when the
                     * upgrade is complete on the Master, by calling
                     * getSecondaryIndex.  [#16655]
                     */
                    secIndex = openSecondaryIndex
                        (txn, primaryIndex, entityClass, entityMeta,
                         keyClass, keyClassName, secKeyMeta, secName,
                         makeSecName(dbClassName, dbKeyName),
                         catalog.isReadOnly() /*doNotCreate*/,
                         null /*priOpenState*/);
                    success = true;
                } finally {
                    if (success) {
                        if (txn != null) {
                            txn.commit();
                        }
                    } else {
                        if (txn != null) {
                            txn.abort();
                        }
                    }
                }
            }
            if (secIndex == null) {
                throw new IndexNotAvailableException
                    ("SecondaryIndex not yet available on this Replica, " +
                     "entity class: " + entityClassName + ", key name: " +
                     keyName);
            }
        }
        return secIndex;
    }

    /**
     * Opens secondary indexes for a given primary index metadata.
     */
    private void openSecondaryIndexes(STransaction txn,
                                      EntityMetadata entityMeta,
                                      PrimaryOpenState priOpenState)
        throws SDatabaseException {

        String entityClassName = entityMeta.getClassName();
        PrimaryIndex<Object, Object> priIndex =
            priIndexMap.get(entityClassName);
        assert priIndex != null;
        Class<Object> entityClass = priIndex.getEntityClass();

        for (SecondaryKeyMetadata secKeyMeta :
             entityMeta.getSecondaryKeys().values()) {
            String keyName = secKeyMeta.getKeyName();
            String secName = makeSecName(entityClassName, keyName);
            SecondaryIndex<Object, Object, Object> secIndex =
                secIndexMap.get(secName);
            if (secIndex == null) {
                String keyClassName = getSecKeyClass(secKeyMeta);
                /* RawMode: should not require class. */
                Class keyClass = catalog.resolveKeyClass(keyClassName);

                /*
                 * Account for database renaming.  The dbClassName or dbKeyName
                 * will be null if the format has not yet been stored. [#16655]
                 */
                final String dbClassName =
                    catalog.getDatabaseClassName(entityClassName);
                final String dbKeyName =
                    catalog.getDatabaseKeyName(entityClassName, keyName);
                if (dbClassName != null && dbKeyName != null) {

                    /*
                     * The doNotCreate param is true below in two cases:
                     * 1- When SecondaryBulkLoad=true, new secondaries are not
                     *    created/populated until getSecondaryIndex is called.
                     * 2- In read-only / Replica upgrade mode, new secondaries
                     *    are not openeed when the primary is opened.  They can
                     *    be opened later when the upgrade is complete on the
                     *    Master, by calling getSecondaryIndex.  [#16655]
                     */
                    openSecondaryIndex
                        (txn, priIndex, entityClass, entityMeta,
                         keyClass, keyClassName, secKeyMeta,
                         secName, makeSecName(dbClassName, dbKeyName),
                         (storeConfig.getSecondaryBulkLoad() ||
                          catalog.isReadOnly()) /*doNotCreate*/,
                         priOpenState);
                }
            }
        }
    }

    /**
     * Opens a secondary index with a given transaction and adds it to the
     * secIndexMap.  We assume that the index is not already open.
     */
    private <SK, PK, E1, E2 extends E1> InternalSecondaryIndex<SK, PK, E2>
        openSecondaryIndex(STransaction txn,
                           PrimaryIndex<PK, E1> primaryIndex,
                           Class<E2> entityClass,
                           EntityMetadata entityMeta,
                           Class<SK> keyClass,
                           String keyClassName,
                           SecondaryKeyMetadata secKeyMeta,
                           String secName,
                           String dbSecName,
                           boolean doNotCreate,
                           PrimaryOpenState priOpenState)
        throws SDatabaseException {

        assert !secIndexMap.containsKey(secName);
        String[] fileAndDbNames = parseDbName(storePrefix + dbSecName);
        SSecondaryConfig config =
            getSecondaryConfig(secName, entityMeta, keyClassName, secKeyMeta);
        SDatabase priDb = primaryIndex.getDatabase();
        SDatabaseConfig priConfig = priDb.getConfig();

        String relatedClsName = secKeyMeta.getRelatedEntity();
        if (relatedClsName != null) {
            PrimaryIndex relatedIndex = getRelatedIndex(relatedClsName);
            config.setForeignKeyDatabase(relatedIndex.getDatabase());
        }

        if (config.getTransactional() != priConfig.getTransactional() ||
            DbCompat.getDeferredWrite(config) !=
            DbCompat.getDeferredWrite(priConfig) ||
            config.getReadOnly() != priConfig.getReadOnly()) {
            throw new IllegalArgumentException
                ("One of these properties was changed to be inconsistent" +
                 " with the associated primary database: " +
                 " Transactional, DeferredWrite, ReadOnly");
        }

        PersistKeyBinding keyBinding = getKeyBinding(keyClassName);

        SSecondaryDatabase db = openSecondaryDatabase
            (txn, fileAndDbNames, primaryIndex, 
             secKeyMeta.getKeyName(), config, doNotCreate);
        if (db == null) {
            assert doNotCreate;
            return null;
        }

        InternalSecondaryIndex<SK, PK, E2> secIndex =
            new InternalSecondaryIndex(db, primaryIndex, keyClass, keyBinding,
                                       getKeyCreator(config));

        /* Update index and database maps. */
        secIndexMap.put(secName, secIndex);
        if (DbCompat.getDeferredWrite(config)) {
            deferredWriteDatabases.put(db, null);
        }
        if (priOpenState != null) {
            priOpenState.addDatabase(db);
            priOpenState.addSecondaryName(secName);
        }
        return secIndex;
    }

    /**
     * Open a secondary database, setting AllowCreate, ExclusiveCreate and
     * AllowPopulate appropriately.  We either set all three of these params to
     * true or all to false.  This ensures that we only populate a database
     * when it is created, never if it just happens to be empty.  [#16399]
     *
     * We also handle correction of a bug in duplicate ordering.  See
     * ComplexFormat.incorrectlyOrderedSecKeys.
     *
     * @param doNotCreate is true when StoreConfig.getSecondaryBulkLoad is true
     * and we are opening a secondary as a side effect of opening a primary,
     * i.e., getSecondaryIndex is not being called.  If doNotCreate is true and
     * the database does not exist, we silently ignore the failure to create
     * the DB and return null.  When getSecondaryIndex is subsequently called,
     * the secondary database will be created and populated from the primary --
     * a bulk load.
     */
    private SSecondaryDatabase
        openSecondaryDatabase(final STransaction txn,
                              final String[] fileAndDbNames,
                              final PrimaryIndex priIndex,
                              final String keyName,
                              final SSecondaryConfig config,
                              final boolean doNotCreate)
        throws SDatabaseException {

        assert config.getAllowPopulate();
        assert !config.getExclusiveCreate();
        final SDatabase priDb = priIndex.getDatabase();
        final ComplexFormat entityFormat = (ComplexFormat)
            ((PersistEntityBinding) priIndex.getEntityBinding()).entityFormat;
        final boolean saveAllowCreate = config.getAllowCreate();
        try {
            if (doNotCreate) {
                config.setAllowCreate(false);
            }
            /* First try creating a new database, populate if needed. */
            if (config.getAllowCreate()) {
                config.setExclusiveCreate(true);
                /* AllowPopulate is true; comparators are set. */
                final SSecondaryDatabase db = DbCompat.openSecondaryDatabase
                    (env, txn, fileAndDbNames[0], fileAndDbNames[1], priDb,
                     config);
                if (db != null) {
                    assert !expectFlush;
                    
                    return db;
                }
            }
            /* Next try opening an existing database. */
            config.setAllowCreate(false);
            config.setAllowPopulate(false);
            config.setExclusiveCreate(false);
            
            final SSecondaryDatabase db = DbCompat.openSecondaryDatabase
                (env, txn, fileAndDbNames[0], fileAndDbNames[1], priDb,
                 config);
            return db;
        } finally {
            config.setAllowPopulate(true);
            config.setExclusiveCreate(false);
            config.setAllowCreate(saveAllowCreate);
        }
    }

    /**
     * Checks that all secondary indexes defined in the given entity metadata
     * are already open.  This method is called when a new entity subclass
     * is encountered when an instance of that class is stored.  [#16399]
     *
     * @throws IllegalArgumentException if a secondary is not open.
     */
    synchronized void
        checkEntitySubclassSecondaries(final EntityMetadata entityMeta,
                                       final String subclassName)
        throws SDatabaseException {

        if (storeConfig.getSecondaryBulkLoad()) {
            return;
        }

        final String entityClassName = entityMeta.getClassName();

        for (final SecondaryKeyMetadata secKeyMeta :
             entityMeta.getSecondaryKeys().values()) {
            final String keyName = secKeyMeta.getKeyName();
            final String secName = makeSecName(entityClassName, keyName);
            if (!secIndexMap.containsKey(secName)) {
                throw new IllegalArgumentException
                    ("Entity subclasses defining a secondary key must be " +
                     "registered by calling EntityModel.registerClass or " +
                     "EntityStore.getSubclassIndex before storing an " +
                     "instance of the subclass: " + subclassName);
            }
        }
    }


    public void truncateClass(Class entityClass)
        throws SDatabaseException {

        truncateClass(null, entityClass);
    }

    public synchronized void truncateClass(STransaction txn, Class entityClass)
        throws SDatabaseException {

        checkOpen();
        checkWriteAllowed();

        /* Close primary and secondary databases. */
        closeClass(entityClass);

        String clsName = entityClass.getName();
        EntityMetadata entityMeta = checkEntityClass(clsName);

        boolean autoCommit = false;
        if (txn == null &&
            DbCompat.getThreadTransaction(env) == null) {
            txn = env.beginTransaction(null, autoCommitTxnConfig);
            autoCommit = true;
        }

        /*
         * Truncate the primary first and let any exceptions propogate
         * upwards.  Then remove each secondary, only throwing the first
         * exception.
         */
        boolean success = false;
        try {
            boolean primaryExists =
                truncateIfExists(txn, storePrefix + clsName);
            if (primaryExists) {
                SDatabaseException firstException = null;
                for (SecondaryKeyMetadata keyMeta :
                     entityMeta.getSecondaryKeys().values()) {
                    /* Ignore secondaries that do not exist. */
                    removeIfExists
                        (txn,
                         storePrefix +
                         makeSecName(clsName, keyMeta.getKeyName()));
                }
                if (firstException != null) {
                    throw firstException;
                }
            }
            success = true;
        } finally {
            if (autoCommit) {
                if (success) {
                    txn.commit();
                } else {
                    txn.abort();
                }
            }
        }
    }

    private boolean truncateIfExists(STransaction txn, String dbName)
        throws SDatabaseException {

        String[] fileAndDbNames = parseDbName(dbName);
        return DbCompat.truncateDatabase
            (env, txn, fileAndDbNames[0], fileAndDbNames[1]);
    }

    private boolean removeIfExists(STransaction txn, String dbName)
        throws SDatabaseException {

        String[] fileAndDbNames = parseDbName(dbName);
        return DbCompat.removeDatabase
            (env, txn, fileAndDbNames[0], fileAndDbNames[1]);
    }

    public synchronized void closeClass(Class entityClass)
        throws SDatabaseException {

        checkOpen();
        String clsName = entityClass.getName();
        EntityMetadata entityMeta = checkEntityClass(clsName);

        PrimaryIndex priIndex = priIndexMap.get(clsName);
        if (priIndex != null) {
            /* Close the secondaries first. */
            SDatabaseException firstException = null;
            for (SecondaryKeyMetadata keyMeta :
                 entityMeta.getSecondaryKeys().values()) {

                String secName = makeSecName(clsName, keyMeta.getKeyName());
                SecondaryIndex secIndex = secIndexMap.get(secName);
                if (secIndex != null) {
                    SDatabase db = secIndex.getDatabase();
                    firstException = closeDb(db, firstException);
                    firstException =
                        closeDb(secIndex.getKeysDatabase(), firstException);
                    secIndexMap.remove(secName);
                    deferredWriteDatabases.remove(db);
                }
            }
            /* Close the primary last. */
            SDatabase db = priIndex.getDatabase();
            firstException = closeDb(db, firstException);
            priIndexMap.remove(clsName);
            deferredWriteDatabases.remove(db);

            /* Throw the first exception encountered. */
            if (firstException != null) {
                throw firstException;
            }
        }
    }

    public synchronized void close()
        throws SDatabaseException {

        if (catalog == null) {
            return;
        }

        SDatabaseException firstException = null;
        try {
            if (rawAccess) {
                boolean allClosed = catalog.close();
                assert allClosed;
            } else {
                synchronized (catalogPool) {
                    Map<String, PersistCatalog> catalogMap =
                        catalogPool.get(env);
                    assert catalogMap != null;
                    boolean removeFromCatalog = true;
                    try {
                        removeFromCatalog = catalog.close();
                    } finally {
                        /*
                         * Remove it if the reference count goes to zero, or
                         * when an exception is thrown while closing the db.
                         */
                        if (removeFromCatalog) {
                            catalogMap.remove(storeName);
                        }
                    }
                }
            }
            catalog = null;
        } catch (SDatabaseException e) {
            if (firstException == null) {
                firstException = e;
            }
        }
        for (SSequence seq : sequenceMap.values()) {
            try {
                seq.close();
            } catch (SDatabaseException e) {
                if (firstException == null) {
                    firstException = e;
                }
            }
        }
        firstException = closeDb(sequenceDb, firstException);
        for (SecondaryIndex index : secIndexMap.values()) {
            firstException = closeDb(index.getDatabase(), firstException);
            firstException = closeDb(index.getKeysDatabase(), firstException);
        }
        for (PrimaryIndex index : priIndexMap.values()) {
            firstException = closeDb(index.getDatabase(), firstException);
        }
        if (firstException != null) {
            throw firstException;
        }
    }

    public synchronized SSequence getSequence(String name)
        throws SDatabaseException {

        checkOpen();

        if (storeConfig.getReadOnly()) {
            throw new IllegalStateException("Store is read-only");
        }

        SSequence seq = sequenceMap.get(name);
        if (seq == null) {
            if (sequenceDb == null) {
                String[] fileAndDbNames =
                    parseDbName(storePrefix + SEQUENCE_DB);
                SDatabaseConfig dbConfig = new SDatabaseConfig();
                dbConfig.setAllowCreate(true);
                DbCompat.setTypeBtree(dbConfig);
                sequenceDb = DbCompat.openDatabase
                    (env, null /*txn*/, fileAndDbNames[0], fileAndDbNames[1],
                     dbConfig);
                assert sequenceDb != null;
            }

            SDatabaseEntry entry = new SDatabaseEntry();
            StringBinding.stringToEntry(name, entry);
                seq = sequenceDb.openSequence(null /*txn*/, entry,
                                              getSequenceConfig(name));
            sequenceMap.put(name, seq);
        }
        return seq;
    }

    public synchronized SSequenceConfig getSequenceConfig(String name) {
        checkOpen();
        SSequenceConfig config = sequenceConfigMap.get(name);
        if (config == null) {
            config = new SSequenceConfig();
            config.setInitialValue(1);
            config.setRange(1, Long.MAX_VALUE);
            config.setCacheSize(100);
            config.setAutoCommitNoSync(true);
            config.setAllowCreate(!storeConfig.getReadOnly());
            sequenceConfigMap.put(name, config);
        }
        return config;
    }

    public synchronized void setSequenceConfig(String name,
                                               SSequenceConfig config) {
        checkOpen();
        if (config.getExclusiveCreate() ||
            config.getAllowCreate() == storeConfig.getReadOnly()) {
            throw new IllegalArgumentException
                ("One of these properties was illegally changed: " +
                 "AllowCreate, ExclusiveCreate");
        }
        if (sequenceMap.containsKey(name)) {
            throw new IllegalStateException
                ("Cannot set config after SSequence is open");
        }
        sequenceConfigMap.put(name, config);
    }

    public synchronized SDatabaseConfig getPrimaryConfig(Class entityClass) {
        checkOpen();
        String clsName = entityClass.getName();
        EntityMetadata meta = checkEntityClass(clsName);
        return getPrimaryConfig(meta).cloneConfig();
    }

    private synchronized SDatabaseConfig getPrimaryConfig(EntityMetadata meta) {
        String clsName = meta.getClassName();
        SDatabaseConfig config = priConfigMap.get(clsName);
        if (config == null) {
            config = new SDatabaseConfig();
            config.setAllowCreate(!storeConfig.getReadOnly());
            config.setReadOnly(storeConfig.getReadOnly());
            DbCompat.setTypeBtree(config);
            setBtreeComparator(config, meta.getPrimaryKey().getClassName());
            priConfigMap.put(clsName, config);
        }
        return config;
    }

    public synchronized void setPrimaryConfig(Class entityClass,
                                              SDatabaseConfig config) {
        checkOpen();
        String clsName = entityClass.getName();
        if (priIndexMap.containsKey(clsName)) {
            throw new IllegalStateException
                ("Cannot set config after DB is open");
        }
        EntityMetadata meta = checkEntityClass(clsName);
        SDatabaseConfig dbConfig = getPrimaryConfig(meta);
        if (config.getExclusiveCreate() ||
            config.getAllowCreate() == config.getReadOnly() ||
            config.getSortedDuplicates() ||
            config.getBtreeComparator() != dbConfig.getBtreeComparator()) {
            throw new IllegalArgumentException
                ("One of these properties was illegally changed: " +
                 "AllowCreate, ExclusiveCreate, SortedDuplicates, Temporary " +
                 "or BtreeComparator, ");
        }
        if (!DbCompat.isTypeBtree(config)) {
            throw new IllegalArgumentException("Only type BTREE allowed");
        }
        priConfigMap.put(clsName, config);
    }

    public synchronized SSecondaryConfig getSecondaryConfig(Class entityClass,
                                                           String keyName) {
        checkOpen();
        String entityClsName = entityClass.getName();
        EntityMetadata entityMeta = checkEntityClass(entityClsName);
        SecondaryKeyMetadata secKeyMeta = checkSecKey(entityMeta, keyName);
        String keyClassName = getSecKeyClass(secKeyMeta);
        String secName = makeSecName(entityClass.getName(), keyName);
        return (SSecondaryConfig) getSecondaryConfig
            (secName, entityMeta, keyClassName, secKeyMeta).cloneConfig();
    }

    private SSecondaryConfig getSecondaryConfig(String secName,
                                               EntityMetadata entityMeta,
                                               String keyClassName,
                                               SecondaryKeyMetadata
                                               secKeyMeta) {
        SSecondaryConfig config = secConfigMap.get(secName);
        if (config == null) {
            /* Set common properties to match the primary DB. */
            SDatabaseConfig priConfig = getPrimaryConfig(entityMeta);
            config = new SSecondaryConfig();
            config.setAllowCreate(!priConfig.getReadOnly());
            config.setReadOnly(priConfig.getReadOnly());
            DbCompat.setTypeBtree(config);
            /* Set secondary properties based on metadata. */
            config.setAllowPopulate(true);
            Relationship rel = secKeyMeta.getRelationship();
            config.setSortedDuplicates(rel == Relationship.MANY_TO_ONE ||
                                       rel == Relationship.MANY_TO_MANY);
            setBtreeComparator(config, keyClassName);
            PersistKeyCreator keyCreator = new PersistKeyCreator
                (catalog, entityMeta, keyClassName, secKeyMeta, rawAccess);
            if (rel == Relationship.ONE_TO_MANY ||
                rel == Relationship.MANY_TO_MANY) {
                config.setMultiKeyCreator(keyCreator);
            } else {
                config.setKeyCreator(keyCreator);
            }
            DeleteAction deleteAction = secKeyMeta.getDeleteAction();
            if (deleteAction != null) {
                SForeignKeyDeleteAction baseDeleteAction;
                switch (deleteAction) {
                case ABORT:
                    baseDeleteAction = SForeignKeyDeleteAction.ABORT;
                    break;
                case CASCADE:
                    baseDeleteAction = SForeignKeyDeleteAction.CASCADE;
                    break;
                case NULLIFY:
                    baseDeleteAction = SForeignKeyDeleteAction.NULLIFY;
                    break;
                default:
                    throw DbCompat.unexpectedState(deleteAction.toString());
                }
                config.setForeignKeyDeleteAction(baseDeleteAction);
                if (deleteAction == DeleteAction.NULLIFY) {
                    config.setForeignMultiKeyNullifier(keyCreator);
                }
            }
            secConfigMap.put(secName, config);
        }
        return config;
    }

    public synchronized void setSecondaryConfig(Class entityClass,
                                                String keyName,
                                                SSecondaryConfig config) {
        checkOpen();
        String entityClsName = entityClass.getName();
        EntityMetadata entityMeta = checkEntityClass(entityClsName);
        SecondaryKeyMetadata secKeyMeta = checkSecKey(entityMeta, keyName);
        String keyClassName = getSecKeyClass(secKeyMeta);
        String secName = makeSecName(entityClass.getName(), keyName);
        if (secIndexMap.containsKey(secName)) {
            throw new IllegalStateException
                ("Cannot set config after DB is open");
        }
        SSecondaryConfig dbConfig =
            getSecondaryConfig(secName, entityMeta, keyClassName, secKeyMeta);
        if (config.getExclusiveCreate() ||
            config.getAllowCreate() == config.getReadOnly() ||
            config.getSortedDuplicates() != dbConfig.getSortedDuplicates() ||
            config.getBtreeComparator() != dbConfig.getBtreeComparator() ||
            config.getAllowPopulate() != dbConfig.getAllowPopulate() ||
            config.getKeyCreator() != dbConfig.getKeyCreator() ||
            config.getMultiKeyCreator() != dbConfig.getMultiKeyCreator() ||
            config.getForeignKeyNullifier() !=
                dbConfig.getForeignKeyNullifier() ||
            config.getForeignMultiKeyNullifier() !=
                dbConfig.getForeignMultiKeyNullifier() ||
            config.getForeignKeyDeleteAction() !=
                dbConfig.getForeignKeyDeleteAction() ||
            config.getForeignKeyDatabase() != null) {
            throw new IllegalArgumentException
                ("One of these properties was illegally changed: " +
                 " AllowCreate, ExclusiveCreate, SortedDuplicates," +
                 " BtreeComparator, DuplicateComparator, Temporary," +
                 " AllowPopulate, KeyCreator, MultiKeyCreator," +
                 " ForeignKeyNullifer, ForeignMultiKeyNullifier," +
                 " SForeignKeyDeleteAction, ForeignKeyDatabase");
        }
        if (!DbCompat.isTypeBtree(config)) {
            throw new IllegalArgumentException("Only type BTREE allowed");
        }
        secConfigMap.put(secName, config);
    }

    private static String makeSecName(String entityClsName, String keyName) {
         return entityClsName + NAME_SEPARATOR + keyName;
    }

    static String makePriDbName(String storePrefix, String entityClsName) {
        return storePrefix + entityClsName;
    }

    static String makeSecDbName(String storePrefix,
                                String entityClsName,
                                String keyName) {
        return storePrefix + makeSecName(entityClsName, keyName);
    }

    /**
     * Parses a whole DB name and returns an array of 2 strings where element 0
     * is the file name (always null for JE, always non-null for DB core) and
     * element 1 is the logical DB name (always non-null for JE, may be null
     * for DB core).
     */
    public String[] parseDbName(String wholeName) {
        return parseDbName(wholeName, storeConfig.getDatabaseNamer());
    }

    /**
     * Allows passing a namer to a static method for testing.
     */
    public static String[] parseDbName(String wholeName, DatabaseNamer namer) {
        String[] result = new String[2];
        if (DbCompat.SEPARATE_DATABASE_FILES) {
            String[] splitName = wholeName.split(NAME_SEPARATOR);
            assert splitName.length == 3 || splitName.length == 4 : wholeName;
            assert splitName[0].equals("persist") : wholeName;
            String storeName = splitName[1];
            String clsName = splitName[2];
            String keyName = (splitName.length > 3) ? splitName[3] : null;
            result[0] = namer.getFileName(storeName, clsName, keyName);
            result[1] = null;
        } else {
            result[0] = null;
            result[1] = wholeName;
        }
        return result;
    }

    /**
     * Creates a message identifying the database from the pair of strings
     * returned by parseDbName.
     */
    String getDbNameMessage(String[] names) {
        if (DbCompat.SEPARATE_DATABASE_FILES) {
            return "file: " + names[0];
        } else {
            return "database: " + names[1];
        }
    }

    private void checkOpen() {
        if (catalog == null) {
            throw new IllegalStateException("Store has been closed");
        }
    }

    private void checkWriteAllowed() {
        if (catalog.isReadOnly()) {
            throw new IllegalStateException
                ("Store is read-only or is operating as a Replica");
        }
    }

    private EntityMetadata checkEntityClass(String clsName) {
        EntityMetadata meta = model.getEntityMetadata(clsName);
        if (meta == null) {
            throw new IllegalArgumentException
                ("Class could not be loaded or is not an entity class: " +
                 clsName);
        }
        return meta;
    }

    private SecondaryKeyMetadata checkSecKey(EntityMetadata entityMeta,
                                             String keyName) {
        SecondaryKeyMetadata secKeyMeta =
            entityMeta.getSecondaryKeys().get(keyName);
        if (secKeyMeta == null) {
            throw new IllegalArgumentException
                ("Not a secondary key: " +
                 makeSecName(entityMeta.getClassName(), keyName));
        }
        return secKeyMeta;
    }

    private String getSecKeyClass(SecondaryKeyMetadata secKeyMeta) {
        String clsName = secKeyMeta.getElementClassName();
        if (clsName == null) {
            clsName = secKeyMeta.getClassName();
        }
        return SimpleCatalog.keyClassName(clsName);
    }

    private PersistKeyBinding getKeyBinding(String keyClassName) {
        PersistKeyBinding binding = keyBindingMap.get(keyClassName);
        if (binding == null) {
            binding = new PersistKeyBinding(catalog, keyClassName, rawAccess);
            keyBindingMap.put(keyClassName, binding);
        }
        return binding;
    }

    private PersistKeyCreator getKeyCreator(final SSecondaryConfig config) {
        PersistKeyCreator keyCreator =
            (PersistKeyCreator) config.getKeyCreator();
        if (keyCreator != null) {
            return keyCreator;
        }
        keyCreator = (PersistKeyCreator) config.getMultiKeyCreator();
        assert keyCreator != null;
        return keyCreator;
    }

    private void setBtreeComparator(SDatabaseConfig config, String clsName) {
        if (!rawAccess) {
            PersistKeyBinding binding = getKeyBinding(clsName);
            Format format = binding.keyFormat;
            if (format instanceof CompositeKeyFormat) {
                throw new UnsupportedOperationException(
                    "Composite key with custom sort order is unsupported.");
            }
        }
    }

    private SDatabaseException closeDb(SDatabase db,
                                      SDatabaseException firstException) {
        if (db != null) {
            try {
                db.close();
            } catch (SDatabaseException e) {
                if (firstException == null) {
                    firstException = e;
                }
            }
        }
        return firstException;
    }

    public EvolveStats evolve(EvolveConfig config)
        throws SDatabaseException {

        checkOpen();
        checkWriteAllowed();

        /*
         * Before starting, ensure that we are not in Replica Upgrade Mode and
         * the catalog metadata is not stale.  If this node is a Replica, a
         * ReplicaWriteException will occur further below.
         */
        if (catalog.isReplicaUpgradeMode() || catalog.isMetadataStale(null)) {
            attemptRefresh();
        }

        /* To ensure consistency use a single catalog instance. [#16655] */
        final PersistCatalog useCatalog = catalog;
        List<Format> toEvolve = new ArrayList<Format>();
        Set<String> configToEvolve = config.getClassesToEvolve();
        if (configToEvolve.isEmpty()) {
            useCatalog.getEntityFormats(toEvolve);
        } else {
            for (String name : configToEvolve) {
                Format format = useCatalog.getFormat(name);
                if (format == null) {
                    throw new IllegalArgumentException
                        ("Class to evolve is not persistent: " + name);
                }
                if (!format.isEntity()) {
                    throw new IllegalArgumentException
                        ("Class to evolve is not an entity class: " + name);
                }
                toEvolve.add(format);
            }
        }

        EvolveEvent event = EvolveInternal.newEvent();
        for (Format format : toEvolve) {
            if (format.getEvolveNeeded()) {
                evolveIndex(format, event, config.getEvolveListener());
                format.setEvolveNeeded(false);
                useCatalog.flush(null);
            }
        }

        return event.getStats();
    }

    private void evolveIndex(Format format,
                             EvolveEvent event,
                             EvolveListener listener)
        throws SDatabaseException {

        /* We may make this configurable later. */
        final int WRITES_PER_TXN = 1;

        Class entityClass = format.getType();
        String entityClassName = format.getClassName();
        EntityMetadata meta = model.getEntityMetadata(entityClassName);
        String keyClassName = meta.getPrimaryKey().getClassName();
        keyClassName = SimpleCatalog.keyClassName(keyClassName);
        SDatabaseConfig dbConfig = getPrimaryConfig(meta);

        PrimaryIndex<Object, Object> index = getPrimaryIndex
            (Object.class, keyClassName, entityClass, entityClassName);
        SDatabase db = index.getDatabase();

        EntityBinding binding = index.getEntityBinding();
        SDatabaseEntry key = new SDatabaseEntry();
        SDatabaseEntry data = new SDatabaseEntry();

        SCursorConfig cursorConfig = null;
        STransaction txn = null;
        if (dbConfig.getTransactional()) {
            txn = env.beginTransaction(null, autoCommitTxnConfig);
            cursorConfig = SCursorConfig.READ_COMMITTED;
        }

        SCursor cursor = null;
        int nWritten = 0;
        try {
            cursor = db.openCursor(txn, cursorConfig);
            SOperationStatus status = cursor.getFirst(key, data, null);
            while (status == SOperationStatus.SUCCESS) {
                boolean oneWritten = false;
                if (evolveNeeded(key, data, binding)) {
                    cursor.putCurrent(data);
                    oneWritten = true;
                    nWritten += 1;
                }
                /* Update event stats, even if no listener. [#17024] */
                EvolveInternal.updateEvent
                    (event, entityClassName, 1, oneWritten ? 1 : 0);
                if (listener != null) {
                    if (!listener.evolveProgress(event)) {
                        break;
                    }
                }
                if (txn != null && nWritten >= WRITES_PER_TXN) {
                    cursor.close();
                    cursor = null;
                    txn.commit();
                    txn = null;
                    txn = env.beginTransaction(null, autoCommitTxnConfig);
                    cursor = db.openCursor(txn, cursorConfig);
                    SDatabaseEntry saveKey = KeyRange.copy(key);
                    status = cursor.getSearchKeyRange(key, data, null);
                    if (status == SOperationStatus.SUCCESS &&
                        KeyRange.equalBytes(key, saveKey)) {
                        status = cursor.getNext(key, data, null);
                    }
                } else {
                    status = cursor.getNext(key, data, null);
                }
            }
        } finally {
            if (cursor != null) {
                cursor.close();
            }
            if (txn != null) {
                if (nWritten > 0) {
                    txn.commit();
                } else {
                    txn.abort();
                }
            }
        }
    }

    /**
     * Checks whether the given data is in the current format by translating it
     * to/from an object.  If true is returned, data is updated.
     */
    private boolean evolveNeeded(SDatabaseEntry key,
                                 SDatabaseEntry data,
                                 EntityBinding binding) {
        Object entity = binding.entryToObject(key, data);
        SDatabaseEntry newData = new SDatabaseEntry();
        binding.objectToData(entity, newData);
        if (data.equals(newData)) {
            return false;
        } else {
            byte[] bytes = newData.getData();
            int off = newData.getOffset();
            int size = newData.getSize();
            data.setData(bytes, off, size);
            return true;
        }
    }

    /**
     * For unit testing.
     */
    public static void setSyncHook(SyncHook hook) {
        syncHook = hook;
    }

    /**
     * For unit testing.
     */
    public interface SyncHook {
        void onSync(SDatabase db);
    }

    /**
     * Attempts to refresh metadata and returns whether a refresh occurred.
     * May be called when we expect that updated metadata may be available on
     * disk, and if so could be used to satisfy the user's request.  For
     * example, if an index is requested and not available, we can try a
     * refresh and the check for the index again.
     */
    public boolean attemptRefresh() {
        final PersistCatalog oldCatalog = catalog;
        final PersistCatalog newCatalog =
            refresh(oldCatalog, -1 /*errorFormatId*/, null /*cause*/);
        return oldCatalog != newCatalog;
    }

    /**
     * Called via RefreshException.refresh when handling the RefreshException
     * in the binding methods, when a Replica detects that its in-memory
     * metadata is stale.
     *
     * During refresh, objects that are visible to the user must not be
     * re-created, since the user may have a reference to them.  The
     * PersistCatalog is re-created by this method, and the additional objects
     * listed below are refreshed without creating a new instance.  The
     * refresh() method of non-indented classes is called, and these methods
     * forward the call to indented classes.
     *
     *  PersistCatalog
     *      EntityModel
     *  PrimaryIndex
     *      PersistEntityBinding
     *          PersistKeyAssigner
     *  SecondaryIndex
     *      PersistKeyCreator
     *  PersistKeyBinding
     *
     * These objects have volatile catalog and format fields.  When a refresh
     * in one thread changes these fields, other threads should notice the
     * changes ASAP.  However, it is not necessary that all access to these
     * fields is synchronized.  It is Ok for a mix of old and new fields to be
     * used at any point in time.  If an old object is used after a refresh,
     * the need for a refresh may be detected, causing another call to this
     * method.  In most cases the redundant refresh will be avoided (see check
     * below), but in some cases an extra unnecessary refresh may be performed.
     * This is undesirable, but is not dangerous.  Synchronization must be
     * avoided to prevent blocking during read/write operations.
     *
     * [#16655]
     */
    synchronized PersistCatalog refresh(final PersistCatalog oldCatalog,
                                        final int errorFormatId,
                                        final RefreshException cause) {

        /*
         * While synchronized, check to see whether metadata has already been
         * refreshed.
         */
        if (oldCatalog != catalog) {
            /* Another thread refreshed the metadata -- nothing to do. */
            return catalog;
        }

        /*
         * First refresh the catalog information, then check that the new
         * metadata contains the format ID we're interested in using.
         */
        try {
            catalog = new PersistCatalog(oldCatalog, storePrefix);
        } catch (SDatabaseException e) {
            throw RuntimeExceptionWrapper.wrapIfNeeded(e);
        }

        if (errorFormatId >= catalog.getNFormats()) {
            /* Even with current metadata, the format is out of range. */
            throw DbCompat.unexpectedException
                ("Catalog could not be refreshed, may indicate corruption, " +
                 "errorFormatId=" + errorFormatId + " nFormats=" +
                 catalog.getNFormats() + ", .", cause);
        }

        /*
         * Finally refresh all other objects that directly reference catalog
         * and format objects.
         */
        for (InternalPrimaryIndex index : priIndexMap.values()) {
            index.refresh(catalog);
        }
        for (InternalSecondaryIndex index : secIndexMap.values()) {
            index.refresh(catalog);
        }
        for (PersistKeyBinding binding : keyBindingMap.values()) {
            binding.refresh(catalog);
        }
        for (SSecondaryConfig config : secConfigMap.values()) {
            PersistKeyCreator keyCreator = getKeyCreator(config);
            keyCreator.refresh(catalog);
        }

        return catalog;
    }

    private class InternalPrimaryIndex<PK, E> extends PrimaryIndex<PK, E> {

        private final PersistEntityBinding entityBinding;

        InternalPrimaryIndex(final SDatabase database,
                             final Class<PK> keyClass,
                             final PersistKeyBinding keyBinding,
                             final Class<E> entityClass,
                             final PersistEntityBinding entityBinding)
            throws SDatabaseException {

            super(database, keyClass, keyBinding, entityClass, entityBinding);
            this.entityBinding = entityBinding;
        }

        void refresh(final PersistCatalog newCatalog) {
            entityBinding.refresh(newCatalog);
        }

    }

    private class InternalSecondaryIndex<SK, PK, E>
        extends SecondaryIndex<SK, PK, E> {

        private final PersistKeyCreator keyCreator;

        InternalSecondaryIndex(final SSecondaryDatabase database,
                               final PrimaryIndex<PK, E> primaryIndex,
                               final Class<SK> secondaryKeyClass,
                               final PersistKeyBinding secondaryKeyBinding,
                               final PersistKeyCreator keyCreator)
            throws SDatabaseException {

            super(database, null /*keysDatabase*/, primaryIndex,
                  secondaryKeyClass, secondaryKeyBinding);
            this.keyCreator = keyCreator;
        }

        void refresh(final PersistCatalog newCatalog) {
            keyCreator.refresh(newCatalog);
        }

    }

    STransactionConfig getAutoCommitTxnConfig() {
        return autoCommitTxnConfig;
    }

}
