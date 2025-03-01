/*-
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 2002, 2017 Oracle and/or its affiliates.  All rights reserved.
 *
 * $Id$
 */

package com.sleepycat.server.handle;


import com.sleepycat.db.DatabaseException;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import java.util.Collection;
import java.util.List;
import java.util.concurrent.ConcurrentHashMap;
import java.util.concurrent.ConcurrentMap;
import java.util.concurrent.atomic.AtomicBoolean;
import java.util.concurrent.atomic.AtomicLong;
import java.util.function.Consumer;
import java.util.stream.Collectors;

/**
 * A HandleManager is responsible for managing BDB handles on behalf of remote
 * clients.
 */
@SuppressWarnings("ThrowableResultOfMethodCallIgnored")
public class HandleManager {
    private static final Logger logger =
            LoggerFactory.getLogger(HandleManager.class);

    /** The handle id counter. */
    private AtomicLong idCounter = new AtomicLong(0);

    /** The lookup table from handle ids to handle descriptors. */
    private ConcurrentMap<Long, HandleDescriptor> idMap =
            new ConcurrentHashMap<>();

    /** If this manager is shut down. */
    private AtomicBoolean shutdown = new AtomicBoolean(false);

    /** The resource manager. */
    private ResourceMemberManager resourceManager = new ResourceMemberManager();

    /**
     * Register a new handle with this manager and return the registered
     * handle. The registered handle has a new id assigned.
     *
     * @param handle the handle to be registered
     * @return a registered handle
     */
    public HandleDescriptor register(HandleDescriptor handle) {
        long newId;

        do {
            newId = this.idCounter.incrementAndGet();

            // We do not allow id to repeat
            if (newId == 0) {
                throw new IllegalStateException("No new handle id available.");
            }

            handle.registerManager(this, newId);
        } while (this.idMap.putIfAbsent(newId, handle) != null);

        this.resourceManager.register(handle);

        /*
         * If this manager started to shutdown after the handle is opened. We
         * need to close it. This check must be done after putting the handle
         * into the lookup table, so that if shutdown is false we're guaranteed
         * that the put happens before the iterator construction in shutdown().
         */
        if (this.shutdown.get()) {
            closeDescriptor(handle);
        }

        return handle;
    }

    /**
     * Close the handle with the specified id, and recursively close all
     * handles which depend on it.
     * <p>
     * This method acquires a write lock on the handle, so it will wait for
     * other users to finish using the handle and then close it.
     *
     * @param handleId the handle id
     * @throws DatabaseException if any error occurs closing the handle
     */
    public void closeHandle(long handleId) throws DatabaseException {
        closeHandle(handleId, handleDescriptor -> {
        });
    }

    /**
     * Close the handle with the specified id and recursively close all handles
     * which depend on it. This version accepts a callback which is called
     * right
     * before closing this handle.
     * <p>
     * This method acquires a write lock on the handle, so it will wait for
     * other users to finish using the handle and then close it.
     *
     * @param handleId the handle id
     * @param preClose an operation performed before closing this handle
     * @throws DatabaseException if any error occurs closing the handle
     */
    public void closeHandle(long handleId, Consumer<HandleDescriptor> preClose)
            throws DatabaseException {
        HandleDescriptor handle = this.idMap.remove(handleId);
        if (handle != null) {
            preClose.accept(handle);
            DatabaseException e = closeDescriptor(handle);
            if (e != null) {
                throw e;
            }
        } else {
            throw new IllegalArgumentException(
                    "The handle is closed or expired.");
        }
    }

    /**
     * Close a collection of handles, and recursively close all handles which
     * depend on any of them. When an error occurs, this method continues to
     * close the rest of handles and returns the first error.
     * <p>
     * This method acquires a write lock on each handle.
     *
     * @param handles handles to close
     * @return the first error closing any handle
     */
    public DatabaseException closeHandles(
            Collection<HandleDescriptor> handles) {
        DatabaseException error = null;
        for (HandleDescriptor handle : handles) {
            // The handle might have been closed explicitly, check it here
            if (this.idMap.remove(handle.getId()) != null) {
                DatabaseException e = closeDescriptor(handle);
                if (error == null) {
                    error = e;
                }
            }
        }
        return error;
    }

    /**
     * Close handles that have been inactive for at least {@timeoutInSeconds}.
     *
     * @param timeoutInSeconds the timeout threshold in seconds
     */
    public void closeInactiveHandles(int timeoutInSeconds) {
        long now = System.currentTimeMillis();

        List<HandleDescriptor> closeList = this.idMap.values().stream()
                .filter(h -> h.isExpired(now, timeoutInSeconds * 1000L))
                .collect(Collectors.toList());

        DatabaseException e = closeHandles(closeList);
        if (e != null) {
            logger.warn("Failed to close inactive handles.", e);
        }
    }

    private DatabaseException closeDescriptor(HandleDescriptor handle) {
        handle.writeLock();
        try {
            this.idMap.remove(handle.getId());
            this.resourceManager.remove(handle);
            DatabaseException e = handle.close();
            if (e != null) {
                logger.warn("Failed to close handle. " + handle, e);
            }
            return e;
        } finally {
            handle.unlockWrite();
        }
    }

    /**
     * Return the handle with the specified in and acquire a read lock on it.
     * The caller must call unlockHandle() to release the read lock after it
     * finishes using the handle.
     * <p>
     * If no handle is registered with the specified id or the read lock cannot
     * be acquired, then the handle must have been closed or is being closed.
     * Throw IllegalArgumentException in this case.
     * <p>
     * The intention of acquiring a read lock on the handle is to prevent the
     * handle from being closed while the handle is still in use.
     *
     * @param handleId the handle id
     * @return a HandleDescriptor with its read lock acquired
     */
    public HandleDescriptor readLockHandle(long handleId) {
        HandleDescriptor handle = this.idMap.get(handleId);
        if (handle != null && handle.tryReadLock()) {
            try {
                /*
                 * The handle may have been closed before we get the lock.
                 * Test if the handle is still registered.
                 */
                if (this.idMap.containsKey(handleId)) {
                    handle.touch(System.currentTimeMillis());
                    return handle;
                }
            } catch (RuntimeException e) {
                handle.unlockRead();
                throw e;
            }
            handle.unlockRead();
        }

        throw new IllegalArgumentException("The handle is closed or expired.");
    }

    /**
     * Release the read lock on the specified handle.
     *
     * @param handle the read locked handle
     */
    public void unlockHandle(HandleDescriptor handle) {
        if (handle != null) {
            handle.unlockRead();
        }
    }

    /**
     * Return the resource members associated with the specified resource.
     *
     * @param resourceKey the key of the resource
     * @return the ResourceMembers associated with the resource
     */
    public ResourceMembers getResourceMembers(ResourceKey resourceKey) {
        return this.resourceManager.getResourceMembers(resourceKey);
    }

    /**
     * Return the resource members associated with the specified resource and
     * acquire a read lock on it. The caller must call unlockRead() to release
     * the read lock. Return null if the read lock cannot be acquired
     * immediately.
     *
     * @param resourceKey the key of the resource
     * @return the ResourceMembers associated with the resource, or null if the
     * lock cannot be acquired immediately
     */
    public ResourceMembers readLockResource(ResourceKey resourceKey) {
        return this.resourceManager.readLockResource(resourceKey);
    }

    /**
     * Release the read lock on the specified resource members.
     *
     * @param resourceMembers the read locked ResourceMembers.
     */
    public void unlockRead(ResourceMembers resourceMembers) {
        this.resourceManager.unlockRead(resourceMembers);
    }

    /**
     * Return the resource members associated with the specified resource and
     * acquire a write lock on it. The caller must call unlockWrite() to
     * release the write lock.
     *
     * @param resourceKey the key of the resource
     * @return the ResourceMembers associated with the resource
     */
    public ResourceMembers writeLockResource(ResourceKey resourceKey) {
        return this.resourceManager.writeLockResource(resourceKey);
    }

    /**
     * Release the write locked resource members.
     *
     * @param resourceMembers the write locked ResourceMembers
     */
    public void unlockWrite(ResourceMembers resourceMembers) {
        this.resourceManager.unlockWrite(resourceMembers);
    }

    /**
     * Close all handles managed by this manager.
     */
    public void shutdown() {
        if (this.shutdown.compareAndSet(false, true)) {
            closeHandles(this.idMap.values());
        }
    }
}
