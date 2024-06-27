/* KeyPairGeneratorAdapter.java --
   Copyright 2001, 2002, 2006, 2015 Free Software Foundation, Inc.

This file is a part of GNU Classpath.

GNU Classpath is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or (at
your option) any later version.

GNU Classpath is distributed in the hope that it will be useful, but
WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
General Public License for more details.

You should have received a copy of the GNU General Public License
along with GNU Classpath; if not, write to the Free Software
Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301
USA

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
exception statement from your version.  */


package gnu.java.security.jce.sig;

import gnu.java.security.Registry;
import gnu.java.security.key.IKeyPairGenerator;
import gnu.java.security.key.KeyPairGeneratorFactory;

import java.security.InvalidAlgorithmParameterException;
import java.security.KeyPair;
import java.security.KeyPairGenerator;
import java.security.SecureRandom;
import java.security.spec.AlgorithmParameterSpec;

/**
 * The implementation of a generic {@link java.security.KeyPairGenerator}
 * adapter class to wrap GNU keypair generator instances.
 * <p>
 * This class defines the <i>Service Provider Interface</i> (<b>SPI</b>) for
 * the {@link java.security.KeyPairGenerator} class, which is used to generate
 * pairs of public and private keys.
 * <p>
 * All the abstract methods in the {@link java.security.KeyPairGeneratorSpi}
 * class are implemented by this class and all its sub-classes.
 * <p>
 * In case the client does not explicitly initialize the KeyPairGenerator (via a
 * call to an <code>initialize()</code> method), the GNU provider supplies
 * (and document) default values to be used. For example, the GNU provider uses
 * a default <i>modulus</i> size (keysize) of 1024 bits for the DSS (Digital
 * Signature Standard) a.k.a <i>DSA</i>.
 */
public abstract class KeyPairGeneratorAdapter
    extends KeyPairGenerator
{
  /** Our underlying keypair instance. */
  protected IKeyPairGenerator adaptee;

  /**
   * Trivial protected constructor.
   *
   * @param kpgName the canonical name of the keypair generator algorithm.
   */
  protected KeyPairGeneratorAdapter(String kpgName)
  {
    super(kpgName);

    this.adaptee = KeyPairGeneratorFactory.getInstance(localiseName(kpgName));
  }

  @Override
  public abstract void initialize(int keysize, SecureRandom random);

  @Override
  public abstract void initialize(AlgorithmParameterSpec params,
                                  SecureRandom random)
      throws InvalidAlgorithmParameterException;

  @Override
  public KeyPair generateKeyPair()
  {
    if (!adaptee.isInitialized())
      initialize(adaptee.getDefaultKeySize());

    return adaptee.generate();
  }

  /**
   * The {@code java.security.*} methods are expected to return
   * standard algorithm names, as listed in
   * <a href="http://docs.oracle.com/javase/7/docs/technotes/guides/security/StandardNames.html#KeyPairGenerator">
   * the on-line Oracle documentation</a>.
   *
   * @return the name specified by the Oracle documentation.
   */
  @Override
  public String getAlgorithm()
  {
    String alg = super.getAlgorithm();

    if ("dh".equals(alg))
      return "DH";
    if ("dsa".equals(alg))
      return "DSA";
    if ("dss".equals(alg))
      return "DSA";
    if ("rsa".equals(alg))
      return "RSA";
    return alg;
  }

  /**
   * The user may specify an algorithm using the names specified in
   * <a href="http://docs.oracle.com/javase/7/docs/technotes/guides/security/StandardNames.html#KeyPairGenerator">
   * the on-line Oracle documentation</a>. We should translate them
   * to names recognised by the GNU registry.
   *
   * @param kpgName the generator name, which may be its standardised
   *                name.
   * @return the name specified by the GNU registry.
   */
  private static String localiseName(String kpgName)
  {
    if ("DiffieHellman".equals(kpgName))
      return Registry.DH_KPG;
    if ("DH".equals(kpgName))
      return Registry.DH_KPG;
    if ("DSA".equals(kpgName))
      return Registry.DSA_KPG;
    if ("RSA".equals(kpgName))
      return Registry.RSA_KPG;
    return kpgName;
  }

}
