/* Messager.java -- Allows an annotation processor to report messages.
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

import javax.lang.model.element.AnnotationMirror;
import javax.lang.model.element.AnnotationValue;
import javax.lang.model.element.Element;

import static javax.tools.Diagnostic.Kind;

/**
 * Provides a way for an annotation processor to report
 * messages to the user.  Messages may use elements,
 * annotations and annotation values to provide a location
 * hint, but these may be either unavailable or only
 * approximate.  Printing a message of kind
 * {@link javax.tools.Diagnostic,Kind#ERROR} will cause
 * an error to be raised.  How the messages are displayed
 * is left to the implementor; it may be a simple use of
 * {@code System.out} and/or {@code System.err} or something
 * more graphical if the application has a user interface.
 *
 * @author Andrew John Hughes (gnu_andrew@member.fsf.org)
 * @since 1.6
 */
public interface Messager
{

  /**
   * Prints a message of the specified kind.
   *
   * @param kind the kind of message to print.
   * @param message the message to print.
   */
  void printMessage(Kind kind, CharSequence message);

  /**
   * Prints a message of the specified kind at
   * the location of the given element.
   *
   * @param kind the kind of message to print.
   * @param message the message to print.
   * @param element the element to use for positioning.
   */
  void printMessage(Kind kind, CharSequence message,
		    Element element);

  /**
   * Prints a message of the specified kind at the
   * location of the annotation mirror of the given
   * annotated element.
   *
   * @param kind the kind of message to print.
   * @param message the message to print.
   * @param element the annotated element.
   * @param annotation the annotation to use for
   *                   positioning.
   */
  void printMessage(Kind kind, CharSequence message,
		    Element element,
		    AnnotationMirror annotation);

  /**
   * Prints a message of the specified kind at the
   * location of the annotation value inside the
   * annotation mirror of the given annotated element.
   *
   * @param kind the kind of message to print.
   * @param message the message to print.
   * @param element the annotated element.
   * @param annotation the annotation containing the value.
   * @param value the annotation value to use for positioning.
   */
  void printMessage(Kind kind, CharSequence message,
		    Element element,
		    AnnotationMirror annotation,
		    AnnotationValue value);

}
