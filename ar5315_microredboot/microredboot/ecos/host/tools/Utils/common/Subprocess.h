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
//===========================================================================
//#####DESCRIPTIONBEGIN####
//
// Author(s): 	sdf
// Contact(s):	sdf
// Date:		1998/08/11
// Version:		0.01
// Purpose:	
// Description:	Interface of the subprocess clas
// Requires:	
// Provides:	
// See also:    
// Known bugs:	
// Usage:	
//
//####DESCRIPTIONEND####
//
//===========================================================================
#ifndef _SUBPROCESS_H
#define _SUBPROCESS_H

//===========================================================================
// This class spawns subprocesses in a host-independent manner [almost]
//===========================================================================

#include "eCosStd.h"
#include "eCosThreadUtils.h"
#include "Collections.h"

class CSubprocess {

public:
  void SetPath (LPCTSTR pszPath) { m_strPath=pszPath; }
	const String ErrorString() const;

  // If bAutodelete is set, the class object will delete itself when the process is finished.
  // This must only be used if the class object is allocated on the heap.
  CSubprocess(bool bAutodelete=false);
	virtual ~CSubprocess();
 
  void SetVerbose   (bool b)         { m_bVerbose=b; }
  void SetDirectory (LPCTSTR pszDir) { m_strDir=pszDir; }

  // Various forms of the Run function.  In each case under UNIX the Run results will always return true
  // (because we can't determine the result of the exec after fork).  Under NT the result correctly represents
  // whether the process creation was successful

  // Run (blocking) sending the output to strOutput.
  bool Run(String &strOutput,LPCTSTR pszCmd) { return Run(AppendFunc,&strOutput,pszCmd,true); }

  // Run sending output to callback
  bool Run(LogFunc *pfnLog,void * pLogparam,LPCTSTR pszCmd,bool bBlock=true);
  
  int Pid() const { return m_idProcess; } // returns process id (even when process is terminated)

  // Get the CPU time of the process (and, optionally, its children)
  // Note that under UNIX this involves running ps and so may not be that cheap.
  Time CpuTime(bool bRecurse=true) const;

  // Get the process exit code.  This can be:
  //   exit code of process (if terminated)
  //   0xffffffff (if process not yet run)
  //   GetLastError result (if process could not be run)
  int GetExitCode() { return m_nExitCode; }
  
  // Kill the process:
  bool Kill(bool bRecurse=true);
  
  // Send some input to the process:
  void Send (LPCTSTR psz); 
  // Close it (cause EOF to be read)
	void CloseInput();
  // Is the process running?
	bool ProcessAlive();

  // Appendfunc can be used to achieve a non-blocking addition to some string
  static void CALLBACK AppendFunc(void *pParam,LPCTSTR psz) { 
    ENTERCRITICAL;
    *((String *)pParam)+=psz; 
    LEAVECRITICAL;
  }

  // This function may be used to stop a process given some condition evaluated externally,
  // As long as the function returns true the process will be allowed to continue
  typedef bool (CALLBACK ContinuationFunc)(void *);
  void SetContinuationFunc(ContinuationFunc *pfnContinue,void *pParam){m_pfnContinue=pfnContinue;m_pContinuationFuncParam=pParam;}

  // Wait for completion of the process, with optional timeout.  If the timeout occurs without the process
  // having terminated, the result will be false.
  bool Wait(Duration dTimeout=0x7fffffff);

protected:
	String m_strPath;
	
  static const String Name (int pid); // for debugging - only works under NT

  ContinuationFunc *m_pfnContinue;
  void *m_pContinuationFuncParam;
  static bool CALLBACK DefaultContinuationFunc(void *) { return true; }

  static void CALLBACK NullLogFunc(void *,LPCTSTR) {}

  struct PInfo;
  struct PInfo {
    PInfo *pParent;
#ifdef _WIN32
    __int64 tCreation;
#endif
    Time tCpu;
    int PID;
    int PPID;
    bool IsChildOf(int pid) const;
  };

  typedef std::vector<PInfo> PInfoArray;

  static bool PSExtract(PInfoArray &arPinfo);
  static void SetParents(PInfoArray &arPinfo);

#ifdef _WIN32
	static DWORD GetPlatform();
	HANDLE m_hrPipe;
	HANDLE m_hwPipe;
  HANDLE m_hProcess;     // This handle is "owned" by the ThreadFunc
  static HINSTANCE hInstLib1, hInstLib2;
	int m_nErr;
#else
  int m_tty;
  String m_strCmd;
#endif

  static void CALLBACK SThreadFunc(void *pParam) { ((CSubprocess *)pParam)->ThreadFunc(); }
  void ThreadFunc();
	bool m_bAutoDelete;
	bool m_bThreadTerminated;
	bool m_bVerbose;
  int m_nExitCode;
  int m_idProcess;
  void *m_pLogparam;
  LogFunc *m_pfnLogfunc;
  bool m_bKillThread;

  static const unsigned int PROCESS_KILL_EXIT_CODE;
	bool CreateProcess(LPCTSTR pszCmdline);

  struct CygProcessInfo {
    int nPid;
    int nPpid;
    int nPgid;
    int nWinpid;
  };


  void Output(LPCTSTR psz);
  String m_strDir;
};

#endif
