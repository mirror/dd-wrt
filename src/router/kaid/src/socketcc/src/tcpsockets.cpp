/******************************************************************************/
/* File: tcpsockets.cpp                                                       */
/******************************************************************************/
/* This source code file contains the source code for all the C++ classes     */
/* within the socketcc library that implement TCP Socket functionality.       */
/******************************************************************************/
/* Written By: Jason But                                                      */
/* Copyright:  CTIE - Monash University 2001-2003                             */
/*                                                                            */
/* Contributions:                                                             */
/* Andreas Almroth - Support for Solaris OS.                                  */
/* Andrea Rui       - Inspiration for cleaner implementation to accept a      */
/*                    connection on a TCPServersocket.                        */
/*                                                                            */
/*                                                                            */
/* Notes:                                                                     */
/*    Version 1.00 - Original Version of module.                              */
/*                                                                            */
/*    Version 1.10 - TCPClientSocket must now be created in the connected     */
/*                   state, no longer need to specify whether to use IPv6 or  */
/*                   IPv4 as this is retrieved from the IP Address provided.  */
/*                   No unconnected client sockets available, makes sense     */
/*                   since you cannot disconnect the socket anyway.           */
/*                 - TCPServerSocket constructor no longer requires the IPv6  */
/*                   flag when binding to a specific IP address, but is still */
/*                   required when binding to a wildcard address.             */
/*                                                                            */
/*    Version 1.11 - Support for Solaris, many thanks to Andreas Almroth for  */
/*                   his effort, fixed hiding of local variable bug.          */
/*                 - Default paramter values in implementation cause some     */
/*                   compilers to complain.                                   */
/*                                                                            */
/*    Version 1.20 - Fixed bug with a non-blocking TCPServerSocket, each no   */
/*                   pending connections situation was not releasing socket   */
/*                   descriptors and resources allocated in the AcceptClient()*/
/*                   method.                                                  */
/*                 - Added new methods to TCPSocket to send and receive       */
/*                   different types of data (uint16_t, uint32_t, C-style     */
/*                   strings).                                                */
/*                                                                            */
/*    Version 1.30 - Enhanced functionality of TCPSocket::SendASCII() and     */
/*                   TCPSocket::RecvASCII() to allow specification of the     */
/*                   string terminating character.  For backwards             */
/*                   compatibility, this has a default value of '\0'.         */
/*                 - Enhanced functionality of send and receive methods in    */
/*                   all TCP and UDP socket classes to allow specification of */
/*                   socket flags.  A default value of zero allows backwards  */
/*                   compatibility.                                           */
/*                                                                            */
/*    Version 1.38 - All Send Data type methods now accept const values as a  */
/*                   parameter, this is more correct when defining what the   */
/*                   method does.  Many thanks to Andrea Rui for pointing     */
/*                   this out.                                                */
/*                 - Created a new constructor for the TCPSocket Class, this  */
/*                   constructor allows creation of a TCPSocket instance by   */
/*                   providing a socket descriptor.  This method is primarily */
/*                   for convenience only as the provided socket descriptor   */
/*                   is not checked to ensure it is a valid descriptor, nor   */
/*                   checked to ensure it refers to an actual TCP socket.  A  */
/*                   similar constructor is not provided for TCPServerSocket  */
/*                   as creating a server socket also means specifying an     */
/*                   address and port number to bind to, this makes no sense  */
/*                   when providing an existing socket descriptor.  A similar */
/*                   argument is provided when constructing an instance of    */
/*                   TCPClientSocket and therefore no similar constructor has */
/*                   been created for this class either.                      */
/*                 - Modified the AcceptClient() method on TCPServerSocket to */
/*                   utilise the new implementation of the base class Accept()*/
/*                   method.  Many thanks to Andrea Rui for reminding me that */
/*                   the original implementation had wastefull allocation of  */
/*                   socket resources and making me think of a better         */
/*                   solution.                                                */
/******************************************************************************/

/******************************************************************************/
/* Include class header file.                                                 */
/******************************************************************************/
#include "socketcc.h"

/******************************************************************************/
/* class TCPSocket.                                                           */
/******************************************************************************/
/* TCPSocket Constructor.                                                     */
/*                                                                            */
/* The default socket type is IPv4.  We call the base class constructor to    */
/* create a Streaming (TCP) Socket using either IPv4 or IPv6 as specified,    */
/* the bLocalSet flag and bIsConnected flag are initialised to false.         */
/******************************************************************************/
TCPSocket::TCPSocket(bool bUseIPv6)
        : SocketBase(bUseIPv6), bLocalSet(false), bIsConnected(false)
{
}

/******************************************************************************/
/* TCPSocket Constructor.                                                     */
/*                                                                            */
/* Create a TCP Socket instance given a socket descriptor.  We call the       */
/* matching base class constructor to create using a socket descriptor.  We   */
/* then proceed to obtain and set the local address fields via a call to the  */
/* SetLocal() method.  We assume the socket is not yet connected and so do    */
/* not directly call SetConnected().                                          */
/******************************************************************************/
TCPSocket::TCPSocket(int iNewSockDesc)
        : SocketBase(iNewSockDesc), bIsConnected(false)
{
    SetLocal();
}

/******************************************************************************/
/* void SetLocal()                                                            */
/*                                                                            */
/* This protected method is called by inherited classes when the local socket */
/* address is valid.  It sets the bLocalSet flag, obtains details of the      */
/* local address and stores them in the internal member variables.            */
/******************************************************************************/
void
TCPSocket::SetLocal()
{
    GetSockName(cLocalAddress, iLocalPort);
    bLocalSet = true;
}

/******************************************************************************/
/* void SetConnected()                                                        */
/*                                                                            */
/* This protected method is called by inherited classes when the socket has   */
/* been connected.  It sets the bIsConnected flag, obtains details of the     */
/* remote connection and stores them in the internal member variables.        */
/******************************************************************************/
void
TCPSocket::SetConnected()
{
    GetPeerName(cRemoteAddress, iRemotePort);
    bIsConnected = true;
}

/******************************************************************************/
/* int SendData(const void *pData, int iDataLen, unsigned int uiFlags)        */
/*                                                                            */
/* This method is called to send a block of data to the remote connection.    */
/* The parameters signify the Data Payload and its size.  If the socket is    */
/* not connected, then we throw a NotConnected exception, otherwise we call   */
/* the base class Send() method to send the data, returning the number of     */
/* bytes actually sent.  There is also provision for specifying flags to pass */
/* to the Send() call, these flags have a default value of 0.                 */
/******************************************************************************/
int
TCPSocket::SendData(const void *pData, int iDataLen, unsigned int uiFlags)
{
    if (!bIsConnected)
    {
        throw SocketException(SocketException::errNotConnected);
    }
    return Send(pData, iDataLen, uiFlags);
}

/******************************************************************************/
/* void SendBinary16Bits(const int iData, unsigned int uiFlags)               */
/*                                                                            */
/* This method is called to send a 16-bit binary value to the remote          */
/* connection.  We convert the parameter to network byte order and call the   */
/* SendData() method to send it.  If two bytes are not sent (the returned     */
/* value is not two), then we throw an unknown Socket Exception.  There is    */
/* also provision for specifying flags to pass to the SendData() call, these  */
/* flags have a default value of 0.                                           */
/******************************************************************************/
void
TCPSocket::SendBinary16Bits(const int iData, unsigned int uiFlags)
{
    uint16_t uiNetOrder = htons(iData);

    if (SendData(&uiNetOrder, sizeof(uint16_t), uiFlags) != sizeof(uint16_t))
    {
        throw SocketException(SocketException::errUnknown);
    }
}

/******************************************************************************/
/* void SendBinary32Bits(const long lData, unsigned int uiFlags)              */
/*                                                                            */
/* This method is called to send a 32-bit binary value to the remote          */
/* connection.  We convert the parameter to network byte order and call the   */
/* SendData() method to send it.  If four bytes are not sent (the returned    */
/* value is not four), then we throw an unknown Socket Exception.  There is   */
/* also provision for specifying flags to pass to the SendData() call, these  */
/* flags have a default value of 0.                                           */
/******************************************************************************/
void
TCPSocket::SendBinary32Bits(const long lData, unsigned int uiFlags)
{
    uint32_t uiNetOrder = htonl(lData);

    if (SendData(&uiNetOrder, sizeof(uint32_t), uiFlags) != sizeof(uint32_t))
    {
        throw SocketException(SocketException::errUnknown);
    }
}

/******************************************************************************/
/* void SendASCII(const char *pcData, char cTerminator, unsigned int uiFlags) */
/*                                                                            */
/* This method is called to send a (cTerminator) terminated ASCII string to   */
/* the remote connection. The number of bytes we send has to be one more than */
/* the length of the string up to the first instance of cTerminator to ensure */
/* that we also send the terminating byte.  If the correct number of bytes    */
/* are not sent, then we throw an unknown Socket Exception.  We generate a    */
/* string out of the terminating character for use in strcspn() which we use  */
/* to determine the length of the string up to the first instance of the      */
/* selected terminating character.  The default value for cTerminator is '\0' */
/* indicating to send the entire ASCII string including the '\0' terminator   */
/* and therefore emulating the functionality of the older version of the      */
/* method.  There is also provision for specifying flags to pass to the       */
/* SendData() call, these flags have a default value of 0.                    */
/******************************************************************************/
void
TCPSocket::SendASCII(const char *pcData, char cTerminator, unsigned int uiFlags)
{
    char    pcTermSet[2] = { cTerminator, '\0' };
    int     iBytesToSend = strcspn(pcData, pcTermSet) + 1;

    if (SendData(pcData, iBytesToSend, uiFlags) != iBytesToSend)
    {
        throw SocketException(SocketException::errUnknown);
    }
}

/******************************************************************************/
/* void RecvData(void *pBuffer, int iBufferLen, unsigned int uiFlags)         */
/*                                                                            */
/* This method is called to receive a block of data on the connected socket.  */
/* The parameters signify the payload receiving buffer and its size.  If the  */
/* socket is not connected, then we throw a NotConnected exception, otherwise */
/* we call the base class Recv() method to receive the data, returning the    */
/* number of bytes actually written.  There is also provision for specifying  */
/* flags to pass to the Recv() call, these flags have a default value of 0.   */
/******************************************************************************/
int
TCPSocket::RecvData(void *pBuffer, int iBufferLen, unsigned int uiFlags)
{
    if (!bIsConnected)
    {
        throw SocketException(SocketException::errNotConnected);
    }
    return Recv(pBuffer, iBufferLen, uiFlags);
}

/******************************************************************************/
/* int RecvBinary16Bits(unsigned int uiFlags)                                 */
/*                                                                            */
/* This method is called to read a 16-bit binary value from the remote        */
/* connection.  We loop - calling RecvData() - until the required number of   */
/* bytes are read, if RecvData() returns a smaller number of bytes due to the */
/* remaining values not yet arriving, we go back into a loop.  If RecvData()  */
/* fails on a non-blocking socket, it will throw the correct exception type   */
/* errWouldBlock.  Once the value is read into uiNetOrder, we convert it to   */
/* host byte order and return the read value.  There is also provision for    */
/* specifying flags to pass to the RecvData() call, these flags have a        */
/* default value of 0.                                                        */
/******************************************************************************/
int
TCPSocket::RecvBinary16Bits(unsigned int uiFlags)
{
    uint16_t    uiNetOrder;
    int         iBytesLeft = sizeof(uint16_t);
    char        *pcEndReadBuffer = ((char *) &uiNetOrder) + iBytesLeft;

    while (iBytesLeft)
    {
        iBytesLeft-= RecvData((void *) (pcEndReadBuffer - iBytesLeft), iBytesLeft, uiFlags);
    }

    return (int) ntohs(uiNetOrder);
}

/******************************************************************************/
/* int RecvBinary32Bits(unsigned int uiFlags)                                 */
/*                                                                            */
/* This method is called to read a 32-bit binary value from the remote        */
/* connection.  We loop - calling RecvData() - until the required number of   */
/* bytes are read, if RecvData() returns a smaller number of bytes due to the */
/* remaining values not yet arriving, we go back into a loop.  If RecvData()  */
/* fails on a non-blocking socket, it will throw the correct exception type   */
/* errWouldBlock.  Once the value is read into uiNetOrder, we convert it to   */
/* host byte order and return the read value.  There is also provision for    */
/* specifying flags to pass to the RecvData() call, these flags have a        */
/* default value of 0.                                                        */
/******************************************************************************/
long
TCPSocket::RecvBinary32Bits(unsigned int uiFlags)
{
    uint32_t    uiNetOrder;
    int         iBytesLeft = sizeof(uint32_t);
    char        *pcEndReadBuffer = ((char *) &uiNetOrder) + iBytesLeft;

    while (iBytesLeft)
    {
        iBytesLeft-= RecvData((void *) (pcEndReadBuffer - iBytesLeft), iBytesLeft, uiFlags);
    }

    return (long) ntohl(uiNetOrder);
}

/******************************************************************************/
/* char * RecvASCII(char cTerminator, unsigned int uiFlags)                   */
/*                                                                            */
/* This method is called to read a cTerminator terminated ASCII string from   */
/* the remote connection.  We loop - calling RecvData() to read one byte at a */
/* time - until we read cTerminator.  With each byte read, it is appended to  */
/* a std::string instance.  Upon reading the terminator, the loop terminates, */
/* and a copy of the std::string instance is made and returned.  If the       */
/* selected terminator is not '\0', we also append the terminator to the      */
/* string.  The returned (char *) pointer must be freed by the caller.  The   */
/* cTerminator parameter has a default value of '\0' for backwards            */
/* compatibility.  There is also provision for specifying flags to pass to    */
/* the RecvData() call, these flags have a default value of 0.                */
/******************************************************************************/
char *
TCPSocket::RecvASCII(char cTerminator, unsigned int uiFlags)
{
    char            cReadChar;
    std::string     strReadValue = "";

    for (RecvData((void *)&cReadChar, 1); cReadChar != cTerminator; RecvData((void *)&cReadChar, 1, uiFlags))
    {
        strReadValue+= cReadChar;
    }
    if (cTerminator) strReadValue+= cTerminator;
    return strdup(strReadValue.c_str());
}

/******************************************************************************/
/* IPAddress & LocalIPAddress()                                               */
/*                                                                            */
/* This method returns the local IP Address of the socket connection.  If the */
/* address has not been set (socket not bound or connection not accepted), we */
/* throw a NotBound exception, otherwise we return a copy of cLocalAddress.   */
/******************************************************************************/
IPAddress &
TCPSocket::LocalIPAddress()
{
    if (!bLocalSet)
    {
        throw SocketException(SocketException::errNotBound);
    }
    return cLocalAddress;
}

/******************************************************************************/
/* int LocalPortNumber()                                                      */
/*                                                                            */
/* This method returns the local port number of the socket connection.  If    */
/* the port number has not been set (socket not bound or connection not       */
/* accepted), we throw a NotBound exception, otherwise we return iLocalPort.  */
/******************************************************************************/
int
TCPSocket::LocalPortNumber()
{
    if (!bLocalSet)
    {
        throw SocketException(SocketException::errNotBound);
    }
    return iLocalPort;
}

/******************************************************************************/
/* IPAddress & RemoteIPAddress()                                              */
/*                                                                            */
/* This method returns the remote IP Address of the socket connection.  If    */
/* the socket is not connected, there is no remote IP Address and we throw a  */
/* NotBound exception, otherwise we return a copy of cRemoteAddress.          */
/******************************************************************************/
IPAddress &
TCPSocket::RemoteIPAddress()
{
    if (!bIsConnected)
    {
        throw SocketException(SocketException::errNotConnected);
    }
    return cRemoteAddress;
}

/******************************************************************************/
/* int RemotePortNumber()                                                     */
/*                                                                            */
/* This method returns the remote port number of the socket connection.  If   */
/* the socket is not connected, there is no remote port number and we throw a */
/* NotBound exception, otherwise we return iRemotePort.                       */
/******************************************************************************/
int
TCPSocket::RemotePortNumber()
{
    if (!bIsConnected)
    {
        throw SocketException(SocketException::errNotConnected);
    }
    return iRemotePort;
}

/******************************************************************************/
/* void GetSockOpt(int iCodeLevel, int iOptionName, void *pOptionData,        */
/*                 int &iDataLength)                                          */
/*                                                                            */
/* This method is a wrapper method for the protected base class GetSockOpt()  */
/* method.                                                                    */
/******************************************************************************/
void
TCPSocket::GetSockOpt(int iCodeLevel, int iOptionName, void *pOptionData, int &iDataLength)
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
TCPSocket::SetSockOpt(int iCodeLevel, int iOptionName, const void *pOptionData, int iDataLength)
{
    SocketBase::SetSockOpt(iCodeLevel, iOptionName, pOptionData, iDataLength);
}

/******************************************************************************/
/* int FCntl(int iCommand, long lArgument)                                    */
/*                                                                            */
/* This method is a wrapper method for the protected base class FCntl() method*/
/******************************************************************************/
int
TCPSocket::FCntl(int iCommand, long lArgument)
{
    return SocketBase::FCntl(iCommand, lArgument);
}

/******************************************************************************/
/* class TCPClientSocket.                                                     */
/******************************************************************************/
/* TCPClientSocket Constructor.                                               */
/*                                                                            */
/* We call the base class constructor to create a Streaming (TCP) Socket of   */
/* type IPv4 or IPv6 as specified by the address family of the servers IP     */
/* Address.  We then call the Connect() method to connect to the specified IP */
/* address and port number.  After connecting, we call the protected methods  */
/* SetLocal() and SetConnected() to obtain the local and remote address       */
/* details of the connection as well as set flags to enable later retrieval   */
/* of these values and transmission of data.  If any error occurs, a suitable */
/* exception will be thrown.                                                  */
/******************************************************************************/
TCPClientSocket::TCPClientSocket(IPAddress &cServAddr, int iServPort)
        : TCPSocket((cServAddr.GetAddressFamily() == AF_INET6))
{
    Connect(cServAddr, iServPort);

    SetLocal();
    SetConnected();
}

/******************************************************************************/
/* class TCPServerSocket.                                                     */
/******************************************************************************/
/* TCPServerSocket Constructor.                                               */
/*                                                                            */
/* The default local port number is automatically allocated and the default   */
/* backlogged queue length is 1.  The socket is bound to the specified IP     */
/* IP Address on the host - the address is queried to determine whether to    */
/* create an IPv4 or IPv6 socket - and we set the socket to begin listening.  */
/* We call the base class constructor to TCPSocket using either IPv4 or IPv6  */
/* as retrieved from the bound IP Address.  We then try to bind the TCP       */
/* Socket to the specified IP Address and port number using the base class    */
/* Bind() method.  Following this, we call the protected method SetLocal() to */
/* obtain the actual IP Address and port number allocated to the socket and   */
/* the base class Listen() method to cause the socket to begin listening for  */
/* connections.  If any of these calls they will throw a suitable exception.  */
/******************************************************************************/
TCPServerSocket::TCPServerSocket(IPAddress &cLocalAddr, int iPortNo, int iBackLog)
        : TCPSocket((cLocalAddr.GetAddressFamily() == AF_INET6))
{
    const int   iReUseAddrFlag = 1;

    SetSockOpt(SOL_SOCKET, SO_REUSEADDR, &iReUseAddrFlag, sizeof(iReUseAddrFlag));
    Bind(cLocalAddr, iPortNo);
    SetLocal();
    Listen(iBackLog);
}

/******************************************************************************/
/* TCPServerSocket Constructor.                                               */
/*                                                                            */
/* The default local port number is automatically allocated, the default back */
/* logged queue length is 1 and the default socket type is IPv4.  We call the */
/* base class constructor to SocketBase using either IPv4 or IPv6 as          */
/* specified, a streaming socket (TCP) is default.  We then try to bind the   */
/* Socket to the specified port number and any local IP Address using the     */
/* base class Bind() method.  Following this, we call the base class          */
/* GetSockName() method to obtain the actual IP Address and port number       */
/* allocated to the socket and the base class Listen() method to cause the    */
/* socket to begin listening for new connections.  If any of these calls fail,*/
/* they will throw a suitable exception.                                      */
/******************************************************************************/
TCPServerSocket::TCPServerSocket(int iPortNo, int iBackLog, bool bUseIPv6)
        : TCPSocket(bUseIPv6)
{
    const int   iReUseAddrFlag = 1;

    SetSockOpt(SOL_SOCKET, SO_REUSEADDR, &iReUseAddrFlag, sizeof(iReUseAddrFlag));
    Bind(iPortNo);
    SetLocal();
    Listen(iBackLog);
}

/******************************************************************************/
/* TCPSocket *AcceptClient()                                                  */
/*                                                                            */
/* This method is called to accept a new connection on the server socket.     */
/* Further communications on the newly connected socket are made via the      */
/* newly created TCPSocket instance of which a pointer is returned when the   */
/* connection is accepted.  We call the templated Accept() method to accept   */
/* the pending connection and create a new instance of TCPSocket with the     */
/* socket descriptor of the new socket.  We then call the TCPSocket method    */
/* SetConnected() to let the newly created socket learn the remote addresses  */
/* of its connection.  Finally we return this newly created socket.  If there */
/* is an exception during accepting the connection, the base class method     */
/* Accept() will throw a suitable exception.                                  */
/******************************************************************************/
TCPSocket *
TCPServerSocket::AcceptClient()
{
    IPAddress   cClientAddress;
    int         iClientPort;
    TCPSocket   *pcNewSocket;

    pcNewSocket = Accept<TCPSocket>(cClientAddress, iClientPort);
    pcNewSocket->SetConnected();
    return pcNewSocket;
}

/******************************************************************************/
/* End of File: tcpsockets.cpp                                                */
/******************************************************************************/
