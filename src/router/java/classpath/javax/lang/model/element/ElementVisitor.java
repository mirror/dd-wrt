/* ElementVisitor.java -- A visitor of program elements.
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

/**
 * <p>A visitor for program elements.  This is used when the specific
 * type of element is not known at compile time.  A visitor instance
 * is passed to the {@link Element#accept(ElementVisitor,P)} method of
 * the element, which then calls the specific {@code visitN} method
 * appropriate to that specific element.</p>
 * <p>The additional parameter supplied to visitor methods may or
 * may not be optional, and so the class is free to throw a
 * {@code NullPointerException} if {@code null} is passed as the
 * additional parameter.</p>
 * <p>As this interface may be extended to accomodate future language
 * versions, implementors are encouraged to extend one of the
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
public interface ElementVisitor<R,P>
{

  /**
   * A convenience method for use when there is no additional
   * parameter to pass.  This is equivalent to {@code #visit(element, null)}.
   *
   * @param element the element to visit.
   * @return the return value specific to the visitor.
   */
  R visit(Element element);

  /**
   * Visits an element.
   *
   * @param element the element to visit.
   * @param param the additional parameter, specific to the visitor.
   *        May be {@code null} if permitted by the visitor.
   */
  R visit(Element element, P param);

  /**
   * Visits a type element.
   *
   * @param element the type element to visit.
   * @param param the additional parameter, specific to the visitor.
   *        May be {@code null} if permitted by the visitor.
   */
  R visitType(TypeElement element, P param);

  /**
   * Visits an unknown element.  This method is called if
   * a new type of element is added to the hierarchy
   * which isn't yet handled by the visitor.
   *
   * @param element the element to visit.
   * @param param the additional parameter, specific to the visitor.
   *        May be {@code null} if permitted by the visitor.
   * @return the return value specific to the visitor.
   * @throws UnknownElementException if the implementation chooses to.
   */
  R visitUnknown(Element element, P param);

  /**
   * Visits an executable element.
   *
   * @param element the executable element to visit.
   * @param param the additional parameter, specific to the visitor.
   *        May be {@code null} if permitted by the visitor.
   */
  R visitExecutable(ExecutableElement element, P param);

  /**
   * Visits a type parameter element.
   *
   * @param element the type parameter element to visit.
   * @param param the additional parameter, specific to the visitor.
   *        May be {@code null} if permitted by the visitor.
   */
  R visitTypeParameter(TypeParameterElement element, P param);

  /**
   * Visits a variable element.
   *
   * @param element the variable element to visit.
   * @param param the additional parameter, specific to the visitor.
   *        May be {@code null} if permitted by the visitor.
   */
  R visitVariable(VariableElement element, P param);

  /**
   * Visits a package element.
   *
   * @param element the package element to visit.
   * @param param the additional parameter, specific to the visitor.
   *        May be {@code null} if permitted by the visitor.
   */
  R visitPackage(PackageElement element, P param);

}
