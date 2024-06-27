/* SimpleAnnotationValueVisitor6.java -- A simple visitor implementation for 1.6.
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

import java.util.List;

import javax.annotation.processing.SupportedSourceVersion;

import javax.lang.model.SourceVersion;
import javax.lang.model.element.AnnotationMirror;
import javax.lang.model.element.AnnotationValue;
import javax.lang.model.element.VariableElement;
import javax.lang.model.type.TypeMirror;

/**
 * <p>A simple implementation of {@link AnnotationValueVisitor} for the
 * 1.6 version of the Java programming language
 * ({@link SourceVersion#RELEASE_6}).  Each {@code visitXYZ} method
 * redirects to {@code defaultAction(value, parameter)}.
 * Implementors may extend this class and provide alternative
 * implementations of {@link #defaultAction(Object, P)} and
 * the {@code visitXYZ} methods as appropriate.</p>
 * <p>As the interface this class implements may be extended in future,
 * in order to support later language versions, methods beginning with
 * the phrase {@code "visit"} should be avoided in subclasses.  This
 * class itself will be extended to direct these new methods to the
 * {@link #visitUnknown(AnnotationValue,P)} method and a new class will be
 * added to provide implementations for the new language version.
 * At this time, all or some of this class may be deprecated.</p>
 * 
 * @param <R> the return type of the visitor's methods.  {@code Void}
 *            can be used where there is no return value.
 * @param <P> the type of the additional parameter supplied to the visitor's
 *            methods.
 *
 * @author Andrew John Hughes (gnu_andrew@member.fsf.org)
 * @since 1.6
 */
@SupportedSourceVersion(SourceVersion.RELEASE_6)
public class SimpleAnnotationValueVisitor6<R,P> extends AbstractAnnotationValueVisitor6<R,P>
{

  /**
   * The default value returned by calls to {@link #defaultAction(Object, P)}.
   */
  protected final R DEFAULT_VALUE;

  /**
   * Constructs a new {@link SimpleAnnotationValueVisitor6} with a {@code null}
   * default value.
   */
  protected SimpleAnnotationValueVisitor6()
  {
    this(null);
  }

  /**
   * Constructs a new {@link SimpleAnnotationValueVisitor6} with the specified
   * default value.
   *
   * @param defaultValue the value to assign to {@link #DEFAULT_VALUE}.
   */
  protected SimpleAnnotationValueVisitor6(R defaultValue)
  {
    DEFAULT_VALUE = defaultValue;
  }

  /**
   * The default action for all visitor methods.  The default implementation
   * simply returns {@link #DEFAULT_VALUE}.
   *
   * @param obj the object to act upon.
   * @param parameter the optional parameter supplied to the visitor.
   * @return {@link #DEFAULT_VALUE}.
   */
  protected R defaultAction(Object obj, P parameter)
  {
    return DEFAULT_VALUE;
  }

  /**
   * Visits a boolean value in an annotation.  This implementation simply
   * calls {@code defaultAction(bool, parameter)}.
   *
   * @param bool the boolean value to visit.
   * @param parameter the additional parameter, specific to the visitor.
   *        May be {@code null}.
   * @return the result of {@code defaultAction(bool, parameter)}.
   */
  @Override
  public R visitBoolean(boolean bool, P parameter)
  {
    return defaultAction(bool, parameter);
  }

  /**
   * Visits a byte value in an annotation.  This implementation simply
   * calls {@code defaultAction(val, parameter)}.
   *
   * @param val the byte value to visit.
   * @param parameter the additional parameter, specific to the visitor.
   *        May be {@code null}.
   * @return the result of {@code defaultAction(val, parameter)}.
   */
  @Override
  public R visitByte(byte val, P parameter)
  {
    return defaultAction(val, parameter);
  }

  /**
   * Visits a character value in an annotation.  This implementation simply
   * calls {@code defaultAction(character, parameter)}.
   *
   * @param character the character value to visit.
   * @param parameter the additional parameter, specific to the visitor.
   *        May be {@code null}.
   * @return the result of {@code defaultAction(character, parameter)}.
   */
  @Override
  public R visitChar(char character, P parameter)
  {
    return defaultAction(character, parameter);
  }

  /**
   * Visits a double value in an annotation.  This implementation simply
   * calls {@code defaultAction(val, parameter)}.
   *
   * @param val the double value to visit.
   * @param parameter the additional parameter, specific to the visitor.
   *        May be {@code null}.
   * @return the result of {@code defaultAction(val, parameter)}.
   */
  @Override
  public R visitDouble(double val, P parameter)
  {
    return defaultAction(val, parameter);
  }

  /**
   * Visits a float value in an annotation.  This implementation simply
   * calls {@code defaultAction(val, parameter)}.
   *
   * @param val the float value to visit.
   * @param parameter the additional parameter, specific to the visitor.
   *        May be {@code null}.
   * @return the result of {@code defaultAction(val, parameter)}.
   */
  @Override
  public R visitFloat(float val, P parameter)
  {
    return defaultAction(val, parameter);
  }

  /**
   * Visits an integer value in an annotation.  This implementation simply
   * calls {@code defaultAction(val, parameter)}.
   *
   * @param val the integer value to visit.
   * @param parameter the additional parameter, specific to the visitor.
   *        May be {@code null}.
   * @return the result of {@code defaultAction(val, parameter)}.
   */
  @Override
  public R visitInt(int val, P parameter)
  {
    return defaultAction(val, parameter);
  }

  /**
   * Visits a long value in an annotation.  This implementation simply
   * calls {@code defaultAction(val, parameter)}.
   *
   * @param val the long value to visit.
   * @param parameter the additional parameter, specific to the visitor.
   *        May be {@code null}.
   * @return the result of {@code defaultAction(val, parameter)}.
   */
  @Override
  public R visitLong(long val, P parameter)
  {
    return defaultAction(val, parameter);
  }

  /**
   * Visits a short value in an annotation.  This implementation simply
   * calls {@code defaultAction(val, parameter)}.
   *
   * @param val the short value to visit.
   * @param parameter the additional parameter, specific to the visitor.
   *        May be {@code null}.
   * @return the result of {@code defaultAction(val, parameter)}.
   */
  @Override
  public R visitShort(short val, P parameter)
  {
    return defaultAction(val, parameter);
  }

  /**
   * Visits a String value in an annotation.  This implementation simply
   * calls {@code defaultAction(str, parameter)}.
   *
   * @param str the String value to visit.
   * @param parameter the additional parameter, specific to the visitor.
   *        May be {@code null}.
   * @return the result of {@code defaultAction(str, parameter)}.
   */
  @Override
  public R visitString(String str, P parameter)
  {
    return defaultAction(str, parameter);
  }

  /**
   * Visits a type value in an annotation.  This implementation simply
   * calls {@code defaultAction(type, parameter)}.
   *
   * @param type the type value to visit.
   * @param parameter the additional parameter, specific to the visitor.
   *        May be {@code null}.
   * @return the result of {@code defaultAction(type, parameter)}.
   */
  @Override
  public R visitType(TypeMirror type, P parameter)
  {
    return defaultAction(type, parameter);
  }

  /**
   * Visits an enumeration constant in an annotation.  This implementation simply
   * calls {@code defaultAction(enumConst, parameter)}.
   *
   * @param enumConst the enumeration constant to visit.
   * @param parameter the additional parameter, specific to the visitor.
   *        May be {@code null}.
   * @return the result of {@code defaultAction(enumConst, parameter)}.
   */
  @Override
  public R visitEnumConstant(VariableElement enumConst, P parameter)
  {
    return defaultAction(enumConst, parameter);
  }

  /**
   * Visits an annotation value in an annotation.  This implementation simply
   * calls {@code defaultAction(annotation, parameter)}.
   *
   * @param annotation the annotation value to visit.
   * @param parameter the additional parameter, specific to the visitor.
   *        May be {@code null}.
   * @return the result of {@code defaultAction(annotation, parameter)}.
   */
  @Override
  public R visitAnnotation(AnnotationMirror annotation, P parameter)
  {
    return defaultAction(annotation, parameter);
  }

  /**
   * Visits an array value in an annotation.  This implementation simply
   * calls {@code defaultAction(array, parameter)}.
   *
   * @param array the array value to visit.
   * @param parameter the additional parameter, specific to the visitor.
   *        May be {@code null}.
   * @return the result of {@code defaultAction(array, parameter)}.
   */
  @Override
  public R visitArray(List<? extends AnnotationValue> array, P parameter)
  {
    return defaultAction(array, parameter);
  }

}

  
