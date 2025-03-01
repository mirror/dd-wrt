/*-
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 2002, 2017 Oracle and/or its affiliates.  All rights reserved.
 *
 * $Id$
 */

package com.sleepycat.server.config;

import com.sleepycat.server.BdbServer;

import java.util.HashMap;
import java.util.Map;
import java.util.function.Consumer;
import java.util.function.Function;

/**
 * A helper class for handling command line options for BdbServer.
 */
public class BdbServerCmdOptions {

    /* Supported options. */
    private static final String HELP = "-h";
    private static final String VERSION = "-v";
    private static final String CONFIG_FILE = "-config-file";
    private static final String LOG4J_CONFIG = "-log-config";

    /** Handlers for options without arguments. */
    private static Map<String, Consumer<BdbServerCmdOptions>> handlers;

    /** Parsers for options with a single argument. */
    private static Map<String, Function<String, ?>> parsers;

    static {
        handlers = new HashMap<>();
        handlers.put(HELP, BdbServerCmdOptions::printUsageAndExit);
        handlers.put(VERSION, BdbServerCmdOptions::printVersionAndExit);

        parsers = new HashMap<>();
        parsers.put(CONFIG_FILE, Function.identity());
        parsers.put(LOG4J_CONFIG, Function.identity());
    }

    /** Parsed option values. */
    private Map<String, Object> options = new HashMap<>();

    /* Hide constructor. */
    private BdbServerCmdOptions() {
        this.options.put(CONFIG_FILE, "bdb.properties");
        this.options.put(LOG4J_CONFIG, "log4j.xml");
    }

    /**
     * Parse the given command line arguments.
     *
     * @param args the command line arguments
     * @return the parsed BdbServerCmdOptions
     */
    public static BdbServerCmdOptions parse(String[] args) {
        BdbServerCmdOptions cmdOptions = new BdbServerCmdOptions();
        cmdOptions.parseArgs(args);
        return cmdOptions;
    }

    private void parseArgs(String[] args) {
        int i = 0;
        while (i < args.length) {
            if (handlers.containsKey(args[i])) {
                handlers.get(args[i]).accept(this);
                i++;
            } else if (parsers.containsKey(args[i])) {
                if ((i + 1) < args.length) {
                    Function<String, ?> parser = parsers.get(args[i]);
                    this.options.put(args[i], parser.apply(args[i + 1]));
                    i += 2;
                } else {
                    System.err.println(args[i] + " requires an argument.\n");
                    printUsageAndExit();
                }
            } else {
                System.err.println("Unknown option: " + args[i]);
                printUsageAndExit();
            }
        }
    }

    private void printVersionAndExit() {
        System.out.println("Oracle Berkeley DB Server " + BdbServer.VERSION);
        System.exit(0);
    }

    private void printUsageAndExit() {
        System.out.println("BdbServer [-v] [-h]\n" +
                "\t[" + CONFIG_FILE + " <server config file>]\n" +
                "\t[" + LOG4J_CONFIG + " <log4j config file>]");
        System.exit(0);
    }

    public String getConfigFile() {
        return (String) this.options.get(CONFIG_FILE);
    }

    public String getLog4jConfig() {
        return (String) this.options.get(LOG4J_CONFIG);
    }
}
