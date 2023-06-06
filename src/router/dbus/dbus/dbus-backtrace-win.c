/* -*- mode: C; c-file-style: "gnu"; indent-tabs-mode: nil; -*- */
/* dbus-backtrace-win.c Backtrace Generator
 *
 * Copyright 2004 Eric Poech
 * Copyright 2004 Robert Shearman
 * Copyright 2010 Patrick von Reth <patrick.vonreth@gmail.com>
 * Copyright 2015 Ralf Habacker <ralf.habacker@freenet.de>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */

#include <config.h>

#include "dbus-internals.h"
#include "dbus-sysdeps.h"

#if !defined (DBUS_DISABLE_ASSERT) || defined(DBUS_ENABLE_EMBEDDED_TESTS)

#if defined(_MSC_VER) || defined(DBUS_WINCE)
# ifdef BACKTRACES
#  undef BACKTRACES
# endif
#else
# define BACKTRACES
#endif

#ifdef BACKTRACES
#include "dbus-sysdeps-win.h"

#include <stdio.h>
#include <windows.h>
#include <imagehlp.h>

#define DPRINTF(fmt, ...) fprintf (stderr, fmt, ##__VA_ARGS__)

#ifdef _MSC_VER
#define BOOL int

#define __i386__
#endif

static void dump_backtrace_for_thread (HANDLE hThread)
{
  ADDRESS old_address;
  STACKFRAME sf;
  CONTEXT context;
  DWORD dwImageType;
  int i = 0;

  SymSetOptions (SYMOPT_UNDNAME | SYMOPT_LOAD_LINES);
  SymInitialize (GetCurrentProcess (), NULL, TRUE);


  /* can't use this function for current thread as GetThreadContext
   * doesn't support getting context from current thread */
  if (hThread == GetCurrentThread())
    return;

  DPRINTF ("Backtrace:\n");

  _DBUS_ZERO (old_address);
  _DBUS_ZERO (context);
  context.ContextFlags = CONTEXT_FULL;

  SuspendThread (hThread);

  if (!GetThreadContext (hThread, &context))
    {
      DPRINTF ("Couldn't get thread context (error %ld)\n", GetLastError ());
      ResumeThread (hThread);
      return;
    }

  _DBUS_ZERO (sf);

#ifdef __i386__
  dwImageType         = IMAGE_FILE_MACHINE_I386;
  sf.AddrFrame.Offset = context.Ebp;
  sf.AddrFrame.Mode   = AddrModeFlat;
  sf.AddrPC.Offset    = context.Eip;
  sf.AddrPC.Mode      = AddrModeFlat;
#elif defined(_M_X64)
  dwImageType         = IMAGE_FILE_MACHINE_AMD64;
  sf.AddrPC.Offset    = context.Rip;
  sf.AddrPC.Mode      = AddrModeFlat;
  sf.AddrFrame.Offset = context.Rsp;
  sf.AddrFrame.Mode   = AddrModeFlat;
  sf.AddrStack.Offset = context.Rsp;
  sf.AddrStack.Mode   = AddrModeFlat;
#elif defined(_M_IA64)
  dwImageType         = IMAGE_FILE_MACHINE_IA64;
  sf.AddrPC.Offset    = context.StIIP;
  sf.AddrPC.Mode      = AddrModeFlat;
  sf.AddrFrame.Offset = context.IntSp;
  sf.AddrFrame.Mode   = AddrModeFlat;
  sf.AddrBStore.Offset= context.RsBSP;
  sf.AddrBStore.Mode  = AddrModeFlat;
  sf.AddrStack.Offset = context.IntSp;
  sf.AddrStack.Mode   = AddrModeFlat;
#else
# error You need to fill in the STACKFRAME structure for your architecture
#endif

  /*
    backtrace format
    <level> <address> <symbol>[+offset] [ '[' <file> ':' <line> ']' ] [ 'in' <module> ]
    example:
      6 0xf75ade6b wine_switch_to_stack+0x2a [/usr/src/debug/wine-snapshot/libs/wine/port.c:59] in libwine.so.1
  */
  while (StackWalk (dwImageType, GetCurrentProcess (),
                    hThread, &sf, &context, NULL, SymFunctionTableAccess,
                    SymGetModuleBase, NULL))
    {
      char buffer[sizeof(SYMBOL_INFO) + MAX_SYM_NAME * sizeof(char)];
      PSYMBOL_INFO pSymbol = (PSYMBOL_INFO)buffer;
      DWORD64 displacement;
      IMAGEHLP_LINE line;
      DWORD dwDisplacement;
      IMAGEHLP_MODULE moduleInfo;

      /*
         on Wine64 version 1.7.54, we get an infinite number of stack entries
         pointing to the same stack frame  (_start+0x29 in <wine-loader>)
         see bug https://bugs.winehq.org/show_bug.cgi?id=39606
      */
#ifndef __i386__
      if (old_address.Offset == sf.AddrPC.Offset)
        {
          break;
        }
#endif

      pSymbol->SizeOfStruct = sizeof(SYMBOL_INFO);
      pSymbol->MaxNameLen = MAX_SYM_NAME;

      if (SymFromAddr (GetCurrentProcess (), sf.AddrPC.Offset, &displacement, pSymbol))
        {
          if (displacement)
            DPRINTF ("%3d %s+0x%I64x", i++, pSymbol->Name, displacement);
          else
            DPRINTF ("%3d %s", i++, pSymbol->Name);
        }
      else
        DPRINTF ("%3d 0x%Ix", i++, sf.AddrPC.Offset);

      line.SizeOfStruct = sizeof(IMAGEHLP_LINE);
      if (SymGetLineFromAddr (GetCurrentProcess (), sf.AddrPC.Offset, &dwDisplacement, &line))
        {
          DPRINTF (" [%s:%ld]", line.FileName, line.LineNumber);
        }

      moduleInfo.SizeOfStruct = sizeof(moduleInfo);
      if (SymGetModuleInfo (GetCurrentProcess (), sf.AddrPC.Offset, &moduleInfo))
        {
          DPRINTF (" in %s", moduleInfo.ModuleName);
        }
      DPRINTF ("\n");
      old_address = sf.AddrPC;
    }
  ResumeThread (hThread);
}

static DWORD WINAPI dump_thread_proc (LPVOID lpParameter)
{
  dump_backtrace_for_thread ((HANDLE) lpParameter);
  return 0;
}

/* cannot get valid context from current thread, so we have to execute
 * backtrace from another thread */
static void
dump_backtrace (void)
{
  HANDLE hCurrentThread;
  HANDLE hThread;
  DWORD dwThreadId;
  DuplicateHandle (GetCurrentProcess (), GetCurrentThread (),
                   GetCurrentProcess (), &hCurrentThread,
                   0, FALSE, DUPLICATE_SAME_ACCESS);
  hThread = CreateThread (NULL, 0, dump_thread_proc, (LPVOID)hCurrentThread,
                          0, &dwThreadId);
  WaitForSingleObject (hThread, INFINITE);
  CloseHandle (hThread);
  CloseHandle (hCurrentThread);
}
#endif
#endif /* asserts or tests enabled */

#ifdef BACKTRACES
void _dbus_print_backtrace (void)
{
  dump_backtrace ();
}
#else
void _dbus_print_backtrace (void)
{
  _dbus_verbose ("  D-Bus not compiled with backtrace support\n");
}
#endif
