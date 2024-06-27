/* TypeKind.java -- Represents the kind of a type.
   Copyright (C) 2012, 2013  Free Software Foundation, Inc.

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

package javax.lang.model.type;

/**
 * Represents the kind of a type mirror, such as a boolean or
 * a declared type.  This enumeration may be extended with
 * further kinds to represent future versions of the language.
 *
 * @author Andrew John Hughes (gnu_andrew@member.fsf.org)
 * @since 1.6
 */
public enum TypeKind
{
  /** An array type. */
  ARRAY,
  /** The primitive type boolean */
  BOOLEAN,
  /** The primitive type byte */
  BYTE,
  /** The primitive type char */
  CHAR,
  /** A class or interface type */
  DECLARED,
  /** The primitive type double */
  DOUBLE,
  /** An unresolvable type */
  ERROR,
  /** A method, constructor or initialiser */
  EXECUTABLE,
  /** The primitive type float */
  FLOAT,
  /** The primitive type int */
  INT,
  /** The primitive type long */
  LONG,
  /** A pseudo-type used when nothing else is appropriate */
  NONE,
  /** The null type */
  NULL,
  /** An implementation-reserved type */
  OTHER,
  /** A psuedo-type used for packages */
  PACKAGE,
  /** The primitive type short */
  SHORT,
  /** A type variable */
  TYPEVAR,
  /** A union type. */
  UNION,
  /** A psuedo-type used for the void return type. */
  VOID,
  /** A wildcard type argument. */
  WILDCARD;

  /**
   * Returns true if this is a primitive type.
   *
   * @return true if this is a primitive type.
   */
  public boolean isPrimitive()
  {
    switch (this)
    {
      case BOOLEAN:
      case BYTE:
      case CHAR:
      case DOUBLE:
      case FLOAT:
      case INT:
      case LONG:
      case SHORT:
	return true;
      default:
	return false;
    }
  }

}
