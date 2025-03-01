/*-
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 2002, 2017 Oracle and/or its affiliates.  All rights reserved.
 *
 * $Id$
 */

package com.sleepycat.client;

import com.sleepycat.thrift.TDatabase;
import com.sleepycat.thrift.TDbt;
import com.sleepycat.thrift.TKeyDataWithSecondaryKeys;
import com.sleepycat.thrift.TPutResult;
import org.apache.thrift.TException;

import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.Set;
import java.util.stream.Collectors;

/**
 * PutHelper implements a few helper methods for making remote put calls.
 */
interface PutHelper extends RemoteCallHelper {

    /**
     * Calculate the secondary keys generated for each secondary database for
     * the specified primary key / data pair.
     *
     * @param key the primary key
     * @param data the primary data
     * @return a TKeyDataWithSecondaryKeys
     */
    default TKeyDataWithSecondaryKeys calculateSKey(SDatabaseEntry key,
            SDatabaseEntry data) {
        if (!getSecondaryDatabases().isEmpty() && data.getPartial()) {
            throw new UnsupportedOperationException("Partial update is not" +
                    " supported for databases having secondary databases.");
        }

        TKeyDataWithSecondaryKeys ret = new TKeyDataWithSecondaryKeys();
        ret.setPkey(key.getThriftObj()).setPdata(data.getThriftObj());

        Map<TDatabase, List<TDbt>> sKeys = new HashMap<>();
        getSecondaryDatabases().forEach(sdb -> {
            Set<SDatabaseEntry> results = sdb.calculateSKeys(key, data);
            List<TDbt> keys = results.stream().map(ThriftWrapper::getThriftObj)
                    .collect(Collectors.toList());
            sKeys.put(sdb.getThriftObj(), keys);
        });
        ret.setSkeys(sKeys);

        return ret;
    }

    default SOperationStatus remotePut(List<TKeyDataWithSecondaryKeys> pairs,
            SDatabaseEntry retKey, RemotePutFunction func)
            throws SDatabaseException {
        return remoteCall(() -> {
            TPutResult result = func.applyWithException(pairs);

            if (result.isSetNewRecordNumber()) {
                retKey.setData(result.getNewRecordNumber());
            }

            return SOperationStatus.toBdb(result.status);
        });
    }

    /**
     * Return the set of secondary databases associated with this primary
     * database.
     *
     * @return the set of associated secondary databases
     */
    Set<SSecondaryDatabase> getSecondaryDatabases();

    @FunctionalInterface
    interface RemotePutFunction {
        TPutResult applyWithException(List<TKeyDataWithSecondaryKeys> pairs)
                throws TException;
    }
}
