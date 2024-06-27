/* ForwardingFileObject.java -- File object that forwards to another.
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
import java.io.Reader;
import java.io.Writer;

import java.net.URI;

/**
 * Forwards calls to a specified file object.
 *
 * @param <F> the kind of object calls are forwarded to.
 * @author Andrew John Hughes (gnu_andrew@member.fsf.org)
 * @since 1.6
 */
public class ForwardingFileObject<F extends FileObject>
  implements FileObject
{

  /**
   * The file object to delegate to.
   */
  protected F fileObject;

  /**
   * Creates a new forwarder, delegating calls to the
   * specified object.
   *
   * @param obj the object to forward calls to.
   */
  protected ForwardingFileObject(F obj)
  {
    fileObject = obj;
  }

  /**
   * @inheritDoc
   */
  @Override
  public boolean delete()
  {
    return fileObject.delete();
  }

  /**
   * @inheritDoc
   */
  @Override
  public CharSequence getCharContent(boolean ignoreEncodingErrors)
    throws IOException
  {
    return fileObject.getCharContent(ignoreEncodingErrors);
  }

  /**
   * @inheritDoc
   */
  @Override
  public long getLastModified()
  {
    return fileObject.getLastModified();
  }

  /**
   * @inheritDoc
   */
  @Override
  public String getName()
  {
    return fileObject.getName();
  }

  /**
   * @inheritDoc
   */
  @Override
  public InputStream openInputStream()
    throws IOException
  {
    return fileObject.openInputStream();
  }

  /**
   * @inheritDoc
   */
  @Override
  public OutputStream openOutputStream()
    throws IOException
  {
    return fileObject.openOutputStream();
  }

  /**
   * @inheritDoc
   */
  @Override
  public Reader openReader(boolean ignoreEncodingErrors)
    throws IOException
  {
    return fileObject.openReader(ignoreEncodingErrors);
  }

  /**
   * @inheritDoc
   */
  @Override
  public Writer openWriter()
    throws IOException
  {
    return fileObject.openWriter();
  }
  
  /**
   * @inheritDoc
   */
  @Override
  public URI toUri()
  {
    return fileObject.toUri();
  }

}
