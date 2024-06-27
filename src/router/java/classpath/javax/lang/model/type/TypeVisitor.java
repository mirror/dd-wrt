/* TypeVisitor.java -- A visitor of types.
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
 * <p>A visitor for types.  This is used when the specific
 * type is not known at compile time.  A visitor instance
 * is passed to the {@link TypeMirror#accept(TypeVisitor,P)} method of
 * the type, which then calls the specific {@code visitN} method
 * appropriate to that specific type.</p>
 * <p>The additional parameter supplied to visitor methods may or
 * may not be optional, and so the class is free to throw a
 * {@code NullPointerException} if {@code null} is passed as the
 * additional parameter.</p>
 * <p>As this interface may be extended to accomodate future language
 * versions, implementators are encouraged to extend one of the
 * appropriate abstract classes rather than implementating this
 * interface.  However, this interface should be used as the type
 * for parameters and return values.</p>
 *
 * @param <R> the return type of the visitor's methods.  {@code Void}
 *            can be used where there is no return value.
 * @param <P> the type of the additional parameter supplied to the visitor's
 *            methods.
 *
 * @author Andrew John Hughes (gnu_andrew@member.fsf.org)
 * @since 1.6
 */
public interface TypeVisitor<R,P>
{

  /**
   * A convenience method for use when there is no additional
   * parameter to pass.  This is equivalent to {@code #visit(type, null)}.
   *
   * @param type the type to visit.
   * @return the return value specific to the visitor.
   */
  R visit(TypeMirror type);

  /**
   * Visits a type.
   *
   * @param type the type to visit.
   * @param param the additional parameter, specific to the visitor.
   *        May be {@code null} if permitted by the visitor.
   */
  R visit(TypeMirror type, P param);

  /**
   * Visits an unknown type.  This method is called if
   * a new type is added to the hierarchy which isn't yet
   * handled by the visitor.
   *
   * @param type the type to visit.
   * @param param the additional parameter, specific to the visitor.
   *        May be {@code null} if permitted by the visitor.
   * @return the return value specific to the visitor.
   * @throws UnknownTypeException if the implementation chooses to.
   */
  R visitUnknown(TypeMirror type, P param);

  /**
   * Visits a declared type.
   *
   * @param type the type to visit.
   * @param param the additional parameter, specific to the visitor.
   *        May be {@code null} if permitted by the visitor.
   * @return the return value specific to the visitor.
   */
  R visitDeclared(DeclaredType type, P param);

  /**
   * Visits an array type.
   *
   * @param type the array type to visit.
   * @param param the additional parameter, specific to the visitor.
   *        May be {@code null} if permitted by the visitor.
   * @return the return value specific to the visitor.
   */
  R visitArray(ArrayType type, P param);

  /**
   * Visits an error type.
   *
   * @param type the error type to visit.
   * @param param the additional parameter, specific to the visitor.
   *        May be {@code null} if permitted by the visitor.
   * @return the return value specific to the visitor.
   */
  R visitError(ErrorType type, P param);

  /**
   * Visits an executable type.
   *
   * @param type the executable type to visit.
   * @param param the additional parameter, specific to the visitor.
   *        May be {@code null} if permitted by the visitor.
   * @return the return value specific to the visitor.
   */
  R visitExecutable(ExecutableType type, P param);

  /**
   * Visits a {@link NoType} instance.
   *
   * @param type the instance to visit.
   * @param param the additional parameter, specific to the visitor.
   *        May be {@code null} if permitted by the visitor.
   * @return the return value specific to the visitor.
   */
  R visitNoType(NoType type, P param);

  /**
   * Visits the null type.
   *
   * @param type the type to visit.
   * @param param the additional parameter, specific to the visitor.
   *        May be {@code null} if permitted by the visitor.
   * @return the return value specific to the visitor.
   */
  R visitNull(NullType type, P param);

  /**
   * Visits a primitive type.
   *
   * @param type the primitive type to visit.
   * @param param the additional parameter, specific to the visitor.
   *        May be {@code null} if permitted by the visitor.
   * @return the return value specific to the visitor.
   */
  R visitPrimitive(PrimitiveType type, P param);

  /**
   * Visits a type variable.
   *
   * @param type the type variable to visit.
   * @param param the additional parameter, specific to the visitor.
   *        May be {@code null} if permitted by the visitor.
   * @return the return value specific to the visitor.
   */
  R visitTypeVariable(TypeVariable type, P param);

  /**
   * Visits a wildcard type.
   *
   * @param type the wildcard type to visit.
   * @param param the additional parameter, specific to the visitor.
   *        May be {@code null} if permitted by the visitor.
   * @return the return value specific to the visitor.
   */
  R visitWildcard(WildcardType type, P param);

  /**
   * Visits a union type.
   *
   * @param type the union type to visit.
   * @param param the additional parameter, specific to the visitor.
   *        May be {@code null} if permitted by the visitor.
   * @return the return value specific to the visitor.
   * @since 1.7
   */
  R visitUnion(UnionType type, P param);

}
