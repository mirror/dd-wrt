/* regression/resolving/TestLoader.java - instrumented class loader

   Copyright (C) 1996-2013
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

import java.util.Hashtable;
import java.io.*;

// A simple class loader that logs events.
public class TestLoader extends ClassLoader {
	public TestLoader(String name, TestController controller) {
		this(getSystemClassLoader(), name, controller);
	}

	public TestLoader(ClassLoader parent, String name, TestController controller) {
		super(parent);
		name_       = name;
		controller_ = controller;
		registry_   = new Hashtable<String, Entry>();
	}

	// tell loader to load class from given file
	public void addClassfile(String classname, String filename) {
		registry_.put(classname, new ClassfileEntry(filename));
	}

	// tell loader to delegate loading to given other loader
	public void addDelegation(String classname, ClassLoader loader) {
		registry_.put(classname, new DelegationEntry(loader));
	}

	// tell loader to delegate loading of class to it's parent
	public void addParentDelegation(String classname) {
		addDelegation(classname, getParent());
	}

	@Override
	public Class<?> loadClass(String classname) throws ClassNotFoundException {
		controller_.reportRequest(this, classname);

		Entry entry = registry_.get(classname);

		if (entry == null) {
			throw new ClassNotFoundException(this + " does not know how to load class " + classname);
		}

		Class<?> cls = entry.loadClass(classname);

		controller_.reportLoaded(this, cls);

		return cls;
	}

	@Override
	public String toString() {
		return "TestLoader(" + name_ + ")";
	}

	// ***** private parts

	private abstract class Entry {
		abstract Class<?> loadClass(String classname) throws ClassNotFoundException;
	}

	private class ClassfileEntry extends Entry {
		public String filename_;
		public ClassfileEntry(String filename) { filename_ = filename; }

		Class<?> loadClass(String classname) throws ClassNotFoundException {
			Class<?> cls = findLoadedClass(classname);

			if (cls != null) {
				controller_.reportFoundLoaded(TestLoader.this, cls);
				return cls;
			}

			String filename = filename_;

			try {
				byte[] bytes = slurpFile(filename);

				cls = defineClass(classname, bytes, 0, bytes.length);

				controller_.reportDefinition(TestLoader.this, cls);

				return cls;
			}
			catch (Exception e) {
				throw new ClassNotFoundException(e.toString());
			}            
		}
	}

	private class DelegationEntry extends Entry {
		public ClassLoader delegate;

		public DelegationEntry(ClassLoader loader) { delegate = loader; }

		Class<?> loadClass(String classname) throws ClassNotFoundException {
			controller_.reportDelegation(TestLoader.this, delegate, classname);

			return delegate.loadClass(classname);      
		}
	}

	private byte[] slurpFile(String filename) throws IOException {
		File file = new File(filename);
		InputStream is = new FileInputStream(file);
		long len = file.length();
		if (len > Integer.MAX_VALUE)
			throw new IOException("file " + file.getName() + " is too large");
		byte[] bytes = new byte[(int) len];

		int ofs = 0;
		int read = 0;
		while ((ofs < len) && (read = is.read(bytes, ofs, bytes.length - ofs)) >= 0)
			ofs += read;

		if (ofs < len)
			throw new IOException("error reading file " + file.getName());

		is.close();
		return bytes;
	}

	private final Hashtable<String, Entry> registry_;
	private final String                   name_;
	private final TestController           controller_;
}

/*
 * These are local overrides for various environment variables in Emacs.
 * Please do not remove this and leave it at the end of the file, where
 * Emacs will automagically detect them.
 * ---------------------------------------------------------------------
 * Local variables:
 * mode: java
 * indent-tabs-mode: t
 * c-basic-offset: 4
 * tab-width: 4
 * End:
 * vim:noexpandtab:sw=4:ts=4:
 */