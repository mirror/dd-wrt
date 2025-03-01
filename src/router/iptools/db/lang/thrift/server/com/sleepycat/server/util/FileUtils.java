/*-
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 2002, 2017 Oracle and/or its affiliates.  All rights reserved.
 *
 * $Id$
 */

package com.sleepycat.server.util;

import com.sleepycat.server.config.EnvDirType;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import java.io.File;
import java.io.IOException;
import java.nio.file.FileVisitResult;
import java.nio.file.Files;
import java.nio.file.Path;
import java.nio.file.Paths;
import java.nio.file.SimpleFileVisitor;
import java.nio.file.attribute.BasicFileAttributes;
import java.nio.file.attribute.PosixFilePermission;
import java.util.Arrays;
import java.util.Collection;
import java.util.HashMap;
import java.util.Map;
import java.util.Set;
import java.util.function.Predicate;

/**
 * Collection of utilities for file operations.
 */
public class FileUtils {
    private static final Logger logger =
            LoggerFactory.getLogger(FileUtils.class);

    private static final Path ROOT = Paths.get("/");

    /**
     * Return true if the specified file has any of the specified permissions.
     *
     * @param file the path of the file
     * @param permissions the permissions to check
     * @return true if the specified file has any of the specified permissions
     * @throws IOException on any I/O error
     */
    public static boolean hasAnyPermission(String file,
            PosixFilePermission... permissions) throws IOException {
        Set<PosixFilePermission> filePermissions =
                Files.getPosixFilePermissions(Paths.get(file));
        filePermissions.retainAll(Arrays.asList(permissions));
        return !filePermissions.isEmpty();
    }

    /**
     * Check if the specified path is valid and return its normalized path. A
     * path is valid if it is relative and does not escape its parent
     * directory. Throw IllegalArgumentException if the path is invalid.
     *
     * @param path a path to check and normalize
     * @return a normalized path
     * @throws IllegalArgumentException if the path is invalid
     */
    public static String checkAndNormalize(String path)
            throws IllegalArgumentException {
        Path normPath = Paths.get(path).normalize();
        if (normPath.isAbsolute()) {
            throw new IllegalArgumentException("Absolute path is not allowed.");
        }
        if (normPath.toString().trim().isEmpty()) {
            throw new IllegalArgumentException("Empty path is not allowed.");
        }

        /*
         * Resolve the normalized path against root and then relativize it back.
         * This will remove leading ".." because the parent of root is still
         * root.
         */
        Path relativePath = ROOT.relativize(ROOT.resolve(normPath).normalize());

        /*
         * If we cannot get back to the normalized path, the normalized path
         * must have leading "..". This is not allowed because it would escape
         * the parent directory when resolved against a non-root path.
         */
        if (!normPath.equals(relativePath)) {
            throw new IllegalArgumentException(
                    "Paths that escape parent is not allowed.");
        }

        return normPath.toString();
    }

    /**
     * Resolve the child path against each parent in the map. Check if every
     * resolved file is a directory and return the resolved directories.
     *
     * @param parents the parent paths
     * @param child the child path
     * @param allowCreate if the resolved directories are allowed to be created
     * @return the resolved directory files
     * @throws IOException if any resolved path is not a directory
     */
    public static Map<EnvDirType, File> resolveDirectories(
            Map<EnvDirType, File> parents, String child, boolean allowCreate)
            throws IOException {
        String normalized = checkAndNormalize(child);

        Map<EnvDirType, File> resolved = new HashMap<>();
        for (EnvDirType type : parents.keySet()) {
            File f = new File(parents.get(type), normalized);
            if (isType(f, FileType.directory, allowCreate)) {
                resolved.put(type, f);
            } else {
                throw new IOException("Cannot access " + type + " directory.");
            }
        }

        return resolved;
    }

    /**
     * Resolve the child path against the parent and return the resolved File
     * if it is a directory. Return null if the resolved path is not a
     * directory.
     *
     * @param parent the parent File
     * @param child the child path
     * @return a directory File or null if the resolved path is not a directory
     */
    public static File resolveDirectory(File parent, String child) {
        return resolve(parent, child, FileType.directory, false);
    }

    /**
     * Resolve the child path against the parent and return the resolved File
     * if it is a regular file. Return null if the resolved path is not a
     * regular file.
     *
     * @param parent the parent File
     * @param child the child path
     * @param allowCreate if the resolved File is allowed to be created
     * @return a regular File or null if the resolved path is not a regular file
     */
    public static File resolveFile(File parent, String child,
            boolean allowCreate) {
        return resolve(parent, child, FileType.file, allowCreate);
    }

    private static File resolve(File parent, String child, FileType type,
            boolean allowCreate) {
        File resolved = new File(parent, checkAndNormalize(child));
        return isType(resolved, type, allowCreate) ? resolved : null;
    }

    private static boolean isType(File file, FileType type,
            boolean allowCreate) {
        Predicate<File> typeCheck =
                type == FileType.file ? File::isFile : File::isDirectory;

        boolean isValid = file.exists() && typeCheck.test(file);
        boolean canCreate = allowCreate && !file.exists();

        if (isValid || canCreate) {
            return true;
        }

        if (file.exists()) {
            logger.info(file.getPath() + "is not a " + type + ".");
        } else {
            logger.info(file.getPath() + "does not exist.");
        }
        return false;
    }

    /**
     * Create a collection of directories and their parent directories.
     *
     * @param dirs directories to be created
     * @throws IOException if any directory cannot be created
     */
    public static void createDirectories(Collection<File> dirs)
            throws IOException {
        try {
            for (File dir : dirs) {
                Files.createDirectories(dir.toPath());
            }
        } catch (IOException e) {
            logger.info("Cannot create directory", e);
            throw new IOException("Cannot access directory.", e);
        }
    }

    /**
     * Delete an entire file tree rooted at a given starting file.
     *
     * @param start the starting file
     * @throws IOException on I/O error
     */
    public static void deleteFileTree(File start) throws IOException {
        if (!start.exists()) {
            return;
        }
        Files.walkFileTree(start.toPath(), new SimpleFileVisitor<Path>() {
            @Override
            public FileVisitResult visitFile(Path file,
                    BasicFileAttributes attributes) throws IOException {
                Files.delete(file);
                return FileVisitResult.CONTINUE;
            }

            @Override
            public FileVisitResult postVisitDirectory(Path dir, IOException exc)
                    throws IOException {
                if (exc == null) {
                    Files.delete(dir);
                    return FileVisitResult.CONTINUE;
                }
                throw exc;
            }
        });
    }

    private enum FileType {file, directory}
}
