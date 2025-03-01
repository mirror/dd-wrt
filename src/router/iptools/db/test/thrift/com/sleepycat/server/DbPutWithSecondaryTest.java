package com.sleepycat.server;

import com.sleepycat.db.DatabaseEntry;
import com.sleepycat.thrift.TDatabase;
import com.sleepycat.thrift.TDatabaseConfig;
import com.sleepycat.thrift.TDatabaseType;
import com.sleepycat.thrift.TDbGetConfig;
import com.sleepycat.thrift.TDbGetMode;
import com.sleepycat.thrift.TDbPutConfig;
import com.sleepycat.thrift.TDbt;
import com.sleepycat.thrift.TEnvironment;
import com.sleepycat.thrift.TEnvironmentConfig;
import com.sleepycat.thrift.TGetResult;
import com.sleepycat.thrift.TGetWithPKeyResult;
import com.sleepycat.thrift.TKeyData;
import com.sleepycat.thrift.TKeyDataWithPKey;
import com.sleepycat.thrift.TKeyDataWithSecondaryKeys;
import com.sleepycat.thrift.TOperationStatus;
import com.sleepycat.thrift.TPutResult;
import com.sleepycat.thrift.TSecondaryDatabaseConfig;
import org.apache.thrift.TException;
import org.junit.Assert;
import org.junit.Before;
import org.junit.Test;

import java.util.Arrays;
import java.util.Collections;
import java.util.List;
import java.util.stream.Collectors;

import static org.hamcrest.CoreMatchers.is;
import static org.hamcrest.MatcherAssert.assertThat;

public class DbPutWithSecondaryTest extends BdbServiceHandlerTestBase {

    private static final String ENV_HOME = "test_second_db_put";

    private static final String PRIMARY_DB_FILE = "primary";

    private static final String SECONDARY_DB_FILE = "secondary";

    private TEnvironment env;

    private TDatabase primary;

    private TDatabase secondary;

    @Before
    public void setUp() throws Exception {
        super.setUp();

        env = handler.openEnvironment(ENV_HOME,
                new TEnvironmentConfig().setAllowCreate(true));

        TDatabaseConfig config = new TDatabaseConfig().setAllowCreate(true)
                .setType(TDatabaseType.BTREE);
        primary =
                handler.openDatabase(env, null, PRIMARY_DB_FILE, null, config);
        secondary = handler.openSecondaryDatabase(env, null, SECONDARY_DB_FILE,
                null, primary,
                new TSecondaryDatabaseConfig().setDbConfig(config));
    }

    @Test
    public void testPutSingleKey() throws Exception {
        test("pKey", "pData", Collections.singletonList("sKey"));
    }

    @Test
    public void testPutMultipleKey() throws Exception {
        test("pKey", "pData", Arrays.asList("sKey1", "sKey2"));
    }

    @Test
    public void testPutNoKey() throws Exception {
        test("pKey", "pData", Collections.emptyList());
    }

    @Test
    public void testUpdate() throws Exception {
        test("pKey", "pData", Collections.singletonList("sKey"));
        test("pKey", "newData", Collections.singletonList("newKey"));
    }

    private void test(String pKey, String pData, List<String> sKeys)
            throws Exception {
        TKeyDataWithSecondaryKeys pair = new TKeyDataWithSecondaryKeys();
        pair.setPkey(new TDbt().setData(pKey.getBytes()));
        pair.setPdata(new TDbt().setData(pData.getBytes()));
        pair.putToSkeys(secondary,
                sKeys.stream().map(sKey -> new TDbt().setData(sKey.getBytes()))
                        .collect(Collectors.toList()));

        TPutResult result =
                handler.dbPut(primary, null, Collections.singletonList(pair),
                        TDbPutConfig.DEFAULT);

        assertThat(result.getStatus(), is(TOperationStatus.SUCCESS));

        TKeyData keyData = new TKeyData().setData(new TDbt());
        keyData.setKey(new TDbt().setData(pKey.getBytes()));

        TGetResult getResult = handler.dbGet(primary, null, keyData,
                new TDbGetConfig().setMode(TDbGetMode.DEFAULT));

        assertThat(getResult.getStatus(), is(TOperationStatus.SUCCESS));
        assertThat(new String(getResult.pairs.get(0).data.getData()),
                is(pData));

        for (String sKey : sKeys) {
            TKeyDataWithPKey keyDataWithPKey = new TKeyDataWithPKey();
            keyDataWithPKey.setPdata(new TDbt()).setPkey(new TDbt());
            keyDataWithPKey.setSkey(new TDbt().setData(sKey.getBytes()));

            TGetWithPKeyResult pGetResult =
                    handler.dbGetWithPKey(secondary, null, keyDataWithPKey,
                            new TDbGetConfig().setMode(TDbGetMode.DEFAULT));

            assertThat(pGetResult.getStatus(), is(TOperationStatus.SUCCESS));
            assertThat(new String(pGetResult.tuple.pkey.getData()), is(pKey));
            assertThat(new String(pGetResult.tuple.pdata.getData()), is(pData));
        }
    }

    @Test
    public void testAppend() throws Exception {
        TDatabaseConfig config = new TDatabaseConfig().setAllowCreate(true)
                .setType(TDatabaseType.RECNO);
        TDatabase recNum =
                handler.openDatabase(env, null, "recNum", null, config);
        handler.openSecondaryDatabase(env, null, "rec_sec", null, recNum,
                new TSecondaryDatabaseConfig().setDbConfig(config));

        TKeyDataWithSecondaryKeys pair = new TKeyDataWithSecondaryKeys();
        DatabaseEntry entry = new DatabaseEntry();
        entry.setRecordNumber(1);
        pair.setPkey(new TDbt().setData(entry.getData()));
        pair.setPdata(new TDbt().setData("pData".getBytes()));
        try {
            handler.dbPut(recNum, null, Collections.singletonList(pair),
                    TDbPutConfig.APPEND);
            Assert.fail();
        } catch (TException e) {
            assertThat(isUnsupportedOperationException(e), is(true));
        }
    }
}