/*-
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 2002, 2017 Oracle and/or its affiliates.  All rights reserved.
 *
 * $Id$
 */

package com.sleepycat.client;

import com.sleepycat.thrift.TDbt;
import com.sleepycat.thrift.TGetResult;
import com.sleepycat.thrift.TGetWithPKeyResult;
import com.sleepycat.thrift.TKeyData;
import com.sleepycat.thrift.TKeyDataWithPKey;
import com.sleepycat.thrift.TOperationStatus;
import org.apache.thrift.TException;

import java.util.List;
import java.util.stream.Collectors;

/**
 * GetHelper implements a few helper methods for making remote get / pGet
 * calls.
 */
interface GetHelper extends RemoteCallHelper {

    /**
     * This method helps to create the TKeyData argument used in a remote get
     * call.
     *
     * @param key the key argument
     * @param data the data argument
     * @return the search term used for a remote get call
     */
    default TKeyData createGetSearchTerm(SDatabaseEntry key,
            SDatabaseEntryBase data) {
        TKeyData searchTerm = new TKeyData();
        if (key != null) {
            searchTerm.setKey(key.getThriftObj());
        }
        if (data instanceof SDatabaseEntry) {
            searchTerm.setData(((SDatabaseEntry) data).getThriftObj());
        } else if (data != null) {
            searchTerm.setData(new TDbt());
        }
        return searchTerm;
    }

    /**
     * This method helps to create the TKeyDataWithPKey argument used in a
     * remote pGet call.
     *
     * @param sKey the sKey argument
     * @param pKey the pKey argument
     * @param pData the pData argument
     * @return the search term used for a remote pGet call
     */
    default TKeyDataWithPKey createPGetSearchTerm(SDatabaseEntry sKey,
            SDatabaseEntry pKey, SDatabaseEntry pData) {
        TKeyDataWithPKey searchTerm = new TKeyDataWithPKey();
        if (sKey != null) {
            searchTerm.setSkey(sKey.getThriftObj());
        }
        if (pKey != null) {
            searchTerm.setPkey(pKey.getThriftObj());
        }
        if (pData != null) {
            searchTerm.setPdata(pData.getThriftObj());
        }
        return searchTerm;
    }

    /**
     * This method helps to update the output arguments of a get call, using
     * values returned from a remote get call.
     *
     * @param key the key argument
     * @param data the data argument
     * @param pairs the key / data pairs retrieved from a remote get call
     */
    default void updateKeyData(SDatabaseEntry key, SDatabaseEntryBase data,
            List<TKeyData> pairs) {
        if (pairs.isEmpty()) {
            return;
        }

        if (data instanceof SMultiplePairs) {
            ((SMultiplePairs) data).setEntriesFromKeyDatas(pairs);
        } else {
            TKeyData first = pairs.get(0);
            if (key != null) {
                key.setDataFromTDbt(first.key);
            }
            if (data instanceof SDatabaseEntry) {
                ((SDatabaseEntry) data).setDataFromTDbt(first.data);
            } else if (data instanceof SMultipleDataEntry) {
                ((SMultipleDataEntry) data).setEntriesFromDbts(
                        pairs.stream().map(p -> p.data)
                                .collect(Collectors.toList()));
            }
        }
    }

    /**
     * This method helps to update the output arguments of a pGet call, using
     * values returned from a remote pGet call.
     *
     * @param sKey the sKey argument
     * @param pKey the pKey argument
     * @param pData the pData argument
     * @param tuple the tuple returned from a remote pGet call
     */
    default void updateTuple(SDatabaseEntry sKey, SDatabaseEntry pKey,
            SDatabaseEntry pData, TKeyDataWithPKey tuple) {
        if (sKey != null) {
            sKey.setDataFromTDbt(tuple.skey);
        }
        if (pKey != null) {
            pKey.setDataFromTDbt(tuple.pkey);
        }
        if (pData != null) {
            pData.setDataFromTDbt(tuple.pdata);
        }
    }

    default SOperationStatus remoteGet(SDatabaseEntry key,
            SDatabaseEntryBase data, RemoteGetFunction getFunc)
            throws SDatabaseException {
        TKeyData searchTerm = createGetSearchTerm(key, data);
        return remoteCall(() -> {
            TGetResult result = getFunc.applyWithException(searchTerm);
            if (result.status == TOperationStatus.SUCCESS) {
                updateKeyData(key, data, result.pairs);
                return SOperationStatus.SUCCESS;
            } else {
                return SOperationStatus.toBdb(result.status);
            }
        });
    }

    default SOperationStatus remotePGet(SDatabaseEntry sKey,
            SDatabaseEntry pKey, SDatabaseEntry pData,
            RemotePGetFunction pGetFunc) throws SDatabaseException {
        if (pKey != null && pKey.getPartial()) {
            throw new IllegalArgumentException(
                    "Partial primary key is not supported.");
        }
        TKeyDataWithPKey searchTerm = createPGetSearchTerm(sKey, pKey, pData);
        return remoteCall(() -> {
            TGetWithPKeyResult result = pGetFunc.applyWithException(searchTerm);
            if (result.status == TOperationStatus.SUCCESS) {
                updateTuple(sKey, pKey, pData, result.tuple);
                return SOperationStatus.SUCCESS;
            } else {
                return SOperationStatus.toBdb(result.status);
            }
        });
    }

    @FunctionalInterface
    interface RemoteGetFunction {
        TGetResult applyWithException(TKeyData keyData) throws TException;
    }

    @FunctionalInterface
    interface RemotePGetFunction {
        TGetWithPKeyResult applyWithException(TKeyDataWithPKey tKeyDataWithPKey)
                throws TException;
    }
}
