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
//=================================================================
//
//        eCosThreadUtils.h
//
//
//=================================================================
//=================================================================
//#####DESCRIPTIONBEGIN####
//
// Author(s):     sdf
// Contributors:  sdf
// Date:          1999-04-01
// Description:   Threading-related utiltities 
// Usage:
//
//####DESCRIPTIONEND####

#ifndef _ECOSTHREADUTILS_H
#define _ECOSTHREADUTILS_H
#include "eCosStd.h"
#include "Collections.h"

#ifndef _WIN32 // UNIX
  #ifndef NO_THREADS
    #include <pthread.h>
  #endif 
#endif

//=================================================================
// This class handles threads in a host-independent manner.  
// It also contains a few thread-related functions such as Sleep
//=================================================================

class CeCosThreadUtils {
public:

#ifdef _WIN32
  typedef DWORD THREAD_ID;
#else // UNIX
  #ifndef NO_THREADS
    typedef int THREAD_ID;
  #else
    typedef pthread_t THREAD_ID;
  #endif
#endif

  static THREAD_ID GetThreadId(); // Get my current thread ID, mostly for debugging
  
  // CS supports a single system-wide critical sections (recursive mutexes).
  // You are expected to use macros ENTERCRITICAL and LEAVECRITICAL to use this class - these macros define
  // a block containing a CS object, which has the effect of creating a critical section.
  // Exit from the block (by whatever means, including an exception) causes the CS dtor
  // to be called so as to release the section.

  class CS{
  public:
    static bool InCriticalSection();     // This thread owns the critical section
    CS();
    virtual ~CS();
  protected:
    static int m_nCriticalSectionLock; // The number of times the recursive mutex has been locked.  Management of this allows us to avoid use of true recursive mutexes on UNIX.
    static THREAD_ID nCSOwner;         // The thread owning the resource.
#ifdef _WIN32
    static CRITICAL_SECTION cs;        // The one and only critical section
    static bool bCSInitialized;
#else // UNIX
    #ifndef NO_THREADS
    static pthread_mutex_t cs;         // The one and only critical section
    #endif
#endif
  };

  #define ENTERCRITICAL {CeCosThreadUtils::CS c
  #define LEAVECRITICAL }
  
  static int AtomicIncrement (int &n); // return old value
  static int AtomicDecrement (int &n); // return old value

  
  // Wait for this boolean to become true, subject to the given timeout
  // If the timeout happens first, the return code will be false - otherwise true
  static bool WaitFor (bool &b, int dTimeout=0x7fffffff);
  
  ///////////////////////////////////////////////////////////////////////////
  // Define the characteristics of a callback procedure:
  
  // A callback procedure, used both for thread entry points and thread completion callbacks
  typedef void (CALLBACK CallbackProc)(void *);

  // Run a thread: pThreadFunc is the entry point (passed pParam).  No notification of completion.
  static bool RunThread(CallbackProc *pThreadFunc, void *pParam, LPCTSTR pszName=_T("")) { return RunThread(pThreadFunc,pParam,0,0,pszName); }
  // Run a thread, setting the bool on completion
  static bool RunThread(CallbackProc *pThreadFunc, void *pParam, bool *pb, LPCTSTR pszName=_T("")) { *pb=false; return RunThread(pThreadFunc,pParam,0,pb,pszName); }
  // Run a thread, calling the callback on completion
  static bool RunThread(CallbackProc *pThreadFunc, void *pParam, CallbackProc *pCompletionFunc, LPCTSTR pszName=_T("")) { return RunThread(pThreadFunc,pParam,pCompletionFunc,pParam,pszName); }
  
  static void Sleep (int nMsec);

protected:

  // Run a thread: arbitrary callbcak
  static bool RunThread(CallbackProc *pThreadFunc, void *pParam, CallbackProc *pCompletionFunc, void *pCompletionParam, LPCTSTR pszName);
  
  // This is the information that is passed to the host-specific thread proc.  It is simply enough to call the thread entry point and
  // call the callback (or set the boolean) at the end.
  struct ThreadInfo {
    CallbackProc *pThreadFunc;      // The thread proc is this function
    void         *pThreadParam;     //            - called with this parameter
    CallbackProc *pCompletionFunc;  // At the end - call this function
    void         *pCompletionParam; //              with this parameter
    String       strName;           // For debugging
    ThreadInfo (CallbackProc *_pThreadFunc,void *_pThreadParam,CallbackProc *_pCompletionFunc,void  *_pCompletionParam,LPCTSTR pszName) :
      pThreadFunc(_pThreadFunc),
      pThreadParam(_pThreadParam),
      pCompletionFunc(_pCompletionFunc),
      pCompletionParam(_pCompletionParam),
      strName(pszName){}
  };

  // THREADFUNC is the result type of the thread function
  #ifdef _WIN32
    typedef unsigned long THREADFUNC; 
    static int CALLBACK FilterFunction(LPEXCEPTION_POINTERS p);
  #else // UNIX
    typedef void * THREADFUNC;
  #endif
  static THREADFUNC CALLBACK SThreadFunc (void *pParam);
    
};

#endif
