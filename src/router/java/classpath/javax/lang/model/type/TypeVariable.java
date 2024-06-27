/* TypeVariable.java -- Represents a type variable.
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

import javax.lang.model.element.Element;

/**
 * Represents a type variable.  Such variables may be the
 * result of an explicitly declared type parameter on
 * a type, method or constructor, or be created implicitly
 * as a function of capture conversion on a wildcard type
 * argument.  See chapter 5 of the Java Language Specification.
 *
 * @author Andrew John Hughes (gnu_andrew@member.fsf.org)
 * @since 1.6
 */
public interface TypeVariable
  extends ReferenceType
{

  /**
   * Returns the element which corresponds to this type variable.
   *
   * @return the element corresponding to this type variable.
   */
  Element asElement();

  /**
   * Returns the lower bound of this type variable.  While type
   * variables can not be given an explicit lower bound, a
   * non-trivial lower bound may be produced through capture
   * conversion on a wildcard type argument.  If this is not the
   * case, the lower bound will be the {@link NullType}.
   *
   * @return the lower bound of this type variable.
   */
  TypeMirror getLowerBound();

  /**
   * Returns the upper bound of this type variable.  If an
   * explicit upper bound was not given, this will return
   * {@code java.lang.Object}.  Where multiple upper bounds
   * are specified, an intersection type is generated and
   * represented as an instance of {@link DeclaredType}.
   * The supertypes of this may be examined to find the
   * actual bounds specified.
   *
   * @return the upper bound of this type variable.
   */
  TypeMirror getUpperBound();

}
