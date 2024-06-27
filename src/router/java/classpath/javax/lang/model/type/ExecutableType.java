/* ExecutableType.java -- Represents a method, constructor or initialiser.
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

/**
 * Represents something which may be executed i.e. a
 * constructor, a method or an initialiser.  The executable
 * is viewed respective to a particular type.  As a result,
 * any type parameters will be substituted with actual type
 * arguments as appropriate.
 *
 * @author Andrew John Hughes (gnu_andrew@member.fsf.org)
 * @since 1.6
 * @see 
 */
public interface ExecutableType
  extends TypeMirror
{

  /**
   * Returns the types of the formal parameters of this
   * executable, or an empty list if the executable takes
   * no parameters.  For example, {@code doThis(int)} will
   * return a list containing a single element, the type
   * mirror for {@code int}.
   *
   * @return the executable's formal parameters.
   */
  List<? extends TypeMirror> getParameterTypes();

  /**
   * Returns the return type of the executable.  If the
   * executable is a constructor or an initialiser, or
   * does not return anything, this method will return
   * an instance of {@link NoType} with the kind
   * {@link TypeKind#VOID}.
   *
   * @return the return type of this executable.
   */
  TypeMirror getReturnType();

  /**
   * Returns the types of any exceptions or other
   * {@link Throwable}s mentioned in the {@code throws}
   * clause of this executable, or an empty list if there
   * are none.
   *
   * @return the exceptions and other throwables declared
   *         as thrown by this executable.
   */
  List<? extends TypeMirror> getThrownTypes();

  /**
   * Returns the type variables declared by the formal type
   * parameters or an empty list if there are none.
   *
   * @return the type variables declared by this executable.
   */
  List<? extends TypeVariable> getTypeVariables();

}
