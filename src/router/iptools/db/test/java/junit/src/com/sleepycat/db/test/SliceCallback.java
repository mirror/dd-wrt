/*-
 * See the file LICENSE for redistribution information.
 * 
 * Copyright (c) 2015, 2017 Oracle and/or its affiliates.  All rights reserved.
 *
 */


package com.sleepycat.db.test;

import com.sleepycat.db.*;

public class SliceCallback implements Slice {
    public boolean slice(final Database db, final DatabaseEntry key, DatabaseEntry sliceKey) {
	byte [] data;

	data = key.getData();
	sliceKey.setData(data);

	return true;
    }
}