/* Objects.java -- Utility methods for working with objects.
   Copyright 2014, 2015 Free Software Foundation, Inc.

This file is a part of GNU Classpath.

GNU Classpath is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or (at
your option) any later version.

GNU Classpath is distributed in the hope that it will be useful, but
WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
General Public License for more details.

You should have received a copy of the GNU General Public License
along with GNU Classpath; if not, write to the Free Software
Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301
USA

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
exception statement from your version.  */

package java.util;

/**
 * This class collects together a number of utility methods
 * for working with objects. It includes implementations of
 * {@link java.lang.Object#hashCode}, {@link java.lang.Object#toString}
 * and {@link java.lang.Object#equals(boolean)} which are
 * safe to use when one or more of the supplied objects
 * are {@code null}.
 *
 * @author Andrew John Hughes (gnu_andrew@member.fsf.org)
 * @since 1.7
 */
public final class Objects
{

  /**
   * <p>
   * Compares the two arguments using the specified
   * {@link Comparator} object and can be used to
   * order such objects. A negative integer is
   * returned if the first argument is less than
   * the second and a positive integer if it is
   * greater. If both arguments are equal (including
   * if both arguments are {@code null}), zero is
   * returned.
   * </p>
   * <p>
   * The method may throw a {@link NullPointerException}
   * if one argument is {@code null} but not the other.
   * This behaviour depends on the behaviour of the
   * {@code Comparator} object supplied.
   * </p>
   *
   * @param T the type of objects to compare.
   * @param a the first object.
   * @param b the object to compare to {@code a}.
   * @param c the comparator to use to compare the objects.
   * @return 0 if the objects are equal, {@code c.compare(a,b)}
   *         otherwise.
   * @see java.lang.Comparable
   * @see java.util.Comparator
   */
  public static <T> int compare(T a, T b, Comparator<? super T> c)
  {
    if (a == b)
      return 0;
    return c.compare(a,b);
  }

  /**
   * <p>
   * Returns true if the two arguments are <emph>deeply equal</emph>
   * to one another. Deep equality is defined as follows:
   * </p>
   * <ul>
   * <li>If both objects are {@code null}, the result is {@code true}.</li>
   * <li>If one object is {@code null} and the other is not, the result is
   * false.</li>
   * <li>If both objects are arrays of primitive types, the result is
   * equal to {@code Arrays.equals(a,b)}.</li>
   * <li>If both objects are arrays of {@code Object}s, the result is equal to
   * {@code Arrays.deepEquals(a,b)}.</li>
   * <li>Otherwise, the result is equal to {@code a.equals(b)}.</li>
   * </ul>
   *
   * @param a the first object.
   * @param b the object to be compared with {@code b}.
   * @return true if the objects are deeply equal, as defined above.
   * @see java.util.Arrays#deepEquals(Object[],Object[])
   * @see java.lang.Object#equals(Object)
   * @see #equals(Object,Object)
   */
  public static boolean deepEquals(Object a, Object b)
  {
    if (a == null && b == null)
      return true;
    if (a == null || b == null)
      return false;

    Class<?> aClass = a.getClass();
    Class<?> bClass = b.getClass();
    if (aClass.isArray() && bClass.isArray())
      {
        if (aClass == bClass)
          {
            if (aClass == boolean[].class)
              return Arrays.equals((boolean[]) a, (boolean[]) b);
            if (aClass == byte[].class)
              return Arrays.equals((byte[]) a, (byte[]) b);
            if (aClass == char[].class)
              return Arrays.equals((char[]) a, (char[]) b);
            if (aClass == double[].class)
              return Arrays.equals((double[]) a, (double[]) b);
            if (aClass == float[].class)
              return Arrays.equals((float[]) a, (float[]) b);
            if (aClass == int[].class)
              return Arrays.equals((int[]) a, (int[]) b);
            if (aClass == long[].class)
              return Arrays.equals((long[]) a, (long[]) b);
            if (aClass == short[].class)
              return Arrays.equals((short[]) a, (short[]) b);
          }
        if (Object[].class.isAssignableFrom(a.getClass()) &&
            Object[].class.isAssignableFrom(b.getClass()))
          return Arrays.deepEquals((Object[]) a, (Object[]) b);
      }

    return equals(a, b);
  }

  /**
   * <p>
   * Returns true if the two arguments are <emph>equal</emph>
   * to one another. Equality is defined as follows:
   * </p>
   * <ul>
   * <li>If both objects are {@code null}, the result is {@code true}.</li>
   * <li>If one object is {@code null} and the other is not, the result is
   * false.</li>
   * <li>Otherwise, the result is equal to {@code a.equals(b)}.</li>
   * </ul>
   *
   * @param a the first object.
   * @param b the object to be compared with {@code b}.
   * @return true if the objects are deeply equal, as defined above.
   * @see java.util.Arrays#deepEquals(Object[],Object[])
   * @see java.lang.Object#equals(Object)
   * @see #equals(Object,Object)
   */
  public static boolean equals(Object a, Object b)
  {
    if (a == null && b == null)
      return true;
    if (a == null || b == null)
      return false;

    return a.equals(b);
  }

  /**
   * <p>
   * Generates a hash code from the specified input values.
   * The hash code generated is the same as if each value
   * was added to an array of {@code Object}s and the
   * method {@code Arrays.hashCode(Object[])} run on this
   * array. In turn, this is the same as if
   * {@code Arrays.asList} was run on the array and the
   * result obtained from the {@code hashCode()} method
   * of the resulting {@code List} object.
   * </p>
   * <p>As a consequence, this means that, for a single
   * object, {@code x}, the value returned by this method
   * is not equal to {@code x.hashCode()} or
   * {@code hashCode(x)}, but to
   * <code>Arrays.hashCode(new Object[]{x})}</code>.
   * </p>
   *
   * @param values the values to compute the hash code of.
   * @return the hash code of the sequence of values.
   * @see java.util.Arrays#hashCode(Object[])
   * @see java.util.Arrays#asList(Object[])
   * @see #hashCode(Object)
   */
  public static int hash(Object... values)
  {
    return Arrays.hashCode(values);
  }

  /**
   * Returns the hash code of the supplied argument
   * if it is not {@code null}, or 0 if it is.
   *
   * @param o the object to obtain the hash code of.
   * @return the hash code of the object, or 0 if
   *         the object was {@code null}.
   */
  public static int hashCode(Object o)
  {
    if (o == null)
      return 0;
    return o.hashCode();
  }

  /**
   * Checks that the supplied argument is not
   * {@code null}. If it is, then a
   * {@link NullPointerException} is thrown.
   * This method is designed as an aid for validating
   * input parameters to a method.
   *
   * @param o the object to check.
   * @return the object if it is not {@code null}.
   * @throws NullPointerException if the object is
   *         {@code null}.
   */
  public static <T> T requireNonNull(T o)
  {
    if (o == null)
      throw new NullPointerException();
    return o;
  }

  /**
   * Checks that the supplied argument is not
   * {@code null}. If it is, then a
   * {@link NullPointerException} is thrown
   * using the specified error message.
   * This method is designed as an aid for validating
   * input parameters to a method.
   *
   * @param o the object to check.
   * @param message the error message to use.
   * @return the object if it is not {@code null}.
   * @throws NullPointerException if the object is
   *         {@code null}.
   */
  public static <T> T requireNonNull(T o, String message)
  {
    if (o == null)
      throw new NullPointerException(message);
    return o;
  }

  /**
   * Returns the {@link Object#toString()} output of
   * the specified object, or {@code "null"} if the
   * object is {@code null}.
   *
   * @param o the object on which to call {@code toString()}.
   * @return the output of {@code o.toString()} or
   *         {@code "null"} if the object was {@code null}.
   */
  public static String toString(Object o)
  {
    if (o == null)
      return "null";
    return o.toString();
  }

  /**
   * Returns the {@link Object#toString()} output of
   * the specified object, or the supplied default
   * string if the object is {@code null}.
   *
   * @param o the object on which to call {@code toString()}.
   * @param defStr the default string to return if the
   *        object is {@code null}.
   * @return the output of {@code o.toString()} or
   *         {@code defStr} if the object was {@code null}.
   */
  public static String toString(Object o, String defStr)
  {
    if (o == null)
      return defStr;
    return o.toString();
  }

}
