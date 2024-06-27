/* Tool.java -- Interface for programatically-invokable tools.
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
import java.io.OutputStream;

import java.util.Set;

import javax.lang.model.SourceVersion;

/**
 * Interface for tools than be invoked from a program.  Tools
 * can be located using {@link java.util.ServiceLoader#load(Class)}.
 *
 * @author Andrew John Hughes (gnu_andrew@member.fsf.org)
 * @since 1.6
 */
public interface Tool
{

  /**
   * Returns the set of source language versions that are supported
   * by this tool.
   *
   * @return the set of supported source code versions.
   */
  Set<SourceVersion> getSourceVersions();

  /**
   * Invokes the tool using the specified input/output streams and
   * arguments, returning the same exit code it would if called from
   * the command line.  Traditionally, zero indicates success and non-zero
   * is used for errors.  Diagnostics from the tool will be written
   * to {@code out} or {@code err} in a way specific to the tool.
   *
   * @param in the standard input for the tool or {@code null} to use
   *           {@code System.in}.
   * @param out the standard output for the tool or {@code null} to use
   *            {@code System.out}
   * @param err the standard error stream for the tool or {@code null}
   *            to use {@code System.err}.
   * @param args the arguments to pass to the tool.
   * @return the return code from the tool.
   * @throws NullPointerException if any argument is {@code null}.
   */
  int run(InputStream in, OutputStream out, OutputStream err,
	  String... arguments);
}
