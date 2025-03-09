/*
 *	Userspace-level compat hacks
 */
#pragma once
/* linux-glibc-devel 2.6.34 header screwup */
#ifndef ALIGN
#	define ALIGN(s, n) (((s) + ((n) - 1)) & ~((n) - 1))
#endif
