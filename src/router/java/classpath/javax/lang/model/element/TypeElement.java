/* TypeElement.java -- Represents a class or interface element.
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

package javax.lang.model.element;

import java.util.List;

import javax.lang.model.type.TypeMirror;

/**
 * <p>Represents a class or interface program element.
 * Note that enumerations are a kind of class and annotations
 * are a kind of interface.  The element provides access
 * to information about the type and its members.</p>
 * <p>A distinction is made between elements and types,
 * with the latter being an invocation of the former.
 * This distinction is most clear when looking at
 * parameterized types.  A {@code TypeElement} represents the
 * general type, such as {@code java.util.Set}, while
 * a {@link DeclaredType} instance represents different
 * instantiations such as {@code java.util.Set<String>},
 * {@code java.util.Set<Integer>} and the raw type
 * {@code java.util.Set}.</p>
 * <p>The methods of this interface return elements in the
 * natural order of the underlying information.  So, for
 * example, if the element is derived from a Java source
 * file, elements are returned in the order they appear
 * in the source code.</p>
 *
 * @author Andrew John Hughes (gnu_andrew@member.fsf.org)
 * @since 1.6
 */
public interface TypeElement
  extends Element, Parameterizable, QualifiedNameable
{

  /**
   * Returns the interface types directly implemented by this
   * class or extended by this interface.  If there are none,
   * an empty list is returned.
   *
   * @return the interface types directly implemented by this
   *         class or extended by this interface.
   */
  List<? extends TypeMirror> getInterfaces();

  /**
   * Returns the direct superclass of this element.  If this
   * is an interface or the class {@link Object}, then a
   * instance of {@link javax.lang.model.type.NoType} with
   * the kind {@link javax.lang.model.type.TypeKind#NONE} is
   * returned.
   *
   * @return the direct superclass or {@code NoType} if there
   *         isn't one.
   */
  TypeMirror getSuperclass();

  /**
   * Returns the formal type parameters of this element in the
   * order they were declared.  If there are none, then an empty
   * list is returned.
   *
   * @return the formal type parameters.
   */
  List<? extends TypeParameterElement> getTypeParameters();

  /**
   * Returns the fully qualified or <emph>canonical</emph>
   * name of this type element.  For a local or anonymous
   * class, the empty string is returned.  Generic types
   * do not include their type parameters in the returned
   * string i.e. {@code "java.util.Set"} not
   * {@code "java.util.Set<E>"}.  A period ({@code "."}) is
   * the only separator used, including for nested classes
   * such as {@code "java.util.Map.Entry"}.  See
   * Section 6.7 of the Java Language Specification for
   * more details.
   *
   * @return the canonical name of this type element.
   * @see javax.lang.model.util.Elements#getBinaryName(TypeElement)
   */
  Name getQualifiedName();

  /**
   * Returns the nesting kind of this element.
   *
   * @return the nesting kind.
   */
  NestingKind getNestingKind();

}
