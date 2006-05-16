/******************************************************************************/
/* File: threadexception.cpp                                                  */
/******************************************************************************/
/* This source code file is the source file for the ThreadException class -   */
/* this class encapsulates any exceptions thrown in the pthreadcc C++ library.*/
/******************************************************************************/
/* Written By: Jason But                                                      */
/* Copyright:  CTIE - Monash University 2001                                  */
/*                                                                            */
/* Notes:                                                                     */
/*    Version 1.00 - Original Version of module.                              */
/*                                                                            */
/*    Version 1.01 - Non specification of default parameter values to keep    */
/*                   some compilers happy.                                    */
/******************************************************************************/

/******************************************************************************/
/* Standard C Includes.                                                       */
/******************************************************************************/
#include <string.h>

/******************************************************************************/
/* Include class header file.                                                 */
/******************************************************************************/
#include "pthreadcc.h"

/******************************************************************************/
/* Class ThreadException                                                      */
/******************************************************************************/
/* ThreadException Constructor.                                               */
/*                                                                            */
/* The Exception type is set to the type specified during construction.  We   */
/* then execute a switch statement on the exception type to set the correct   */
/* error string in pcExceptionString.                                         */
/******************************************************************************/
ThreadException::ThreadException(ErrorCodes ecType) : ecException(ecType)
{
    switch (ecException)
    {
        case errMutexUnknown:         SetString("Unknown Mutual Exclusion Error\n");
                                      break;
        case errMutexWouldDeadlock:   SetString("Error Checking MutEx - Calling Thread has already locked this MutEx\n");
                                      break;
        case errMutexNotOwned:        SetString("Error Checking MutEx - Calling Thread does not own this MutEx to Unlock it\n");
                                      break;
        case errSemUnknown:           SetString("Unknown Semaphore Error\n");
                                      break;
        case errMaxSemCount:          SetString("Operation would increase the Semaphore Count above the maximum value\n");
                                      break;
        case errThreadLaunch:         SetString("Unable to create a process to support the new thread\n");
                                      break;
        case errUnknown:              SetString("An unknown error occured\n");
    }
}

/******************************************************************************/
/* ThreadException Copy Constructor.                                          */
/*                                                                            */
/* This constructor is called when making a copy of an existing class         */
/* instance.  We copy the exception cose and set the string parameter to the  */
/* copy of the string in the existing instance.                               */
/******************************************************************************/
ThreadException::ThreadException(const ThreadException &cOriginal)
{
    ecException = cOriginal.ecException;
    SetString(cOriginal.pcExceptionString);
}

/******************************************************************************/
/* ThreadException Destructor.                                                */
/*                                                                            */
/* Free any memory allocated to the error string.                             */
/******************************************************************************/
ThreadException::~ThreadException()
{
    delete pcExceptionString;
}

/******************************************************************************/
/* void SetString(const char *pcErrString)                                    */
/*                                                                            */
/* This private member method sets the internal error string to a copy of the */
/* string provided as a parameter.  It is called by the constructor when the  */
/* Exception is created.  Memory is allocated to store the error string, if   */
/* their is not enough memory, then the string is set to NULL but the actual  */
/* exception value is still set.                                              */
/******************************************************************************/
void
ThreadException::SetString(const char *pcErrString)
{
    try
    {
        pcExceptionString = new char[strlen(pcErrString) + 1];
        strcpy(pcExceptionString, pcErrString);
    }
    catch (...)
    {
        pcExceptionString = NULL;
    }
}

/******************************************************************************/
/* End of File: threadexception.cpp                                           */
/******************************************************************************/
