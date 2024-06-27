/* StandardJavaFileManager.java -- File manager for regular files.
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

import java.io.File;
import java.io.IOException;

/**
 * <p>File manager for working with regular files or entries
 * within file system containers, such as zip files.  An
 * implementation of this interface is usually retrieved using the
 * {@link javax.tools.JavaCompiler#getStandardFileManager(DiagnosticListener,Locale,Charset)}
 * of {@link javax.tools.JavaCompiler}, an instance of which
 * is, in turn, returned from
 * {@link javax.tools.ToolProvider#getSystemJavaCompiler()}.</p>
 * <p>Regular file objects returned from a file manager implementing this
 * interface must meet the following criteria:</p>
 * <ul>
 * <li>{@link FileObject#delete()} is equivalent to
 * {@link java.io.File#delete()}.</li>
 * <li>{@link FileObject#getLastModified} is equivalent to
 * {@link java.io.File#lastModified}.</li>
 * <li>Methods for reading input ({@link FileObject#getCharContent(boolean)},
 * {@link FileObject#openInputStream()},
 * {@link FileObject#openReader(boolean)}) must succeed,
 * ignoring character encoding issues, if
 * {@code new FileInputStream(new File(fileObj.toUri()))}.</li>
 * <li>Methods for writing output ({@link FileObject#openOutputStream()},
 * {@link FileObject.openWriter()}) must succeed,
 * ignoring character encoding issues, if
 * {@code new FileOutputStream(new File(fileObj.toUri()))}.</li>
 * <li><p>The URI returned from {@link FileObject#toUri()} must
 * be have a schema and a normalised path component, which can
 * be resolved without regard to context (i.e. it must be absolute,
 * not relative to a particular directory).  For example, the
 * URI {@code file:///home/bob/sources} would be valid, while
 * {@code file:sources} (not absolute) or
 * {@code file:///home/bob/lib/../sources} (not normalised) would
 * not.</li>
 * </ul>
 * <p>The file names used by the file objects need not be canonical.</p>
 *
 * @author Andrew John Hughes (gnu_andrew@member.fsf.org)
 * @since 1.6
 */
public interface StandardJavaFileManager
  extends JavaFileManager
{

  /**
   * Returns file objects representing the given files.  This is
   * a convenience method equivalent to calling
   * {@code getJavaFileObjectsFromFiles(Arrays.asList(files))}.
   *
   * @param files the files to retrieve objects for.
   * @return a list of file objects matching the specified array.
   * @throws IllegalArgumentException if the array contains a directory.
   * @throws NullPointerException if one of the array elements is {@code null}.
   */
  Iterable<? extends JavaFileObject> getJavaFileObjects(File... files);

  /**
   * Returns file objects representing the given file names.  This is
   * a convenience method equivalent to calling
   * {@code getJavaFileObjectsFromString(Arrays.asList(fileNames))}.
   *
   * @param fileNames the file names to retrieve objects for.
   * @return a list of file objects matching the specified array.
   * @throws IllegalArgumentException if the array contains a directory.
   * @throws NullPointerException if one of the array elements is {@code null}.
   */
  Iterable<? extends JavaFileObject> getJavaFileObjects(String... files);

  /**
   * Returns file objects representing the given files.
   *
   * @param files the files to retrieve objects for.
   * @return a list of file objects.
   * @throws IllegalArgumentException if the array contains a directory.
   */
  Iterable<? extends JavaFileObject> getJavaFileObjectsFromFiles(Iterable<? extends File> files);

  /**
   * Returns file objects representing the given file names.
   *
   * @param fileNames the file names to retrieve objects for.
   * @return a list of file objects.
   * @throws IllegalArgumentException if the array contains a directory.
   */
  Iterable<? extends JavaFileObject> getJavaFileObjectsFromStrings(Iterable<String> files);

  /**
   * Returns the list of paths associated with the specified location or
   * {@code null} if there is no such mapping.  Output locations only
   * return a single path, representing the output directory.
   *
   * @param location the location whose path should be retrieved.
   * @return the associated list of paths.
   * @see #setLocation(Location,Iterable)
   */
  Iterable<? extends File> getLocation(Location location);

  /**
   * Returns true if the two file objects represent the same
   * file.
   *
   * @param objA the first file object to compare.
   * @param objB the second file object to compare.
   * @return true if the two objects represent the same file.
   * @throws IllegalArgumentException if either object was created
   *                                  by a different implementation.
   */
  boolean isSameFile(FileObject objA, FileObject objB);

  /**
   * Associates the given list of paths with the specified location,
   * discarding any previous mapping.  If the list is {@code null},
   * the mapping is reset to the default.
   *
   * @param location the location which will refer to this list of paths.
   * @param paths a list of paths to associate with the location, or
   *              {@code null} to trigger a reset to the default list.
   * @throws IllegalArgumentException if the location is an output location
   *                                  and the list contains more than one path.
   * @throws IOException if the location is an output location but
   *                     the list contains a path that isn't a directory.
   * @see #getLocation(Location)
   */
  void setLocation(Location location, Iterable<? extends File> paths)
    throws IOException;
}
