/* ToolProvider.java -- Provides methods for obtaining tool implementations.
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

import gnu.classpath.Configuration;

import static gnu.classpath.debug.Component.SERVICE_LOADING_VERBOSE;
import static gnu.classpath.debug.Component.SERVICE_LOADING_WARNING;
import static gnu.classpath.debug.SystemLogger.SYSTEM;

import gnu.classpath.debug.Component;

import java.io.File;

import java.lang.reflect.InvocationTargetException;

import java.net.MalformedURLException;
import java.net.URL;
import java.net.URLClassLoader;

/**
 * Provides methods for obtaining tool implementations.
 * This complements the functionality provided by
 * {@link ServiceLoader}.
 *
 * @author Andrew John Hughes (gnu_andrew@member.fsf.org)
 * @since 1.6
 */
public class ToolProvider
{

  /** The system compiler if available. */
  private static JavaCompiler compiler;

  /** The class that implements the system compiler. */
  private static final String COMPILER_CLASS =
    "org.eclipse.jdt.internal.compiler.tool.EclipseCompiler";

  /**
   * Returns the system Java compiler.  This does not take
   * account of compilers provided through the service
   * loader functionality.
   *
   * @return the system compiler or {@code null} if one is not
   *         provided.
   */
  public static synchronized JavaCompiler getSystemJavaCompiler()
  {
    if (compiler == null)
    {
      Class<?> compilerClass = getCompilerClass();
      if (compilerClass != null)
	try
	{
	  compiler = (JavaCompiler)
	    compilerClass.getConstructor().newInstance();
	}
	catch (NoSuchMethodException e)
	{
	  log(SERVICE_LOADING_WARNING, e,
	      "Couldn't find compiler constructor");
	}
	catch (InstantiationException e)
	{
	  log(SERVICE_LOADING_WARNING, e,
	      "Couldn't run compiler constructor");
	}
	catch (IllegalAccessException e)
	{
	  log(SERVICE_LOADING_WARNING, e,
	      "Couldn't access compiler constructor");
	}
	catch (InvocationTargetException e)
	{
	  log(SERVICE_LOADING_WARNING, e,
	      "Exception running compiler constructor");
	}
    }
    return compiler;
  }

  /**
   * Returns the class loader used to load system tools.
   *
   * @return the tool class loader or {@code null} if no
   *         tools are provided.
   */
  public static synchronized ClassLoader getSystemToolClassLoader()
  {
    if (compiler != null)
      return compiler.getClass().getClassLoader();
    return null;
  }

  private static Class<?> getCompilerClass()
  {
    Class<?> compilerClass = null;
    try
    {
      compilerClass = Class.forName(COMPILER_CLASS);
    }
    catch (ClassNotFoundException ex)
    {
      log(SERVICE_LOADING_VERBOSE, ex,
	  "Couldn''t find {0} on classpath",
	  COMPILER_CLASS);

      File jar = new File(Configuration.ECJ_JAR);
      if (jar.exists() && jar.canRead())
	{
	  try
	    {
	      ClassLoader loader =
		new URLClassLoader(new URL[] { jar.toURL() });
	      compilerClass = loader.loadClass(COMPILER_CLASS);
	    }
	  catch (MalformedURLException e)
	  {
	    log(SERVICE_LOADING_WARNING, e,
		"Bad ecj JAR URL: {0}", jar);
	  }
	  catch (ClassNotFoundException e) 
	  {
	    log(SERVICE_LOADING_VERBOSE, e,
		"Couldn''t find {0} in ecj jar", COMPILER_CLASS);
	  }
	}
    }
    return compilerClass;
  }
  
  /**
   * Passes a log message to the <code>java.util.logging</code>
   * framework. This call returns very quickly if no log message will
   * be produced, so there is not much overhead in the standard case.
   *
   * @param level the severity of the message, for instance {@link
   * Component#SERVICE_LOGGING_WARNING}.
   * @param t a Throwable that is associated with the log record, or
   * <code>null</code> if the log message is not associated with a
   * Throwable.
   * @param msg the log message, for instance <code>&#x201c;Could not
   * load {0}.&#x201d;</code> 
   * @param params the parameter(s) for the log message, or
   * <code>null</code> if <code>msg</code> does not specify any
   * parameters. If <code>param</code> is not an array, an array with
   * <code>param</code> as its single element gets passed to the
   * logging framework.
   */
  private static void log(Component level, Throwable t, String msg, Object... params)
  {
    SYSTEM.logp(level, ToolProvider.class.getName(), "getSystemJavaCompiler",
		t, msg, params);
  }

}
