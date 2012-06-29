#include "timers.h"

/* Plugin includes */
#include "pud.h"

/* OLSRD includes */
#include "olsr_cookie.h"

/* System includes */

/*
 * OLSR Tx Timer
 */

/** The timer cookie, used to trace back the originator in debug */
static struct olsr_cookie_info *pud_olsr_tx_timer_cookie = NULL;

/** The timer */
static struct timer_entry * pud_olsr_tx_timer = NULL;

/**
 Start the OLSR tx timer. Does nothing when the timer is already running.

 @param interval
 The interval in seconds
 @param cb_func
 The callback function to call when the timer expires

 @return
 - false on failure
 - true otherwise
 */
static int startOlsrTxTimer(unsigned long long interval, timer_cb_func cb_func) {
	if (pud_olsr_tx_timer == NULL) {
		pud_olsr_tx_timer = olsr_start_timer(interval * MSEC_PER_SEC, 0,
				OLSR_TIMER_PERIODIC, cb_func, NULL,
				pud_olsr_tx_timer_cookie);
		if (pud_olsr_tx_timer == NULL) {
			return false;
		}
	}

	return true;
}

/**
 Stop the OLSR tx timer
 */
static void stopOlsrTxTimer(void) {
	if (pud_olsr_tx_timer != NULL) {
		olsr_stop_timer(pud_olsr_tx_timer);
		pud_olsr_tx_timer = NULL;
	}
}

/**
 Restart the OLSR tx timer

 @param interval
 The interval in seconds
 @param cb_func
 The callback function to call when the timer expires

 @return
 - false on failure
 - true otherwise
 */
int restartOlsrTxTimer(unsigned long long interval, timer_cb_func cb_func) {
	stopOlsrTxTimer();
	return startOlsrTxTimer(interval, cb_func);
}

/**
 Initialise the OLSR tx timer.

 @return
 - false on failure
 - true otherwise
 */
int initOlsrTxTimer(void) {
	if (pud_olsr_tx_timer_cookie == NULL) {
		pud_olsr_tx_timer_cookie = olsr_alloc_cookie(
				PUD_PLUGIN_ABBR ": OLSR tx timer", OLSR_COOKIE_TYPE_TIMER);
		if (pud_olsr_tx_timer_cookie == NULL) {
			return false;
		}
	}
	return true;
}

/**
 Destroy the OLSR tx timer.
 */
void destroyOlsrTxTimer(void) {
	stopOlsrTxTimer();
	if (pud_olsr_tx_timer_cookie != NULL) {
		olsr_free_cookie(pud_olsr_tx_timer_cookie);
		pud_olsr_tx_timer_cookie = NULL;
	}
}

/*
 * Uplink Tx Timer
 */

/** The timer cookie, used to trace back the originator in debug */
static struct olsr_cookie_info *pud_uplink_tx_timer_cookie = NULL;

/** The timer */
static struct timer_entry * pud_uplink_tx_timer = NULL;

/**
 Start the uplink tx timer. Does nothing when the timer is already running.

 @param interval
 The interval in seconds
 @param cb_func
 The callback function to call when the timer expires

 @return
 - false on failure
 - true otherwise
 */
static int startUplinkTxTimer(unsigned long long interval, timer_cb_func cb_func) {
	if (pud_uplink_tx_timer == NULL) {
		pud_uplink_tx_timer = olsr_start_timer(interval * MSEC_PER_SEC, 0,
				OLSR_TIMER_PERIODIC, cb_func, NULL,
				pud_uplink_tx_timer_cookie);
		if (pud_uplink_tx_timer == NULL) {
			return false;
		}
	}

	return true;
}

/**
 Stop the uplink tx timer
 */
static void stopUplinkTxTimer(void) {
	if (pud_uplink_tx_timer != NULL) {
		olsr_stop_timer(pud_uplink_tx_timer);
		pud_uplink_tx_timer = NULL;
	}
}

/**
 Restart the uplink tx timer

 @param interval
 The interval in seconds
 @param cb_func
 The callback function to call when the timer expires

 @return
 - false on failure
 - true otherwise
 */
int restartUplinkTxTimer(unsigned long long interval, timer_cb_func cb_func) {
	stopUplinkTxTimer();
	return startUplinkTxTimer(interval, cb_func);
}

/**
 Initialise the uplink tx timer.

 @return
 - false on failure
 - true otherwise
 */
int initUplinkTxTimer(void) {
	if (pud_uplink_tx_timer_cookie == NULL) {
		pud_uplink_tx_timer_cookie = olsr_alloc_cookie(
				PUD_PLUGIN_ABBR ": uplink tx timer", OLSR_COOKIE_TYPE_TIMER);
		if (pud_uplink_tx_timer_cookie == NULL) {
			return false;
		}
	}
	return true;
}

/**
 Destroy the uplink tx timer.
 */
void destroyUplinkTxTimer(void) {
	stopUplinkTxTimer();
	if (pud_uplink_tx_timer_cookie != NULL) {
		olsr_free_cookie(pud_uplink_tx_timer_cookie);
		pud_uplink_tx_timer_cookie = NULL;
	}
}

/*
 * Best Gateway Timer
 */

/** The timer cookie, used to trace back the originator in debug */
static struct olsr_cookie_info *pud_gateway_timer_cookie = NULL;

/** The timer */
static struct timer_entry * pud_gateway_timer = NULL;

/**
 Start the gateway timer. Does nothing when the timer is already running.

 @param interval
 The interval in seconds
 @param cb_func
 The callback function to call when the timer expires

 @return
 - false on failure
 - true otherwise
 */
static int startGatewayTimer(unsigned long long interval, timer_cb_func cb_func) {
	if (pud_gateway_timer == NULL) {
		pud_gateway_timer = olsr_start_timer(interval * MSEC_PER_SEC, 0,
				OLSR_TIMER_PERIODIC, cb_func, NULL,
				pud_gateway_timer_cookie);
		if (pud_gateway_timer == NULL) {
			return false;
		}
	}

	return true;
}

/**
 Stop the gateway timer
 */
static void stopGatewayTimer(void) {
	if (pud_gateway_timer != NULL) {
		olsr_stop_timer(pud_gateway_timer);
		pud_gateway_timer = NULL;
	}
}

/**
 Restart the gateway timer

 @param interval
 The interval in seconds
 @param cb_func
 The callback function to call when the timer expires

 @return
 - false on failure
 - true otherwise
 */
int restartGatewayTimer(unsigned long long interval, timer_cb_func cb_func) {
	stopGatewayTimer();
	return startGatewayTimer(interval, cb_func);
}

/**
 Initialise the gateway timer.

 @return
 - false on failure
 - true otherwise
 */
int initGatewayTimer(void) {
	if (pud_gateway_timer_cookie == NULL) {
		pud_gateway_timer_cookie = olsr_alloc_cookie(
				PUD_PLUGIN_ABBR ": gateway timer", OLSR_COOKIE_TYPE_TIMER);
		if (pud_gateway_timer_cookie == NULL) {
			return false;
		}
	}
	return true;
}

/**
 Destroy the gateway timer.
 */
void destroyGatewayTimer(void) {
	stopGatewayTimer();
	if (pud_gateway_timer_cookie != NULL) {
		olsr_free_cookie(pud_gateway_timer_cookie);
		pud_gateway_timer_cookie = NULL;
	}
}
