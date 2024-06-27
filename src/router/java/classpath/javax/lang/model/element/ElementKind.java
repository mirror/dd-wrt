/* ElementKind.java -- Represents the kind of an element.
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

package javax.lang.model.element;

/**
 * <p>Represents the kind of an element, such as a class element
 * or a field element.  This enumeration may be extended with
 * further kinds to represent future versions of the language.</p>
 *
 * @author Andrew John Hughes (gnu_andrew@member.fsf.org)
 * @since 1.6
 */
public enum ElementKind
{
  /** An annotation type. */
  ANNOTATION_TYPE,
  /** A normal class, not described by a more specific type. */
  CLASS,
  /** A constructor */
  CONSTRUCTOR,
  /** An enumeration */
  ENUM,
  /** A constant field used in an enumeration. */
  ENUM_CONSTANT,
  /** A parameter passed to an exception handler. */
  EXCEPTION_PARAMETER,
  /** A normal field, not described by a more specific type. */ 
  FIELD,
  /** An instance initializer. */
  INSTANCE_INIT,
  /** A normal interface, not described by a more specific type. */
  INTERFACE,
  /** A local variable. */
  LOCAL_VARIABLE,
  /** A method. */
  METHOD,
  /** An element reserved for implementation-specific usage. */
  OTHER,
  /** A package. */
  PACKAGE,
  /** A parameter passed to a method or constructor. */
  PARAMETER,
  /** A resource variable. */
  RESOURCE_VARIABLE,
  /** A static initializer. */
  STATIC_INIT,
  /** A type parameter. */
  TYPE_PARAMETER;

  /**
   * Returns true if this element is a class i.e. either a
   * general {@code CLASS} or the specific {@code ENUM}.
   *
   * @return true if this kind is either {@code CLASS} or
   *         {@code ENUM}.
   */
  public boolean isClass()
  {
    return this == CLASS || this == ENUM;
  }

  /**
   * Returns true if this element is a field i.e. either a
   * general {@code FIELD} or the specific {@code ENUM_CONSTANT}.
   *
   * @return true if this kind is either {@code FIELD} or
   *         {@code ENUM_CONSTANT}.
   */
  public boolean isField()
  {
    return this == FIELD || this == ENUM_CONSTANT;
  }
  
  /**
   * Returns true if this element is a interface i.e. either a
   * general {@code INTERFACE} or the specific {@code ANNOTATION_TYPE}.
   *
   * @return true if this kind is either {@code INTERFACE} or
   *         {@code ANNOTATION_TYPE}.
   */
  public boolean isInterface()
  {
    return this == INTERFACE || this == ANNOTATION_TYPE;
  }

}
