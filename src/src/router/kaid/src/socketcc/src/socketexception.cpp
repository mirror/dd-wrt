/******************************************************************************/
/* File: socketexception.cpp                                                  */
/******************************************************************************/
/* This source code file is the source file for the SocketException class -   */
/* this class encapsulates any exceptions thrown in the socketcc C++ library. */
/******************************************************************************/
/* Written By: Jason But                                                      */
/* Copyright:  CTIE - Monash University 2001                                  */
/*                                                                            */
/* Notes:                                                                     */
/*    Version 1.00 - Original Version of module.                              */
/*                                                                            */
/*    Version 1.10 - Added new exception type to signify error when mapping   */
/*                   from an IPv6 address to an IPv4 address.                 */
/*                                                                            */
/*    Version 1.11 - Default paramter values in implementation cause some     */
/*                   compilers to complain.                                   */
/*                                                                            */
/*    Version 1.20 - SocketException constructor now takes optional value of  */
/*                   an integer error number.  If the error code is of type   */
/*                   errUnknown, this error code is included in the string    */
/*                   representation of the exception.                         */
/*                 - New exception type - errHostUnreachable - added.         */
/*                                                                            */
/*    Version 1.30 - Error number for errUnknown exceptions printed correcly. */
/******************************************************************************/

/******************************************************************************/
/* Standard C Includes.                                                       */
/******************************************************************************/
#include <stdio.h>
#include <string.h>

/******************************************************************************/
/* Include class header file.                                                 */
/******************************************************************************/
#include "socketcc.h"

/******************************************************************************/
/* Class SocketException                                                      */
/******************************************************************************/
/* SocketException Constructor.                                               */
/*                                                                            */
/* The Exception type is set to the type specified during construction.  We   */
/* then execute a switch statement on the exception type to set the correct   */
/* error string in pcExceptionString.  If ecType is errUnknown, then the      */
/* value of iErrCode is included in pcExceptionString (for further debug      */
/* information).  This is not true if iErrCode is 0.                          */
/******************************************************************************/
SocketException::SocketException(ErrorCodes ecType, int iErrNo) : ecException(ecType)
{
    switch (ecException)
    {
        case errBadHostName:          SetString("Specified Host Name is Invalid\n");
                                      break;
        case errNoIPAddress:          SetString("No IP Address for this host\n");
                                      break;
        case errDNSError:             SetString("DNS Error - Try again later\n");
                                      break;
        case errNoProtocolSupport:    SetString("Protocol Type not Supported\n");
                                      break;
        case errKernelMemory:         SetString("Out of Kernel Memory\n");
                                      break;
        case errCannotConvertToIPv4:  SetString("IP Address is not an IPv4-mapped IPv6 address, cannot convert it to an IPv4 address\n");
                                      break;
        case errNoDescriptors:        SetString("No Descriptors left for Socket\n");
                                      break;
        case errPermissionDenied:     SetString("Permission to create Socket denied\n");
                                      break;
        case errMemory:               SetString("Out of Memory Error\n");
                                      break;
        case errInvalidProtocol:      SetString("Protocol unknown or not available\n");
                                      break;
        case errBadDescriptor:        SetString("Bad Socket Descriptor\n");
                                      break;
        case errIllegalPointer:       SetString("Pointer is outside the user's address space\n");
                                      break;
        case errAlreadyConnected:     SetString("The socket is already connected\n");
                                      break;
        case errConnectRefused:       SetString("Server refused the Connection\n");
                                      break;
        case errConnectTimeOut:       SetString("Timeout while attempting connection\n");
                                      break;
        case errNetUnreachable:       SetString("The Network is unreachable\n");
                                      break;
        case errHostUnreachable:      SetString("The specified IP Host is unreachable\n");
                                      break;
        case errAddrInUse:            SetString("The provided Address is already in use\n");
                                      break;
        case errInProgress:           SetString("Socket is Non-Blocking, operation currently in progress\n");
                                      break;
        case errAlreadyConnecting:    SetString("Socket is Non-Blocking, previous connect still in progress\n");
                                      break;
        case errIncorrectAddrFamily:  SetString("Incorrect Address Family\n");
                                      break;
        case errBrdCastNotEnabled:    SetString("Broadcast flag was not enabled\n");
                                      break;
        case errAlreadyBound:         SetString("The socket is already bound to an address\n");
                                      break;
        case errAddressProtected:     SetString("Permission to Access the specified Port Number is denied\n");
                                      break;
        case errCantListen:           SetString("Listen Operation not Supported on this Socket\n");
                                      break;
        case errNotStreamSock:        SetString("Cannot Accept Connection - not a TCP Socket\n");
                                      break;
        case errNoPendingConnections: SetString("Socket is Non-Blocking, no pending connections\n");
                                      break;
        case errFirewall:             SetString("Firewall rules forbid connection\n");
                                      break;
        case errNotConnected:         SetString("Illegal operation on an unconnected Socket\n");
                                      break;
        case errWouldBlock:           SetString("Socket is Non-Blocking, requested operation would block\n");
                                      break;
        case errInterrupted:          SetString("The send/receive was interrupted by a signal\n");
                                      break;
        case errInvalidArgument:      SetString("An invalid argument was presented\n");
                                      break;
        case errMessageSizeTooBig:    SetString("Message too Big for atomic transmission\n");
                                      break;
        case errNotBound:             SetString("Local Socket is not bound to an address/port number\n");
                                      break;
        case errOptionNotSupported:   SetString("This options is unsupported at the specified Code Level\n");
                                      break;
        case errUnknown:              if (iErrNo)
                                      {
                                          char      pcTemp[50];
                                          sprintf(pcTemp, "An unknown error occured (errno = %#.8x)\n", iErrNo);
                                          SetString(pcTemp);
                                      } else
                                      {
                                          SetString("An unknown error occured\n");
                                      }
    }
}

/******************************************************************************/
/* SocketException Copy COnstructor.                                          */
/*                                                                            */
/* This constructor is called when making a copy of an existing class         */
/* instance.  We copy the exception cose and set the string parameter to the  */
/* copy of the string in the existing instance.                               */
/******************************************************************************/
SocketException::SocketException(const SocketException &cOriginal)
{
    ecException = cOriginal.ecException;
    SetString(cOriginal.pcExceptionString);
}

/******************************************************************************/
/* SocketException Destructor.                                                */
/*                                                                            */
/* Free any memory allocated to the error string.                             */
/******************************************************************************/
SocketException::~SocketException()
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
SocketException::SetString(const char *pcErrString)
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
/* End of File: socketexception.cpp                                           */
/******************************************************************************/
