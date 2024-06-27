/* TypeMirror.java -- Represents a realised type.
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

package javax.lang.model.type;

/**
 * <p>
 * Represents a realised type in the Java programming language.  For example,
 * a {@code TypeMirror} may represent a primitive type, a declared type
 * (classes and interfaces), an array type, a type variable or the
 * {@code null} type.  To complete the possible types, wildcard types
 * are represented as is a pseudo-type to represent packages and
 * {@code void}, and the signature and return types of executables
 * (constructors, methods and initialisers).
 * </p>
 * <p>To compare two instances, use the utility methods in
 * {@link javax.lang.model.util.Types} as there
 * is no guarantee that == will hold.  To determine the subtype of
 * {@code TypeMirror}, use {@link #getKind()} or a visitor, as
 * implementations may use the same class to implement multiple
 * subinterfaces.</p>
 *
 * @author Andrew John Hughes (gnu_andrew@member.fsf.org)
 * @since 1.6
 */
public interface TypeMirror
{
  
  /**
   * Applies a visitor to this type.
   *
   * @param <R> the return type of the visitor's methods.
   * @param <P> the type of the additional parameter used in the visitor's
   *            methods.
   * @param visitor the visitor to apply to the element.
   * @param param the additional parameter to be sent to the visitor.
   * @return the return value from the visitor.
   */
  <R,P> R accept(TypeVisitor<R,P> visitor, P param);

  /**
   * Obeys the general contract specified by {@link #equals(Object)}
   * but does not indicate that two types represent the same type.
   * For this, {@link javax.lang.model.Types#isSameType(TypeMirror,
   * TypeMirror)} should be used and the result of this method and
   * that method may differ.
   *
   * @param obj the object to compare.
   * @return {@code true} if {@code this} and {@code obj} are equal.
   */
  boolean equals(Object obj);

  /**
   * Returns the kind of this element.
   *
   * @return the kind of element.
   */
  TypeKind getKind();

  /**
   * Obeys the general contract of {@link java.lang.Object#hashCode()}.
   *
   * @return a hash code for this element.
   * @see #equals(Object)
   */
  int hashCode();

  /**
   * Returns an informative representation of the type.  If possible,
   * the form used should be the same as that used if the type was
   * to be represented in source code.  Any type names embedded in the
   * result should be qualified where possible.
   *
   * @return a textual representation of the type.
   */
  String toString();

}
