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
#include "eCosThreadUtils.h"
#include "eCosTrace.h"

CeCosThreadUtils::THREAD_ID CeCosThreadUtils::CS::nCSOwner=(CeCosThreadUtils::THREAD_ID)-1;
int  CeCosThreadUtils::CS::m_nCriticalSectionLock=0;
#ifdef _WIN32
  CRITICAL_SECTION CeCosThreadUtils::CS::cs;
  bool CeCosThreadUtils::CS::bCSInitialized=false;
#else // UNIX
  // Static recursive mutex for unix critical section
  #ifndef NO_THREADS
    pthread_mutex_t CeCosThreadUtils::CS::cs = PTHREAD_MUTEX_INITIALIZER;
  #endif
#endif


CeCosThreadUtils::THREAD_ID CeCosThreadUtils::GetThreadId()
{
  return 
  #ifdef _WIN32
    GetCurrentThreadId();
  #else // UNIX
    #ifdef NO_THREADS
    42;
    #else
    pthread_self();
    #endif
  #endif
}

// Wait for the specified Boolean to turn true or the timeout to occur
// The value of the Boolean is returned as result
bool CeCosThreadUtils::WaitFor (bool &b, Duration dTimeout)
{
  Time t=Now();
  do {
    if(b){
      break;
    }
    Sleep(250);
  } while (Now()-t<dTimeout);
  return b;
}

// This function spawns a thread and causes it to run asynchrously.
// The callback may be
//      A function, which is called when the thread completes
//      A boolean, which is set when the thread completes
//      Null, in which case no notification is received of thread completion
// pParam is passed to the thread function pThreadFunc on thread initiation.

bool CeCosThreadUtils::RunThread(CallbackProc *pThreadFunc, void *pParam, CallbackProc *pCompletionFunc, void *pCompletionParam, LPCTSTR pszName)
{
  TRACE(_T("RunThread %s\n"),pszName);
  // Do not spawn a thread while in possession of a mutex
  //    ENTERCRITICAL;
  //        VTRACE(_T("csl=%d\n"),CS::m_nCriticalSectionLock);
  //        assert(1==CS::m_nCriticalSectionLock);
  //    LEAVECRITICAL;
  
  ThreadInfo *pInfo=new ThreadInfo(pThreadFunc,pParam,pCompletionFunc,pCompletionParam,pszName); // SThreadFunc will delete
  bool rc=false;

#ifdef _WIN32
  // FIXME: CreateThread incompatible with C runtime calls?
  DWORD dwID;
  HANDLE hThread=CreateThread(NULL,0,SThreadFunc, pInfo, 0, &dwID);
  if(hThread){
    ::CloseHandle(hThread);
    rc=true;
  } else {
    ERROR(_T("Failed to create thread\n"));
  }
#else // UNIX
  #ifdef NO_THREADS
  assert(false);
  #else
  pthread_t hThread;
  int n=pthread_create(&hThread, NULL, SThreadFunc, pInfo);
  TRACE(  _T("RunThread: - non-blocking call (new thread=%x)\n"),hThread);
  if (n != 0) {
    ERROR(_T("RunThread(): pthread_create failed - %s\n"),strerror(errno));
  } else {
    
    int n = pthread_detach(hThread);
    
    if (0==n) {
      rc=true;
    } else {
      ERROR(_T("RunThread(): pthread_detach failed - %s\n"),strerror(errno));
      hThread=0;
    }
  }
  #endif
#endif
  if(!rc){
    delete pInfo;
  }
  return rc;
  
}

#ifdef _WIN32
int CALLBACK CeCosThreadUtils::FilterFunction(EXCEPTION_POINTERS *p)
{
  EXCEPTION_RECORD *pRec    =p->ExceptionRecord;
  TRACE(_T("!!!Exception!!! address=%08x code=%08x\n"),pRec->ExceptionAddress,pRec->ExceptionCode);
  /*
  CONTEXT          *pContext=p->ContextRecord;
  const unsigned char *ESP=(const unsigned char *)pContext->Esp;
  for(int i=0;i<16;i++){
  TRACE(_T("%08X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X\n"),ESP,
  ESP[0],ESP[1],ESP[2],ESP[3],ESP[4],ESP[5],ESP[6],ESP[7],ESP[8],ESP[9],ESP[10],ESP[11],ESP[12],ESP[13],ESP[14],ESP[15]);
  ESP+=16;
  }
  */
  return EXCEPTION_EXECUTE_HANDLER;
}
#endif

CeCosThreadUtils::THREADFUNC CALLBACK CeCosThreadUtils::SThreadFunc (void *pParam)
{
  THREAD_ID id=GetThreadId();
  ThreadInfo *pInfo=(ThreadInfo*)pParam;
  TRACE(_T("Thread %x [%s] created\n"),id,(LPCTSTR)pInfo->strName);
#if defined(_WIN32) && !defined(__CYGWIN__)
  __try {
    // Call what we are instructed to (e.g. LocalThreadFunc):
    pInfo->pThreadFunc(pInfo->pThreadParam);
  }
    __except ( CeCosThreadUtils::FilterFunction(GetExceptionInformation() )) 
  { 
    TRACE(_T("Handling exception!!!...\n"));
  }
#else // UNIX
  try {
    // Call what we are instructed to (e.g. LocalThreadFunc):
    pInfo->pThreadFunc(pInfo->pThreadParam);
  }
  catch (...){
    TRACE(_T("Exception caught in SThreadFunc!!!\n"));
  }
#endif
  
  // Call the Callback:
  if(pInfo->pCompletionFunc){
    pInfo->pCompletionFunc (pInfo->pCompletionParam);
  } else if (pInfo->pCompletionParam) {
    // No function - just a flag:
    *(bool *)pInfo->pCompletionParam=true;
  }
  // No more references to pInfo->pTest from now on...
  TRACE(_T("Thread %x [%s] terminated\n"),id,(LPCTSTR)pInfo->strName);
  delete pInfo;
  return 0;
}

bool CeCosThreadUtils::CS::InCriticalSection()
{
  return GetThreadId()==nCSOwner;
}

int CeCosThreadUtils::AtomicIncrement (int &n)
{
  int rc;
  ENTERCRITICAL;
  rc=n++;
  LEAVECRITICAL;
  return rc;
}

int CeCosThreadUtils::AtomicDecrement (int &n)
{
  int rc;
  ENTERCRITICAL;
  rc=n--;
  LEAVECRITICAL;
  return rc;
}

CeCosThreadUtils::CS::CS()
{
  // Get mutex lock; block until available unless current
  // thread already owns the mutex.
  if(!InCriticalSection()){
#ifdef _WIN32
    if(!bCSInitialized){
		    InitializeCriticalSection(&cs);
        bCSInitialized=true;
    }
    ::EnterCriticalSection(&cs);
#else // UNIX
    #ifndef NO_THREADS
      pthread_mutex_lock(&cs);
    #endif
#endif
    // As we now own the CS it is safe to perform the following assignment:
    nCSOwner=GetThreadId();
  }
  // As we now own the CS it is safe to perform the following increment:
  m_nCriticalSectionLock++;
}

CeCosThreadUtils::CS::~CS()
{
  assert(InCriticalSection());
  // As we own the CS we can safely manipulate variables:
  m_nCriticalSectionLock--;
  assert(m_nCriticalSectionLock>=0);
  if(0==m_nCriticalSectionLock){
    // Last lock is being released - let go of the mutex
    nCSOwner=(THREAD_ID)-1;
    
    // Release mutex lock.  
#ifdef _WIN32
    ::LeaveCriticalSection(&cs);
#else // UNIX
    #ifndef NO_THREADS
      pthread_mutex_unlock(&cs);
    #endif
#endif
  }
}

void CeCosThreadUtils::Sleep(int nMsec)
{
#ifdef _WIN32
  ::Sleep(nMsec);
#else
  sched_yield();
  usleep((int)nMsec * 1000);
#endif
}
