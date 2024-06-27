/* ForwardingFileManager.java -- File manager that forwards to another.
   Copyright (C) 2013  Free Software Foundation, Inc.

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

import java.io.IOException;

import java.util.Iterator;
import java.util.Set;

/**
 * Forwards calls to a specified file manager.
 *
 * @param <M> the kind of file manager calls are forwarded to.
 * @author Andrew John Hughes (gnu_andrew@member.fsf.org)
 * @since 1.6
 */
public class ForwardingJavaFileManager<M extends JavaFileManager>
  implements JavaFileManager
{

  /**
   * The file manager to delegate to.
   */
  protected M fileManager;

  /**
   * Creates a new forwarder, delegating calls to the
   * specified file manager.
   *
   * @param manager the manager to forward calls to.
   */
  protected ForwardingJavaFileManager(M manager)
  {
    fileManager = manager;
  }

  /**
   * @inheritDoc
   */
  @Override
  public void close() throws IOException
  {
    fileManager.close();
  }

  /**
   * @inheritDoc
   */
  @Override
  public void flush() throws IOException
  {
    fileManager.flush();
  }

  /**
   * @inheritDoc
   */
  @Override
  public ClassLoader getClassLoader(JavaFileManager.Location loc)
  {
    return fileManager.getClassLoader(loc);
  }

  /**
   * @inheritDoc
   */
  @Override
  public FileObject getFileForInput(JavaFileManager.Location loc,
				    String pkgName, String relativeName)
    throws IOException
  {
    return fileManager.getFileForInput(loc, pkgName, relativeName);
  }

  /**
   * @inheritDoc
   */
  @Override
  public FileObject getFileForOutput(JavaFileManager.Location loc,
				     String pkgName, String relativeName,
				     FileObject sibling)
    throws IOException
  {
    return fileManager.getFileForOutput(loc, pkgName, relativeName,
					sibling);
  }

  /**
   * @inheritDoc
   */
  @Override
  public JavaFileObject getJavaFileForInput(JavaFileManager.Location loc,
					    String className,
					    JavaFileObject.Kind kind)
    throws IOException
  {
    return fileManager.getJavaFileForInput(loc, className, kind);
  }

  /**
   * @inheritDoc
   */
  @Override
  public JavaFileObject getJavaFileForOutput(JavaFileManager.Location loc,
					     String className,
					     JavaFileObject.Kind kind,
					     FileObject sibling)
    throws IOException
  {
    return fileManager.getJavaFileForOutput(loc, className, kind,
					    sibling);
  }

  /**
   * @inheritDoc
   */
  @Override
  public boolean handleOption(String current, Iterator<String> remaining)
  {
    return fileManager.handleOption(current, remaining);
  }

  /**
   * @inheritDoc
   */
  @Override
  public boolean hasLocation(JavaFileManager.Location loc)
  {
    return fileManager.hasLocation(loc);
  }

  /**
   * @inheritDoc
   */
  @Override
  public String inferBinaryName(JavaFileManager.Location loc,
				JavaFileObject fileObj)
  {
    return fileManager.inferBinaryName(loc, fileObj);
  }

  /**
   * @inheritDoc
   */
  @Override
  public boolean isSameFile(FileObject a, FileObject b)
  {
    return fileManager.isSameFile(a, b);
  }

  /**
   * @inheritDoc
   */
  @Override
  public int isSupportedOption(String option)
  {
    return fileManager.isSupportedOption(option);
  }

  /**
   * @inheritDoc
   */
  @Override
  public Iterable<JavaFileObject> list(JavaFileManager.Location loc,
				       String pkgName,
				       Set<JavaFileObject.Kind> kinds,
				       boolean recurse)
    throws IOException
  {
    return fileManager.list(loc, pkgName, kinds, recurse);
  }

}
