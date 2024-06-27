/* AnnotationValue.java -- Represents an annotation value.
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
 * <p>Represents the value of an element in an annotation.  This
 * may be one of the following types:</p>
 * <ul>
 * <li>a wrapper class for a primitive type (e.g. {@code Integer} 
 * for {@code int}.</li>
 * <li>{@code String}</li>
 * <li>{@code TypeMirror}</li>
 * <li>{@code VariableElement} (an enum constant)</li>
 * <li>{@code AnnotationMirror}</li>
 * <li>{@code List<? extends AnnotationValue>} (an array,
 * with elements in the order they were declared)</li>
 * </ol>
 *
 * @author Andrew John Hughes (gnu_andrew@member.fsf.org)
 * @since 1.6
 */
public interface AnnotationValue
{

  /**
   * Applies a visitor to this type.
   *
   * @param <R> the return type of the visitor's methods.
   * @param <P> the type of the additional parameter used in the visitor's
   *            methods.
   * @param visitor the visitor to apply to the element.
   * @param param the additional parameter to be sent to the visitor.
   * @return the return value from the visitor.
   */
  <R,P> R accept(AnnotationValueVisitor<R,P> visitor, P param);

  /**
   * Returns the value.
   *
   * @return the value.
   */
  Object getValue();

  /**
   * Returns a textual representation of the value.
   *
   * @return a textual representation.
   */
  String toString();

}
