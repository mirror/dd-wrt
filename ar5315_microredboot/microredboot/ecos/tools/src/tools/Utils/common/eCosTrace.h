//####COPYRIGHTBEGIN####
//                                                                          
// ----------------------------------------------------------------------------
// Copyright (C) 1998, 1999, 2000 Red Hat, Inc.
//
// This program is part of the eCos host tools.
//
// This program is free software; you can redistribute it and/or modify it 
// under the terms of the GNU General Public License as published by the Free 
// Software Foundation; either version 2 of the License, or (at your option) 
// any later version.
// 
// This program is distributed in the hope that it will be useful, but WITHOUT 
// ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or 
// FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for 
// more details.
// 
// You should have received a copy of the GNU General Public License along with
// this program; if not, write to the Free Software Foundation, Inc., 
// 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
//
// ----------------------------------------------------------------------------
//                                                                          
//####COPYRIGHTEND####
//#####DESCRIPTIONBEGIN####
//
// Author(s):     sdf
// Contributors:  sdf
// Date:          1999-04-01
// Description:   Standard include file for test infra
// Usage:
//
//####DESCRIPTIONEND####

//=================================================================
// This class handles output of errors, debugging trace and so on.  All its members are static, so it's really a namespace :-).
// It handles two output streams - error and output - and allows these to be redirected independently to files etc...
// Part of the justification for this (which might be carried out on the command line) involves shortcomings of
// SAMBA client, which deals badly with the flushing of file buffers.
// The definition of LogFunc, which defines a function (void *,LPCTSTR) to which output is sent, is in eCosStd.
//=================================================================

#ifndef _ECOSTRACE_H
#define _ECOSTRACE_H
#include "eCosStd.h"
#include "Collections.h"

class CeCosTrace {
public:
	static const String Timestamp();
  // Diagnostic output
  static void Out(LPCTSTR  psz) { pfnOut(pOutParam,psz); }
  static void Err(LPCTSTR  psz) { pfnError(pErrorParam,psz); } // Send to "stderr" 
  static void TimeStampedErr(LPCTSTR pszFormat,...);

  enum TraceLevel {TRACE_LEVEL_ERRORS, TRACE_LEVEL_TRACE, TRACE_LEVEL_VTRACE}; // These are the levels of trace
  
  // Here's how to set and get the current trace value:
  static void EnableTracing(TraceLevel n) { nVerbosity=n; }
  static TraceLevel TracingEnabled() { return nVerbosity; }

  static void SetInteractive(bool b) { bInteractive=b; } // Declare this program to be "interactive" (usually means command-line)
  static bool IsInteractive() { return bInteractive; }

  static void SetOutput (LogFunc *pFn,void *pParam) { pfnOut=pFn; pOutParam=pParam; }     // Make stdout go to this callback
  static void SetError  (LogFunc *pFn,void *pParam) { pfnError=pFn; pErrorParam=pParam; } // Make stderr go to this callback

  static bool SetOutput (LPCTSTR pszFilename); // Make stdout go to this file
  static bool SetError  (LPCTSTR pszFilename); // Make stderr go to this file

  // Some macros...
  #ifndef TRACE // because if running under a debugger we might have a better definition (via OutputDebugString) already
     // Use this to generate output that will only appear if trace level is at least TRACE_LEVEL_TRACE (turned on by -v)
     #define TRACE if(CeCosTrace::TracingEnabled()>=CeCosTrace::TRACE_LEVEL_TRACE) CeCosTrace::TimeStampedErr
  #endif

  #undef ERROR

  #define ERROR CeCosTrace::TimeStampedErr
  // Use this to generate output that will only appear if trace level is at least TRACE_LEVEL_VTRACE (turned on by -V)
  #define VTRACE if(CeCosTrace::TracingEnabled()>=CeCosTrace::TRACE_LEVEL_VTRACE) CeCosTrace::TimeStampedErr
  
  // Use this to generate output that will only appear if mode is interactive (see above)
  #define INTERACTIVE if(CeCosTrace::IsInteractive()||CeCosTrace::TracingEnabled()>=CeCosTrace::TRACE_LEVEL_TRACE) CeCosTrace::TimeStampedErr

  // Thus log function can be used to direct output to a FILE* (e.g. stdout) passed as the first argument:
  static void CALLBACK StreamLogFunc  (void *, LPCTSTR psz);

protected:
  // Information we need to know for a stream (error or output)
  struct StreamInfo {
    Time tLastReopen;
    String strFilename;
    FILE *f;
    StreamInfo(LPCTSTR pszFile,FILE *_f) : tLastReopen(Now()),strFilename(pszFile),f(_f) {}
    ~StreamInfo() { fclose(f); }
  };
  
  // Here are the two streams
  static StreamInfo OutInfo,ErrInfo;

  static void CALLBACK StreamInfoFunc (void *, LPCTSTR psz);
  
  static LPCTSTR arpszDow[7];

  static TraceLevel nVerbosity;
  static bool bInteractive;
  static LogFunc *pfnOut;    static void *pOutParam;
  static LogFunc *pfnError;  static void *pErrorParam;
};
#endif
