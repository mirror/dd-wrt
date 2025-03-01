/*-
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 2002, 2017 Oracle and/or its affiliates.  All rights reserved.
 *
 * $Id$
 */

package com.sleepycat.client;

import org.apache.thrift.TBase;
import org.apache.thrift.TFieldIdEnum;

import java.util.Objects;

/**
 * A base class for all classes which are simple wrappers of Thrift-generated
 * classes.
 * <p>
 * This class implements some utility methods for all subclasses.
 */
abstract class ThriftWrapper<T extends TBase<T, F>, F extends TFieldIdEnum> {
    /** The Thrift object. */
    private final T tObject;

    /**
     * Create a wrapper object.
     *
     * @param obj the Thrift object
     */
    protected ThriftWrapper(T obj) {
        this.tObject = Objects.requireNonNull(obj);
    }

    /**
     * Return the wrapped Thrift object. This method is safe to be called with a
     * null object, in which case null is returned.
     *
     * @param wrapper the wrapper object, or null
     * @return the wrapped object or null if the given object is null
     */
    static <T extends TBase<T, ?>> T nullSafeGet(ThriftWrapper<T, ?> wrapper) {
        return wrapper == null ? null : wrapper.tObject;
    }

    protected T getThriftObj() {
        return this.tObject;
    }

    /**
     * Return the value set on a specified field. Throw IllegalStateException
     * if the field is not set.
     *
     * @param field the field
     * @return the field's value
     * @throws IllegalStateException if the field is not set
     */
    protected Object getField(F field) throws IllegalStateException {
        if (this.tObject.isSet(field)) {
            return this.tObject.getFieldValue(field);
        } else {
            throw new IllegalStateException(
                    field.getFieldName() + " is not set.");
        }
    }
}
