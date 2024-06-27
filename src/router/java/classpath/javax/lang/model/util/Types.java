/* Types.java -- Utility methods for operating on types.
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

import java.util.List;

import javax.lang.model.element.Element;
import javax.lang.model.element.TypeElement;

import javax.lang.model.type.ArrayType;
import javax.lang.model.type.DeclaredType;
import javax.lang.model.type.ExecutableType;
import javax.lang.model.type.NoType;
import javax.lang.model.type.NullType;
import javax.lang.model.type.PrimitiveType;
import javax.lang.model.type.TypeKind;
import javax.lang.model.type.TypeMirror;
import javax.lang.model.type.WildcardType;

/**
 * Utility methods for operating on types.
 *
 * @author Andrew John Hughes (gnu_andrew@member.fsf.org)
 * @since 1.6
 */
public interface Types
{

  /**
   * Returns the element corresponding to this type
   * or {@code null} if no such element exists.
   * The type may be a {@link javax.lang.model.type.DeclaredType}
   * or a {@link javax.lang.model.type.TypeVariable}.
   * 
   * @param type the type to return the corresponding element to.
   * @return the element corresponding to this type.
   */
  Element asElement(TypeMirror type);

  /**
   * Returns a view of an element when seen as a member of,
   * or otherwise contained by, the specified type.  For
   * example, the method {@code Set.add(E)} becomes
   * {@code Set.add(String)} when viewed as a member of
   * the parameterised type {@code Set<String>}.
   *
   * @param container the type containing the element.
   * @param element the element being contained.
   * @return the type of the element viewed from the container.
   * @throws IllegalArgumentException if the element is not applicable
   *                                  for the specified container.
   */
  TypeMirror asMemberOf(DeclaredType container, Element element);

  /**
   * Applies capture conversion to the type, as specified
   * in section 5.1.10 of the Java Language Specification.
   *
   * @param type the type to capture convert.
   * @return the result of applying capture conversion.
   * @throws IllegalArgumentException if given an executable or package type.
   */
  TypeMirror capture(TypeMirror type);

  /**
   * Returns {@code true} if {@code type1} contains {@code type2}.
   *
   * @param type1 the first type.
   * @param type2 the second type.
   * @return true if type1 contains type2.
   * @throws IllegalArgumentException if given an executable or package type.
   */
  boolean contains(TypeMirror type1, TypeMirror type2);

  /**
   * Returns the direct supertypes of the specified type.  The superclass
   * appears first, if any, followed by the interfaces, if any.
   *
   * @param type the type to examine.
   * @return the direct supertypes or an empty list if there are none.
   * @throws IllegalArgumentException if given an executable or package type.
   */
  List<? extends TypeMirror> directSupertypes(TypeMirror type);

  /**
   * Returns the erasure of the type, as specified by section
   * 4.6 of the Java Language Specification.
   *
   * @param type the type to erase.
   * @return the type erasure.
   * @throws IllegalArgumentException if given a package type.
   */
  TypeMirror erasure(TypeMirror type);

  /**
   * <p>Returns the type corresponding to a type element and actual
   * type arguments, given the specified containing type of which
   * it is a member.  This method is needed to retrieve types
   * contained by parameterised types, for example
   * {@code Outer<String>.Inner<Integer>}.  To obtain this type,
   * this method should be supplied with the {@link DeclaredType}
   * for {@code Outer<String>} (probably obtained by a call
   * to {@link #getDeclaredType(TypeElement, TypeMirror...)}),
   * the element representing {@code Inner} and the
   * {@link TypeMirror} for {@code String}</p>.
   * <p>If the container is parameterised, the number of type
   * arguments must equal the number of formal type parameters
   * used in the specified element.  Calls where the container
   * is not parameterised, or is {@code null}, are equivalent
   * to calls to {@link #getDeclaredType(TypeElement, TypeMirror...)}.
   *
   * @param container the type containing the returned type,
   *                  or {@code null} if there is none.
   * @param element the type element to return the realised type for.
   * @param args the actual type arguments to match to the element's
   *             formal parameters.
   * @return the type corresponding to the specified element using
   *         the specified type arguments.
   * @throws IllegalArgumentException if the number of type arguments
   *                                  doesn't match the number of type
   *                                  parameters, or an inappropriate
   *                                  container, type element or argument
   *                                  is given.
   */
  DeclaredType getDeclaredType(DeclaredType container, TypeElement element,
			       TypeMirror... args);

  /**
   * Returns the type corresponding to a type element and actual
   * type arguments.  So, for the type element for {@code Set<T>},
   * and the type mirror for {@code String}, this method would
   * return the declared type for {@code Set<String>}.  The number
   * of type arguments must either match the number of formal
   * type parameters in the element, or be zero in order to obtain
   * the raw type ({@code Set} in the above).  The type element
   * must not be contained within a generic outer class; for that,
   * use {@link  #getDeclaredType(DeclaredType,TypeElement,TypeMirror...)}.
   *
   * @param element the type element to return the realised type for.
   * @param args the actual type arguments to match to the element's
   *             formal parameters.
   * @return the type corresponding to the specified element using
   *         the specified type arguments.
   * @throws IllegalArgumentException if the number of type arguments
   *                                  doesn't match the number of type
   *                                  parameters, or an inappropriate
   *                                  type element or argument is given.
   */
  DeclaredType getDeclaredType(TypeElement element, TypeMirror... args);

  /**
   * Returns {@code true} if {@code type1} may be assigned to {@code type2},
   * according to section 5.2 of the Java Language Specification.
   *
   * @param type1 the first type.
   * @param type2 the second type.
   * @return true if type1 can be assigned to type2.
   * @throws IllegalArgumentException if given an executable or package type.
   */
  boolean isAssignable(TypeMirror type1, TypeMirror type2);

  /**
   * Returns {@code true} if the two types are the same.  Note that
   * wildcard types are never the same as any other type, including
   * themselves.  This rule prevents wildcard types being used as
   * method arguments.
   *
   * @param type1 the first type.
   * @param type2 the second type.
   * @return true iff the two types are the same.
   */
  boolean isSameType(TypeMirror type1, TypeMirror type2);

  /**
   * Returns {@code true} if {@code type1} is a subtype of {@code type2},
   * according to section 4.10 of the Java Language Specification.  A
   * type is always considered a subtype of itself.
   *
   * @param type1 the first type.
   * @param type2 the second type.
   * @return true if type1 is a subtype of type2.
   * @throws IllegalArgumentException if given an executable or package type.
   */
  boolean isSubtype(TypeMirror type1, TypeMirror type2);

  /**
   * <p>Returns the class which is used to wrap the given primitive
   * type, according to the following mapping given in section
   * 5.1.7 of the Java Language Specification:</p>
   * <ul>
   * <li>{@code boolean} ==&gt; {@code Boolean}</li>
   * <li>{@code byte} ==&gt; {@code Byte}</li>
   * <li>{@code char} ==&gt; {@code Character}</li>
   * <li>{@code double} ==&gt; {@code Double}</li>
   * <li>{@code float} ==&gt; {@code Float}</li>
   * <li>{@code int} ==&gt; {@code Integer}</li>
   * <li>{@code long} ==&gt; {@code Long}</li>
   * <li>{@code short} ==&gt; {@code Short}</li>
   * </ul>
   *
   * @param primitive the primitive type whose wrapper class should
   *                  be returned.
   * @return the wrapper class used for the given primitive type.
   */
  TypeElement boxedClass(PrimitiveType primitive);

  /**
   * Returns an array type with the specified component type.
   *
   * @param componentType the component type to be used in the array.
   * @return an array type using the specified component type.
   * @throws IllegalArgumentException if the component type given
   *                                  can not be used in an array.
   */
  ArrayType getArrayType(TypeMirror componentType);

  /**
   * Returns a pseudo-type of the specified kind for use where a real
   * type is not applicable.  Only the kinds {@link TypeKind#VOID}
   * and {@link TypeKind#NONE} should be passed to this method.
   * For packages, use
   * {@code Elements#getPackageElement(CharSequence).asType()}
   * instead.
   *
   * @param kind the kind of {@link NoType} to return.
   * @return the corresponding instance.
   * @throws IllegalArgumentException if an invalid kind is given.
   */
  NoType getNoType(TypeKind kind);

  /**
   * Returns the null type i.e. the type of {@code null}.
   *
   * @return the null type.
   */
  NullType getNullType();

  /**
   * Returns a primitive type of the given kind.
   *
   * @param kind the kind of primitive type to return.
   * @return the corresponding instance.
   * @throws IllegalArgumentException if the kind given is not
   *                                  a primitive type.
   */
  PrimitiveType getPrimitiveType(TypeKind kind);

  /**
   * Returns a wildcard type with the specified bounds.
   * Each bound is optional and {@code null} may be passed
   * instead.  It is invalid for both bounds to be non-null.
   *
   * @param extendsBound the upper bound, which usually follows the
   *                     {@code extends clause}, or {@code null}.
   * @param superBound the lower bound, which usually follows the
   *                   {@code super clause}, or {@code null}.
   * @return the corresponding wildcard type.
   * @throws IllegalArgumentException if the bounds are invalid.
   */
  WildcardType getWildcardType(TypeMirror extendsBound,
			       TypeMirror superBound);

  /**
   * Returns {@code true} if the signature of {@code method1} is
   * a subsignature of the signature of {@code method2}, according
   * to section 8.4.2 of the Java Language Specification.
   *
   * @param method1 the first method.
   * @param method2 the second method.
   * @return true if method1's signature is a subsignature of method2's.
   */
  boolean isSubsignature(ExecutableType method1, ExecutableType method2);

  /**
   * <p>Returns the primitive type which is used to unwrap the value
   * contained in the specified wrapper class, according to the following
   * mapping given in section 5.1.8 of the Java Language Specification:</p>
   * <ul>
   * <li>{@code Boolean} ==&gt; {@code boolean}</li>
   * <li>{@code Byte} ==&gt; {@code byte}</li>
   * <li>{@code Character} ==&gt; {@code char}</li>
   * <li>{@code Double} ==&gt; {@code double}</li>
   * <li>{@code Float} ==&gt; {@code float}</li>
   * <li>{@code Integer} ==&gt; {@code int}</li>
   * <li>{@code Long} ==&gt; {@code long}</li>
   * <li>{@code Short} ==&gt; {@code short}</li>
   * </ul>
   *
   * @param wrapper the wrapper class for which the corresponding primitive
   *                type should be returned.
   * @return the corresponding primitive type.
   * @throws IllegalArgumentException if the given class is not a wrapper class.
   */
  PrimitiveType unboxedType(TypeMirror wrapper);

}
