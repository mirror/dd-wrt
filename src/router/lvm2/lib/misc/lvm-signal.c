/*
 * Copyright (C) 2001-2004 Sistina Software, Inc. All rights reserved.
 * Copyright (C) 2004-2014 Red Hat, Inc. All rights reserved.
 *
 * This file is part of LVM2.
 *
 * This copyrighted material is made available to anyone wishing to use,
 * modify, copy, or redistribute it subject to the terms and conditions
 * of the GNU Lesser General Public License v.2.1.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

#include "lib/misc/lib.h"
#include "lib/misc/lvm-signal.h"
#include "lib/mm/memlock.h"

#include <signal.h>

static sigset_t _oldset;
static int _signals_blocked = 0;
static volatile sig_atomic_t _sigint_caught = 0;
static volatile sig_atomic_t _handler_installed = 0;

/* Support 3 level nesting, increase if needed more */
#define MAX_SIGINTS 3
static struct sigaction _oldhandler[MAX_SIGINTS];
static int _oldmasked[MAX_SIGINTS];

static void _catch_sigint(int unused __attribute__((unused)))
{
	_sigint_caught = 1;
}

int sigint_caught(void) {
	if (_sigint_caught)
		log_error("Interrupted...");

	return _sigint_caught;
}

void sigint_clear(void)
{
	_sigint_caught = 0;
}

/*
 * Temporarily allow keyboard interrupts to be intercepted and noted;
 * saves interrupt handler state for sigint_restore().  Users should
 * use the sigint_caught() predicate to check whether interrupt was
 * requested and act appropriately.  Interrupt flags are never
 * cleared automatically by this code, but the tools clear the flag
 * before running each command in lvm_run_command().  All other places
 * where the flag needs to be cleared need to call sigint_clear().
 */

void sigint_allow(void)
{
	struct sigaction handler;
	sigset_t sigs;

	if (memlock_count_daemon())
		return;
	/*
	 * Do not overwrite the backed-up handler data -
	 * just increase nesting count.
	 */
	if (++_handler_installed > MAX_SIGINTS)
		return;

	/* Grab old sigaction for SIGINT: shall not fail. */
	if (sigaction(SIGINT, NULL, &handler))
		log_sys_debug("sigaction", "SIGINT");

	handler.sa_flags &= ~SA_RESTART; /* Clear restart flag */
	handler.sa_handler = _catch_sigint;

	/* Override the signal handler: shall not fail. */
	if (sigaction(SIGINT, &handler, &_oldhandler[_handler_installed  - 1]))
		log_sys_debug("sigaction", "SIGINT");

	/* Unmask SIGINT.  Remember to mask it again on restore. */
	if (sigprocmask(0, NULL, &sigs))
		log_sys_debug("sigprocmask", "");

	if ((_oldmasked[_handler_installed - 1] = sigismember(&sigs, SIGINT))) {
		sigdelset(&sigs, SIGINT);
		if (sigprocmask(SIG_SETMASK, &sigs, NULL))
			log_sys_debug("sigprocmask", "SIG_SETMASK");
	}
}

void sigint_restore(void)
{
	if (memlock_count_daemon())
		return;

	if (!_handler_installed ||
	    --_handler_installed >= MAX_SIGINTS)
		return;

	/* Nesting count went below MAX_SIGINTS. */
	if (_oldmasked[_handler_installed]) {
		sigset_t sigs;
		sigprocmask(0, NULL, &sigs);
		sigaddset(&sigs, SIGINT);
		if (sigprocmask(SIG_SETMASK, &sigs, NULL))
			log_sys_debug("sigprocmask", "SIG_SETMASK");
	}

	if (sigaction(SIGINT, &_oldhandler[_handler_installed], NULL))
		log_sys_debug("sigaction", "SIGINT restore");
}

void block_signals(uint32_t flags __attribute__((unused)))
{
	sigset_t set;

	if (memlock_count_daemon())
		return;

	if (_signals_blocked)
		return;

	if (sigfillset(&set)) {
		log_sys_error("sigfillset", "_block_signals");
		return;
	}

	if (sigprocmask(SIG_SETMASK, &set, &_oldset)) {
		log_sys_error("sigprocmask", "_block_signals");
		return;
	}

	_signals_blocked = 1;
}

void unblock_signals(void)
{
	if (memlock_count_daemon())
		return;

	/* Don't unblock signals while any locks are held */
	if (!_signals_blocked)
		return;

	if (sigprocmask(SIG_SETMASK, &_oldset, NULL)) {
		log_sys_error("sigprocmask", "_block_signals");
		return;
	}

	_signals_blocked = 0;
}
