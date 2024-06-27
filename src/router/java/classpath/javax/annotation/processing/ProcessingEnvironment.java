/* ProcessingEnvironment.java -- Links the annotation processor to the framework.
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

package javax.annotation.processing;

import java.util.Locale;
import java.util.Map;

import javax.lang.model.SourceVersion;

import javax.lang.model.util.Elements;
import javax.lang.model.util.Types;

/**
 * Provides the annotation processor with access to the
 * facilities of the framework, such as the
 * {@link Filer} for creating files and the
 * {@link Messager} for printing messages to the user.
 * It is possible to wrap implementations of this interface
 * in order to provide additional functionality.  However,
 * doing so requires also implementing the dependent
 * facility objects, such as the {@code Filer}, so that
 * these additional changes are reflected throughout.
 *
 * @author Andrew John Hughes (gnu_andrew@member.fsf.org)
 * @since 1.6
 */
public interface ProcessingEnvironment
{

  /**
   * Returns an implementation of a number of utility
   * methods which provide additional functionality for
   * working with (@link javax.lang.model.element.Element}
   * instances.
   *
   * @return the element utilities.
   */
  Elements getElementUtils();

  /**
   * Returns an implementation of the {@link Filer}
   * which can be used to create source, class or
   * resource files.
   *
   * @return the filer.
   */
  Filer getFiler();

  /**
   * Returns the locale used to localise messages from
   * the {@link Messager} or {@code null} if one has not
   * been set.
   *
   * @return the current locale or {@code null} if no
   *         locale has been setup.
   */
  Locale getLocale();

  /**
   * Returns an implementation of the {@link Messager}
   * which is used to report back to the user, whether
   * that be with errors, warnings or other issues.
   *
   * @return the messager.
   */
  Messager getMessager();

  /**
   * Returns the options passed to the annotation processor
   * as a map of option names to values.  If an option does
   * not have a value, it will map to {@code null}.  For
   * details of the options themselves, see the documentation
   * for the annotation processor.
   *
   * @return a map of the options passed to the annotation processor.
   */
  Map<String,String> getOptions();

  /**
   * Returns the version of Java source code that generated
   * source files and class files should conform to.
   *
   * @return the version of Java code used in generated files.
   * @see Processor#getSupportedSourceVersion()
   */
  SourceVersion getSourceVersion();

  /**
   * Returns an implementation of a number of utility
   * methods which provide additional functionality for
   * working with (@link javax.lang.model.type.TypeMirror}
   * instances.
   *
   * @return the type utilities.
   */
  Types getTypeUtils();
  
}
