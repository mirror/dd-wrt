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

/**
 * A Ticket represents a parking ticket issued by a
 * {@link com.sleepycat.sample.park.Meter}.
 */
@Entity
public class Ticket {
	/* The ticket id. */
	@PrimaryKey(sequence = "TicketIdSeq")
	private Long ticketId;

	/* The id of the parking meter issuing this ticket. */
	private String meterId;

	/* The issuing time of this ticket, represented as milliseconds after epoch. */
	private Long issuingTime;

	/**
	 * Primary constructor.
	 *
	 * @param ticketId    the ticket id
	 * @param meterId     the parking meter id
	 * @param issuingTime the time this ticket is issued
	 */
	public Ticket(Long ticketId, String meterId, Long issuingTime) {
		this.ticketId = ticketId;
		this.meterId = meterId;
		this.issuingTime = issuingTime;
	}

	/* For DPL binding only. */
	private Ticket() {
	}

	/**
	 * Get the ticket id.
	 *
	 * @return the id of the ticket
	 */
	public Long getTicketId() {
		return ticketId;
	}

	/**
	 * Get the id of the issuing parking meter
	 *
	 * @return the parking meter id
	 */
	public String getMeterId() {
		return meterId;
	}

	/**
	 * Get the time this ticket is issued
	 *
	 * @return the issuing time
	 */
	public Long getIssuingTime() {
		return issuingTime;
	}
}
