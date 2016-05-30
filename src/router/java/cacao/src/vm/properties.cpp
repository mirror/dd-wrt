/* src/vm/properties.cpp - handling commandline properties

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

#include <errno.h>
#include <stdlib.h>

#include <string.h>
#include <time.h>
#include <unistd.h>

#include "mm/memory.hpp"

#include "native/llni.hpp"

#include "vm/class.hpp"
#include "vm/global.hpp"
#include "vm/options.hpp"
#include "vm/os.hpp"
#include "vm/properties.hpp"
#include "vm/string.hpp"
#include "vm/vm.hpp"

#ifdef HAVE_LOCALE_H
#include <locale.h>
#endif

#include "toolbox/logging.hpp"

struct methodinfo;

#define DEBUG_NAME "properties"

/**
 * Constructor fills the properties list with default values.
 */
Properties::Properties()
{
	int             len;
	char           *p;

	char           *boot_class_path;

#if defined(ENABLE_JAVASE)

# if defined(WITH_JAVA_RUNTIME_LIBRARY_GNU_CLASSPATH)
	struct utsname *utsnamebuf;
# endif
#endif

#if defined(ENABLE_JRE_LAYOUT)
	/* SUN also uses a buffer of 4096-bytes (strace is your friend). */

	p = MNEW(char, 4096);

	if (os::readlink("/proc/self/exe", p, 4095) == -1)
		os::abort_errno("readlink failed");

	/* We have a path like:

	   /path/to/executable/bin/java

	   or
	   
	   /path/to/executeable/jre/bin/java

	   Now let's strip two levels. */

	p = os::dirname(p);
	p = os::dirname(p);

# if defined(WITH_JAVA_RUNTIME_LIBRARY_GNU_CLASSPATH)

	/* Set java.home. */

	char* java_home = strdup(p);

	/* Set the path to Java core native libraries. */

	len = strlen(java_home) + strlen("/lib/classpath") + strlen("0");

	char* boot_library_path = MNEW(char, len);

	strcpy(boot_library_path, java_home);
	strcat(boot_library_path, "/lib/classpath");

# elif defined(WITH_JAVA_RUNTIME_LIBRARY_OPENJDK)

	/* Find correct java.home.  We check if there is a JRE
	   co-located. */

	/* NOTE: We use the server VM here as it should be available on
	   all architectures. */

	len =
		strlen(p) +
		strlen("/jre/lib/" JAVA_ARCH "/server/libjvm.so") +
		strlen("0");

	char* java_home = MNEW(char, len);

	strcpy(java_home, p);
	strcat(java_home, "/jre/lib/" JAVA_ARCH "/server/libjvm.so");

	// Check if that libjvm.so exists.
	if (os::access(java_home, F_OK) == 0) {
		// Yes, we add /jre to java.home.
		strcpy(java_home, p);
		strcat(java_home, "/jre");
	}
	else {
		// No, java.home is parent directory.
		strcpy(java_home, p);
	}

	/* Set the path to Java core native libraries. */

	len = strlen(java_home) + strlen("/lib/" JAVA_ARCH) + strlen("0");

	char* boot_library_path = MNEW(char, len);

	strcpy(boot_library_path, java_home);
	strcat(boot_library_path, "/lib/" JAVA_ARCH);

# else
#  error unknown classpath configuration
# endif

	/* Free path. */

	MFREE(p, char, len);

#else
	const char* java_home = CACAO_PREFIX;

# if defined(WITH_JAVA_RUNTIME_LIBRARY_GNU_CLASSPATH)

	const char* boot_library_path = JAVA_RUNTIME_LIBRARY_LIBDIR"/classpath";

# elif defined(WITH_JAVA_RUNTIME_LIBRARY_OPENJDK)

	const char* boot_library_path = JAVA_RUNTIME_LIBRARY_LIBDIR;

# elif defined(WITH_JAVA_RUNTIME_LIBRARY_CLDC1_1)

	// No boot_library_path required.

# else
#  error unknown classpath configuration
# endif
#endif

	put("java.home", java_home);

	/* Set the bootclasspath. */

	p = os::getenv("BOOTCLASSPATH");

	if (p != NULL) {
		boot_class_path = MNEW(char, strlen(p) + strlen("0"));
		strcpy(boot_class_path, p);
	}
	else {
#if defined(ENABLE_JRE_LAYOUT)
# if defined(WITH_JAVA_RUNTIME_LIBRARY_GNU_CLASSPATH)

		len =
			strlen(java_home) + strlen("/share/cacao/vm.zip:") +
			strlen(java_home) + strlen("/share/classpath/glibj.zip") +
			strlen("0");

		boot_class_path = MNEW(char, len);

		strcpy(boot_class_path, java_home);
		strcat(boot_class_path, "/share/cacao/vm.zip");
		strcat(boot_class_path, ":");
		strcat(boot_class_path, java_home);
		strcat(boot_class_path, "/share/classpath/glibj.zip");

# elif defined(WITH_JAVA_RUNTIME_LIBRARY_OPENJDK)

		/* This is the bootclasspath taken from HotSpot (see
		   hotspot/src/share/vm/runtime/os.cpp
		   (os::set_boot_path)). */

		len =
			strlen(java_home) + strlen("/lib/resources.jar:") +
			strlen(java_home) + strlen("/lib/rt.jar:") +
			strlen(java_home) + strlen("/lib/sunrsasign.jar:") +
			strlen(java_home) + strlen("/lib/jsse.jar:") +
			strlen(java_home) + strlen("/lib/jce.jar:") +
			strlen(java_home) + strlen("/lib/charsets.jar:") +
			strlen(java_home) + strlen("/classes") +
			strlen("0");

		boot_class_path = MNEW(char, len);

		strcpy(boot_class_path, java_home);
		strcat(boot_class_path, "/lib/resources.jar:");
		strcat(boot_class_path, java_home);
		strcat(boot_class_path, "/lib/rt.jar:");
		strcat(boot_class_path, java_home);
		strcat(boot_class_path, "/lib/sunrsasign.jar:");
		strcat(boot_class_path, java_home);
		strcat(boot_class_path, "/lib/jsse.jar:");
		strcat(boot_class_path, java_home);
		strcat(boot_class_path, "/lib/jce.jar:");
		strcat(boot_class_path, java_home);
		strcat(boot_class_path, "/lib/charsets.jar:");
		strcat(boot_class_path, java_home);
		strcat(boot_class_path, "/classes");

# else
#  error unknown classpath configuration
# endif
#else
# if defined(WITH_JAVA_RUNTIME_LIBRARY_GNU_CLASSPATH)

		len =
			strlen(CACAO_VM_ZIP) +
			strlen(":") +
			strlen(JAVA_RUNTIME_LIBRARY_CLASSES) +
			strlen("0");

		boot_class_path = MNEW(char, len);

		strcpy(boot_class_path, CACAO_VM_ZIP);
		strcat(boot_class_path, ":");
		strcat(boot_class_path, JAVA_RUNTIME_LIBRARY_CLASSES);

# elif defined(WITH_JAVA_RUNTIME_LIBRARY_OPENJDK)

		/* This is the bootclasspath taken from HotSpot (see
		   hotspot/src/share/vm/runtime/os.cpp
		   (os::set_boot_path)). */

		len =
			strlen(JAVA_RUNTIME_LIBRARY_PREFIX"/lib/resources.jar:") +
			strlen(JAVA_RUNTIME_LIBRARY_PREFIX"/lib/rt.jar:") +
			strlen(JAVA_RUNTIME_LIBRARY_PREFIX"/lib/sunrsasign.jar:") +
			strlen(JAVA_RUNTIME_LIBRARY_PREFIX"/lib/jsse.jar:") +
			strlen(JAVA_RUNTIME_LIBRARY_PREFIX"/lib/jce.jar:") +
			strlen(JAVA_RUNTIME_LIBRARY_PREFIX"/lib/charsets.jar:") +
			strlen(JAVA_RUNTIME_LIBRARY_PREFIX"/classes") +
			strlen("0");

		boot_class_path = MNEW(char, len);

		strcpy(boot_class_path, JAVA_RUNTIME_LIBRARY_PREFIX"/lib/resources.jar:");
		strcat(boot_class_path, JAVA_RUNTIME_LIBRARY_PREFIX"/lib/rt.jar:");
		strcat(boot_class_path, JAVA_RUNTIME_LIBRARY_PREFIX"/lib/sunrsasign.jar:");
		strcat(boot_class_path, JAVA_RUNTIME_LIBRARY_PREFIX"/lib/jsse.jar:");
		strcat(boot_class_path, JAVA_RUNTIME_LIBRARY_PREFIX"/lib/jce.jar:");
		strcat(boot_class_path, JAVA_RUNTIME_LIBRARY_PREFIX"/lib/charsets.jar:");
		strcat(boot_class_path, JAVA_RUNTIME_LIBRARY_PREFIX"/classes");

# elif defined(WITH_JAVA_RUNTIME_LIBRARY_CLDC1_1)

		len =
			strlen(JAVA_RUNTIME_LIBRARY_CLASSES) +
			strlen("0");

		boot_class_path = MNEW(char, len);

		strcpy(boot_class_path, JAVA_RUNTIME_LIBRARY_CLASSES);

# else
#  error unknown classpath configuration
# endif
#endif
	}

	put("sun.boot.class.path", boot_class_path);
	put("java.boot.class.path", boot_class_path);

#if defined(ENABLE_JAVASE)

	/* Set the classpath. */

	p = os::getenv("CLASSPATH");

	char* class_path;

	if (p != NULL) {
		class_path = MNEW(char, strlen(p) + strlen("0"));
		strcpy(class_path, p);
	}
	else {
		class_path = MNEW(char, strlen(".") + strlen("0"));
		strcpy(class_path, ".");
	}

	put("java.class.path", class_path);

	// Add java.vm properties.
	put("java.vm.specification.version", "1.0");
	put("java.vm.specification.vendor", "Sun Microsystems Inc.");
	put("java.vm.specification.name", "Java Virtual Machine Specification");
	put("java.vm.version", VERSION_FULL);
	put("java.vm.vendor", "CACAOVM - Verein zur Foerderung der freien virtuellen Maschine CACAO");
	put("java.vm.name", "CACAO");

# if defined(ENABLE_INTRP)
	if (opt_intrp) {
		/* XXX We don't support java.lang.Compiler */
/*  		put("java.compiler", "cacao.intrp"); */
		put("java.vm.info", "interpreted mode");
	}
	else
# endif
	{
		/* XXX We don't support java.lang.Compiler */
/*  		put("java.compiler", "cacao.jit"); */
		put("java.vm.info", "compiled mode");
	}

	// Get and set java.library.path.
	const char* java_library_path = os::getenv("LD_LIBRARY_PATH");

	if (java_library_path == NULL)
		java_library_path = "";

	put("java.library.path", java_library_path);

# if defined(WITH_JAVA_RUNTIME_LIBRARY_GNU_CLASSPATH)

	/* Get properties from system. */

	char* cwd      = os::getcwd();

	char* env_user = os::getenv("USER");
	char* env_home = os::getenv("HOME");
	char* env_lang = os::getenv("LANG");

	utsnamebuf = NEW(struct utsname);

	uname(utsnamebuf);

	put("java.runtime.version", VERSION_FULL);
	put("java.runtime.name", "CACAO");

	put("java.specification.version", "1.5");
	put("java.specification.vendor", "Sun Microsystems Inc.");
	put("java.specification.name", "Java Platform API Specification");

	put("java.version", JAVA_VERSION);
	put("java.vendor", "GNU Classpath");
	put("java.vendor.url", "http://www.gnu.org/software/classpath/");

	put("java.class.version", CLASS_VERSION);

	put("gnu.classpath.boot.library.path", boot_library_path);

	put("java.io.tmpdir", "/tmp");

#  if defined(ENABLE_INTRP)
	if (opt_intrp) {
		put("gnu.java.compiler.name", "cacao.intrp");
	}
	else
#  endif
	{
		put("gnu.java.compiler.name", "cacao.jit");
	}

	/* Set the java.ext.dirs property. */

	len = strlen(java_home) + strlen("/jre/lib/ext") + strlen("0");

	char* extdirs = MNEW(char, len);

	sprintf(extdirs, "%s/jre/lib/ext", java_home);

	put("java.ext.dirs", extdirs);

	/* Set the java.ext.endorsed property. */

	len = strlen(java_home) + strlen("/jre/lib/endorsed") + strlen("0");

	char* endorseddirs = MNEW(char, len);

	sprintf(endorseddirs, "%s/jre/lib/endorsed", java_home);

	put("java.endorsed.dirs", endorseddirs);

#  if defined(DISABLE_GC)
	/* When we disable the GC, we mmap the whole heap to a specific
	   address, so we can compare call traces. For this reason we have
	   to add the same properties on different machines, otherwise
	   more memory may be allocated (e.g. strlen("i386")
	   vs. strlen("alpha"). */

	put("os.arch", "unknown");
 	put("os.name", "unknown");
	put("os.version", "unknown");
#  else
	put("os.arch", JAVA_ARCH);
 	put("os.name", utsnamebuf->sysname);
	put("os.version", utsnamebuf->release);
#  endif

#  if WORDS_BIGENDIAN == 1
	put("gnu.cpu.endian", "big");
#  else
	put("gnu.cpu.endian", "little");
#  endif

	put("file.separator", "/");
	put("path.separator", ":");
	put("line.separator", "\n");

	put("user.name", env_user ? env_user : "null");
	put("user.home", env_home ? env_home : "null");
	put("user.dir", cwd ? cwd : "null");

	/* get locale */

	bool use_en_US = true;
	if (env_lang != NULL) {
#if defined(HAVE_SETLOCALE) && defined(HAVE_LC_MESSAGES)
		/* get the locale stuff from the environment */
		char *locale;

		if ((locale = setlocale(LC_MESSAGES, ""))) {
			int len = strlen(locale);
			if (((len >= 5) && (locale[2] == '_')) || len == 2)  {
				use_en_US = false;
				char* lang = MNEW(char, 3);
				strncpy(lang, (char*) &locale[0], 2);
				lang[2] = '\0';
				put("user.language", lang);

				if (len >= 5) {
					char* country = MNEW(char, 3);
					strncpy(country, (char*) &locale[3], 2);
					country[2] = '\0';

					put("user.country", country);
				}
			}
		}
#endif
	}
	if (use_en_US) {
		/* if no default locale was specified, use `en_US' */

		put("user.language", "en");
		put("user.country", "US");
	}

# elif defined(WITH_JAVA_RUNTIME_LIBRARY_OPENJDK)

	/* Actually this property is set by OpenJDK, but we need it in
	   nativevm_preinit(). */

	put("sun.boot.library.path", boot_library_path);

	// Set the java.ext.dirs property.
	len =
		strlen(java_home) + strlen("/lib/ext") +
		strlen(":") +
		strlen("/usr/java/packages/lib/ext") +
		strlen("0");

	char* extdirs = MNEW(char, len);

	sprintf(extdirs, "%s/lib/ext:/usr/java/packages/lib/ext", java_home);

	put("java.ext.dirs", extdirs);

	// Set the java.ext.endorsed property.
	len = strlen(java_home) + strlen("/lib/endorsed") + strlen("0");

	char* endorseddirs = MNEW(char, len);

	sprintf(endorseddirs, "%s/lib/endorsed", java_home);

	put("java.endorsed.dirs", endorseddirs);

# else

#  error unknown classpath configuration

# endif

#elif defined(ENABLE_JAVAME_CLDC1_1)

    put("microedition.configuration", "CLDC-1.1");
    put("microedition.platform", "generic");
    put("microedition.encoding", "ISO8859_1");
    put("microedition.profiles", "");

#else

# error unknown Java configuration

#endif
}


/**
 * Add the given property to the given Java system properties.
 *
 * @param p     Java properties object.
 * @param key   Key.
 * @param value Value.
 */
void Properties::put(java_handle_t* p, const char* key, const char* value)
{
	// Get Properties.put() method to add properties.
	classinfo* c;
	LLNI_class_get(p, c);

	methodinfo* m = class_resolveclassmethod(c,
											 utf8::put,
											 Utf8String::from_utf8("(Ljava/lang/Object;Ljava/lang/Object;)Ljava/lang/Object;"),
											 NULL,
											 true);

	if (m == NULL)
		return;

	// Add to the Java system properties.
	java_handle_t* k = JavaString::from_utf8(key);
	java_handle_t* v = JavaString::from_utf8(value);

	(void) vm_call_method(m, p, k, v);
}


/**
 * Put the given property into the internal property map.  If there's
 * already an entry with the same key, replace it.
 *
 * @param key   Key.
 * @param value Value.
 */
void Properties::put(const char* key, const char* value)
{
	// Try to find the key.
	std::map<const char*, const char*>::iterator it = _properties.find(key);

	// The key is already in the map.
	if (it != _properties.end()) {
		LOG("[Properties::put: " << "key=" << key << ", old value="<< it->second << ", new value=" << value << "]" << cacao::nl);

		// Replace the value in the current entry.
		it->second = value;

		return;
	}

	// The key was not found, insert the pair.
	LOG("[Properties::put: " << "key=" << key << ", value=" << value << "]" << cacao::nl);

	_properties.insert(std::make_pair(key, value));
}


/**
 * Get a property entry from the internal property map.
 *
 * @param key Key.
 *
 * @return Value associated with the key or NULL when not found.
 */
const char* Properties::get(const char* key)
{
	// Try to find the key.
	std::map<const char*, const char*>::iterator it = _properties.find(key);

	// The key is not in the map.
	if (it == _properties.end())
		return NULL;

	// Return the value.
	return it->second;
}


/**
 * Fill the given Java system properties with all properties from the
 * internal properties map.
 *
 * @param p Java Properties object.
 */
#if defined(ENABLE_JAVASE)
void Properties::fill(java_handle_t* p)
{
	// Get Properties.put() method to add properties.
	classinfo* c;
	LLNI_class_get(p, c);

	methodinfo* m = class_resolveclassmethod(c,
											 utf8::put,
											 Utf8String::from_utf8("(Ljava/lang/Object;Ljava/lang/Object;)Ljava/lang/Object;"),
											 NULL,
											 true);

	if (m == NULL)
		return;

	// Iterator over all properties.
	for (std::map<const char*, const char*>::iterator it = _properties.begin(); it != _properties.end(); it++) {
		// Put into the Java system properties.
		java_handle_t* key   = JavaString::from_utf8(it->first);
		java_handle_t* value = JavaString::from_utf8(it->second);

		(void) vm_call_method(m, p, key, value);
	}
}
#endif


/**
 * Dump all property entries.
 */
#if !defined(NDEBUG)
void Properties::dump()
{
	for (std::map<const char*, const char*>::iterator it = _properties.begin(); it != _properties.end(); it++) {
		log_println("[Properties::dump: key=%s, value=%s]", it->first, it->second);
	}
}
#endif


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
