/* VMClassLoader.java -- Reference implementation of native interface
   required by ClassLoader
   Copyright (C) 1998, 2001, 2002, 2004, 2005, 2006 Free Software Foundation

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

/*
Robert Lougher 17/11/2003.
Major modifications have been made to this Classpath reference
implementation to work with JamVM.
*/

package java.lang;

import gnu.classpath.Configuration;
import gnu.classpath.SystemProperties;
import gnu.java.lang.InstrumentationImpl;

import java.lang.instrument.Instrumentation;
import java.net.MalformedURLException;
import java.net.URL;
import java.security.ProtectionDomain;
import java.util.Enumeration;
import java.util.HashMap;
import java.util.Map;
import java.util.Vector;

import java.util.jar.Manifest;
import java.util.jar.Attributes;

/**
 * java.lang.VMClassLoader is a package-private helper for VMs to implement
 * on behalf of java.lang.ClassLoader.
 *
 * @author John Keiser
 * @author Mark Wielaard (mark@klomp.org)
 * @author Eric Blake (ebb9@email.byu.edu)
 */
final class VMClassLoader
{
  static PackageInfo[] packageInfo;

  /**
   * Helper to define a class using a string of bytes. This assumes that
   * the security checks have already been performed, if necessary.
   *
   * @param name the name to give the class, or null if unknown
   * @param data the data representing the classfile, in classfile format
   * @param offset the offset into the data where the classfile starts
   * @param len the length of the classfile data in the array
   * @param pd the protection domain
   * @return the class that was defined
   * @throws ClassFormatError if data is not in proper classfile format
   */
  static final native Class defineClass(ClassLoader cl, String name,
                                 byte[] data, int offset, int len,
                                 ProtectionDomain pd)
    throws ClassFormatError;

  /**
   * Helper to resolve all references to other classes from this class.
   *
   * @param c the class to resolve
   */
  static final native void resolveClass(Class c);

  /**
   * Helper to load a class from the bootstrap class loader.
   *
   * @param name the class name to load
   * @param resolve whether to resolve it
   * @return the class, loaded by the bootstrap classloader or null
   * if the class wasn't found. Returning null is equivalent to throwing
   * a ClassNotFoundException (but a possible performance optimization).
   */
  static final native Class loadClass(String name, boolean resolve)
    throws ClassNotFoundException;

  /**
   * Helper to load a resource from the bootstrap class loader.
   *
   * @param name the resource to find
   * @return the URL to the resource
   */
  static URL getResource(String name) {
    int entries = getBootClassPathSize();

    for(int i = 0; i < entries; i++) {
      String url = getBootClassPathResource(name, i);
      if(url != null)
        try {
          return new URL(url);
        } catch (MalformedURLException e) {}
    }

    return null;
  }

  synchronized static void initBootPackages()
  {
    if(packageInfo == null) {
      int entries = getBootClassPathSize();
      packageInfo = new PackageInfo[entries];

      for(int i = 0; i < entries; i++) {
        String specTitle = null, specVendor = null, specVersion = null;
        String implTitle = null, implVendor = null, implVersion = null;

        String res = getBootClassPathResource("java/lang/Object.class", i);
  
        if(res != null) {
          specTitle = SystemProperties.getProperty("java.specification.name");
          specVendor = SystemProperties.getProperty("java.specification.vendor");
          specVersion = SystemProperties.getProperty("java.specification.version");
          implTitle = "GNU Classpath";
          implVendor = "GNU";
          implVersion = Configuration.CLASSPATH_VERSION;
        } else {
          res = getBootClassPathResource("jamvm/java/lang/JarLauncher.class", i);

          if(res != null) {
            specTitle = SystemProperties.getProperty("java.specification.name");
            specVendor = SystemProperties.getProperty("java.specification.vendor");
            specVersion = SystemProperties.getProperty("java.specification.version");
            implTitle = "JamVM";
            implVendor = "Robert Lougher";
            implVersion = SystemProperties.getProperty("java.runtime.version");
          } else {
            res = getBootClassPathResource("META-INF/MANIFEST.MF", i);
  
            if(res != null)
              try {
                URL url = new URL(res);
                Manifest man = new Manifest(url.openStream());
                Attributes attr = man.getMainAttributes();

                specTitle = attr.getValue(Attributes.Name.SPECIFICATION_TITLE);
                specVersion = attr.getValue(Attributes.Name.SPECIFICATION_VERSION);
                specVendor = attr.getValue(Attributes.Name.SPECIFICATION_VENDOR);
                implTitle = attr.getValue(Attributes.Name.IMPLEMENTATION_TITLE);
                implVersion = attr.getValue(Attributes.Name.IMPLEMENTATION_VERSION);
                implVendor = attr.getValue(Attributes.Name.IMPLEMENTATION_VENDOR);
              } catch(Exception e) {}
          }
        }

        packageInfo[i] = new PackageInfo(specTitle, specVersion, specVendor,
                                         implTitle, implVersion, implVendor);
      }
    }
  }

  /**
   * Helper to get a list of resources from the bootstrap class loader.
   *
   * @param name the resource to find
   * @return an enumeration of resources
   */
  static Enumeration getResources(String name) {
    int entries = getBootClassPathSize();
    Vector list = new Vector();

    for(int i = 0; i < entries; i++) {
      String url = getBootClassPathResource(name, i);
      if(url != null)
        try {
          list.add(new URL(url));
        } catch (MalformedURLException e) {}
    }

    return list.elements();
  }

  /**
   * Helper to get a package from the bootstrap class loader.
   *
   * @param name the name to find
   * @return the named package, if it exists
   */
  static Package getPackage(String name)
  {
    initBootPackages();
    return getBootPackage(name);
  }

  /**
   * Helper to get all packages from the bootstrap class loader.  
   *
   * @return all named packages, if any exist
   */
  static Package[] getPackages()
  {
    initBootPackages();
    return getBootPackages();
  }

  /**
   * Helper for java.lang.Integer, Byte, etc to get the TYPE class
   * at initialization time. The type code is one of the chars that
   * represents the primitive type as in JNI.
   *
   * <ul>
   * <li>'Z' - boolean</li>
   * <li>'B' - byte</li>
   * <li>'C' - char</li>
   * <li>'D' - double</li>
   * <li>'F' - float</li>
   * <li>'I' - int</li>
   * <li>'J' - long</li>
   * <li>'S' - short</li>
   * <li>'V' - void</li>
   * </ul>
   *
   * @param type the primitive type
   * @return a "bogus" class representing the primitive type
   */
  static final native Class getPrimitiveClass(char type);

  /**
   * The system default for assertion status. This is used for all system
   * classes (those with a null ClassLoader), as well as the initial value for
   * every ClassLoader's default assertion status.
   *
   * XXX - Not implemented yet; this requires native help.
   *
   * @return the system-wide default assertion status
   */
  static final boolean defaultAssertionStatus()
  {
    return true;
  }

  /**
   * The system default for package assertion status. This is used for all
   * ClassLoader's packageAssertionStatus defaults. It must be a map of
   * package names to Boolean.TRUE or Boolean.FALSE, with the unnamed package
   * represented as a null key.
   *
   * XXX - Not implemented yet; this requires native help.
   *
   * @return a (read-only) map for the default packageAssertionStatus
   */
  static final Map packageAssertionStatus()
  {
    return new HashMap();
  }

  /**
   * The system default for class assertion status. This is used for all
   * ClassLoader's classAssertionStatus defaults. It must be a map of
   * class names to Boolean.TRUE or Boolean.FALSE
   *
   * XXX - Not implemented yet; this requires native help.
   *
   * @return a (read-only) map for the default classAssertionStatus
   */
  static final Map classAssertionStatus()
  {
    return new HashMap();
  }

  static ClassLoader getSystemClassLoader()
  {
    return ClassLoader.defaultGetSystemClassLoader();
  }

  /**
   * Find the class if this class loader previously defined this class
   * or if this class loader has been recorded as the initiating class loader
   * for this class.
   */
  static native Class findLoadedClass(ClassLoader cl, String name);

  /**
   * The Instrumentation object created by the vm when agents are defined.
   */
  static final Instrumentation instrumenter = null;

  /**
   * Call the transformers of the possible Instrumentation object. This
   * implementation assumes the instrumenter is a
   * <code>InstrumentationImpl</code> object. VM implementors would
   * have to redefine this method if they provide their own implementation
   * of the <code>Instrumentation</code> interface.
   *
   * @param loader the initiating loader
   * @param name the name of the class
   * @param data the data representing the classfile, in classfile format
   * @param offset the offset into the data where the classfile starts
   * @param len the length of the classfile data in the array
   * @param pd the protection domain
   * @return the new data representing the classfile
   */
  static final Class defineClassWithTransformers(ClassLoader loader,
      String name, byte[] data, int offset, int len, ProtectionDomain pd)
  {
    
    if (instrumenter != null)
      {
        byte[] modifiedData = new byte[len];
        System.arraycopy(data, offset, modifiedData, 0, len);
        modifiedData =
          ((InstrumentationImpl)instrumenter).callTransformers(loader, name,
            null, pd, modifiedData);
        
        return defineClass(loader, name, modifiedData, 0, modifiedData.length,
            pd);
      }
    else
      {
        return defineClass(loader, name, data, offset, len, pd);
      }
  }

  private static Package createBootPackage(String name, int index)
  {
      return new Package(name, packageInfo[index].specTitle,
                               packageInfo[index].specVendor,
                               packageInfo[index].specVersion,
                               packageInfo[index].implTitle,
                               packageInfo[index].implVendor,
                               packageInfo[index].implVersion,
                               null, null);
  }

  /* Native helper functions */

  private static native Package[] getBootPackages();
  private static native Package getBootPackage(String name);

  private static native int getBootClassPathSize();
  private static native String getBootClassPathResource(String name, int index);

  private static class PackageInfo {
    String specTitle;
    String specVersion;
    String specVendor;
    String implTitle;
    String implVersion;
    String implVendor;

    PackageInfo(String specTitle, String specVersion, String specVendor,
                String implTitle, String implVersion, String implVendor)
    {
      this.specTitle = specTitle;
      this.specVersion = specVersion;
      this.specVendor = specVendor;
      this.implTitle = implTitle;
      this.implVersion = implVersion;
      this.implVendor = implVendor;
    }
  }
}
