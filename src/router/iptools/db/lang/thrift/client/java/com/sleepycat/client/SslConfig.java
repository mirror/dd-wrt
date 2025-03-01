/*-
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 2002, 2017 Oracle and/or its affiliates.  All rights reserved.
 *
 * $Id$
 */

package com.sleepycat.client;

import org.apache.thrift.transport.TSSLTransportFactory;

/**
 * Specifies the attributes used to setup SSL connections.
 */
public class SslConfig {
    /** The parameters. */
    private final TSSLTransportFactory.TSSLTransportParameters parameters =
            new TSSLTransportFactory.TSSLTransportParameters();

    /**
     * Configure key store parameters.
     *
     * @param keyStore the path to the key store
     * @param keyPass the password to access the key store
     * @param keyManagerType the KeyManager type
     * @param keyStoreType the KeyStore type
     * @return this
     */
    public SslConfig setKeyStore(String keyStore, String keyPass,
            String keyManagerType, String keyStoreType) {
        parameters.setKeyStore(keyStore, keyPass, keyManagerType, keyStoreType);
        return this;
    }

    /**
     * Configure key store parameters.
     *
     * @param keyStore the path to the key store
     * @param keyPass the password to access the key store
     * @return this
     */
    public SslConfig setKeyStore(String keyStore, String keyPass) {
        parameters.setKeyStore(keyStore, keyPass);
        return this;
    }

    /**
     * Configure trust store parameters.
     *
     * @param trustStore the path to the trust store
     * @param trustPass the password to access the trust store
     * @param trustManagerType the TrustManager type
     * @param trustStoreType the TrustStore type
     * @return this
     */
    public SslConfig setTrustStore(String trustStore, String trustPass,
            String trustManagerType, String trustStoreType) {
        parameters
                .setTrustStore(trustStore, trustPass, trustManagerType,
                        trustStoreType);
        return this;
    }

    /**
     * Configure trust store parameters.
     *
     * @param trustStore the path to the trust store
     * @param trustPass the password to access the trust store
     * @return this
     */
    public SslConfig setTrustStore(String trustStore, String trustPass) {
        parameters.setTrustStore(trustStore, trustPass);
        return this;
    }

    TSSLTransportFactory.TSSLTransportParameters getParameters() {
        return parameters;
    }
}
