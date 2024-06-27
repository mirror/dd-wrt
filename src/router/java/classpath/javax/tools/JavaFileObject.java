/* JavaFileObject.java -- Abstraction for working with class & source files.
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

import javax.lang.model.element.Modifier;
import javax.lang.model.element.NestingKind;

/**
 * Abstraction for tools working with source and class files.
 * All methods may throw a {@link SecurityException}.  All
 * methods may throw a {@link NullPointerException} if supplied
 * with a {@code null} argument, unless otherwise specified.
 *
 * @author Andrew John Hughes (gnu_andrew@member.fsf.org)
 * @since 1.6
 */
public interface JavaFileObject
  extends FileObject
{

  /**
   * Kinds of Java files.
   *
   * @author Andrew John Hughes (gnu_andrew@member.fsf.org)
   * @since 1.6
   */
  public enum Kind
  {
    /** Java class files, which end in {@code ".class"} */ CLASS (".class"),
    /** Web pages, which end in {@code "*.html"} */ HTML (".html"), 
    /** Any other kind of file. */ OTHER (""), 
    /** Java source files, which end in {@code ".java"} */ SOURCE (".java");

    /**
     * The file extension usually used for files of this
     * type.  If there is no conventional extension for
     * this type, this field is set to the empty string,
     * {@code ""}.
     */
    public final String extension;

    Kind(String extension)
    {
      this.extension = extension;
    }

  }

  /**
   * Returns the kind of this file object.
   *
   * @return the kind.
   */
  Kind getKind();

  /**
   * Checks if this file object represents the specified
   * kind of file for the supplied type.  The class name is
   * a simple name i.e. not qualified.
   *
   * @param simpleName the simple name of the class.
   * @param kind the kind of file.
   * @return true if this object matches the type specified.
   */
  boolean isNameCompatible(String simpleName, Kind kind);

  /**
   * Provides a hint about the access level of this class, based
   * on its modifiers.  If the access level is unknown, or this
   * file object does not represent a class file, {@code null}
   * is returned.
   *
   * @return the access level.
   */
  Modifier getAccessLevel();

  /**
   * Provides a hint about the nesting level of this class.
   * It may return {@link NestingKind#MEMBER} instead of
   * {@link NestingKind#LOCAL} or {@link NestingKind#ANONYMOUS}
   * if it can not determine the exact type of nesting.  If
   * the nesting level is completely unknown, or this file
   * object does not represent a class file, {@code null}
   * is returned.
   *
   * @return the nesting level.
   */
  NestingKind getNestingKind();

}
