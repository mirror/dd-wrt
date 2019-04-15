/*
 * Copyright 2010 The Miyamoto Team Licensed under the Apache License, Version 2.0 (the "License"); you may not use this
 * file except in compliance with the License. You may obtain a copy of the License at
 * http://www.apache.org/licenses/LICENSE-2.0 Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND,
 * either express or implied. See the License for the specific language governing permissions and limitations under the
 * License.
 */
package com.googlecode.miyamoto;

import java.lang.annotation.Annotation;
import java.lang.reflect.AccessibleObject;
import java.lang.reflect.InvocationHandler;
import java.lang.reflect.InvocationTargetException;
import java.lang.reflect.Method;
import java.lang.reflect.Proxy;
import java.security.AccessController;
import java.security.PrivilegedAction;
import java.util.LinkedHashMap;
import java.util.Map;
import java.util.Map.Entry;

/**
 *
 *
 * @param <A> The annotation type has to be proxed.
 * @version $Id$
 */
public final class AnnotationProxyBuilder<A extends Annotation> implements Annotation, InvocationHandler {

  /**
   * The multiplicator required in the hash code calculation.
   */
  private static final int MEMBER_NAME_MULTIPLICATOR = 127;

  /**
   * Creates a new annotation proxy.
   *
   * @param <A> the annotation type has to be proxed.
   * @param annotationType the annotation type class has to be proxed.
   * @return a new annotation proxy.
   */
  public static <A extends Annotation> AnnotationProxyBuilder<A> newBuilder(final Class<A> annotationType) {
    if (annotationType == null) {
      throw new IllegalArgumentException("Parameter 'annotationType' must be not null");
    }
    return new AnnotationProxyBuilder<>(annotationType);
  }

  /**
   * Retrieves the annotation proxy, if any, given the annotation.
   *
   * @param obj the annotation.
   * @return the annotation proxy, if any, given the annotation.
   */
  private static AnnotationProxyBuilder<?> getAnnotationProxy(final Object obj) {
    if (Proxy.isProxyClass(obj.getClass())) {
      final InvocationHandler handler = Proxy.getInvocationHandler(obj);
      if (handler instanceof AnnotationProxyBuilder) {
        return (AnnotationProxyBuilder<?>) handler;
      }
    }
    return null;
  }

  /**
   * Access to the declared methods of an annotation, given the type.
   *
   * @param <A> the annotation type.
   * @param annotationType the annotation type class.
   * @return the declared methods of an annotation, given the type.
   */
  private static <A extends Annotation> Method[] getDeclaredMethods(final Class<A> annotationType) {
    return AccessController.doPrivileged(new PrivilegedAction<Method[]>() {
      @Override
      public Method[] run() {
        final Method[] declaredMethods = annotationType.getDeclaredMethods();
        AccessibleObject.setAccessible(declaredMethods, true);
        return declaredMethods;
      }
    });
  }

  /**
   * The annotation type class has to be proxed.
   */
  private final Class<A>                        annotationType;

  /**
   * The annotation properties registry.
   */
  private final Map<String, AnnotationProperty> properties = new LinkedHashMap<>();

  /**
   * The proxed annotation.
   */
  private final A                               proxedAnnotation;

  /**
   * Build a new proxy annotation given the annotation type.
   *
   * @param annotationType the annotation type class has to be proxed.
   */
  private AnnotationProxyBuilder(final Class<A> annotationType) {
    this.annotationType = annotationType;

    String propertyName;
    Class<?> returnType;
    Object defaultValue;
    for (final Method method : getDeclaredMethods(annotationType)) {
      propertyName = method.getName();
      returnType = method.getReturnType();
      defaultValue = method.getDefaultValue();

      final AnnotationProperty property = new AnnotationProperty(propertyName, returnType);
      property.setValue(defaultValue);
      this.properties.put(propertyName, property);
    }

    this.proxedAnnotation = annotationType.cast(Proxy.newProxyInstance(annotationType.getClassLoader(), new Class<?>[] {
        annotationType
    }, this));
  }

  /**
   * Set a property value.
   *
   * @param name the property name.
   * @param value the property value.
   */
  public void setProperty(final String name, final Object value) {
    if (name == null) {
      throw new IllegalArgumentException("Parameter 'name' must be not null");
    }
    if (value == null) {
      throw new IllegalArgumentException("Parameter 'value' must be not null");
    }

    if (!this.properties.containsKey(name)) {
      throw new IllegalArgumentException(
          "Annotation '" + this.annotationType.getName() + "' does not contain a property named '" + name + "'");
    }

    this.properties.get(name).setValue(value);
  }

  /**
   * Returns the property value, given the name, if present.
   *
   * @param name the property name.
   * @return the property value, given the name, if present.
   */
  public Object getProperty(final String name) {
    if (name == null) {
      throw new IllegalArgumentException("Parameter 'name' must be not null");
    }
    return this.properties.get(name).getValue();
  }

  /**
   * {@inheritDoc}
   */
  @Override
  public Object invoke(final Object proxy, final Method method, final Object[] args) throws Throwable {
    final String name = method.getName();
    if (this.properties.containsKey(name)) {
      return this.getProperty(name);
    }
    return method.invoke(this, args);
  }

  /**
   * {@inheritDoc}
   */
  @Override
  public Class<? extends Annotation> annotationType() {
    return this.annotationType;
  }

  /**
   * Returns the proxed annotation.
   *
   * @return the proxed annotation.
   */
  public A getProxedAnnotation() {
    return this.proxedAnnotation;
  }

  /**
   * {@inheritDoc}
   */
  @Override
  public boolean equals(final Object obj) {
    if (this == obj) {
      return true;
    }

    if (obj == null) {
      return false;
    }

    if (!this.annotationType.isInstance(obj)) {
      return false;
    }

    String propertyName;
    AnnotationProperty expected;
    for (final Method method : getDeclaredMethods(this.annotationType())) {
      propertyName = method.getName();

      if (!this.properties.containsKey(propertyName)) {
        return false;
      }

      expected = this.properties.get(propertyName);
      final AnnotationProperty actual = new AnnotationProperty(propertyName, method.getReturnType());

      final AnnotationProxyBuilder<?> proxy = getAnnotationProxy(obj);
      if (proxy != null) {
        actual.setValue(proxy.getProperty(propertyName));
      } else {
        try {
          actual.setValue(method.invoke(obj));
        }
        catch (final IllegalArgumentException e) {
          return false;
        }
        catch (final IllegalAccessException e) {
          throw new AssertionError(e);
        }
        catch (final InvocationTargetException e) {
          return false;
        }
      }

      if (!expected.equals(actual)) {
        return false;
      }
    }

    return true;
  }

  /**
   * {@inheritDoc}
   */
  @Override
  public int hashCode() {
    int hashCode = 0;

    for (final Entry<String, AnnotationProperty> property : this.properties.entrySet()) {
      hashCode += ((MEMBER_NAME_MULTIPLICATOR * property.getKey().hashCode()) ^ property.getValue().getValueHashCode());
    }

    return hashCode;
  }

  /**
   * {@inheritDoc}
   */
  @Override
  public String toString() {
    final StringBuilder stringBuilder = new StringBuilder("@").append(this.annotationType.getName()).append('(');
    int counter = 0;
    for (final Entry<String, AnnotationProperty> property : this.properties.entrySet()) {
      if (counter > 0) {
        stringBuilder.append(", ");
      }
      stringBuilder.append(property.getKey()).append('=').append(property.getValue().valueToString());
      counter++;
    }
    return stringBuilder.append(')').toString();
  }

}
