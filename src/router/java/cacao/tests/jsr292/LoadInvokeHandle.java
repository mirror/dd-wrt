import java.lang.reflect.*;

import org.junit.*;

import java.lang.invoke.*;
import static java.lang.invoke.MethodHandles.*;
import static java.lang.invoke.MethodType.*;

import java.util.Arrays;

/***
 * Smoke test for loader, a file that contains MethodHandles & MethodTypes in the constant pool.
 * Nothing is executed.
 *
 * This class is intended to be processed by the indify tool.
 */
public class LoadInvokeHandle {
	@Test
	public void test() throws Throwable {
	}

	// perform a invokedynamic call so there is a CONSTANT_InvokeDynamic entry in the constant pool
	// not actually called by test() on purpose.
	public Integer callIndy() throws Throwable {
		return (Integer) INDY_tester().invokeExact((Integer) 5, (Integer) 6);
	}

	private static void untransformed(String self) {
		throw new RuntimeException("Untransformed call to indified method");
	}

	// actual methods
	public static Integer adder(Integer x, Integer y) { return x + y; }

	public static Object bootstrap(Object x, Object y, Object z) throws Throwable {
		return new ConstantCallSite(MH_adder());
	}

	// method types 
	private static MethodType MT_gen1() {
		untransformed("MT_gen1");
		return methodType(Object.class, Object.class);
	}
	private static MethodType MT_gen3() {
		untransformed("MT_gen3");
		return methodType(Object.class, Object.class, Object.class, Object.class);
	}

	// method handles
	private static MethodHandle MH_adder() throws ReflectiveOperationException {
		untransformed("MH_adder");
		return lookup().findStatic(LoadInvokeHandle.class, "adder", methodType(Integer.class, Integer.class, Integer.class));
	}

	private static MethodHandle MH_bootstrap() throws ReflectiveOperationException {
		untransformed("MH_bootstrap");
		return lookup().findStatic(LoadInvokeHandle.class, "bootstrap", MT_gen3());
	}

	// invokedynamic
	private static MethodHandle INDY_tester() throws Throwable {
		untransformed("INDY_tester");

		CallSite cs = (CallSite) MH_bootstrap().invokeWithArguments(lookup(), "indy", fromMethodDescriptorString("(Ljava/lang/Integer;Ljava/lang/Integer;)Ljava/lang/Integer;", null));
		return cs.dynamicInvoker();
	}
}
