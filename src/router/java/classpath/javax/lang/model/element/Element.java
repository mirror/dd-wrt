/* Element.java -- Represents a program element at the language level.
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

import java.lang.annotation.Annotation;

import java.util.List;
import java.util.Set;

import javax.lang.model.type.TypeMirror;

/**
 * <p>
 * Represents a program element at the language level.  For example,
 * an {@code Element} may represent a package, a class or a type
 * parameter.  Types only present at runtime in the virtual machine
 * are not represented.
 * </p>
 * <p>To compare two instances, use {@link #equals(Object)} as there
 * is no guarantee that == will hold.  To determine the subtype of
 * {@code Element}, use {@link #getKind()} or a visitor, as
 * implementations may use the same class to implement multiple
 * subinterfaces.</p>
 *
 * @author Andrew John Hughes (gnu_andrew@member.fsf.org)
 * @since 1.6
 */
public interface Element
{
  
  /**
   * Applies a visitor to this element.
   *
   * @param <R> the return type of the visitor's methods.
   * @param <P> the type of the additional parameter used in the visitor's
   *            methods.
   * @param visitor the visitor to apply to the element.
   * @param param the additional parameter to be sent to the visitor.
   * @return the return value from the visitor.
   */
  <R,P> R accept(ElementVisitor<R,P> visitor, P param);

  /**
   * <p>Returns {@code true} if this instance represents the same
   * element as the one supplied.</p>
   * <p>Please note that an element may not be equal to the same
   * element provided by a different implementation of this framework,
   * as the equivalence comparison may include the use of internal
   * state which is inaccessible from the element's methods.  This
   * is similar to the way {@link Class} objects provided by different
   * classloaders are not guaranteed to be equal.</p>
   *
   * @param obj the object to compare.
   * @return {@code true} if {@code this} and {@code obj} represent
   *         the same element.
   */
  boolean equals(Object obj);

  /**
   * <p>Returns the element's annotation for the specified annotation type,
   * if present, or {@code null} if not.  The annotation may be directly
   * present on the element or inherited.</p>
   * <p>If the annotation contains an element of type {@code Class} or
   * {@code Class[]}, attempts to read such an object will result in
   * a {@link javax.lang.model.type.MirroredTypeException} or
   * {@link javax.lang.model.type.MirroredTypesException} respectively.
   * This is because information required to load the class (such as its
   * class loader) is unavailable and so it may not be possible to load
   * the class at all.</p>
   * <p>Note that, unlike other methods in this framework, this method
   * operates on runtime information obtained through reflection.
   * As a result, the methods of the returned annotation object may
   * throw exceptions relating to reflection failures.</p>
   *
   * @param <A> the type of annotation.
   * @param annonType the {@code Class} object representing the annotation type.
   * @return an annotation of the specified type, if present, or {@code null}.
   * @see #getAnnotationMirrors()
   * @see Elements#getAllAnnotationMirrors(Element)
   * @see java.lang.reflect.AnnotatedElement#getAnnotation(Class)
   * @see java.lang.annotation.AnnotationTypeMismatchException
   * @see java.lang.annotation.IncompleteAnnotationException
   * @see javax.lang.model.type.MirroredTypeException
   * @see javax.lang.model.type.MirroredTypesException
   */
  <A extends Annotation> A getAnnotation(Class<A> annonType);

  /**
   * Returns the elements directly enclosed by this element.
   * For example, a class element encloses constructor, method,
   * field and further class elements.  The returned list
   * includes elements automatically generated by the compiler
   * such as the default constructor and {@code values} and
   * {@code valueOf} methods present in enumerations.  Package
   * elements contain class and interface elements, but not
   * other packages.  All other types of element do not contain
   * other elements at this time.
   *
   * @return the enclosed elements, or an empty list if the
   *         element has none.
   * @see Elements#getAllMembers(TypeElement)
   */
  List<? extends Element> getEnclosedElements();

  /**
   * Returns the element that encloses this element.  For a top-level
   * type, its package is returned.  Package and type parameter
   * elements return {@code null}.
   *
   * @return the enclosing element or {@code null} if there isn't one.
   * @see Elements#getPackageOf(Element)
   */
  Element getEnclosingElement();

  /**
   * Returns the kind of this element.
   *
   * @return the kind of element.
   */
  ElementKind getKind();

  /**
   * Obeys the general contract of {@link java.lang.Object#hashCode()}.
   *
   * @return a hash code for this element.
   * @see #equals(Object)
   */
  int hashCode();

  /**
   * Returns the type defined by this element.  A generic
   * element defines not one type, but a family of types.
   * For such elements, a prototypical type is returned
   * i.e. the element {@code Set<N extends Number>} will
   * return the type {@code Set<N>}.  The methods of
   * {@link javax.lang.model.util.Types} should be used to
   * obtain the full family of types.
   *
   * @return the type defined by this element.
   * @see javax.lang.model.util.Types
   */
  TypeMirror asType();

  /**
   * Returns the annotations directly present on this element.
   * To obtain inherited annotations as well, call
   * the {@link javax.lang.model.util.Elements#getAllAnnotationMirrors()}
   * method of {@link javax.lang.model.util.Elements}.
   *
   * @return the annotations directly present on this element or
   *         an empty list if there are none.
   * @see javax.lang.model.util.ElementFilter
   */
  List<? extends AnnotationMirror> getAnnotationMirrors();

  /**
   * Returns the modifiers attached to this element, other than
   * annotations e.g. {@code public} or {@code abstract}.  If
   * there are no modifiers, an empty set is returned.
   *
   * @return the modifiers of this element.
   */
  Set<Modifier> getModifiers();

  /**
   * Returns the simple name of this element i.e. without any
   * package prefix.  The simple name of a generic type is the
   * same as that of the raw variant i.e. {@code Set<E>}'s simple
   * name is {@code "Set"}.  Constructors are named {@code "<init>"}
   * and static initializers {@code "<clinit>"}.  Unnamed packages,
   * anonymous classes and instance initializers all return the
   * empty string, {@code ""}.
   *
   * @return the simple name of this element.
   */
  Name getSimpleName();

}
