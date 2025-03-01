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
 * A ValueAdapter where the "value" is the data, although the data in this case
 * is the primary key in a KeysIndex.
 *
 * @author Mark Hayes
 */
class DataValueAdapter<V> implements ValueAdapter<V> {

    private EntryBinding dataBinding;

    DataValueAdapter(Class<V> keyClass, EntryBinding dataBinding) {
        this.dataBinding = dataBinding;
    }

    public SDatabaseEntry initKey() {
        return new SDatabaseEntry();
    }

    public SDatabaseEntry initPKey() {
        return null;
    }

    public SDatabaseEntry initData() {
        return new SDatabaseEntry();
    }

    public void clearEntries(SDatabaseEntry key,
                             SDatabaseEntry pkey,
                             SDatabaseEntry data) {
        key.setData(null);
        data.setData(null);
    }

    public V entryToValue(SDatabaseEntry key,
                          SDatabaseEntry pkey,
                          SDatabaseEntry data) {
        return (V) dataBinding.entryToObject(data);
    }

    public void valueToData(V value, SDatabaseEntry data) {
        throw new UnsupportedOperationException
            ("Cannot change the data in a key-only index");
    }
}
