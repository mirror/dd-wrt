/* AnnotationValueVisitor.java -- A visitor of annotation values.
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
 * <p>A visitor for annotation values.  Unlike other visitors, such
 * as {@link ElementVisitor} and {@link javax.lang.model.type.TypeVisitor},
 * which work on a concrete type hierarchy, this visitor dispatches based
 * on the type of data stored in the annotation, some of which don't have
 * distinct subclasses for storing them (e.g. primitives like {@code boolean}
 * and {@code int}.</p>
 * <p> The visitor is used when the specific  type is not known at compile
 * time.  A visitor instance is passed to the
 * {@link AnnotationValue#accept(AnnotationValueVisitor,P)} method of
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
public interface AnnotationValueVisitor<R,P>
{

  /**
   * A convenience method for use when there is no additional
   * parameter to pass.  This is equivalent to {@code #visit(value, null)}.
   *
   * @param value the value to visit.
   * @return the return value specific to the visitor.
   */
  R visit(AnnotationValue value);

  /**
   * Visits a value.
   *
   * @param value the value to visit.
   * @param param the additional parameter, specific to the visitor.
   *        May be {@code null} if permitted by the visitor.
   */
  R visit(AnnotationValue value, P param);

  /**
   * Visits an annotation value.
   *
   * @param value the value to visit.
   * @param param the additional parameter, specific to the visitor.
   *        May be {@code null} if permitted by the visitor.
   */
  R visitAnnotation(AnnotationMirror value, P param);

  /**
   * Visits an array value.
   *
   * @param value the value to visit.
   * @param param the additional parameter, specific to the visitor.
   *        May be {@code null} if permitted by the visitor.
   */
  R visitArray(List<? extends AnnotationValue> value, P param);

  /**
   * Visits a boolean value.
   *
   * @param value the value to visit.
   * @param param the additional parameter, specific to the visitor.
   *        May be {@code null} if permitted by the visitor.
   */
  R visitBoolean(boolean value, P param);

  /**
   * Visits a byte value.
   *
   * @param value the value to visit.
   * @param param the additional parameter, specific to the visitor.
   *        May be {@code null} if permitted by the visitor.
   */
  R visitByte(byte value, P param);

  /**
   * Visits a character value.
   *
   * @param value the value to visit.
   * @param param the additional parameter, specific to the visitor.
   *        May be {@code null} if permitted by the visitor.
   */
  R visitChar(char value, P param);

  /**
   * Visits a double value.
   *
   * @param value the value to visit.
   * @param param the additional parameter, specific to the visitor.
   *        May be {@code null} if permitted by the visitor.
   */
  R visitDouble(double value, P param);

  /**
   * Visits an enum value.
   *
   * @param value the value to visit.
   * @param param the additional parameter, specific to the visitor.
   *        May be {@code null} if permitted by the visitor.
   */
  R visitEnumConstant(VariableElement value, P param);

  /**
   * Visits a float value.
   *
   * @param value the value to visit.
   * @param param the additional parameter, specific to the visitor.
   *        May be {@code null} if permitted by the visitor.
   */
  R visitFloat(float value, P param);

  /**
   * Visits an int value.
   *
   * @param value the value to visit.
   * @param param the additional parameter, specific to the visitor.
   *        May be {@code null} if permitted by the visitor.
   */
  R visitInt(int value, P param);

  /**
   * Visits a long value.
   *
   * @param value the value to visit.
   * @param param the additional parameter, specific to the visitor.
   *        May be {@code null} if permitted by the visitor.
   */
  R visitLong(long value, P param);

  /**
   * Visits a short value.
   *
   * @param value the value to visit.
   * @param param the additional parameter, specific to the visitor.
   *        May be {@code null} if permitted by the visitor.
   */
  R visitShort(short value, P param);

  /**
   * Visits a {@code String} value.
   *
   * @param value the value to visit.
   * @param param the additional parameter, specific to the visitor.
   *        May be {@code null} if permitted by the visitor.
   */
  R visitString(String value, P param);

  /**
   * Visits a type value.
   *
   * @param value the value to visit.
   * @param param the additional parameter, specific to the visitor.
   *        May be {@code null} if permitted by the visitor.
   */
  R visitType(TypeMirror value, P param);

  /**
   * Visits an unknown type of value.  This method is called if
   * a new type is added to the hierarchy which isn't yet
   * handled by the visitor.
   *
   * @param value the value to visit.
   * @param param the additional parameter, specific to the visitor.
   *        May be {@code null} if permitted by the visitor.
   * @return the return value specific to the visitor.
   * @throws UnknownAnnotationValueException if the implementation
   *                                         chooses to.
   */
  R visitUnknown(AnnotationValue value, P param);

}
