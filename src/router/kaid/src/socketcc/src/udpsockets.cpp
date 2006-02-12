/******************************************************************************/
/* File: udpsockets.cpp                                                       */
/******************************************************************************/
/* This source code file contains the source code for all the C++ classes     */
/* within the socketcc library that implement UDP Socket functionality.       */
/******************************************************************************/
/* Written By: Jason But                                                      */
/* Copyright:  CTIE - Monash University 2001-2003                             */
/*                                                                            */
/* Notes:                                                                     */
/*    Version 1.00 - Original Version of module.                              */
/*                                                                            */
/*    Version 1.11 - Default parameter values in implementation cause some    */
/*                   compilers to complain.                                   */
/*                                                                            */
/*    Version 1.30 - Enhanced functionality of send and receive methods in    */
/*                   all TCP and UDP socket classes to allow specification of */
/*                   socket flags.  A default value of zero allows backwards  */
/*                   compatibility.                                           */
/*                                                                            */
/*    Version 1.38 - Send datagram methods now accept const void pointers as  */
/*                   a parameter, this is more correct when defining what the */
/*                   method does.  Many thanks to Andrea Rui for pointing     */
/*                   this out.                                                */
/******************************************************************************/

/******************************************************************************/
/* Include class header file.                                                 */
/******************************************************************************/
#include "socketcc.h"

/******************************************************************************/
/* class UDPSocket.                                                           */
/******************************************************************************/
/* UDPSocket Constructor.                                                     */
/*                                                                            */
/* The default socket type is IPv4.  We call the base class constructor to    */
/* create a DataGram (UDP) Socket using either IPv4 or IPv6 as specified.     */
/******************************************************************************/
UDPSocket::UDPSocket(bool bUseIPv6) : SocketBase(bUseIPv6, SOCK_DGRAM)
{
}

/******************************************************************************/
/* void SendDatagram(void const *pBuffer, int iBufLength,                     */
/*                   IPAddress &cDestAddr, int iDestPort,                     */
/*                   unsigned int uiFlags)                                    */
/*                                                                            */
/* This method is called to send a UDP datagram to the specified IP Address   */
/* and port number.  The first two parameters signify the UDP payload and its */
/* size, the second two signify the destination.  We call the base class      */
/* SendTo() method to send the datagram, returning the actual number of bytes */
/* sent.  Provision is made to allow specification of flag values to pass to  */
/* the SendTo() method, the default value of uiFlags is 0.                    */
/******************************************************************************/
int
UDPSocket::SendDatagram(const void *pBuffer, int iBufLength, IPAddress &cDestAddr,
                        int iDestPort, unsigned int uiFlags)
{
    return SendTo(pBuffer, iBufLength, uiFlags, cDestAddr, iDestPort);
}

/******************************************************************************/
/* void ReceiveDatagram(void *pBuffer, int iBufLength, IPAddress &cSourceAddr,*/
/*                      int &iSourcePort, unsigned int uiFlags)               */
/*                                                                            */
/* This method is called to receive a UDP datagram on the socket, recording   */
/* its source IP Address and port number.  The first two parameters signify   */
/* the payload receiving buffer and its size, the second two are used to      */
/* record the datagram source.  We call the base class RecvFrom() method to   */
/* receive the datagram, returning the actual size of the datagram. Provision */
/* is made to allow specification of flag values to pass to the RecvFrom()    */
/* method, the default value of uiFlags is 0.                                 */
/******************************************************************************/
int
UDPSocket::ReceiveDatagram(void *pBuffer, int iBufLength, IPAddress &cSourceAddr,
                           int &iSourcePort, unsigned int uiFlags)
{
    return RecvFrom(pBuffer, iBufLength, uiFlags, cSourceAddr, iSourcePort);
}

/******************************************************************************/
/* void GetSockOpt(int iCodeLevel, int iOptionName, void *pOptionData,        */
/*                 int &iDataLength)                                          */
/*                                                                            */
/* This method is a wrapper method for the protected base class GetSockOpt()  */
/* method.                                                                    */
/******************************************************************************/
void
UDPSocket::GetSockOpt(int iCodeLevel, int iOptionName, void *pOptionData, int &iDataLength)
{
    SocketBase::GetSockOpt(iCodeLevel, iOptionName, pOptionData, iDataLength);
}

/******************************************************************************/
/* void SetSockOpt(int iCodeLevel, int iOptionName, const void *pOptionData,  */
/*                 int iDataLength)                                           */
/*                                                                            */
/* This method is a wrapper method for the protected base class SetSockOpt()  */
/* method.                                                                    */
/******************************************************************************/
void
UDPSocket::SetSockOpt(int iCodeLevel, int iOptionName, const void *pOptionData, int iDataLength)
{
    SocketBase::SetSockOpt(iCodeLevel, iOptionName, pOptionData, iDataLength);
}

/******************************************************************************/
/* int FCntl(int iCommand, long lArgument)                                    */
/*                                                                            */
/* This method is a wrapper method for the protected base class FCntl() method*/
/******************************************************************************/
int
UDPSocket::FCntl(int iCommand, long lArgument)
{
    return SocketBase::FCntl(iCommand, lArgument);
}

/******************************************************************************/
/* class UDPServerSocket.                                                     */
/******************************************************************************/
/* UDPServerSocket Constructor.                                               */
/*                                                                            */
/* The default local port number is automatically allocated.  The socket is   */
/* bound to the specified IP Address on the host.  We call the base class     */
/* constructor to UDP Socket using either IPv4 or IPv6 as extracted from the  */
/* supplied cLocalAddr.  We then try to bind the UDP Socket to the specified  */
/* IP Address and port number using the base class Bind() method.  Finally    */
/* we call the base class GetSockName() method to obtain the actual IP        */
/* Address and port number allocated to the socket.  These methods throw an   */
/* appropriate exception if they fail.                                        */
/******************************************************************************/
UDPServerSocket::UDPServerSocket(IPAddress &cLocalAddr, int iLocalPortNo)
        : UDPSocket((cLocalAddr.GetAddressFamily() == AF_INET6))
{
    Bind(cLocalAddr, iLocalPortNo);
    GetSockName(cLocalAddress, iLocalPort);
}

/******************************************************************************/
/* UDPServerSocket Constructor.                                               */
/*                                                                            */
/* The default local port number is automatically allocated and the default   */
/* socket type is IPv4.  We call the base class constructor to UDP Socket     */
/* using either IPv4 or IPv6 as specified.  We then try to bind the Socket to */
/* the specified port number and any local IP Address using the base class    */
/* Bind() method.  Finally we call the base class GetSockName() method to     */
/* obtain the actual IP Address and port number allocated to the socket.      */
/* These methods throw an appropriate exception if they fail.                 */
/******************************************************************************/
UDPServerSocket::UDPServerSocket(int iLocalPortNo, bool bUseIPv6)
        : UDPSocket(bUseIPv6)
{
    Bind(iLocalPortNo);
    GetSockName(cLocalAddress, iLocalPort);
}

/******************************************************************************/
/* class UDPConnectedSocket.                                                  */
/******************************************************************************/
/* UDPConnectedSocket Constructor.                                            */
/*                                                                            */
/* The default socket type is IPv4.  We call the base class constructor to    */
/* create a DataGram (UDP) Socket using either IPv4 or IPv6 as specified, the */
/* socket is currently unconnected so the bIsConnected flag is set to false.  */
/******************************************************************************/
UDPConnectedSocket::UDPConnectedSocket(bool bUseIPv6)
        : SocketBase(bUseIPv6, SOCK_DGRAM), bIsConnected(false)
{
}

/******************************************************************************/
/* UDPConnectedSocket Constructor.                                            */
/*                                                                            */
/* The default socket type is IPv4.  We call the base class constructor to    */
/* create a DataGram (UDP) Socket using either IPv4 or IPv6 as specified.  We */
/* then call the Connect() method to connect to the specified IP address and  */
/* port number.  The result of this call will set the bIsConnected flag.  If  */
/* an error occurs, the Connect() call will throw an appropriate exception.   */
/******************************************************************************/
UDPConnectedSocket::UDPConnectedSocket(IPAddress &cServAddr, int iServPortNo, bool bUseIPv6)
        : SocketBase(bUseIPv6, SOCK_DGRAM)
{
    Connect(cServAddr, iServPortNo);
}

/******************************************************************************/
/* void Connect(IPAddress &cServAddr, int iServPortNo)                        */
/*                                                                            */
/* This method is called to connect the socket to a server UDP socket that is */
/* specified by the provided IP Address and port number.  We call the base    */
/* class Connect() method to perform the connection, after connecting, we set */
/* the bIsConnected flag.                                                     */
/******************************************************************************/
void
UDPConnectedSocket::Connect(IPAddress &cServAddr, int iServPortNo)
{
    SocketBase::Connect(cServAddr, iServPortNo);
    cServerAddress = cServAddr;
    iServerPort = iServPortNo;

    GetSockName(cLocalAddress, iLocalPort);
    bIsConnected = true;
}

/******************************************************************************/
/* void DisConnect()                                                          */
/*                                                                            */
/* This method is called to disconnect the UDP socket from the server.  We do */
/* this by resetting the bIsConnected flag.                                   */
/******************************************************************************/
void
UDPConnectedSocket::DisConnect()
{
    bIsConnected = false;
}

/******************************************************************************/
/* void SendDatagram(void const *pBuffer, int iBufLength,                     */
/*                   unsigned int uiFlags)                                    */
/*                                                                            */
/* This method is called to send a UDP datagram to the connected UDP Server   */
/* Socket.  The parameters signify the UDP Payload and its size.  If the      */
/* socket is not connected, then we throw a NotConnected exception, otherwise */
/* we call the base class Send() method to send the datagram, returning the   */
/* actual number of bytes sent.  Provision is made to allow specification of  */
/* flag values to pass to the Send() method, the default value of uiFlags is 0*/
/******************************************************************************/
int
UDPConnectedSocket::SendDatagram(void const *pBuffer, int iBufLength, unsigned int uiFlags)
{
    if (!bIsConnected)
    {
        throw SocketException(SocketException::errNotConnected);
    }
    return Send(pBuffer, iBufLength, uiFlags);
}

/******************************************************************************/
/* void ReceiveDatagram(void *pBuffer, int iBufLength, unsigned int uiFlags)  */
/*                                                                            */
/* This method is called to receive a UDP datagram on the connected socket.   */
/* The parameters signify the payload receiving buffer and its size.  If the  */
/* socket is not connected, then we throw a NotConnected exception, otherwise */
/* we call the base class Recv() method to receive the datagram, returning    */
/* the actual size of the datagram.  Provision is made to allow specification */
/* of flag values to pass to the Recv() method, the default value of uiFlags  */
/* is 0.                                                                      */
/******************************************************************************/
int
UDPConnectedSocket::ReceiveDatagram(void *pBuffer, int iBufLength, unsigned int uiFlags)
{
    if (!bIsConnected)
    {
        throw SocketException(SocketException::errNotConnected);
    }
    return Recv(pBuffer, iBufLength, uiFlags);
}

/******************************************************************************/
/* void GetSockOpt(int iCodeLevel, int iOptionName, void *pOptionData,        */
/*                 int &iDataLength)                                          */
/*                                                                            */
/* This method is a wrapper method for the protected base class GetSockOpt()  */
/* method.                                                                    */
/******************************************************************************/
void
UDPConnectedSocket::GetSockOpt(int iCodeLevel, int iOptionName, void *pOptionData, int &iDataLength)
{
    SocketBase::GetSockOpt(iCodeLevel, iOptionName, pOptionData, iDataLength);
}

/******************************************************************************/
/* void SetSockOpt(int iCodeLevel, int iOptionName, const void *pOptionData,  */
/*                 int iDataLength)                                           */
/*                                                                            */
/* This method is a wrapper method for the protected base class SetSockOpt()  */
/* method.                                                                    */
/******************************************************************************/
void
UDPConnectedSocket::SetSockOpt(int iCodeLevel, int iOptionName, const void *pOptionData, int iDataLength)
{
    SocketBase::SetSockOpt(iCodeLevel, iOptionName, pOptionData, iDataLength);
}

/******************************************************************************/
/* int FCntl(int iCommand, long lArgument)                                    */
/*                                                                            */
/* This method is a wrapper method for the protected base class FCntl() method*/
/******************************************************************************/
int
UDPConnectedSocket::FCntl(int iCommand, long lArgument)
{
    return SocketBase::FCntl(iCommand, lArgument);
}

/******************************************************************************/
/* End of File: udpsockets.cpp                                                */
/******************************************************************************/
