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
using System.Xml;
using NUnit.Framework;
using BerkeleyDB;

namespace CsharpAPITest
{
	[TestFixture]
	public class LogConfigTest : CSharpTestFixture
	{

		[TestFixtureSetUp]
		public void SetUpTestFixture()
		{
			testFixtureName = "LogConfigTest";
			base.SetUpTestfixture();
		}

		[Test]
		public void TestConfig()
		{
			testName = "TestConfig";
			SetUpTest(false);
			/* 
			 * Configure the fields/properties and see if 
			 * they are updated successfully.
			 */
			LogConfig logConfig = new LogConfig();
			XmlElement xmlElem = Configuration.TestSetUp(testFixtureName, testName);
			Config(xmlElem, ref logConfig, true);
			Confirm(xmlElem, logConfig, true);
		}

		[Test, ExpectedException(typeof(ExpectedTestException))]
		public void TestFullLogBufferException()
		{
			testName = "TestFullLogBufferException";
			SetUpTest(true);

			// Open an environment and configured log subsystem.
			DatabaseEnvironmentConfig cfg =
			    new DatabaseEnvironmentConfig();
			cfg.Create = true;
			cfg.TxnNoSync = true;
			cfg.UseTxns = true;
			cfg.UseLocking = true;
			cfg.UseMPool = true;
			cfg.UseLogging = true;
			cfg.LogSystemCfg = new LogConfig();
			cfg.LogSystemCfg.AutoRemove = false;
			cfg.LogSystemCfg.BufferSize = 409600;
			cfg.LogSystemCfg.MaxFileSize = 10480;
			cfg.LogSystemCfg.NoBuffer = false;
			cfg.LogSystemCfg.NoSync = true;
			cfg.LogSystemCfg.ZeroOnCreate = true;
			cfg.LogSystemCfg.InMemory = true;
			cfg.LogSystemCfg.LogExternalFileContent = true;
			DatabaseEnvironment env = DatabaseEnvironment.Open(testHome, cfg);

			BTreeDatabase db;
			try
			{
				Transaction openTxn = env.BeginTransaction();
				try
				{
					BTreeDatabaseConfig dbConfig =
					    new BTreeDatabaseConfig();
					dbConfig.Creation = CreatePolicy.IF_NEEDED;
					dbConfig.Env = env;
					db = BTreeDatabase.Open(testName + ".db", dbConfig, openTxn);
					openTxn.Commit();
				}
				catch (DatabaseException e)
				{
					openTxn.Abort();
					throw e;
				}

				Transaction writeTxn = env.BeginTransaction();
				try
				{
					/*
					 * Writing 10 large records into in-memory logging 
					 * database should throw FullLogBufferException since 
					 * the amount of put data is larger than buffer size.
					 */
					byte[] byteArr = new byte[204800];
					for (int i = 0; i < 10; i++)
						db.Put(new DatabaseEntry(BitConverter.GetBytes(i)),
						    new DatabaseEntry(byteArr), writeTxn);
					writeTxn.Commit();
				}
				catch (Exception e)
				{
					writeTxn.Abort();
					throw e;
				}
				finally
				{
					db.Close(true);
				}
			}
			catch (FullLogBufferException e)
			{
				Assert.AreEqual(ErrorCodes.DB_LOG_BUFFER_FULL, e.ErrorCode);
				throw new ExpectedTestException();
			}
			finally
			{
				env.Close();
			}
		}

		[Test]
		public void TestLoggingSystemStats()
		{
			testName = "TestLoggingSystemStats";
			SetUpTest(true);
			string logDir = "./";

			Directory.CreateDirectory(testHome + "/" + logDir);

			DatabaseEnvironmentConfig cfg =
			    new DatabaseEnvironmentConfig();
			cfg.Create = true;
			cfg.UseTxns = true;
			cfg.AutoCommit = true;
			cfg.UseLocking = true;
			cfg.UseMPool = true;
			cfg.UseLogging = true;
			cfg.MPoolSystemCfg = new MPoolConfig();
			cfg.MPoolSystemCfg.CacheSize = new CacheInfo(0, 1048576, 1);

			cfg.LogSystemCfg = new LogConfig();
			cfg.LogSystemCfg.AutoRemove = false;
			cfg.LogSystemCfg.BufferSize = 10240;
			cfg.LogSystemCfg.Dir = logDir;
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

			LogStats stats = env.LoggingSystemStats();
			env.Msgfile = testHome + "/" + testName+ ".log";
			env.PrintLoggingSystemStats();
			Assert.AreEqual(10240, stats.BufferSize);
			Assert.AreEqual(1, stats.CurrentFile);
			Assert.AreNotEqual(0, stats.CurrentOffset);
			Assert.AreEqual(0, stats.FileId);
			Assert.AreEqual(1048576, stats.FileSize);
			Assert.AreEqual(0, stats.InitFileId);
			Assert.AreNotEqual(0, stats.MagicNumber);
			Assert.AreEqual(0, stats.MaxFileId);
			Assert.AreNotEqual(0, stats.PermissionsMode);
			Assert.AreEqual(1, stats.Records);
			Assert.AreNotEqual(0, stats.RegionLockNoWait);
			Assert.LessOrEqual(204800, stats.RegionSize);
			Assert.AreNotEqual(0, stats.Version);

			Transaction openTxn = env.BeginTransaction();
			BTreeDatabaseConfig dbConfig = new BTreeDatabaseConfig();
			dbConfig.Creation = CreatePolicy.IF_NEEDED;
			dbConfig.Env = env;
			BTreeDatabase db = BTreeDatabase.Open(testName + ".db", dbConfig, openTxn);
			openTxn.Commit();

			Transaction writeTxn = env.BeginTransaction();
			byte[] byteArr = new byte[1024];
			for (int i = 0; i < 1000; i++)
				db.Put(new DatabaseEntry(BitConverter.GetBytes(i)),
				    new DatabaseEntry(byteArr), writeTxn);
			writeTxn.Commit();

			stats = env.LoggingSystemStats();
			Assert.AreNotEqual(0, stats.Bytes);
			Assert.AreNotEqual(0, stats.BytesSinceCheckpoint);
			Assert.AreNotEqual(0, stats.DiskFileNumber);
			Assert.AreNotEqual(0, stats.DiskOffset);
			Assert.AreNotEqual(0, stats.MaxCommitsPerFlush);
			Assert.AreNotEqual(0, stats.MBytes);
			Assert.AreNotEqual(0, stats.MBytesSinceCheckpoint);
			Assert.AreNotEqual(0, stats.MinCommitsPerFlush);
			Assert.AreNotEqual(0, stats.OverflowWrites);
			Assert.AreEqual(0, stats.Syncs);
			Assert.AreNotEqual(0, stats.Writes);
			Assert.AreEqual(0, stats.Reads);
			Assert.AreEqual(0, stats.RegionLockWait);

			stats = env.LoggingSystemStats(true);
			stats = env.LoggingSystemStats();
			Assert.AreEqual(0, stats.Bytes);
			Assert.AreEqual(0, stats.BytesSinceCheckpoint);
			Assert.AreEqual(0, stats.MaxCommitsPerFlush);
			Assert.AreEqual(0, stats.MBytes);
			Assert.AreEqual(0, stats.MBytesSinceCheckpoint);
			Assert.AreEqual(0, stats.MinCommitsPerFlush);
			Assert.AreEqual(0, stats.OverflowWrites);
			Assert.AreEqual(0, stats.Syncs);
			Assert.AreEqual(0, stats.Writes);
			Assert.AreEqual(0, stats.Reads);

			env.PrintLoggingSystemStats(true, true);

			db.Close();
			env.Close();
		}
	
		[Test]
		public void TestLogStatPrint()
		{
			testName = "TestLogStatPrint";
			SetUpTest(true);

			string[] messageInfo = new string[]
			{
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
			  "The number of region locks that required waiting (0%)"
			};

			string logDir = "./";
			Directory.CreateDirectory(testHome + "/" + logDir);

			// Configure and open an environment.
			DatabaseEnvironmentConfig envConfig =
			    new DatabaseEnvironmentConfig();
			envConfig.Create = true;
			envConfig.UseTxns = true;

			envConfig.LogSystemCfg = new LogConfig();
			envConfig.LogSystemCfg.AutoRemove = false;
			envConfig.LogSystemCfg.BufferSize = 10240;
			envConfig.LogSystemCfg.Dir = logDir;
			envConfig.LogSystemCfg.FileMode = 755;
			envConfig.LogSystemCfg.ForceSync = true;
			envConfig.LogSystemCfg.InMemory = false;
			envConfig.LogSystemCfg.LogExternalFileContent = false;
			envConfig.LogSystemCfg.MaxFileSize = 1048576;
			envConfig.LogSystemCfg.NoBuffer = false;
			envConfig.LogSystemCfg.NoSync = true;
			envConfig.LogSystemCfg.RegionSize = 204800;
			envConfig.LogSystemCfg.ZeroOnCreate = true;

			DatabaseEnvironment env =
			    DatabaseEnvironment.Open(testHome, envConfig);

			// Confirm message file does not exist.
			string messageFile = testHome + "/" + "msgfile";
			Assert.AreEqual(false, File.Exists(messageFile));

			// Call set_msgfile() of env.
			env.Msgfile = messageFile;

			// Print env statistic to message file.
			env.PrintLoggingSystemStats();

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

		[Test]
		public void TestLsn()
		{
			testName = "TestLsn";
			SetUpTest(true);

			LSN lsn = new LSN(12, 411);
			Assert.AreEqual(12, lsn.LogFileNumber);
			Assert.AreEqual(411, lsn.Offset);

			LSN newLsn = new LSN(15, 410);
			Assert.AreEqual(0, LSN.Compare(lsn, lsn));
			Assert.Greater(0, LSN.Compare(lsn, newLsn));
		}
		
		public static void Confirm(XmlElement
		    xmlElement, LogConfig logConfig, bool compulsory)
		{
			Configuration.ConfirmBool(xmlElement, "AutoRemove",
			    logConfig.AutoRemove, compulsory);
			Configuration.ConfirmUint(xmlElement, "BufferSize",
			    logConfig.BufferSize, compulsory);
			Configuration.ConfirmString(xmlElement, "Dir",
			    logConfig.Dir, compulsory);
			Configuration.ConfirmInt(xmlElement, "FileMode",
			    logConfig.FileMode, compulsory);
			Configuration.ConfirmBool(xmlElement, "ForceSync",
			    logConfig.ForceSync, compulsory);
			Configuration.ConfirmBool(xmlElement, "InMemory",
			    logConfig.InMemory, compulsory);
			Configuration.ConfirmBool(xmlElement,
			    "LogExternalFileContent",
			    logConfig.LogExternalFileContent, compulsory);
			Configuration.ConfirmUint(xmlElement, "MaxFileSize",
			    logConfig.MaxFileSize, compulsory);
			Configuration.ConfirmBool(xmlElement, "NoBuffer",
			    logConfig.NoBuffer, compulsory);
			Configuration.ConfirmBool(xmlElement, "NoSync",
			    logConfig.NoSync, compulsory);
			Configuration.ConfirmUint(xmlElement, "RegionSize",
			    logConfig.RegionSize, compulsory);
			Configuration.ConfirmBool(xmlElement, "ZeroOnCreate",
			    logConfig.ZeroOnCreate, compulsory);
		}

		public static void Config(XmlElement
		    xmlElement, ref LogConfig logConfig, bool compulsory)
		{
			uint uintValue = new uint();
			int intValue = new int();

			Configuration.ConfigBool(xmlElement, "AutoRemove",
			    ref logConfig.AutoRemove, compulsory);
			if (Configuration.ConfigUint(xmlElement, "BufferSize",
			    ref uintValue, compulsory))
				logConfig.BufferSize = uintValue;
			Configuration.ConfigString(xmlElement, "Dir",
			    ref logConfig.Dir, compulsory);
			if (Configuration.ConfigInt(xmlElement, "FileMode",
			    ref intValue, compulsory))
				logConfig.FileMode = intValue;
			Configuration.ConfigBool(xmlElement, "ForceSync",
			    ref logConfig.ForceSync, compulsory);
			Configuration.ConfigBool(xmlElement, "InMemory",
			    ref logConfig.InMemory, compulsory);
			Configuration.ConfigBool(xmlElement,
			    "LogExternalFileContent",
			    ref logConfig.LogExternalFileContent, compulsory);
			if (Configuration.ConfigUint(xmlElement, "MaxFileSize",
			    ref uintValue, compulsory))
				logConfig.MaxFileSize = uintValue;
			Configuration.ConfigBool(xmlElement, "NoBuffer",
			    ref logConfig.NoBuffer, compulsory);
			Configuration.ConfigBool(xmlElement, "NoSync",
			    ref logConfig.NoSync, compulsory);
			if (Configuration.ConfigUint(xmlElement, "RegionSize",
			    ref uintValue, compulsory))
				logConfig.RegionSize = uintValue;
			Configuration.ConfigBool(xmlElement, "ZeroOnCreate",
			    ref logConfig.ZeroOnCreate, compulsory);
		}
	}
}
