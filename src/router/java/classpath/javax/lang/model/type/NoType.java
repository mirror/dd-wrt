/* NoType.java -- Represents a pseudo-type used when nothing is appropriate.
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

package javax.lang.model.type;

/**
 * <p>Represents a pseudo-type used when a {@link TypeMirror}
 * needs to be returned, but there is not one suitable.</p>
 * <p>Further information is provided by the kind of the
 * instance, as follows:</p>
 * <ul>
 * <li>{@link TypeKind#VOID}; used for method return types.</li>
 * <li>{@link TypeKind#PACKAGE}; used for packages.</li>
 * <li>{@link TypeKind#NONE}: used when nothing else fits.
 * For example, requesting a superclass from
 * {@code  java.lang.Object}.</li>
 * </ul>
 *
 * @author Andrew John Hughes (gnu_andrew@member.fsf.org)
 * @since 1.6
 * @see javax.lang.model.element.ExecutableElement#getReturnType()
 */
public interface NoType
  extends TypeMirror
{
}
