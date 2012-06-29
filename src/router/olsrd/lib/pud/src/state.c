#include "state.h"

/* Plugin includes */
#include "configuration.h"

/* OLSRD includes */

/* System includes */

/*
 * Types
 */

/** Type describing substate */
typedef struct _SubStateType {
	MovementState internalState; /**< the internal movement state */
	unsigned long long hysteresisCounter; /**< the hysteresis counter */
	unsigned long long hysteresisCounterToStationary; /**< the hysteresis counter threshold for changing the state to STATIONARY */
	unsigned long long hysteresisCounterToMoving; /**< the hysteresis counter threshold for changing the state to MOVING */
	MovementState externalState; /**< the externally visible movement state */
} SubStateType;

/** Type describing state */
typedef struct _StateType {
	SubStateType substate[SUBSTATE_COUNT]; /**< the sub states */
	MovementState externalState; /**< the externally visible movement state */
} StateType;

/*
 * Variables
 */

/** The state */
static StateType state;

/*
 * Functions
 */

void initState(void) {
	state.substate[SUBSTATE_POSITION].internalState = MOVEMENT_STATE_STATIONARY;
	state.substate[SUBSTATE_POSITION].hysteresisCounter = 0;
	state.substate[SUBSTATE_POSITION].hysteresisCounterToStationary = getHysteresisCountToStationary();
	state.substate[SUBSTATE_POSITION].hysteresisCounterToMoving = getHysteresisCountToMoving();
	state.substate[SUBSTATE_POSITION].externalState = MOVEMENT_STATE_STATIONARY;
	state.substate[SUBSTATE_GATEWAY].internalState = MOVEMENT_STATE_MOVING;
	state.substate[SUBSTATE_GATEWAY].hysteresisCounter = 0;
	state.substate[SUBSTATE_GATEWAY].hysteresisCounterToStationary = getGatewayHysteresisCountToStationary();
	state.substate[SUBSTATE_GATEWAY].hysteresisCounterToMoving = getGatewayHysteresisCountToMoving();
	state.substate[SUBSTATE_GATEWAY].externalState = MOVEMENT_STATE_MOVING;
	state.externalState = MOVEMENT_STATE_MOVING; /* must comply to AND/OR conditions of sub-states */
}

MovementState getExternalState(void) {
	return state.externalState;
}

/**
 * Determine new state
 * @param subStateIndex
 * the sub-state for/from which the new state should be determined
 * @param movingNow
 * the movement result of the sub-state
 * @param externalState
 * a pointer to the variable in which to store the new external state
 * @param externalStateChange
 * a pointer to the variable in which to store whether the external state changed
 * @param subStateExternalStateChange
 * a pointer to the variable in which to store whether the sub-state external state changed
 */
void determineStateWithHysteresis(SubStateIndex subStateIndex, TristateBoolean movingNow, MovementState * externalState,
		bool * externalStateChange, bool * subStateExternalStateChange) {
	MovementState newState;
	bool internalStateChange;
	bool subStateExternalStateChanged;
	SubStateType * subState = &state.substate[subStateIndex];

	/*
	 * Substate Internal State
	 */

	if (movingNow == TRISTATE_BOOLEAN_SET) {
		newState = MOVEMENT_STATE_MOVING;
	} else if (movingNow == TRISTATE_BOOLEAN_UNSET) {
		newState = MOVEMENT_STATE_STATIONARY;
	} else {
		/* keep current sub-state */
		newState = subState->internalState;
	}
	internalStateChange = (subState->internalState != newState);
	subState->internalState = newState;

	/*
	 * Substate External State (+ hysteresis)
	 */

	if (internalStateChange) {
		/* restart hysteresis for external state change when we have an internal
		 * state change */
		subState->hysteresisCounter = 0;
	}

	/* when internal state and external state are not the same we need to
	 * perform hysteresis before we can propagate the internal state to the
	 * external state */
	newState = subState->externalState;
	if (subState->internalState != subState->externalState) {
		switch (subState->internalState) {
		case MOVEMENT_STATE_STATIONARY:
			/* internal state is STATIONARY, external state is MOVING */

			/* delay going to stationary a bit */
			subState->hysteresisCounter++;

			if (subState->hysteresisCounter >= subState->hysteresisCounterToStationary) {
				/* outside the hysteresis range, go to stationary */
				newState = MOVEMENT_STATE_STATIONARY;
			}
			break;

		case MOVEMENT_STATE_MOVING:
			/* internal state is MOVING, external state is STATIONARY */

			/* delay going to moving a bit */
			subState->hysteresisCounter++;

			if (subState->hysteresisCounter >= subState->hysteresisCounterToMoving) {
				/* outside the hysteresis range, go to moving */
				newState = MOVEMENT_STATE_MOVING;
			}
			break;

		default:
			/* when unknown then don't change state */
			break;
		}
	}

	subStateExternalStateChanged = (subState->externalState != newState);
	if (subStateExternalStateChange) {
		*subStateExternalStateChange = subStateExternalStateChanged;
	}
	subState->externalState = newState;

	/*
	 * external state may transition into MOVING when either one of the sub-states say so (OR), and
	 * may transition into STATIONARY when all of the sub-states say so (AND)
	 */
	if (externalStateChange) {
		*externalStateChange = false;
	}
	if (subStateExternalStateChanged) {
		bool transition;

		if (newState == MOVEMENT_STATE_STATIONARY) {
			/* AND: all sub-states must agree on STATIONARY */
			int i = 0;

			transition = true;
			for (i = 0; i < SUBSTATE_COUNT; i++) {
				transition = transition && (state.substate[i].externalState == newState);
			}
		} else /* if (newState == MOVEMENT_STATE_MOVING) */{
			/* OR: one sub-state wanting MOVING is enough */
			transition = true;
		}

		if (transition) {
			if (externalStateChange) {
				*externalStateChange = (state.externalState != newState);
			}
			state.externalState = newState;
		}
	}

	if (externalState) {
		*externalState = state.externalState;
	}
}

MovementState getInternalState(SubStateIndex subStateIndex) {
	return state.substate[subStateIndex].internalState;
}
