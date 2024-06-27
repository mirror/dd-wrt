/* TypeParameterElement.java -- Represents a formal type parameter.
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

import java.util.List;

import javax.lang.model.type.TypeMirror;

/**
 * Represents a formal type parameter used by a generic class,
 * interface, method or constructor element.  A type parameter
 * is a declaration of a {@link javax.lang.model.type.TypeVariable}.
 *
 * @author Andrew John Hughes (gnu_andrew@member.fsf.org)
 * @since 1.6
 * @see javax.lang.model.type.TypeVariable
 */
public interface TypeParameterElement
  extends Element
{

  /**
   * Returns the bounds of the type parameter.  These are the types
   * declared after the {@code extends} clause.  For example,
   * the bounds for the type parameter {@code T} in
   * {@code Set<T extends Integer>} would be a single element
   * list containing the type mirror for {@code Integer}.  Similarly,
   * for {@code Set<T extends Number & Runnable>}, the bounds
   * would be a two element list containing the type mirrors
   * for {@code Number} and {@code Runnable}.  For a parameter
   * with no explicit bounds, {@code Object} is assumed to be
   * the sole bound and an empty list is returned.
   *
   * @return the bounds of this type parameter, or an empty list if
   *         there are none.
   */
  List<? extends TypeMirror> getBounds();

  /**
   * Returns the generic class, interface, method or constructor
   * in which this type parameter is used.
   *
   * @return the element parameterised by this type parameter.
   */
  Element getGenericElement();

}
