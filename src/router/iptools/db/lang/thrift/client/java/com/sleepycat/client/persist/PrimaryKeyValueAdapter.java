/*-
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 2002, 2017 Oracle and/or its affiliates.  All rights reserved.
 *
 */

package com.sleepycat.client.persist;

import com.sleepycat.client.bind.EntryBinding;
import com.sleepycat.client.SDatabaseEntry;

/**
 * A ValueAdapter where the "value" is the primary key.
 *
 * @author Mark Hayes
 */
class PrimaryKeyValueAdapter<V> implements ValueAdapter<V> {

    private EntryBinding keyBinding;

    PrimaryKeyValueAdapter(Class<V> keyClass, EntryBinding keyBinding) {
        this.keyBinding = keyBinding;
    }

    public SDatabaseEntry initKey() {
        return new SDatabaseEntry();
    }

    public SDatabaseEntry initPKey() {
        return new SDatabaseEntry();
    }

    public SDatabaseEntry initData() {
        return BasicIndex.NO_RETURN_ENTRY;
    }

    public void clearEntries(SDatabaseEntry key,
                             SDatabaseEntry pkey,
                             SDatabaseEntry data) {
        key.setData(null);
        pkey.setData(null);
    }

    public V entryToValue(SDatabaseEntry key,
                          SDatabaseEntry pkey,
                          SDatabaseEntry data) {
        return (V) keyBinding.entryToObject(pkey);
    }

    public void valueToData(V value, SDatabaseEntry data) {
        throw new UnsupportedOperationException
            ("Cannot change the data in a key-only index");
    }
}
