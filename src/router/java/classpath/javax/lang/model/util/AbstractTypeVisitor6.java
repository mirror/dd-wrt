/* AbstractTypeVisitor6.java -- A visitor of types for 1.6.
   Copyright (C) 2015  Free Software Foundation, Inc.

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

package javax.lang.model.util;

import javax.lang.model.SourceVersion;

import javax.lang.model.type.TypeMirror;
import javax.lang.model.type.TypeVisitor;
import javax.lang.model.type.UnionType;
import javax.lang.model.type.UnknownTypeException;

/**
 * <p>A skeletal implementation of {@link TypeVisitor} for the
 * 1.6 version of the Java programming language
 * ({@link SourceVersion#RELEASE_6}).  Implementors can extend this
 * class and need provide only implementations of the
 * {@code visitXYZ} methods appropriate to 1.6.</p>
 * <p>As the interface this class implements may be extended in future,
 * in order to support later language versions, methods beginning with
 * the phrase {@code "visit"} should be avoided in subclasses.  This
 * class itself will be extended to direct these new methods to the
 * {@link #visitUnknown(TypeMirror,P)} method and a new class will be
 * added to provide implementations for the new language version.
 * At this time, all or some of this class may be deprecated.</p>
 *
 * @param <R> the return type of the visitor's methods.  {@code Void}
 *            can be used where there is no return value.
 * @param <P> the type of the additional parameter supplied to the visitor's
 *            methods. {@code Void} can be used where no additional
 *            parameter is required.
 *
 * @author Andrew John Hughes (gnu_andrew@member.fsf.org)
 * @since 1.6
 */
public abstract class AbstractTypeVisitor6<R,P> implements TypeVisitor<R,P>
{

  /**
   * Constructs a new {@code AbstractTypeVisitor6}.
   */
  protected AbstractTypeVisitor6() {}

  /**
   * Visits a type mirror by passing itself to the type
   * mirror's {@code accept} method, with {@code null} for the additional
   * parameter i.e. {@code{v.visit(typeMirror)} is equivalent to 
   * {@code{typeMirror.accept(v, null)}.
   *
   * @param typeMirror the type to visit.
   * @return the return value specific to the visitor.
   */
  @Override
  public final R visit(TypeMirror typeMirror)
  {
    return typeMirror.accept(this, null);
  }

  /**
   * Visits a type mirror by passing itself to the type
   * mirror's {@code accept} method, with the specified value as the additional
   * parameter i.e. {@code{v.visit(typeMirror, parameter)} is equivalent to 
   * {@code{typeMirror.accept(v, parameter)}.
   *
   * @param typeMirror the type to visit.
   * @param parameter the value to use as the additional parameter.
   * @return the return value specific to the visitor.
   */
  @Override
  public final R visit(TypeMirror typeMirror, P parameter)
  {
    return typeMirror.accept(this, parameter);
  }

  /**
   * Visits an unknown type mirror.  This method is called if
   * this visitor is used in a version of the language later
   * than 1.6, where new types have been added.  This
   * implementation always throws a {@link UnknownTypeException}
   * but this is not required of subclasses.
   *
   * @param typeMirror the type to visit.
   * @param parameter the additional parameter, specific to the visitor.
   * @return the return value specific to the visitor.
   * @throws UnknownTypeException by default.
   */
  @Override
  public R visitUnknown(TypeMirror typeMirror, P parameter)
  {
    throw new UnknownTypeException(typeMirror, parameter);
  }

  /**
   * Provides a default implementation for the {@code visitUnion} method
   * added in 1.7, so that existing subclasses continue to compile. It
   * simply redirects to {@link #visitUnknown(TypeMirror,P)}.
   *
   * @param type the union type to visit.
   * @param param the additional parameter, specific to the visitor.
   *        May be {@code null} if permitted by the visitor.
   * @return the return value specific to the visitor.
   * @since 1.7
   */
  @Override
  public R visitUnion(UnionType type, P parameter)
  {
    return visitUnknown(type, parameter);
  }

}
