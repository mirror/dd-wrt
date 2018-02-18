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

#ifndef _PUD_STATE_H_
#define _PUD_STATE_H_

/* Plugin includes */

/* OLSRD includes */

/* System includes */
#include <stdbool.h>

/*
 * Types
 */

/** Type describing a tri-state boolean */
typedef enum _TristateBoolean {
	TRISTATE_BOOLEAN_UNKNOWN = 0,
	TRISTATE_BOOLEAN_UNSET = 1,
	TRISTATE_BOOLEAN_SET = 2
} TristateBoolean;

#define TristateBooleanToString(s)	((s == TRISTATE_BOOLEAN_SET) ? "set" : \
									 (s == TRISTATE_BOOLEAN_UNSET) ? "unset" : \
									 "unknown")

/** Type describing movement state */
typedef enum _MovementState {
	MOVEMENT_STATE_STATIONARY = 0,
	MOVEMENT_STATE_MOVING = 1
} MovementState;

#define MovementStateToString(s)	((s == MOVEMENT_STATE_MOVING) ? "moving" : \
									 "stationary")

/** Type describing substate indexes */
typedef enum _SubStateIndex {
	SUBSTATE_POSITION = 0,
	SUBSTATE_GATEWAY = 1,
	SUBSTATE_COUNT = 2
} SubStateIndex;

/*
 * Functions
 */

void initState(void);
MovementState getExternalState(void);
MovementState getInternalState(SubStateIndex subStateIndex);
void determineStateWithHysteresis(SubStateIndex subStateIndex, TristateBoolean movingNow, MovementState * externalState,
		bool * externalStateChange, bool * subStateExternalStateChange, bool gpnonChanged);

#endif /* _PUD_STATE_H_ */
