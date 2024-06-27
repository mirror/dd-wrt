/* SourceVersion.java -- Source versions of the Java programming language.
   Copyright (C) 2012, 2013  Free Software Foundation, Inc.

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

package javax.lang.model;

import java.util.Arrays;

/**
 * Source versions of the Java programming language.
 * Note that this will be extended with additional
 * constants to represent new versions.
 *
 * @author Andrew John Hughes (gnu_andrew@member.fsf.org)
 * @since 1.6
 */
public enum SourceVersion
{
  /** The original version of Java. */ RELEASE_0,
  /** Java 1.1 */ RELEASE_1,
  /** Java 1.2 */ RELEASE_2,
  /** Java 1.3 */ RELEASE_3,
  /** Java 1.4 */ RELEASE_4,
  /** Java 5 */ RELEASE_5,
  /** Java 6 */ RELEASE_6,
  /** Java 7 @since 1.7 */ RELEASE_7;

  /** List of language keywords, as specified in 3.9 of the Java
   * Language Specification.  Please keep sorted. */
  private static final String[] KEYWORDS = { "abstract", "assert", "boolean",
					     "break", "byte", "case", "catch",
					     "char", "class", "const", "continue",
					     "default", "do", "double", "else",
					     "enum", "extends", "final", "finally",
					     "float", "for", "if", "goto", "implements",
					     "import", "instanceof", "int", "interface",
					     "long", "native", "new", "package", "private",
					     "protected", "public", "return", "short",
					     "static", "strictfp", "super", "switch",
					     "synchronized", "this", "throw", "throws",
					     "transient", "try", "void", "volatile",
					     "while" };

  /**
   * Returns true if {@code name} is a syntactically valid identifier or
   * keyword in the latest version of the language.  That is, this
   * method returns true if the {@link Character#isJavaIdentifierStart(int)}
   * holds for the first character and {@link Character#isJavaIdentifierPart(int)}
   * for the rest.  This matches all regular identifiers, keywords
   * and the literals {@code true}, {@code false} and {@code null}.
   *
   * @param name the name to check.
   * @return true if {@code name} represents a valid identifier, keyword or literal.
   */
  public static boolean isIdentifier(CharSequence name)
  {
    int size = name.length();
    if (size > 0 && Character.isJavaIdentifierStart(name.charAt(0)))
      {
	for (int a = 1; a < size; ++a)
	  if (!Character.isJavaIdentifierPart(name.charAt(a)))
	    return false;
	return true;
      }
    return false;
  }

  /**
   * Returns the latest source version that can be modeled.
   *
   * @return the latest modelable source version.
   */
  public static SourceVersion latest()
  {
    return RELEASE_6;
  }

  /**
   * Returns the latest source version fully supported
   * by GNU Classpath.  Must be at least {@code RELEASE_5}.
   *
   * @return the latest supported source version.
   */
  public static SourceVersion latestSupported()
  {
    return RELEASE_5;
  }

  /**
   * Returns true if the specified character sequence
   * represents a keyword or one of the literals:
   * {@code "null"}, {@code "true"} or {@code "false"}.
   *
   * @param string the string to check.
   * @return true if {@code string} represents a valid keyword or literal.
   */
  public static boolean isKeyword(CharSequence string)
  {
    if (Arrays.binarySearch(KEYWORDS, string) >= 0)
      return true;
    if (string.equals("true") || string.equals("false") ||
	string.equals("null"))
      return true;
    return false;
  }

  /**
   * Returns true if the specified character sequence
   * represents a qualified name such as {@code "java.lang.Object"}
   * or {@code "String"}.  Unlike {@link #isIdentifier(CharSequence)},
   * this method does not return {@code true} for keywords
   * and literals.
   *
   * @param string the string to check.
   * @return true if {@code string} represents a valid qualified name.
   */
  public static boolean isName(CharSequence string)
  {
    int dotLocation = -1;
    int length = string.length();

    // Find a '.' if there is one
    for (int a = 0; a < length; ++a)
      if (string.charAt(a) == '.')
	dotLocation = a;
    
    if (dotLocation == -1)
      return isIdentifier(string) && !isKeyword(string);

    CharSequence identifier = string.subSequence(dotLocation + 1, length);
    return (isName(string.subSequence(0, dotLocation)) &&
	    (isIdentifier(identifier) && !isKeyword(identifier)));
  }

}
