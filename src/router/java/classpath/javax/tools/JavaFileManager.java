/* JavaFileManager.java -- File manager for source & class file tools.
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

import java.io.Closeable;
import java.io.Flushable;
import java.io.IOException;

import java.util.Iterator;
import java.util.Set;

/**
 * <p>File manager for tools which operate on source and class files.
 * In this context, the term {@code file} is used to refer to the abstract
 * concept of a data source, rather than specifically a regular file on
 * a filesystem.</p>
 * <p>The file manager determines where to create new {@link FileObject}s.
 * It may have a default directory where files are created.  The user may
 * provide hints as to how to perform file creation, but the file manager
 * may choose to ignore these.</p>
 * <p>For methods in this interface which use class names, these must
 * be given in the internal virtual machine form of fully qualified
 * class and interface names.  The use of '/' and '.' are interchangeable,
 * making {@code java.lang.Object}, {@code java/lang.Object} and
 * {@code java/lang/Object} all equivalent.</p>
 * <p>All names should be treated as being case-sensitive.  If the underlying
 * system is not case-aware, steps should be taken to handle this in the
 * implementation, such as the use of {@link java.io.File#getCanonicalFile()}
 * to preserve case.</p>
 * <p>For methods in this interface which use relative names, these are
 * path separated by {@code '/'} and do not include the
 * segments {@code '.'} and {@code '..'}, so that they may only
 * refer to subdirectories.  A valid relative name must match
 * the "path-rootless" rule of RFC 3986, section 3.3.  The construction
 * {@code URI.create(name).normalize().getPath().equals(name)} should hold.</p>
 * <p>All methods may throw a {@link SecurityException}.  All methods may
 * throw a {@link NullPointerException} if an argument is null, unless
 * otherwise specified.  It is not required that the implementation support
 * concurrent access to the file manager, but the file objects created by
 * it should be thread-safe.</p>
 *
 * @author Andrew John Hughes (gnu_andrew@member.fsf.org)
 * @since 1.6
 */
public interface JavaFileManager
  extends Closeable, Flushable, OptionChecker
{

  /**
   * Interface for obtaining the location of {@link FileObject}s.
   * Used by the file manager to know where to create or locate them.
   *
   * @author Andrew John Hughes (gnu_andrew@member.fsf.org)
   * @since 1.6
   */
  public static interface Location
  {
    /**
     * Returns the name of this location.
     *
     * @return the name.
     */
    String getName();

    /**
     * Returns true if this location is used for output.
     *
     * @return true if the location is used for output.
     */
    boolean isOutputLocation();
  }

  /**
   * Closes the file manager and releases any resources in
   * use.  Calling {@code close()} on an already closed
   * file manager has no effect.  The effect of calling
   * other methods on a closed file manager is undefined,
   * unless specified in the documentation of that method.
   *
   * @throws IOException if an I/O error occurs.
   * @see #flush()
   */
  void close() throws IOException;

  /**
   * Flushes data to any resources controlled by this file
   * manager.  Flushing a closed file manager has no effect.
   *
   * @throws IOException if an I/O error occurs.
   * @see #close()
   */
  void flush() throws IOException;

  /**
   * Returns the class loader that is responsible for loading
   * class files from the specified location.  For example,
   * {@code getClassLoader(StandardLocation.ANNOTATION_PROCESSOR_PATH)}
   * will return the class loader which is used to load
   * annotation processors.
   *
   * @param location the location to retrieve the class loader for.
   * @return the class loader for class files in that location, or
   *         {@code null} if the location is unknown or class loading
   *         from that location is disabled.
   * @throws SecurityException if the class loader is not retrievable
   *                           under the current security manager.
   * @throws IllegalStateException if {@link #close()} has been called
   *                               and the file manager can't be reopened.
   */
  ClassLoader getClassLoader(Location location);

  /**
   * <p>Returns a file object for reading a file.  If this
   * is a source or class file, the returned instance
   * must be a {@link JavaFileObject}.  The file name is
   * constructed as a concatenation of the location, the
   * package name and the relative file name.</p>
   * <p>For example, for the call
   * {@ocode getFileForInput(StandardLocation.SOURCE_PATH,
   * "gnu.classpath.util", "CoolCode.java")}, assuming
   * that the source path is set to
   * {@code "/home/bob/sources"}, the returned file object
   * would represent the file
   * {@code "/home/bob/sources/gnu/classpath/util/CoolCode.java"}.</p>
   *
   * @param location the base location for the file.
   * @param pkg the package the file will be read from.
   * @param relName the path to the file, relative to {@code location+pkg}.
   * @return a file object or may return {@code null} if the file does not exist.
   * @throws IllegalArgumentException if the location is unknown and unknown locations
   *                                  are not supported, or if the relative name is invalid.
   * @throws IOException if an I/O error occurs, or the file manager
   *                     has been closed and can't be reopened.
   * @throws IllegalStateException if the file manager has been closed and can't be reopened.
   */
  FileObject getFileForInput(Location location, String pkg, String relName)
    throws IOException;

  /**
   * <p>Returns a file object for writing a file.  If this
   * is a source or class file, the returned instance
   * must be a {@link JavaFileObject}.  The file name is
   * constructed as a concatenation of the location, the
   * package name and the relative file name.</p>
   * <p>For example, for the call
   * {@ocode getFileForOutput(StandardLocation.SOURCE_OUTPUT,
   * "gnu.classpath.util", "GenSyms.java")}, assuming
   * that the source output is set to
   * {@code "/home/bob/generated"}, the returned file object
   * would represent the file
   * {@code "/home/bob/generated/gnu/classpath/util/GenSyms.java"}.</p>
   * <p>The file manager may optionally use the supplied
   * sibling as a hint as to where to place the output file.
   * The exact semantics of this hint are left to the implementation.
   * As an example, the compiler places class files in the same location
   * as the corresponding source file, if no destination is specified.
   * To facilitate this, the source file location may be passed as
   * a hint to the file manager.</p>
   *
   * @param location the base location for the file.
   * @param pkg the package in which the file will be stored.
   * @param relName the path to the file, relative to {@code location+pkg}.
   * @param sibling an optional hint as to where to place the output file.
   *                May be {@code null}.
   * @return a file object.
   * @throws IllegalArgumentException if the location is unknown and unknown locations
   *                                  are not supported, if the relative name is invalid or
   *                                  if the sibling is not known to this file manager.
   * @throws IOException if an I/O error occurs, or the file manager
   *                     has been closed and can't be reopened.
   * @throws IllegalStateException if the file manager has been closed and can't be reopened.
   */
  FileObject getFileForOutput(Location location, String pkg, String relName,
			      FileObject sibling)
    throws IOException;

  /**
   * <p>Returns a {@link JavaFileObject} for reading
   * a source file or class file.  The file name is
   * constructed as a concatenation of the location
   * and the information provided by the type name
   * and the kind of the file.</p>
   * <p>For example, for the call
   * {@ocode getJavaFileForInput(StandardLocation.SOURCE_PATH,
   * "gnu.classpath.util.CoolCode", JavaFileObject.Kind.SOURCE)},
   * assuming that the source path is set to
   * {@code "/home/bob/sources"}, the returned file object
   * would represent the file
   * {@code "/home/bob/sources/gnu/classpath/util/CoolCode.java"}.</p>
   *
   * @param location the base location for the file.
   * @param className the name of the class.
   * @param kind the kind of file, either {@link JavaFileObject.Kind.SOURCE} or
   *             {@link JavaFileObject.Kind.CLASS}.
   * @return a file object or may return {@code null} if the file does not exist.
   * @throws IllegalArgumentException if the location is unknown and unknown locations
   *                                  are not supported, or if the kind is invalid.
   * @throws IOException if an I/O error occurs, or the file manager
   *                     has been closed and can't be reopened.
   * @throws IllegalStateException if the file manager has been closed and can't be reopened.
   */
  JavaFileObject getJavaFileForInput(Location location, String className, JavaFileObject.Kind kind)
    throws IOException;

  /**
   * <p>Returns a {@link JavaFileObject} for writing a
   * source or class file.  The file name is
   * constructed as a concatenation of the location and
   * the information provided by the type name and the
   * kind of the file.</p>
   * <p>For example, for the call
   * {@ocode getJavaFileForOutput(StandardLocation.CLASS_OUTPUT,
   * "gnu.classpath.util.CoolCode", JavaFileObject.Kind.CLASS)}, assuming
   * that the class output is set to
   * {@code "/home/bob/build"}, the returned file object
   * would represent the file
   * {@code "/home/bob/build/gnu/classpath/util/GenSyms.class"}.</p>
   * <p>The file manager may optionally use the supplied
   * sibling as a hint as to where to place the output file.
   * The exact semantics of this hint are left to the implementation.
   * As an example, the compiler places class files in the same location
   * as the corresponding source file, if no destination is specified.
   * To facilitate this, the source file location may be passed as
   * a hint to the file manager.</p>
   *
   * @param location the base location for the file.
   * @param className the name of the class.
   * @param kind the kind of file, either {@link JavaFileObject.Kind.SOURCE} or
   *             {@link JavaFileObject.Kind.CLASS}.
   * @param sibling an optional hint as to where to place the output file.
   *                May be {@code null}.
   * @return a file object.
   * @throws IllegalArgumentException if the location is unknown and unknown locations
   *                                  are not supported, if the kind is invalid or
   *                                  if the sibling is not known to this file manager.
   * @throws IOException if an I/O error occurs, or the file manager
   *                     has been closed and can't be reopened.
   * @throws IllegalStateException if the file manager has been closed and can't be reopened.
   */
  JavaFileObject getJavaFileForOutput(Location location, String className,
				      JavaFileObject.Kind kind, FileObject sibling)
    throws IOException;

  /**
   * Processes one option.  If the specified option
   * is supported by this file manager, it will read
   * any necessary arguments to the option from the
   * iterator and then return {@code true}.  Otherwise, it
   * will return {@code false}.
   *
   * @param option the option to process.
   * @param remaining the remaining arguments/options.
   * @return true if the option was handled.
   * @throws IllegalArgumentException if the option is used incorrectly.
   * @throws IllegalStateException if the file manager has been closed and can't be reopened.
   */
  boolean handleOption(String option, Iterator<String> remaining);

  /**
   * Returns {@code true} if the specified location
   * is known to this file manager.
   *
   * @param location the location to check.
   * @return true if the location is known.
   */
  boolean hasLocation(Location location);

  /**
   * Infers a binary name for the given file object,
   * based on its location.  The returned name may
   * not be a valid binary name, according to the
   * Java language specification.
   *
   * @param location the location to use as a basis.
   * @param file the file object.
   * @return a binary name or {@code null} if the file
   *         object is not found in the given location.
   * @throws IllegalStateException if the file manager has been closed and can't be reopened.
   */
  String inferBinaryName(Location location, JavaFileObject file);

  /**
   * Returns true if the two given file objects represent
   * the same file.
   *
   * @param x the first object.
   * @param y the second object.
   * @return true if {@code x} and {@code y} represent the
   *         same file.
   * @throws IllegalArgumentException if either {@code x} or
   *         {@code y} were created by a foreign file manager and
   *         this file manager does not support such comparisons.
   */
  boolean isSameFile(FileObject x, FileObject y);

  /**
   * Lists all file objects matching the given criteria in the
   * specified location.  If {@code recurse} is true, the list
   * will include those found in subpackages.  Note that a file
   * manager may not return a {@code null} list or throw an
   * exception if the location is unknown.
   *
   * @param location the location to search.
   * @param pkg the name of the package to search.
   * @param kinds the kinds of object to return.
   * @param recurse {@code true} if subpackages should be searched.
   * @return an {@link Iterable} over the matching file object.
   * @throws IOException if an I/O error occurs, or the file manager
   *                     has been closed and can't be reopened.
   * @throws IllegalStateException if the file manager has been closed and can't be reopened.
   */
  Iterable<JavaFileObject> list(Location location, String pkg,
				Set<JavaFileObject.Kind> kinds, boolean recurse)
    throws IOException;

}
