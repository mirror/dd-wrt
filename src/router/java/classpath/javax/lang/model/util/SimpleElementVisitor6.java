/* SimpleElementVisitor6.java -- A simple visitor implementation for 1.6.
   Copyright (C) 2013, 2015  Free Software Foundation, Inc.

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
import javax.lang.model.element.ElementKind;
import javax.lang.model.element.ElementVisitor;
import javax.lang.model.element.ExecutableElement;
import javax.lang.model.element.PackageElement;
import javax.lang.model.element.TypeElement;
import javax.lang.model.element.TypeParameterElement;
import javax.lang.model.element.VariableElement;

/**
 * <p>A simple implementation of {@link ElementVisitor} for the
 * 1.6 version of the Java programming language
 * ({@link SourceVersion#RELEASE_6}).  Each {@code visitXYZ} method
 * redirects to {@code defaultAction(element, parameter)}.
 * Implementors may extend this class and provide alternative
 * implementations of {@link #defaultAction(Element, P)} and
 * the {@code visitXYZ} methods as appropriate.</p>
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
 *            methods.
 *
 * @author Andrew John Hughes (gnu_andrew@member.fsf.org)
 * @since 1.6
 */
@SupportedSourceVersion(SourceVersion.RELEASE_6)
public class SimpleElementVisitor6<R,P> extends AbstractElementVisitor6<R,P>
{

  /**
   * The default value returned by calls to {@link #defaultAction(Element, P)}.
   */
  protected final R DEFAULT_VALUE;

  /**
   * Constructs a new {@link SimpleElementVisitor6} with a {@code null}
   * default value.
   */
  protected SimpleElementVisitor6()
  {
    this(null);
  }

  /**
   * Constructs a new {@link SimpleElementVisitor6} with the specified
   * default value.
   *
   * @param defaultValue the value to assign to {@link #DEFAULT_VALUE}.
   */
  protected SimpleElementVisitor6(R defaultValue)
  {
    DEFAULT_VALUE = defaultValue;
  }

  /**
   * The default action for all visitor methods.  The default implementation
   * simply returns {@link #DEFAULT_VALUE}.
   *
   * @param element the element to act upon.
   * @param parameter the optional parameter supplied to the visitor.
   * @return {@link #DEFAULT_VALUE}.
   */
  protected R defaultAction(Element element, P parameter)
  {
    return DEFAULT_VALUE;
  }

  /**
   * Visits an executable element.  This implementation simply
   * calls {@code defaultAction(element, parameter)}.
   *
   * @param element the executable element to visit.
   * @param parameter the additional parameter, specific to the visitor.
   *        May be {@code null}.
   * @return the result of {@code defaultAction(element, parameter)}.
   */
  @Override
  public R visitExecutable(ExecutableElement element, P parameter)
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
   * Visits a type element.  This implementation simply
   * calls {@code defaultAction(element, parameter)}.
   *
   * @param element the type element to visit.
   * @param parameter the additional parameter, specific to the visitor.
   *        May be {@code null}.
   * @return the result of {@code defaultAction(element, parameter)}.
   */
  @Override
  public R visitType(TypeElement element, P parameter)
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
   * Visits a variable element.  This implementation calls
   * {@code defaultAction(element, parameter)}, unless the element is a
   * {@code RESOURCE_VARIABLE}, in which case it calls
   * {@code visitUnknown(element, parameter)} to retain 1.6 behaviour.
   *
   * @param element the variable element to visit.
   * @param parameter the additional parameter, specific to the visitor.
   *        May be {@code null}.
   * @return the result of {@code defaultAction(element, parameter)},
   *         or {@code visitUnknown(element, parameter)} if the element
   *         is a {@code RESOURCE_VARIABLE}.
   */
  @Override
  public R visitVariable(VariableElement element, P parameter)
  {
    if (element.getKind() == ElementKind.RESOURCE_VARIABLE)
      return visitUnknown(element, parameter);
    return defaultAction(element, parameter);
  }

}

  
