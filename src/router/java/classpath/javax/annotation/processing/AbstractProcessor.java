/* AbstractProcessor.java -- Base Processor implementation using annotations.
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

import java.util.Collections;
import java.util.HashSet;
import java.util.Set;

import javax.lang.model.SourceVersion;

import javax.lang.model.element.AnnotationMirror;
import javax.lang.model.element.Element;
import javax.lang.model.element.ExecutableElement;
import javax.lang.model.element.TypeElement;

import javax.tools.Diagnostic;

/**
 * This class provides a base implementation of an
 * annotation {@link Processor}, using annotations to
 * determine the supported options, annotations and
 * source version.  The getter methods may issue warnings
 * using the diagnostic facilities provided after
 * processor initialization.

 * @author Andrew John Hughes (gnu_andrew@member.fsf.org)
 * @since 1.6
 */
public abstract class AbstractProcessor
  implements Processor
{

  /** The processing environment used by the processor framework. */
  protected ProcessingEnvironment processingEnv;

  /** True if the processor has been initialized. */
  private boolean initialized = false;

  /**
   * Constructs a new {@link AbstractProcessor}.
   */
  protected AbstractProcessor() { }

  /**
   * Returns an empty {@link Iterable} of {@link Completion}s.
   *
   * @param element the element being annotated.
   * @param annotation the annotation (possibly partial) being applied to the element.
   * @param member the annotation member to return completions for.
   * @param userText the source code text to be completed.
   * @return an empty {@code Iterable}.
   */
  @Override
  public Iterable<? extends Completion> getCompletions(Element element,
						       AnnotationMirror annotation,
						       ExecutableElement member,
						       String userText)
  {
    return Collections.emptyList();
  }

  /**
   * Uses the {@link SupportedAnnotationTypes} annotation to provide an
   * unmodifiable set of {@link String}s representing the supported
   * annotation types.  If the annotation is not present on the processor
   * class, the empty set is returned.
   *
   * @return the annotation types supported by this processor.
   * @see SupportedAnnotationTypes
   */
  @Override
  public Set<String> getSupportedAnnotationTypes()
  {
    SupportedAnnotationTypes types = getClass().getAnnotation(SupportedAnnotationTypes.class);
    if (types == null)
      {
	if (processingEnv != null)
	  processingEnv.getMessager().printMessage(Diagnostic.Kind.WARNING,
						   "Could not retrieve supported annotation types.");
	return Collections.emptySet();
      }

    String[] supported = types.value();
    Set<String> supportedTypes = new HashSet<String>();
    for (String s : supported)
      supportedTypes.add(s);

    return Collections.unmodifiableSet(supportedTypes);
  }

  /**
   * Uses the {@link SupportedOptions} annotation to provide an
   * unmodifiable set of {@link String}s representing the supported
   * options.  If the annotation is not present on the processor
   * class, the empty set is returned.
   *
   * @return the options supported by this processor.
   * @see SupportedOptions
   */
  @Override
  public Set<String> getSupportedOptions()
  {
    SupportedOptions options = getClass().getAnnotation(SupportedOptions.class);
    if (options == null)
      {
	if (processingEnv != null)
	  processingEnv.getMessager().printMessage(Diagnostic.Kind.WARNING,
						   "Could not retrieve supported options.");
	return Collections.emptySet();
      }

    String[] supported = options.value();
    Set<String> supportedOptions = new HashSet<String>();
    for (String s : supported)
      supportedOptions.add(s);

    return Collections.unmodifiableSet(supportedOptions);
  }

  /**
   * Uses the {@link SupportedSourceVersion} annotation to provide
   * the source version supported by this processor.  If the annotation
   * is not present on the processor class, {@link SourceVersion#RELEASE_6}
   * is returned.
   *
   * @return the source version supported by this processor.
   * @see SupportedVersion
   */
  @Override
  public SourceVersion getSupportedSourceVersion()
  {
    SupportedSourceVersion version = getClass().getAnnotation(SupportedSourceVersion.class);
    if (version == null)
      return SourceVersion.RELEASE_6;
    return version.value();
  }

  /**
   * Initialises the processor by setting {@link #processingEnv} to the supplied
   * processing environment.  This method can only be run once.  If invoked a
   * second time (i.e.{@link #isInitialized()} returns {@code true}), an
   * {@link IllegalStateException} will be thrown.
   *
   * @param env the environment to initialise the processor with.
   * @throws IllegalStateException if the processor has been initialised.
   */
  public void init(ProcessingEnvironment env)
  {
    if (isInitialized())
      throw new IllegalStateException("The processor has already been initialised.");
    processingEnv = env;
    initialized = true;
  }

  /**
   * Returns true if this processor has been initialized i.e.
   * {@link #init(ProcessingEnvironment)}has been run.
   *
   * @return {@code true} if the processor has been initialised.
   */
  protected boolean isInitialized()
  {
    return initialized;
  }

  /**
   * @inheritDoc
   */
  public abstract boolean process(Set<? extends TypeElement> types, RoundEnvironment roundEnv);

}
