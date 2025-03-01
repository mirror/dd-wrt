/*-
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 2000, 2017 Oracle and/or its affiliates.  All rights reserved.
 *
 */

package com.sleepycat.client.bind.tuple;

import com.sleepycat.client.SDatabaseEntry;
import com.sleepycat.client.util.PackedInteger;

/**
 * A concrete <code>TupleBinding</code> for an unsorted <code>Integer</code>
 * primitive wrapper or an unsorted <code>int</code> primitive, that stores the
 * value in the smallest number of bytes possible.
 *
 * <p>There are two ways to use this class:</p>
 * <ol>
 * <li>When using the {@link com.sleepycat.client} package directly, the static
 * methods in this class can be used to convert between primitive values and
 * {@link SDatabaseEntry} objects.</li>
 * <li>When using the {@link com.sleepycat.client.collections} package, an instance of
 * this class can be used with any stored collection.</li>
 * </ol>
 *
 * @see <a href="package-summary.html#integerFormats">Integer Formats</a>
 */
public class PackedIntegerBinding extends TupleBinding<Integer> {
    
    // javadoc is inherited
    public Integer entryToObject(TupleInput input) {

        return input.readPackedInt();
    }
    
    // javadoc is inherited
    public void objectToEntry(Integer object, TupleOutput output) {

        output.writePackedInt(object);
    }
    
    // javadoc is inherited
    protected TupleOutput getTupleOutput(Integer object) {

        return sizedOutput();
    }
    
    /**
     * Converts an entry buffer into a simple <code>int</code> value.
     *
     * @param entry is the source entry buffer.
     *
     * @return the resulting value.
     */
    public static int entryToInt(SDatabaseEntry entry) {

        return entryToInput(entry).readPackedInt();
    }
    
    /**
     * Converts a simple <code>int</code> value into an entry buffer, using 
     * PackedInteger format.
     *
     * @param val is the source value.
     *
     * @param entry is the destination entry buffer.
     */
    public static void intToEntry(int val, SDatabaseEntry entry) {

        outputToEntry(sizedOutput().writePackedInt(val), entry);
    }
    
    /**
     * Returns a tuple output object of the maximum size needed, to avoid
     * wasting space when a single primitive is output.
     */
    private static TupleOutput sizedOutput() {

        return new TupleOutput(new byte[PackedInteger.MAX_LENGTH]);
    }
}
