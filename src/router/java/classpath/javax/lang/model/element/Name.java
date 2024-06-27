/* Name.java -- An immutable sequence of characters.
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
 * <p>An immutable sequence of characters, representing a name.
 * An empty name has a length of zero.</p>
 * <p>Names created by the same implementation are comparable
 * using {@link #equals(Object)} and thus usable in a collection.
 * This guarantee as regards the "same implementation" also
 * applies to successive annotation rounds in the context of
 * annotation processing.</p>
 *
 * @author Andrew John Hughes (gnu_andrew@member.fsf.org)
 * @since 1.6
 * @see javax.lang.model.util.Elements#getName(CharSequence)
 */
public interface Name
  extends CharSequence
{

  /**
   * Returns true if the sequence of characters within this
   * name is the same as the specified sequence of characters.
   *
   * @param chars the sequence of characters to compare.
   * @return true if {@code chars} represents the same sequence
   *         of characters as this name.
   * @see String#contentEquals(CharSequence)
   */
  boolean contentEquals(CharSequence chars);

  /**
   * Returns {@code true} if the specified object represents the
   * same name as this instance.  Name comparisons are not
   * just reliant on the content of the name (see
   * {@link #contentEquals(CharSequence)}) but also
   * the implementation which created the name.
   *
   * @param obj the object to compare.
   * @return {@code true} if {@code this} and {@code obj} represent
   *         the same element.
   */
  boolean equals(Object obj);

  /**
   * Obeys the general contract of {@link java.lang.Object#hashCode()}.
   *
   * @return a hash code for this element.
   * @see #equals(Object)
   */
  int hashCode();

}

