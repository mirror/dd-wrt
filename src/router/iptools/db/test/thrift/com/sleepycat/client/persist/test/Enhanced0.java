/*-
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 2002, 2017 Oracle and/or its affiliates.  All rights reserved.
 *
 */

package com.sleepycat.client.persist.test;

import static com.sleepycat.client.persist.model.Relationship.MANY_TO_ONE;

import com.sleepycat.client.persist.model.Entity;
import com.sleepycat.client.persist.model.PrimaryKey;
import com.sleepycat.client.persist.model.SecondaryKey;

/**
 * For running ASMifier -- before any enhancements.
 */
@Entity
class Enhanced0 {

    @PrimaryKey
    private String f1;

    @SecondaryKey(relate=MANY_TO_ONE)
    private int f2;
    @SecondaryKey(relate=MANY_TO_ONE)
    private String f3;
    @SecondaryKey(relate=MANY_TO_ONE)
    private String f4;

    private int f5;
    private String f6;
    private String f7;
}
