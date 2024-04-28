/*
 **************************************************************************
 * Copyright (c) 2014-2015, 2017, The Linux Foundation. All rights reserved.
 * Permission to use, copy, modify, and/or distribute this software for
 * any purpose with or without fee is hereby granted, provided that the
 * above copyright notice and this permission notice appear in all copies.
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT
 * OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 **************************************************************************
 */

/**
 * @file nss_profiler.h
 *	NSS Profiler APIs
 */

#ifndef __NSS_PROFILER_H
#define __NSS_PROFILER_H

/**
 * @addtogroup nss_profiler_subsystem
 * @{
 */

/**
 * Length of the counter name.
 *
 * This value allows all counter values to fit in a single 1400-byte UDP packet.
 */
#define PROFILE_COUNTER_NAME_LENGTH 20

#define PROFILE_MAX_APP_COUNTERS 24	/**< Maximum number of application counters. */

/**
 * nss_profile_counter
 *	Counter statistics.
 */
struct nss_profile_counter {
	char name[PROFILE_COUNTER_NAME_LENGTH];		/**< Counter name. */
	uint32_t value;					/**< Current value. */
};

/**
 * nss_profiler_message_types
 *	Message types for the Profiler.
 *
 * Do not alter this enumeration. However, adding more types is allowed.
 */
enum nss_profiler_message_types {
	NSS_PROFILER_CHANGE_SAMPLING_RATE_MSG,	/**< Host-to-NSS: ask to do a rate change. */
	NSS_PROFILER_START_MSG,			/**< Host-to-NSS: start the NSS Profiler. */
	NSS_PROFILER_STOP_MSG,			/**< Host-to-NSS: stop the NSS Profiler. */
	NSS_PROFILER_FLOWCTRL_MSG,		/**< Host-to-NSS: do flow control on sampling. */
	NSS_PROFILER_DEBUG_RD_MSG,		/**< Host-to-NSS: debug the output. */
	NSS_PROFILER_DEBUG_WR_MSG,		/**< Host-to-NSS: debug the input. */
	NSS_PROFILER_DEBUG_REPLY_MSG,		/**< NSS-to-host: debug response. */
	NSS_PROFILER_REPLY_MSG,			/**< Check the response. */
	NSS_PROFILER_FIXED_INFO_MSG,		/**< NSS-to-host: constant data. */
	NSS_PROFILER_COUNTERS_MSG,		/**< NSS-to-host: counter information. */
	NSS_PROFILER_SAMPLES_MSG,		/**< NSS-to-host: main sample data. */
	NSS_PROFILER_START_CAL,			/**< Not for the host to use. */
	NSS_PROFILER_GET_SYS_STAT_EVENT,	/**< Get the system status event. */
	NSS_PROFILER_SET_SYS_STAT_EVENT,	/**< Set the system status event. */
	NSS_PROFILER_MAX_MSG_TYPES,		/**< Maximum number of message types. */
};

/**
 * nss_profile_errors
 *	Profiler error types returned from the NSS.
 */
enum nss_profile_errors {
	PROFILE_ERROR_NO_PROF_INIT = 1,
	PROFILE_ERROR_EMEM,
	PROFILE_ERROR_BAD_PKT,
	PROFILE_ERROR_UNKNOWN_CMD,
};

/**
 * nss_profiler_cmd_param
 *	Parameter information for the Profiler.
 *
 * Use this structure for per-session commands: START, STOP, FLOWCTRL, RATE.
 */
struct nss_profiler_cmd_param {
	uint32_t hd_magic;		/**< Common overlay in all headers. */
	uint32_t num_counters;
			/**< Number of registered performance (application) counters. */
	uint32_t ocm_size;		/**< Size of the on-chip-memory. */
	uint32_t sram_start;		/**< DDR starting address. */
	uint32_t rate;			/**< Sampling rate. */
	uint32_t cpu_id;		/**< ID of the chip register. */
	uint32_t cpu_freq;		/**< Chip clock frequency. */
	uint32_t ddr_freq;		/**< DDR memory speed. */

	struct nss_profile_counter counters[PROFILE_MAX_APP_COUNTERS];
			/**< Application profiling counters. */
};

/**
 * nss_profiler_data_msg
 *	Message information for the Profiler.
 */
struct nss_profiler_data_msg {
	uint32_t hd_magic;		/**< Magic header for verification. */
	uint32_t msg_data[1];		/**< Variable length private data. */
};

/**
 * nss_profiler_debug_msg
 *	Message information for Profiler debugging.
 */
struct nss_profiler_debug_msg {
	uint32_t hd_magic;		/**< Magic header for verification. */
	uint32_t debug_data[256];	/**< Fixed length debug data. */
};

/**
 * nss_profiler_msg
 *	Data for sending and receiving Profiler messages.
 */
struct nss_profiler_msg {
	struct nss_cmn_msg cm;		/**< Common message header. */

	/**
	 * Payload of a Profiler message.
	 */
	union npm_body {
		struct nss_profiler_cmd_param pcmdp;	/**< Command parameters. */
		struct nss_profiler_debug_msg pdm;	/**< Debug packet. */
		struct nss_profiler_data_msg msg;	/**< Sampling data. */
	} payload;	/**< Message payload. The data length is set in common message header. */
};

/**
 * Callback function for receiving Profiler messages.
 *
 * @note: Memory (buffer) pointed by npm is owned by caller, that is, NSS driver.
 *
 * @datatypes
 * nss_profiler_msg
 *
 * @param[in] ctx  Pointer to the context of the NSS process (core).
 * @param[in] npm  Pointer to the NSS Profiler message.
 */
typedef void (*nss_profiler_callback_t)(void *ctx, struct nss_profiler_msg *npm);

/**
 * nss_profiler_notify_register
 *	Registers the Profiler interface with the NSS driver for sending and receiving messages.
 *
 * This function must be called once for each core.
 *
 * @datatypes
 * nss_core_id_t \n
 * nss_profiler_callback_t
 *
 * @param[in] profiler_callback  Callback for the data.
 * @param[in] core_id            NSS core ID.
 * @param[in] ctx                Pointer to the context of the NSS core. The context is
                                 provided to caller in the registered callback function.
 *
 * @return
 * Pointer to the NSS core context.
 *
 * @dependencies
 * The caller must provide the valid core ID that is being profiled.
 */
extern void *nss_profiler_notify_register(nss_core_id_t core_id, nss_profiler_callback_t profiler_callback, void *ctx);

/**
 * nss_profiler_notify_unregister
 *	Deregisters the Profiler interface from the NSS driver.
 *
 * @datatypes
 * nss_core_id_t
 *
 * @param[in] core_id  NSS core ID.
 *
 * @return
 * None.
 *
 * @dependencies
 * The interface must have been previously registered.
 */
extern void nss_profiler_notify_unregister(nss_core_id_t core_id);

/**
 * nss_profiler_if_tx_buf
 *	Sends a Profiler command to the NSS firmware.
 *
 * @param[in] nss_ctx   Pointer to the NSS context.
 * @param[in] buf       Buffer to send to NSS firmware.
 * @param[in] len       Length of the buffer.
 * @param[in] cb        Pointer to the message callback.
 * @param[in] app_data  Pointer to the application context of the message.
 *
 * @return
 * Status of the Tx operation.
 *
 * @dependencies
 * A valid context must be provided (for the right core).
 * This context was returned during registration.
 */
extern nss_tx_status_t nss_profiler_if_tx_buf(void *nss_ctx,
		void *buf, uint32_t len, void *cb, void *app_data);

/**
 * profile_register_performance_counter
 *	Registers a Linux counter with the profiler for any variables.
 *
 * @param[in] counter	Pointer to the variable address.
 * @param[in] name	Pointer to the variable name: if name is longer than
			23 characters, then only the first 23 bytes are used.
 *
 * @return
 * 0	if counter array is full -- too many registered counters.
 * 1	on success
 */
extern int profile_register_performance_counter(volatile unsigned int *counter, char *name);

/**
 * nss_profiler_msg_init
 *	Initializes a Profiler-specific message.
 *
 * @datatypes
 * nss_profiler_msg \n
 * nss_profiler_callback_t
 *
 * @param[in,out] npm       Pointer to the NSS Profiler message.
 * @param[in]     if_num    NSS interface number.
 * @param[in]     type      Type of message.
 * @param[in]     len       Size of the message.
 * @param[in]     cb        Callback function for the message.
 * @param[in]     app_data  Pointer to the application context of the message.
 *
 * @return
 * None.
 */
extern void nss_profiler_msg_init(struct nss_profiler_msg *npm, uint16_t if_num,
				uint32_t type, uint32_t len,
				nss_profiler_callback_t cb, void *app_data);

/**
 * @}
 */

#endif
