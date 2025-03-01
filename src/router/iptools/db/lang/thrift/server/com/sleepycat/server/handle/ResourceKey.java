/*-
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 2002, 2017 Oracle and/or its affiliates.  All rights reserved.
 *
 * $Id$
 */

package com.sleepycat.server.handle;

/**
 * A ResourceKey is an object that uniquely identifies a resource. A resource
 * is a physical object whose existence is required by some BDB handles. For
 * example, the home directory is the required resource for an environment.
 * <p>
 * A ResourceKey can be used to look up ResourceMembers of a specified
 * resource. Subclasses must guarantee that two ResourceKeys of the same
 * physical resource must be equal.
 */
public interface ResourceKey {
}
