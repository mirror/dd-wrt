/* DeclaredType.java -- Represents a class or interface type.
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

import java.util.List;

import javax.lang.model.element.Element;

/**
 * <p>Represents a type declared in the source code i.e. a class
 * or an interface.  This includes parameterised types, such
 * as {@code Set<String>} as well as raw types, such as {@code Set}.</p>
 * <p>A type represents the usage of a particular element and, as
 * a result, multiple types can be represented by the same element.
 * Both the {@code Set<String>} and {@code Set} types mentioned above
 * would be represented by the same element.</p>
 * <p>This interface is also used to represent <emph>intersection types</emph>
 * which don't exist explicitly in the source code.  For example,
 * in {@code <T extends Number & Runnable>}, the bound is an intersection
 * type.  It is represented by a {@code DeclaredType} with {@code Number}
 * as its superclass and {@code Runnable} as its single superinterface.</p>
 * <p>To obtain the supertypes of a declared type, the method
 * {@link javax.lang.model.util.Types#directSupertypes(TypeMirror)} should
 * be used.  This returns the superclass and any superinterfaces of the
 * type with any type arguments substituted in.</p>
 *
 * @author Andrew John Hughes (gnu_andrew@member.fsf.org)
 * @since 1.6
 * @see javax.lang.model.element.TypeElement
 */
public interface DeclaredType
  extends ReferenceType
{

  /**
   * Returns the element which corresponds to this type.
   * For example, the element for the type {@code Set<String>}
   * would be the element for {@code Set}.
   *
   * @return the element corresponding to this type.
   */
  Element asElement();

  /**
   * Returns the type of the innermost enclosing element or
   * an instance of {@link NoType} with the kind {@link TypeKind#NONE}
   * if this is a top-level type.  Only types representing
   * inner classes have enclosing instances.
   *
   * @return a type mirror for the enclosing type, if any.
   */
  TypeMirror getEnclosingType();

  /**
   * Returns a list of the actual type arguments used by this
   * type.  For example, for {@code Set<String>}, the returned list
   * would include a single type representing {@code String}.
   * Only type arguments for this particular type are returned; type
   * parameters for enclosing types are not included.  If there are
   * no type arguments, an empty list is returned.
   *
   * @return a list of the type arguments used by the type.
   */
  List<? extends TypeMirror> getTypeArguments();

}
