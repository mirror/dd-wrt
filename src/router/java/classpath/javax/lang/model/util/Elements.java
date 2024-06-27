/* Elements.java -- Utility methods for operating on elements.
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

package javax.lang.model.util;

import java.io.Writer;

import java.util.List;
import java.util.Map;

import javax.lang.model.element.AnnotationMirror;
import javax.lang.model.element.AnnotationValue;
import javax.lang.model.element.Element;
import javax.lang.model.element.ExecutableElement;
import javax.lang.model.element.Name;
import javax.lang.model.element.PackageElement;
import javax.lang.model.element.TypeElement;

/**
 * Utility methods for operating on elements.
 *
 * @author Andrew John Hughes (gnu_andrew@member.fsf.org)
 * @since 1.6
 */
public interface Elements
{

  /**
   * Returns all members of a type element, whether declared
   * directly in that element or inherited.  For a class element,
   * this includes all constructors, but not local or anonymous
   * classes.
   *
   * @param type the type to return the members of.
   * @return all members of the type.
   * @see Element#getEnclosedElements()
   */
  List<? extends Element> getAllMembers(TypeElement type);

  /**
   * Returns the text of a constant expression which represents
   * either a primitive value or a {@link String}.  The returned
   * text is in a form suitable for inclusion in source code.
   *
   * @param value a primitive value or string.
   * @return the text of the constant expression.
   * @throws IllegalArgumentExpression if the argument is not a
   *         primitive value or string.
   */
  String getConstantExpression(Object value);

  /**
   * Returns the text of the documentation comment attached to
   * an element.
   *
   * @param elem the element whose comment should be returned.
   * @return the documentation comment, or {@code null} if there is none.
   */
  String getDocComment(Element elem);

  /**
   * Returns a type element, given its canonical name.
   *
   * @param name the canonical name of the element.
   * @return the named type element or {@code null} if it wasn't found.
   */
  TypeElement getTypeElement(CharSequence name);

  /**
   * Tests whether a type, method or field hides another.
   *
   * @param hider the element that is doing the hiding.
   * @param hidden the element hidden by the hider.
   * @return true if, and only if, the hider hides the hidden element.
   */
  boolean hides(Element hider, Element hidden);

  /**
   * Returns true if the specified element has been deprecated.
   *
   * @param elem the element to check for deprecation.
   * @return true if the element is deprecated.
   */
  boolean isDeprecated(Element elem);

  /**
   * Prints out a representation of the elements in the order specified
   * using the supplied writer.  The main purpose of this method is for
   * debugging purposes and the format of the output is not defined.
   *
   * @param writer the writer to send the output to.
   * @param elements the elements to print.
   */
  void printElements(Writer w, Element... elements);

  /**
   * Returns all annotations associated with this element,
   * whether present directly or inherited.
   *
   * @param element the element to examine.
   * @return the annotations associated with this element or
   *         an empty list if there are none.
   * @see Element#getAnnotationMirrors()
   */
  List<? extends AnnotationMirror> getAllAnnotationMirrors(Element element);

  /**
   * Returns a map of elements to their values, including both
   * those explicitly specified and default values.
   * A marker annotation, by definition, returns an empty map.
   * The order of the elements in the map follows that of the
   * source code.
   *
   * @param annotation the annotation whose values should be returned.
   * @return the map of elements to values.
   */
  Map<? extends ExecutableElement, ? extends AnnotationValue>
    getElementValuesWithDefaults(AnnotationMirror annotation);

  /**
   * <p>Returns true if the method {@code overrider} overrides
   * the method {@code overridden}, when {@code overrider} is
   * a member of the given type, according to sections
   * 8.4.8 and 9.4.1 of the Java Language Specification.</p>
   * <p>In most cases, the specified type will simply be the
   * class containing {@code overrider}.  For example, when
   * checking if {@code String.hashCode()} overrides
   * {@code Object.hashCode()}, the type will be {@code String}.
   * However, in a more complex situation, the overridden and
   * the overrider methods may be members of distinct types,
   * {@code A} and {@code B}.  If the type refers to {@code B},
   * the result will be {@code false} as {@code B} has no
   * relationship to the methods in {@code A}.  However, if the
   * type refers to a third type {@code C}, which forms the
   * intersection of {@code A} and {@code B} (say, it extends
   * the class {@code A} and implements the interface {@code B}),
   * then the result may be true.</p>
   *
   * @param overrider the method which may overrider the other.
   * @param overridden the method which may be overridden.
   * @param type the type of which the overrider is a member.
   * @return true if the overrider overriders the overridden
   *              method in the specified type.
   */
  boolean overrides(ExecutableElement overrider,
		    ExecutableElement overridden,
		    TypeElement type);

  /**
   * <p>Returns the <emph>binary name</emph> of a type element.
   * This is determined as follows, according to section
   * 13.1 of the Java Language Specification:</p>
   * <ul>
   * <li>The binary name of a top-level element is its canonical name
   * e.g. {@code java.util.Set}.</li>
   * <li>The binary name of a member type is the binary name of
   * its immediate enclosing type followed by the simple name of the
   * member, separated by a {@code $} e.g. {@code java.util.Map$Entry}.</li>
   * <li>The binary name of a local class is the binary name of
   * its immediate enclosing type followed by the simple name of the
   * local class, separated by {@code $} and a non-empty sequence of
   * digits e.g. {@code java.awt.Window$1DisposeAction}.</li>
   * <li>The binary name of an anonymous class is the binary name of
   * its immediate enclosing type followed by a non-empty sequence of
   * digits, separated by {@code $} e.g. {@code java.io.Console$1}.</li>
   * <li>the binary name of a type variable declared by a generic
   * class or interface is the binary name of its immediate enclosing type
   * followed by the simple name of the type variable, separated by
   * {@code $} e.g. {@code java.util.Set$E}</li>.
   * <li>the binary name of a type variable declared by a generic
   * method or constructor is the binary name of the type declaring the method
   * or constructor, then a {@code $}, folllowed by the method or constructor
   * descriptor, another {@code $} and the simple name of the type variable e.g.
   * {@code java.util.Set$(Ljava/lang/Object;)Z$E</li>.
   * </ul>
   * 
   * @param type the type whose binary name should be returned.
   * @return the binary name, according to the above.
   * @see javax.lang.model.util.element.TypeElement#getQualifiedName()
   */
  Name getBinaryName(TypeElement type);

  /**
   * Returns a name using the specified sequence of characters.
   *
   * @param chars the character sequence to use for the name.
   * @return the resulting name.
   */
  Name getName(CharSequence chars);

  /**
   * Returns a package element corresponding to the specified
   * package name, or {@code null} if it could not be found.
   *
   * @param chars the character sequence which makes up the name
   *              of the package, or {@code ""} for the unnamed package.
   * @return a corresponding package element.
   */
  PackageElement getPackageElement(CharSequence chars);

  /**
   * Returns the package which contains the specified element.
   * If the given element is itself a package, it is returned.
   *
   * @param element the element whose package should be returned.
   * @return the package for the specified type.
   */
  PackageElement getPackageOf(Element element);

}
