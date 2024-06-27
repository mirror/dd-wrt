/* ElementScanner6.java -- A scanning visitor of program elements for 1.6.
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
 * <p>A scanning implementation of {@link ElementVisitor} for the
 * 1.6 version of the Java programming language
 * ({@link SourceVersion#RELEASE_6}).  Each {@code visitXYZ} method
 * calls {@link scan(Element)} on the elements enclosed within the
 * one it receives as an argument.  Implementors may extend this
 * class and provide alternative implementations of the {@code visitXYZ}
 * methods appropriate, so that scanning takes place in a different
 * order.</p>
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
public class ElementScanner6<R,P> extends AbstractElementVisitor6<R,P>
{

  /** The default return value when no elements are scanned. */
  protected final R DEFAULT_VALUE;

  /**
   * Constructs a new {@code ElementScanner6} using {@code null}
   * as the default value.
   */
  protected ElementScanner6()
  {
    this(null);
  }

  /**
   * Constructs a new {@code ElementScanner6} using the specified
   * default value.
   *
   * @param defaultValue the default value to return when there
   *                     no elements to scan.
   */
  protected ElementScanner6(R defaultValue)
  {
    super();
    DEFAULT_VALUE = defaultValue;
  }

  /**
   * Convenience method to simplify calls with no parameter.
   * This is equivalent to {@code v.scan(element, null)}.
   *
   * @param element the element to scan.
   * @return the result of scanning the element.
   */
  public final R scan(Element element)
  {
    return scan(element, null);
  }

  /**
   * Scans a single element by calling {@code element.accept(this, parameter)}.
   * Subclasses may override this to provide different behaviour.
   *
   * @param element the element to scan.
   * @param parameter the value to use as the additional parameter.
   * @return the result of scanning the element.
   */
  public R scan(Element element, P parameter)
  {
    return element.accept(this, parameter);
  }

  /**
   * Scans a series of elements by iterating over them and calling
   * {@code scan(element, parameter)}.  The result returned is that
   * of the last call or {@code DEFAULT_VALUE} if the iterable is
   * empty.
   *
   * @param elements the elements to scan.
   * @param parameter the value to use as the additional parameter.
   * @return the result of scanning the last element or
   *         {@code DEFAULT_VALUE} if there are none.
   */
  public final R scan(Iterable<? extends Element> elements, P parameter)
  {
    R result = DEFAULT_VALUE;
    for (Element element : elements)
    {
      result = scan(element, parameter);
    }
    return result;
  }

  /**
   * Visits an executable element.  This implementation scans the
   * parameters.
   *
   * @param element the executable element to visit.
   * @param parameter the additional parameter, specific to the visitor.
   *        May be {@code null}.
   * @return the result of scanning the element's parameters.
   */
  @Override
  public R visitExecutable(ExecutableElement element, P parameter)
  {
    return scan(element.getParameters(), parameter);
  }


  /**
   * Visits a package element.  This implementation scans the
   * enclosed elements.
   *
   * @param element the package element to visit.
   * @param parameter the additional parameter, specific to the visitor.
   *        May be {@code null}.
   * @return the result of scanning the enclosed elements.
   */
  @Override
  public R visitPackage(PackageElement element, P parameter)
  {
    return scan(element.getEnclosedElements(), parameter);
  }

  /**
   * Visits a type element.  This implementation scans the
   * enclosed elements.
   *
   * @param element the type element to visit.
   * @param parameter the additional parameter, specific to the visitor.
   *        May be {@code null}.
   * @return the result of scanning the enclosed elements.
   */
  @Override
  public R visitType(TypeElement element, P parameter)
  {
    return scan(element.getEnclosedElements(), parameter);
  }

  /**
   * Visits a type parameter element.  This implementation scans the
   * enclosed elements.
   *
   * @param element the type parameter element to visit.
   * @param parameter the additional parameter, specific to the visitor.
   *        May be {@code null}.
   * @return the result of scanning the enclosed elements.
   */
  @Override
  public R visitTypeParameter(TypeParameterElement element, P parameter)
  {
    return scan(element.getEnclosedElements(), parameter);
  }

  /**
   * Visits a variable element.  This implementation scans the
   * enclosed elements, unless the element is a {@code RESOURCE_VARIABLE},
   * in which case it calls {@code visitUnknown(element, parameter)} to
   * retain 1.6 behaviour.
   *
   * @param element the variable element to visit.
   * @param parameter the additional parameter, specific to the visitor.
   *        May be {@code null}.
   * @return the result of scanning the enclosed elements, or
   *         the result of {@code visitUnknown(element, parameter)} if
   *         the element is a {@code RESOURCE_VARIABLE}.
   */
  @Override
  public R visitVariable(VariableElement element, P parameter)
  {
    if (element.getKind() == ElementKind.RESOURCE_VARIABLE)
      return visitUnknown(element, parameter);
    return scan(element.getEnclosedElements(), parameter);
  }

}
