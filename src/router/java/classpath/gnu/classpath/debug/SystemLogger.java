/* SystemLogger.java -- Classpath's system debugging logger.
   Copyright (C) 2005  Free Software Foundation, Inc.

This file is a part of GNU Classpath.

GNU Classpath is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or (at
your option) any later version.

GNU Classpath is distributed in the hope that it will be useful, but
WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
General Public License for more details.

You should have received a copy of the GNU General Public License
along with GNU Classpath; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
02110-1301 USA

Linking this library statically or dynamically with other modules is
making a combined work based on this library.  Thus, the terms and
conditions of the GNU General Public License cover the whole
combination.

As a special exception, the copyright holders of this library give you
permission to link this library with independent modules to produce an
executable, regardless of the license terms of these independent
modules, and to copy and distribute the resulting executable under terms
of your choice, provided that you also meet, for each linked independent
module, the terms and conditions of the license of that module.  An
independent module is a module which is not derived from or based on
this library.  If you modify this library, you may extend this exception
to your version of the library, but you are not obligated to do so.  If
you do not wish to do so, delete this exception statement from your
version.  */


package gnu.classpath.debug;

import gnu.java.security.action.GetPropertyAction;

import java.security.AccessController;

import java.util.StringTokenizer;

import java.util.logging.ConsoleHandler;
import java.util.logging.Handler;
import java.util.logging.Level;
import java.util.logging.Logger;
import java.util.logging.LogRecord;

public final class SystemLogger extends Logger
{
  public static final SystemLogger SYSTEM = new SystemLogger();

  static
  {
    SYSTEM.setLevel (Level.FINE); // So selection is left to filter
    SYSTEM.setFilter (PreciseFilter.GLOBAL);
    Handler handler = new ConsoleHandler();
    handler.setLevel(Level.FINE);
    handler.setFilter(PreciseFilter.GLOBAL);
    SYSTEM.addHandler(handler);
    SYSTEM.setUseParentHandlers(false);
    String defaults = (String) AccessController.doPrivileged
      (new GetPropertyAction("gnu.classpath.debug.components"));

    if (defaults != null)
      {
        StringTokenizer tok = new StringTokenizer (defaults, ",");
        while (tok.hasMoreTokens ())
          {
            Component c = Component.forName (tok.nextToken ());
            if (c != null)
              PreciseFilter.GLOBAL.enable (c);
            SYSTEM.log (Level.INFO, "enabled: {0}", c);
          }
      }
  }

  /**
   * Fetch the system logger instance. The logger returned is meant for debug
   * and diagnostic logging for Classpath internals.
   *
   * @return The system logger.
   */
  public static SystemLogger getSystemLogger()
  {
    // XXX Check some permission here?
    return SYSTEM;
  }

  /**
   * Keep only one instance of the system logger.
   */
  private SystemLogger()
  {
    super("gnu.classpath", null);
  }

  /**
   * Variable-arguments log method.
   *
   * @param level The level to log to.
   * @param format The format string.
   * @param args The arguments.
   */
  public void logv(Level level, String format, Object... args)
  {
    log(level, format, args);
  }

  /**
   * Passes a log message to the <code>java.util.logging</code>
   * framework. This call returns very quickly if no log message will
   * be produced, so there is not much overhead in the standard case.
   *
   * @param level the severity of the message, for instance {@link
   * Component#SERVICE_LOGGING_WARNING}.
   * @param sourceClass the name of the class that issued the logging
   * request.
   * @param sourceMethod the name of the method that issued the logging
   * request.
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
  public void logp(Component level, String sourceClass, String sourceMethod,
		   Throwable t, String msg, Object... params)
  {
    LogRecord rec;

    // Return quickly if no log message will be produced.
    if (!PreciseFilter.GLOBAL.isEnabled(level))
      return;

    rec = new LogRecord(level, msg);
    if (params != null)
      rec.setParameters(params);

    rec.setThrown(t);

    // While java.util.logging can sometimes infer the class and
    // method of the caller, this automatic inference is not reliable
    // on highly optimizing VMs. Also, log messages make more sense to
    // developers when they display a public method in a public class;
    // otherwise, they might feel tempted to figure out the internals
    // in order to understand the problem.
    rec.setSourceClassName(sourceClass);
    rec.setSourceMethodName(sourceMethod);

    log(rec);
  }

}
