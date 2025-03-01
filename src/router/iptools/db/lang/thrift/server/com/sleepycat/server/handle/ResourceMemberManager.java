/*-
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 2002, 2017 Oracle and/or its affiliates.  All rights reserved.
 *
 * $Id$
 */

package com.sleepycat.server.handle;

import java.util.concurrent.ConcurrentHashMap;

/**
 * A ResourceMemberManager is responsible for managing open handles from the
 * resource perspective.
 * <p>
 * This manager groups open handles by their resource owners, and provides
 * methods to synchronize accesses to ResourceMembers.
 * <p>
 * Because a ResourceMembers object is also the read-write lock for its
 * associated resource key. Once a ResourceMembers is created and associated
 * with a resource key, the mapping must never be removed or changed.
 */
class ResourceMemberManager {
    /** The lookup table from resource keys to resource members. */
    private ConcurrentHashMap<ResourceKey, ResourceMembers> resourceMap =
            new ConcurrentHashMap<>();

    /**
     * Register an open handle to this manager.
     *
     * @param handle an open handle
     */
    public void register(HandleDescriptor handle) {
        for (ResourceKey key : handle.resourceOwners()) {
            getResourceMembers(key).add(handle);
        }
    }

    /**
     * Remove a closed handle from this manager.
     *
     * @param handle a closed handle
     */
    public void remove(HandleDescriptor handle) {
        for (ResourceKey key : handle.resourceOwners()) {
            getResourceMembers(key).remove(handle);
        }
    }

    /**
     * Get the ResourceMembers associated with the resource.
     *
     * @param key the resource key
     * @return a ResourceMembers
     */
    public ResourceMembers getResourceMembers(ResourceKey key) {
        if (!this.resourceMap.containsKey(key)) {
            this.resourceMap.putIfAbsent(key, new ResourceMembers());
        }
        return this.resourceMap.get(key);
    }

    /**
     * Return the resource members registered under the resource and acquire a
     * read lock on the ResourceMembers. Return null if the read lock cannot be
     * acquired immediately.
     * <p>
     * The caller must call unlockRead() to release the read lock.
     *
     * @param key the key of the resource
     * @return the ResourceMembers associated with the resource, or null if the
     * read lock cannot be acquired immediately
     */
    public ResourceMembers readLockResource(ResourceKey key) {
        ResourceMembers members = getResourceMembers(key);
        if (members.tryReadLock()) {
            return members;
        } else {
            return null;
        }
    }

    /**
     * Release the read lock on the ResourceMembers.
     *
     * @param resourceMembers the read locked resource members
     */
    public void unlockRead(ResourceMembers resourceMembers) {
        if (resourceMembers != null) {
            resourceMembers.unlockRead();
        }
    }

    /**
     * Return the resource members registered under the resource and acquire a
     * write lock on the ResourceMembers.
     * <p>
     * The caller must call unlockWrite() to release the write lock.
     *
     * @param key the key of the resource
     * @return the ResourceMembers associated with the resource
     */
    public ResourceMembers writeLockResource(ResourceKey key) {
        ResourceMembers members = getResourceMembers(key);
        members.writeLock();
        return members;
    }

    /**
     * Release the write lock on the ResourceMembers.
     *
     * @param resourceMembers the write locked resource members.
     */
    public void unlockWrite(ResourceMembers resourceMembers) {
        if (resourceMembers != null) {
            resourceMembers.unlockWrite();
        }
    }
}
