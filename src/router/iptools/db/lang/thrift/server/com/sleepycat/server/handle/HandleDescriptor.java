/*-
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 2002, 2017 Oracle and/or its affiliates.  All rights reserved.
 *
 * $Id$
 */

package com.sleepycat.server.handle;

import com.sleepycat.db.DatabaseException;

import java.util.Arrays;
import java.util.Collection;
import java.util.List;
import java.util.Objects;
import java.util.concurrent.ConcurrentLinkedDeque;
import java.util.concurrent.atomic.AtomicBoolean;
import java.util.stream.Collectors;

/**
 * A HandleDescriptor encapsulates all the information needed by a
 * HandleManager to manage a BDB handle.
 */
public abstract class HandleDescriptor<T> extends ReadWriteLockable {
    /** The BDB handle. */
    private final T handle;

    /** The resource key for this handle. */
    private final ResourceKey resourceKey;

    /** A list of parent handles under which this handle is opened. */
    private final List<HandleDescriptor> parents;

    /** A list of open children handles which depend on this handle. */
    private ConcurrentLinkedDeque<HandleDescriptor> children;

    /** The handle manager managing this handle. */
    private HandleManager manager;

    /**
     * The handle id which can be used by clients to remotely reference this
     * handle.
     */
    private long id;

    /** The last time this descriptor's BDB handle is accessed. */
    private volatile long lastAccessTime;

    /** If this descriptor is closed. */
    private AtomicBoolean closed = new AtomicBoolean(false);

    /**
     * Create a HandleDescriptor.
     *
     * @param bdbHandle the BDB handle
     * @param resourceKey the resource key
     * @param parents the parent handles
     */
    protected HandleDescriptor(T bdbHandle, ResourceKey resourceKey,
            HandleDescriptor... parents) {
        this.handle = Objects.requireNonNull(bdbHandle, "bdbHandle is null.");
        this.resourceKey = resourceKey;
        this.children = new ConcurrentLinkedDeque<>();
        this.parents = Arrays.stream(
                Objects.requireNonNull(parents, "parents is null.")).filter(
                p -> p != null).collect(Collectors.toList());

        this.parents.forEach(p -> p.addChild(this));
    }

    public T getHandle() {
        return this.handle;
    }

    protected ResourceKey getResourceKey() {
        return this.resourceKey;
    }

    protected HandleDescriptor[] getParents() {
        return this.parents.toArray(new HandleDescriptor[this.parents.size()]);
    }

    protected final void addChild(HandleDescriptor child) {
        this.children.addFirst(child);
    }

    protected final void removeChild(HandleDescriptor child) {
        this.children.remove(child);
    }

    /* For test only. */
    Collection<HandleDescriptor> getChildren() {
        return this.children;
    }

    /**
     * Return an array of resource keys of which this handle is a member.
     *
     * @return resource keys owning this handle
     */
    public ResourceKey[] resourceOwners() {
        return new ResourceKey[]{this.resourceKey};
    }

    public long getId() {
        return this.id;
    }

    /**
     * Check if this handle is expired. A handle is expired if it has been idle
     * longer than minIdleDuration milliseconds.
     *
     * @param now the current time used to calculate the inactive duration
     * @param minIdleDuration the minimum idle duration in milliseconds
     * @return true if the handle is expired; false, otherwise
     */
    public boolean isExpired(long now, long minIdleDuration) {
        if (now < this.lastAccessTime) {
            throw new IllegalArgumentException(
                    "The handle is accessed in future. Last accessed: " +
                            this.lastAccessTime + ", now: " + now);
        }

        return (now - this.lastAccessTime) > minIdleDuration;
    }

    /* For test only. */
    long getLastAccessTime() {
        return this.lastAccessTime;
    }

    /**
     * Update the last accessed time of this handle and this handle's parent
     * handles.
     * <p>
     * This method should only be called by HandleManager.
     *
     * @param now the new accessed time
     * @return this
     */
    HandleDescriptor touch(long now) {
        this.lastAccessTime = now;
        this.parents.forEach(p -> p.touch(now));
        return this;
    }

    /**
     * Set the manager and the assigned id of this handle.
     * <p>
     * This method should only be called by HandleManager.
     *
     * @param manager the manager managing this handle
     * @param id the assigned id
     */
    void registerManager(HandleManager manager, long id) {
        this.manager = manager;
        this.id = id;
        touch(System.currentTimeMillis());
    }

    /**
     * Close all children handles and then close this handle. When an error
     * occurs, this method continues to close the rest of handles and returns
     * the first error.
     * <p>
     * This method should only be called by HandleManager.
     *
     * @return the first error occurred closing any handles
     */
    DatabaseException close() {
        DatabaseException error = null;
        if (this.closed.compareAndSet(false, true)) {
            error = this.manager.closeHandles(this.children);

            this.parents.forEach(p -> p.removeChild(this));

            try {
                closeBdbHandle();
            } catch (DatabaseException e) {
                if (error == null) {
                    error = e;
                }
            }
        }
        return error;
    }

    /* For test only. */
    boolean isClosed() {
        return this.closed.get();
    }

    /**
     * Close the actual BDB handle of this descriptor.
     *
     * @throws DatabaseException if any error occurs
     */
    protected abstract void closeBdbHandle() throws DatabaseException;

    @Override
    public String toString() {
        return "HandleDescriptor{" +
                "id=" + id +
                ", resourceKey=" + resourceKey +
                '}';
    }
}
