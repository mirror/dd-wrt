/*-
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 2002, 2017 Oracle and/or its affiliates.  All rights reserved.
 *
 * $Id$
 */

package com.sleepycat.client;

import org.apache.thrift.transport.TIOStreamTransport;
import org.apache.thrift.transport.TServerTransport;
import org.apache.thrift.transport.TTransport;
import org.apache.thrift.transport.TTransportException;

import java.io.InputStream;
import java.io.OutputStream;

class IOStreamServerTransport extends TServerTransport {

    private TTransport transport;

    IOStreamServerTransport(InputStream in, OutputStream out) {
        this.transport = new TIOStreamTransport(in, out);
    }

    @Override
    public void listen() throws TTransportException {
    }

    @Override
    public void close() {
        this.transport.close();
    }

    @Override
    protected TTransport acceptImpl() throws TTransportException {
        return this.transport;
    }
}
