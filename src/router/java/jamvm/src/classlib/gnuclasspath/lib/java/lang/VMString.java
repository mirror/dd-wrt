package java.lang;

/**
 * Helper class for String, to abstract VM-specifics.
 *
 * @author Robert Lougher
 */
final class VMString
{
  /**
   * JamVM uses its own internal String hashtable which is much
   * faster than the reference Classpath implementation.
   *
   * @param s the String to intern
   * @return existing interned string or s
   */
  static native String intern(String s);
}
