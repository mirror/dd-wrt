/*-
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 2009, 2017 Oracle and/or its affiliates.  All rights reserved.
 *
 */
using System;
using System.Collections;
using System.Collections.Generic;
using System.Diagnostics;
using System.IO;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading;
using System.Xml;
using NUnit.Framework;
using BerkeleyDB;

namespace CsharpAPITest
{
    class BackupWriter : IBackup {
        FileStream f;
        public int Close(string dbname) {
            f.Close();
            return 0;
        }
        public int Open(string dbname, string target) {
            string path = Path.Combine(target, dbname);
            f = new FileStream(path, FileMode.Create);
            return 0;
        }
        public int Write(byte[] data, long offset, int count) {
            f.Seek(offset, SeekOrigin.Begin);
            f.Write(data, 0, count);
            return 0;
        }
    }
	[TestFixture]
	public class DatabaseEnvironmentTest : CSharpTestFixture
	{

		private DatabaseEnvironment testBeginTransactionEnv;
		private BTreeDatabase testBeginTransactionDB;

		private DatabaseEnvironment testCheckpointEnv;
		private BTreeDatabase testCheckpointDB;

		private DatabaseEnvironment testDetectDeadlocksEnv;
		private BTreeDatabase testDetectDeadlocksDB;

		private DatabaseEnvironment testFailCheckEnv;

		private EventWaitHandle signal;

		[TestFixtureSetUp]
		public void SetUp()
		{
			testFixtureName = "DatabaseEnvironmentTest";
			base.SetUpTestfixture();
		}

		[Test]
		public void TestArchivableDatabaseFiles()
		{
			testName = "TestArchivableDatabaseFiles";
			SetUpTest(true);
			string dbFileName1 = testName + "1.db";
			string dbFileName2 = testName + "2.db";

			// Open an environment.
			DatabaseEnvironmentConfig envConfig =
			    new DatabaseEnvironmentConfig();
			envConfig.AutoCommit = true;
			envConfig.Create = true;
			envConfig.UseMPool = true;
			envConfig.UseLogging = true;
			envConfig.UseTxns = true;
			DatabaseEnvironment env = DatabaseEnvironment.Open(
			    testHome, envConfig);

			// Open two databases.
			BTreeDatabaseConfig dbConfig =
			    new BTreeDatabaseConfig();
			dbConfig.Creation = CreatePolicy.IF_NEEDED;
			dbConfig.Env = env;
			BTreeDatabase db1 = BTreeDatabase.Open(dbFileName1, dbConfig);
			db1.Close();
			BTreeDatabase db2 = BTreeDatabase.Open(dbFileName2, dbConfig);
			db2.Close();

			/*
			 * Get all database files name in the environment.
			 * Two database file name should be returned and
			 * the same as the ones when opening the databases.
			 */
			List<string> dbFiles = env.ArchivableDatabaseFiles(false);
			Assert.AreEqual(2, dbFiles.Count);
			Assert.IsTrue(dbFiles.Contains(dbFileName1));
			Assert.IsTrue(dbFiles.Contains(dbFileName2));

			/*
			 * Get all database file's abosolute path in the
			 * environment. Confirm those files exist.
			 */
			List<string> dbFilesPath = env.ArchivableDatabaseFiles(true);
			Assert.IsTrue(File.Exists(dbFilesPath[0]));
			Assert.IsTrue(File.Exists(dbFilesPath[1]));

			env.Close();
		}

		[Test]
		public void TestArchivableLogFiles()
		{
			testName = "TestArchivableLogFiles";
			SetUpTest(true);
			string dbFileName = testName + ".db";

			// Open an environment.
			DatabaseEnvironmentConfig envConfig =
			    new DatabaseEnvironmentConfig();
			envConfig.AutoCommit = true;
			envConfig.Create = true;
			envConfig.UseMPool = true;
			envConfig.UseLogging = true;
			envConfig.UseTxns = true;
			DatabaseEnvironment env = DatabaseEnvironment.Open(
			    testHome, envConfig);

			// Open a databases.
			BTreeDatabaseConfig dbConfig =
			    new BTreeDatabaseConfig();
			dbConfig.Creation = CreatePolicy.IF_NEEDED;
			dbConfig.Env = env;
			BTreeDatabase db = BTreeDatabase.Open(
			    dbFileName, dbConfig);

			/*
			 * Put 1000 records into the database to generate
			 * more than one log files.
			 */
			byte[] byteArr = new byte[1024];
			for (int i = 0; i < 1000; i++)
				db.Put(new DatabaseEntry(
				    BitConverter.GetBytes(i)),
				    new DatabaseEntry(byteArr));

			db.Close();

			List<string> logFiles = env.ArchivableLogFiles(false);

			List<string> logFilesPath =
			    env.ArchivableLogFiles(true);
			for (int i = 0; i < logFilesPath.Count; i++)
				Assert.IsTrue(File.Exists(logFilesPath[i]));

			env.Close();
		}

        [Test]
        public void TestBackup() {
            testName = "TestBackup";
            SetUpTest(true);
            string dbFileName = testName + ".db";

            // Open an environment.
            DatabaseEnvironmentConfig envConfig =
                new DatabaseEnvironmentConfig();
            envConfig.AutoCommit = true;
            envConfig.Create = true;
            envConfig.UseMPool = true;
            envConfig.UseLogging = true;
            envConfig.UseTxns = true;
            DatabaseEnvironment env = DatabaseEnvironment.Open(
                testHome, envConfig);

            // Open a databases.
            BTreeDatabaseConfig dbConfig =
                new BTreeDatabaseConfig();
            dbConfig.Creation = CreatePolicy.IF_NEEDED;
            dbConfig.Env = env;
            BTreeDatabase db = BTreeDatabase.Open(
                dbFileName, dbConfig);

            /*
             * Put 1000 records into the database to generate
             * more than one log file.
             */
            byte[] byteArr = new byte[1024];
            for (int i = 0; i < 1000; i++)
                db.Put(new DatabaseEntry(
                    BitConverter.GetBytes(i)),
                    new DatabaseEntry(byteArr));

            db.Close();

            /* Set up the target directory for the backup. */
            string target = testFixtureHome + "/" + testName + "_backup";
            Configuration.ClearDir(target);
            
            BackupOptions opt = new BackupOptions();
            opt.Creation = CreatePolicy.ALWAYS;
            env.Backup(target, opt);
            Assert.Greater(Directory.GetFiles(target).Length, 0);
            Directory.Delete(target, true);
            env.Close();
        }

        [Test]
        public void TestBackupCallbacks() {
            testName = "TestBackupCallbacks";
            SetUpTest(true);
            string dbFileName = testName + ".db";

            // Open an environment.
            DatabaseEnvironmentConfig envConfig =
                new DatabaseEnvironmentConfig();
            envConfig.AutoCommit = true;
            envConfig.Create = true;
            envConfig.UseMPool = true;
            envConfig.UseLogging = true;
            envConfig.UseTxns = true;
            DatabaseEnvironment env = DatabaseEnvironment.Open(
                testHome, envConfig);

            // Open a databases.
            BTreeDatabaseConfig dbConfig =
                new BTreeDatabaseConfig();
            dbConfig.Creation = CreatePolicy.IF_NEEDED;
            dbConfig.Env = env;
            BTreeDatabase db = BTreeDatabase.Open(
                dbFileName, dbConfig);

            /*
             * Populate the database but keep it small, we're going to do a 
             * byte-by-byte comparison at the end.
             */
            byte[] byteArr = new byte[1024];
            for (int i = 0; i < 10; i++)
                db.Put(new DatabaseEntry(
                    BitConverter.GetBytes(i)),
                    new DatabaseEntry(byteArr));

            db.Close();

            /* Set up the target directory for the backup. */
            string target = testFixtureHome + "/" + testName + "_backup";
            Configuration.ClearDir(target);
            
            BackupWriter backup = new BackupWriter();
            env.BackupHandler = backup;

            BackupOptions opt = new BackupOptions();
            opt.Creation = CreatePolicy.ALWAYS;
            env.Backup(target, opt);
            Assert.Greater(Directory.GetFiles(target).Length, 0);
            
            /* Check that our callbacks wrote the file correctly. */
            FileStream orig = new FileStream(Path.Combine(testHome, dbFileName), FileMode.Open, FileAccess.Read);
            FileStream bak = new FileStream(Path.Combine(target, dbFileName), FileMode.Open, FileAccess.Read);
            int orig_byte, bak_byte;
            do {
                orig_byte = orig.ReadByte();
                bak_byte = bak.ReadByte();
                Assert.AreEqual(orig_byte, bak_byte);
            } while (orig_byte != -1 && bak_byte != -1);
            orig.Close();
            bak.Close();
            Directory.Delete(target, true);
            env.Close();
        }

        [Test]
        public void TestBackupDatabase() {
            testName = "TestBackupDatabase";
            SetUpTest(true);
            string dbFileName = testName + ".db";

            // Open an environment.
            DatabaseEnvironmentConfig envConfig =
                new DatabaseEnvironmentConfig();
            envConfig.AutoCommit = true;
            envConfig.Create = true;
            envConfig.UseMPool = true;
            envConfig.UseLogging = true;
            envConfig.UseTxns = true;
            DatabaseEnvironment env = DatabaseEnvironment.Open(
                testHome, envConfig);

            // Open a databases.
            BTreeDatabaseConfig dbConfig =
                new BTreeDatabaseConfig();
            dbConfig.Creation = CreatePolicy.IF_NEEDED;
            dbConfig.Env = env;
            BTreeDatabase db = BTreeDatabase.Open(
                dbFileName, dbConfig);
            
            /*
             * Put 1000 records into the database to generate
             * more than one log file.
             */
            byte[] byteArr = new byte[1024];
            for (int i = 0; i < 1000; i++)
                db.Put(new DatabaseEntry(
                    BitConverter.GetBytes(i)),
                    new DatabaseEntry(byteArr));
            
            db.Close();

            /* Set up the target directory for the backup. */
            string target = testFixtureHome + "/" + testName + "_backup";
            Configuration.ClearDir(target);
            
            /* We should only copy one file, the database. */
            env.BackupDatabase(target, dbFileName, true);
            Assert.AreEqual(1, Directory.GetFiles(target).Length);
            
            Directory.Delete(target, true);
            env.Close();
        }

        [Test]
        public void TestBackupOptions() {
            testName = "TestBackupOptions";
            SetUpTest(true);
            
            // Open an environment.
            DatabaseEnvironmentConfig envConfig =
                new DatabaseEnvironmentConfig();
            envConfig.AutoCommit = true;
            envConfig.Create = true;
            envConfig.UseMPool = true;
            envConfig.UseLogging = true;
            envConfig.UseTxns = true;
            DatabaseEnvironment env = DatabaseEnvironment.Open(
                testHome, envConfig);

            /* 
             * Check that we can get and set backup options successfully.  There
             * are other tests to check that the backup options are obeyed.
             */
            env.BackupBufferSize = (uint)1024;
            Assert.AreEqual((uint)1024, env.BackupBufferSize);
            
            env.BackupReadCount = (uint)4096;
            Assert.AreEqual((uint)4096, env.BackupReadCount);
            
            env.BackupReadSleepDuration = (uint)1000;
            Assert.AreEqual((uint)1000, env.BackupReadSleepDuration);
            
            env.BackupWriteDirect = true;
            Assert.IsTrue(env.BackupWriteDirect);
        }

		[Test]
		public void TestBeginCDSGroup()
		{
			testName = "TestBeginCDSGroup";
			SetUpTest(true);

			DatabaseEnvironmentConfig cfg =
			    new DatabaseEnvironmentConfig();
			cfg.Create = true;
			cfg.UseCDB = true;
			cfg.UseMPool = true;
			DatabaseEnvironment env = DatabaseEnvironment.Open(testHome, cfg);
			Transaction txn = env.BeginCDSGroup();
			BTreeDatabaseConfig dbConfig = new BTreeDatabaseConfig();
			dbConfig.Creation = CreatePolicy.IF_NEEDED;
			dbConfig.Env = env;
			BTreeDatabase db = BTreeDatabase.Open(
			    testName + ".db", dbConfig, txn);
			db.Put(new DatabaseEntry(
			    ASCIIEncoding.ASCII.GetBytes("key")),
			    new DatabaseEntry(
			    ASCIIEncoding.ASCII.GetBytes("data")), txn);
			db.Close();
			txn.Commit();
			env.Close();
		}

		[Test]
		public void TestBeginTransaction()
		{
			testName = "TestBeginTransaction";
			SetUpTest(true);

			// Open an environment.
			DatabaseEnvironmentConfig cfg =
			    new DatabaseEnvironmentConfig();
			cfg.Create = true;
			cfg.UseTxns = true;
			cfg.UseMPool = true;
			cfg.UseLogging = true;
			cfg.UseLocking = true;
			cfg.NoLocking = false;
			cfg.FreeThreaded = true;
			testBeginTransactionEnv = DatabaseEnvironment.Open(testHome, cfg);
			testBeginTransactionEnv.DeadlockResolution = DeadlockPolicy.OLDEST;

			// Open btree database.
			BTreeDatabaseConfig dbConfig = new BTreeDatabaseConfig();
			dbConfig.AutoCommit = true;
			dbConfig.Creation = CreatePolicy.IF_NEEDED;
			dbConfig.Env = testBeginTransactionEnv;
			dbConfig.Duplicates = DuplicatesPolicy.NONE;
			dbConfig.FreeThreaded = true;
			testBeginTransactionDB = BTreeDatabase.Open(
			    testName + ".db", dbConfig);

			testBeginTransactionDB.Put(
			    new DatabaseEntry(ASCIIEncoding.ASCII.GetBytes("key")),
			    new DatabaseEntry(ASCIIEncoding.ASCII.GetBytes("data")));

			// Begin two threads to run dead lock detection.
			Thread thread1 = new Thread(new ThreadStart(
			    DeadLockThreadWithLockTimeOut));
			Thread thread2 = new Thread(new ThreadStart(
			    DeadLockThreadWithTxnTimeout));
			signal = new EventWaitHandle(false, EventResetMode.ManualReset);
			thread1.Start();
			thread2.Start();
			Thread.Sleep(1000);
			signal.Set();
			thread1.Join();
			thread2.Join();

			// Close all.
			testBeginTransactionDB.Close();
			testBeginTransactionEnv.Close();
		}

		public void DeadLockThreadWithLockTimeOut()
		{
			// Configure and begin a transaction.
			TransactionConfig txnConfig = new TransactionConfig();
			txnConfig.LockTimeout = 5000;
			txnConfig.Name = "DeadLockThreadWithLockTimeOut";
			Transaction txn = 
			    testBeginTransactionEnv.BeginTransaction(txnConfig, null);
			try
			{
				testBeginTransactionDB.Put(
				    new DatabaseEntry(ASCIIEncoding.ASCII.GetBytes("key")),
				    new DatabaseEntry(ASCIIEncoding.ASCII.GetBytes("data")));
				signal.WaitOne();
				testBeginTransactionDB.Put(
				    new DatabaseEntry(ASCIIEncoding.ASCII.GetBytes("newkey")),
				    new DatabaseEntry(ASCIIEncoding.ASCII.GetBytes("newdata")),
				    txn);
				txn.Commit();
			}
			catch (DeadlockException)
			{
				try
				{
					txn.Abort();
				}
				catch (DatabaseException)
				{
					throw new TestException();
				}
			}
			catch (DatabaseException)
			{
				try
				{
					txn.Abort();
				}
				catch (DatabaseException)
				{
					throw new TestException();
				}
			}
		}

		public void DeadLockThreadWithTxnTimeout()
		{
			// Configure and begin a transaction.
			TransactionConfig txnConfig = new TransactionConfig();
			txnConfig.TxnTimeout = 5000;
			txnConfig.Name = "DeadLockThreadWithTxnTimeout";
			Transaction txn =
			    testBeginTransactionEnv.BeginTransaction(txnConfig, null);
			try
			{
				testBeginTransactionDB.Put(
				    new DatabaseEntry(ASCIIEncoding.ASCII.GetBytes("key")),
				    new DatabaseEntry(ASCIIEncoding.ASCII.GetBytes("data")));
				signal.WaitOne();
				testBeginTransactionDB.Put(
				    new DatabaseEntry(ASCIIEncoding.ASCII.GetBytes("newkey")),
				    new DatabaseEntry(ASCIIEncoding.ASCII.GetBytes("newdata")),
				    txn);
				txn.Commit();
			}
			catch (DeadlockException)
			{
				try
				{
					txn.Abort();
				}
				catch (DatabaseException)
				{
					throw new TestException();
				}
			}
			catch (DatabaseException)
			{
				try
				{
					txn.Abort();
				}
				catch (DatabaseException)
				{
					throw new TestException();
				}
			}
		}

		[Test]
		public void TestBlob()
		{
			testName = "TestBlob";
			SetUpTest(true);

			DatabaseEnvironmentConfig envConfig =
			    new DatabaseEnvironmentConfig();
			envConfig.AutoCommit = true;
			envConfig.Create = true;
			envConfig.UseMPool = true;
			envConfig.UseLogging = true;
			envConfig.UseTxns = true;

			// Not set the external file directory when enabling
			// external file.
			envConfig.ExternalFileThreshold = 10485760;
			DatabaseEnvironment env = DatabaseEnvironment.Open(
			    testHome, envConfig);
			Assert.AreEqual(null, env.ExternalFileDir);
			Assert.AreEqual(10485760, env.ExternalFileThreshold);
			env.Close();

			Configuration.ClearDir(testHome);

			// Set the external file directory with an empty string.
			envConfig.ExternalFileDir = "";
			env = DatabaseEnvironment.Open(testHome, envConfig);
			Assert.AreEqual("", env.ExternalFileDir);
			Assert.AreEqual(10485760, env.ExternalFileThreshold);
			env.Close();

			Configuration.ClearDir(testHome);

			// Set the external file directory with a non-emptry
			// string.
			envConfig.ExternalFileDir = "BLOBDIR";
			env = DatabaseEnvironment.Open(testHome, envConfig);
			Assert.AreEqual("BLOBDIR", env.ExternalFileDir);
			Assert.AreEqual(10485760, env.ExternalFileThreshold);
			env.Close();
		}

		[Test]
		public void TestCheckpoint()
		{
			testName = "TestCheckpoint";
			SetUpTest(true);

			// Open an environment.
			DatabaseEnvironmentConfig cfg =
			    new DatabaseEnvironmentConfig();
			cfg.Create = true;
			cfg.UseTxns = true;
			cfg.UseMPool = true;
			cfg.UseLogging = true;
			cfg.UseLocking = true;
			cfg.NoLocking = false;
			cfg.FreeThreaded = true;
			testCheckpointEnv = DatabaseEnvironment.Open(testHome, cfg);

			// Open btree database.
			BTreeDatabaseConfig dbConfig = new BTreeDatabaseConfig();
			dbConfig.AutoCommit = true;
			dbConfig.Creation = CreatePolicy.IF_NEEDED;
			dbConfig.Env = testCheckpointEnv;
			dbConfig.FreeThreaded = true;
			testCheckpointDB = BTreeDatabase.Open(testName + ".db", dbConfig);

			// Run a thread to put records into database.
			Thread thread1 = new Thread(new ThreadStart(PutRecordsThread));

			/*
			 * Run a thread to do checkpoint periodically and
			 * finally do a checkpoint to flush all in memory pool
			 * to log files.
			 */
			Thread thread2 = new Thread(new ThreadStart(CheckpointThread));

			thread1.Start();
			thread2.Start();
			thread1.Join();
			thread2.Join();

			// Close all.
			testCheckpointDB.Close();
			testCheckpointEnv.Close();
		}

		public void PutRecordsThread()
		{
			Transaction txn = testCheckpointEnv.BeginTransaction();
			byte[] byteArr = new byte[1024];
			for (int i = 0; i < 1000; i++)
				testCheckpointDB.Put(
				    new DatabaseEntry(BitConverter.GetBytes(i)),
				    new DatabaseEntry(byteArr), txn);
			txn.Commit();
		}

		public void CheckpointThread()
		{
			uint bytes = 64;
			uint minutes = 1;
			uint count = 1;
			while (count < 3)
			{
				testCheckpointEnv.Checkpoint(bytes, minutes);
				count++;
			}
			Thread.Sleep(500);
			testCheckpointEnv.Checkpoint();
		}

		[Test]
		public void TestClose()
		{
			testName = "TestClose";
			SetUpTest(true);

			DatabaseEnvironmentConfig cfg =
			    new DatabaseEnvironmentConfig();
			cfg.Create = true;
			DatabaseEnvironment env = DatabaseEnvironment.Open(
			    testHome, cfg);
			env.Close();
		}

		[Test]
		public void TestConfigAll()
		{
			testName = "TestConfigAll";
			SetUpTest(true);

			XmlElement xmlElem = Configuration.TestSetUp(
			    testFixtureName, testName);

			/*
			 * Open a new environment with all properties,
			 * fields and subsystems configured.
			 */
			DatabaseEnvironmentConfig envConig =
			    new DatabaseEnvironmentConfig();
			Config(xmlElem, ref envConig, true, true, true,
			    true, true, true);

			// Configure with methods.
			ReplicationHostAddress address =
			    new ReplicationHostAddress("::1:11111");
			envConig.RepSystemCfg.Clockskew(102, 100);
			envConig.RepSystemCfg.RetransmissionRequest(10, 100);
			envConig.RepSystemCfg.TransmitLimit(1, 1024);

			// Open the environment.
			DatabaseEnvironment env = DatabaseEnvironment.Open(
			    testHome, envConig);

			// Confirm environment status with its configuration.
			Confirm(xmlElem, env, true, true, true, true, true, true);

			// Print statistics of the current environment.
			env.Msgfile = testHome + "/" + testName+ ".log";
			env.PrintStats(true, true);

			// Print statistics of all subsytems.
			env.PrintSubsystemStats(true, true);

			env.Close();
		}

		[Test]
		public void TestDeadlockPolicy()
		{
			testName = "TestDeadlockPolicy";
			SetUpTest(true);

			DetectDeadlockPolicy(testHome + "_DEFAULT",
			    DeadlockPolicy.DEFAULT);

			DetectDeadlockPolicy(testHome + "_EXPIRE",
			    DeadlockPolicy.EXPIRE);
			DetectDeadlockPolicy(testHome + "_MAX_LOCKS",
			    DeadlockPolicy.MAX_LOCKS);
			DetectDeadlockPolicy(testHome + "_MAX_WRITE",
			    DeadlockPolicy.MAX_WRITE);
			DetectDeadlockPolicy(testHome + "_MIN_LOCKS",
			    DeadlockPolicy.MIN_LOCKS);
			DetectDeadlockPolicy(testHome + "_MIN_WRITE",
			    DeadlockPolicy.MIN_WRITE);
			DetectDeadlockPolicy(testHome + "_OLDEST",
			    DeadlockPolicy.OLDEST);
			DetectDeadlockPolicy(testHome + "_RANDOM",
			    DeadlockPolicy.RANDOM);
			DetectDeadlockPolicy(testHome + "_YOUNGEST",
			    DeadlockPolicy.YOUNGEST);
		}

		public void DetectDeadlockPolicy(
		    string home, DeadlockPolicy deadlock)
		{
			Configuration.ClearDir(home);
			DatabaseEnvironmentConfig envConfig =
			    new DatabaseEnvironmentConfig();
			envConfig.Create = true;
			envConfig.UseLocking = true;
			envConfig.UseLogging = true;
			envConfig.UseMPool = true;
			envConfig.UseTxns = true;
			DatabaseEnvironment env = DatabaseEnvironment.Open(
			    home, envConfig);
			env.DeadlockResolution = deadlock;
			Assert.AreEqual(deadlock, env.DeadlockResolution);
			env.DetectDeadlocks(deadlock);
			env.Close();
		}

		[Test]
		public void TestDetectDeadlocks()
		{
			testName = "TestDetectDeadlocks";
			SetUpTest(true);

			// Open an environment.
			DatabaseEnvironmentConfig cfg =
			    new DatabaseEnvironmentConfig();
			cfg.Create = true;
			cfg.UseTxns = true;
			cfg.UseMPool = true;
			cfg.UseLogging = true;
			cfg.UseLocking = true;
			cfg.FreeThreaded = true;
			testDetectDeadlocksEnv = DatabaseEnvironment.Open(
			    testHome, cfg);

			// Open btree database.
			BTreeDatabaseConfig dbConfig = new BTreeDatabaseConfig();
			dbConfig.AutoCommit = true;
			dbConfig.Creation = CreatePolicy.IF_NEEDED;
			dbConfig.Env = testDetectDeadlocksEnv;
			dbConfig.Duplicates = DuplicatesPolicy.NONE;
			dbConfig.FreeThreaded = true;
			testDetectDeadlocksDB = BTreeDatabase.Open(
			    testName + ".db", dbConfig);

			// Put one record("key", "data") into database.
			testDetectDeadlocksDB.Put(
			    new DatabaseEntry(ASCIIEncoding.ASCII.GetBytes("key")),
			    new DatabaseEntry(ASCIIEncoding.ASCII.GetBytes("data")));

			// Begin two threads to read and write record.
			Thread thread1 = new Thread(new ThreadStart(ReadAndPutRecordThread));
			Thread thread2 = new Thread(new ThreadStart(ReadAndPutRecordThread));
			signal = new EventWaitHandle(false, EventResetMode.ManualReset);
			thread1.Start();
			thread2.Start();

			// Give enough time for threads to read record.
			Thread.Sleep(1000);

			/*
			 * Let the two threads apply for write lock
			 * synchronously.
			 */
			signal.Set();

			// Confirm that there is deadlock in the environment.
			Thread.Sleep(1000);
			uint deadlockNum = testDetectDeadlocksEnv.DetectDeadlocks(
			    DeadlockPolicy.DEFAULT);
			Assert.Less(0, deadlockNum);

			thread1.Join();
			thread2.Join();

			// Close all.
			testDetectDeadlocksDB.Close(false);
			testDetectDeadlocksEnv.Close();
		}

		public void ReadAndPutRecordThread()
		{
			Transaction txn =
			    testDetectDeadlocksEnv.BeginTransaction();
			try
			{
				testDetectDeadlocksDB.GetBoth(
				    new DatabaseEntry(ASCIIEncoding.ASCII.GetBytes("key")),
				    new DatabaseEntry(ASCIIEncoding.ASCII.GetBytes("data")), txn);
				signal.WaitOne();
				testDetectDeadlocksDB.Put(
				    new DatabaseEntry(ASCIIEncoding.ASCII.GetBytes("newKey")),
				    new DatabaseEntry(ASCIIEncoding.ASCII.GetBytes("newData")),
				    txn);
				txn.Commit();
			}
			catch (DeadlockException)
			{
				txn.Abort();
			}
		}

		[Test]
		public void TestEncryption() 
		{
			testName = "TestEncryption";
			SetUpTest(true);

			// Open an encrypted environment.
			DatabaseEnvironmentConfig envCfg =
			    new DatabaseEnvironmentConfig();
			envCfg.Create = true;
			envCfg.UseMPool = true;
			envCfg.UseLogging = true;
			envCfg.UseTxns = true;
			envCfg.SetEncryption("bdb", EncryptionAlgorithm.AES);
			DatabaseEnvironment env =
			DatabaseEnvironment.Open(testHome, envCfg);
			Transaction txn = env.BeginTransaction();

			// Open a non-encrypted database in the environment.
			BTreeDatabaseConfig dbCfg =
			    new BTreeDatabaseConfig();
			dbCfg.Creation = CreatePolicy.IF_NEEDED;
			dbCfg.Env = env;
			BTreeDatabase db1 = BTreeDatabase.Open(
			    testName + "1.db", dbCfg, txn);
			Assert.IsFalse(db1.Encrypted);
			for (int i = 0; i < 10; i++)
				db1.Put(new DatabaseEntry(
				    BitConverter.GetBytes(i)),
				    new DatabaseEntry(
				    BitConverter.GetBytes(i)), txn);

			// Open an encrypted database in the environment.
			dbCfg.Encrypted = true;
			BTreeDatabase db2 = BTreeDatabase.Open(
			    testName + "2.db", dbCfg, txn);
			Assert.IsTrue(db2.Encrypted);
			for (int i = 0; i < 10; i++)
			db2.Put(new DatabaseEntry(BitConverter.GetBytes(i)),
			    new DatabaseEntry(BitConverter.GetBytes(i)), txn);

			// Close all.
			txn.Commit();
			db1.Close();
			db2.Close();
			env.Close();

			// Verify the environment is encrypted.
			DatabaseEnvironmentConfig verifyEnvCfg =
			    new DatabaseEnvironmentConfig();
			verifyEnvCfg.Create = true;
			verifyEnvCfg.UseMPool = true;
			verifyEnvCfg.UseLogging = true;
			verifyEnvCfg.UseTxns = true;
			try {
				env = DatabaseEnvironment.Open(
				    testHome, verifyEnvCfg);
				// Do nothing
				env.Close();
				throw new TestException();
			} catch (DatabaseException) {
			}

			// Verify the non-encrypted database is not encrypted.
			BTreeDatabaseConfig verifyDbCfg =
			    new BTreeDatabaseConfig();
			verifyDbCfg.Creation = CreatePolicy.IF_NEEDED;

			using (db1 = BTreeDatabase.Open(
			    testHome + "/" + testName + "1.db", verifyDbCfg)) {
				for (int i = 0; i < 10; i++)
					db1.Get(new DatabaseEntry(
					    BitConverter.GetBytes(i)));
			};

			/// Verify the encrypted database is encrypted.
			verifyDbCfg.SetEncryption(envCfg.EncryptionPassword,
			    envCfg.EncryptAlgorithm);
			verifyDbCfg.Encrypted = true;
			using (db1 = BTreeDatabase.Open(
			    testHome + "/" + testName + "2.db",
			    verifyDbCfg)) {
				for (int i = 0; i < 10; i++)
					db1.Get(new DatabaseEntry(
					    BitConverter.GetBytes(i)));
			};
		}

		[Test]
		public void TestFailCheck()
		{
			testName = "TestFailCheck";
			SetUpTest(true);

			DatabaseEnvironmentConfig cfg =
			    new DatabaseEnvironmentConfig();
			cfg.Create = true;
			cfg.UseTxns = true;
			cfg.UseMPool = true;
			cfg.UseLogging = true;
			cfg.UseLocking = true;
			cfg.FreeThreaded = true;
			cfg.ThreadIsAlive = new ThreadIsAliveDelegate(ThrdAlive);
			cfg.SetThreadID = new SetThreadIDDelegate(SetThrdID);
			cfg.ThreadCount = 10;
			testFailCheckEnv = DatabaseEnvironment.Open(testHome, cfg);

			Thread thread = new Thread(new ThreadStart(WriteThreadWithoutTxnCommit));
			thread.Start();
			thread.Join();
			testFailCheckEnv.FailCheck();
			testFailCheckEnv.Close();
		}

		public void WriteThreadWithoutTxnCommit()
		{
			Transaction txn = testFailCheckEnv.BeginTransaction();
			BTreeDatabaseConfig dbConfig = new BTreeDatabaseConfig();
			dbConfig.Creation = CreatePolicy.IF_NEEDED;
			dbConfig.Env = testFailCheckEnv;
			BTreeDatabase db = BTreeDatabase.Open("TestFailCheck.db", dbConfig, txn);
			db.Close();
			txn.Commit();
		}

		public bool ThrdAlive(DbThreadID info, bool procOnly)
		{
			Process pcs = Process.GetProcessById(info.processID);
			if (pcs.HasExited == true)
				return false;
			else if (procOnly)
				return true;
			ProcessThreadCollection thrds = pcs.Threads;
			foreach (ProcessThread pcsThrd in thrds)
			{
				if (pcsThrd.Id == info.threadID)
				{
					/*
					 * We have to use the fully qualified name, ThreadState
					 * defaults to System.Threading.ThreadState.
					 */
					return (pcsThrd.ThreadState !=
						System.Diagnostics.ThreadState.Terminated);
				}
			}
			// If we can't find the thread, we say it's not alive
			return false;
		}

		[DllImport("kernel32.dll")]
		static extern uint GetCurrentThreadId();
		public DbThreadID SetThrdID()
		{
			DbThreadID threadID;

			int pid = Process.GetCurrentProcess().Id;
			uint tid = GetCurrentThreadId();
			threadID = new DbThreadID(pid, tid);
			return threadID;
		}

		[Test]
		public void TestFeedback()
		{
			testName = "TestFeedback";
			SetUpTest(true);

			// Open the environment.
			DatabaseEnvironmentConfig cfg =
			    new DatabaseEnvironmentConfig();
			cfg.AutoCommit = true;
			cfg.UseLocking = true;
			cfg.UseLogging = true;
			cfg.UseMPool = true;
			cfg.UseTxns = true;
			cfg.Create = true;
			DatabaseEnvironment env = DatabaseEnvironment.Open(testHome, cfg);

			env.Feedback = new EnvironmentFeedbackDelegate(
			    EnvRecovery10PercentFeedback);
			env.Feedback(EnvironmentFeedbackEvent.RECOVERY, 10);

			env.Close();
		}

		public void EnvRecovery10PercentFeedback(
		    EnvironmentFeedbackEvent opcode, int percent)
		{
			Assert.AreEqual(opcode, EnvironmentFeedbackEvent.RECOVERY);
			Assert.AreEqual(10, percent);
		}

		[Test]
		public void TestMessageCall()
		{
			testName = "TestMessageCall";
			SetUpTest(true);

			// Configure and open an environment.
			DatabaseEnvironmentConfig envConfig =
			    new DatabaseEnvironmentConfig();
			envConfig.Create = true;
			envConfig.UseMPool = true;
			DatabaseEnvironment env =
			    DatabaseEnvironment.Open(testHome, envConfig);

			// Confirm message file does not exist.
			string messageCallFile = testHome + "/" + "MessageCallFile";
			Assert.AreEqual(false, File.Exists(messageCallFile));

			string messageInfo = "Message come from db.set_msgcall!";

			// Call set_msgcall() of env.
			env.messageFeedback = new MessageFeedbackDelegate(Msgcall_fcn);
			env.messageFeedback(null, messageInfo);

			// Unconfigures the callback interface.
			env.messageFeedback = null;

			// Confirm message file exists now.
			Assert.AreEqual(true, File.Exists(messageCallFile));

			// Read the first line of message file.
			string line = null;
			System.IO.StreamReader file = new System.IO.StreamReader(@"" + messageCallFile);
			line = file.ReadLine();

			// Confirm the message file is not empty.
			Assert.AreEqual(line, messageInfo);

			file.Close();
			env.Close();
		}

		public void Msgcall_fcn(string msgpfx, string message)
		{
			string msgfile = testHome + "/" + "MessageCallFile";
			FileStream fs = new FileStream(msgfile, FileMode.OpenOrCreate);
			StreamWriter sw = new StreamWriter(fs);
			if (msgpfx != null)
				sw.Write(msgpfx + ": ");
			sw.Write(message);
			sw.Flush();
			sw.Close();
			fs.Close();
		}

		[Test]
		public void TestMessageFile()
		{
			testName = "TestMessageFile";
			SetUpTest(true);

			// Configure and open an environment.
			DatabaseEnvironmentConfig envConfig =
			    new DatabaseEnvironmentConfig();
			envConfig.Create = true;
			envConfig.UseMPool = true;
			DatabaseEnvironment env =
			    DatabaseEnvironment.Open(testHome, envConfig);

			// Confirm message file does not exist.
			string messageFile = testHome + "/" + "msgfile";
			Assert.AreEqual(false, File.Exists(messageFile));

			// Call set_msgfile() of db.
			env.Msgfile = messageFile;

			// Print db statistic to message file.
			env.PrintStats(true, true);

			// Confirm message file exists now.
			Assert.AreEqual(true, File.Exists(messageFile));

			env.Msgfile = "";
			string line = null;

			// Read the third line of message file.
			System.IO.StreamReader file = new System.IO.StreamReader(@"" + messageFile);
			line = file.ReadLine();
			line = file.ReadLine();
			line = file.ReadLine();

			// Confirm the message file is not empty.
			Assert.AreEqual(line, "Default database environment information:");

			file.Close();
			env.Close();
		}

		[Test]
		public void TestMetadataDir()
		{
			testName = "TestMetadataDir";
			SetUpTest(true);

			DatabaseEnvironmentConfig cfg =
			    new DatabaseEnvironmentConfig();
			cfg.Create = true;
			cfg.UseLocking = true;
			cfg.UseLogging = true;
			cfg.UseMPool = true;
			cfg.UseReplication = true;
			cfg.UseTxns = true;

			// Metadata Directory defaults to NULL
			DatabaseEnvironment env =
			    DatabaseEnvironment.Open(testHome, cfg);
			Assert.Null(env.MetadataDir);
			env.Close();
			int cnt = Directory.GetFiles(
			    testHome, "__db.rep.system*").Length +
			    Directory.GetFiles(
			    testHome, "__db.rep.gen*").Length +
			    Directory.GetFiles(
			    testHome, "__db.rep.egen*").Length +
			    Directory.GetFiles(
			    testHome, "__db.rep.init*").Length;
			Assert.Less(0, cnt);

			// Set Metadata Directory
			Configuration.ClearDir(testHome);
			string md = testHome + "/" + "Meta";
			Directory.CreateDirectory(md);
			cfg.MetadataDir = "Meta";
			env = DatabaseEnvironment.Open(testHome, cfg);
			Assert.AreEqual("Meta", env.MetadataDir);
			env.Close();
			cnt = Directory.GetFiles(
			    testHome, "__db.rep.system*").Length +
			    Directory.GetFiles(
			    testHome, "__db.rep.gen*").Length +
			    Directory.GetFiles(
			    testHome, "__db.rep.egen*").Length +
			    Directory.GetFiles(
			    testHome, "__db.rep.init*").Length;
			Assert.AreEqual(0, cnt);
			cnt =
			    Directory.GetFiles(md, "__db.rep.system*").Length +
			    Directory.GetFiles(md, "__db.rep.gen*").Length +
			    Directory.GetFiles(md, "__db.rep.egen*").Length +
			    Directory.GetFiles(md, "__db.rep.init*").Length;
			Assert.Less(0, cnt);
		}

		[Test]
		public void TestMutexSystemStats()
		{
			testName = "TestMutexSystemStats";
			SetUpTest(true);

			DatabaseEnvironmentConfig cfg =
			    new DatabaseEnvironmentConfig();
			cfg.Create = true;
			cfg.UseLogging = true;
			cfg.UseLocking = true;
			cfg.UseMPool = true;
			cfg.UseTxns = true;
			cfg.MutexSystemCfg = new MutexConfig();
			cfg.MutexSystemCfg.Alignment = 512;
			cfg.MutexSystemCfg.Increment = 128;
			cfg.MutexSystemCfg.MaxMutexes = 150;
			cfg.MutexSystemCfg.NumTestAndSetSpins = 10;
			DatabaseEnvironment env = DatabaseEnvironment.Open(testHome, cfg);

			MutexStats stats = env.MutexSystemStats();
			env.Msgfile = testHome + "/" + testName+ ".log";
			env.PrintMutexSystemStats(true, true);
			Assert.AreEqual(512, stats.Alignment);
			Assert.AreEqual(stats.Count, stats.Available + stats.InUse);
			Assert.LessOrEqual(stats.InUse, stats.MaxInUse);
			Assert.AreNotEqual(0, stats.InitCount);
			Assert.AreNotEqual(0, stats.Max);
			Assert.AreNotEqual(0, stats.RegionMax);
			Assert.AreNotEqual(0, stats.RegionSize);
			Assert.AreEqual(0, stats.RegionWait);
			Assert.AreEqual(10, stats.TASSpins);
			ulong regionNoWait = stats.RegionNoWait;

			BTreeDatabaseConfig dbCfg = new BTreeDatabaseConfig();
			dbCfg.Creation = CreatePolicy.IF_NEEDED;
			dbCfg.Env = env;
			BTreeDatabase db = BTreeDatabase.Open(testName + ".db", dbCfg);
			for (int i = 0; i < 1000; i++)
			{
				db.Put(new DatabaseEntry(BitConverter.GetBytes(i)),
				    new DatabaseEntry(BitConverter.GetBytes(i)));
				stats = env.MutexSystemStats();
			}
			Assert.LessOrEqual(regionNoWait, stats.RegionNoWait);
			regionNoWait = stats.RegionNoWait;

			stats = env.MutexSystemStats(true);
			env.PrintMutexSystemStats();
			stats = env.MutexSystemStats();
			Assert.GreaterOrEqual(regionNoWait, stats.RegionNoWait);

			db.Close();
			env.Close();
		}

		[Test]
		public void TestMutexStatPrint()
		{
			testName = "TestMutexStatPrint";
			SetUpTest(true);

			string[] messageInfo = new string[]
			{ 
			  "Mutex region size",
			  "Mutex region max size",
			  "The number of region locks that required waiting (0%)",
			  "Mutex alignment",
			  "Mutex test-and-set spins",
			  "Mutex initial count",
			  "Mutex total count",
			  "Mutex max count",
			  "Mutex free count",
			  "Mutex in-use count",
			  "Mutex maximum in-use count",
			  "",
			  "Unallocated",
			  "env region",
			  "mutex region",
			};

			// Configure and open an environment.
			DatabaseEnvironmentConfig envConfig =
			new DatabaseEnvironmentConfig();
			envConfig.Create = true;
			envConfig.MutexSystemCfg = new MutexConfig();
			envConfig.MutexSystemCfg.Alignment = 512;
			envConfig.MutexSystemCfg.Increment = 128;
			envConfig.MutexSystemCfg.MaxMutexes = 150;
			envConfig.MutexSystemCfg.NumTestAndSetSpins = 10;

			DatabaseEnvironment env =
			    DatabaseEnvironment.Open(testHome, envConfig);

			// Confirm message file does not exist.
			string messageFile = testHome + "/" + "msgfile";
			Assert.AreEqual(false, File.Exists(messageFile));

			// Call set_msgfile() of env.
			env.Msgfile = messageFile;

			// Print env statistic to message file.
			env.PrintMutexSystemStats();

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
				if (tempStr[0] != "Mutex counts")
				{
					Assert.AreEqual(tempStr[1], messageInfo[counter]);
				}
				counter++;
			}
			Assert.AreNotEqual(0, counter);

			file.Close();
			env.Close();
		}

		[Test]
		public void TestLogFile()
		{
			testName = "TestLogFile";
			SetUpTest(true);

			// Open environment and configure logging subsystem.
			DatabaseEnvironmentConfig cfg =
			    new DatabaseEnvironmentConfig();
			cfg.Create = true;
			cfg.UseTxns = true;
			cfg.AutoCommit = true;
			cfg.UseLocking = true;
			cfg.UseMPool = true;
			cfg.UseLogging = true;
			cfg.MPoolSystemCfg = new MPoolConfig();
			cfg.MPoolSystemCfg.CacheSize = 
			    new CacheInfo(0, 1048576, 1);
			cfg.LogSystemCfg = new LogConfig();
			cfg.LogSystemCfg.AutoRemove = false;
			cfg.LogSystemCfg.BufferSize = 10240;
			cfg.LogSystemCfg.Dir = "./";
			cfg.LogSystemCfg.FileMode = 755;
			cfg.LogSystemCfg.ForceSync = true;
			cfg.LogSystemCfg.InMemory = false;
			cfg.LogSystemCfg.LogExternalFileContent = false;
			cfg.LogSystemCfg.MaxFileSize = 1048576;
			cfg.LogSystemCfg.NoBuffer = false;
			cfg.LogSystemCfg.NoSync = true;
			cfg.LogSystemCfg.RegionSize = 204800;
			cfg.LogSystemCfg.ZeroOnCreate = true;
			DatabaseEnvironment env = DatabaseEnvironment.Open(testHome, cfg);

			// Open database.
			Transaction allTxn = env.BeginTransaction();
			TransactionConfig txnConfig = new TransactionConfig();
			txnConfig.Name = "OpenTransaction";
			Transaction openTxn = env.BeginTransaction(txnConfig, allTxn);
			BTreeDatabaseConfig dbConfig = new BTreeDatabaseConfig();
			dbConfig.Creation = CreatePolicy.IF_NEEDED;
			dbConfig.Env = env;
			BTreeDatabase db = BTreeDatabase.Open(
			    testName + ".db", dbConfig, openTxn);

			List<ActiveTransaction> activeTxns =
			    env.TransactionSystemStats().Transactions;
			for (int i = 0; i < activeTxns.Count; i++)
				if (activeTxns[i].Name == "OpenTransaction")
				{
					LSN lsn = new LSN(
					    activeTxns[i].Begun.LogFileNumber,
					    activeTxns[i].Begun.Offset);
					env.LogFlush(lsn);
					string fileName = env.LogFile(lsn);
				}

			openTxn.Commit();

			// Write "##" to log before putting data into database.
			env.WriteToLog("##");

			// Write 1000 records into database.
			TransactionConfig writeTxnConfig = new TransactionConfig();
			writeTxnConfig.Name = "WriteTxn";
			Transaction writeTxn = env.BeginTransaction(writeTxnConfig, allTxn);
			byte[] byteArr = new byte[1024];
			for (int i = 0; i < 1000; i++)
			{
				db.Put(new DatabaseEntry(BitConverter.GetBytes(i)),
				    new DatabaseEntry(byteArr), writeTxn);
				env.LogFlush();
				env.WriteToLog("#" + i.ToString(), writeTxn);
			}

			activeTxns = env.TransactionSystemStats().Transactions;
			for (int i = 0; i < activeTxns.Count; i++)
				if (activeTxns[i].Name == "WriteTxn")
				{
					LSN lsn = new LSN(
					    activeTxns[i].Begun.LogFileNumber,
					    activeTxns[i].Begun.Offset);
					env.LogFlush(lsn);
					string fileName = env.LogFile(lsn);
				}

			writeTxn.Commit();
			db.Close();

			// Write "##" after data has been put.
			env.WriteToLog("##");

			List<string> logFiles = env.LogFiles(true);

			env.LogWrite(new DatabaseEntry(), true);

			env.RemoveUnusedLogFiles();

			allTxn.Commit();
			env.Close();
		}

		[Test]
		public void TestLogVerifyConfig()
		{
			testName = "TestLogVerifyConfig";
			SetUpTest(true);

			// Open enviornment.
			DatabaseEnvironmentConfig cfg =
			    new DatabaseEnvironmentConfig();
			cfg.Create = true;
			cfg.UseLocking = true;
			cfg.UseLogging = true;
			cfg.UseMPool = true;
			cfg.UseTxns = true;
			DatabaseEnvironment env = DatabaseEnvironment.Open(
			    testHome, cfg);

			// Open database.
			Transaction txn = env.BeginTransaction();
			BTreeDatabaseConfig dbCfg = new BTreeDatabaseConfig();
			dbCfg.Env = env;
			dbCfg.Creation = CreatePolicy.IF_NEEDED;
			dbCfg.Duplicates = DuplicatesPolicy.UNSORTED;
			BTreeDatabase db = BTreeDatabase.Open(
			    testName, dbCfg, txn);
			txn.Commit();

			// Do a lot of transactional operations to create logs.
			DatabaseEntry key, data;
			for (int i = 0; i < 5; i++) {
				txn = env.BeginTransaction();
				switch (i) {
				case 0:
					/* 
					 * Put 1000 records and commit the 
					 * transaction.
					 */ 						 
					for (int j = 0; j < 1000;j++) {
						key = new DatabaseEntry(
						    BitConverter.GetBytes(j));
						data = new DatabaseEntry(
						    BitConverter.GetBytes(j));
						db.Put(key, data, txn);
					}
					txn.Commit();
					break;
				case 1:
					/*
					 * Put 10 overflow records and commit 
					 * the transaction.
					 */ 					 
					for (int j = 0; j < 10;j++) {
						key = new DatabaseEntry(
						    BitConverter.GetBytes(j));
						data = new DatabaseEntry(
						    new byte[1024 * 4]);
						db.Put(key, data, txn);
					}
					txn.Commit();
					break;
				case 2:
					/*
					 * Put 1000 records and abort the 
					 * transaction.
					 */ 					 
					for (int j = 0; j < 1000;j++) {
						key = new DatabaseEntry(
						    BitConverter.GetBytes(j));
						data = new DatabaseEntry(
						    BitConverter.GetBytes(j + 1));
						db.Put(key, data, txn);
					}
					txn.Abort();
					break;
				case 3:
					/*
					 * Put 10 overflow records and abort the
					 * transaction.
					 */ 					 
					for (int j = 10; j < 20;j++) {
						key = new DatabaseEntry(
						    BitConverter.GetBytes(j));
						data = new DatabaseEntry(
						    new byte[1024 * 4]);
						db.Put(key, data, txn);
					}
					txn.Abort();
					break;
				case 4:
					// Get 100 records.
					for (int j = 100; j < 200; j++) {
						key = new DatabaseEntry(
						    BitConverter.GetBytes(j));
						data = new DatabaseEntry(
						    BitConverter.GetBytes(j));
						db.GetBoth(key, data, txn);
					}
					txn.Commit();
					break;
				}
			}

			// Verfiy entire log.
			string tempHome = testHome + "_tmp";
			Configuration.ClearDir(tempHome);
			LogVerifyConfig logVerifyCfg = new LogVerifyConfig();
			logVerifyCfg.CacheSize = 1024 * 10;
			logVerifyCfg.ContinueAfterFail = true;
			logVerifyCfg.EnvHome = tempHome;
			logVerifyCfg.Verbose = true;
			Assert.AreEqual(0, env.LogVerify(logVerifyCfg));

			// Verify specific database.
			logVerifyCfg.DbFile = testName + ".db";
			logVerifyCfg.DbName = testName;
			Assert.AreEqual(0, env.LogVerify(logVerifyCfg));

			// Verify partial log.
			DateTime time = DateTime.Now;
			DateTime startTime = time.AddMinutes(-2);
			DateTime endTime = time.AddMinutes(-1);
			logVerifyCfg.StartTime = startTime;
			logVerifyCfg.EndTime = endTime;
			Assert.AreEqual(0, env.LogVerify(logVerifyCfg));

			// Verfiy unexisting log.
			logVerifyCfg.StartLsn = new LSN(1, 3);
			logVerifyCfg.EndLsn = new LSN(2, 100);
			Assert.AreEqual(0, env.LogVerify(logVerifyCfg));

			Assert.AreEqual(logVerifyCfg.CacheSize, 1024 * 10);
			Assert.AreEqual(logVerifyCfg.ContinueAfterFail, true);
			Assert.AreEqual(logVerifyCfg.DbFile, testName + ".db");
			Assert.AreEqual(logVerifyCfg.DbName, testName);
			Assert.AreEqual(logVerifyCfg.EndLsn.LogFileNumber, 2);
			Assert.AreEqual(logVerifyCfg.EndLsn.Offset, 100);
			Assert.AreEqual(logVerifyCfg.EndTime, endTime);
			Assert.AreEqual(logVerifyCfg.EnvHome, testHome + "_tmp");
			Assert.AreEqual(logVerifyCfg.StartLsn.LogFileNumber, 1);
			Assert.AreEqual(logVerifyCfg.StartLsn.Offset, 3);
			Assert.AreEqual(logVerifyCfg.StartTime, startTime);
			Assert.AreEqual(logVerifyCfg.Verbose, true);
			db.Close();
			env.Close();
		}

		[Test]
		public void TestOpen()
		{
			testName = "TestOpen";
			SetUpTest(true);

			DatabaseEnvironmentConfig cfg =
			    new DatabaseEnvironmentConfig();
			cfg.Create = true;
			DatabaseEnvironment env = DatabaseEnvironment.Open(testHome, cfg);

			// Confirm that the environment is initialized.
			Assert.IsNotNull(env);

			// Confirm the environment home directory.
			Assert.AreEqual(testHome, env.Home);

			// Print statistics of the current environment.
			env.Msgfile = testHome + "/" + testName+ ".log";
			env.PrintStats();

			// Print statistics of all subsytems.
			env.PrintSubsystemStats();

			env.Close();
		}

		[Test]
		public void TestStatPrint()
		{
			testName = "TestStatPrint";
			SetUpTest(true);

			string[] messageInfo = new string[]
			{ 
			  "Local time",
			  "Magic number",
			  "Panic value",
			  "Environment version",
			  "Btree version",
			  "Hash version",
			  "Lock version",
			  "Log version",
			  "Queue version",
			  "Sequence version",
			  "Txn version",
			  "Creation time",
			  "Environment ID",
			  "Primary region allocation and reference count mutex [0/4 0% !Own], env region (alloc)",
			  "References",
			  "Current region size",
			  "Maximum region size",
			  "Process failure detected"	// This appears only with HAVE_FAILCHK_BROADCAST.
			};

			// Configure and open an environment.
			DatabaseEnvironmentConfig envConfig =
			    new DatabaseEnvironmentConfig();
			envConfig.Create = true;
			envConfig.FreeThreaded = true;
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

			// Confirm message file does not exist.
			string messageFile = testHome + "/" + "msgfile";
			Assert.AreEqual(false, File.Exists(messageFile));

			// Call set_msgfile() of env.
			env.Msgfile = messageFile;

			// Print env statistic to message file.
			env.PrintStats();

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
				Assert.AreEqual(messageInfo[counter], tempStr[1]);
				counter++;
			}
			Assert.AreNotEqual(0, counter);

			file.Close();
			env.Close();
		}

		[Test]
		public void TestSubsystemStatPrint()
		{
			testName = "TestSubsystemStatPrint";
			SetUpTest(true);

			string[] messageInfo = new string[]
			{ 
			  "Local time",
			  "Magic number",
			  "Panic value",
			  "Environment version",
			  "Btree version",
			  "Hash version",
			  "Lock version",
			  "Log version",
			  "Queue version",
			  "Sequence version",
			  "Txn version",
			  "Creation time",
			  "Environment ID",
			  "Primary region allocation and reference count mutex [0/4 0% !Own], env region (alloc)",
			  "References",
			  "Current region size",
			  "Maximum region size",
			  "",
			  "Log magic number",
			  "Log version number",
			  "Log record cache size",
			  "Log file mode",
			  "Current log file size",
			  "Initial fileid allocation",
			  "Current fileids in use",
			  "Maximum fileids used",
			  "Records entered into the log",
			  "Log bytes written",
			  "Log bytes written since last checkpoint",
			  "Total log file I/O writes",
			  "Total log file I/O writes due to overflow",
			  "Total log file flushes",
			  "Total log file I/O reads",
			  "Current log file number",
			  "Current log file offset",
			  "On-disk log file number",
			  "On-disk log file offset",
			  "Maximum commits in a log flush",
			  "Minimum commits in a log flush",
			  "Region size",
			  "The number of region locks that required waiting (0%)",
			  "",
			  "",
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
			  "Maximum hash bucket length",
			  "",
			  "Total cache size",
			  "Number of caches",
			  "Maximum number of caches",
			  "Pool individual cache size",
			  "Pool individual cache max",
			  "Maximum memory-mapped file size",
			  "Maximum open file descriptors",
			  "Maximum sequential buffer writes",
			  "Sleep after writing maximum sequential buffers",
			  "Requested pages mapped into the process' address space",
			  "Requested pages found in the cache (0%)",
			  "Requested pages not found in the cache",
			  "Pages created in the cache",
			  "Pages read into the cache",
			  "Pages written from the cache to the backing file",
			  "Clean pages forced from the cache",
			  "Dirty pages forced from the cache",
			  "Dirty pages written by trickle-sync thread",
			  "Current total page count",
			  "Current clean page count",
			  "Current dirty page count",
			  "Number of hash buckets used for page location",
			  "Number of mutexes for the hash buckets",
			  "Assumed page size used",
			  "Total number of times hash chains searched for a page",
			  "The longest hash chain searched for a page",
			  "Total number of hash chain entries checked for page",
			  "The number of hash bucket locks that required waiting (0%)",
			  "The maximum number of times any hash bucket lock was waited for (0%)",
			  "The number of region locks that required waiting (0%)",
			  "The number of buffers frozen",
			  "The number of buffers thawed",
			  "The number of frozen buffers freed",
			  "The number of outdated intermediate versions reused",
			  "The number of page allocations",
			  "The number of hash buckets examined during allocations",
			  "The maximum number of hash buckets examined for an allocation",
			  "The number of pages examined during allocations",
			  "The max number of pages examined for an allocation",
			  "Threads waited on page I/O",
			  "The number of times a sync is interrupted",
			  "",
			  "No checkpoint LSN",
			  "Checkpoint timestamp",
			  "Last transaction ID allocated",
			  "Maximum number of active transactions configured",
			  "Initial number of transactions configured",
			  "Active transactions",
			  "Maximum active transactions",
			  "Number of transactions begun",
			  "Number of transactions aborted",
			  "Number of transactions committed",
			  "Snapshot transactions",
			  "Maximum snapshot transactions",
			  "Number of transactions restored",
			  "Region size",
			  "The number of region locks that required waiting (0%)",
			  "",
			  "",
			  "Mutex region size",
			  "Mutex region max size",
			  "The number of region locks that required waiting (0%)",
			  "Mutex alignment",
			  "Mutex test-and-set spins",
			  "Mutex initial count",
			  "Mutex total count",
			  "Mutex max count",
			  "Mutex free count",
			  "Mutex in-use count",
			  "Mutex maximum in-use count",
			  "",
			  "Unallocated",
			  "env dblist",
			  "env handle",
			  "env region",
			  "lock region",
			  "log filename",
			  "log flush",
			  "log region",
			  "mpool file bucket",
			  "mpool handle",
			  "mpool hash bucket",
			  "mpool region",
			  "mutex region",
			  "twister",
			  "txn active list",
			  "transaction checkpoint"
			};

			// Configure and open an environment.
			DatabaseEnvironmentConfig envConfig =
			    new DatabaseEnvironmentConfig();
			envConfig.Create = true;
			envConfig.FreeThreaded = true;
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

			// Confirm message file does not exist.
			string messageFile = testHome + "/" + "msgfile";
			Assert.AreEqual(false, File.Exists(messageFile));

			// Call set_msgfile() of env.
			env.Msgfile = messageFile;

			// Print env statistic to message file.
			env.PrintSubsystemStats();

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
				if (tempStr[0] != "=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=" &&
				tempStr[0] != "Active transactions:" && tempStr[0] != "Mutex counts")
				{
					// Ignore statistics lines which appear only some of the time.
					if (tempStr[1] == "Process failure detected")
						continue;
					Assert.AreEqual(messageInfo[counter], tempStr[1]);
				}
				counter++;
			}
			Assert.AreNotEqual(0, counter);

			file.Close();
			env.Close();
		}

		[Test]
		public void TestMPoolSystemStats()
		{
			testName = "TestMPoolSystemStats";
			SetUpTest(true);

			DatabaseEnvironmentConfig envConfig =
			    new DatabaseEnvironmentConfig();
			envConfig.AutoCommit = true;
			envConfig.MPoolSystemCfg = new MPoolConfig();
			envConfig.MPoolSystemCfg.CacheSize = 
			    new CacheInfo(0, 1048576, 3);
			envConfig.Create = true;
			envConfig.UseLocking = true;
			envConfig.UseLogging = true;
			envConfig.UseMPool = true;
			envConfig.UseTxns = true;

			DatabaseEnvironment env = DatabaseEnvironment.Open(
			    testHome, envConfig);

			MPoolStats stats = env.MPoolSystemStats();
			env.Msgfile = testHome + "/" + testName+ ".log";
			env.PrintMPoolSystemStats();

			Assert.AreEqual(0, stats.BlockedOperations);
			Assert.AreEqual(0, stats.BucketsCheckedDuringAlloc);
			Assert.AreEqual(3, stats.MaxCacheRegions);
			Assert.LessOrEqual(1048576, stats.CacheSettings.Bytes);
			Assert.AreEqual(0, stats.CacheSettings.Gigabytes);
			Assert.AreEqual(3, stats.CacheSettings.NCaches);
			Assert.AreEqual(0, stats.CleanPages);
			Assert.AreEqual(0, stats.CleanPagesEvicted);
			Assert.AreEqual(0, stats.DirtyPages);
			Assert.AreEqual(0, stats.DirtyPagesEvicted);
			Assert.IsNotNull(stats.Files);
			Assert.AreEqual(0, stats.FrozenBuffers);
			Assert.AreEqual(0, stats.FrozenBuffersFreed);
			Assert.LessOrEqual(stats.HashBuckets, 
			    stats.HashBucketMutexes);
			Assert.LessOrEqual(37, stats.HashBuckets);
			Assert.LessOrEqual(0, stats.HashChainSearches);
			Assert.AreEqual(0, stats.HashEntriesSearched);
			Assert.AreEqual(0, stats.HashLockNoWait);
			Assert.AreEqual(0, stats.HashLockWait);
			Assert.AreEqual(0, stats.LongestHashChainSearch);
			Assert.AreEqual(0, stats.MappedPages);
			Assert.AreEqual(0, stats.MaxBucketsCheckedDuringAlloc);
			Assert.AreEqual(0, stats.MaxBufferWrites);
			Assert.AreEqual(0, stats.MaxBufferWritesSleep);
			Assert.AreEqual(0, stats.MaxHashLockNoWait);
			Assert.AreEqual(0, stats.MaxHashLockWait);
			Assert.AreEqual(0, stats.MaxMMapSize);
			Assert.AreEqual(0, stats.MaxOpenFileDescriptors);
			Assert.AreEqual(0, stats.MaxPagesCheckedDuringAlloc);
			Assert.AreEqual(0, stats.OddFileSizeDetected);
			Assert.AreEqual(0, stats.OddFileSizeResolve);
			Assert.AreEqual(0, stats.PageAllocations);
			Assert.AreEqual(0, stats.Pages);
			Assert.AreEqual(0, stats.PagesCheckedDuringAlloc);
			Assert.LessOrEqual(0, stats.PagesCreatedInCache);
			Assert.AreEqual(0, stats.PagesInCache);
			Assert.AreEqual(0, stats.PagesNotInCache);
			Assert.AreEqual(0, stats.PagesRead);
			Assert.AreEqual(0, stats.PagesTrickled);
			Assert.AreEqual(0, stats.PagesWritten);
			Assert.AreNotEqual(0, stats.RegionLockNoWait);
			Assert.AreEqual(0, stats.RegionLockWait);
			Assert.LessOrEqual(0, stats.RegionSize);
			Assert.AreNotEqual(0, stats.RegionMax);
			Assert.AreEqual(0, stats.ThawedBuffers);

			Transaction txn = env.BeginTransaction();
			BTreeDatabaseConfig dbConfig = new BTreeDatabaseConfig();
			dbConfig.Creation = CreatePolicy.IF_NEEDED;
			dbConfig.Env = env;
			dbConfig.PageSize = 4096;
			BTreeDatabase db = BTreeDatabase.Open(
			    testName + ".db", dbConfig, txn);

			stats = env.MPoolSystemStats();
			Assert.AreEqual(1, stats.Files.Count);
			Assert.AreEqual(testName + ".db", stats.Files[0].FileName);
			Assert.AreEqual(0, stats.Files[0].BackupSpins);
			Assert.AreEqual(0, stats.Files[0].MappedPages);
			Assert.AreEqual(0, stats.Files[0].PagesCreatedInCache);
			Assert.AreEqual(0, stats.Files[0].PagesInCache);
			Assert.AreEqual(4096, stats.Files[0].PageSize);
			Assert.LessOrEqual(1, stats.Files[0].PagesNotInCache);
			Assert.AreEqual(1, stats.Files[0].PagesRead);
			Assert.AreEqual(0, stats.Files[0].PagesWritten);

			byte[] largeByte = new byte[1088576];
			for (int i = 0; i < 10; i++)
				db.Put(new DatabaseEntry(BitConverter.GetBytes(i)), 
				    new DatabaseEntry(largeByte), txn);
			db.Put(new DatabaseEntry(largeByte), 
			    new DatabaseEntry(largeByte), txn);

			stats = env.MPoolSystemStats();
			Assert.Less(0, stats.Files[0].PagesCreatedInCache);
			Assert.Less(0, stats.Files[0].PagesInCache);
			Assert.LessOrEqual(1, stats.Files[0].PagesNotInCache);
			Assert.LessOrEqual(1, stats.Files[0].PagesRead);
			Assert.Less(0, stats.Files[0].PagesWritten);

			txn.Commit();
			db.Close();

			// Clean the stats after printing.
			stats = env.MPoolSystemStats(true);
			env.PrintMPoolSystemStats(true, true);
			stats = env.MPoolSystemStats();
			env.PrintMPoolSystemStats(true, true, true);

			env.Close();
		}

		[Test]
		public void TestMPoolStatPrint()
		{
			testName = "TestMPoolStatPrint";
			SetUpTest(true);

			string[] messageInfo = new string[]
			{
			  "Total cache size",
			  "Number of caches",
			  "Maximum number of caches",
			  "Pool individual cache size",
			  "Pool individual cache max",
			  "Maximum memory-mapped file size",
			  "Maximum open file descriptors",
			  "Maximum sequential buffer writes",
			  "Sleep after writing maximum sequential buffers",
			  "Requested pages mapped into the process' address space",
			  "Requested pages found in the cache (0%)",
			  "Requested pages not found in the cache",
			  "Pages created in the cache",
			  "Pages read into the cache",
			  "Pages written from the cache to the backing file",
			  "Clean pages forced from the cache",
			  "Dirty pages forced from the cache",
			  "Dirty pages written by trickle-sync thread",
			  "Current total page count",
			  "Current clean page count",
			  "Current dirty page count",
			  "Number of hash buckets used for page location",
			  "Number of mutexes for the hash buckets",
			  "Assumed page size used",
			  "Total number of times hash chains searched for a page",
			  "The longest hash chain searched for a page",
			  "Total number of hash chain entries checked for page",
			  "The number of hash bucket locks that required waiting (0%)",
			  "The maximum number of times any hash bucket lock was waited for (0%)",
			  "The number of region locks that required waiting (0%)",
			  "The number of buffers frozen",
			  "The number of buffers thawed",
			  "The number of frozen buffers freed",
			  "The number of outdated intermediate versions reused",
			  "The number of page allocations",
			  "The number of hash buckets examined during allocations",
			  "The maximum number of hash buckets examined for an allocation",
			  "The number of pages examined during allocations",
			  "The max number of pages examined for an allocation",
			  "Threads waited on page I/O",
			  "The number of times a sync is interrupted"
			};

			// Configure and open an environment.
			DatabaseEnvironmentConfig envConfig =
			    new DatabaseEnvironmentConfig();
			envConfig.Create = true;
			envConfig.MPoolSystemCfg = new MPoolConfig();
			envConfig.MPoolSystemCfg.CacheSize = new CacheInfo(0, 104800, 1);
			envConfig.UseMPool = true;
			DatabaseEnvironment env =
			    DatabaseEnvironment.Open(testHome, envConfig);

			// Confirm message file does not exist.
			string messageFile = testHome + "/" + "msgfile";
			Assert.AreEqual(false, File.Exists(messageFile));

			// Call set_msgfile() of env.
			env.Msgfile = messageFile;

			// Print env statistic to message file.
			env.PrintMPoolSystemStats();

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
				Assert.AreEqual(messageInfo[counter], tempStr[1]);
				counter++;
			}
			Assert.AreNotEqual(0, counter);

			file.Close();
			env.Close();
		}

		[Test]
		public void TestRemove()
		{
			testName = "TestRemove";
			SetUpTest(true);

			// Open new environment.
			DatabaseEnvironmentConfig envConig = 
			    new DatabaseEnvironmentConfig();
			envConig.Create = true;
			envConig.ErrorPrefix = testFixtureName + ":" + testName;
			DatabaseEnvironment env = DatabaseEnvironment.Open(
			    testHome, envConig);
			env.Close();

			// Remove the existing environment.
			DatabaseEnvironment.Remove(testHome);

			// Confirm that the __db.001 is removed.
			Assert.IsFalse(File.Exists(testHome + "__db.001"));
		}

		[Test]
		public void TestRemoveCorruptedEnv()
		{
			testName = "TestRemoveCorruptedEnv";
			SetUpTest(true);

			// Open new environment.
			DatabaseEnvironmentConfig envConig =
			    new DatabaseEnvironmentConfig();
			envConig.Create = true;
			envConig.ErrorPrefix = testFixtureName + ":" + testName;
			DatabaseEnvironment env = DatabaseEnvironment.Open(testHome, envConig);

			// Panic the environment.
			env.Panic();

			// Remove the corrupted environment.
			DatabaseEnvironment.Remove(testHome, true);

			// Confirm that the __db.001 is removed.
			Assert.IsFalse(File.Exists(testHome + "__db.001"));
		}

		[Test, ExpectedException(typeof(ExpectedTestException))]
		public void TestRenameDB()
		{
			testName = "TestRenameDB";
			SetUpTest(true);

			RenameDB(testHome, testName, false);
		}

		[Test, ExpectedException(typeof(ExpectedTestException))]
		public void TestRenameDBWithTxn()
		{
			testName = "TestRenameDBWithTxn";
			SetUpTest(true);

			RenameDB(testHome, testName, true);
		}

		public void RenameDB(string home, string name, bool ifTxn)
		{
			string dbFileName = name + ".db";
			string dbName = "db1";
			string dbNewName = "db2";

			Configuration.ClearDir(home);

			DatabaseEnvironmentConfig envConig =
			    new DatabaseEnvironmentConfig();
			envConig.Create = true;
			envConig.UseTxns = true;
			envConig.UseLogging = true;
			envConig.UseMPool = true;
			DatabaseEnvironment env = DatabaseEnvironment.Open(
			    home, envConig);

			Transaction openTxn = env.BeginTransaction();
			BTreeDatabaseConfig dbConfig =
			    new BTreeDatabaseConfig();
			dbConfig.Creation = CreatePolicy.IF_NEEDED;
			dbConfig.Env = env;
			BTreeDatabase db = BTreeDatabase.Open(
			    dbFileName, dbName, dbConfig, openTxn);
			db.Close();
			openTxn.Commit();

			// Open the database.
			if (ifTxn == false)
				env.RenameDB(dbFileName, dbName, dbNewName, true);
			else
			{
				Transaction renameTxn = env.BeginTransaction();
				env.RenameDB(dbFileName, dbName, dbNewName, false, renameTxn);
				renameTxn.Commit();
			}

			// Confirm that the database are renamed.
			Transaction reopenTxn = env.BeginTransaction();
			try
			{
				Database db1 = Database.Open(
				    dbFileName, new DatabaseConfig());
				db1.Close();
			}
			catch (DatabaseException)
			{
				throw new ExpectedTestException();
			}
			finally
			{
				reopenTxn.Commit();
				env.Close();
			}
		}

		[Test]
		public void TestResetFileID()
		{
			testName = "TestResetFileID";
			SetUpTest(true);
			string dbFileName = testName + ".db";
			string dbNewFileName = testName + "_new.db";

			DatabaseEnvironmentConfig envConfig =
			    new DatabaseEnvironmentConfig();
			envConfig.Create = true;
			envConfig.UseMPool = true;
			DatabaseEnvironment env = DatabaseEnvironment.Open(
			    testHome, envConfig);

			// Opening a new database.
			BTreeDatabaseConfig dbConfig =
			    new BTreeDatabaseConfig();
			dbConfig.Creation = CreatePolicy.IF_NEEDED;
			dbConfig.Env = env;
			BTreeDatabase db = BTreeDatabase.Open(
			    dbFileName, dbConfig);
			db.Close();

			// Copy the physical database file.
			File.Copy(testHome + "/" + dbFileName,
			    testHome + "/" + dbNewFileName);

			// Reset the file ID.
			env.ResetFileID(dbNewFileName, false);

			// Open the exisiting database in copied database file.
			BTreeDatabaseConfig cfg = new BTreeDatabaseConfig();
			cfg.Creation = CreatePolicy.NEVER;
			cfg.Env = env;
			BTreeDatabase newDB = BTreeDatabase.Open(
			    dbNewFileName, cfg);
			newDB.Close();
			env.Close();
		}

		[Test, ExpectedException(typeof(ExpectedTestException))]
		public void TestRemoveDB()
		{
			testName = "TestRemoveDB";
			SetUpTest(true);

			RmDBWithoutTxn(testHome, testName, false);
		}

		[Test, ExpectedException(typeof(ExpectedTestException))]
		public void TestRemoveDBWithAutoCommit()
		{
			testName = "TestRemoveDBWithAutoCommit";
			SetUpTest(true);

			RmDBWithoutTxn(testHome, testName, true);
		}

		[Test, ExpectedException(typeof(ExpectedTestException))]
		public void TestRemoveDBWithinTxn()
		{
			testName = "TestRemoveDBWithinTxn";
			SetUpTest(true);
			string dbFileName = testName + ".db";
			string dbName1 = testName + "1";
			string dbName2 = testName + "2";

			// Open environment.
			DatabaseEnvironmentConfig envConfig =
			    new DatabaseEnvironmentConfig();
			envConfig.Create = true;
			envConfig.UseMPool = true;
			envConfig.UseTxns = true;
			envConfig.UseLogging = true;
			DatabaseEnvironment env = DatabaseEnvironment.Open(
			    testHome, envConfig);
			Transaction txn = env.BeginTransaction();

			// Create two databases in the environment.
			BTreeDatabaseConfig dbConfig =
			    new BTreeDatabaseConfig();
			dbConfig.Creation = CreatePolicy.IF_NEEDED;
			dbConfig.Env = env;
			BTreeDatabase btreeDB1 = BTreeDatabase.Open(
			    dbFileName, dbName1, dbConfig, txn);
			btreeDB1.Close();
			BTreeDatabase btreeDB2 = BTreeDatabase.Open(
			    dbFileName, dbName2, dbConfig, txn);
			btreeDB2.Close();

			// Remove one database from the environment.
			env.RemoveDB(dbFileName, dbName2, false, txn);

			// Try to open the existing database.
			DatabaseConfig cfg = new DatabaseConfig();
			cfg.Env = env;
			Database db1 = Database.Open(dbFileName, dbName1, cfg, txn);
			db1.Close();

			/*
			 * Attempting to open the removed database should
			 * cause error.
			 */
			try
			{
				Database db2 = Database.Open(
				    dbFileName, dbName2, cfg, txn);
				db2.Close();
			}
			catch (DatabaseException)
			{
				throw new ExpectedTestException();
			}
			finally
			{
				txn.Commit();
				env.Close();
			}
		}

		public void RmDBWithoutTxn(string home, string dbName,
		    bool ifAutoCommit)
		{
			string dbFileName = dbName + ".db";
			string dbName1 = dbName + "1";
			string dbName2 = dbName + "2";

			// Open environment.
			DatabaseEnvironmentConfig envConfig =
			    new DatabaseEnvironmentConfig();
			envConfig.Create = true;
			envConfig.UseMPool = true;
			if (ifAutoCommit == true)
			{
				envConfig.AutoCommit = true;
				envConfig.UseTxns = true;
			}

			DatabaseEnvironment env = DatabaseEnvironment.Open(
			    home, envConfig);

			// Create two databases in the environment.
			BTreeDatabaseConfig dbConfig =
			    new BTreeDatabaseConfig();
			dbConfig.Creation = CreatePolicy.IF_NEEDED;
			dbConfig.Env = env;
			BTreeDatabase btreeDB1 = BTreeDatabase.Open(
			    dbFileName, dbName1, dbConfig);
			btreeDB1.Close();
			BTreeDatabase btreeDB2 = BTreeDatabase.Open(
			    dbFileName, dbName2, dbConfig);
			btreeDB2.Close();

			// Remove one database from the environment.
			env.RemoveDB(dbFileName, dbName2, false);

			// Try to open the existing database.
			DatabaseConfig cfg = new DatabaseConfig();
			cfg.Env = env;
			Database db1 = Database.Open(dbFileName, dbName1, cfg);
			db1.Close();

			/*
			 * Attempting to open the removed database should
			 * cause error.
			 */
			try
			{
				Database db2 = Database.Open(
				    dbFileName, dbName2, cfg);
				db2.Close();
			}
			catch (DatabaseException)
			{
				throw new ExpectedTestException();
			}
			finally
			{
				env.Close();
			}
		}

		[Test]
		public void TestTransactionSystemStats()
		{
			testName = "TestTransactionSystemStats";
			SetUpTest(true);

			TransactionStats stats;
			BTreeDatabase db;
			Transaction openTxn = null;

			// Open an environment.
			long dateTime;
			DatabaseEnvironmentConfig envConfig =
			    new DatabaseEnvironmentConfig();
			envConfig.Create = true;
			envConfig.MaxTransactions = 50;
			envConfig.UseLogging = true;
			envConfig.UseLocking = true;
			envConfig.UseMPool = true;
			envConfig.UseTxns = true;
			envConfig.TxnNoSync = false;
			envConfig.TxnNoWait = true;
			envConfig.TxnSnapshot = true;
			envConfig.TxnTimestamp = DateTime.Now;
			envConfig.TxnWriteNoSync = false;
			DatabaseEnvironment env = DatabaseEnvironment.Open(
			    testHome, envConfig);

			try
			{
				try
				{
					// Confirm initial transaction subsystem statistics.
					stats = env.TransactionSystemStats();
					env.Msgfile = testHome + "/" + testName+ ".log";
					env.PrintTransactionSystemStats(true, true);
					Assert.AreEqual(0, stats.Aborted);
					Assert.AreEqual(0, stats.Active);
					Assert.AreEqual(0, stats.Begun);
					Assert.AreEqual(0, stats.Committed);
					Assert.AreEqual(0, stats.InitTxns);
					Assert.AreEqual(0, stats.LastCheckpoint.LogFileNumber);
					Assert.AreEqual(0, stats.LastCheckpoint.Offset);
					Assert.AreEqual(50, stats.MaxTransactions);
					Assert.AreNotEqual(0, stats.RegionSize);
					Assert.AreEqual(0, stats.Transactions.Count);
				}
				catch (AssertionException e)
				{
					throw e;
				}

				try
				{
					//Begin a transaction called openTxn and open a database.
					TransactionConfig openTxnCfg = new TransactionConfig();
					openTxnCfg.Name = "openTxn";
					openTxn = env.BeginTransaction(openTxnCfg);
					openTxn.Priority = 50;
					BTreeDatabaseConfig dbConfig = new BTreeDatabaseConfig();
					dbConfig.Creation = CreatePolicy.IF_NEEDED;
					dbConfig.Env = env;
					db = BTreeDatabase.Open(testName + ".db", dbConfig, openTxn);
				}
				catch (DatabaseException e)
				{
					if (openTxn != null)
						openTxn.Abort();
					throw e;
				}

				try
				{
					// At least there is one transaction that is alive.
					env.Checkpoint();
					stats = env.TransactionSystemStats();
					env.PrintTransactionSystemStats();
					Assert.AreNotEqual(0, stats.Active);
					Assert.AreNotEqual(0, stats.Transactions.Count);
					Assert.AreNotEqual(0, stats.Transactions.Capacity);
					Assert.AreNotEqual(0, stats.RegionLockNoWait);
					dateTime = stats.LastCheckpointTime;

					// Begin an embedded transaction called putTxn.
					TransactionConfig putTxnCfg =
					    new TransactionConfig();
					putTxnCfg.Name = "putTxn";
					putTxnCfg.NoWait = false;
					Transaction putTxn = env.BeginTransaction(
					    putTxnCfg, openTxn);
					putTxn.Priority = 50;

					try
					{
						// Put some records into database within putTxn.
						for (int i = 0; i < 50; i++)
							db.Put(new DatabaseEntry(BitConverter.GetBytes(i)), 
							    new DatabaseEntry(BitConverter.GetBytes(i)), putTxn);
							stats = env.TransactionSystemStats();
							Assert.AreNotEqual(0, stats.MaxActive);
							Assert.AreNotEqual(0, stats.MaxTransactions);
							Assert.AreEqual(0, stats.MaxSnapshot);
							Assert.AreEqual(0, stats.Snapshot);
							Assert.AreEqual(stats.Begun, 
							    stats.Aborted + stats.Active + stats.Committed);
							Assert.AreEqual(2, stats.Transactions.Count);

						/*
						 * Both of LogFileNumber and Offset in active transaction
						 * couldn't be 0.
						 */ 
						uint logFileNumbers = 0;
						uint offSets = 0;
						for (int i = 0; i < stats.Transactions.Count;i++)
						{
							logFileNumbers += stats.Transactions[i].Begun.LogFileNumber;
							offSets += stats.Transactions[i].Begun.Offset;
						}
						Assert.AreNotEqual(0, logFileNumbers);
						Assert.AreNotEqual(0, offSets);

						// All active transactions are run by the same process and thread.

						Assert.AreEqual(stats.Transactions[0].ThreadID, 
						    stats.Transactions[1].ThreadID);
						Assert.AreEqual(stats.Transactions[0].ProcessID,
						    stats.Transactions[1].ProcessID);

						Assert.AreEqual(50, stats.Transactions[0].Priority);
						Assert.AreEqual(50, stats.Transactions[1].Priority);

						/*
						 * Find the openTxn in active transactions, which is the
						 * parent transaction of putTxn.
						 */
						int parentPos = 0;
						if (stats.Transactions[0].Name == "putTxn")
							parentPos = 1;

						// putTxn's parent id should be the openTxn.
						Assert.AreEqual(stats.Transactions[parentPos].ID,
						    stats.Transactions[1 - parentPos].ParentID);

						// Other stats should be an positive integer.
						for (int i = 0; i < stats.Transactions.Count - 1; i++)
						{
							Assert.LessOrEqual(0, 
							    stats.Transactions[i].BufferCopiesInCache);
							Assert.LessOrEqual(0, 
							    stats.Transactions[i].SnapshotReads.LogFileNumber);
							Assert.LessOrEqual(0,
							    stats.Transactions[i].SnapshotReads.Offset);
							Assert.IsNotNull(stats.Transactions[i].GlobalID);
						}

						// Commit putTxn.
						putTxn.Commit();
					}
					catch (DatabaseException e)
					{
						putTxn.Abort();
						throw e;
					}

					stats = env.TransactionSystemStats();
					Assert.AreNotEqual(0, stats.LastCheckpoint.LogFileNumber);
					Assert.AreNotEqual(0, stats.LastCheckpoint.Offset);
					Assert.AreEqual(dateTime, stats.LastCheckpointTime);

					openTxn.Commit();
				}
				catch (DatabaseException e)
				{
					openTxn.Abort();
					throw e;
				}
				finally
				{
					db.Close();
				}
			}
			finally
			{
				env.Close();
			}
		}

		[Test]
		public void TestTransactionStatPrint()
		{
			testName = "TestTransactionStatPrint";
			SetUpTest(true);

			string[] messageInfo = new string[]
			{
			  "No checkpoint LSN",
			  "Checkpoint timestamp",
			  "Last transaction ID allocated",
			  "Maximum number of active transactions configured",
			  "Initial number of transactions configured",
			  "Active transactions",
			  "Maximum active transactions",
			  "Number of transactions begun",
			  "Number of transactions aborted",
			  "Number of transactions committed",
			  "Snapshot transactions",
			  "Maximum snapshot transactions",
			  "Number of transactions restored",
			  "Region size",
			  "The number of region locks that required waiting (0%)",
			  ""
			};            

			// Configure and open an environment.
			DatabaseEnvironmentConfig envConfig =
			    new DatabaseEnvironmentConfig();
			envConfig.Create = true;
			envConfig.FreeThreaded = true;
			envConfig.LockTimeout = 1000;
			envConfig.MPoolSystemCfg = new MPoolConfig();
			envConfig.MPoolSystemCfg.CacheSize = new CacheInfo(0, 104800, 1);
			envConfig.UseLocking = true;
			envConfig.UseMPool = true;
			envConfig.UseTxns = true;
			envConfig.MaxTransactions = 50;

			DatabaseEnvironment env =
			    DatabaseEnvironment.Open(testHome, envConfig);

			//Begin a transaction called openTxn and open a database.
			Transaction openTxn = null;
			TransactionConfig openTxnCfg = new TransactionConfig();
			openTxnCfg.Name = "openTxn";
			openTxn = env.BeginTransaction(openTxnCfg);
			openTxn.Priority = 50;
			BTreeDatabaseConfig dbConfig = new BTreeDatabaseConfig();
			dbConfig.Creation = CreatePolicy.IF_NEEDED;
			dbConfig.Env = env;
			BTreeDatabase db;
			db = BTreeDatabase.Open(testName + ".db", dbConfig, openTxn);

			// Confirm message file does not exist.
			string messageFile = testHome + "/" + "msgfile";
			Assert.AreEqual(false, File.Exists(messageFile));

			// Call set_msgfile() of env.
			env.Msgfile = messageFile;

			// Print env statistic to message file.
			env.PrintTransactionSystemStats();

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
				if (tempStr[0] == "Active transactions:")
					break;
				Assert.AreEqual(messageInfo[counter], tempStr[1]);
				counter++;
			}
			Assert.AreNotEqual(0, counter);

			openTxn.Commit();
			file.Close();
			env.Close();
		}

		/*
		 * Configure an environment. Here only configure those that could be 
		 * set before environment open.
		 */
		public void Config(XmlElement xmlElem, 
		    ref DatabaseEnvironmentConfig envConfig, bool compulsory, 
		    bool logging, bool locking, bool mutex, bool mpool, bool replication)
		{
			XmlNode childNode;

			// Configure environment without any subsystems.
			DatabaseEnvironmentConfigTest.Config(xmlElem, ref envConfig, compulsory);

			// Configure environment with logging subsystem.
			if (logging == true)
			{
				childNode = XMLReader.GetNode(xmlElem, "LogConfig");
				envConfig.LogSystemCfg = new LogConfig();
				LogConfigTest.Config((XmlElement)childNode, 
				    ref envConfig.LogSystemCfg, compulsory);
			}

			// Configure environment with locking subsystem.
			if (locking == true)
			{
				childNode = XMLReader.GetNode(xmlElem, "LockingConfig");
				envConfig.LockSystemCfg = new LockingConfig();
				LockingConfigTest.Config((XmlElement)childNode,
				    ref envConfig.LockSystemCfg, compulsory);
			}

			// Configure environment with mutex subsystem.
			if (mutex == true)
			{
				childNode = XMLReader.GetNode(xmlElem, "MutexConfig");
				envConfig.MutexSystemCfg = new MutexConfig();
				MutexConfigTest.Config((XmlElement)childNode,
				    ref envConfig.MutexSystemCfg, compulsory);
			}

			if (mpool == true)
			{
				childNode = XMLReader.GetNode(xmlElem, "MPoolConfig");
				envConfig.MPoolSystemCfg = new MPoolConfig();
				MPoolConfigTest.Config((XmlElement)childNode,
				    ref envConfig.MPoolSystemCfg, compulsory);
			}

			// Configure environment with replication.
			if (replication == true)
			{
				childNode = XMLReader.GetNode(xmlElem, "ReplicationConfig");
				envConfig.RepSystemCfg = new ReplicationConfig();
				ReplicationConfigTest.Config((XmlElement)childNode,
				    ref envConfig.RepSystemCfg, compulsory);
			}
		}

		/*
		 * Confirm the fields/properties in the environment.
		 * Those set by setting functions are not checked here.
		 */
		public static void Confirm(XmlElement xmlElement,
		    DatabaseEnvironment env, bool compulsory,
		    bool logging, bool locking, bool mutex, bool mpool,
		    bool replication)
		{
			XmlElement childElem;
			CacheInfo cacheInfo = new CacheInfo(0, 0, 0);

			// Confirm environment configuration.
			Configuration.ConfirmBool(xmlElement, "AutoCommit",
			    env.AutoCommit, compulsory);
			Configuration.ConfirmBool(xmlElement, "CDB_ALLDB",
			    env.CDB_ALLDB, compulsory);
			Configuration.ConfirmBool(xmlElement, "Create",
			    env.Create, compulsory);
			Configuration.ConfirmStringList(xmlElement, "DataDirs",
			    env.DataDirs, compulsory);
			Configuration.ConfirmString(xmlElement, "ErrorPrefix",
			    env.ErrorPrefix, compulsory);
			Configuration.ConfirmBool(xmlElement, "ForceFlush",
			    env.ForceFlush, compulsory);
			Configuration.ConfirmBool(xmlElement, "FreeThreaded",
			    env.FreeThreaded, compulsory);
			Configuration.ConfirmBool(xmlElement, "InitRegions",
			    env.InitRegions, compulsory);
			Configuration.ConfirmString(xmlElement, "IntermediateDirMode",
			    env.IntermediateDirMode, compulsory);
			Configuration.ConfirmBool(xmlElement, "Lockdown",
			    env.Lockdown, compulsory);
			Configuration.ConfirmUint(xmlElement, "LockTimeout",
			    env.LockTimeout, compulsory);
			Configuration.ConfirmUint(xmlElement, "MaxTransactions",
			    env.MaxTransactions, compulsory);
			Configuration.ConfirmBool(xmlElement, "NoBuffer",
			    env.NoBuffer, compulsory);
			Configuration.ConfirmBool(xmlElement, "NoLocking",
			    env.NoLocking, compulsory);
			Configuration.ConfirmBool(xmlElement, "NoMMap",
			    env.NoMMap, compulsory);
			Configuration.ConfirmBool(xmlElement, "NoPanic",
			    env.NoPanic, compulsory);
			Configuration.ConfirmBool(xmlElement, "Overwrite",
			    env.Overwrite, compulsory);
			Configuration.ConfirmBool(xmlElement, "Private",
			    env.Private, compulsory);
			Configuration.ConfirmBool(xmlElement, "Register",
			    env.Register, compulsory);
			Configuration.ConfirmBool(xmlElement, "RunFatalRecovery", 
			    env.RunFatalRecovery, compulsory);
			Configuration.ConfirmBool(xmlElement, "RunRecovery",
			    env.RunRecovery, compulsory);
			Configuration.ConfirmBool(xmlElement, "SystemMemory",
			    env.SystemMemory, compulsory);
			Configuration.ConfirmString(xmlElement, "TempDir",
			    env.TempDir, compulsory);
			Configuration.ConfirmBool(xmlElement, "TimeNotGranted",
			    env.TimeNotGranted, compulsory);
			Configuration.ConfirmBool(xmlElement, "TxnNoSync",
			    env.TxnNoSync, compulsory);
			Configuration.ConfirmBool(xmlElement, "TxnNoWait",
			    env.TxnNoWait, compulsory);
			Configuration.ConfirmBool(xmlElement, "TxnSnapshot",
			    env.TxnSnapshot, compulsory);
			Configuration.ConfirmDateTime(xmlElement, "TxnTimestamp", 
			    env.TxnTimestamp, compulsory);
			Configuration.ConfirmBool(xmlElement, "TxnWriteNoSync",
			    env.TxnWriteNoSync, compulsory);
			Configuration.ConfirmBool(xmlElement, "UseMVCC",
			    env.UseMVCC, compulsory);
			Configuration.ConfirmBool(xmlElement, "UseCDB",
			    env.UsingCDB, compulsory);
			Configuration.ConfirmBool(xmlElement, "UseLocking",
			    env.UsingLocking, compulsory);
			Configuration.ConfirmBool(xmlElement, "UseLogging",
			    env.UsingLogging, compulsory);
			Configuration.ConfirmBool(xmlElement, "UseMPool",
			    env.UsingMPool, compulsory);
			Configuration.ConfirmBool(xmlElement, "UseReplication",
			    env.UsingReplication, compulsory);
			Configuration.ConfirmBool(xmlElement, "UseTxns",
			    env.UsingTxns, compulsory);
			env.Verbosity = new VerboseMessages();
			Configuration.ConfirmVerboseMessages(xmlElement,
			    "Verbosity", env.Verbosity, compulsory);
			Configuration.ConfirmBool(xmlElement, "YieldCPU",
			    env.YieldCPU, compulsory);

			/*
			 * If the locking subsystem is set, check the
			 * field/properties set in LockingConfig.
			 */
			if (locking == true)
			{
				childElem = (XmlElement)XMLReader.GetNode(
				    xmlElement, "LockingConfig");
				Configuration.ConfirmByteMatrix(childElem,
				    "Conflicts", env.LockConflictMatrix,
				    compulsory);
				Configuration.ConfirmDeadlockPolicy(
				    childElem, "DeadlockResolution",
				    env.DeadlockResolution, compulsory);
				Configuration.ConfirmUint(childElem,
				    "Partitions", env.LockPartitions,
				    compulsory);
				Configuration.ConfirmUint(childElem,
				    "MaxLockers", env.MaxLockers, compulsory);
				Configuration.ConfirmUint(childElem,
				    "MaxLocks", env.MaxLocks, compulsory);
				Configuration.ConfirmUint(childElem,
				    "MaxObjects", env.MaxObjects, compulsory);
			}

			/*
			 * If the locking subsystem is set, check the
			 * field/properties set in LogConfig.
			 */
			if (logging == true)
			{
				childElem = (XmlElement)XMLReader.GetNode(
				    xmlElement, "LogConfig");
				Configuration.ConfirmBool(childElem,
				    "AutoRemove", env.LogAutoRemove,
				    compulsory);
				Configuration.ConfirmUint(childElem,
				    "BufferSize", env.LogBufferSize,
				    compulsory);
				Configuration.ConfirmString(childElem,
				    "Dir", env.LogDir, compulsory);
				Configuration.ConfirmInt(childElem,
				    "FileMode", env.LogFileMode, compulsory);
				Configuration.ConfirmBool(childElem,
				    "ForceSync", env.LogForceSync, compulsory);
				Configuration.ConfirmBool(childElem,
				    "InMemory", env.LogInMemory, compulsory);
				Configuration.ConfirmBool(childElem,
				    "LogExternalFileContent",
				    env.LogExternalFileContent,
				    compulsory);
				Configuration.ConfirmBool(childElem,
				    "NoBuffer", env.LogNoBuffer, compulsory);
				Configuration.ConfirmBool(childElem,
				    "NoSync", env.LogNoSync, compulsory);
				Configuration.ConfirmUint(childElem,
				    "RegionSize", env.LogRegionSize,
				    compulsory);
				Configuration.ConfirmBool(childElem,
				    "ZeroOnCreate", env.LogZeroOnCreate,
				    compulsory);
				Configuration.ConfirmUint(childElem,
				    "MaxFileSize", env.MaxLogFileSize,
				    compulsory);
			}

			/*
			 * If the locking subsystem is set, check the
			 * field/properties set in MutexConfig.
			 */
			if (mutex == true)
			{
				childElem = (XmlElement)XMLReader.GetNode(
				    xmlElement, "MutexConfig");
				Configuration.ConfirmUint(childElem,
				    "Alignment", env.MutexAlignment, 
				    compulsory);
				Configuration.ConfirmUint(childElem,
				    "MaxMutexes", env.MaxMutexes, compulsory);
				try
				{
					Configuration.ConfirmUint(childElem,
					    "Increment", env.MutexIncrement,
					    compulsory);
				}
				catch (AssertionException)
				{
					Assert.AreEqual(0, env.MutexIncrement);
				}

				Configuration.ConfirmUint(childElem,
				    "NumTestAndSetSpins",
				    env.NumTestAndSetSpins, compulsory);
			}

			if (mpool == true)
			{
				childElem = (XmlElement)XMLReader.GetNode(
				    xmlElement, "MPoolConfig");
				Configuration.ConfirmCacheSize(childElem,
				    "CacheSize", env.CacheSize, compulsory);
				if (env.UsingMPool == false)
					Configuration.ConfirmCacheSize(childElem,
					    "MaxCacheSize", env.MaxCacheSize, compulsory);
				Configuration.ConfirmInt(childElem,
				    "MaxOpenFiles", env.MaxOpenFiles, compulsory);
				Configuration.ConfirmMaxSequentialWrites(childElem, 
				    "MaxSequentialWrites", env.SequentialWritePause,
				    env.MaxSequentialWrites, compulsory);
				Configuration.ConfirmUint(childElem,
				    "MMapSize", env.MMapSize, compulsory);
			}

			if (replication == true)
			{
				childElem = (XmlElement)XMLReader.GetNode(
				    xmlElement, "ReplicationConfig");
				Configuration.ConfirmUint(childElem,
				    "AckTimeout", env.RepAckTimeout, compulsory);
				Configuration.ConfirmBool(childElem,
				    "BulkTransfer", env.RepBulkTransfer, compulsory);
				Configuration.ConfirmUint(childElem,
				    "CheckpointDelay", env.RepCheckpointDelay, compulsory);
				Configuration.ConfirmUint(childElem,
				    "ConnectionRetry", env.RepConnectionRetry, compulsory);
				Configuration.ConfirmBool(childElem,
				    "DelayClientSync", env.RepDelayClientSync, compulsory);
				Configuration.ConfirmUint(childElem,
				    "ElectionRetry", env.RepElectionRetry, compulsory);
				Configuration.ConfirmUint(childElem,
				    "ElectionTimeout", env.RepElectionTimeout, compulsory);
				Configuration.ConfirmUint(childElem,
				    "FullElectionTimeout", env.RepFullElectionTimeout,compulsory);
				Configuration.ConfirmUint(childElem,
				    "HeartbeatMonitor", env.RepHeartbeatMonitor, compulsory);
				Configuration.ConfirmUint(childElem,
				    "HeartbeatSend", env.RepHeartbeatSend, compulsory);
				Configuration.ConfirmUint(childElem,
				    "LeaseTimeout", env.RepLeaseTimeout, compulsory);
				Configuration.ConfirmBool(childElem,
				    "AutoInit", env.RepAutoInit, compulsory);
				Configuration.ConfirmBool(childElem,
				    "NoBlocking", env.RepNoBlocking, compulsory);
				Configuration.ConfirmUint(childElem,
				    "Priority", env.RepPriority, compulsory);
				Configuration.ConfirmAckPolicy(childElem,
				    "RepMgrAckPolicy", env.RepMgrAckPolicy, compulsory);
				Configuration.ConfirmBool(childElem,
				    "Strict2Site", env.RepStrict2Site, compulsory);
				Configuration.ConfirmBool(childElem,
				    "UseMasterLeases", env.RepUseMasterLeases, compulsory);
			}
		}
	}
}
