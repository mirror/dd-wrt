/*-
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 2002, 2017 Oracle and/or its affiliates.  All rights reserved.
 *
 * $Id$
 */

package com.sleepycat.server;

import com.sleepycat.server.config.BdbServerCmdOptions;
import com.sleepycat.server.config.BdbServerConfig;
import com.sleepycat.server.config.BdbServiceConfig;
import com.sleepycat.server.util.FileUtils;
import com.sleepycat.thrift.BdbService;
import org.apache.log4j.xml.DOMConfigurator;
import org.apache.thrift.TProcessor;
import org.apache.thrift.TProcessorFactory;
import org.apache.thrift.server.TServer;
import org.apache.thrift.transport.TTransportException;

import java.io.FileReader;
import java.io.IOException;
import java.io.Reader;
import java.net.UnknownHostException;
import java.nio.file.attribute.PosixFilePermission;
import java.util.Properties;

import static java.lang.System.err;
import static java.lang.System.out;

/**
 * The server program.
 */
public class BdbServer {
    /** Version number. */
    public static final String VERSION = "6.2.32";

    public static void main(String[] args) {
        try {
            BdbServerCmdOptions options = BdbServerCmdOptions.parse(args);

            DOMConfigurator.configure(options.getLog4jConfig());

            Properties properties = loadProperties(options.getConfigFile());
            BdbServerConfig serverConfig = new BdbServerConfig(properties);
            BdbServiceConfig serviceConfig = new BdbServiceConfig(properties);

            serviceConfig.initRootDirs();

            BdbServiceHandler handler = new BdbServiceHandler();
            TServer server = createServer(serverConfig, handler);

            handler.setServerAndConfig(server, serviceConfig);
            shutdownHook(handler);

            out.println("Starting to listen on " + serverConfig.getPort());
            server.serve();
        } catch (Exception e) {
            exit("", e);
        }
    }

    private static Properties loadProperties(String file) {
        Properties properties = new Properties();
        try (Reader r = new FileReader(file)) {
            if (FileUtils.hasAnyPermission(file,
                    PosixFilePermission.OTHERS_EXECUTE,
                    PosixFilePermission.OTHERS_READ,
                    PosixFilePermission.OTHERS_WRITE)) {
                exit("The properties file '" + file +
                        "' must not be globally accessible.", null);
            }

            properties.load(r);
        } catch (IOException e) {
            exit("Failed to load '" + file + "'. ", e);
        }
        return properties;
    }

    private static TServer createServer(BdbServerConfig config,
            BdbServiceHandler handler) {
        TProcessor processor = new BdbService.Processor<>(handler);
        TProcessorFactory factory = new TProcessorFactory(processor);
        TServer server = null;
        try {
            server = config.createServer(factory);
        } catch (TTransportException | UnknownHostException e) {
            exit("Failed to start server. ", e);
        }
        return server;
    }

    private static void shutdownHook(BdbServiceHandler handler) {
        Runtime.getRuntime().addShutdownHook(new Thread(handler::shutdown));
    }

    private static void exit(String msg, Exception e) {
        err.println(msg + (e == null ? "" : e.getMessage()));
        System.exit(-1);
    }
}
