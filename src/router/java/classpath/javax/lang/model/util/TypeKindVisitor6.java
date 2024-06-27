/* TypeKindVisitor6.java -- A type visitor implementation for 1.6.
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

import javax.annotation.processing.SupportedSourceVersion;

import javax.lang.model.SourceVersion;
import javax.lang.model.type.NoType;
import javax.lang.model.type.PrimitiveType;

/**
 * <p>An implementation of {@link TypeVisitor} for the
 * 1.6 version of the Java programming language
 * ({@link SourceVersion#RELEASE_6}) which redirects each
 * {@code visitXYZ} method call to a more specific
 * {@code visitXYZAsKind} method, depending on the kind
 * of the first argument. For example, a call to
 * {@code visitNoType} redirects to {@code visitNoTypeAsNone},
 * {@code visitNoTypeAsPackage} or {@code visitNoTypeAsVoid},
 * depending on the type of {@code NoType} supplied. {@code visitXYZAsKind} then
 * redirects to {@code defaultAction(element, parameter)}.
 * Implementors may extend this class and provide alternative
 * implementations of {@link #defaultAction(TypeMirror, P)} and
 * the {@code visitXYZKind} methods as appropriate.</p>
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
 *            methods. {@code Void} can be used if this is not needed.
 *
 * @author Andrew John Hughes (gnu_andrew@member.fsf.org)
 * @since 1.6
 */
@SupportedSourceVersion(SourceVersion.RELEASE_6)
public class TypeKindVisitor6<R,P> extends SimpleTypeVisitor6<R,P>
{

  /**
   * Constructs a new {@link TypeKindVisitor6} with a {@code null}
   * default value.
   */
  protected TypeKindVisitor6()
  {
    this(null);
  }

  /**
   * Constructs a new {@link TypeKindVisitor6} with the specified
   * default value.
   *
   * @param defaultValue the value to assign to {@link SimpleTypeVisitor6#DEFAULT_VALUE}.
   */
  protected TypeKindVisitor6(R defaultValue)
  {
    super(defaultValue);
  }

  /**
   * Visits a {@code NoType} instance.  This implementation dispatches
   * the call to the appropriate visit method, corresponding to the
   * exact kind of {@code NoType}: {@code NONE}, {@code PACKAGE} or
   * {@code VOID}.  For example, a package type will be redirected to the
   * {@link visitNoTypeAsPackage(NoType,P)} method.
   *
   * @param type the type to visit.
   * @param parameter the additional parameter, specific to the visitor.
   *        May be {@code null}.
   * @return the result of the kind-specific method.
   */
  @Override
  public R visitNoType(NoType type, P parameter)
  {
    switch (type.getKind())
      {
      case NONE:
	return visitNoTypeAsNone(type, parameter);
      case PACKAGE:
	return visitNoTypeAsPackage(type, parameter);
      case VOID:
	return visitNoTypeAsVoid(type, parameter);
      default:
	return super.visitNoType(type, parameter);
      }
  }

  /**
   * Visits a {@code NONE} type.  This implementation
   * simply calls {@code defaultAction(type, parameter)}.
   *
   * @param type the type to visit.
   * @param parameter the additional parameter, specific to the visitor.
   *        May be {@code null}.
   * @return the result of {@code defaultAction(type, parameter)}.
   */
  public R visitNoTypeAsNone(NoType type, P parameter)
  {
    return defaultAction(type, parameter);
  }

  /**
   * Visits a {@code PACKAGE} type.  This implementation
   * simply calls {@code defaultAction(type, parameter)}.
   *
   * @param type the type to visit.
   * @param parameter the additional parameter, specific to the visitor.
   *        May be {@code null}.
   * @return the result of {@code defaultAction(type, parameter)}.
   */
  public R visitNoTypeAsPackage(NoType type, P parameter)
  {
    return defaultAction(type, parameter);
  }

  /**
   * Visits a {@code VOID} type.  This implementation
   * simply calls {@code defaultAction(type, parameter)}.
   *
   * @param type the type to visit.
   * @param parameter the additional parameter, specific to the visitor.
   *        May be {@code null}.
   * @return the result of {@code defaultAction(type, parameter)}.
   */
  public R visitNoTypeAsVoid(NoType type, P parameter)
  {
    return defaultAction(type, parameter);
  }

  /**
   * Visits a primitive type.  This implementation dispatches
   * the call to the appropriate visit method, corresponding to the
   * exact kind of primitive: {@code BOOLEAN}, {@code BYTE},
   * {@code CHAR}, {@code DOUBLE}, {@code FLOAT}, {@code INT},
   * {@code LONG} or {@code SHORT}.  For example, a boolean will be
   * redirected to the {@link visitPrimitiveAsBoolean(PrimitiveType,P)}
   * method.
   *
   * @param type the type to visit.
   * @param parameter the additional parameter, specific to the visitor.
   *        May be {@code null}.
   * @return the result of the kind-specific method.
   */
  @Override
  public R visitPrimitive(PrimitiveType type, P parameter)
  {
    switch (type.getKind())
      {
      case BOOLEAN:
	return visitPrimitiveAsBoolean(type, parameter);
      case BYTE:
	return visitPrimitiveAsByte(type, parameter);
      case CHAR:
	return visitPrimitiveAsChar(type, parameter);
      case DOUBLE:
	return visitPrimitiveAsDouble(type, parameter);
      case FLOAT:
	return visitPrimitiveAsFloat(type, parameter);
      case INT:
	return visitPrimitiveAsInt(type, parameter);
      case LONG:
	return visitPrimitiveAsLong(type, parameter);
      case SHORT:
	return visitPrimitiveAsShort(type, parameter);
      default:
	return super.visitPrimitive(type, parameter);
      }
  }

  /**
   * Visits a {@code BOOLEAN} type.  This implementation
   * simply calls {@code defaultAction(type, parameter)}.
   *
   * @param type the type to visit.
   * @param parameter the additional parameter, specific to the visitor.
   *        May be {@code null}.
   * @return the result of {@code defaultAction(type, parameter)}.
   */
  public R visitPrimitiveAsBoolean(PrimitiveType type, P parameter)
  {
    return defaultAction(type, parameter);
  }

  /**
   * Visits a {@code BYTE} type.  This implementation
   * simply calls {@code defaultAction(type, parameter)}.
   *
   * @param type the type to visit.
   * @param parameter the additional parameter, specific to the visitor.
   *        May be {@code null}.
   * @return the result of {@code defaultAction(type, parameter)}.
   */
  public R visitPrimitiveAsByte(PrimitiveType type, P parameter)
  {
    return defaultAction(type, parameter);
  }

  /**
   * Visits a {@code CHAR} type.  This implementation
   * simply calls {@code defaultAction(type, parameter)}.
   *
   * @param type the type to visit.
   * @param parameter the additional parameter, specific to the visitor.
   *        May be {@code null}.
   * @return the result of {@code defaultAction(type, parameter)}.
   */
  public R visitPrimitiveAsChar(PrimitiveType type, P parameter)
  {
    return defaultAction(type, parameter);
  }

  /**
   * Visits a {@code DOUBLE} type.  This implementation
   * simply calls {@code defaultAction(type, parameter)}.
   *
   * @param type the type to visit.
   * @param parameter the additional parameter, specific to the visitor.
   *        May be {@code null}.
   * @return the result of {@code defaultAction(type, parameter)}.
   */
  public R visitPrimitiveAsDouble(PrimitiveType type, P parameter)
  {
    return defaultAction(type, parameter);
  }

  /**
   * Visits a {@code FLOAT} type.  This implementation
   * simply calls {@code defaultAction(type, parameter)}.
   *
   * @param type the type to visit.
   * @param parameter the additional parameter, specific to the visitor.
   *        May be {@code null}.
   * @return the result of {@code defaultAction(type, parameter)}.
   */
  public R visitPrimitiveAsFloat(PrimitiveType type, P parameter)
  {
    return defaultAction(type, parameter);
  }

  /**
   * Visits a {@code INT} type.  This implementation
   * simply calls {@code defaultAction(type, parameter)}.
   *
   * @param type the type to visit.
   * @param parameter the additional parameter, specific to the visitor.
   *        May be {@code null}.
   * @return the result of {@code defaultAction(type, parameter)}.
   */
  public R visitPrimitiveAsInt(PrimitiveType type, P parameter)
  {
    return defaultAction(type, parameter);
  }

  /**
   * Visits a {@code LONG} type.  This implementation
   * simply calls {@code defaultAction(type, parameter)}.
   *
   * @param type the type to visit.
   * @param parameter the additional parameter, specific to the visitor.
   *        May be {@code null}.
   * @return the result of {@code defaultAction(type, parameter)}.
   */
  public R visitPrimitiveAsLong(PrimitiveType type, P parameter)
  {
    return defaultAction(type, parameter);
  }

  /**
   * Visits a {@code SHORT} type.  This implementation
   * simply calls {@code defaultAction(type, parameter)}.
   *
   * @param type the type to visit.
   * @param parameter the additional parameter, specific to the visitor.
   *        May be {@code null}.
   * @return the result of {@code defaultAction(type, parameter)}.
   */
  public R visitPrimitiveAsShort(PrimitiveType type, P parameter)
  {
    return defaultAction(type, parameter);
  }

}

  
