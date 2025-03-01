/*-
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 2002, 2017 Oracle and/or its affiliates.  All rights reserved.
 *
 */

package com.sleepycat.client.persist.model;

import static java.lang.annotation.ElementType.FIELD;
import static java.lang.annotation.RetentionPolicy.RUNTIME;

import java.lang.annotation.Documented;
import java.lang.annotation.Retention;
import java.lang.annotation.Target;

import com.sleepycat.client.SEnvironment;

/**
 * Indicates the sorting position of a key field in a composite key class when
 * the {@code Comparable} interface is not implemented.  The {@code KeyField}
 * integer value specifies the sort order of this field within the set of
 * fields in the composite key.
 *
 * <p>If the field type of a {@link PrimaryKey} or {@link SecondaryKey} is a
 * composite key class containing more than one key field, then a {@code
 * KeyField} annotation must be present on each non-transient instance field of
 * the composite key class.  The {@code KeyField} value must be a number
 * between one and the number of non-transient instance fields declared in the
 * composite key class.</p>
 *
 * <p>Note that a composite key class is a flat container for one or more
 * simple type fields.  All non-transient instance fields in the composite key
 * class are key fields, and its superclass must be {@code Object}.</p>
 *
 * <p>For example:</p>
 * <pre class="code">
 *  {@literal @Entity}
 *  class Animal {
 *      {@literal @PrimaryKey}
 *      Classification classification;
 *      ...
 *  }
 *
 *  {@literal @Persistent}
 *  class Classification {
 *      {@literal @KeyField(1) String kingdom;}
 *      {@literal @KeyField(2) String phylum;}
 *      {@literal @KeyField(3) String clazz;}
 *      {@literal @KeyField(4) String order;}
 *      {@literal @KeyField(5) String family;}
 *      {@literal @KeyField(6) String genus;}
 *      {@literal @KeyField(7) String species;}
 *      {@literal @KeyField(8) String subspecies;}
 *      ...
 *  }</pre>
 *
 * <p>This causes entities to be sorted first by {@code kingdom}, then by
 * {@code phylum} within {@code kingdom}, and so on.</p>
 *
 * <p>The fields in a composite key class may not be null.</p>
 *
 * <p><a name="comparable"><strong>Custom Sort Order</strong></a></p>
 *
 * <p>Custom sort order is not supported by BDB Server</p>
 *
 * @author Mark Hayes
 */
@Documented @Retention(RUNTIME) @Target(FIELD)
public @interface KeyField {

    int value();
}
