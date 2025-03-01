/*-
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 2002, 2017 Oracle and/or its affiliates.  All rights reserved.
 *
 * $Id$
 */

package com.sleepycat.sample;

import com.sleepycat.sample.data.Ticket;
import com.sleepycat.sample.dbinterface.DbManager;
import com.sleepycat.sample.park.Meter;
import com.sleepycat.sample.park.Reporting;

import java.text.SimpleDateFormat;
import java.util.*;

/**
 * The parking demo application.
 * <p/>
 * This demo application simulates a parking lot with one parking meter:
 * 1. A parking meter is created.
 * 2. A few cars arrive at the parking lot and the parking meter issues a
 * parking ticket for each car.
 * 3. Some of the parked cars leave the parking lot and the parking meter
 * calculates the parking fee for each car.
 * 4. At the end of the day, the parking lot manager runs a few analysis
 * about what happened in the day.
 */
public class ParkingDemo {
	/* The home directory for the database environment. */
	private static final String ENV_HOME = "parking_demo";

	public static void main(String[] args) throws Exception {
		// Create a helper factory object
		DbManagerFactory factory =
				new DbManagerFactory(ENV_HOME, DbManagerFactory.API.SQL);

		// Setup the database: create database files/tables, indexes, etc.
		try (DbManager mgr = factory.createManager()) {
			mgr.setupDb();
		}

		// Create a parking meter and run the simulated events
		System.out.println("======= Simulation starts =======");
		Map<String, Ticket> ticketMap = new HashMap<>();
		try (DbManager mgr = factory.createManager();
		     Meter meter = new Meter("meter", mgr)) {
			for (DemoData.SimEvent e : DemoData.getEventsSortedByTime()) {
				if (e.isPark) {
					Ticket t = meter.park(new Date(e.time));
					ticketMap.put(e.carName, t);
					System.out.println(e.carName + " has entered the parking" +
							"lot and got ticket " + t.getTicketId());
				} else {
					Ticket t = ticketMap.get(e.carName);
					int fee = meter.depart(t, new Date(e.time));
					System.out.println(e.carName + " has left the parking lot" +
							" and paid " + fee / 100 + " dollar(s) " +
							fee % 100 + " cent(s)");
				}
			}
		}
		System.out.println("======= Simulation ends =======");

		// Generate a few reports at the end of the simulation
		SimpleDateFormat fmt = new SimpleDateFormat("yyyy-MM-dd'T'HH:mm:ss");
		Date from = fmt.parse("2014-12-20T00:00:00");
		Date until = fmt.parse("2014-12-21T00:00:00");
		try (DbManager mgr = factory.createManager();
		     Reporting rpt = new Reporting(mgr)) {
			int carsParked = rpt.totalCarsParkedDuring(
					from.getTime(), until.getTime(), "meter");
			int feesCollected = rpt.totalFeesCollectedDuring(
					from.getTime(), until.getTime(), "meter");
			System.out.println("Total number of cars entered on 2014-12-20: " +
					carsParked);
			System.out.println("Total fees collected on 2014-12-20: " +
					feesCollected / 100 + " dollar(s) " +
					feesCollected % 100 + " cent(s).");
		}

	}
}

/**
 * DemoData class holds the parking data for the demo application.
 */
class DemoData {
	/*
	 * The raw parking data, in the following form:
	 * {<car name>, <park time>, <depart time>}
	 */
	private static String[][] data = {
			{"car1", "2014-12-20T01:12:00", "2014-12-20T21:10:15"},
			{"car2", "2014-12-20T02:03:25", "2014-12-20T03:12:50"},
			{"car3", "2014-12-20T03:05:12", null},
			{"car4", "2014-12-19T21:02:14", "2014-12-20T00:02:10"},
			{"car5", "2014-12-20T22:05:37", "2014-12-21T02:05:32"}
	};

	/* Convert the raw data to SimEvents and sort them by time. */
	public static List<SimEvent> getEventsSortedByTime() throws Exception {
		SimpleDateFormat fmt = new SimpleDateFormat("yyyy-MM-dd'T'HH:mm:ss");
		List<SimEvent> events = new ArrayList<>(data.length * 2);

		for (String[] row : data) {
			events.add(new SimEvent(
					row[0], fmt.parse(row[1]).getTime(), true));
			if (row[2] != null) {
				events.add(new SimEvent(
						row[0], fmt.parse(row[2]).getTime(), false));
			}
		}

		Collections.sort(events);

		return events;
	}

	/* The structure for simulation events. */
	public static class SimEvent implements Comparable<SimEvent> {
		/* The car name. */
		String carName;
		/* The time of the event. */
		Long time;
		/* The event: true for park; false for depart. */
		boolean isPark;

		public SimEvent(String carName, Long time, boolean isPark) {
			this.carName = carName;
			this.time = time;
			this.isPark = isPark;
		}

		@Override
		public int compareTo(SimEvent o) {
			return time.compareTo(o.time);
		}
	}
}
