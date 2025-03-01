/*-
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 2002, 2017 Oracle and/or its affiliates.  All rights reserved.
 *
 */

package com.sleepycat.client.collections.test;

import com.sleepycat.client.bind.EntityBinding;
import com.sleepycat.client.bind.RecordNumberBinding;
import com.sleepycat.client.SDatabaseEntry;

import java.nio.ByteOrder;

/**
 * @author Mark Hayes
 */
class TestEntityBinding implements EntityBinding {

    private boolean isRecNum;

    TestEntityBinding(boolean isRecNum) {

        this.isRecNum = isRecNum;
    }

    public Object entryToObject(SDatabaseEntry key, SDatabaseEntry value) {

        byte keyByte;
        if (isRecNum) {
            if (key.getSize() != 4) {
                throw new IllegalStateException();
            }
            keyByte = (byte) RecordNumberBinding.entryToRecordNumber(key,
                    ByteOrder.nativeOrder());
        } else {
            if (key.getSize() != 1) {
                throw new IllegalStateException();
            }
            keyByte = key.getData()[key.getOffset()];
        }
        if (value.getSize() != 1) {
            throw new IllegalStateException();
        }
        byte valByte = value.getData()[value.getOffset()];
        return new TestEntity(keyByte, valByte);
    }

    public void objectToKey(Object object, SDatabaseEntry key) {

        byte val = (byte) ((TestEntity) object).key;
        if (isRecNum) {
            RecordNumberBinding.recordNumberToEntry(val, key,
                ByteOrder.nativeOrder());
        } else {
            key.setData(new byte[] { val }, 0, 1);
        }
    }

    public void objectToData(Object object, SDatabaseEntry value) {

        byte val = (byte) ((TestEntity) object).value;
        value.setData(new byte[] { val }, 0, 1);
    }
}
