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
		bool * externalStateChange, bool * subStateExternalStateChange);

#endif /* _PUD_STATE_H_ */
