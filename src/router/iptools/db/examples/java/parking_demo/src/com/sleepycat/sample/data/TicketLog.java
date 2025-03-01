/*-
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 2002, 2017 Oracle and/or its affiliates.  All rights reserved.
 *
 * $Id$
 */

package com.sleepycat.sample.data;

import com.sleepycat.persist.model.Entity;
import com.sleepycat.persist.model.PrimaryKey;
import com.sleepycat.persist.model.Relationship;
import com.sleepycat.persist.model.SecondaryKey;

/**
 * A TicketLog represents a log entry for ticketing actions.
 */
@Entity
public class TicketLog {
	/**
	 * The type of actions performed on tickets.
	 */
	public enum Action {
		ISSUE,
		CHARGE
	}

	/* The time this log is created (milliseconds after epoch). */
	@PrimaryKey
	private long logTime;

	/* The id of the parking meter issuing this ticket. */
	@SecondaryKey(relate = Relationship.MANY_TO_ONE)
	private String meterId;

	/* The ticket id. */
	private Long ticketId;

	/* The action performed on the ticket. */
	private Action action;

	/* The amount charged (in cents) for the ticket (0 for ISSUE). */
	private int chargeAmountInCents;

	/**
	 * Primary constructor.
	 *
	 * @param logTime             the time of this log event
	 * @param meterId             the parking meter id
	 * @param ticketId            the parking ticket id
	 * @param action              the action of the event
	 * @param chargeAmountInCents the amount charged for the ticket
	 */
	public TicketLog(long logTime, String meterId, Long ticketId,
	                 Action action, int chargeAmountInCents) {
		this.logTime = logTime;
		this.meterId = meterId;
		this.ticketId = ticketId;
		this.action = action;
		this.chargeAmountInCents = chargeAmountInCents;
	}

	/* For DPL binding only. */
	private TicketLog() {
	}

	/**
	 * Get the time of the logged action.
	 *
	 * @return the time of the action in milliseconds after epoch
	 */
	public long getLogTime() {
		return logTime;
	}

	/**
	 * Get the parking meter id.
	 *
	 * @return the parking meter id
	 */
	public String getMeterId() {
		return meterId;
	}

	/**
	 * Get the ticket id.
	 *
	 * @return the ticket id
	 */
	public Long getTicketId() {
		return ticketId;
	}

	/**
	 * Get the action performed on the ticket.
	 *
	 * @return the type of the action
	 */
	public Action getAction() {
		return action;
	}

	/**
	 * Get the amount charged (in cents) for the ticket.
	 *
	 * @return charged amount in cents
	 */
	public int getChargeAmountInCents() {
		return chargeAmountInCents;
	}
}
