/*-
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 2002, 2017 Oracle and/or its affiliates.  All rights reserved.
 *
 * $Id$
 */

package com.sleepycat.client;

import com.sleepycat.thrift.TCachePriority;
import com.sleepycat.thrift.TDatabaseConfig;
import com.sleepycat.thrift.TDatabaseType;

import java.util.Comparator;

/**
 * Specify the attributes of a database.
 * <p>
 * For a user created instance, no attribute is set by default. In addition,
 * calling the getter method of an unset attribute results in an
 * IllegalStateException. To set an attribute, call the setter method of the
 * attribute.
 * <p>
 * When used to create a database, system default values are used for
 * unset attributes. When used to modify the configuration of an existing
 * database, only set attributes are modified; unset attributes are not
 * modified.
 */
public class SDatabaseConfig
        extends ThriftWrapper<TDatabaseConfig, TDatabaseConfig._Fields> {

    /**
     * Create an empty SDatabaseConfig with no attribute set.
     */
    public SDatabaseConfig() {
        super(new TDatabaseConfig());
        setBtreeRecordNumbers(false);
        setExclusiveCreate(false);
        setSortedDuplicates(false);
        setUnsortedDuplicates(false);
    }

    /**
     * Create an SDatabaseConfig wrapping a specified thrift object.
     *
     * @param tConfig the thrift config object
     */
    SDatabaseConfig(TDatabaseConfig tConfig) {
        super(tConfig);
    }

    /**
     * Create a deep copy of this configuration object.
     *
     * @return a deep copy of this configuration object
     */
    public SDatabaseConfig cloneConfig() {
        return new SDatabaseConfig(getThriftObj().deepCopy());
    }

    /**
     * Return true if the {@link SEnvironment#openDatabase} method is
     * configured
     * to create the database if it does not already exist.
     * <p>
     *
     * @return true if the {@link SEnvironment#openDatabase} method is
     * configured to create the database if it does not already exist
     */
    public boolean getAllowCreate() {
        return (boolean) getField(TDatabaseConfig._Fields.ALLOW_CREATE);
    }

    /**
     * Configure the {@link SEnvironment#openDatabase}  method to create
     * the database if it does not already exist.
     *
     * @param allowCreate if true, configure the {@link SEnvironment#openDatabase}
     * method to create the database if it does not already exist.
     * @return this
     */
    public SDatabaseConfig setAllowCreate(final boolean allowCreate) {
        getThriftObj().setAllowCreate(allowCreate);
        return this;
    }

    /**
     * Return the threshold value in bytes beyond which data items are
     * stored as blobs.
     *
     * @return the threshold value in bytes beyond which data items are
     * stored as blobs; if 0, blob is not used by the database
     */
    public int getBlobThreshold() {
        return (int) getField(TDatabaseConfig._Fields.BLOB_THRESHOLD);
    }

    /**
     * Set the size in bytes which is used to determine when a data item will
     * be stored as a blob.
     * <p>
     * Any data item that is equal to or larger in size than the threshold
     * value will automatically be stored as a blob.
     * <p>
     * It is illegal to enable blob in the database which is configured with
     * chksum, encryption, duplicates, sorted duplicates, compression,
     * multiversion concurrency control and transactional read operations with
     * degree 1 isolation.
     * <p>
     * This threshold value can not be set after opening the database.
     *
     * @param value The size in bytes which is used to determine when a data
     * item will be stored as a blob; if 0, blob will be never used by the
     * database.
     * @return this
     */
    public SDatabaseConfig setBlobThreshold(int value) {
        getThriftObj().setBlobThreshold(value);
        return this;
    }

    /**
     * Always return null. For compatibility with DPL APIs.
     *
     * @return always null
     */
    public Comparator<byte[]> getBtreeComparator() {
        return null;
    }

    /**
     * Return the minimum number of key/data pairs intended to be stored
     * on any single Btree leaf page.
     * <p>
     *
     * @return the minimum number of key/data pairs intended to be stored
     * on any single Btree leaf page
     */
    public int getBtreeMinKey() {
        return (int) getField(TDatabaseConfig._Fields.BTREE_MIN_KEY);
    }

    /**
     * Set the minimum number of key/data pairs intended to be stored on any
     * single Btree leaf page.
     * <p>
     * This value is used to determine if key or data items will be stored
     * on overflow pages instead of Btree leaf pages.  The value must be
     * at least 2; if the value is not explicitly set, a value of 2 is used.
     * <p>
     * This method configures a database, not only operations performed using
     * the specified handle.
     * <p>
     * This attribute cannot be changed after the database is created. If
     * opening an existing database, this attribute is ignored.
     *
     * @param btMinKey The minimum number of key/data pairs intended to be
     * stored on any single Btree leaf page.
     * @return this
     */
    public SDatabaseConfig setBtreeMinKey(final int btMinKey) {
        getThriftObj().setBtreeMinKey(btMinKey);
        return this;
    }

    /**
     * Return true if the Btree is configured to support retrieval by record
     * number.
     *
     * @return true if the Btree is configured to support retrieval by record
     * number
     */
    public boolean getBtreeRecordNumbers() {
        return (boolean) getField(TDatabaseConfig._Fields.BTREE_RECORD_NUMBERS);
    }

    /**
     * Configure the Btree to support retrieval by record number.
     * <p>
     * Logical record numbers in Btree databases are mutable in the face of
     * record insertion or deletion.
     * <p>
     * Maintaining record counts within a Btree introduces a serious point
     * of contention, namely the page locations where the record counts are
     * stored.  In addition, the entire database must be locked during both
     * insertions and deletions, effectively single-threading the database
     * for those operations.  Configuring a Btree for retrieval by record
     * number can result in serious performance degradation for some
     * applications and data sets.
     * <p>
     * Retrieval by record number may not be configured for a Btree that also
     * supports duplicate data items.
     * <p>
     * This method configures a database, not only operations performed using
     * the specified handle.
     * <p>
     * If the database already exists when the database is opened, any database
     * configuration specified by this method must be the same as the existing
     * database or an error will be returned.
     *
     * @param btreeRecordNumbers if true, configure the Btree to support
     * retrieval by record number.
     * @return this
     */
    public SDatabaseConfig setBtreeRecordNumbers(
            final boolean btreeRecordNumbers) {
        getThriftObj().setBtreeRecordNumbers(btreeRecordNumbers);
        return this;
    }

    /**
     * Return true if the database environment is configured to do checksum
     * verification of pages read into the cache from the backing filestore.
     *
     * @return true if the database environment is configured to do checksum
     * verification of pages read into the cache from the backing filestore
     */
    public boolean getChecksum() {
        return (boolean) getField(TDatabaseConfig._Fields.CHECKSUM);
    }

    /**
     * Configure the database environment to do checksum verification of
     * pages read into the cache from the backing filestore.
     * <p>
     * Berkeley DB uses the SHA1 Secure Hash Algorithm if encryption is
     * also configured for this database, and a general hash algorithm if
     * it is not.
     * <p>
     * Calling this method only affects the specified {@link SDatabase} handle
     * (and any other library handles opened within the scope of that handle).
     * <p>
     * If the database already exists when the database is opened, any database
     * configuration specified by this method will be ignored.
     * If creating additional databases in a file, the checksum behavior
     * specified must be consistent with the existing databases in the file or
     * an error will be returned.
     *
     * @param checksum if true, configure the database environment to do
     * checksum verification of pages read into the cache from the backing
     * filestore
     * @return this
     */
    public SDatabaseConfig setChecksum(final boolean checksum) {
        getThriftObj().setChecksum(checksum);
        return this;
    }

    /**
     * Return true if the {@link SEnvironment#openDatabase} method is
     * configured to fail if the database already exists.
     *
     * @return true if the {@link SEnvironment#openDatabase} method is
     * configured to fail if the database already exists
     */
    public boolean getExclusiveCreate() {
        return (boolean) getField(TDatabaseConfig._Fields.EXCLUSIVE_CREATE);
    }

    /**
     * Configure the {@link SEnvironment#openDatabase} method to fail if
     * the database already exists.
     * <p>
     * The exclusiveCreate mode is only meaningful if specified with the
     * allowCreate mode.
     *
     * @param exclusiveCreate if true, configure the {@link
     * SEnvironment#openDatabase} method to fail if the database already exists
     * @return this
     */
    public SDatabaseConfig setExclusiveCreate(final boolean exclusiveCreate) {
        getThriftObj().setExclusiveCreate(exclusiveCreate);
        return this;
    }

    /**
     * Return the hash table density.
     *
     * @return the hash table density.
     */
    public int getHashFillFactor() {
        return (int) getField(TDatabaseConfig._Fields.HASH_FILL_FACTOR);
    }

    /**
     * Set the desired density within the hash table.
     * <p>
     * If no value is specified, the fill factor will be selected dynamically
     * as pages are filled.
     * <p>
     * The density is an approximation of the number of keys allowed to
     * accumulate in any one bucket, determining when the hash table grows or
     * shrinks.  If you know the average sizes of the keys and data in your
     * data set, setting the fill factor can enhance performance.  A reasonable
     * rule computing fill factor is to set it to the following:
     * <pre>
     *  (pagesize - 32) / (average_key_size + average_data_size + 8)
     * </pre>
     * <p>
     * This method configures a database, not only operations performed using
     * the specified handle.
     * <p>
     * This attribute cannot be changed after the database is created. If
     * opening an existing database, this attribute is ignored.
     *
     * @param hashFillFactor the desired density within the hash table
     * @return this
     */
    public SDatabaseConfig setHashFillFactor(final int hashFillFactor) {
        getThriftObj().setHashFillFactor(hashFillFactor);
        return this;
    }

    /**
     * Return the estimate of the final size of the hash table.
     *
     * @return the estimate of the final size of the hash table
     */
    public int getHashNumElements() {
        return (int) getField(TDatabaseConfig._Fields.HASH_NUM_ELEMENTS);
    }

    /**
     * Set an estimate of the final size of the hash table.
     * <p>
     * In order for the estimate to be used when creating the database, the
     * {@link #setHashFillFactor} method must also be called. If the estimate
     * or fill factor are not set or are set too low, hash tables will still
     * expand gracefully as keys are entered, although a slight performance
     * degradation may be noticed.
     * <p>
     * This method configures a database, not only operations performed using
     * the specified handle.
     * <p>
     * This attribute cannot be changed after the database is created. If
     * opening an existing database, this attribute is ignored.
     *
     * @param hashNumElements an estimate of the final size of the hash table.
     * @return this
     */
    public SDatabaseConfig setHashNumElements(final int hashNumElements) {
        getThriftObj().setHashNumElements(hashNumElements);
        return this;
    }

    /**
     * Return the maximum on-disk database file size.
     *
     * @return the maximum on-disk database file size
     */
    public long getHeapsize() {
        return (long) getField(TDatabaseConfig._Fields.HEAPSIZE);
    }

    /**
     * Set the maximum on-disk database file size used by a database configured
     * to use the Heap access method. If this method is never called, the
     * database's file size can grow without bound. If this method is called,
     * then the heap file can never grow larger than the limit defined by this
     * method. In that case, attempts to update or create records in a Heap
     * database that has reached its maximum size will throw a {@link
     * SHeapFullException}.
     * <p>
     * The size specified to this method must be at least three times the
     * database page size. That is, a Heap database must contain at least three
     * database pages. You can set the database page size using {@link
     * #setPageSize}.
     * <p>
     * This method may not be called after the database is opened. Further, if
     * this method is called on an existing Heap database, the size specified
     * here must match the size used to create the database. Note, however,
     * that specifying an incorrect size to this method will not result in an
     * error return until the database is opened.
     *
     * @param bytes the maximum on-disk database file size
     * @return this
     */
    public SDatabaseConfig setHeapsize(final long bytes) {
        getThriftObj().setHeapsize(bytes);
        return this;
    }

    /**
     * Return the number of pages in a region of the database.
     *
     * @return the size of the region, in pages
     */
    public int getHeapRegionSize() {
        return (int) getField(TDatabaseConfig._Fields.HEAP_REGION_SIZE);
    }

    /**
     * Sets the number of pages in a region of a database configured to use
     * the Heap access method. If this method is never called, the default
     * region size for the database's page size will be used. You can set the
     * database page size using {@link #setPageSize}.
     * <p>
     * This method may not be called after the database is opened. Further, if
     * this method is called on an existing Heap database, the value specified
     * here will be ignored. If the specified region size is larger than the
     * maximum region size for the database's page size, an error will be
     * returned when the database is opened. The maximum allowable region size
     * will be included in the error message.
     *
     * @param npages The size of the region, in pages.
     * @return this
     */
    public SDatabaseConfig setHeapRegionSize(final int npages) {
        getThriftObj().setHeapRegionSize(npages);
        return this;
    }

    /**
     * Return true if the database is configured for multiversion concurrency
     * control.
     *
     * @return true if the database is configured for multiversion concurrency
     * control
     */
    public boolean getMultiversion() {
        return (boolean) getField(TDatabaseConfig._Fields.MULTIVERSION);
    }

    /**
     * Configured the database with support for multiversion concurrency
     * control. This will cause updates to the database to follow a
     * copy-on-write protocol, which is required to support Snapshot Isolation.
     * See {@link STransactionConfig#setSnapshot}) for more information.
     * Multiversion access is not supported for queue databases.
     *
     * @param multiversion if true, configure the database with support for
     * multiversion concurrency control.
     * @return this
     */
    public SDatabaseConfig setMultiversion(final boolean multiversion) {
        getThriftObj().setMultiversion(multiversion);
        return this;
    }

    /**
     * Return whether the {@link SDatabase} handle is configured to obtain a
     * write lock on the entire database.
     *
     * @return True if the {@link SDatabase} handle is configured for immediate
     * exclusive database locking. In this case, the locking operation will
     * error out if it cannot immediately obtain an exclusive lock. False if
     * the {@link SDatabase} handle is configured for exclusive database
     * locking. In this case, it will block until it can obtain the exclusive
     * database lock when database is opened. Null if the {@link SDatabase}
     * handle is not configured for exclusive locking.
     */
    public Boolean getNoWaitDbExclusiveLock() {
        TDatabaseConfig tConfig = this.getThriftObj();
        return tConfig.isSetNoWaitDbExclusiveLock() ? tConfig
                .isNoWaitDbExclusiveLock() : null;
    }

    /**
     * Configure the {@link SDatabase} handle to obtain a write lock on the
     * entire database.
     * <p>
     * Handles configured for a write lock on the entire database can only have
     * one active transaction at a time.
     *
     * @param noWaitDbExclLock If True, configure the {@link SDatabase} handle
     * to obtain a write lock on the entire database. When the database is
     * opened it will immediately throw {@link SLockNotGrantedException} if it
     * cannot obtain the exclusive lock immediately. If False, configure the
     * {@link SDatabase} handle to obtain a write lock on the entire database.
     * When the database is opened, it will block until it can obtain the
     * exclusive lock. If null, do not configure the {@link SDatabase} handle
     * to obtain a write lock on the entire database.
     * @return this
     */
    public SDatabaseConfig setNoWaitDbExclusiveLock(Boolean noWaitDbExclLock) {
        getThriftObj().setFieldValue(
                TDatabaseConfig._Fields.NO_WAIT_DB_EXCLUSIVE_LOCK,
                noWaitDbExclLock);
        return this;
    }

    /**
     * Return the size of the pages used to hold items in the database, in
     * bytes.
     *
     * @return the size of the pages used to hold items in the database, in
     * bytes.
     */
    public int getPageSize() {
        return (int) getField(TDatabaseConfig._Fields.PAGE_SIZE);
    }

    /**
     * Set the size of the pages used to hold items in the database, in bytes.
     * <p>
     * The minimum page size is 512 bytes, the maximum page size is 64K bytes,
     * and the page size must be a power-of-two.  If the page size is not
     * explicitly set, one is selected based on the underlying filesystem I/O
     * block size.  The automatically selected size has a lower limit of 512
     * bytes and an upper limit of 16K bytes.
     * <p>
     * This method configures a database, not only operations performed using
     * the specified handle.
     * <p>
     * This method may not be called after the database is opened. If the
     * database already exists when it is opened, the information specified to
     * this method will be ignored. If creating additional databases in a file,
     * the page size specified must be consistent with the existing databases
     * in the file or an error will be returned.
     *
     * @param pageSize the size of the pages used to hold items in the
     * database, in bytes.
     * @return this
     */
    public SDatabaseConfig setPageSize(final int pageSize) {
        getThriftObj().setPageSize(pageSize);
        return this;
    }

    /**
     * Return the cache priority for pages referenced by this handle.
     *
     * @return the cache priority for pages referenced by this handle.
     */
    public SCacheFilePriority getPriority() {
        return SCacheFilePriority.toBdb((TCachePriority) getField(
                TDatabaseConfig._Fields.PRIORITY));
    }

    /**
     * Set the cache priority for pages referenced by the DB handle.
     * <p>
     * The priority of a page biases the replacement algorithm to be more or
     * less likely to discard a page when space is needed in the buffer pool.
     * The bias is temporary, and pages will eventually be discarded if they
     * are
     * not referenced again. The priority setting is only advisory, and does
     * not guarantee pages will be treated in a specific way.
     *
     * @param priority the desired cache priority.
     * @return this
     */
    public SDatabaseConfig setPriority(final SCacheFilePriority priority) {
        getThriftObj().setPriority(SCacheFilePriority.toThrift(priority));
        return this;
    }

    /**
     * Return the size of the extents used to hold pages in a Queue database,
     * specified as a number of pages.
     *
     * @return the size of the extents used to hold pages in a Queue database,
     * specified as a number of pages.
     */
    public int getQueueExtentSize() {
        return (int) getField(TDatabaseConfig._Fields.QUEUE_EXTENT_SIZE);
    }

    /**
     * Set the size of the extents used to hold pages in a Queue database,
     * specified as a number of pages.
     * <p>
     * Each extent is created as a separate physical file.  If no extent
     * size is set, the default behavior is to create only a single
     * underlying database file.
     * <p>
     * This method configures a database, not only operations performed using
     * the specified handle.
     * <p>
     * This method may not be called after the database is opened. If the
     * database already exists when it is opened, the information specified to
     * this method will be ignored.
     *
     * @param queueExtentSize The number of pages in a Queue database extent.
     * @return this
     */
    public SDatabaseConfig setQueueExtentSize(final int queueExtentSize) {
        getThriftObj().setQueueExtentSize(queueExtentSize);
        return this;
    }

    /**
     * Return true if the database is configured in read-only mode.
     *
     * @return true if the database is configured in read-only mode
     */
    public boolean getReadOnly() {
        return (boolean) getField(TDatabaseConfig._Fields.READ_ONLY);
    }

    /**
     * Configure the database in read-only mode.
     * <p>
     * Any attempt to modify items in the database will fail, regardless
     * of the actual permissions of any underlying files.
     *
     * @param readOnly if true, configure the database in read-only mode.
     * @return this
     */
    public SDatabaseConfig setReadOnly(final boolean readOnly) {
        getThriftObj().setReadOnly(readOnly);
        return this;
    }

    /**
     * Return true if the database is configured to support read uncommitted.
     *
     * @return true if the database is configured to support read uncommitted.
     */
    public boolean getReadUncommitted() {
        return (boolean) getField(TDatabaseConfig._Fields.READ_UNCOMMITTED);
    }

    /**
     * Configure the database to support read uncommitted.
     * <p>
     * Read operations on the database may request the return of modified
     * but not yet committed data.  This flag must be specified on all
     * {@link SDatabase} handles used to perform read uncommitted or database
     * updates, otherwise requests for read uncommitted may not be honored and
     * the read may block.
     *
     * @param readUncommitted if true, configure the database to support read
     * uncommitted.
     * @return this
     */
    public SDatabaseConfig setReadUncommitted(final boolean readUncommitted) {
        getThriftObj().setReadUncommitted(readUncommitted);
        return this;
    }

    /**
     * Return the database record length, in bytes.
     *
     * @return the database record length, in bytes.
     */
    public int getRecordLength() {
        return (int) getField(TDatabaseConfig._Fields.RECORD_LENGTH);
    }

    /**
     * Specify the database record length, in bytes.
     * <p>
     * For the Queue access method, specify the record length.  For the
     * Queue access method, the record length must be enough smaller than
     * the database's page size that at least one record plus the database
     * page's metadata information can fit on each database page.
     * <p>
     * For the Recno access method, specify the records are fixed-length,
     * not byte-delimited, and are of length {@code recordLength}.
     * <p>
     * Any records added to the database that are less than the specified
     * length are automatically padded (see {@link #setRecordPad} for more
     * information).
     * <p>
     * Any attempt to insert records into the database that are greater
     * than the specified length will cause the call to fail.
     * <p>
     * This method configures a database, not only operations performed using
     * the specified handle.
     * <p>
     * This method may not be called after the database is opened. If the
     * database already exists when it is opened, the information specified to
     * this method will be ignored.
     *
     * @param recordLength the database record length, in bytes.
     * @return this
     */
    public SDatabaseConfig setRecordLength(final int recordLength) {
        getThriftObj().setRecordLength(recordLength);
        return this;
    }

    /**
     * Return the padding character for short, fixed-length records for the
     * Queue and Recno access methods.
     *
     * @return the padding character for short, fixed-length records for the
     * Queue and Recno access methods.
     */
    public int getRecordPad() {
        return (int) getField(TDatabaseConfig._Fields.RECORD_PAD);
    }

    /**
     * Set the padding character for short, fixed-length records for the Queue
     * and Recno access methods.
     * <p>
     * If no pad character is specified, "space" characters (that is, ASCII
     * 0x20) are used for padding.
     * <p>
     * This method configures a database, not only operations performed using
     * the specified handle.
     * <p>
     * This method may not be called after the database is opened.
     * If the database already exists when it is opened,
     * the information specified to this method will be ignored.
     *
     * @param recordPad the padding character for short, fixed-length records
     * for the Queue and Recno access methods.
     * @return this
     */
    public SDatabaseConfig setRecordPad(final int recordPad) {
        getThriftObj().setRecordPad(recordPad);
        return this;
    }

    /**
     * Return true if the logical record numbers are mutable, and change as
     * records are added to and deleted from the database.
     *
     * @return true if the logical record numbers are mutable, and change as
     * records are added to and deleted from the database.
     */
    public boolean getRenumbering() {
        return (boolean) getField(TDatabaseConfig._Fields.RENUMBERING);
    }

    /**
     * Configure the logical record numbers to be mutable, and change as
     * records are added to and deleted from the database.
     * <p>
     * For example, the deletion of record number 4 causes records numbered
     * 5 and greater to be renumbered downward by one.  If a cursor was
     * positioned to record number 4 before the deletion, it will refer to
     * the new record number 4, if any such record exists, after the
     * deletion.  If a cursor was positioned after record number 4 before
     * the deletion, it will be shifted downward one logical record,
     * continuing to refer to the same record as it did before.
     * <p>
     * Creating new records will cause the creation of multiple records if
     * the record number is more than one greater than the largest record
     * currently in the database.  For example, creating record 28, when
     * record 25 was previously the last record in the database, will
     * create records 26 and 27 as well as 28.  Attempts to retrieve
     * records that were created in this manner will result in an error
     * return of {@link SOperationStatus#KEYEMPTY}.
     * <p>
     * If a created record is not at the end of the database, all records
     * following the new record will be automatically renumbered upward by one.
     * For example, the creation of a new record numbered 8 causes records
     * numbered 8 and greater to be renumbered upward by one.  If a cursor was
     * positioned to record number 8 or greater before the insertion, it will
     * be shifted upward one logical record, continuing to refer to the same
     * record as it did before.
     * <p>
     * For these reasons, concurrent access to a Recno database configured
     * with mutable record numbers may be largely meaningless, although it
     * is supported.
     * <p>
     * Calling this method affects the database, including all threads of
     * control accessing the database.
     * <p>
     * If the database already exists when the database is opened, any database
     * configuration specified by this method must be the same as the existing
     * database or an error will be returned.
     *
     * @param renumbering if true, configure the logical record numbers to be
     * mutable, and change as records are added to and deleted from the
     * database.
     * @return this
     */
    public SDatabaseConfig setRenumbering(final boolean renumbering) {
        getThriftObj().setRenumbering(renumbering);
        return this;
    }

    /**
     * Return true if the Btree has been configured to not do reverse splits.
     *
     * @return true if the Btree has been configured to not do reverse splits.
     */
    public boolean getReverseSplitOff() {
        return (boolean) getField(TDatabaseConfig._Fields.REVERSE_SPLIT_OFF);
    }

    /**
     * Configure the Btree to not do reverse splits.
     * <p>
     * As pages are emptied in a database, the Btree implementation
     * attempts to coalesce empty pages into higher-level pages in order
     * to keep the database as small as possible and minimize search time.
     * This can hurt performance in applications with cyclical data
     * demands; that is, applications where the database grows and shrinks
     * repeatedly.  For example, because Berkeley DB does page-level locking,
     * the maximum level of concurrency in a database of two pages is far
     * smaller than that in a database of 100 pages, so a database that has
     * shrunk to a minimal size can cause severe deadlocking when a new
     * cycle of data insertion begins.
     * <p>
     * Calling this method only affects the specified {@link SDatabase} handle
     * (and any other library handles opened within the scope of that handle).
     *
     * @param reverseSplitOff if true, configure the Btree to not do reverse
     * splits.
     * @return this
     */
    public SDatabaseConfig setReverseSplitOff(final boolean reverseSplitOff) {
        getThriftObj().setReverseSplitOff(reverseSplitOff);
        return this;
    }

    /**
     * Return true if the database is configured to support sorted duplicate
     * data items.
     *
     * @return true if the database is configured to support sorted duplicate
     * data items
     */
    public boolean getSortedDuplicates() {
        return (boolean) getField(TDatabaseConfig._Fields.SORTED_DUPLICATES);
    }

    /**
     * Configure the database to support sorted, duplicate data items.
     * <p>
     * Insertion when the key of the key/data pair being inserted already
     * exists in the database will be successful.  The ordering of
     * <p>
     * If a primary database is to be associated with one or more secondary
     * databases, it may not be configured for duplicates.
     * <p>
     * A Btree that supports duplicate data items cannot also be configured
     * for retrieval by record number.
     * <p>
     * Calling this method affects the database, including all threads of
     * control accessing the database.
     * <p>
     * If the database already exists when the database is opened, any database
     * configuration specified by this method must be the same as the existing
     * database or an error will be returned.
     *
     * @param sortedDuplicates if true, configure the database to support
     * duplicate data items
     * @return this
     */
    public SDatabaseConfig setSortedDuplicates(final boolean sortedDuplicates) {
        getThriftObj().setSortedDuplicates(sortedDuplicates);
        return this;
    }

    /**
     * Return true if the database is configured to support duplicate data
     * items.
     *
     * @return true if the database is configured to support duplicate data
     * items
     */
    public boolean getUnsortedDuplicates() {
        return (boolean) getField(TDatabaseConfig._Fields.UNSORTED_DUPLICATES);
    }

    /**
     * Configure the database to support unsorted duplicate data items.
     * <p>
     * Insertion when the key of the key/data pair being inserted already
     * exists in the database will be successful.  The ordering of
     * duplicates in the database is determined by the order of insertion,
     * unless the ordering is otherwise specified by use of a database
     * cursor operation.
     * <p>
     * If a primary database is to be associated with one or more secondary
     * databases, it may not be configured for duplicates.
     * <p>
     * Sorted duplicates are preferred to unsorted duplicates for
     * performance reasons.  Unsorted duplicates should only be used by
     * applications wanting to order duplicate data items manually.
     * <p>
     * Calling this method affects the database, including all threads of
     * control accessing the database.
     * <p>
     * If the database already exists when the database is opened, any database
     * configuration specified by this method must be the same as the existing
     * database or an error will be returned.
     *
     * @param unsortedDuplicates if true, configure the database to support
     * unsorted duplicate data items
     * @return this
     */
    public SDatabaseConfig setUnsortedDuplicates(
            final boolean unsortedDuplicates) {
        getThriftObj().setUnsortedDuplicates(unsortedDuplicates);
        return this;
    }

    /**
     * Always return true. For compatibility with DPL APIs.
     *
     * @return always true
     */
    public boolean getTransactional() {
        return true;
    }

    /**
     * Return true if the database environment is configured to not write log
     * records for this database.
     *
     * @return true if the database environment is configured to not write log
     * records for this database
     */
    public boolean getTransactionNotDurable() {
        return (boolean) getField(
                TDatabaseConfig._Fields.TRANSACTION_NOT_DURABLE);
    }

    /**
     * Configure the database environment to not write log records for this
     * database.
     * <p>
     * This means that updates of this database exhibit the ACI (atomicity,
     * consistency, and isolation) properties, but not D (durability); that
     * is, database integrity will be maintained, but if the server fails,
     * integrity will not persist.  The database file must be verified and/or
     * restored from backup after a failure.  In order to ensure integrity
     * after server shut down, all database changes must be flushed from the
     * database environment cache using {@link SEnvironment#checkpoint}.
     * <p>
     * All database handles for a single physical file must call this method,
     * including database handles for different databases in a physical file.
     * <p>
     * Calling this method only affects the specified {@link SDatabase} handle
     * (and any other library handles opened within the scope of that handle).
     *
     * @param transactionNotDurable if true, configure the database environment
     * to not write log records for this database
     * @return this
     */
    public SDatabaseConfig setTransactionNotDurable(
            final boolean transactionNotDurable) {
        getThriftObj().setTransactionNotDurable(transactionNotDurable);
        return this;
    }

    /**
     * Return the type of the database.
     * <p>
     * This method may be used to determine the type of the database after
     * opening it.
     *
     * @return the type of the database
     */
    public SDatabaseType getType() {
        return SDatabaseType.toBdb(
                (TDatabaseType) getField(TDatabaseConfig._Fields.TYPE));
    }

    /**
     * Configure the type of the database.
     *
     * @param type the type of the database.
     * @return this
     */
    public SDatabaseConfig setType(final SDatabaseType type) {
        getThriftObj().setType(SDatabaseType.toThrift(type));
        return this;
    }
}
