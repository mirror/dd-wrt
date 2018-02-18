/*
 * The olsr.org Optimized Link-State Routing daemon (olsrd)
 *
 * (c) by the OLSR project
 *
 * See our Git repository to find out who worked on this file
 * and thus is a copyright holder on it.
 *
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * * Redistributions of source code must retain the above copyright
 *   notice, this list of conditions and the following disclaimer.
 * * Redistributions in binary form must reproduce the above copyright
 *   notice, this list of conditions and the following disclaimer in
 *   the documentation and/or other materials provided with the
 *   distribution.
 * * Neither the name of olsr.org, olsrd nor the names of its
 *   contributors may be used to endorse or promote products derived
 *   from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 * ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 * Visit http://www.olsr.org for more information.
 *
 * If you find this software useful feel free to make a donation
 * to the project. For more information see the website or contact
 * the copyright holders.
 *
 */

#ifndef _PUD_POSAVG_H_
#define _PUD_POSAVG_H_

/* Plugin includes */

/* OLSR includes */

/* System includes */
#include <nmealib/info.h>
#include <stdbool.h>

/** Stores angle components */
typedef struct _AngleComponents {
		double x; /**< cos of the angle (in radians) */
		double y; /**< sin of the angle (in radians) */
} AngleComponents;

/** Stores an nmeaINFO entry, used in the averaging */
typedef struct _PositionUpdateEntry {
		NmeaInfo nmeaInfo; /**< the position information */

		/* used for averaging of angles */
		AngleComponents track; /**< the track angle components */
		AngleComponents mtrack; /**< the mtrack angle components */
		AngleComponents magvar; /**< the magvar angle components */
} PositionUpdateEntry;

/**
 Counts the number of GPxxx based entries. Some parameters in nmeaINFO are
 dependent on different GPxxx NMEA sentences. These counters are used to
 determine which information would be valid for the average position.
 Also counts the fix values.
 */
typedef struct _PositionUpdateCounters {
		/* present */
		unsigned long long smask; /**< the number of entries with SMASK present */
		unsigned long long utcdate; /**< the number of entries with UTCDATE present */
		unsigned long long utctime; /**< the number of entries with UTCTIME present */
		unsigned long long sig; /**< the number of entries with SIG present */
		unsigned long long fix; /**< the number of entries with FIX present */
		unsigned long long pdop; /**< the number of entries with PDOP present */
		unsigned long long hdop; /**< the number of entries with HDOP present */
		unsigned long long vdop; /**< the number of entries with VDOP present */
		unsigned long long lat; /**< the number of entries with LAT present */
		unsigned long long lon; /**< the number of entries with LON present */
		unsigned long long elv; /**< the number of entries with ELV present */
		unsigned long long speed; /**< the number of entries with SPEED present */
		unsigned long long track; /**< the number of entries with TRACK present */
		unsigned long long mtrack; /**< the number of entries with MTRACK present */
		unsigned long long magvar; /**< the number of entries with MAGVAR present */
		unsigned long long satinusecount; /**< the number of entries with SATINUSECOUNT present */
		unsigned long long satinuse; /**< the number of entries with SATINUSE present */
		unsigned long long satinviewcount; /**< the number of entries with SATINVIEWCOUNT present */
		unsigned long long satinview; /**< the number of entries with SATINVIEW present */
		unsigned long long height; /**< the number of entries with HEIGHT present */
		unsigned long long dgpsage; /**< the number of entries with DGPSAGE present */
		unsigned long long dgpssid; /**< the number of entries with DGPSSID present */

		/* smask */
		unsigned long long gpgga; /**< the number of GPGGA based entries */
		unsigned long long gpgsa; /**< the number of GPGSA based entries */
		unsigned long long gpgsv; /**< the number of GPGSV based entries */
		unsigned long long gprmc; /**< the number of GPRMC based entries */
		unsigned long long gpvtg; /**< the number of GPVTG based entries */

		/* sig */
		unsigned long long sigBad; /**< the number of entries with a bad sig */
		unsigned long long sigLow; /**< the number of entries with a low sig */
		unsigned long long sigMid; /**< the number of entries with a mid sig */
		unsigned long long sigHigh; /**< the number of entries with a high sig */

		/* fix */
		unsigned long long fixBad; /**< the number of entries with a bad fix */
		unsigned long long fix2d; /**< the number of entries with a 2D fix */
		unsigned long long fix3d; /**< the number of entries with a 3D fix */
} PositionUpdateCounters;

/**
 A list of position updates that are used to determine the average position.

 The list uses 1 extra entry: when 5 entries have to be averaged then the list
 will have 6 entries. The 6th entry is used for the incoming entry, so that
 it is already in the list (together with the old entry that will be removed
 from the average) and does not need to be copied into the list. This is for
 better performance.

 The list is a circular list.
 This means that there is a gap/unused entry in the list between the
 newest entry and the oldest entry, which is the 'incoming entry'.

 Note that 'positionAverageCumulative' stores cumulative values for parameters
 for which an average is calculated. The reason is to minimise the number of
 calculations to be performed.
 */
typedef struct _PositionAverageList {
		unsigned long long entriesMaxCount; /**< the maximum number of entries in the list */
		PositionUpdateEntry * entries; /**< the list entries */

		unsigned long long entriesCount; /**< the number of entries in the list */
		unsigned long long newestEntryIndex; /**< index of the newest entry in the list (zero-based) */
		PositionUpdateCounters counters; /**< the counters */

		PositionUpdateEntry positionAverageCumulative; /**< the average position with cumulative values */
		PositionUpdateEntry positionAverage; /**< the average position */
} PositionAverageList;

/**
 Enumeration describing the type of an entry position in the average list
 */
typedef enum _AverageEntryPositionType {
	OLDEST, NEWEST, INCOMING, AVERAGECUMULATIVE, AVERAGE
} AverageEntryPositionType;

bool initPositionAverageList(PositionAverageList * positionAverageList,
		unsigned long long maxEntries);
void flushPositionAverageList(PositionAverageList * positionAverageList);
void destroyPositionAverageList(PositionAverageList * positionAverageList);

PositionUpdateEntry * getPositionAverageEntry(
		PositionAverageList * positionAverageList,
		AverageEntryPositionType positionType);

void addNewPositionToAverage(PositionAverageList * positionAverageList,
		PositionUpdateEntry * newEntry);

#endif /* _PUD_POSAVG_H_ */
