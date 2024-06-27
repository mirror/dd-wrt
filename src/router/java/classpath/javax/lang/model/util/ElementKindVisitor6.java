/* ElementKindVisitor6.java -- An element visitor implementation for 1.6.
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

import javax.lang.model.element.Element;
import javax.lang.model.element.ElementVisitor;
import javax.lang.model.element.ExecutableElement;
import javax.lang.model.element.PackageElement;
import javax.lang.model.element.TypeElement;
import javax.lang.model.element.TypeParameterElement;
import javax.lang.model.element.VariableElement;

/**
 * <p>An implementation of {@link ElementVisitor} for the
 * 1.6 version of the Java programming language
 * ({@link SourceVersion#RELEASE_6}) which redirects each
 * {@code visitXYZ} method call to a more specific
 * {@code visitXYZAsKind} method, depending on the kind
 * of the first argument. For example, a call to
 * {@code visitExecutable} redirects to {@code visitExecutableAsConstructor},
 * {@code visitExecutableAsInstanceInit},
 * {@code visitExecutableAsMethod} or {@code visitExecutableAsStaticInit},
 * depending on the type of executable supplied. {@code visitXYZAsKind} then
 * redirects to {@code defaultAction(element, parameter)}.
 * Implementors may extend this class and provide alternative
 * implementations of {@link #defaultAction(Element, P)} and
 * the {@code visitXYZKind} methods as appropriate.</p>
 * <p>As the interface this class implements may be extended in future,
 * in order to support later language versions, methods beginning with
 * the phrase {@code "visit"} should be avoided in subclasses.  This
 * class itself will be extended to direct these new methods to the
 * {@link #visitUnknown(Element,P)} method and a new class will be
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
public class ElementKindVisitor6<R,P> extends SimpleElementVisitor6<R,P>
{

  /**
   * Constructs a new {@link ElementKindVisitor6} with a {@code null}
   * default value.
   */
  protected ElementKindVisitor6()
  {
    this(null);
  }

  /**
   * Constructs a new {@link ElementKindVisitor6} with the specified
   * default value.
   *
   * @param defaultValue the value to assign to {@link SimpleElementVisitor6#DEFAULT_VALUE}.
   */
  protected ElementKindVisitor6(R defaultValue)
  {
    super(defaultValue);
  }

  /**
   * Visits an executable element.  This implementation dispatches
   * the call to the appropriate visit method, corresponding to the
   * exact kind of executable element: {@code CONSTRUCTOR},
   * {@code INSTANCE_INIT}, {@code METHOD} or {@code STATIC_INIT}.
   * For example, a constructor element will be redirected to the
   * {@link visitExecutableAsConstructor(ExecutableElement,P)} method.
   *
   * @param element the executable element to visit.
   * @param parameter the additional parameter, specific to the visitor.
   *        May be {@code null}.
   * @return the result of the kind-specific method.
   */
  @Override
  public R visitExecutable(ExecutableElement element, P parameter)
  {
    switch (element.getKind())
      {
      case CONSTRUCTOR:
	return visitExecutableAsConstructor(element, parameter);
      case INSTANCE_INIT:
	return visitExecutableAsInstanceInit(element, parameter);
      case METHOD:
	return visitExecutableAsMethod(element, parameter);
      case STATIC_INIT:
	return visitExecutableAsStaticInit(element, parameter);
      default:
	return super.visitExecutable(element, parameter);
      }
  }

  /**
   * Visits a {@code CONSTRUCTOR} executable element.  This implementation
   * simply calls {@code defaultAction(element, parameter)}.
   *
   * @param element the constructor element to visit.
   * @param parameter the additional parameter, specific to the visitor.
   *        May be {@code null}.
   * @return the result of {@code defaultAction(element, parameter)}.
   */
  public R visitExecutableAsConstructor(ExecutableElement element, P parameter)
  {
    return defaultAction(element, parameter);
  }

  /**
   * Visits an {@code INSTANCE_INIT} executable element.  This implementation
   * simply calls {@code defaultAction(element, parameter)}.
   *
   * @param element the instance initializer element to visit.
   * @param parameter the additional parameter, specific to the visitor.
   *        May be {@code null}.
   * @return the result of {@code defaultAction(element, parameter)}.
   */
  public R visitExecutableAsInstanceInit(ExecutableElement element, P parameter)
  {
    return defaultAction(element, parameter);
  }

  /**
   * Visits a {@code METHOD} executable element.  This implementation
   * simply calls {@code defaultAction(element, parameter)}.
   *
   * @param element the method element to visit.
   * @param parameter the additional parameter, specific to the visitor.
   *        May be {@code null}.
   * @return the result of {@code defaultAction(element, parameter)}.
   */
  public R visitExecutableAsMethod(ExecutableElement element, P parameter)
  {
    return defaultAction(element, parameter);
  }

  /**
   * Visits a {@code STATIC_INIT} executable element.  This implementation
   * simply calls {@code defaultAction(element, parameter)}.
   *
   * @param element the static initializer element to visit.
   * @param parameter the additional parameter, specific to the visitor.
   *        May be {@code null}.
   * @return the result of {@code defaultAction(element, parameter)}.
   */
  public R visitExecutableAsStaticInit(ExecutableElement element, P parameter)
  {
    return defaultAction(element, parameter);
  }

  /**
   * Visits a package element.  This implementation simply
   * calls {@code defaultAction(element, parameter)}.
   *
   * @param element the package element to visit.
   * @param parameter the additional parameter, specific to the visitor.
   *        May be {@code null}.
   * @return the result of {@code defaultAction(element, parameter)}.
   */
  @Override
  public R visitPackage(PackageElement element, P parameter)
  {
    return defaultAction(element, parameter);
  }

  /**
   * Visits a type element.  This implementation dispatches
   * the call to the appropriate visit method, corresponding to the
   * exact kind of type element: {@code ANNOTATION_TYPE},
   * {@code CLASS}, {@code ENUM} or {@code INTERFACE}.
   * For example, an annotation type element will be redirected to the
   * {@link visitTypeAsAnnotationType(TypeElement,P)} method.
   *
   * @param element the type element to visit.
   * @param parameter the additional parameter, specific to the visitor.
   *        May be {@code null}.
   * @return the result of the kind-specific method.
   */
  @Override
  public R visitType(TypeElement element, P parameter)
  {
    switch (element.getKind())
      {
      case ANNOTATION_TYPE:
	return visitTypeAsAnnotationType(element, parameter);
      case CLASS:
	return visitTypeAsClass(element, parameter);
      case ENUM:
	return visitTypeAsEnum(element, parameter);
      case INTERFACE:
	return visitTypeAsInterface(element, parameter);
      default:
	return super.visitType(element, parameter);
      }
  }

  /**
   * Visits an {@code ANNOTATION_TYPE} type element.  This implementation
   * simply calls {@code defaultAction(element, parameter)}.
   *
   * @param element the annotation type element to visit.
   * @param parameter the additional parameter, specific to the visitor.
   *        May be {@code null}.
   * @return the result of {@code defaultAction(element, parameter)}.
   */
  public R visitTypeAsAnnotationType(TypeElement element, P parameter)
  {
    return defaultAction(element, parameter);
  }

  /**
   * Visits a {@code CLASS} type element.  This implementation
   * simply calls {@code defaultAction(element, parameter)}.
   *
   * @param element the class element to visit.
   * @param parameter the additional parameter, specific to the visitor.
   *        May be {@code null}.
   * @return the result of {@code defaultAction(element, parameter)}.
   */
  public R visitTypeAsClass(TypeElement element, P parameter)
  {
    return defaultAction(element, parameter);
  }

  /**
   * Visits a {@code ENUM} type element.  This implementation
   * simply calls {@code defaultAction(element, parameter)}.
   *
   * @param element the enumeration element to visit.
   * @param parameter the additional parameter, specific to the visitor.
   *        May be {@code null}.
   * @return the result of {@code defaultAction(element, parameter)}.
   */
  public R visitTypeAsEnum(TypeElement element, P parameter)
  {
    return defaultAction(element, parameter);
  }

  /**
   * Visits an {@code INTERFACE} type element.  This implementation
   * simply calls {@code defaultAction(element, parameter)}.
   *
   * @param element the interface element to visit.
   * @param parameter the additional parameter, specific to the visitor.
   *        May be {@code null}.
   * @return the result of {@code defaultAction(element, parameter)}.
   */
  public R visitTypeAsInterface(TypeElement element, P parameter)
  {
    return defaultAction(element, parameter);
  }

  /**
   * Visits a type parameter element.  This implementation simply
   * calls {@code defaultAction(element, parameter)}.
   *
   * @param element the type parameter element to visit.
   * @param parameter the additional parameter, specific to the visitor.
   *        May be {@code null}.
   * @return the result of {@code defaultAction(element, parameter)}.
   */
  @Override
  public R visitTypeParameter(TypeParameterElement element, P parameter)
  {
    return defaultAction(element, parameter);
  }

  /**
   * Visits a variable element.  This implementation dispatches
   * the call to the appropriate visit method, corresponding to the
   * exact kind of variable element: {@code ENUM_CONSTANT},
   * {@code EXCEPTION_PARAMETER}, {@code FIELD}, {@code LOCAL_VARIABLE},
   * {@code PARAMETER} or {@code RESOURCE_VARIABLE}.
   * For example, a field variable element will be redirected to the
   * {@link visitVariableAsField(TypeElement,P)} method.
   *
   * @param element the variable element to visit.
   * @param parameter the additional parameter, specific to the visitor.
   *        May be {@code null}.
   * @return the result of the kind-specific method.
   */
  @Override
  public R visitVariable(VariableElement element, P parameter)
  {
    switch (element.getKind())
      {
      case ENUM_CONSTANT:
	return visitVariableAsEnumConstant(element, parameter);
      case EXCEPTION_PARAMETER:
	return visitVariableAsExceptionParameter(element, parameter);
      case FIELD:
	return visitVariableAsField(element, parameter);
      case LOCAL_VARIABLE:
	return visitVariableAsField(element, parameter);
      case PARAMETER:
	return visitVariableAsParameter(element, parameter);
      case RESOURCE_VARIABLE:
	return visitVariableAsResourceVariable(element, parameter);
      default:
	return super.visitVariable(element, parameter);
      }
  }

  /**
   * Visits an {@code ENUM_CONSTANT} variable element.  This implementation
   * simply calls {@code defaultAction(element, parameter)}.
   *
   * @param element the enum constant variable element to visit.
   * @param parameter the additional parameter, specific to the visitor.
   *        May be {@code null}.
   * @return the result of {@code defaultAction(element, parameter)}.
   */
  public R visitVariableAsEnumConstant(VariableElement element, P parameter)
  {
    return defaultAction(element, parameter);
  }

  /**
   * Visits an {@code EXCEPTION_PARAMETER} variable element.  This implementation
   * simply calls {@code defaultAction(element, parameter)}.
   *
   * @param element the exception parameter variable element to visit.
   * @param parameter the additional parameter, specific to the visitor.
   *        May be {@code null}.
   * @return the result of {@code defaultAction(element, parameter)}.
   */
  public R visitVariableAsExceptionParameter(VariableElement element, P parameter)
  {
    return defaultAction(element, parameter);
  }

  /**
   * Visits a {@code FIELD} variable element.  This implementation
   * simply calls {@code defaultAction(element, parameter)}.
   *
   * @param element the field variable element to visit.
   * @param parameter the additional parameter, specific to the visitor.
   *        May be {@code null}.
   * @return the result of {@code defaultAction(element, parameter)}.
   */
  public R visitVariableAsField(VariableElement element, P parameter)
  {
    return defaultAction(element, parameter);
  }

  /**
   * Visits a {@code LOCAL_VARIABLE} variable element.  This implementation
   * simply calls {@code defaultAction(element, parameter)}.
   *
   * @param element the local variable variable element to visit.
   * @param parameter the additional parameter, specific to the visitor.
   *        May be {@code null}.
   * @return the result of {@code defaultAction(element, parameter)}.
   */
  public R visitVariableAsLocalVariable(VariableElement element, P parameter)
  {
    return defaultAction(element, parameter);
  }

  /**
   * Visits a {@code PARAMETER} variable element.  This implementation
   * simply calls {@code defaultAction(element, parameter)}.
   *
   * @param element the parameter variable element to visit.
   * @param parameter the additional parameter, specific to the visitor.
   *        May be {@code null}.
   * @return the result of {@code defaultAction(element, parameter)}.
   */
  public R visitVariableAsParameter(VariableElement element, P parameter)
  {
    return defaultAction(element, parameter);
  }

  /**
   * Visits a {@code RESOURCE_VARIABLE} variable element.  This implementation
   * calls {@code visitUnknown(element, parameter)}.
   *
   * @param element the resource variable element to visit.
   * @param parameter the additional parameter, specific to the visitor.
   *        May be {@code null}.
   * @return the result of {@code defaultAction(element, parameter)}.
   */
  public R visitVariableAsResourceVariable(VariableElement element, P parameter)
  {
    return visitUnknown(element, parameter);
  }

}

  
