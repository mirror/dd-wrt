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

#include "timers.h"

/* Plugin includes */
#include "pud.h"

/* OLSRD includes */
#include "olsr_cookie.h"

/* System includes */

/*
 * gpsd Fetch Timer
 */

/** The timer cookie, used to trace back the originator in debug */
static struct olsr_cookie_info *pud_gpsd_fetch_timer_cookie = NULL;

/** The timer */
static struct timer_entry * pud_gpsd_fetch_timer = NULL;

/**
 Start the gpsd fetch timer. Does nothing when the timer is already running.

 @param cb_func
 The callback function to call when the timer expires

 @return
 - false on failure
 - true otherwise
 */
static int startGpsdFetchTimer(timer_cb_func cb_func) {
  if (!pud_gpsd_fetch_timer) {
    pud_gpsd_fetch_timer = olsr_start_timer(1 * (MSEC_PER_SEC / TIMER_GPSD_READS_PER_SEC), 0, OLSR_TIMER_PERIODIC, cb_func, NULL, pud_gpsd_fetch_timer_cookie);
    if (!pud_gpsd_fetch_timer) {
      return false;
    }
  }

  return true;
}

/**
 Stop the gpsd fetch timer
 */
static void stopGpsdFetchTimer(void) {
  if (pud_gpsd_fetch_timer) {
    olsr_stop_timer(pud_gpsd_fetch_timer);
    pud_gpsd_fetch_timer = NULL;
  }
}

/**
 Restart the gpsd fetch timer

 @param cb_func
 The callback function to call when the timer expires

 @return
 - false on failure
 - true otherwise
 */
int restartGpsdFetchTimer(timer_cb_func cb_func) {
  stopGpsdFetchTimer();
  return startGpsdFetchTimer(cb_func);
}

/**
 Initialise the gpsd fetch timer.

 @return
 - false on failure
 - true otherwise
 */
int initGpsdFetchTimer(void) {
  if (!pud_gpsd_fetch_timer_cookie) {
    pud_gpsd_fetch_timer_cookie = olsr_alloc_cookie(PUD_PLUGIN_ABBR ": gpsd fetch timer", OLSR_COOKIE_TYPE_TIMER);
    if (!pud_gpsd_fetch_timer_cookie) {
      return false;
    }
  }

  return true;
}

/**
 Destroy the gpsd fetch timer.
 */
void destroyGpsdFetchTimer(void) {
  stopGpsdFetchTimer();
  if (pud_gpsd_fetch_timer_cookie) {
    olsr_free_cookie(pud_gpsd_fetch_timer_cookie);
    pud_gpsd_fetch_timer_cookie = NULL;
  }
}

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
