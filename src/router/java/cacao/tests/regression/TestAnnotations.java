/* tests/regression/TestAnnotations.java - checks correct functionality of the
   annotation API

   Copyright (C) 2007, 2008
   CACAOVM - Verein zur Foerderung der freien virtuellen Maschine CACAO

   This file is part of CACAO.

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public License as
   published by the Free Software Foundation; either version 2, or (at
   your option) any later version.

   This program is distributed in the hope that it will be useful, but
   WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
   02110-1301, USA.

*/

import java.lang.annotation.Annotation;
import java.lang.annotation.Inherited;
import java.lang.annotation.Retention;
import java.lang.annotation.RetentionPolicy;
import java.lang.reflect.AnnotatedElement;
import java.lang.reflect.Array;
import java.lang.reflect.Constructor;
import java.lang.reflect.Method;
import java.lang.reflect.Field;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.Collection;
import java.util.Collections;
import java.util.Comparator;
import java.util.List;
import java.util.Map;
import java.util.HashMap;
import java.util.SortedSet;
import java.util.TreeMap;
import java.util.LinkedHashMap;
import java.util.Set;
import java.util.HashSet;
import java.util.TreeSet;
import java.util.LinkedHashSet;

/* ********* helper classes for the tests *************************************/
enum IndexingType {
	HASH,
	LINKED_HASH,
	TREE
}

class MapFactory {
	private IndexingType indexingType;
	
	public MapFactory(IndexingType indexingType) {
		this.indexingType = indexingType;
	}
	
	public <K, V> Map<K, V> createMap() {
		switch (indexingType) {
		case HASH:        return new HashMap<K, V>();
		case LINKED_HASH: return new LinkedHashMap<K, V>(); 
		case TREE:        return new TreeMap<K, V>();
		default:
			throw new IllegalArgumentException(
					"Illegal indexing type: " + indexingType);
		}
	}

	public <K, V> Map<K, V> createMap(Map<? extends K,? extends V> map) {
		switch (indexingType) {
		case HASH:        return new HashMap<K, V>(map);
		case LINKED_HASH: return new LinkedHashMap<K, V>(map);
		case TREE:        return new TreeMap<K, V>(map);
		default:
			throw new IllegalArgumentException(
					"Illegal indexing type: " + indexingType);
		}
	}
}

class SetFactory {
	private IndexingType indexingType;
	
	public SetFactory(IndexingType indexingType) {
		this.indexingType = indexingType;
	}
	
	public <E> Set<E> createSet() {
		switch (indexingType) {
		case HASH:        return new HashSet<E>();
		case TREE:        return new TreeSet<E>();
		case LINKED_HASH: return new LinkedHashSet<E>();
		default:
			throw new IllegalArgumentException(
					"Illegal indexing type: " + indexingType);
		}
	}
	
	public <E> Set<E> createSet(E[] keys) {
		Set<E> set = createSet();
		
		for (E key : keys) {
			set.add(key);
		}
		
		return set;
	}
	
	public <E> Set<E> createSet(Collection<? extends E> collection) {
		switch (indexingType) {
		case HASH:        return new HashSet<E>(collection);
		case TREE:        return new TreeSet<E>(collection);
		case LINKED_HASH: return new LinkedHashSet<E>(collection);
		default:
			throw new IllegalArgumentException(
					"Illegal indexing type: " + indexingType);
		}
	}
}

class TestHelper {
	private static MapFactory mapFactory = new MapFactory(IndexingType.HASH);
	private static SetFactory setFactory = new SetFactory(IndexingType.HASH);
	
	private static long testCount = 0;
	private static long failCount = 0;

	public static MapFactory getMapFactory() {
		return mapFactory;
	}
	
	public static SetFactory getSetFactory() {
		return setFactory;
	}
	
	public static void clear() {
		testCount = 0;
		failCount = 0;
	}

	public static long getTestCount() {
		return testCount;
	}

	public static long getFailCount() {
		return failCount;
	}

	public static void printStatistics() {
		System.out.printf("         passed: %8d\n", testCount - failCount);
		System.out.printf("         failed: %8d\n", failCount);
		System.out.printf("         ----------------\n");
		System.out.printf("         sum:    %8d\n", testCount);
	}

	public static void log() {
		System.out.println();
	}

	public static void log(String msg) {
		System.out.println(msg);
	}

	public static void log(String fmt, Object... args) {
		System.out.printf(fmt + "\n", args);
	}

	public static boolean ok(boolean test, String msg) {
		return ok(test, "%s", msg);
	}

	public static boolean ok(boolean test, String fmt, Object... args) {
		++testCount;

		if (test) {
			System.out.printf("[  OK  ] " + fmt + "\n", args);
			return true;
		} else {
			++failCount;
			System.out.printf("[ FAIL ] " + fmt + "\n", args);
			return false;
		}
	}

	public static boolean okx(boolean test, String what, String fmt,
			String errfmt, Object[] args, Object... errargs) {
		if (!test)
			return ok(test, fmt + ": %s\n         error: " + errfmt, concat(
					concat(args, what), errargs));
		else
			return ok(test, fmt + ": %s", concat(args, what));
	}

	/* helper methods: */
	@SuppressWarnings("unchecked")
	public static <T> T[] concat(T[] firstArray, T... moreElements) {
		return concat2(firstArray, moreElements);
	}

	@SuppressWarnings("unchecked")
	public static <T> T[] concat2(T[] firstArray, T[]... moreArrays) {
		int length = firstArray.length;

		for (T[] array : moreArrays) {
			length += array.length;
		}

		T[] result = (T[]) Array.newInstance(firstArray.getClass()
				.getComponentType(), length);

		System.arraycopy(firstArray, 0, result, 0, firstArray.length);

		int pos = firstArray.length;
		for (T[] array : moreArrays) {
			System.arraycopy(array, 0, result, pos, array.length);
			pos += array.length;
		}

		return result;
	}

	public static <T> String str(T object) {
		/* null */
		if (object == null) {
			return "null";
		}
		/* array */
		else if (object.getClass().isArray()) {
			StringBuilder buf = new StringBuilder();
			int length = Array.getLength(object);

			buf.append('{');

			if (length > 0) {
				buf.append(str(Array.get(object, 0)));

				for (int i = 1; i < length; ++i) {
					buf.append(", ");
					buf.append(str(Array.get(object, i)));
				}
			}

			buf.append('}');

			return buf.toString();
		}
		/* escape String */
		else if (object instanceof String) {
			String s = object.toString();
			StringBuilder buf = new StringBuilder();

			buf.append('"');

			for (int i = 0; i < s.length(); ++i) {
				char ch = s.charAt(i);

				switch (ch) {
				case '"':
					buf.append("\\\"");
					break;
				case '\\':
					buf.append("\\\\");
					break;
				case '\b':
					buf.append("\\b");
					break;
				case '\f':
					buf.append("\\f");
					break;
				case '\t':
					buf.append("\\t");
					break;
				case '\n':
					buf.append("\\n");
					break;
				case '\r':
					buf.append("\\r");
					break;
				default:
					buf.append(ch);
				}
			}

			buf.append('"');
			return buf.toString();
		}
		/* escape Character */
		else if (object instanceof Character) {
			switch ((Character) object) {
			case '\'':
				return "'\\''";
			case '\\':
				return "'\\\\'";
			case '\b':
				return "'\\b'";
			case '\f':
				return "'\\f'";
			case '\t':
				return "'\\t'";
			case '\n':
				return "'\\n'";
			case '\r':
				return "'\\r'";
			default:
				return "'" + object + "'";
			}
		}
		/* Class */
		else if (object instanceof Class) {
			return ((Class<?>) object).getName();
		}

		return object.toString();
	}
	
	public static <E extends Comparable<? super E>> Collection<E> sorted(
			Collection<E> collection) {
		if (collection instanceof SortedSet) {
			return collection;
		}
		else {
			List<E> reply = new ArrayList<E>(collection);
			Collections.sort(reply);
			
			return reply;
		}
	}

	public static <E> Collection<E> sorted(
			Collection<E> collection,
			Comparator<? super E> comparator) {
		if (collection instanceof SortedSet) {
			return collection;
		}
		else {
			List<E> reply = new ArrayList<E>(collection);
			Collections.sort(reply, comparator);
			
			return reply;
		}
	}

	public static <E extends Comparable<? super E>> E[] sorted(E[] values) {
		Arrays.sort(values);
		return values;
	}
	
	public static <E> E[] sorted(
			E[] values,
			Comparator<? super E> comparator) {
		Arrays.sort(values, comparator);
		return values;
	}
}

class Entry implements Map.Entry<String, Object> {
	private String key;
	private Object val;

	public Entry(String key, Object value) {
		this.key = key;
		this.val = value;
	}

	public String getKey() {
		return key;
	}

	public Object getValue() {
		return val;
	}

	public Object setValue(Object value) {
		Object oldval = val;
		val = value;
		return oldval;
	}

	public int hashCode() {
		return (key != null ? key.hashCode() << 8 : 0)
				^ (val != null ? val.hashCode() : 0);
	}

	public boolean equals(Object other) {
		if (other != null && other instanceof Entry) {
			Entry otherEntry = (Entry) other;

			return (key == null ? otherEntry.key == null :
				key.equals(otherEntry.key))
					&& (val == null ? otherEntry.val == null :
						val.equals(otherEntry.val));
		}

		return false;
	}
}

class AnnotationTester implements Comparable<AnnotationTester> {
	private Class<? extends Annotation> annotationType;

	private Map<String, Object> values =
		TestHelper.getMapFactory().<String, Object>createMap();

	public AnnotationTester(Class<? extends Annotation> annotationType,
			Map<String, Object> values) {
		this.annotationType = annotationType;
		this.values.putAll(values);

		checkValues();
	}

	public AnnotationTester(Class<? extends Annotation> annotationType) {
		this.annotationType = annotationType;

		checkValues();
	}

	public AnnotationTester(Class<? extends Annotation> annotationType,
			Map.Entry<String, Object>... values) {
		this.annotationType = annotationType;

		for (Map.Entry<String, Object> value : values) {
			if (this.values.put(value.getKey(), value.getValue()) != null)
				throw new IllegalArgumentException(
						"duplicated key: " + TestHelper.str(value.getKey()));
		}

		checkValues();
	}

	public Class<? extends Annotation> annotationType() {
		return annotationType;
	}

	private void checkValues() {
		for (String methodName : values.keySet()) {
			try {
				annotationType.getDeclaredMethod(methodName);
			} catch (NoSuchMethodException e) {
				throw new IllegalArgumentException(
						"annotation " + annotationType.getName() +
						" has no method of name " + methodName + "()", e);
			}
		}

		for (Method method : annotationType.getDeclaredMethods()) {
			Object value = values.get(method.getName());
			Class<?> returnType = method.getReturnType();
			Class<?> valueType = value.getClass();

			if (value == null) {
				throw new IllegalArgumentException(
						"annotation method of name " + method.getName() +
						"() needs an expected value");
			} else if (value instanceof AnnotationTester) {
				AnnotationTester tester = (AnnotationTester) value;

				if (!tester.annotationType().equals(returnType)) {
					throw new IllegalArgumentException(
							"illegal value type for annotation method " +
							method.getName() + "()");
				}
			} else if (!returnType.isInstance(value)) {
				if (returnType.isArray()
						&& returnType.getComponentType().isAnnotation()) {
					if (!valueType.isArray()
							|| !isSubclassOf(valueType.getComponentType(),
									AnnotationTester.class)) {
						throw new IllegalArgumentException(
								"illegal value type for annotation method " +
								method.getName() + "()");
					}

					for (AnnotationTester tester : (AnnotationTester[]) value) {
						if (!tester.annotationType().equals(
								returnType.getComponentType())) {
							throw new IllegalArgumentException(
									"illegal value type for annotation method " +
									method.getName() + "()");
						}
					}
				} else if (!returnType.isPrimitive()
						|| !valueType.equals(getBoxingType(returnType))) {
					throw new IllegalArgumentException(
							"illegal value type for annotation method " +
							method.getName() + "()");
				}
			}
		}
	}

	public static boolean isSubclassOf(Class<?> subClass, Class<?> superClass) {
		do {
			if (subClass.equals(superClass)) {
				return true;
			}
			subClass = subClass.getSuperclass();
		} while (subClass != null);

		return false;
	}

	private static Map<Class<?>, Class<?>> boxingMap =
		TestHelper.getMapFactory().<Class<?>, Class<?>>createMap();

	static {
		boxingMap.put(byte.class,   Byte.class);
		boxingMap.put(char.class,   Character.class);
		boxingMap.put(short.class,  Short.class);
		boxingMap.put(long.class,   Long.class);
		boxingMap.put(int.class,    Integer.class);
		boxingMap.put(float.class,  Float.class);
		boxingMap.put(double.class, Double.class);
	}

	public static Class<?> getBoxingType(Class<?> primitiveType) {
		Class<?> type = boxingMap.get(primitiveType);

		if (type == null) {
			throw new IllegalArgumentException(
					"illegal type for boxing: "	+ primitiveType.getName());
		}

		return type;
	}

	public int hashCode() {
		return (annotationType.hashCode() << 8) ^ values.hashCode();
	}

	public boolean equals(Object other) {
		if (other != null) {
			if (other instanceof AnnotationTester) {
				AnnotationTester otherAnnotationTester =
					(AnnotationTester) other;

				if (!otherAnnotationTester.annotationType.equals(annotationType) ||
						otherAnnotationTester.values.size() != values.size())
					return false;

				for (Map.Entry<String, Object> entry : values.entrySet()) {
					if (!otherAnnotationTester.values.containsKey(entry.getKey()) ||
							!otherAnnotationTester.values.get(
									entry.getKey()).equals(entry.getValue()))
						return false;
				}

				return true;
			} else if (other instanceof Annotation) {
				Annotation anno = (Annotation) other;
				Method[] annotationFields = anno.annotationType()
						.getDeclaredMethods();

				if (!annotationType.equals(anno.annotationType())
						|| annotationFields.length != values.size())
					return false;

				for (Method method : annotationFields) {
					if (!values.get(method.getName()).equals(
							method.getDefaultValue()))
						return false;
				}

				return true;
			}
		}
		return false;
	}
	
	public String toString() {
		StringBuilder buf = new StringBuilder();
		
		buf.append('@');
		buf.append(annotationType.getName());
		buf.append('(');
		
		int i = 0;
		for (Map.Entry<String, Object> entry : values.entrySet()) {
			buf.append(entry.getKey());
			buf.append('=');
			buf.append(TestHelper.str(entry.getValue()));
			
			++ i;
			if (i < values.size()) {
				buf.append(", ");
			}
		}
		buf.append(')');
		
		return buf.toString();
	}

	private final static Object[] EMPTY_OBJECT_ARRAY = new Object[] {};

	protected boolean ok(boolean test, String what, String errfmt,
			Object... errargs) {
		return TestHelper.okx(test, what, annotationType.getName(), errfmt,
				EMPTY_OBJECT_ARRAY, errargs);
	}

	public boolean test(Annotation annotation) throws SecurityException,
			NoSuchMethodException {
		boolean ok = true;
		Method[] declaredMedthods = annotation.annotationType()
				.getDeclaredMethods();

		TestHelper.log(" * Testing %s", annotationType.getName());
		
		ok = ok(annotationType.equals(annotation.annotationType()),
				"test annotationType", "expected %s but got %s",
				annotationType, annotation.annotationType());

		if (ok) {
			ok = ok(declaredMedthods.length == values.size(),
					"test annotation methods count", "expected %d but got %d",
					values.size(), declaredMedthods.length)
					&& ok;
			
			for (String methodName : TestHelper.sorted(values.keySet())) {
				Object    expected = values.get(methodName);
				Object    got = null;
				Exception ex  = null;

				try {
					got = annotation.getClass().getMethod(methodName).invoke(
							annotation);
				} catch (Exception e) {
					ex = e;
				}

				ok = ok(ex == null,
						"test invocation of the annotation method " +
						methodName + "()",
						"exception occured while invokation: %s",
						ex != null ? ex.getMessage() : "")
						&& ok;

				if (ex != null) {
					ex.printStackTrace();
				} else {
					ok = ok(got != null, "test return value of " + methodName +
							"() != null", "got null!")
							&& ok;

					ok = ok(equals(got, expected), "test return value of "
							+ methodName + "()", "expected %s (type: %s) but"
							+ " got %s (type: %s)", TestHelper.str(got), got
							.getClass().getName(), TestHelper.str(expected),
							expected.getClass().getName())
							&& ok;
				}
			}
		}

		return ok;
	}

	public static boolean equals(Object a, Object b) {
		if (a == null) {
			return b == null;
		}
		else if (a instanceof Annotation && b instanceof AnnotationTester) {
			return equals((Annotation) a, (AnnotationTester) b);
		}
		else if (b instanceof Annotation && a instanceof AnnotationTester) {
			return equals((Annotation) b, (AnnotationTester) a);
		}
		else if (a.getClass().isArray()) {
			if (!b.getClass().isArray()) {
				return false;
			}

			int alen = Array.getLength(a);
			int blen = Array.getLength(b);

			if (alen != blen) {
				return false;
			}

			for (int i = 0; i < alen; ++i) {
				if (!equals(Array.get(a, i), Array.get(b, i))) {
					return false;
				}
			}

			return true;
		}
		else {
			return a.equals(b);
		}
	}

	public static boolean equals(Annotation annoation, AnnotationTester tester) {
		if (!tester.annotationType().equals(annoation.annotationType())) {
			return false;
		}

		try {
			for (Map.Entry<String, Object> bEntry : tester.values.entrySet()) {
				Object aValue = annoation.getClass().getMethod(bEntry.getKey())
						.invoke(annoation);

				if (!equals(bEntry.getValue(), aValue)) {
					return false;
				}
			}
		} catch (Exception e) {
			// TODO: better error handling?
			e.printStackTrace();
			return false;
		}

		return true;
	}

	public int compareTo(AnnotationTester other) {
		return annotationType.getName().compareTo(
				other.annotationType().getName());
	}
}

abstract class AnnotatedElementAnnotationTester<T extends AnnotatedElement>
		implements Comparable<AnnotatedElementAnnotationTester<? extends AnnotatedElement>> {
	protected ClassAnnotationTester declaringClass;
	private T                       element;
	private String                  name;
	
	private Map<Class<? extends Annotation>, AnnotationTester> annotations =
		TestHelper.getMapFactory().
		<Class<? extends Annotation>, AnnotationTester>createMap();
	
	private Map<Class<? extends Annotation>, AnnotationTester> declaredAnnotations =
		TestHelper.getMapFactory().
		<Class<? extends Annotation>, AnnotationTester>createMap();

	protected final static Object[] EMPTY_OBJECT_ARRAY = new Object[] {};

	public AnnotatedElementAnnotationTester(
			ClassAnnotationTester clazz,
			T                     element,
			String                name) {
		this.declaringClass = clazz;
		this.element        = element;
		this.name           = name;
	}
	
	public T annotatedElement() {
		return element;
	}
	
	public String getName() {
		return name;
	}
	
	public int compareTo(
			AnnotatedElementAnnotationTester<? extends AnnotatedElement> other) {
		return name.compareTo(other.getName());
	}
	
	private static Comparator<Annotation> annotationComparator =
		new Comparator<Annotation>() {
			public int compare(Annotation o1, Annotation o2) {
				return o1.annotationType().getName().compareTo(
						o2.annotationType().getName());
			}
	};
	
	protected static Annotation[] sorted(Annotation[] annotations) {
		return TestHelper.sorted(annotations, annotationComparator);
	}

	protected boolean ok(
			boolean test, String what,
			String errfmt, Object... errargs) {
		return TestHelper.okx(test, what, name, errfmt,
				EMPTY_OBJECT_ARRAY, errargs);
	}
	
	protected void logName() {
		TestHelper.log("-- Testing %s --", getName());
	}
	
	protected void logHeader(String fmt, Object... args) {
		TestHelper.log("-- " + getName() + ": Testing " + fmt + " --", args);
	}

	protected void log() {
		TestHelper.log();
	}

	public void putInheritedAnnotation(
			Class<? extends Annotation> annotationType,
			Map.Entry<String, Object>... values) {
		if (annotations.containsKey(annotationType))
			throw new IllegalArgumentException(
					"Annotation already exists: " + annotationType.getName());

		annotations.put(annotationType,
				new AnnotationTester(annotationType, values));
	}

	public void putInheritedAnnotation(
			Class<? extends Annotation> annotationType,
			Map<String, Object> values) {
		if (annotations.containsKey(annotationType))
			throw new IllegalArgumentException(
					"Annotation already exists: " + annotationType.getName());

		annotations.put(annotationType,
				new AnnotationTester(annotationType, values));
	}

	public void putDeclaredAnnotation(
			Class<? extends Annotation> annotationType,
			Map.Entry<String, Object>... values) {
		if (annotations.containsKey(annotationType))
			throw new IllegalArgumentException(
					"Annotation already exists: " + annotationType.getName());

		AnnotationTester tester = new AnnotationTester(annotationType, values);

		annotations.put(annotationType, tester);
		declaredAnnotations.put(annotationType, tester);
	}

	public void putDeclaredAnnotation(
			Class<? extends Annotation> annotationType,
			Map<String, Object> values) {
		if (annotations.containsKey(annotationType))
			throw new IllegalArgumentException(
					"Annotation already exists: " + annotationType.getName());

		AnnotationTester tester = new AnnotationTester(annotationType, values);

		annotations.put(annotationType, tester);
		declaredAnnotations.put(annotationType, tester);
	}

	public boolean test() throws SecurityException, NoSuchMethodException {
		boolean ok;
		
		logName();
		
		ok = testGetDeclaredAnnotations();
		ok = testGetAnnotations()      && ok;
		ok = testGetAnnotation()       && ok;
		ok = testIsAnnotationPresent() && ok;

		return ok;
	}


	private boolean testGetDeclaredAnnotations() throws SecurityException,
			NoSuchMethodException {
		Annotation[] declaredAnnotations = element.getDeclaredAnnotations();
		boolean ok = true;
		Set<Class<? extends Annotation>> annoTypes =
			TestHelper.getSetFactory().<Class<? extends Annotation>>createSet();

		logHeader("getDeclaredAnnotations()");
		
		ok = ok(this.declaredAnnotations.size() == declaredAnnotations.length,
				"test declared annotations count", "expected %d but got %d",
				this.declaredAnnotations.size(), declaredAnnotations.length)
				&& ok;

		for (Annotation anno : sorted(declaredAnnotations)) {
			AnnotationTester tester = this.annotations.get(
					anno.annotationType());

			ok = ok(!annoTypes.contains(anno.annotationType()),
					"test unique occurrence of the annotation type " +
					anno.annotationType().getName(),
					"duplicated annotation!") && ok;

			annoTypes.add(anno.annotationType());

			ok = ok(tester != null, "test if annotation " +
					anno.annotationType().getName() + " should be there",
					"wrong annotation") && ok;

			if (tester != null) {
				ok = tester.test(anno) && ok;
			}
		}

		return ok;
	}
	
	private boolean testGetAnnotations() throws SecurityException,
			NoSuchMethodException {
		Annotation[] annotations = element.getAnnotations();
		boolean ok = true;
		Set<Class<? extends Annotation>> annoTypes =
			TestHelper.getSetFactory().<Class<? extends Annotation>>createSet();

		logHeader("getAnnotations()");
		
		ok = ok(this.annotations.size() == annotations.length,
				"test annotations count", "expected %d but got %d",
				this.annotations.size(), annotations.length)
				&& ok;

		for (Annotation anno : sorted(annotations)) {
			AnnotationTester tester = this.annotations.get(anno
					.annotationType());

			ok = ok(!annoTypes.contains(anno.annotationType()),
					"test unique occurrence of the annotation type " +
					anno.annotationType().getName(),
					"duplicated annotation!")
					&& ok;

			annoTypes.add(anno.annotationType());

			ok = ok(tester != null, "test if annotation " +
					anno.annotationType().getName() + " should be there",
					"wrong annotation")
					&& ok;

			if (tester != null) {
				ok = tester.test(anno) && ok;
			}
		}

		return ok;
	}

	private boolean testGetAnnotation() throws SecurityException,
			NoSuchMethodException {
		boolean ok = true;

		logHeader("getAnnotation(Class<? extends Annotation>)");
		
		for (AnnotationTester tester : TestHelper.sorted(annotations.values())) {
			Class<? extends Annotation> annotationType = tester
					.annotationType();
			Annotation anno = element.getAnnotation(annotationType);

			ok = ok(anno != null, "try to get required annotation " +
					annotationType.getName() +
					" using getAnnotation(Class<? extends Annotation>)",
					"annotation dose not exist")
					&& ok;

			if (anno != null) {
				ok = tester.test(anno) && ok;
			}
		}

		return ok;
	}

	private boolean testIsAnnotationPresent() {
		boolean ok = true;

		logHeader("isAnnotationPresent(Class<? extends Annotation>)");
		
		for (AnnotationTester tester : TestHelper.sorted(annotations.values())) {
			Class<? extends Annotation> annotationType =
				tester.annotationType();

			ok = ok(element.isAnnotationPresent(annotationType),
					"test if annotation " + annotationType.getName() +
					" is present using isAnnotationPresent()",
					"annotation dose not exist")
					&& ok;
		}

		return ok;
	}
}

class FieldAnnotationTester extends AnnotatedElementAnnotationTester<Field> {
	public FieldAnnotationTester(ClassAnnotationTester clazz, Field field) {
		super(clazz, field, field.getDeclaringClass().getName()	+ "." +
				field.getName());
	}
}

abstract class AbstractMethodAnnotationTester<T extends AnnotatedElement>
		extends	AnnotatedElementAnnotationTester<T> {
	private Map<Class<? extends Annotation>, AnnotationTester>[] parameterAnnotations;

	@SuppressWarnings("unchecked")
	public AbstractMethodAnnotationTester(ClassAnnotationTester clazz,
			T element, String name, Class<?>[] parameterTypes) {
		super(clazz, element,
				methodName(clazz.annotatedElement(), name, parameterTypes));

		parameterAnnotations = new Map[parameterTypes.length];

		MapFactory mapFactory = TestHelper.getMapFactory();
		
		for (int i = 0; i < parameterTypes.length; ++i) {
			parameterAnnotations[i] = mapFactory.
				<Class<? extends Annotation>, AnnotationTester>createMap();
		}
	}

	private static String methodName(Class<?> declaringClass, String name,
			Class<?>[] parameterTypes) {
		StringBuilder buf = new StringBuilder(128);

		buf.append(declaringClass.getName());
		buf.append('.');
		buf.append(name);
		buf.append('(');

		if (parameterTypes.length > 0) {
			buf.append(parameterTypes[0].getName());

			for (int i = 1; i < parameterTypes.length; ++i) {
				buf.append(',');
				buf.append(parameterTypes[i].getName());
			}
		}

		buf.append(')');

		return buf.toString();
	}

	abstract protected Annotation[][] getParameterAnnotations();

	public void putParameterAnnotation(int index,
			Class<? extends Annotation> annotationType,
			Map.Entry<String, Object>... values) {
		if (parameterAnnotations[index].containsKey(annotationType))
			throw new IllegalArgumentException(
					"Annotation already exists: " + annotationType.getName());

		parameterAnnotations[index].put(
				annotationType,
				new AnnotationTester(annotationType, values));
	}

	public void putParameterAnnotation(int index,
			Class<? extends Annotation> annotationType,
			Map<String, Object> values) {
		if (parameterAnnotations[index].containsKey(annotationType))
			throw new IllegalArgumentException(
					"Annotation already exists: " + annotationType.getName());

		parameterAnnotations[index].put(
				annotationType,
				new AnnotationTester(annotationType, values));
	}

	public boolean test() throws SecurityException, NoSuchMethodException {
		boolean ok = super.test();

		ok = testParameterAnnotations() && ok;

		return ok;
	}

	private boolean testParameterAnnotations() throws SecurityException,
			NoSuchMethodException {
		boolean ok = true;
		Annotation[][] parameterAnnotations = getParameterAnnotations();

		logHeader("getParameterAnnotations()");
		
		ok = ok(
				this.parameterAnnotations.length == parameterAnnotations.length,
				"test parameter count", "expected %d but got %d",
				this.parameterAnnotations.length, parameterAnnotations.length)
				&& ok;

		if (this.parameterAnnotations.length == parameterAnnotations.length) {
			for (int i = 0; i < parameterAnnotations.length; ++i) {
				Set<Class<? extends Annotation>> annoTypes =
					TestHelper.getSetFactory().
					<Class<? extends Annotation>>createSet();

				ok = ok(
						this.parameterAnnotations[i].size() == parameterAnnotations[i].length,
						"test parameter annotations count for parameter " + i,
						"expected %d but got %d",
						Integer.valueOf(this.parameterAnnotations.length),
						Integer.valueOf(parameterAnnotations.length))
						&& ok;

				for (Annotation anno : sorted(parameterAnnotations[i])) {
					AnnotationTester tester =
						this.parameterAnnotations[i].get(anno.annotationType());

					ok = ok(!annoTypes.contains(anno.annotationType()),
							"test unique occurrence of the annotation type " +
							anno.annotationType().getName(),
							"duplicated annotation!")
							&& ok;

					annoTypes.add(anno.annotationType());

					ok = ok(tester != null, "test if annotation of type " +
							anno.annotationType().getName() +
							" should be defined for parameter " + i,
							"no, it shouldn't be!")
							&& ok;

					if (tester != null) {
						ok = tester.test(anno) && ok;
					}
				}
			}
		}

		return ok;
	}
}

class MethodAnnotationTester extends AbstractMethodAnnotationTester<Method> {
	private Object defaultValue = null;

	public MethodAnnotationTester(ClassAnnotationTester clazz, Method method) {
		super(clazz, method, method.getName(), method.getParameterTypes());
	}

	public MethodAnnotationTester(ClassAnnotationTester clazz, Method method,
			Object defaultValue) {
		this(clazz, method);
		setDefaultValue(defaultValue);
	}

	public void setDefaultValue(Object value) {
		if (value != null && !declaringClass.isAnnotation())
			throw new IllegalArgumentException(
					"cannot set annotation default value of a method " +
					"of a non-annotation class.");

		defaultValue = value;
	}

	public Object getDefaultValue() {
		return defaultValue;
	}

	public boolean test() throws SecurityException, NoSuchMethodException {
		boolean ok = testGetDefaultValue();

		return super.test() && ok;
	}

	private boolean testGetDefaultValue() {
		boolean ok = true;
		Object defaultValue = annotatedElement().getDefaultValue();
		
		logHeader("getDefaultValue()");

		if (this.defaultValue == null) {
			ok = ok(defaultValue == null, "test for annotation " +
					"default value", "there is one, but it should NOT be one!")
					&& ok;
		} else {
			ok = ok(defaultValue != null, "test for annotation " +
					"default value", "there is NONE, but it should be one!")
					&& ok;

			ok = ok(AnnotationTester.equals(this.defaultValue, defaultValue),
					"test default value", "expected %s but got %s",
					this.defaultValue, defaultValue)
					&& ok;
		}

		return ok;
	}

	protected Annotation[][] getParameterAnnotations() {
		return annotatedElement().getParameterAnnotations();
	}
}

class ConstructorAnnotationTester
		extends AbstractMethodAnnotationTester<Constructor<?>> {
	public ConstructorAnnotationTester(ClassAnnotationTester clazz,
			Constructor<?> constructor) {
		super(clazz, constructor, constructor.getName(),
				constructor.getParameterTypes());
	}

	protected Annotation[][] getParameterAnnotations() {
		return annotatedElement().getParameterAnnotations();
	}
}

class ClassAnnotationTester extends AnnotatedElementAnnotationTester<Class<?>> {
	private boolean isAnnotation;
	
	private Map<Constructor<?>, ConstructorAnnotationTester> constructors =
		TestHelper.getMapFactory().
		<Constructor<?>, ConstructorAnnotationTester>createMap();
	
	private Map<Method, MethodAnnotationTester> methods =
		TestHelper.getMapFactory().
		<Method, MethodAnnotationTester>createMap();
	
	private Map<String, FieldAnnotationTester> fields =
		TestHelper.getMapFactory().
		<String, FieldAnnotationTester>createMap();

	public ClassAnnotationTester(Class<?> clazz, boolean isAnnotation) {
		super(null, clazz, clazz.getName());

		this.isAnnotation = isAnnotation;
	}

	public ClassAnnotationTester(Class<?> clazz) {
		this(clazz, false);
	}

	public boolean isAnnotation() {
		return isAnnotation;
	}

	public FieldAnnotationTester addField(String name)
			throws SecurityException, NoSuchFieldException {
		FieldAnnotationTester field = new FieldAnnotationTester(this,
				annotatedElement().getField(name));

		if (fields.put(name, field) != null)
			throw new IllegalArgumentException("field already defined");

		return field;
	}

	public MethodAnnotationTester addMethod(String name, Object defalutValue,
			Class<?>... parameterTypes) throws SecurityException,
			NoSuchMethodException {
		Method reflMethod = annotatedElement().getMethod(
				name, parameterTypes);
		MethodAnnotationTester method = new MethodAnnotationTester(this,
				reflMethod, defalutValue);

		if (methods.put(reflMethod, method) != null)
			throw new IllegalArgumentException("method already defined");

		return method;
	}

	public ConstructorAnnotationTester addConstructor(
			Class<?>... parameterTypes) throws SecurityException,
			NoSuchMethodException {
		Constructor<?> reflConstructor =
			annotatedElement().getConstructor(parameterTypes);
		ConstructorAnnotationTester constructor =
			new ConstructorAnnotationTester(this, reflConstructor);

		if (constructors.put(reflConstructor, constructor) != null)
			throw new IllegalArgumentException("constructor already defined");

		return constructor;
	}
	
	protected void logName() {
		TestHelper.log("== Testing %s ==", getName());
	}

	public boolean test() throws SecurityException, NoSuchMethodException {
		boolean ok = super.test();
		
		ok = testIsAnnotation() && ok;

		logHeader("Constructors");
		for (ConstructorAnnotationTester tester :
				TestHelper.sorted(constructors.values())) {
			ok = tester.test() && ok;
		}

		logHeader("Methods");
		for (MethodAnnotationTester tester :
				TestHelper.sorted(methods.values())) {
			ok = tester.test() && ok;
		}

		logHeader("Fields");
		for (FieldAnnotationTester tester : 
				TestHelper.sorted(fields.values())) {
			ok = tester.test() && ok;
		}

		log();
		return ok;
	}
	
	private boolean testIsAnnotation() {
		logHeader("isAnnotation()");
		
		return TestHelper.okx(
				isAnnotation == annotatedElement().isAnnotation(),
				"test if the isAnnotation attribute is set properly",
				annotatedElement().getName(),
				(isAnnotation ? "class should be an annotation, but isn't!"
						: "class should NOT be an annotation, but it is!"),
				EMPTY_OBJECT_ARRAY);
	}
}

/* ********* the testcases ****************************************************/

/**
 * Test Annotations onto enums and their values.
 */
@AnnotationB(string = "onto a enum")
enum EnumA {
	@AnnotationB(string = "onto a enum field")
	VALUE1,
	VALUE2,
	VALUE3
}

/**
 * Test Annotations on Annotations and their methods. Test Annotation on itself.
 */
@Retention(value = RetentionPolicy.RUNTIME)
@AnnotationA(
		integer = 1,
		string  = "onto itself",
		classes = {AnnotationA.class, Class.class},
		enumeration = EnumA.VALUE2)
@interface AnnotationA {
	@AnnotationA(
			integer = 2,
			string  = "onto a method of itself")
	int        integer();
	String     string();
	Class<?>[] classes()     default {EnumA.class, Object[][].class};
	EnumA      enumeration() default EnumA.VALUE1;
}

/**
 * This Annotation will be inherited as Annotation of a derived class.
 * Inheritance applies only for class annotations, not for methods or fields.
 */
@Inherited
@Retention(value = RetentionPolicy.RUNTIME)
@interface AnnotationB {
	String string();
	Class<?> clazz() default AnnotationB.class;
}

/**
 * Test all possible types of enum fields.
 */
@Retention(value = RetentionPolicy.RUNTIME)
@interface AnnotationC {
	byte               aByte()            default 100;
	char               aChar()            default 's';
	short              aShort()           default 88;
	int                aInt()             default Integer.MIN_VALUE;
	long               aLong()            default Long.MAX_VALUE;
	float              aFloat()           default 23.42f;
	double             aDouble()          default 555.0815d;
	String             aString()          default "AOU";
	EnumA              aEnum()            default EnumA.VALUE2;
	Class<?>           aClass()           default EnumA.class;
	SuppressWarnings   aAnnotation()      default @SuppressWarnings("unchecked");
	byte[]             aByteArray()       default {(byte)255};
	char[]             aCharArray()       default {'a', 'o', 'u'};
	short[]            aShortArray()      default {512};
	int[]              aIntArray()        default {640, 480};
	long[]             aLongArray()       default {1204l, 2048l};
	float[]            aFloatArray()      default {0.0f};
	double[]           aDoubleArray()     default {-2.2d, -3.3d};
	String[]           aStringArray()     default {""};
	EnumA[]            aEnumArray()       default EnumA.VALUE1;
	Class<?>[]         aClassArray()      default void.class;
	SuppressWarnings[] aAnnotationArray() default {};
}

/**
 * This annotation will not be stored into the class file.
 */
@interface AnnotationD {
}

/**
 * Test annotations onto a class.
 */
@AnnotationB(string = "onto a class", clazz = Foo.class)
@AnnotationA(integer = 3, string = "onto a class")
class Foo {
	/**
	 * Test annotations onto a field.
	 */
	@AnnotationA(integer = 4, string = "onto a field")
	public int afield;

	/**
	 * Test annotations onto a constructor.
	 */
	@AnnotationA(integer = 9, string = "onto a constructor")
	public Foo() {
	}

	/**
	 * Test annotations onto a method.
	 * 
	 * @param x
	 *            Test annotations onto a parameter.
	 * @return
	 */
	@AnnotationA(integer = 5, string = "onto a method")
	public int method(
			@AnnotationA(
					integer = 6,
					string  = "onto a parameter")
			int x) {
		return x;
	}

	/**
	 * Test annotations onto a static method.
	 * 
	 * @param x
	 *            Test annotations onto a parameter.
	 * @return
	 */
	@AnnotationA(integer = 7, string = "onto a static method")
	public static int staticMethod(
			@AnnotationA(
					integer = 8,
					string  = "onto a parameter of a static method")
			int x) {
		return x;
	}
}

/**
 * Test inheritance of annotations. Test all possible annotation field types as
 * default values. Test if an annotation without RetentionPolicy.RUNTIME is
 * really not visible at runtime.
 */
@AnnotationC
@AnnotationD
class Bar extends Foo {
	/**
	 * Test that superclass field annotations will not be visible here.
	 */
	public int afield;

	/**
	 * Test that superclass constructor annotations will not be visible here.
	 */
	public Bar() {
	}

	/**
	 * Test that superclass method (and parameter) annotations will not be
	 * visible here.
	 */
	public int method(int x) {
		return x;
	}

	/**
	 * Test that superclass method (and parameter) annotations will not be
	 * visible here.
	 */
	public static int staticMethod(int x) {
		return x;
	}
}

/**
 * Test availability of annotations of inherited fields/methods. Test all
 * possible annotation field types. Test if not overloaded (=inherited)
 * methods/fields still have their annotations.
 */
@AnnotationB(string = "onto a derived class", clazz = Baz.class)
@AnnotationC(
		aByte            = 0,
		aChar            = 'a',
		aShort           = 1,
		aInt             = 2,
		aLong            = 3l,
		aFloat           = 4.4f,
		aDouble          = 5.5d,
		aString          = "a string",
		aEnum            = EnumA.VALUE3,
		aClass           = Class.class,
		aAnnotation      = @SuppressWarnings("unchecked"),
		aByteArray       = {0, 1, 2, 3},
		aCharArray       = {'a', 'b', 'c'},
		aShortArray      = 4,
		aIntArray        = {5, 6, 7},
		aLongArray       = {8l, 9l},
		aFloatArray      = {10.10f, 11.11f, 12.12f},
		aDoubleArray     = {},
		aStringArray     = {"a string","another string"},
		aEnumArray       = {EnumA.VALUE3, EnumA.VALUE3},
		aClassArray      = {Class.class, Integer.class, Long.class},
		aAnnotationArray = {
			@SuppressWarnings(value = "unchecked"),
			@SuppressWarnings(value = {"unused", "deprecation"})})
class Baz extends Foo {
}

/* ********* running the testcases ********************************************/
public class TestAnnotations {
	@SuppressWarnings("unchecked")
	public static void main(String[] args) {
		boolean ok = true;
		MethodAnnotationTester mtester;

		try {
			ClassAnnotationTester classEnumA       =
				new ClassAnnotationTester(EnumA.class);
			ClassAnnotationTester classAnnotationA =
				new ClassAnnotationTester(AnnotationA.class, true);
			ClassAnnotationTester classAnnotationB =
				new ClassAnnotationTester(AnnotationB.class, true);
			ClassAnnotationTester classAnnotationC =
				new ClassAnnotationTester(AnnotationC.class, true);
			ClassAnnotationTester classAnnotationD =
				new ClassAnnotationTester(AnnotationD.class, true);
			ClassAnnotationTester classFoo         =
				new ClassAnnotationTester(Foo.class);
			ClassAnnotationTester classBar         =
				new ClassAnnotationTester(Bar.class);
			ClassAnnotationTester classBaz         =
				new ClassAnnotationTester(Baz.class);

			/* EnumA */
			classEnumA.putDeclaredAnnotation(
					AnnotationB.class,
					new Entry("string", "onto a enum"),
					new Entry("clazz", AnnotationB.class));
			classEnumA.addField("VALUE1").putDeclaredAnnotation(
					AnnotationB.class,
					new Entry("string", "onto a enum field"),
					new Entry("clazz", AnnotationB.class)
			);

			/* AnnotationA */
			classAnnotationA.putDeclaredAnnotation(
					Retention.class,
					new Entry("value", RetentionPolicy.RUNTIME)
			);
			classAnnotationA.putDeclaredAnnotation(
					AnnotationA.class,
					new Entry("integer", 1),
					new Entry("string",  "onto itself"),
					new Entry("classes", new Class<?>[] {
							AnnotationA.class, Class.class}),
					new Entry("enumeration", EnumA.VALUE2)
			);
			classAnnotationA.addMethod("integer", null).putDeclaredAnnotation(
					AnnotationA.class,
					new Entry("integer", 2),
					new Entry("string", "onto a method of itself"),
					new Entry("classes", new Class<?>[] {
							EnumA.class, Object[][].class }),
					new Entry("enumeration", EnumA.VALUE1)
			);
			classAnnotationA.addMethod("classes", 
					new Class<?>[] {EnumA.class, Object[][].class});
			classAnnotationA.addMethod("enumeration", EnumA.VALUE1);

			/* AnnotationB */
			classAnnotationB.putDeclaredAnnotation(Inherited.class);
			classAnnotationB.putDeclaredAnnotation(
					Retention.class,
					new Entry("value", RetentionPolicy.RUNTIME)
			);
			classAnnotationB.addMethod("clazz", AnnotationB.class);

			/* AnnotationC */
			classAnnotationC.putDeclaredAnnotation(
					Retention.class,
					new Entry("value", RetentionPolicy.RUNTIME)
			);

			/* Foo */
			classFoo.putDeclaredAnnotation(
					AnnotationB.class,
					new Entry("string", "onto a class"),
					new Entry("clazz",  Foo.class)
			);
			classFoo.putDeclaredAnnotation(
					AnnotationA.class,
					new Entry("integer", 3),
					new Entry("string",  "onto a class"),
					new Entry("classes", new Class<?>[] {
							EnumA.class, Object[][].class}),
					new Entry("enumeration", EnumA.VALUE1)
			);
			classFoo.addField("afield").putDeclaredAnnotation(
					AnnotationA.class,
					new Entry("integer", 4),
					new Entry("string",  "onto a field"),
					new Entry("classes", new Class<?>[] {
							EnumA.class, Object[][].class}),
					new Entry("enumeration", EnumA.VALUE1)
			);
			mtester = classFoo.addMethod("method", null, int.class);
			mtester.putDeclaredAnnotation(
					AnnotationA.class,
					new Entry("integer", 5),
					new Entry("string",  "onto a method"),
					new Entry("classes", new Class<?>[] {
							EnumA.class, Object[][].class}),
					new Entry("enumeration", EnumA.VALUE1)
			);
			mtester.putParameterAnnotation(0,
					AnnotationA.class,
					new Entry("integer", 6),
					new Entry("string", "onto a parameter"),
					new Entry("classes", new Class<?>[] {
							EnumA.class, Object[][].class}),
					new Entry("enumeration", EnumA.VALUE1)
			);
			mtester = classFoo.addMethod("staticMethod", null, int.class);
			mtester.putDeclaredAnnotation(
					AnnotationA.class,
					new Entry("integer", 7),
					new Entry("string", "onto a static method"),
					new Entry("classes", new Class<?>[] {
							EnumA.class, Object[][].class }),
					new Entry("enumeration", EnumA.VALUE1)
			);
			mtester.putParameterAnnotation(0,
					AnnotationA.class,
					new Entry("integer", 8),
					new Entry("string", "onto a parameter of a static method"),
					new Entry("classes", new Class<?>[] {
							EnumA.class, Object[][].class}),
					new Entry("enumeration", EnumA.VALUE1)
			);
			classFoo.addConstructor().putDeclaredAnnotation(
					AnnotationA.class,
					new Entry("integer", 9),
					new Entry("string", "onto a constructor"),
					new Entry("classes", new Class<?>[] {
							EnumA.class, Object[][].class}),
					new Entry("enumeration", EnumA.VALUE1)
			);

			/* Bar */
			classBar.putInheritedAnnotation(
					AnnotationB.class,
					new Entry("string", "onto a class"),
					new Entry("clazz", Foo.class)
			);
			classBar.putDeclaredAnnotation(
					AnnotationC.class,
					new Entry("aByte",            (byte)100),
					new Entry("aChar",            (char)'s'),
					new Entry("aShort",           (short)88),
					new Entry("aInt",             Integer.MIN_VALUE),
					new Entry("aLong",            Long.MAX_VALUE),
					new Entry("aFloat",           (float)23.42f),
					new Entry("aDouble",          (double)555.0815d),
					new Entry("aString",          "AOU"),
					new Entry("aEnum",            EnumA.VALUE2),
					new Entry("aClass",           EnumA.class),
					new Entry("aAnnotation",      new AnnotationTester(
							SuppressWarnings.class,
							new Entry("value", new String[] {"unchecked"}))),
					new Entry("aByteArray",       new byte[]   {(byte) 255}),
					new Entry("aCharArray",       new char[]   {'a', 'o', 'u'}),
					new Entry("aShortArray",      new short[]  {512}),
					new Entry("aIntArray",        new int[]    {640, 480}),
					new Entry("aLongArray",       new long[]   {1204l, 2048l}),
					new Entry("aFloatArray",      new float[]  {0.0f}),
					new Entry("aDoubleArray",     new double[] {-2.2d, -3.3d}),
					new Entry("aStringArray",     new String[] {""}),
					new Entry("aEnumArray",       new EnumA[]  {EnumA.VALUE1}),
					new Entry("aClassArray",      new Class<?>[] {void.class}),
					new Entry("aAnnotationArray", new AnnotationTester[] {})
			);
			classBar.addField("afield");
			classBar.addMethod("method", null, int.class);
			classBar.addMethod("staticMethod", null, int.class);
			classBar.addConstructor();

			/* Baz */
			classBaz.putDeclaredAnnotation(
					AnnotationB.class,
					new Entry("string", "onto a derived class"),
					new Entry("clazz",  Baz.class)
			);
			classBaz.putDeclaredAnnotation(
					AnnotationC.class,
					new Entry("aByte",            (byte)0),
					new Entry("aChar",            (char)'a'),
					new Entry("aShort",           (short)1),
					new Entry("aInt",             (int)2),
					new Entry("aLong",            (long)3l),
					new Entry("aFloat",           (float)4.4f),
					new Entry("aDouble",          (double)5.5d),
					new Entry("aString",          "a string"),
					new Entry("aEnum",            EnumA.VALUE3),
					new Entry("aClass",           Class.class),
					new Entry("aAnnotation",      new AnnotationTester(
							SuppressWarnings.class,
							new Entry("value",new String[] {"unchecked"}))),
					new Entry("aByteArray",       new byte[]  {0, 1, 2, 3}),
					new Entry("aCharArray",       new char[]  {'a', 'b', 'c'}),
					new Entry("aShortArray",      new short[] {4}),
					new Entry("aIntArray",        new int[]   {5, 6, 7}),
					new Entry("aLongArray",       new long[]  {8l, 9l}), 
					new Entry("aFloatArray",      new float[] {
							10.10f, 11.11f, 12.12f}),
					new Entry("aDoubleArray",     new double[] {}),
					new Entry("aStringArray",     new String[] {
							"a string",	"another string"}),
					new Entry("aEnumArray",       new EnumA[] {
							EnumA.VALUE3, EnumA.VALUE3}),
					new Entry("aClassArray",      new Class<?>[] {
							Class.class, Integer.class, Long.class}),
					new Entry("aAnnotationArray", new AnnotationTester[] {
							new AnnotationTester(
									SuppressWarnings.class,
									new Entry("value", new String[] {
											"unchecked"})),
							new AnnotationTester(
									SuppressWarnings.class,
									new Entry("value", new String[] {
											"unused", "deprecation"}))})
			);
			classBaz.addField("afield").putDeclaredAnnotation(
					AnnotationA.class,
					new Entry("integer", 4),
					new Entry("string", "onto a field"),
					new Entry("classes", new Class<?>[] {
							EnumA.class, Object[][].class}),
					new Entry("enumeration", EnumA.VALUE1)
			);
			mtester = classBaz.addMethod("method", null, int.class);
			mtester.putDeclaredAnnotation(
					AnnotationA.class,
					new Entry("integer", 5),
					new Entry("string",  "onto a method"),
					new Entry("classes", new Class<?>[] {
							EnumA.class, Object[][].class }),
					new Entry("enumeration", EnumA.VALUE1)
			);
			mtester.putParameterAnnotation(0,
					AnnotationA.class,
					new Entry("integer", 6),
					new Entry("string",  "onto a parameter"),
					new Entry("classes", new Class<?>[] {
							EnumA.class, Object[][].class }),
					new Entry("enumeration", EnumA.VALUE1)
			);
			mtester = classBaz.addMethod("staticMethod", null, int.class);
			mtester.putDeclaredAnnotation(
					AnnotationA.class,
					new Entry("integer",     7),
					new Entry("string",      "onto a static method"),
					new Entry("classes",     new Class<?>[] {
							EnumA.class, Object[][].class}),
					new Entry("enumeration", EnumA.VALUE1)
			);
			mtester.putParameterAnnotation(0,
					AnnotationA.class,
					new Entry("integer", 8),
					new Entry("string",	 "onto a parameter of a static method"),
					new Entry("classes", new Class<?>[] {
							EnumA.class, Object[][].class}),
					new Entry("enumeration", EnumA.VALUE1)
			);

			ok = classEnumA.test();
			ok = classAnnotationA.test() && ok;
			ok = classAnnotationB.test() && ok;
			ok = classAnnotationC.test() && ok;
			ok = classAnnotationD.test() && ok;
			ok = classFoo.test() && ok;
			ok = classBar.test() && ok;
			ok = classBaz.test() && ok;
		} catch (Exception e) {
			ok = TestHelper.ok(false, "exception free execution\n");
			e.printStackTrace();
		}
		
		TestHelper.printStatistics();

		if (!ok) {
			System.exit(1);
		}
	}
}
