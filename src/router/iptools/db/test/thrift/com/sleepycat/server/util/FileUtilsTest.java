/*-
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 2002, 2017 Oracle and/or its affiliates.  All rights reserved.
 *
 * $Id$
 */

package com.sleepycat.server.util;

import com.sleepycat.server.config.EnvDirType;
import org.junit.After;
import org.junit.Before;
import org.junit.Test;

import java.io.File;
import java.io.IOException;
import java.nio.file.Files;
import java.nio.file.Path;
import java.util.HashMap;
import java.util.Map;

import static org.hamcrest.CoreMatchers.is;
import static org.hamcrest.CoreMatchers.notNullValue;
import static org.hamcrest.CoreMatchers.nullValue;
import static org.hamcrest.MatcherAssert.assertThat;

public class FileUtilsTest {

    private Path testRoot;

    private Map<EnvDirType, File> dirs;

    @Before
    public void setUp() throws Exception {
        testRoot = Files.createTempDirectory("FileUtilsTest");
        dirs = new HashMap<>();
        dirs.put(EnvDirType.HOME, testRoot.toFile());
    }

    @After
    public void tearDown() throws Exception {
        FileUtils.deleteFileTree(testRoot.toFile());
    }

    @Test
    public void testCheckAndNormalizePath() throws Exception {
        assertThat(FileUtils.checkAndNormalize("ok"),
                is("ok"));
        assertThat(FileUtils.checkAndNormalize("parent/child"),
                is("parent/child"));
        assertThat(FileUtils.checkAndNormalize("parent/../sibling"),
                is("sibling"));
        assertThat(FileUtils.checkAndNormalize("./parent/./a/b/."),
                is("parent/a/b"));
        assertThat(FileUtils.checkAndNormalize("parent/a/../b/c/../d"),
                is("parent/b/d"));
        assertThat(FileUtils.checkAndNormalize("a/b/./../c/./d/../e/.."),
                is("a/c"));
    }

    @Test(expected = IllegalArgumentException.class)
    public void testCheckAndNormalizePathWithAbsolutePath() {
        FileUtils.checkAndNormalize("/absolute/is/bad/");
    }

    @Test(expected = IllegalArgumentException.class)
    public void testCheckAndNormalizePathWithEmptyPath() {
        FileUtils.checkAndNormalize(".");
    }

    @Test(expected = IllegalArgumentException.class)
    public void testCheckAndNormalizePathWithEscapePath() {
        FileUtils.checkAndNormalize("cannot/escape/../../../parent");
    }

    @Test
    public void testDeleteFileTree() throws Exception {
        Path tmp = Files.createTempDirectory(testRoot, null);

        Files.createTempFile(tmp, null, null);
        Files.createTempFile(tmp, null, null);
        Files.createTempDirectory(tmp, null);
        Path child = Files.createTempDirectory(tmp, null);
        Files.createTempFile(child, null, null);
        Files.createTempFile(child, null, null);

        FileUtils.deleteFileTree(tmp.toFile());
        assertThat(Files.notExists(tmp), is(true));
    }

    @Test
    public void testDeleteFileTreeNotExist() throws Exception {
        FileUtils.deleteFileTree(testRoot.resolve("notExist").toFile());
    }

    @Test
    public void testResolveDirectories() throws Exception {
        Path child = testRoot.resolve("child");

        assertThat(FileUtils.resolveDirectories(dirs, "child", true)
                .get(EnvDirType.HOME).toPath(), is(child));

        Files.createDirectory(child);

        assertThat(FileUtils.resolveDirectories(dirs, "child", false).get(
                EnvDirType.HOME).toPath(), is(child));
    }

    @Test(expected = IOException.class)
    public void testResolveDirectoriesNotExist() throws Exception {
        FileUtils.resolveDirectories(dirs, "child", false);
    }

    @Test(expected = IOException.class)
    public void testResolveDirectoriesNotDir() throws Exception {
        Files.createFile(testRoot.resolve("child"));
        FileUtils.resolveDirectories(dirs, "child", true);
    }

    @Test
    public void testResolveFile() throws Exception {
        File root = testRoot.toFile();

        assertThat(FileUtils.resolveFile(root, "notExist", false), nullValue());
        assertThat(FileUtils.resolveFile(root, "notExist", true),
                notNullValue());

        Files.createDirectory(testRoot.resolve("dir"));
        assertThat(FileUtils.resolveFile(root, "dir", true), nullValue());

        Files.createFile(testRoot.resolve("file"));
        assertThat(FileUtils.resolveFile(root, "file", false), notNullValue());
    }
}