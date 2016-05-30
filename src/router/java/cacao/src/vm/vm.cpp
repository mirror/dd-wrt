/* src/vm/vm.cpp - VM startup and shutdown functions

   Copyright (C) 1996-2014
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


#include "config.h"

#include <cassert>
#include <cerrno>
#include <cstdlib>
#include <exception>
#include <stdint.h>
#include <inttypes.h>

#include "md-abi.hpp"

#include "mm/codememory.hpp"
#include "mm/dumpmemory.hpp"
#include "mm/gc.hpp"
#include "mm/memory.hpp"

#include "native/jni.hpp"
#include "native/llni.hpp"
#include "native/localref.hpp"
#include "native/native.hpp"

#include "native/vm/nativevm.hpp"

#include "threads/lock.hpp"
#include "threads/thread.hpp"

#include "toolbox/logging.hpp"

#include "vm/array.hpp"
#include "vm/assertion.hpp"
#include "vm/classcache.hpp"
#include "vm/exceptions.hpp"
#include "vm/descriptor.hpp"
#include "vm/finalizer.hpp"
#include "vm/global.hpp"
#include "vm/globals.hpp"
#include "vm/hook.hpp"
#include "vm/initialize.hpp"
#include "vm/javaobjects.hpp"
#include "vm/options.hpp"
#include "vm/os.hpp"
#include "vm/primitive.hpp"
#include "vm/properties.hpp"
#include "vm/rt-timing.hpp"
#include "vm/signallocal.hpp"
#include "vm/statistics.hpp"
#include "vm/string.hpp"
#include "vm/suck.hpp"
#include "vm/types.hpp"
#include "vm/vm.hpp"

#include "vm/jit/abi-asm.hpp"
#include "vm/jit/argument.hpp"
#include "vm/jit/asmpart.hpp"
#include "vm/jit/builtin.hpp"
#include "vm/jit/code.hpp"
#include "vm/jit/disass.hpp"
#include "vm/jit/jit.hpp"
#include "vm/jit/methodtree.hpp"
#include "vm/jit/stacktrace.hpp"
#include "vm/jit/trap.hpp"

#include "vm/jit/optimizing/profile.hpp"
#include "vm/jit/optimizing/recompiler.hpp"

using namespace cacao;


STAT_DECLARE_GROUP(function_call_stat)
STAT_REGISTER_GROUP_VAR(u8,count_calls_native_to_java,0,"calls native to java","native-to-java calls",function_call_stat)
/**
 * This is _the_ VM instance.
 */
VM* VM::_vm = NULL;


/* global variables ***********************************************************/

s4 vms = 0;                             /* number of VMs created              */

#if !defined(NDEBUG)
static classinfo *mainclass = NULL;
#endif

#if defined(ENABLE_INTRP)
u1 *intrp_main_stack = NULL;
#endif


/* define heap sizes **********************************************************/

#define HEAP_MAXSIZE      128 * 1024 * 1024 /* default 128MB                  */
#define HEAP_STARTSIZE      2 * 1024 * 1024 /* default 2MB                    */
#define STACK_SIZE               512 * 1024 /* default 512kB                  */


/* define command line options ************************************************/

enum {
	OPT_FOO,

	/* Java options */

	OPT_JAR,

	OPT_D32,
	OPT_D64,

	OPT_CLASSPATH,
	OPT_D,

	OPT_VERBOSE,

	OPT_VERSION,
	OPT_SHOWVERSION,
	OPT_FULLVERSION,

	OPT_HELP,
	OPT_X,
	OPT_XX,

	OPT_EA,
	OPT_DA,
	OPT_EA_NOARG,
	OPT_DA_NOARG,


	OPT_ESA,
	OPT_DSA,

	/* Java non-standard options */

	OPT_JIT,
	OPT_INTRP,

	OPT_BOOTCLASSPATH,
	OPT_BOOTCLASSPATH_A,
	OPT_BOOTCLASSPATH_P,

	OPT_BOOTCLASSPATH_C,

#if defined(ENABLE_PROFILING)
	OPT_PROF,
	OPT_PROF_OPTION,
#endif

	OPT_MS,
	OPT_MX,

	OPT_XCHECK_JNI,

	/* CACAO options */

	OPT_VERBOSE1,

#if defined(ENABLE_STATISTICS)
	OPT_TIME,
	OPT_STAT,
#endif

	OPT_LOG,
	OPT_CHECK,
	OPT_LOAD,
	OPT_SHOW,
	OPT_DEBUGCOLOR,

#if defined(ENABLE_VERIFIER)
	OPT_NOVERIFY,
	OPT_XVERIFY_ALL,
	OPT_XVERIFY_NONE,
#if defined(TYPECHECK_VERBOSE)
	OPT_VERBOSETC,
#endif
#endif /* defined(ENABLE_VERIFIER) */

	/* optimization options */

#if defined(ENABLE_LOOP)
	OPT_OLOOP,
#endif

#if defined(ENABLE_IFCONV)
	OPT_IFCONV,
#endif

#if defined(ENABLE_LSRA) || defined(ENABLE_SSA)
	OPT_LSRA,
#endif

#if defined(ENABLE_INTRP)
	/* interpreter options */

	OPT_NO_DYNAMIC,
	OPT_NO_REPLICATION,
	OPT_NO_QUICKSUPER,
	OPT_STATIC_SUPERS,
	OPT_TRACE,
#endif

	OPT_SS,

#if defined(ENABLE_JVMTI)
	OPT_AGENTLIB,
	OPT_AGENTPATH,
	OPT_RUN,
#endif

#if defined(ENABLE_DEBUG_FILTER)
	OPT_FILTER_VERBOSECALL_INCLUDE,
	OPT_FILTER_VERBOSECALL_EXCLUDE,
	OPT_FILTER_SHOW_METHOD,
#endif

	DUMMY
};


opt_struct opts[] = {
	{ "foo",               false, OPT_FOO },

	/* Java options */

	{ "jar",               false, OPT_JAR },

	{ "d32",               false, OPT_D32 },
	{ "d64",               false, OPT_D64 },
	{ "client",            false, OPT_IGNORE },
	{ "server",            false, OPT_IGNORE },
	{ "jvm",               false, OPT_IGNORE },
	{ "hotspot",           false, OPT_IGNORE },

	{ "classpath",         true,  OPT_CLASSPATH },
	{ "cp",                true,  OPT_CLASSPATH },
	{ "D",                 true,  OPT_D },
	{ "version",           false, OPT_VERSION },
	{ "showversion",       false, OPT_SHOWVERSION },
	{ "fullversion",       false, OPT_FULLVERSION },
	{ "help",              false, OPT_HELP },
	{ "?",                 false, OPT_HELP },
	{ "X",                 false, OPT_X },
	{ "XX:",               true,  OPT_XX },

	{ "ea:",               true,  OPT_EA },
	{ "da:",               true,  OPT_DA },
	{ "ea",                false, OPT_EA_NOARG },
	{ "da",                false, OPT_DA_NOARG },

	{ "enableassertions:",  true,  OPT_EA },
	{ "disableassertions:", true,  OPT_DA },
	{ "enableassertions",   false, OPT_EA_NOARG },
	{ "disableassertions",  false, OPT_DA_NOARG },

	{ "esa",                     false, OPT_ESA },
	{ "enablesystemassertions",  false, OPT_ESA },
	{ "dsa",                     false, OPT_DSA },
	{ "disablesystemassertions", false, OPT_DSA },

	{ "noasyncgc",         false, OPT_IGNORE },
#if defined(ENABLE_VERIFIER)
	{ "noverify",          false, OPT_NOVERIFY },
	{ "Xverify:all",       false, OPT_XVERIFY_ALL },
	{ "Xverify:none",      false, OPT_XVERIFY_NONE },
#endif
	{ "v",                 false, OPT_VERBOSE1 },
	{ "verbose:",          true,  OPT_VERBOSE },

#if defined(ENABLE_VERIFIER) && defined(TYPECHECK_VERBOSE)
	{ "verbosetc",         false, OPT_VERBOSETC },
#endif
#if defined(ENABLE_STATISTICS)
	{ "time",              false, OPT_TIME },
	{ "stat",              false, OPT_STAT },
#endif
	{ "log",               true,  OPT_LOG },
	{ "c",                 true,  OPT_CHECK },
	{ "l",                 false, OPT_LOAD },

#if defined(ENABLE_LOOP)
	{ "oloop",             false, OPT_OLOOP },
#endif
#if defined(ENABLE_IFCONV)
	{ "ifconv",            false, OPT_IFCONV },
#endif
#if defined(ENABLE_LSRA)
	{ "lsra",              false, OPT_LSRA },
#endif
#if  defined(ENABLE_SSA)
	{ "lsra",              true, OPT_LSRA },
#endif

#if defined(ENABLE_INTRP)
	/* interpreter options */

	{ "trace",             false, OPT_TRACE },
	{ "static-supers",     true,  OPT_STATIC_SUPERS },
	{ "no-dynamic",        false, OPT_NO_DYNAMIC },
	{ "no-replication",    false, OPT_NO_REPLICATION },
	{ "no-quicksuper",     false, OPT_NO_QUICKSUPER },
#endif

	/* JVMTI Agent Command Line Options */
#if defined(ENABLE_JVMTI)
	{ "agentlib:",         true,  OPT_AGENTLIB },
	{ "agentpath:",        true,  OPT_AGENTPATH },
#endif

	/* Java non-standard options */

	{ "Xjit",              false, OPT_JIT },
	{ "Xint",              false, OPT_INTRP },
	{ "Xbootclasspath:",   true,  OPT_BOOTCLASSPATH },
	{ "Xbootclasspath/a:", true,  OPT_BOOTCLASSPATH_A },
	{ "Xbootclasspath/p:", true,  OPT_BOOTCLASSPATH_P },
	{ "Xbootclasspath/c:", true,  OPT_BOOTCLASSPATH_C },

#if defined(ENABLE_JVMTI)
	{ "Xdebug",            false, OPT_IGNORE },
	{ "Xnoagent",          false, OPT_IGNORE },
	{ "Xrun",              true,  OPT_RUN },
#endif

	{ "Xms",               true,  OPT_MS },
	{ "ms",                true,  OPT_MS },
	{ "Xmx",               true,  OPT_MX },
	{ "mx",                true,  OPT_MX },
	{ "Xss",               true,  OPT_SS },
	{ "ss",                true,  OPT_SS },

	{ "Xcheck:jni",        false, OPT_XCHECK_JNI },

#if defined(ENABLE_PROFILING)
	{ "Xprof:",            true,  OPT_PROF_OPTION },
	{ "Xprof",             false, OPT_PROF },
#endif

	/* keep these at the end of the list */

	{ "s",                 true,  OPT_SHOW },
	{ "debug-color",      false,  OPT_DEBUGCOLOR },

#if defined(ENABLE_DEBUG_FILTER)
	{ "XXfi",              true,  OPT_FILTER_VERBOSECALL_INCLUDE },
	{ "XXfx",              true,  OPT_FILTER_VERBOSECALL_EXCLUDE },
	{ "XXfm",              true,  OPT_FILTER_SHOW_METHOD },
#endif

	{ NULL,                false, 0 }
};


/* usage ***********************************************************************

   Prints the correct usage syntax to stdout.

*******************************************************************************/

static void usage(void)
{
	puts("Usage: cacao [-options] classname [arguments]");
	puts("               (to run a class file)");
	puts("   or  cacao [-options] -jar jarfile [arguments]");
	puts("               (to run a standalone jar file)\n");

	puts("where options include:");
	puts("    -d32                     use 32-bit data model if available");
	puts("    -d64                     use 64-bit data model if available");
	puts("    -client                  compatibility (currently ignored)");
	puts("    -server                  compatibility (currently ignored)");
	puts("    -jvm                     compatibility (currently ignored)");
	puts("    -hotspot                 compatibility (currently ignored)\n");

	puts("    -cp <path>               specify a path to look for classes");
	puts("    -classpath <path>        specify a path to look for classes");
	puts("    -D<name>=<value>         add an entry to the property list");
	puts("    -verbose[:class|gc|jni]  enable specific verbose output");
	puts("    -version                 print product version and exit");
	puts("    -fullversion             print jpackage-compatible product version and exit");
	puts("    -showversion             print product version and continue");
	puts("    -help, -?                print this help message");
	puts("    -X                       print help on non-standard Java options");
	puts("    -XX                      print help on debugging options");
    puts("    -ea[:<packagename>...|:<classname>]");
    puts("    -enableassertions[:<packagename>...|:<classname>]");
	puts("                             enable assertions with specified granularity");
	puts("    -da[:<packagename>...|:<classname>]");
	puts("    -disableassertions[:<packagename>...|:<classname>]");
	puts("                             disable assertions with specified granularity");
	puts("    -esa | -enablesystemassertions");
	puts("                             enable system assertions");
	puts("    -dsa | -disablesystemassertions");
	puts("                             disable system assertions");

#if defined(ENABLE_JVMTI)
	puts("    -agentlib:<agent-lib-name>=<options>");
	puts("                             load native agent library by library name");
	puts("                             for additional help use: -agentlib:jdwp=help");
	puts("    -agentpath:<path-to-agent>=<options>");
	puts("                             load native agent library by full pathname");
#endif

	/* exit with error code */

	exit(1);
}


static void Xusage(void)
{
#if defined(ENABLE_JIT)
	puts("    -Xjit                    JIT mode execution (default)");
#endif
#if defined(ENABLE_INTRP)
	puts("    -Xint                    interpreter mode execution");
#endif
	puts("    -Xbootclasspath:<zip/jar files and directories separated by :>");
    puts("                             value is set as bootstrap class path");
	puts("    -Xbootclasspath/a:<zip/jar files and directories separated by :>");
	puts("                             value is appended to the bootstrap class path");
	puts("    -Xbootclasspath/p:<zip/jar files and directories separated by :>");
	puts("                             value is prepended to the bootstrap class path");
	puts("    -Xbootclasspath/c:<zip/jar files and directories separated by :>");
	puts("                             value is used as Java core library, but the");
	puts("                             hardcoded VM interface classes are prepended");
	printf("    -Xms<size>               set the initial size of the heap (default: %dMB)\n", HEAP_STARTSIZE / 1024 / 1024);
	printf("    -Xmx<size>               set the maximum size of the heap (default: %dMB)\n", HEAP_MAXSIZE / 1024 / 1024);
	printf("    -Xss<size>               set the thread stack size (default: %dkB)\n", STACK_SIZE / 1024);

#if defined(ENABLE_PROFILING)
	puts("    -Xprof[:bb]              collect and print profiling data");
#endif

	/* exit with error code */

	exit(1);
}


#if 0
static void XXusage(void)
{
	puts("    -v                       write state-information");
#if !defined(NDEBUG)
	puts("    -verbose:jit             enable specific verbose output");
	puts("    -debug-color             colored output for ANSI terms");
#endif
#ifdef TYPECHECK_VERBOSE
	puts("    -verbosetc               write debug messages while typechecking");
#endif
#if defined(ENABLE_VERIFIER)
	puts("    -noverify                don't verify classfiles");
#endif
#if defined(ENABLE_STATISTICS)
	puts("    -time                    measure the runtime");
	puts("    -stat                    detailed compiler statistics");
#endif
	puts("    -log logfile             specify a name for the logfile");
	puts("    -c(heck)b(ounds)         don't check array bounds");
	puts("            s(ync)           don't check for synchronization");
#if defined(ENABLE_LOOP)
	puts("    -oloop                   optimize array accesses in loops");
#endif
	puts("    -l                       don't start the class after loading");

	puts("    -s...                    show...");
	puts("      (c)onstants            the constant pool");
	puts("      (m)ethods              class fields and methods");
	puts("      (u)tf                  the utf - hash");
	puts("      (i)ntermediate         intermediate representation");
#if defined(ENABLE_DISASSEMBLER)
	puts("      (a)ssembler            disassembled listing");
	puts("      n(o)ps                 show NOPs in disassembler output");
#endif
	puts("      (d)atasegment          data segment listing");

#if defined(ENABLE_IFCONV)
	puts("    -ifconv                  use if-conversion");
#endif
#if defined(ENABLE_LSRA)
	puts("    -lsra                    use linear scan register allocation");
#endif
#if defined(ENABLE_SSA)
	puts("    -lsra:...                use linear scan register allocation (with SSA)");
	puts("       (d)ead code elimination");
	puts("       (c)opy propagation");
#endif
#if defined(ENABLE_DEBUG_FILTER)
	puts("    -XXfi <regex>            begin of dynamic scope for verbosecall filter");
	puts("    -XXfx <regex>            end of dynamic scope for verbosecall filter");
	puts("    -XXfm <regex>            filter for show options");
#endif
	/* exit with error code */

	exit(1);
}
#endif


/* version *********************************************************************

   Only prints cacao version information.

*******************************************************************************/

static void version(bool opt_exit)
{
	puts("java version \"" JAVA_VERSION "\"");
	puts("CACAO version " VERSION_FULL "\n");

	puts("Copyright (C) 1996-2014");
	puts("CACAOVM - Verein zur Foerderung der freien virtuellen Maschine CACAO");
	puts("This is free software; see the source for copying conditions.  There is NO");
	puts("warranty; not even for MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.");

	/* exit normally, if requested */

	if (opt_exit)
		exit(0);
}


/* fullversion *****************************************************************

   Prints a Sun compatible version information (required e.g. by
   jpackage, www.jpackage.org).

*******************************************************************************/

static void fullversion(void)
{
	puts("java full version \"cacao-" JAVA_VERSION "\"");

	/* exit normally */

	exit(0);
}


/* forward declarations *******************************************************/

static char *vm_get_mainclass_from_jar(char *mainstring);
#if !defined(NDEBUG)
static void  vm_compile_all(void);
static void  vm_compile_method(char* mainname);
#endif


/**
 * Implementation for JNI_CreateJavaVM.  This function creates a VM
 * object.
 *
 * @param p_vm
 * @param p_env
 * @param vm_args
 *
 * @return true on success, false otherwise.
 */
bool VM::create(JavaVM** p_vm, void** p_env, void* vm_args)
{
	JavaVMInitArgs* _vm_args;

	// Get the arguments for the new JVM.
	_vm_args = (JavaVMInitArgs *) vm_args;

	// Instantiate a new VM.
	try {
		_vm = new VM(_vm_args);
	}
	catch (std::exception e) {
		// FIXME How can we delete the resources allocated?
// 		/* release allocated memory */
// 		FREE(env, _Jv_JNIEnv);
// 		FREE(vm, _Jv_JavaVM);

		_vm = NULL;

		return false;
	}

	// Return the values.

	*p_vm  = _vm->get_javavm();
	*p_env = _vm->get_jnienv();

	return true;
}


/**
 * C wrapper for VM::create.
 */
extern "C" {
	bool VM_create(JavaVM** p_vm, void** p_env, void* vm_args)
	{
		return VM::create(p_vm, p_env, vm_args);
	}
}


/**
 * VM constructor.
 */
VM::VM(JavaVMInitArgs* vm_args)
{
	// Very first thing to do: we are initializing.
	_initializing = true;

	// Make sure logging works


	// Make ourself globally visible.
	// XXX Is this a good idea?
	_vm = this;

	/* create and fill a JavaVM structure */

	_javavm = new JavaVM();

#if defined(ENABLE_JNI)
	_javavm->functions = &_Jv_JNIInvokeInterface;
#endif

	/* get the VM and Env tables (must be set before vm_create) */
	/* XXX JVMTI Agents needs a JavaVM  */

	_jnienv = new JNIEnv();

#if defined(ENABLE_JNI)
	_jnienv->functions = &_Jv_JNINativeInterface;
#endif

	/* actually create the JVM */

	int   len = 0;
	char *p;
	char *boot_class_path;
	char *boot_class_path_p = NULL;
	char *class_path;
	int   opt;
	bool  opt_version;
	bool  opt_exit;

#if defined(ENABLE_JNI)
	/* Check the JNI version requested. */

	if (!jni_version_check(vm_args->version))
		throw std::exception();
#endif

	/* We only support 1 JVM instance. */

	if (vms > 0)
		throw std::exception();

	/* Install the exit handler. */

	if (atexit(vm_exit_handler))
		os::abort("atexit failed: %s\n", strerror(errno));

	/* Set some options. */

	opt_version       = false;
	opt_exit          = false;

	opt_heapmaxsize   = HEAP_MAXSIZE;
	opt_heapstartsize = HEAP_STARTSIZE;
	opt_stacksize     = STACK_SIZE;

	// First of all, parse the -XX options.
	options_xx(vm_args);

	// After -XX options are parsed, print the build-time
	// configuration, if requested.
	if (opt_PrintConfig)
		print_build_time_config();

	/* We need to check if the actual size of a java.lang.Class object
	   is smaller or equal than the assumption made in
	   src/vm/class.hpp. */

// FIXME We need to check the size of java.lang.Class!!!
// 	if (sizeof(java_lang_Class) > sizeof(dummy_java_lang_Class))
// 		vm_abort("vm_create: java_lang_Class structure is bigger than classinfo.object (%d > %d)", sizeof(java_lang_Class), sizeof(dummy_java_lang_Class));

	/* set the VM starttime */

	_starttime = builtin_currenttimemillis();

	/* iterate over all passed options */

	while ((opt = options_get(opts, vm_args)) != OPT_DONE) {
		switch (opt) {
		case OPT_FOO:
			opt_foo = true;
			break;

		case OPT_IGNORE:
			break;

		case OPT_JAR:
			opt_jar = true;
			break;

		case OPT_D32:
#if SIZEOF_VOID_P == 8
			puts("Running a 32-bit JVM is not supported on this platform.");
			exit(1);
#endif
			break;

		case OPT_D64:
#if SIZEOF_VOID_P == 4
			puts("Running a 64-bit JVM is not supported on this platform.");
			exit(1);
#endif
			break;

		case OPT_CLASSPATH:
			/* Forget old classpath and set the argument as new
			   classpath. */

			// FIXME Make class_path const char*.
			class_path = (char*) _properties.get("java.class.path");

			p = MNEW(char, strlen(opt_arg) + strlen("0"));

			strcpy(p, opt_arg);

#if defined(ENABLE_JAVASE)
			_properties.put("java.class.path", p);
#endif

			MFREE(class_path, char, strlen(class_path));
			break;

		case OPT_D:
			for (unsigned int i = 0; i < strlen(opt_arg); i++) {
				if (opt_arg[i] == '=') {
					opt_arg[i] = '\0';
					_properties.put(opt_arg, opt_arg + i + 1);
					goto opt_d_done;
				}
			}

			/* if no '=' is given, just create an empty property */

			_properties.put(opt_arg, "");

		opt_d_done:
			break;

		case OPT_BOOTCLASSPATH:
			/* Forget default bootclasspath and set the argument as
			   new boot classpath. */

			/* Forget stored -Xbootclasspath/p value */
			if (boot_class_path_p != NULL) {
				MFREE(boot_class_path_p, char, len);
				boot_class_path_p = NULL;
			}
			// FIXME Make boot_class_path const char*.
			boot_class_path = (char*) _properties.get("sun.boot.class.path");

			p = MNEW(char, strlen(opt_arg) + strlen("0"));

			strcpy(p, opt_arg);

			_properties.put("sun.boot.class.path", p);
			_properties.put("java.boot.class.path", p);

			MFREE(boot_class_path, char, strlen(boot_class_path));
			break;

		case OPT_BOOTCLASSPATH_A:
			/* Append to bootclasspath. */

			// FIXME Make boot_class_path const char*.
			boot_class_path = (char*) _properties.get("sun.boot.class.path");

			len = strlen(boot_class_path);

			// XXX (char*) quick hack
			p = (char*) MREALLOC(boot_class_path,
						 char,
						 len + strlen("0"),
						 len + strlen(":") +
						 strlen(opt_arg) + strlen("0"));

			strcat(p, ":");
			strcat(p, opt_arg);

			_properties.put("sun.boot.class.path", p);
			_properties.put("java.boot.class.path", p);
			break;

		case OPT_BOOTCLASSPATH_P:
			/* Prepend to bootclasspath. */
			/* Note: we can not add this value directly to sun/java.boot.class.path
			   because we need to take care of the ordering regarding the
			   endorseddirs property */

			if (boot_class_path_p == NULL) {
				boot_class_path_p = MNEW(char, strlen(opt_arg) + strlen("0"));
				strcpy(boot_class_path_p, opt_arg);
			} else {
				len = strlen(boot_class_path_p);
				p = MNEW(char, strlen(opt_arg) + strlen(":") + len + strlen("0"));
				strcpy(p, opt_arg);
				strcat(p, ":");
				strcat(p, boot_class_path_p);
				MFREE(boot_class_path_p, char, len);
				boot_class_path_p = p;
			}
			break;

		case OPT_BOOTCLASSPATH_C:
			/* Use as Java core library, but prepend VM interface
			   classes. */

			// FIXME Make boot_class_path const char*.
			boot_class_path = (char*) _properties.get("sun.boot.class.path");

			len =
				strlen(CACAO_VM_ZIP) +
				strlen(":") +
				strlen(opt_arg) +
				strlen("0");

			p = MNEW(char, len);

			strcpy(p, CACAO_VM_ZIP);
			strcat(p, ":");
			strcat(p, opt_arg);

			_properties.put("sun.boot.class.path", p);
			_properties.put("java.boot.class.path", p);

			MFREE(boot_class_path, char, strlen(boot_class_path));
			break;

#if defined(ENABLE_JVMTI)
		case OPT_AGENTLIB:
			// Parse option argument.
			p = strchr(opt_arg, '=');
			if (p != NULL)
				*(p++) = '\0';

			_nativeagents.register_agent_library(opt_arg, p);
			break;

		case OPT_AGENTPATH:
			// Parse option argument.
			p = strchr(opt_arg, '=');
			if (p != NULL)
				*(p++) = '\0';

			_nativeagents.register_agent_path(opt_arg, p);
			break;

		case OPT_RUN:
			// Parse option argument.
			p = strchr(opt_arg, ':');
			if (p != NULL)
				*(p++) = '\0';

			_nativeagents.register_agent_library(opt_arg, p);
			break;
#endif

		case OPT_MX:
		case OPT_MS:
		case OPT_SS:
			{
				char c;
				int j;

				c = opt_arg[strlen(opt_arg) - 1];

				if ((c == 'k') || (c == 'K')) {
					j = atoi(opt_arg) * 1024;

				} else if ((c == 'm') || (c == 'M')) {
					j = atoi(opt_arg) * 1024 * 1024;

				} else
					j = atoi(opt_arg);

				if (opt == OPT_MX)
					opt_heapmaxsize = j;
				else if (opt == OPT_MS)
					opt_heapstartsize = j;
				else
					opt_stacksize = j;
			}
			break;

		case OPT_XCHECK_JNI:
			// HotSpot compatibility option.
			break;

		case OPT_VERBOSE1:
			opt_verbose = true;
			break;

		case OPT_VERBOSE:
			if (strcmp("class", opt_arg) == 0) {
				opt_verboseclass = true;
			}
			else if (strcmp("gc", opt_arg) == 0) {
				opt_verbosegc = true;
			}
			else if (strcmp("jni", opt_arg) == 0) {
				opt_verbosejni = true;
			}
#if !defined(NDEBUG)
			else if (strcmp("jit", opt_arg) == 0) {
				opt_verbose = true;
				loadverbose = true;
				initverbose = true;
				compileverbose = true;
			}
#endif
			else {
				printf("Unknown -verbose option: %s\n", opt_arg);
				usage();
			}
			break;

		case OPT_DEBUGCOLOR:
			opt_debugcolor = true;
			break;

#if defined(ENABLE_VERIFIER) && defined(TYPECHECK_VERBOSE)
		case OPT_VERBOSETC:
			opt_typecheckverbose = true;
			break;
#endif

		case OPT_VERSION:
			opt_version = true;
			opt_exit    = true;
			break;

		case OPT_FULLVERSION:
			fullversion();
			break;

		case OPT_SHOWVERSION:
			opt_version = true;
			break;

#if defined(ENABLE_VERIFIER)
		case OPT_XVERIFY_ALL:
			opt_verify = true;
			break;

		case OPT_NOVERIFY:
		case OPT_XVERIFY_NONE:
			opt_verify = false;
			break;
#endif

#if defined(ENABLE_STATISTICS)
		case OPT_TIME:
			opt_getcompilingtime = true;
			opt_getloadingtime = true;
			break;

		case OPT_STAT:
			opt_stat = true;
			break;
#endif

		case OPT_LOG:
			log_init(opt_arg);
			break;

		case OPT_CHECK:
			for (unsigned int i = 0; i < strlen(opt_arg); i++) {
				switch (opt_arg[i]) {
				case 'b':
					checkbounds = false;
					break;
				case 's':
					checksync = false;
					break;
				default:
					usage();
				}
			}
			break;

		case OPT_LOAD:
			opt_run = false;
			makeinitializations = false;
			break;

		case OPT_SHOW:       /* Display options */
			for (unsigned int i = 0; i < strlen(opt_arg); i++) {
				switch (opt_arg[i]) {
				case 'c':
					showconstantpool = true;
					break;

				case 'u':
					showutf = true;
					break;

				case 'm':
					showmethods = true;
					break;

				case 'i':
					opt_showintermediate = true;
					compileverbose = true;
					break;

#if defined(ENABLE_DISASSEMBLER)
				case 'a':
					opt_showdisassemble = true;
					compileverbose = true;
					break;
#endif

				case 'd':
					opt_showddatasegment = true;
					break;

				default:
					usage();
				}
			}
			break;

#if defined(ENABLE_LOOP)
		case OPT_OLOOP:
			opt_loops = true;
			break;
#endif

#if defined(ENABLE_IFCONV)
		case OPT_IFCONV:
			opt_ifconv = true;
			break;
#endif

#if defined(ENABLE_LSRA)
		case OPT_LSRA:
			opt_lsra = true;
			break;
#endif
#if  defined(ENABLE_SSA)
		case OPT_LSRA:
			opt_lsra = true;
			for (unsigned int i = 0; i < strlen(opt_arg); i++) {
				switch (opt_arg[i]) {
				case 'c':
					opt_ssa_cp = true;
					break;

				case 'd':
					opt_ssa_dce = true;
					break;

				case ':':
					break;

				default:
					usage();
				}
			}
			break;
#endif

		case OPT_HELP:
			usage();
			break;

		case OPT_X:
			Xusage();
			break;

		case OPT_XX:
			/* Already parsed. */
			break;

		case OPT_EA:
#if defined(ENABLE_ASSERTION)
			assertion_ea_da(opt_arg, true);
#endif
			break;

		case OPT_DA:
#if defined(ENABLE_ASSERTION)
			assertion_ea_da(opt_arg, false);
#endif
			break;

		case OPT_EA_NOARG:
#if defined(ENABLE_ASSERTION)
			assertion_user_enabled = true;
#endif
			break;

		case OPT_DA_NOARG:
#if defined(ENABLE_ASSERTION)
			assertion_user_enabled = false;
#endif
			break;

		case OPT_ESA:
#if defined(ENABLE_ASSERTION)
			assertion_system_enabled = true;
#endif
			break;

		case OPT_DSA:
#if defined(ENABLE_ASSERTION)
			assertion_system_enabled = false;
#endif
			break;

#if defined(ENABLE_PROFILING)
		case OPT_PROF_OPTION:
			/* use <= to get the last \0 too */

			for (unsigned int i = 0, j = 0; i <= strlen(opt_arg); i++) {
				if (opt_arg[i] == ',')
					opt_arg[i] = '\0';

				if (opt_arg[i] == '\0') {
					if (strcmp("bb", opt_arg + j) == 0)
						opt_prof_bb = true;

					else {
						fprintf(stderr, "Unknown option: -Xprof:%s\n", opt_arg + j);
						usage();
					}

					/* set k to next char */

					j = i + 1;
				}
			}
			/* fall through */

		case OPT_PROF:
			opt_prof = true;
			break;
#endif

		case OPT_JIT:
#if defined(ENABLE_JIT)
			opt_jit = true;
#else
			printf("-Xjit option not enabled.\n");
			exit(1);
#endif
			break;

		case OPT_INTRP:
#if defined(ENABLE_INTRP)
			opt_intrp = true;
#else
			printf("-Xint option not enabled.\n");
			exit(1);
#endif
			break;

#if defined(ENABLE_INTRP)
		case OPT_STATIC_SUPERS:
			opt_static_supers = atoi(opt_arg);
			break;

		case OPT_NO_DYNAMIC:
			opt_no_dynamic = true;
			break;

		case OPT_NO_REPLICATION:
			opt_no_replication = true;
			break;

		case OPT_NO_QUICKSUPER:
			opt_no_quicksuper = true;
			break;

		case OPT_TRACE:
			vm_debug = true;
			break;
#endif

#if defined(ENABLE_DEBUG_FILTER)
		case OPT_FILTER_VERBOSECALL_INCLUDE:
			opt_filter_verbosecall_include = opt_arg;
			break;

		case OPT_FILTER_VERBOSECALL_EXCLUDE:
			opt_filter_verbosecall_exclude = opt_arg;
			break;

		case OPT_FILTER_SHOW_METHOD:
			opt_filter_show_method = opt_arg;
			break;

#endif
		default:
			fprintf(stderr, "Unknown option: %s\n",
				   vm_args->options[opt_index++].optionString);
		}
	}

	// Print the preliminary run-time VM configuration after options
	// are parsed.
	if (opt_PrintConfig)
		print_run_time_config();

	/* initialize the garbage collector */

	gc_init(opt_heapmaxsize, opt_heapstartsize);

	/* AFTER: gc_init */

  	threads_preinit();
	lock_init();

	/* install architecture dependent signal handlers */

	if (!signal_init())
		os::abort("vm_create: signal_init failed");

#if defined(ENABLE_INTRP)
	/* Allocate main thread stack on the Java heap. */

	if (opt_intrp) {
		intrp_main_stack = GCMNEW(u1, opt_stacksize);
		MSET(intrp_main_stack, 0, u1, opt_stacksize);
	}
#endif

	/* AFTER: threads_preinit */

	JavaString::initialize();

	/* AFTER: threads_preinit */

	Utf8String::initialize();

	// Hook point before the VM is initialized.
	Hook::vm_preinit();

#if defined(ENABLE_JVMTI)
	// AFTER: utf8_init
	if (!_nativeagents.load_agents())
		os::abort("vm_create: load_agents failed");
#endif

	/* AFTER: thread_preinit */

	/* Add -Xbootclasspath/p if it exists */
	if (boot_class_path_p != NULL)
		_suckclasspath.add(boot_class_path_p);

	_suckclasspath.add_from_property("java.endorsed.dirs");

	/* Now we have all options handled and we can print the version
	   information.

	   AFTER: suck_add_from_property("java.endorsed.dirs"); */

	if (opt_version)
		version(opt_exit);

	/* AFTER: utf8_init */

	// FIXME Make boot_class_path const char*.
	boot_class_path = (char*) _properties.get("sun.boot.class.path");
	_suckclasspath.add(boot_class_path);

	/* initialize the classcache hashtable stuff: lock, hashtable
	   (must be done _after_ threads_preinit) */

	if (!classcache_init())
		os::abort("vm_create: classcache_init failed");

	/* Initialize the code memory management. */
	/* AFTER: threads_preinit */

	codememory_init();

	/* initialize the finalizer stuff (must be done _after_
	   threads_preinit) */

	if (!finalizer_init())
		os::abort("vm_create: finalizer_init failed");

	/* Initialize the JIT compiler. */

	jit_init();
	code_init();
	methodtree_init();

	/* AFTER: utf8_init, classcache_init */

	loader_preinit();
	linker_preinit();

	// AFTER: loader_preinit, linker_preinit
	Primitive::initialize_table();

	loader_init();
	jobjects_register_dyn_offsets();
	linker_init();

	// AFTER: loader_init, linker_init
	Primitive::post_initialize_table();
	method_init();

#if defined(ENABLE_JIT)
	trap_init();
#endif

	if (!builtin_init())
		os::abort("vm_create: builtin_init failed");

	/* Register the native methods implemented in the VM. */
	/* BEFORE: threads_init */

	nativevm_preinit();

#if defined(ENABLE_JNI)
	/* Initialize the JNI subsystem (must be done _before_
	   threads_init, as threads_init can call JNI methods
	   (e.g. NewGlobalRef). */

	if (!jni_init())
		os::abort("vm_create: jni_init failed");
#endif

#if defined(ENABLE_JNI) || defined(ENABLE_HANDLES)
	/* Initialize the local reference table for the main thread. */
	/* BEFORE: threads_init */

	if (!localref_table_init())
		os::abort("vm_create: localref_table_init failed");
#endif

	/* Iinitialize some important system classes. */
	/* BEFORE: threads_init */

	initialize_init();
  	threads_init();

	/* Initialize the native VM subsystem. */
	/* AFTER: threads_init (at least for SUN's classes) */

	if (!nativevm_init())
		os::abort("vm_create: nativevm_init failed");

#if defined(ENABLE_PROFILING)
	/* initialize profiling */

	if (!profile_init())
		os::abort("vm_create: profile_init failed");
#endif

	/* start the signal handler thread */

#if defined(__LINUX__)
	/* XXX Remove for exact-GC. */
	if (threads_pthreads_implementation_nptl)
#endif
		if (!signal_start_thread())
			os::abort("vm_create: signal_start_thread failed");

	/* finally, start the finalizer thread */

	if (!finalizer_start_thread())
		os::abort("vm_create: finalizer_start_thread failed");

#if !defined(NDEBUG)
	/* start the memory profiling thread */

	if (opt_ProfileMemoryUsage || opt_ProfileGCMemoryUsage)
		if (!memory_start_thread())
			os::abort("vm_create: memory_start_thread failed");
#endif

#ifdef ENABLE_THREADS
	// Start the recompilation thread (must be done before the
	// profiling thread).
	// FIXME Only works for one recompiler.
	_recompiler.start();
#endif

#if defined(ENABLE_PROFILING)
	/* start the profile sampling thread */

/* 	if (opt_prof) */
/* 		if (!profile_start_thread()) */
/* 			os::abort("vm_create: profile_start_thread failed"); */
#endif

	/* Increment the number of VMs. */

	vms++;

	// Initialization is done, VM is created.
	_created      = true;
	_initializing = false;

	// Set the VM inittime.
	_inittime = builtin_currenttimemillis();

	// Hook point after the VM is initialized.
	Hook::vm_init();

	// Print the run-time VM configuration after all stuff is set and
	// the VM is initialized.
	if (opt_PrintConfig)
		print_run_time_config();

	// Start runtime agents after the VM is created.
	if (!start_runtime_agents())
		os::abort("vm_create: start_runtime_agents failed");
}


/**
 * Print build-time (default) VM configuration.
 */
void VM::print_build_time_config(void)
{
	puts("CACAO " VERSION_FULL " configure/build options:");
	puts("");
	puts("  ./configure: " VERSION_CONFIGURE_ARGS "");
#if defined(__VERSION__)
	puts("  CC         : " VERSION_CC  " (" __VERSION__ ")");
	puts("  CXX        : " VERSION_CXX " (" __VERSION__ ")");
#else
	puts("  CC         : " VERSION_CC  "");
	puts("  CXX        : " VERSION_CXX "");
#endif
	puts("  CFLAGS     : " VERSION_CFLAGS   "");
	puts("  CXXFLAGS   : " VERSION_CXXFLAGS "");
	puts("  CPPFLAGS   : " VERSION_CPPFLAGS "");

	puts("");

	puts("Build-time (default) variables:\n");
	printf("  maximum heap size              : %d\n", HEAP_MAXSIZE);
	printf("  initial heap size              : %d\n", HEAP_STARTSIZE);
	printf("  stack size                     : %d\n", STACK_SIZE);

#if defined(ENABLE_JRE_LAYOUT)
	// When we're building with JRE-layout, the default paths are the
	// same as the runtime paths.
#else
# if defined(WITH_JAVA_RUNTIME_LIBRARY_GNU_CLASSPATH)
	puts("  gnu.classpath.boot.library.path: " JAVA_RUNTIME_LIBRARY_LIBDIR);
	puts("  java.boot.class.path           : " CACAO_VM_ZIP ":" JAVA_RUNTIME_LIBRARY_CLASSES);
# elif defined(WITH_JAVA_RUNTIME_LIBRARY_OPENJDK)
	puts("  sun.boot.library.path          : " JAVA_RUNTIME_LIBRARY_LIBDIR);
	puts("  java.boot.class.path           : " JAVA_RUNTIME_LIBRARY_CLASSES);
# endif
#endif

	puts("");
}


/**
 * Print run-time VM configuration.
 */
void VM::print_run_time_config()
{
	puts("Run-time variables:\n");
	printf("  maximum heap size              : %d\n", opt_heapmaxsize);
	printf("  initial heap size              : %d\n", opt_heapstartsize);
	printf("  stack size                     : %d\n", opt_stacksize);

#if defined(WITH_JAVA_RUNTIME_LIBRARY_GNU_CLASSPATH)
	printf("  gnu.classpath.boot.library.path: %s\n", _properties.get("gnu.classpath.boot.library.path"));
#elif defined(WITH_JAVA_RUNTIME_LIBRARY_OPENJDK)
	printf("  sun.boot.library.path          : %s\n", _properties.get("sun.boot.library.path"));
#endif

	printf("  java.boot.class.path           : %s\n", _properties.get("java.boot.class.path"));
	printf("  java.class.path                : %s\n", _properties.get("java.class.path"));

	puts("");
}


/**
 * Start runtime agents which are provided by the JRE but need to be
 * started explicitly by the VM.
 */
bool VM::start_runtime_agents()
{
#if defined(WITH_JAVA_RUNTIME_LIBRARY_GNU_CLASSPATH)

	// Nothing to do.

#elif defined(WITH_JAVA_RUNTIME_LIBRARY_OPENJDK)

	// Check whether the management agent should be loaded.
	if ((_properties.get("com.sun.management.jmxremote") != NULL) ||
		(_properties.get("com.sun.management.snmp") != NULL))
	{

		// Load the management agent class.
		classinfo* class_sun_management_Agent;
		if (!(class_sun_management_Agent = load_class_from_sysloader(Utf8String::from_utf8("sun/management/Agent"))))
			return false;

		// Link the management agent class.
		if (!link_class(class_sun_management_Agent))
			return false;

		// Actually start the management agent.
		methodinfo* m = class_resolveclassmethod(class_sun_management_Agent,
												 Utf8String::from_utf8("startAgent"),
												 utf8::void__void,
												 class_java_lang_Object,
												 false);

		if (m == NULL)
			return false;

		(void) vm_call_method(m, NULL);

		if (exceptions_get_exception() != NULL)
			return false;
	}

#elif defined(WITH_JAVA_RUNTIME_LIBRARY_CLDC1_1)

	// Nothing to do.

#else
# error unknown classpath configuration
#endif

	return true;
}

static void write_logfiles();

/* vm_run **********************************************************************

   Runs the main-method of the passed class.

*******************************************************************************/

void vm_run(JavaVM *vm, JavaVMInitArgs *vm_args)
{
	methodinfo* m;
	int         status;

#if !defined(NDEBUG)
	if (opt_CompileAll) {
		vm_compile_all();
		/* write logfiles */
		write_logfiles();
		return;
	}
#endif

	/* Get the main class or jar file argument. */

	char* mainname = NULL;

	if (opt_index < vm_args->nOptions) {
		/* Get main-class argument. */

		mainname = vm_args->options[opt_index].optionString;

		/* If the main class argument is a jar file, put it into the
		   classpath. */

		if (opt_jar == true) {
			char* p = MNEW(char, strlen(mainname) + strlen("0"));

			strcpy(p, mainname);

#if defined(ENABLE_JAVASE)
			VM::get_current()->get_properties().put("java.class.path", p);
#endif
		}
		else {
			/* Replace dots with slashes in the class name. */

			for (unsigned int i = 0; i < strlen(mainname); i++)
				if (mainname[i] == '.')
					mainname[i] = '/';
		}

		/* Move index to first argument. */

		opt_index++;
	}

	/* Do we have a main-class argument? */

	if (mainname == NULL)
		usage();

#if !defined(NDEBUG)
	if (opt_CompileMethod != NULL) {
		vm_compile_method(mainname);
		/* write logfiles */
		write_logfiles();
		return;
	}
#endif

	/* Build argument array. */

	int32_t oalength = vm_args->nOptions - opt_index;

	ObjectArray oa(oalength, class_java_lang_String);

	if (oa.is_null())
		vm_exit(1);

	for (int i = 0; i < oalength; i++) {
		char* option = vm_args->options[opt_index + i].optionString;

		java_handle_t* s = JavaString::from_utf8(option);

		oa.set_element(i, s);
	}

	/* set return value to OK */

	status = 0;

	if (opt_jar == true) {
		/* open jar file with java.util.jar.JarFile */

		mainname = vm_get_mainclass_from_jar(mainname);

		if (mainname == NULL)
			vm_exit(1);
	}

	/* load the main class */

	Utf8String mainutf = Utf8String::from_utf8(mainname);

#if defined(ENABLE_JAVAME_CLDC1_1)
	classinfo* mainclass = load_class_bootstrap(mainutf);
#else
	classinfo* mainclass = load_class_from_sysloader(mainutf);
#endif

	/* error loading class */

	java_handle_t* e = exceptions_get_and_clear_exception();

	if ((e != NULL) || (mainclass == NULL)) {
		exceptions_throw_noclassdeffounderror_cause(e);
		exceptions_print_stacktrace();
		vm_exit(1);
	}

	if (!link_class(mainclass)) {
		exceptions_print_stacktrace();
		vm_exit(1);
	}

	/* find the `main' method of the main class */

	m = class_resolveclassmethod(mainclass,
								 Utf8String::from_utf8("main"),
								 Utf8String::from_utf8("([Ljava/lang/String;)V"),
								 class_java_lang_Object,
								 false);

	if (exceptions_get_exception()) {
		exceptions_print_stacktrace();
		vm_exit(1);
	}

	/* there is no main method or it isn't static */

	if ((m == NULL) || !(m->flags & ACC_STATIC)) {
		exceptions_clear_exception();
		exceptions_throw_nosuchmethoderror(mainclass,
										   Utf8String::from_utf8("main"),
										   Utf8String::from_utf8("([Ljava/lang/String;)V"));

		exceptions_print_stacktrace();
		vm_exit(1);
	}

#ifdef TYPEINFO_DEBUG_TEST
	/* test the typeinfo system */
	typeinfo_test();
#endif

	/* start the main thread */

	(void) vm_call_method(m, NULL, oa.get_handle());

	/* exception occurred? */

	if (exceptions_get_exception()) {
		exceptions_print_stacktrace();
		status = 1;
	}

    /* Detach the main thread so that it appears to have ended when
	   the application's main method exits. */

	if (!thread_detach_current_thread())
		os::abort("vm_run: Could not detach main thread.");

	/* write logfiles */
	write_logfiles();

	/* Destroy the JavaVM. */

	(void) vm_destroy(vm);

	/* And exit. */

	vm_exit(status);
}


/* vm_destroy ******************************************************************

   Unloads a Java VM and reclaims its resources.

*******************************************************************************/

int vm_destroy(JavaVM *vm)
{
	/* Create a a trivial new Java waiter thread called
	   "DestroyJavaVM". */

	JavaVMAttachArgs args;

	args.name  = (char*) "DestroyJavaVM";
	args.group = NULL;

	if (!thread_attach_current_thread(&args, false))
		return 1;

	/* Wait until we are the last non-daemon thread. */

	threads_join_all_threads();

	// Hook point before the VM is actually destroyed.
	Hook::vm_shutdown();

	/* VM is gone. */

// 	_created = false;

	/* Everything is ok. */

	return 0;
}


/* vm_exit *********************************************************************

   Calls java.lang.System.exit(I)V to exit the JavaVM correctly.

*******************************************************************************/

void vm_exit(s4 status)
{
	methodinfo *m;

	/* signal that we are exiting */

// 	_exiting = true;

	assert(class_java_lang_System);
	assert(class_java_lang_System->state & CLASS_LOADED);

#if defined(ENABLE_JVMTI)
	if (jvmti || (dbgcom!=NULL)) {
		jvmti_set_phase(JVMTI_PHASE_DEAD);
		if (jvmti) jvmti_agentunload();
	}
#endif

	if (!link_class(class_java_lang_System)) {
		exceptions_print_stacktrace();
		exit(1);
	}

	/* call java.lang.System.exit(I)V */

	m = class_resolveclassmethod(class_java_lang_System,
								 Utf8String::from_utf8("exit"),
								 utf8::int__void,
								 class_java_lang_Object,
								 true);

	if (m == NULL) {
		exceptions_print_stacktrace();
		exit(1);
	}

	/* call the exit function with passed exit status */

	(void) vm_call_method(m, NULL, status);

	/* If we had an exception, just ignore the exception and exit with
	   the proper code. */

	vm_shutdown(status);
}


/* vm_shutdown *****************************************************************

   Terminates the system immediately without freeing memory explicitly
   (to be used only for abnormal termination).

*******************************************************************************/

void vm_shutdown(s4 status)
{
	if (opt_verbose
#if defined(ENABLE_STATISTICS)
		|| opt_getcompilingtime || opt_stat
#endif
	   )
	{
		log_text("CACAO terminated by shutdown");
		dolog("Exit status: %d\n", (s4) status);

	}

#if defined(ENABLE_JVMTI)
	/* terminate cacaodbgserver */
	if (dbgcom!=NULL) {
		mutex_lock(&dbgcomlock);
		dbgcom->running=1;
		mutex_unlock(&dbgcomlock);
		jvmti_cacaodbgserver_quit();
	}
#endif

#if defined(ENABLE_THREADS)
	finalizer_join_thread();
#endif

	exit(status);
}

#include "toolbox/OStream.hpp"

/* vm_exit_handler *************************************************************

   The exit_handler function is called upon program termination.

   ATTENTION: Don't free system resources here! Some threads may still
   be running as this is called from VMRuntime.exit(). The OS does the
   cleanup for us.

*******************************************************************************/

void vm_exit_handler(void)
{
#if !defined(NDEBUG)
	if (showmethods)
		class_showmethods(mainclass);

	if (showconstantpool)
		class_showconstantpool(mainclass);

# if defined(ENABLE_PROFILING)
	if (opt_prof)
		profile_printstats();
# endif
#endif /* !defined(NDEBUG) */

#if defined(ENABLE_CYCLES_STATS)
	builtin_print_cycles_stats(log_get_logfile());
	stacktrace_print_cycles_stats(log_get_logfile());
#endif

	if (opt_verbose
#if defined(ENABLE_STATISTICS)
		|| opt_getcompilingtime || opt_stat
#endif
	   )
	{
		log_text("CACAO terminated");

#if 0 && defined(ENABLE_STATISTICS)
		if (opt_stat) {
#ifdef TYPECHECK_STATISTICS
// XXX TYPECHECK_STATISTICS is currently not usable
			typecheck_print_statistics(get_logfile());
#endif
		}

#endif /* defined(ENABLE_STATISTICS) */
	}
	/* vm_print_profile(stderr);*/
}

static void write_logfiles() {
#if defined(ENABLE_RT_TIMING)
	if (!opt_RtTimingLogfile) {
		FILE *file = fopen("rt-timing.log", "w");
		if (file == NULL)
			/* fallback to stdout */
			file = stdout;
		opt_RtTimingLogfile = file;
	}
	{
		assert(opt_RtTimingLogfile);
		cacao::OStream OS(opt_RtTimingLogfile);
		OS << "\nreal-time measurment:\n" << cacao::nl;
		cacao::RTGroup::root().print(OS);
	}
#endif

#if defined(ENABLE_STATISTICS)
	if (!opt_StatisticsLogfile) {
		FILE *file = fopen("statistics.log", "w");
		if (file == NULL)
			/* fallback to stdout */
			file = stdout;
		opt_StatisticsLogfile = file;
	}
	{
		assert(opt_StatisticsLogfile);
		cacao::OStream OS(opt_StatisticsLogfile);
		cacao::StatGroup::root().print(OS);
	}
#endif

}

/* vm_abort_disassemble ********************************************************

   Prints an error message, disassemble the given code range (if
   enabled) and aborts the VM.

   IN:
       pc.......PC to disassemble
	   count....number of instructions to disassemble

*******************************************************************************/

void vm_abort_disassemble(void *pc, int count, const char *text, ...)
{
	va_list ap;
#if defined(ENABLE_DISASSEMBLER)
	int     i;
#endif

	/* Print debug message. */

	log_start();

	va_start(ap, text);
	log_vprint(text, ap);
	va_end(ap);

	log_finish();

	/* Print the PC. */

	log_println("PC=0x%0" PRINTF_INTPTR_NUM_HEXDIGITS PRIxPTR, (intptr_t) pc);

#if defined(ENABLE_DISASSEMBLER)
	log_println("machine instructions at PC:");

	/* Disassemble the given number of instructions. */

	for (i = 0; i < count; i++)
		// FIXME disassinstr should use void*.
		pc = disassinstr((u1*) pc);
#endif

	os::abort("Aborting...");
}


/* vm_get_mainclass_from_jar ***************************************************

   Gets the name of the main class from a JAR's manifest file.

*******************************************************************************/

static char *vm_get_mainclass_from_jar(char *mainname)
{
	classinfo     *c;
	java_handle_t *o;
	methodinfo    *m;
	java_handle_t *s;

#if defined(ENABLE_JAVAME_CLDC1_1)
	c = load_class_bootstrap(Utf8String::from_utf8("java/util/jar/JarFile"));
#else
	c = load_class_from_sysloader(Utf8String::from_utf8("java/util/jar/JarFile"));
#endif

	if (c == NULL) {
		exceptions_print_stacktrace();
		return NULL;
	}

	/* create JarFile object */

	o = builtin_new(c);

	if (o == NULL) {
		exceptions_print_stacktrace();
		return NULL;
	}

	m = class_resolveclassmethod(c,
								 utf8::init,
								 utf8::java_lang_String__void,
								 class_java_lang_Object,
								 true);

	if (m == NULL) {
		exceptions_print_stacktrace();
		return NULL;
	}

	s = JavaString::from_utf8(mainname);

	(void) vm_call_method(m, o, s);

	if (exceptions_get_exception()) {
		exceptions_print_stacktrace();
		return NULL;
	}

	/* get manifest object */

	m = class_resolveclassmethod(c,
								 Utf8String::from_utf8("getManifest"),
								 Utf8String::from_utf8("()Ljava/util/jar/Manifest;"),
								 class_java_lang_Object,
								 true);

	if (m == NULL) {
		exceptions_print_stacktrace();
		return NULL;
	}

	o = vm_call_method(m, o);

	if (o == NULL) {
		fprintf(stderr, "Could not get manifest from %s (invalid or corrupt jarfile?)\n", mainname);
		return NULL;
	}


	/* get Main Attributes */

	LLNI_class_get(o, c);

	m = class_resolveclassmethod(c,
								 Utf8String::from_utf8("getMainAttributes"),
								 Utf8String::from_utf8("()Ljava/util/jar/Attributes;"),
								 class_java_lang_Object,
								 true);

	if (m == NULL) {
		exceptions_print_stacktrace();
		return NULL;
	}

	o = vm_call_method(m, o);

	if (o == NULL) {
		fprintf(stderr, "Could not get main attributes from %s (invalid or corrupt jarfile?)\n", mainname);
		return NULL;
	}


	/* get property Main-Class */

	LLNI_class_get(o, c);

	m = class_resolveclassmethod(c,
								 Utf8String::from_utf8("getValue"),
								 Utf8String::from_utf8("(Ljava/lang/String;)Ljava/lang/String;"),
								 class_java_lang_Object,
								 true);

	if (m == NULL) {
		exceptions_print_stacktrace();
		return NULL;
	}

	s = JavaString::from_utf8("Main-Class");

	o = vm_call_method(m, o, s);

	if (o == NULL) {
		fprintf(stderr, "Failed to load Main-Class manifest attribute from\n");
		fprintf(stderr, "%s\n", mainname);
		return NULL;
	}

	return JavaString(o).to_chars();
}


/* vm_compile_all **************************************************************

   Compile all methods found in the bootclasspath.

*******************************************************************************/

#if !defined(NDEBUG)
static void vm_compile_all(void)
{
	classinfo              *c;
	methodinfo             *m;
	u4                      slot;
	classcache_name_entry  *nmen;
	classcache_class_entry *clsen;
	s4                      i;

	/* create all classes found in the bootclasspath */
	/* XXX currently only works with zip/jar's */

	loader_load_all_classes();

	/* link all classes */

	for (slot = 0; slot < hashtable_classcache.size; slot++) {
		nmen = (classcache_name_entry *) hashtable_classcache.ptr[slot];

		for (; nmen; nmen = nmen->hashlink) {
			/* iterate over all class entries */

			for (clsen = nmen->classes; clsen; clsen = clsen->next) {
				c = clsen->classobj;

				if (c == NULL)
					continue;

				if (!(c->state & CLASS_LINKED)) {
					if (!link_class(c)) {
						fprintf(stderr, "Error linking: ");
						utf_fprint_printable_ascii_classname(stderr, c->name);
						fprintf(stderr, "\n");

						/* print out exception and cause */

						exceptions_print_current_exception();

						/* goto next class */

						continue;
					}
				}

				/* compile all class methods */

				for (i = 0; i < c->methodscount; i++) {
					m = &(c->methods[i]);

					if (m->jcode != NULL) {
						if (!jit_compile(m)) {
							fprintf(stderr, "Error compiling: ");
							utf_fprint_printable_ascii_classname(stderr, c->name);
							fprintf(stderr, ".");
							utf_fprint_printable_ascii(stderr, m->name);
							utf_fprint_printable_ascii(stderr, m->descriptor);
							fprintf(stderr, "\n");

							/* print out exception and cause */

							exceptions_print_current_exception();
						}
					}
				}
			}
		}
	}
}
#endif /* !defined(NDEBUG) */


/* vm_compile_method ***********************************************************

   Compile a specific method.

*******************************************************************************/

#if !defined(NDEBUG)
static void vm_compile_method(char* mainname)
{
	methodinfo *m;

	/* create, load and link the main class */

	mainclass = load_class_bootstrap(Utf8String::from_utf8(mainname));

	if (mainclass == NULL)
		exceptions_print_stacktrace();

	if (!link_class(mainclass))
		exceptions_print_stacktrace();

	if (opt_CompileSignature != NULL) {
		m = class_resolveclassmethod(mainclass,
									 Utf8String::from_utf8(opt_CompileMethod),
									 Utf8String::from_utf8(opt_CompileSignature),
									 mainclass,
									 false);
	}
	else {
		m = class_resolveclassmethod(mainclass,
									 Utf8String::from_utf8(opt_CompileMethod),
									 NULL,
									 mainclass,
									 false);
	}

	if (m == NULL)
		os::abort("vm_compile_method: java.lang.NoSuchMethodException: %s.%s",
				 opt_CompileMethod, opt_CompileSignature ? opt_CompileSignature : "");

	jit_compile(m);
}
#endif /* !defined(NDEBUG) */


/* vm_call_array ***************************************************************

   Calls a Java method with a variable number of arguments, passed via
   an argument array.

   ATTENTION: This function has to be used outside the nativeworld.

*******************************************************************************/

#define VM_CALL_ARRAY(name, type)                                 \
static type vm_call##name##_array(methodinfo *m, uint64_t *array) \
{                                                                 \
	methoddesc *md;                                               \
	void       *pv;                                               \
	type        value;                                            \
                                                                  \
	assert(m->code != NULL);                                      \
                                                                  \
	md = m->parseddesc;                                           \
	pv = m->code->entrypoint;                                     \
                                                                  \
	STATISTICS(count_calls_native_to_java++);                  \
                                                                  \
	value = asm_vm_call_method##name(pv, array, md->memuse);      \
                                                                  \
	return value;                                                 \
}

static java_handle_t *vm_call_array(methodinfo *m, uint64_t *array)
{
	methoddesc    *md;
	void          *pv;
	java_object_t *o;

	assert(m->code != NULL);

	md = m->parseddesc;
	pv = m->code->entrypoint;

	STATISTICS(count_calls_native_to_java++);

	o = asm_vm_call_method(pv, array, md->memuse);

	if (md->returntype.type == TYPE_VOID)
		o = NULL;

	return LLNI_WRAP(o);
}

VM_CALL_ARRAY(_int,    int32_t)
VM_CALL_ARRAY(_long,   int64_t)
VM_CALL_ARRAY(_float,  float)
VM_CALL_ARRAY(_double, double)


/* vm_call_method **************************************************************

   Calls a Java method with a variable number of arguments.

*******************************************************************************/

#define VM_CALL_METHOD(name, type)                                  \
type vm_call_##name(methodinfo *m, java_handle_t *o, ...)     \
{                                                                   \
	va_list ap;                                                     \
	type    value;                                                  \
                                                                    \
	va_start(ap, o);                                                \
	value = vm_call_##name##_valist(m, o, ap);                \
	va_end(ap);                                                     \
                                                                    \
	return value;                                                   \
}

VM_CALL_METHOD(method,        java_handle_t *)
VM_CALL_METHOD(method_int,    int32_t)
VM_CALL_METHOD(method_long,   int64_t)
VM_CALL_METHOD(method_float,  float)
VM_CALL_METHOD(method_double, double)


/* vm_call_method_valist *******************************************************

   Calls a Java method with a variable number of arguments, passed via
   a va_list.

*******************************************************************************/

#define VM_CALL_METHOD_VALIST(name, type)                               \
type vm_call_method##name##valist(methodinfo *m, java_handle_t *o,      \
								  va_list ap)                           \
{                                                                       \
	uint64_t *array;                                                    \
	type      value;                                                    \
                                                                        \
	if (m->code == NULL)                                                \
		if (!jit_compile(m))                                            \
			return 0;                                                   \
                                                                        \
	THREAD_NATIVEWORLD_EXIT;                                            \
																		\
	DumpMemoryArea dma;													\
                                                                        \
	array = argument_vmarray_from_valist(m, o, ap);                     \
	value = vm_call##name##array(m, array);                             \
                                                                        \
	THREAD_NATIVEWORLD_ENTER;                                           \
                                                                        \
	return value;                                                       \
}

VM_CALL_METHOD_VALIST(_,        java_handle_t *)
VM_CALL_METHOD_VALIST(_int_,    int32_t)
VM_CALL_METHOD_VALIST(_long_,   int64_t)
VM_CALL_METHOD_VALIST(_float_,  float)
VM_CALL_METHOD_VALIST(_double_, double)


/* vm_call_method_jvalue *******************************************************

   Calls a Java method with a variable number of arguments, passed via
   a jvalue array.

*******************************************************************************/

#define VM_CALL_METHOD_JVALUE(name, type)                               \
type vm_call_method##name##jvalue(methodinfo *m, java_handle_t *o,      \
						           const jvalue *args)                  \
{                                                                       \
	uint64_t *array;                                                    \
	type      value;                                                    \
                                                                        \
	if (m->code == NULL)                                                \
		if (!jit_compile(m))                                            \
			return 0;                                                   \
                                                                        \
	THREAD_NATIVEWORLD_EXIT;                                            \
																		\
	DumpMemoryArea dma;													\
                                                                        \
	array = argument_vmarray_from_jvalue(m, o, args);                   \
	value = vm_call##name##array(m, array);                             \
                                                                        \
	THREAD_NATIVEWORLD_ENTER;                                           \
                                                                        \
	return value;                                                       \
}

VM_CALL_METHOD_JVALUE(_,        java_handle_t *)
VM_CALL_METHOD_JVALUE(_int_,    int32_t)
VM_CALL_METHOD_JVALUE(_long_,   int64_t)
VM_CALL_METHOD_JVALUE(_float_,  float)
VM_CALL_METHOD_JVALUE(_double_, double)


/* vm_call_method_objectarray **************************************************

   Calls a Java method with a variable number if arguments, passed via
   an objectarray of boxed values. Returns a boxed value.

*******************************************************************************/

java_handle_t *vm_call_method_objectarray(methodinfo *m, java_handle_t *o,
										  java_handle_objectarray_t *params)
{
	uint64_t      *array;
	java_handle_t *xptr;
	java_handle_t *ro;
	imm_union      value;

	/* Prevent compiler warnings. */

	ro = NULL;

	/* compile methods which are not yet compiled */

	if (m->code == NULL)
		if (!jit_compile(m))
			return NULL;

	/* leave the nativeworld */

	THREAD_NATIVEWORLD_EXIT;

	// Create new dump memory area.
	DumpMemoryArea dma;

	/* Fill the argument array from a object-array. */

	array = argument_vmarray_from_objectarray(m, o, params);

	if (array == NULL) {
		/* enter the nativeworld again */

		THREAD_NATIVEWORLD_ENTER;

		exceptions_throw_illegalargumentexception();

		return NULL;
	}

	switch (m->parseddesc->returntype.primitivetype) {
	case PRIMITIVETYPE_VOID:
		value.a = vm_call_array(m, array);
		break;

	case PRIMITIVETYPE_BOOLEAN:
	case PRIMITIVETYPE_BYTE:
	case PRIMITIVETYPE_CHAR:
	case PRIMITIVETYPE_SHORT:
	case PRIMITIVETYPE_INT:
		value.i = vm_call_int_array(m, array);
		break;

	case PRIMITIVETYPE_LONG:
		value.l = vm_call_long_array(m, array);
		break;

	case PRIMITIVETYPE_FLOAT:
		value.f = vm_call_float_array(m, array);
		break;

	case PRIMITIVETYPE_DOUBLE:
		value.d = vm_call_double_array(m, array);
		break;

	case TYPE_ADR:
		ro = vm_call_array(m, array);
		break;

	default:
		os::abort("vm_call_method_objectarray: invalid return type %d", m->parseddesc->returntype.primitivetype);
	}

	/* enter the nativeworld again */

	THREAD_NATIVEWORLD_ENTER;

	/* box the return value if necesarry */

	if (m->parseddesc->returntype.primitivetype != (PrimitiveType) TYPE_ADR)
		ro = Primitive::box(m->parseddesc->returntype.primitivetype, value);

	/* check for an exception */

	xptr = exceptions_get_exception();

	if (xptr != NULL) {
		/* clear exception pointer, we are calling JIT code again */

		exceptions_clear_exception();

		exceptions_throw_invocationtargetexception(xptr);
	}

	return ro;
}


/* Legacy C interface *********************************************************/

extern "C" {

JNIEnv* VM_get_jnienv()      { return VM::get_current()->get_jnienv(); }

void vm_abort(const char* text, ...)
{
	va_list ap;

	log_println("vm_abort: WARNING, port me to C++ and use os::abort() instead.");

	// Print the log message.
	log_start();

	va_start(ap, text);
	log_vprint(text, ap);
	va_end(ap);

	log_finish();

	// Print a backtrace.
	os::print_backtrace();

	// Now abort the VM.
	os::abort();
}

}


/*
 * These are local overrides for various environment variables in Emacs.
 * Please do not remove this and leave it at the end of the file, where
 * Emacs will automagically detect them.
 * ---------------------------------------------------------------------
 * Local variables:
 * mode: c++
 * indent-tabs-mode: t
 * c-basic-offset: 4
 * tab-width: 4
 * End:
 * vim:noexpandtab:sw=4:ts=4:
 */
