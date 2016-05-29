/*
 * Copyright (C) 2010, 2011 Robert Lougher <rob@jamvm.org.uk>.
 *
 * This file is part of JamVM.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2,
 * or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include "jam.h"
#include "properties.h"

char *classlibDefaultJavaHome() {
    return INSTALL_DIR;
}

void classlibAddDefaultProperties(Object *properties) {
    setProperty(properties, "java.runtime.version", VERSION);
    setProperty(properties, "java.version", JAVA_COMPAT_VERSION);
    setProperty(properties, "java.vendor", "GNU Classpath");
    setProperty(properties, "java.vendor.url", "http://www.classpath.org");
    setProperty(properties, "java.specification.version", "1.5");
    setProperty(properties, "java.specification.vendor",
                            "Sun Microsystems Inc.");
    setProperty(properties, "java.specification.name",
                            "Java Platform API Specification");
    setProperty(properties, "java.class.version", "48.0");
    setProperty(properties, "java.boot.class.path", getBootClassPath());
    setProperty(properties, "gnu.classpath.boot.library.path",
                            getBootDllPath());
    setProperty(properties, "gnu.cpu.endian",
                            IS_BIG_ENDIAN ? "big" : "little");
}

