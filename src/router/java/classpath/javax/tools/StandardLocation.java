/* StandardLocation.java -- Enumeration of standard file locations.
   Copyright (C) 2012  Free Software Foundation, Inc.

This file is part of GNU Classpath.

GNU Classpath is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2, or (at your option)
any later version.

GNU Classpath is distributed in the hope that it will be useful, but
WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
General Public License for more details.

You should have received a copy of the GNU General Public License
along with GNU Classpath; see the file COPYING.  If not, write to the
Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
02110-1301 USA.

Linking this library statically or dynamically with other modules is
making a combined work based on this library.  Thus, the terms and
conditions of the GNU General Public License cover the whole
combination.

As a special exception, the copyright holders of this library give you
permission to link this library with independent modules to produce an
executable, regardless of the license terms of these independent
modules, and to copy and distribute the resulting executable under
terms of your choice, provided that you also meet, for each linked
independent module, the terms and conditions of the license of that
module.  An independent module is a module which is not derived from
or based on this library.  If you modify this library, you may extend
this exception to your version of the library, but you are not
obligated to do so.  If you do not wish to do so, delete this
exception statement from your version. */

package javax.tools;

import java.util.concurrent.ConcurrentMap;
import java.util.concurrent.ConcurrentHashMap;

import static javax.tools.JavaFileManager.Location;

/**
 * Enumeration of standard locations for storing
 * {@link FileObject}s.
 *
 * @author Andrew John Hughes (gnu_andrew@member.fsf.org)
 * @since 1.6
 */
public enum StandardLocation
  implements Location
{
  /** Location where annotation processors are found. */
  ANNOTATION_PROCESSOR_PATH { @Override public boolean isOutputLocation() { return false; } },
  /** Location to write class files to. */
  CLASS_OUTPUT { @Override public boolean isOutputLocation() { return true; } },
  /** Location where class files are found. */
  CLASS_PATH { @Override public boolean isOutputLocation() { return false; } },
  /** Location where platform class files are found. */
  PLATFORM_CLASS_PATH { @Override public boolean isOutputLocation() { return false; } },
  /** Location to write source files to. */
  SOURCE_OUTPUT { @Override public boolean isOutputLocation() { return true; } },
  /** Location where source files are found. */
  SOURCE_PATH { @Override public boolean isOutputLocation() { return false; } };
  
  private static final ConcurrentMap<String,Location> locCache =
    new ConcurrentHashMap<String,Location>();

  static
  {
    for (StandardLocation loc : values())
      locCache.put(loc.name(), loc);
  }

  /**
   * Returns the name of the location.  This is simply
   * the enum constant.
   *
   * @return the name of the location.
   */
  @Override
  public String getName()
  {
    return name();
  }

  /**
   * Returns an instance of {@link JavaFileManager.Location}
   * for the given name.  If the name is one of the standard
   * names, the enumeration constant is returned.  Otherwise,
   * a new instance is generated.  For location names {@code x}
   * and {@code y}, {@code locationFor(x) == locationFor(y)}
   * if, and only if, {@code x.equals(y)}.  The returned location
   * will only be an output location if the name ends with
   * the suffix {@code "_OUTPUT"}.
   *
   * @param name the name of the location.
   * @return the location.
   */
  public static Location locationFor(String name)
  {
    final String locName = name;
    Location loc = locCache.get(name);
    if (loc == null)
      {
	loc = new Location() {
	  public String getName() { return locName; }
	  public boolean isOutputLocation() { return locName.endsWith("_OUTPUT"); }
	};
	Location mapLoc = locCache.putIfAbsent(name, loc);
	if (mapLoc != null) // Someone got there first
	  loc = mapLoc;
      }
    return loc;
  }

}
