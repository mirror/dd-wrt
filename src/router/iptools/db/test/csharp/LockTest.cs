/*-
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 2009, 2017 Oracle and/or its affiliates.  All rights reserved.
 *
 */
using System;
using System.Collections;
using System.Collections.Generic;
using System.IO;
using System.Text;
using System.Threading;
using System.Xml;
using NUnit.Framework;
using BerkeleyDB;

namespace CsharpAPITest {
    [TestFixture]
    public class LockTest  : CSharpTestFixture
    {
        DatabaseEnvironment testLockStatsEnv;
        BTreeDatabase testLockStatsDb;
        int DeadlockDidPut = 0;

        [TestFixtureSetUp]
        public void SetUpTestFixture() {
            testFixtureName = "LockTest";
            base.SetUpTestfixture();
        }

        [Test]
        public void TestLockStats() {
            testName = "TestLockStats";
            SetUpTest(true);

            // Configure locking subsystem.
            LockingConfig lkConfig = new LockingConfig();
            lkConfig.MaxLockers = 60;
            lkConfig.MaxLocks = 50;
            lkConfig.MaxObjects = 70;
            lkConfig.Partitions = 20;
            lkConfig.DeadlockResolution = DeadlockPolicy.DEFAULT;

            // Configure and open environment.
            DatabaseEnvironmentConfig envConfig =
                new DatabaseEnvironmentConfig();
            envConfig.Create = true;
            envConfig.FreeThreaded = true;
            envConfig.LockSystemCfg = lkConfig;
            envConfig.LockTimeout = 1000;
            envConfig.MPoolSystemCfg = new MPoolConfig();
            envConfig.MPoolSystemCfg.CacheSize = new CacheInfo(0, 104800, 1);
            envConfig.NoLocking = false;
            envConfig.TxnTimeout = 2000;
            envConfig.UseLocking = true;
            envConfig.UseMPool = true;
            envConfig.UseTxns = true;
            DatabaseEnvironment env =
                DatabaseEnvironment.Open(testHome, envConfig);

            // Get and confirm locking subsystem statistics.
            LockStats stats = env.LockingSystemStats();
	    env.Msgfile = testHome + "/" + testName+ ".log";
            env.PrintLockingSystemStats(true, true);
            Assert.AreEqual(0, stats.AllocatedLockers);
            Assert.AreNotEqual(0, stats.AllocatedLocks);
            Assert.AreNotEqual(0, stats.AllocatedObjects);
            Assert.AreEqual(0, stats.InitLockers);
            Assert.AreNotEqual(0, stats.InitLocks);
            Assert.AreNotEqual(0, stats.InitObjects);
            Assert.AreEqual(0, stats.LastAllocatedLockerID);
            Assert.AreEqual(0, stats.LockConflictsNoWait);
            Assert.AreEqual(0, stats.LockConflictsWait);
            Assert.AreEqual(0, stats.LockDeadlocks);
            Assert.AreEqual(0, stats.LockDowngrades);
            Assert.AreEqual(0, stats.LockerNoWait);
            Assert.AreEqual(0, stats.Lockers);
            Assert.AreEqual(0, stats.LockerWait);
            Assert.AreEqual(9, stats.LockModes);
            Assert.AreEqual(0, stats.LockPuts);
            Assert.AreEqual(0, stats.LockRequests);
            Assert.AreEqual(0, stats.Locks);
            Assert.AreEqual(0, stats.LockSteals);
            Assert.AreEqual(1000, stats.LockTimeoutLength);
            Assert.AreEqual(0, stats.LockTimeouts);
            Assert.AreEqual(0, stats.LockUpgrades);
            Assert.AreEqual(0, stats.MaxBucketLength);
            Assert.AreEqual(0, stats.MaxLockers);
            Assert.AreEqual(60, stats.MaxLockersInTable);
            Assert.AreEqual(0, stats.MaxLocks);
            Assert.AreEqual(0, stats.MaxLocksInBucket);
            Assert.AreEqual(50, stats.MaxLocksInTable);
            Assert.AreEqual(0, stats.MaxLockSteals);
            Assert.AreEqual(0, stats.MaxObjects);
            Assert.AreEqual(0, stats.MaxObjectsInBucket);
            Assert.AreEqual(70, stats.MaxObjectsInTable);
            Assert.AreEqual(0, stats.MaxObjectSteals);
            Assert.AreEqual(0, stats.MaxPartitionLockNoWait);
            Assert.AreEqual(0, stats.MaxPartitionLockWait);
            Assert.AreNotEqual(0, stats.MaxUnusedID);
            Assert.AreEqual(20, stats.nPartitions);
            Assert.AreEqual(0, stats.ObjectNoWait);
            Assert.AreEqual(0, stats.Objects);
            Assert.AreEqual(0, stats.ObjectSteals);
            Assert.AreEqual(0, stats.ObjectWait);
            Assert.LessOrEqual(0, stats.PartitionLockNoWait);
            Assert.AreEqual(0, stats.PartitionLockWait);
            Assert.Less(0, stats.RegionNoWait);
            Assert.AreNotEqual(0, stats.RegionSize);
            Assert.AreEqual(0, stats.RegionWait);
            Assert.AreNotEqual(0, stats.TableSize);
            Assert.AreEqual(2000, stats.TxnTimeoutLength);
            Assert.AreEqual(0, stats.TxnTimeouts);

            env.PrintLockingSystemStats();

            Transaction txn = env.BeginTransaction();
            BTreeDatabaseConfig dbConfig = new BTreeDatabaseConfig();
            dbConfig.Creation = CreatePolicy.IF_NEEDED;
            dbConfig.Env = env;
                dbConfig.FreeThreaded = true;
            BTreeDatabase db = BTreeDatabase.Open(
                testName + ".db", dbConfig, txn);
            txn.Commit();

            testLockStatsEnv = env;
            testLockStatsDb = db;

            // Use some locks, to ensure  the stats work when populated.
            txn = testLockStatsEnv.BeginTransaction();
            for (int i = 0; i < 500; i++)
            {
                testLockStatsDb.Put(
                    new DatabaseEntry(BitConverter.GetBytes(i)),
                    new DatabaseEntry(ASCIIEncoding.ASCII.GetBytes(
                    Configuration.RandomString(i))), txn);
                testLockStatsDb.Sync();
            }
            txn.Commit();

            env.PrintLockingSystemStats();
            stats = env.LockingSystemStats();
            Assert.Less(0, stats.LastAllocatedLockerID);
            Assert.Less(0, stats.LockDowngrades);
            Assert.LessOrEqual(0, stats.LockerNoWait);
            Assert.Less(0, stats.Lockers);
            Assert.LessOrEqual(0, stats.LockerWait);
            Assert.Less(0, stats.LockPuts);
            Assert.Less(0, stats.LockRequests);
            Assert.Less(0, stats.Locks);
            Assert.LessOrEqual(0, stats.LockSteals);
            Assert.LessOrEqual(0, stats.LockTimeouts);
            Assert.LessOrEqual(0, stats.LockUpgrades);
            Assert.Less(0, stats.MaxBucketLength);
            Assert.Less(0, stats.MaxLockers);
            Assert.Less(0, stats.MaxLocks);
            Assert.Less(0, stats.MaxLocksInBucket);
            Assert.LessOrEqual(0, stats.MaxLockSteals);
            Assert.Less(0, stats.MaxObjects);
            Assert.Less(0, stats.MaxObjectsInBucket);
            Assert.LessOrEqual(0, stats.MaxObjectSteals);
            Assert.LessOrEqual(0, stats.MaxPartitionLockNoWait);
            Assert.LessOrEqual(0, stats.MaxPartitionLockWait);
            Assert.Less(0, stats.MaxUnusedID);
            Assert.LessOrEqual(0, stats.ObjectNoWait);
            Assert.Less(0, stats.Objects);
            Assert.LessOrEqual(0, stats.ObjectSteals);
            Assert.LessOrEqual(0, stats.ObjectWait);
            Assert.Less(0, stats.PartitionLockNoWait);
            Assert.LessOrEqual(0, stats.PartitionLockWait);
            Assert.Less(0, stats.RegionNoWait);
            Assert.LessOrEqual(0, stats.RegionWait);
            Assert.LessOrEqual(0, stats.TxnTimeouts);

            // Force a deadlock to ensure the stats work.
            txn = testLockStatsEnv.BeginTransaction();
            testLockStatsDb.Put(new DatabaseEntry(BitConverter.GetBytes(10)),
                new DatabaseEntry(ASCIIEncoding.ASCII.GetBytes(
                Configuration.RandomString(200))), txn);

            Thread thread1 = new Thread(GenerateDeadlock);
            thread1.Start();
            while (DeadlockDidPut == 0)
                Thread.Sleep(10);

            try
            {
                testLockStatsDb.Get(new DatabaseEntry(
                    BitConverter.GetBytes(100)), txn);
            }
            catch (DeadlockException) { }
            // Abort unconditionally - we don't care about the transaction
            txn.Abort();
            thread1.Join();

            stats = env.LockingSystemStats();
            Assert.Less(0, stats.LockConflictsNoWait);
            Assert.LessOrEqual(0, stats.LockConflictsWait);

            db.Close(); 
            env.Close();
        }


        [Test]
        public void TestLockStatPrint()
        {
            testName = "TestLockStatPrint";
            SetUpTest(true);

            string[] messageInfo = new string[]
                { 
                  "Last allocated locker ID",
                  "Current maximum unused locker ID",
                  "Number of lock modes",
                  "Initial number of locks allocated",
                  "Initial number of lockers allocated",
                  "Initial number of lock objects allocated",
                  "Maximum number of locks possible",
                  "Maximum number of lockers possible",
                  "Maximum number of lock objects possible",
                  "Current number of locks allocated",
                  "Current number of lockers allocated",
                  "Current number of lock objects allocated",
                  "Number of lock object partitions",
                  "Size of object hash table",
                  "Number of current locks",
                  "Maximum number of locks at any one time",
                  "Maximum number of locks in any one bucket",
                  "Maximum number of locks stolen by for an empty partition",
                  "Maximum number of locks stolen for any one partition",
                  "Number of current lockers",
                  "Maximum number of lockers at any one time",
                  "Number of hits in the thread locker cache",
                  "Total number of lockers reused",
                  "Number of current lock objects",
                  "Maximum number of lock objects at any one time",
                  "Maximum number of lock objects in any one bucket",
                  "Maximum number of objects stolen by for an empty partition",
                  "Maximum number of objects stolen for any one partition",
                  "Total number of locks requested",
                  "Total number of locks released",
                  "Total number of locks upgraded",
                  "Total number of locks downgraded",
                  "Lock requests not available due to conflicts, for which we waited",
                  "Lock requests not available due to conflicts, for which we did not wait",
                  "Number of deadlocks",
                  "Lock timeout value",
                  "Number of locks that have timed out",
                  "Transaction timeout value",
                  "Number of transactions that have timed out",
                  "Region size",
                  "The number of partition locks that required waiting (0%)",
                  "The maximum number of times any partition lock was waited for (0%)",
                  "The number of object queue operations that required waiting (0%)",
                  "The number of locker allocations that required waiting (0%)",
                  "The number of region locks that required waiting (0%)",
                  "Maximum hash bucket length"
                };

            // Configure locking subsystem.
            LockingConfig lkConfig = new LockingConfig();
            lkConfig.MaxLockers = 60;
            lkConfig.MaxLocks = 50;
            lkConfig.MaxObjects = 70;
            lkConfig.Partitions = 20;
            lkConfig.DeadlockResolution = DeadlockPolicy.DEFAULT;

            // Configure and open an environment.
            DatabaseEnvironmentConfig envConfig =
                new DatabaseEnvironmentConfig();
            envConfig.Create = true;
            envConfig.LockSystemCfg = lkConfig;
            envConfig.LockTimeout = 1000;
            envConfig.NoLocking = false;
            envConfig.UseLocking = true;
            DatabaseEnvironment env =
                DatabaseEnvironment.Open(testHome, envConfig);

            // Confirm message file does not exist.
            string messageFile = testHome + "/" + "msgfile";
            Assert.AreEqual(false, File.Exists(messageFile));

            // Call set_msgfile() of env.
            env.Msgfile = messageFile;

            // Print env statistic to message file.
            env.PrintLockingSystemStats();

            // Confirm message file exists now.
            Assert.AreEqual(true, File.Exists(messageFile));

            env.Msgfile = "";
            int counter = 0;
            string line;
            line = null;

            // Read the message file line by line.
            System.IO.StreamReader file = new System.IO.StreamReader(@"" + messageFile);
            while ((line = file.ReadLine()) != null)
            {
                string[] tempStr = line.Split('\t');
                // Confirm the content of the message file.
                Assert.AreEqual(tempStr[1], messageInfo[counter]);
                counter++;
            }
            Assert.AreNotEqual(counter, 0);

            file.Close();
            env.Close();
        }

        public void GenerateDeadlock()
        {
            Transaction txn = testLockStatsEnv.BeginTransaction();
            try
            {
                testLockStatsDb.Put(
                    new DatabaseEntry(BitConverter.GetBytes(100)),
                    new DatabaseEntry(ASCIIEncoding.ASCII.GetBytes(
                    Configuration.RandomString(200))), txn);
                DeadlockDidPut = 1;
                testLockStatsDb.Get(new DatabaseEntry(
                BitConverter.GetBytes(10)), txn);

            }
            catch (DeadlockException) { }
            // Abort unconditionally - we don't care about the transaction
            txn.Abort();
        }

        public static void LockingEnvSetUp(string testHome,
            string testName, out DatabaseEnvironment env,
            uint maxLock, uint maxLocker, uint maxObject,
            uint partition) {
            // Configure env and locking subsystem.
            LockingConfig lkConfig = new LockingConfig();
            /*
             * If the maximum number of locks/lockers/objects
             * is given, then the LockingConfig is set. Unless, 
             * it is not set to any value.  
             */
            if (maxLock != 0)
                lkConfig.MaxLocks = maxLock;
            if (maxLocker != 0)
                lkConfig.MaxLockers = maxLocker;
            if (maxObject != 0)
                lkConfig.MaxObjects = maxObject;
            if (partition != 0)
                lkConfig.Partitions = partition;

            DatabaseEnvironmentConfig envConfig =
                new DatabaseEnvironmentConfig();
            envConfig.Create = true;
            envConfig.LockSystemCfg = lkConfig;
            envConfig.UseLocking = true;
            envConfig.ErrorPrefix = testName;

            env = DatabaseEnvironment.Open(testHome, envConfig);
        }
    }
}
