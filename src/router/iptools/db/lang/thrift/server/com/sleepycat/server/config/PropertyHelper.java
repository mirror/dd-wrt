/*-
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 2002, 2017 Oracle and/or its affiliates.  All rights reserved.
 *
 * $Id$
 */

package com.sleepycat.server.config;

import java.util.Objects;
import java.util.Properties;

/**
 * A helper class for getting and setting properties.
 */
public class PropertyHelper {

    /** The properties. */
    private final Properties properties;

    public PropertyHelper(Properties properties) {
        this.properties =
                Objects.requireNonNull(properties, "properties is null.");
    }

    /**
     * Get the value of an integer property.
     *
     * @param key the property key
     * @param defaultValue the default property value
     * @param min the minimum valid value (inclusive)
     * @param max the maximum valid value (inclusive)
     * @return the property's integer value
     * @throws IllegalArgumentException if the value is invalid
     */
    public int getInt(String key, String defaultValue, int min, int max) {
        try {
            String strValue = this.properties.getProperty(key, defaultValue);
            int value = Integer.parseInt(strValue);
            if (min <= value && value <= max) {
                return value;
            }
        } catch (NumberFormatException e) {
            // fall through
        }
        throw new IllegalArgumentException(
                "Property " + key + " must be an integer in [" + min + ", " +
                        max + "]");
    }

    /**
     * Get the value of a string property.
     *
     * @param key the property key
     * @param defaultValue the default property value
     * @return the property's string value
     */
    public String getString(String key, String defaultValue) {
        return this.properties.getProperty(key, defaultValue);
    }
}
