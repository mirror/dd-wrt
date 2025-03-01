/*-
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 2002, 2017 Oracle and/or its affiliates.  All rights reserved.
 *
 */

package com.sleepycat.client.collections.test;

import com.sleepycat.client.bind.RecordNumberBinding;
import com.sleepycat.client.collections.PrimaryKeyAssigner;
import com.sleepycat.client.SDatabaseEntry;

import java.nio.ByteOrder;

/**
 * @author Mark Hayes
 */
class TestKeyAssigner implements PrimaryKeyAssigner {

    private byte next = 1;
    private final boolean isRecNum;

    TestKeyAssigner(boolean isRecNum) {

        this.isRecNum = isRecNum;
    }

    public void assignKey(SDatabaseEntry keyData) {
        if (isRecNum) {
            RecordNumberBinding.recordNumberToEntry(next, keyData,
                ByteOrder.nativeOrder());
        } else {
            keyData.setData(new byte[] { next }, 0, 1);
        }
        next += 1;
    }

    void reset() {

        next = 1;
    }
}
