/*-
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 2002, 2017 Oracle and/or its affiliates.  All rights reserved.
 *
 * $Id$
 */

package com.sleepycat.server.util;

import org.apache.thrift.TEnum;

import java.util.Collection;
import java.util.HashMap;
import java.util.Map;
import java.util.Objects;

/**
 * A TEnumMap is a bi-directional one-to-one map between TEnum and BDB values.
 */
class TEnumMap<T extends TEnum, B> {
    /** The map from TEnum to BDB values. */
    private final Map<T, B> tbMap = new HashMap<>();

    /** The map from BDB to TEnum values. */
    private final Map<B, T> btMap = new HashMap<>();

    public TEnumMap(Collection<Pair<T, B>> pairs) {
        pairs.forEach(this::put);
    }

    private void put(Pair<T, B> pair) {
        if (this.tbMap.containsKey(pair.t) || this.btMap.containsKey(pair.b)) {
            throw new IllegalArgumentException("Duplicate entries.");
        }
        this.tbMap.put(pair.t, pair.b);
        this.btMap.put(pair.b, pair.t);
    }

    public B toBdb(T t) {
        if (!this.tbMap.containsKey(t)) {
            throw new IllegalArgumentException("Unsupported value: " + t);
        }
        return this.tbMap.get(t);
    }

    public T toThrift(B b) {
        if (!this.btMap.containsKey(b)) {
            throw new IllegalArgumentException("Unsupported value: " + b);
        }
        return this.btMap.get(b);
    }

    public static class Pair<T, B> {
        private final T t;
        private final B b;

        public Pair(T t, B b) {
            this.t = Objects.requireNonNull(t);
            this.b = Objects.requireNonNull(b);
        }
    }
}
