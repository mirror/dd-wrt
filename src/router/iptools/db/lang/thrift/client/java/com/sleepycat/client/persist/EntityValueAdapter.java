/*-
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 2002, 2017 Oracle and/or its affiliates.  All rights reserved.
 *
 */

package com.sleepycat.client.persist;

import com.sleepycat.client.bind.EntityBinding;
import com.sleepycat.client.SDatabaseEntry;

/**
 * A ValueAdapter where the "value" is the entity.
 *
 * @author Mark Hayes
 */
class EntityValueAdapter<V> implements ValueAdapter<V> {

    private EntityBinding entityBinding;
    private boolean isSecondary;

    EntityValueAdapter(Class<V> entityClass,
                       EntityBinding entityBinding,
                       boolean isSecondary) {
        this.entityBinding = entityBinding;
        this.isSecondary = isSecondary;
    }

    public SDatabaseEntry initKey() {
        return new SDatabaseEntry();
    }

    public SDatabaseEntry initPKey() {
        return isSecondary ? (new SDatabaseEntry()) : null;
    }

    public SDatabaseEntry initData() {
        return new SDatabaseEntry();
    }

    public void clearEntries(SDatabaseEntry key,
                             SDatabaseEntry pkey,
                             SDatabaseEntry data) {
        key.setData(null);
        if (isSecondary) {
            pkey.setData(null);
        }
        data.setData(null);
    }

    public V entryToValue(SDatabaseEntry key,
                          SDatabaseEntry pkey,
                          SDatabaseEntry data) {
        return (V) entityBinding.entryToObject(isSecondary ? pkey : key, data);
    }

    public void valueToData(V value, SDatabaseEntry data) {
        entityBinding.objectToData(value, data);
    }
}
