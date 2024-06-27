/* ExecutableElement.java -- Represents a method, constructor or initialiser.
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
 * Represents a method, constructor or initialiser (static
 * or instance) for a class or interface (including annotation
 * types).
 *
 * @author Andrew John Hughes (gnu_andrew@member.fsf.org)
 * @since 1.6
 * @see javax.lang.model.type.ExecutableType
 */
public interface ExecutableElement
  extends Element, Parameterizable
{

  /**
   * Returns the default value of this type if this is an
   * annotation with a default, or {@code null} otherwise.
   *
   * @return the default value or {@code null}.
   */
  AnnotationValue getDefaultValue();

  /**
   * Returns the formal parameters of this executable
   * in the order they were declared.  If there are no
   * parameters, the list is empty.
   *
   * @return the formal parameters.
   */
  List<? extends VariableElement> getParameters();

  /**
   * Returns the return type of this executable or
   * an instance of {@link javax.lang.model.type.NoType}
   * with the kind {@link javax.lang.model.type.TypeKind#VOID}
   * if there is no return type or this is a
   * constructor or initialiser.
   *
   * @return the return type of the executbale.
   */
  TypeMirror getReturnType();

  /**
   * Returns the exceptions or other throwables listed
   * in the {@code throws} clause in the order they
   * are declared.  The list is empty if there is no
   * {@code throws} clause.
   *
   * @return the exceptions or other throwables listed
   *         as thrown by this executable.
   */
  List<? extends TypeMirror> getThrownTypes();

  /**
   * Returns the formal type parameters of this executable
   * in the order they were declared.  The list is empty
   * if there are no type parameters.
   *
   * @return the formal type parameters.
   */
  List<? extends TypeParameterElement> getTypeParameters();

  /**
   * Returns {@code true} if this method or constructor accepts
   * a variable number of arguments.
   *
   * @return true if this is a varargs method or constructor.
   */
  boolean isVarArgs();

}
