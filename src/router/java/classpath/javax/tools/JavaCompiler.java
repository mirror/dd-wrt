/* JavaCompiler.java -- Programmatic interface for a compiler.
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

import java.io.Writer;

import java.nio.charset.Charset;

import java.util.Locale;

import java.util.concurrent.Callable;

import javax.annotation.processing.Processor;

/**
 * <p>
 * Provides a programmatic interface to a compiler for the Java
 * programming language.  The compiler relies on two services;
 * a diagnostic listener for producing textual output including
 * warnings or errors from the code, and a file manager for providing
 * access to the files to be compiled and the output location.
 * More detail on these services and various helper classes related
 * to them is provided below.  The classes {@link DiagnosticListener},
 * {@link JavaFileManager}, {@link FileObject} and {@link JavaFileObject}
 * provide the service provider interface for these services and are not
 * intended for user consumption.</p>
 * <p>Compilers implementing {@link JavaCompiler} must conform to the Java
 * Language Specification and produce class files conforming to the
 * Java Virtual Machine Specification.  In addition, those compilers
 * which claim to support {@link SourceVersion#RELEASE_6} or later
 * must support annotation processing.</p>
 * <h2>Diagnostics</h2>
 * <p>Where possible, the compiler provides diagnostic output to
 * a {@link DiagnosticListener} if provided.  If a listener is
 * not provided, diagnostics are instead written to the default
 * output ({@code{System.err} by default).  This may also happen
 * where the diagnostics are inappropriate for a {@link Diagnostic}
 * object.  The class {@link DiagnosticCollector} provides a means
 * collate together multiple {@link Diagnostic}s in a list.</p>
 * <h2>The File Manager</h2>
 * <p>A compiler tool has an associated standard file manager which
 * is native to it. An instance of it can be obtained by calling
 * {@link #getStandardFileManager(DiagnosticListener,Locale,Charset)}
 * and this is automatically called if no file manager is supplied to
 * the compiler.  However, the compiler must work with other file
 * managers, provided they meet the requirements detailed in the methods
 * below.  Such file managers allow the user to customise how the compiler
 * reads and writes files, and can be shared between multiple compilation
 * tasks.  Such reuse between different tasks can potentially provide
 * a performance improvement.</p>
 * <p>The standard file manager returned by this interface is an instance
 * of the subinterface, {@link StandardJavaFileManager}, which is intended
 * for operating on regular files and provides additional methods to support
 * this.</p>
 * <p>All file managers return a {@link FileObject} which provides an
 * abstraction from the underlying file.  The class {@link SimpleJavaFileObject}
 * is provided as a means to simplifying implementing this interface.</p>
 * <h2>Forwarding</h2>
 * <p>As file managers and file objects are provided as return values from
 * the methods of {@link JavaCompiler} and the file manager respectively,
 * there is no means to override their behaviour by subclassing.  Instead,
 * it is necessary to wrap the returned instance in another implementation
 * and forward method calls to it as appropriate.  The classes
 * {@link ForwardingJavaFileManager}, {@link ForwardingFileObject} and
 * {@link ForwardingJavaFileObject} facilitate this by providing an
 * implementation that simply forwards to another, which can then be subclassed
 * to provide additional behaviour.</p>
 *
 * @author Andrew John Hughes (gnu_andrew@member.fsf.org)
 * @since 1.6
 */
public interface JavaCompiler
  extends Tool, OptionChecker
{

  /**
   * Interface representing a compilation task as an asynchronous
   * event or {@link java.util.concurrent.Future}.  The task may be
   * started by invoking {@link #call()}.  Prior to this, the task
   * may be configured by the user using the other methods of this
   * interface.
   *
   * @author Andrew John Hughes (gnu_andrew@member.fsf.org)
   * @since 1.6
   */
  public static interface CompilationTask
    extends Callable<Boolean>
  {
    
    /**
     * Performs the compilation.  The compilation may only
     * be performed once. Subsequent calls will throw
     * an {@link IllegalStateException}.
     *
     * @return true if all files were successfully compiled.
     * @throws RuntimeException if an unrecoverable error occurred
     *                          in a user-supplied component of
     *                          the compilation process.  The user
     *                          exception will be provided as the
     *                          cause of this exception.
     * @throws IllegalStateException if the compilation has already run.
     */
    Boolean call();
    
    /**
     * Sets the locale to use for outputting diagnostics.
     *
     * @param locale the locale to use.  A value of {@code null}
     *               means no locale is applied.
     * @throws IllegalStateException if the compilation has started.
     */
    void setLocale(Locale locale);

    /**
     * Explicitly set the annotation processors to be used,
     * overriding the usual discovery process.
     *
     * @param processors the processors to use.
     * @throws IllegalStateException if the compilation has started.
     */
    void setProcessors(Iterable<? extends Processor> processors);

  }

  /**
   * Returns a new instance of the standard file manager implementation
   * used by this tool.  Any non-fatal diagnostics produced by the tool
   * will be passed to the specified {@link DiagnosticListener}, if
   * supplied, using the given locale.  Calls to the file manager after
   * a {@link JavaFileManager#flush()} or {@link JavaFileManager#close()}
   * will cause the file manager to be reopened.  The file manager must
   * be usable with other tools.
   *
   * @param listener the diagnostic listener to use or {@code null} if
   *                 the compiler should use its own methods for producing
   *                 diagnostics.
   * @param locale the locale to use to format the diagnostics or {@code null}
   *               to use the default locale.
   * @param charset the character set to use for decoding bytes or {@code null}
   *                to use the platform default.
   * @return the standard file manager implementation.
   */
  StandardJavaFileManager getStandardFileManager(DiagnosticListener<? super JavaFileObject> listener,
						 Locale locale, Charset charset);

  /**
   * Returns an asynchrononous task for performing the specified compilation.
   * The compilation may not have completed upon return of this method.
   * If a file manager is specified, it must supported all locations specified
   * in {@link StandardLocation}.
   *
   * @param out the output stream for compiler output beyond that provided by
   *            diagnostics; {@code System.err} is used if this is {@code null}.
   * @param fileManager the file manager to use or {@code null} to use a new
   *                    instance from
   *                    {@link #getStandardFileManaager(DiagnosticListener,Locale,Charset)}
   * @param listener the listener to pass diagnostic output to, or
   *                 {@code null} if the compiler's default method should
   *                 be used instead.
   * @param options the options to supply to the compiler or {@code null} if there are none.
   * @param classes the names of classes to use for annotation processing or {@code null}
   *                if there are none.
   * @param objects the file objects to compile, or {@code null} if there are none.
   * @return a task representing the proposed compilation.
   * @throws RuntimeException if an unrecoverable error occurred
   *                          in a user-supplied component of
   *                          the compilation process.  The user
   *                          exception will be provided as the
   *                          cause of this exception.
   * @throws IllegalArgumentException if the kind of any of the objects is something
   *                                  other than {@link JavaFileObject.Kind#SOURCE}.
   */
  CompilationTask getTask(Writer out, JavaFileManager fileManager,
			  DiagnosticListener<? super JavaFileObject> listener,
			  Iterable<String> options, Iterable<String> classes,
			  Iterable<? extends JavaFileObject> objects);

}
