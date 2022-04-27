/*-
 *  And then Indranet Technologies Ltd sponsored Derek Smithies to work
 * on this code. Derek Smithies (derek@indranet.co.nz) took parts of the 
 * adm module and pasted it into this code base. 
 *
 * This version of John Bicket's code takes the experimental approach one
 * step further.
 * When in auto rate mode, packets are sent at the selected rate.
 * The Hal asks for what alternative rate to use if the selected rate fails.
 * We provide the alternative rate from a random selection of 1.. max rate.
 * Given the probability of success, multiplied with the transmission time,
 * we can determine the rate which maximises packet throughput.
 *
 * Different rates are used for every remote node - some nodes will work 
 * better on different rates.
 * Every second, a timer fires, to assess the throughput at each rate with 
 *  each remote node.
 * This timer will then determine the optimum rate for each remote node, based
 * on the performance figures.
 *
 * This code is called minstrel, because we have taken a wandering minstrel
 * apporoch. Wander aimlessly around the different rates, singing wherever 
 * you can. And then, look at the performacnce, and make a choice.
 *
 *  Enjoy.  Derek Smithies.
 *
 ***********************************************************************
 * Copyright (c) 2005 John Bicket
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer,
 *    without modification.
 * 2. Redistributions in binary form must reproduce at minimum a disclaimer
 *    similar to the "NO WARRANTY" disclaimer below ("Disclaimer") and any
 *    redistribution must be conditioned upon including a substantially
 *    similar Disclaimer requirement for further binary redistribution.
 * 3. Neither the names of the above-listed copyright holders nor the names
 *    of any contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * Alternatively, this software may be distributed under the terms of the
 * GNU General Public License ("GPL") version 2 as published by the Free
 * Software Foundation.
 *
 * NO WARRANTY
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF NONINFRINGEMENT, MERCHANTIBILITY
 * AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL
 * THE COPYRIGHT HOLDERS OR CONTRIBUTORS BE LIABLE FOR SPECIAL, EXEMPLARY,
 * OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER
 * IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGES.
 *
 * $Id: minstrel.h 1441 2006-02-06 16:03:21Z mrenzmann $
 */

/*
 * Defintions for the Atheros Wireless LAN controller driver.
 */
#ifndef _DEV_ATH_RATE_MINSTREL_H
#define _DEV_ATH_RATE_MINSTREL_H
#define MINSTREL_COLUMNS 10

/* per-device state */
struct minstrel_softc {
	struct ath_ratectrl arc; 	/* base state */

#ifdef CONFIG_SYSCTL
	struct ctl_table_header *sysctl_header;
	struct ctl_table *sysctls;
#endif
	struct ath_softc  *sc;
	struct net_device *sc_dev;


	struct timer_list timer;	/* periodic timer */
	int close_timer_now;
};


#define ATH_SOFTC_MINSTREL(sc)    ((struct minstrel_softc *)sc->sc_rc)

struct rate_info {
	int rate;
	int rix;
	int rateCode;
	int shortPreambleRateCode;
};



/* per-node state */
struct minstrel_node {
	int static_rate_ndx; /*User has bypassed dynamic selection. Fix on one rate */
	int num_rates;
	struct rate_info rates[IEEE80211_RATE_MAXSIZE];

	unsigned perfect_tx_time[IEEE80211_RATE_MAXSIZE];  /* transmit time for 0 retries */
	unsigned retry_count[IEEE80211_RATE_MAXSIZE];      /*The number of retrys permitted for this particular rate */
	unsigned retry_adjusted_count[IEEE80211_RATE_MAXSIZE]; /*retry_count, but altered, depending on if this is a very poor (or very good) link */
	int current_rate;

	u_int32_t		rs_rateattempts[IEEE80211_RATE_MAXSIZE];
	u_int32_t               rs_thisprob    [IEEE80211_RATE_MAXSIZE];
	u_int32_t		rs_ratesuccess [IEEE80211_RATE_MAXSIZE];
	u_int32_t		rs_lastrateattempts[IEEE80211_RATE_MAXSIZE];
	u_int32_t		rs_lastratesuccess [IEEE80211_RATE_MAXSIZE];
	u_int32_t               rs_probability [IEEE80211_RATE_MAXSIZE]; /* units of parts per thousand */
	u_int64_t               rs_succ_hist   [IEEE80211_RATE_MAXSIZE];
	u_int64_t               rs_att_hist    [IEEE80211_RATE_MAXSIZE];

	u_int32_t               rs_this_tp     [IEEE80211_RATE_MAXSIZE]; /*Throughput, each rate */

       /**This flag on indicates to try other than the optimal rate,
          to see what we find */
	int                     is_sampling;

        /**The current minstrel sample rate - which could be anything
         from 1..11 - assuming there are 12 possible rates. Note that
         we never ever sample at the base rate of 0.*/
       int                     rs_sample_rate;

       /**Flag to indicate that the sample rate is slower than the
          current maximum throughput rate. This determines the order
          of the retry chain. */
       int                     rs_sample_rate_slower;

	/** These four parameters are indexes into the current rate
	    table, and are calculated in ath_rate_statistics(), which
	    happens every time the timer for rate adjustment fires */
	int max_tp_rate;       /*  Has the current highest recorded throughput */
	int max_tp_rate2;      /*  Has the second highest recorded throughput */
	int max_prob_rate;     /*  This rate has the highest chance of success. */

	/**These two variables are used to keep track of what
	   percentage of packets have been used to do sample on. 
	   Thus,if ath_lookaround_rate is set to 10%, we can expect that
	   sample_count
	       ------------                    = 0.1
	   sample_count + packet_count                           */
	int packet_count;     /*  The number of times we have  
				  sent a packet to this node. */
	int sample_count;     /* The number of times we have 
				 sent a sample packet to this node */

	/**The table that holds the sequence of rates that we use. In
	   each column in the table, the rates we sample are provided
	   in some "random" order. Each rate we sample occurs once in
	   each column. There are several columns, so we sample the
	   different rates in a different order each time. */
	u_int8_t              rs_sampleTable[IEEE80211_RATE_MAXSIZE][MINSTREL_COLUMNS];
	int                   rs_sampleColumn;
	int                   rs_sampleIndex;

	   /**Random number generator is
	       Rn+1 = (A*Rn) + B. 

	       This Random number generator determines when we send a minstrel
	       packet, or a packet at an optimal rate.*/
	int random_n;
	int a, b;          /**Coefficients of the random thing */

	unsigned long last_update;
};


#define	ATH_NODE_MINSTREL(an)	((struct minstrel_node *)&an[1])

#endif /* _DEV_ATH_RATE_MINSTEL_H */

/* The comment below is magic for those who use emacs to edit this file. */
/* With the comment below, the tab key does auto indent to 8 spaces.     */

/*
 * Local Variables:
 * mode:c
 * c-file-style:"linux"
 * c-basic-offset:8
 * End:
 */
