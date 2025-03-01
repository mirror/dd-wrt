/*-
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 2002, 2017 Oracle and/or its affiliates.  All rights reserved.
 *
 */

package com.sleepycat.client.collections.test;

import com.sleepycat.client.bind.RecordNumberBinding;
import com.sleepycat.client.SDatabaseEntry;
import com.sleepycat.client.SSecondaryDatabase;
import com.sleepycat.client.SSecondaryKeyCreator;

import java.nio.ByteOrder;

/**
 * Unused until secondaries are available.
 * @author Mark Hayes
 */
class TestKeyCreator implements SSecondaryKeyCreator {

    private final boolean isRecNum;

    TestKeyCreator(boolean isRecNum) {

        this.isRecNum = isRecNum;
    }

    public boolean createSecondaryKey(SSecondaryDatabase db,
                                      SDatabaseEntry primaryKeyData,
                                      SDatabaseEntry valueData,
                                      SDatabaseEntry indexKeyData) {
        if (valueData.getSize() == 0) {
            return false;
        }
        if (valueData.getSize() != 1) {
            throw new IllegalStateException();
        }
        byte val = valueData.getData()[valueData.getOffset()];
        if (val == 0) {
            return false; // fixed-len pad value
        }
        val -= 100;
        if (isRecNum) {
            RecordNumberBinding.recordNumberToEntry(val, indexKeyData,
                ByteOrder.nativeOrder());
        } else {
            indexKeyData.setData(new byte[] { val }, 0, 1);
        }
        return true;
    }

    public void clearIndexKey(SDatabaseEntry valueData) {

        throw new RuntimeException("not supported");
    }
}
