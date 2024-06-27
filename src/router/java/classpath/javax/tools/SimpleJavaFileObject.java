/* SimpleJavaFileObject.java -- Simple implementation of JavaFileObject.
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

package javax.tools;

import java.io.InputStream;
import java.io.IOException;
import java.io.OutputStream;
import java.io.OutputStreamWriter;
import java.io.Reader;
import java.io.StringReader;
import java.io.Writer;

import java.net.URI;

import javax.lang.model.element.Modifier;
import javax.lang.model.element.NestingKind;

/**
 * Provides a simple implementation of many of the
 * {@link JavaFileObject} methods, thus giving a useful basis
 * for a subclass to complete the work.
 *
 * @author Andrew John Hughes (gnu_andrew@member.fsf.org)
 * @since 1.6
 */
public class SimpleJavaFileObject
  implements JavaFileObject
{

  /**
   * The kind of this file object.
   */
  protected Kind kind;

  /**
   * The URI of this file object.
   */
  protected URI uri;

  /**
   * Constructs a new {@link SimpleJavaFileObject}
   * of the specified kind using the supplied URI.
   *
   * @param uri the URI of this file object.
   * @param kind the kind of this file object.
   */
  protected SimpleJavaFileObject(URI uri, Kind kind)
  {
    this.uri = uri;
    this.kind = kind;
  }

  /**
   * This implementation does nothing and always
   * returns {@code false}.
   *
   * @return false.
   */
  @Override
  public boolean delete()
  {
    return false;
  }

  /**
   * This implementation throws {@link UnsupportedOperationException}
   * and should be overridden in the subclass.
   *
   * @param ignoreEncodingErrors whether or not to ignore
   *                             any encoding errors that
   *                             occur.
   * @return a character sequence if available.  Otherwise {@code null}.
   * @throws IOException if an I/O error occurs.
   */
  @Override
  public CharSequence getCharContent(boolean ignoreEncodingErrors)
    throws IOException
  {
    throw new UnsupportedOperationException("getCharContent not implemented.");
  }

  /**
   * Returns a user-friendly name for this object.
   * This implementation simply uses the path of the
   * URI.
   *
   * @return a user-friendly name.
   */
  @Override
  public String getName() 
  {
    return uri.getPath();
  }

  /**
   * Returns the kind of this file object.
   *
   * @return {@code this.kind}
   */
  @Override
  public Kind getKind()
  {
    return kind;
  }

  /**
   * This implementation does nothing and always
   * returns {@code 0L}.
   *
   * @return 0L.
   */
  @Override
  public long getLastModified()
  {
    return 0L;
  }

  /**
   * <p>Returns true if the given kind is the same as the
   * kind of this file object, and the path of this
   * object's URI is equal to the supplied name followed
   * by the extension specified by its kind.</p>
   * <p>This method uses {@link #getKind()} and
   * {@link #toUri()} to retrieve the kind and URI
   * respectively, so subclasses may override the values
   * used.</p>
   *
   * @param simpleName the simple name to compare.
   * @param kind the kind to compare.
   * @return true if the above criteria are met.
   */
  @Override
  public boolean isNameCompatible(String simpleName, Kind kind)
  {
    return getKind().equals(kind) &&
      toUri().getPath().equals(simpleName + kind.extension);
  }

  /**
   * This implementation throws {@link UnsupportedOperationException}
   * and should be overridden in the subclass.
   *
   * @return an input stream.
   * @throws IOException if an I/O error occurs.
   */
  @Override
  public InputStream openInputStream()
    throws IOException
  {
    throw new UnsupportedOperationException("openInputStream not implemented.");
  }

  /**
   * This implementation throws {@link UnsupportedOperationException}
   * and should be overridden in the subclass.
   *
   * @return an output stream.
   * @throws IOException if an I/O error occurs.
   */
  @Override
  public OutputStream openOutputStream()
    throws IOException
  {
    throw new UnsupportedOperationException("openOutputStream not implemented.");
  }

  /**
   * Wraps the result of {@link #getCharContent(boolean)}
   * in a {@link Reader}.
   *
   * @param ignoreEncodingErrors whether or not to ignore
   *                             any encoding errors that
   *                             occur.
   * @return a Reader instance wrapping the result of
   *         {@code getCharContent(ignoreEncodingErrors)}
   * @throws IllegalStateException if this file was opened for writing and
   *         does not support reading.
   * @throws UnsupportedOperationException if this kind of file does not support
   *         character access.
   * @throws IOException if an I/O error occurs.
   */
  @Override
  public Reader openReader(boolean ignoreEncodingErrors)
    throws IOException
  {
    CharSequence content = getCharContent(ignoreEncodingErrors);
    if (content == null)
      throw new IllegalStateException("No character content available.");
    return new StringReader(content.toString());
  }

  /**
   * Wraps the result of {@link #openOutputStream()}
   * in a {@link Writer}.
   *
   * @return a Writer instance wrapping the result
   *         of {@code openOutputStream()}.
   * @throws IllegalStateException if this file was opened for reading and
   *         does not support writing.
   * @throws UnsupportedOperationException if this kind of file does not support
   *         character access.
   * @throws IOException if an I/O error occurs.
   */
  @Override
  public Writer openWriter()
    throws IOException
  {
    return new OutputStreamWriter(openOutputStream());
  }

  /**
   * Returns a {@link URI} identifying this object.
   *
   * @return a URI.
   */
  @Override
  public URI toUri()
  {
    return uri;
  }

  /**
   * This implementation does nothing and always
   * returns {@code null}.
   *
   * @return null.
   */
  @Override
  public Modifier getAccessLevel()
  {
    return null;
  }

  /**
   * This implementation does nothing and always
   * returns {@code null}.
   *
   * @return null.
   */
  @Override
  public NestingKind getNestingKind()
  {
    return null;
  }

}
