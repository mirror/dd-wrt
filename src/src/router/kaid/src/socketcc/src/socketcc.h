/******************************************************************************/
/* File: socketcc.h                                                           */
/******************************************************************************/
/* This source code file is a header file for a series of C++ classes that    */
/* encapsulate the socket libraries to provide network functionality.  The    */
/* C++ classes wrap the C function calls to provide the same functionality to */
/* object oriented programmers.                                               */
/******************************************************************************/
/* Written By: Jason But                                                      */
/* Copyright:  CTIE - Monash University 2001-2003                             */
/*                                                                            */
/* Contributions:                                                             */
/* Andreas Almroth  - Support for Solaris OS.                                 */
/* Desmond Schmidt  - Support for MacOS X 1.                                  */
/* Daniel Grimm     - Testing for MacOS X Jaguar.                             */
/* Clayborne Taylor - Support for FreeBSD.                                    */
/* Andrea Rui       - Inspiration for cleaner implementation to accept a      */
/*                    connection on a listening socket.                       */
/*                                                                            */
/* Notes:                                                                     */
/*    Version 1.00 - Original Version of module.                              */
/*                                                                            */
/*    Version 1.10 - pthreadcc Class Library now used for thread-safe         */
/*                   implementation.                                          */
/*                 - New exception type added to SocketException to signify   */
/*                   a failure in converting an IPv6 address to IPv4.         */
/*                 - Lazy evaluation of pcHostName via reverse DNS lookup and */
/*                   pcStrAddress.  Evaluation performed only if the string   */
/*                   values are requested - major performance increase.       */
/*                 - Added new member method to convert an IPAddress instance */
/*                   into either an IPv4 or IPv6 address.                     */
/*                 - TCPClientSocket must now be created in the connected     */
/*                   state, no longer need to specify whether to use IPv6 or  */
/*                   IPv4 as this is retrieved from the IP Address provided.  */
/*                   No unconnected client sockets available, makes sense     */
/*                   since you cannot disconnect the socket anyway.           */
/*                 - TCPServerSocket constructor no longer requires the IPv6  */
/*                   flag when binding to a specific IP address, but is still */
/*                   required when binding to a wildcard address.             */
/*                                                                            */
/*    Version 1.20 - SocketException constructor now takes optional value of  */
/*                   an integer error number.  If the error code is of type   */
/*                   errUnknown, this error code is included in the string    */
/*                   representation of the exception.                         */
/*                 - New exception type - errHostUnreachable - added.         */
/*                 - UDPServerSocket constructor no longer requires the IPv6  */
/*                   flag when binding to a specific IP address, but is still */
/*                   required when binding to a wildcard address.             */
/*                 - Added new methods to TCPSocket to send and receive       */
/*                   different types of data (uint16_t, uint32_t, C-style     */
/*                   strings).                                                */
/*                 - pcHostName replaced with strHostName of type std::string */
/*                   This should fix a range of minor memory leakage problems */
/*                   as well as simplify the code somewhat.                   */
/*                                                                            */
/*    Version 1.30 - Support for MacOS X, many thanks to Desmond Schmidt and  */
/*                   Daniel Grimm.                                            */
/*                 - Header file <sys/socket.h> now included in socketcc.h    */
/*                   as SOCK_STREAM is defined here on the Mac Platform.      */
/*                 - The <string> header file must be included as extern "C++"*/
/*                   otherwise the MacOS X g++ compiler complains when using  */
/*                   SocketCC in your own applications.                       */
/*                 - Enhanced functionality of TCPSocket::SendASCII() and     */
/*                   TCPSocket::RecvASCII() to allow specification of the     */
/*                   string terminating character.  For backwards             */
/*                   compatibility, this has a default value of '\0'.         */
/*                 - Added new method to IPAddress to return an unmapped      */
/*                   (not IPv6 Mapped IPv4) string representation of the IP   */
/*                   Address.                                                 */
/*                 - Enhanced functionality of send and receive methods in    */
/*                   all TCP and UDP socket classes to allow specification of */
/*                   socket flags.  A default value of zero allows backwards  */
/*                   compatibility.                                           */
/*                                                                            */
/*    Version 1.38 - Send methods for all socket types now accept const void  */
/*                   pointers as a parameter, this is more correct when       */
/*                   defining what the method does.  Many thanks to Andrea    */
/*                   for pointing this out.                                   */
/*                 - Created a new constructor for the SocketBase Class, this */
/*                   constructor allows creation of a SocketBase instance by  */
/*                   providing a socket descriptor.  This method is primarily */
/*                   for convenience as well as a better implementation of    */
/*                   accepting connections on TCP Sockets.  Please note that  */
/*                   the provided socket descriptor is not checked to ensure  */
/*                   it is a valid descriptor.                                */
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
/*                 - Created a new constructor for the UDPSocket Class, this  */
/*                   constructor allows creation of a UDPSocket instance by   */
/*                   providing a socket descriptor.  This method is primarily */
/*                   for convenience only as the provided socket descriptor   */
/*                   is not checked to ensure it is a valid descriptor, nor   */
/*                   checked to ensure it refers to an actual UDP socket.  A  */
/*                   similar constructor is not provided for UDPServerSocket  */
/*                   as creating a server socket also means specifying an     */
/*                   address and port number to bind to, this makes no sense  */
/*                   when providing an existing socket descriptor.  A similar */
/*                   argument is provided when constructing an instance of    */
/*                   UDPConnectedSocket and therefore no similar constructor  */
/*                   has been created for this class either.                  */
/*                 - The interface for the SocketBase::Accept() method has    */
/*                   been changed.  Hopefully this will have minimal impact   */
/*                   on existing code as most people will be using the        */
/*                   inherited TCP classes rather than SocketBase.  I am sorry*/
/*                   if you are affected by this but the new implementation   */
/*                   has better usage of resources.                           */
/******************************************************************************/

/******************************************************************************/
/* Check to see if already included.                                          */
/******************************************************************************/
#ifndef SOCKETCC_H
#define SOCKETCC_H

/******************************************************************************/
/* Include definitions required to define classes.                            */
/******************************************************************************/
#include <unistd.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <pthreadcc.h>

extern "C++"
{
#include <string>
}

/******************************************************************************/
/* class SocketException.                                                     */
/*                                                                            */
/* This class is used to store the exception details when an error occurs.    */
/* When any class method call in the socketcc library fails, it throws an     */
/* instance of SocketException, this can be caught and queried for the actual */
/* error code.  The exception can be cast to either the error code or a char  */
/* pointer for a textual description of the error.                            */
/******************************************************************************/
class SocketException
{
    public:
        enum ErrorCodes { errBadHostName, errNoIPAddress, errDNSError,
                          errNoProtocolSupport, errKernelMemory, errCannotConvertToIPv4,
                          errNoDescriptors, errPermissionDenied, errMemory,
                          errInvalidProtocol, errBadDescriptor, errIllegalPointer,
                          errAlreadyConnected, errConnectRefused, errConnectTimeOut,
                          errNetUnreachable, errHostUnreachable, errAddrInUse, errInProgress,
                          errAlreadyConnecting, errIncorrectAddrFamily, errBrdCastNotEnabled,
                          errAlreadyBound, errAddressProtected, errCantListen, errNotStreamSock,
                          errNoPendingConnections, errFirewall, errNotConnected,
                          errWouldBlock, errInterrupted, errInvalidArgument,
                          errMessageSizeTooBig, errNotBound, errOptionNotSupported,
                          errUnknown };

                        SocketException(ErrorCodes ecType = errUnknown, int iErrNo = 0);
                        SocketException(const SocketException &cOriginal);
                        ~SocketException();

        operator        const ErrorCodes() { return ecException; }
        operator        const char*() { return pcExceptionString; }

    private:
        ErrorCodes      ecException;
        char            *pcExceptionString;

        void            SetString(const char *pcErrString);
};

/******************************************************************************/
/* class IPAddress.                                                           */
/*                                                                            */
/* This class is used to provide transparent IPv4 and IPv6 address support    */
/* for the Socket classes within the library.  The constructor creates a      */
/* default IPv4 address to localhost.  The address can be assigned and parts  */
/* of the address can be extracted for usage.  This class is intended to be   */
/* used in conjunction with the Socket classes.  If the class instance is     */
/* assigned a binary IP address value, the hostname and address strings are   */
/* avaluated lazily, the reverse DNS lookup only occurs if and when the user  */
/* application requests these values.  This should speed up execution of code */
/* where these values are not required.  The member methods are:              */
/*                                                                            */
/* operator=                  : Assignment of an IPAddress class.  Can be     */
/*                              either from an existing IPAddress class, a    */
/*                              string representing an IPv4 host name, an     */
/*                              IPv4 address in an in_addr struct or an IPv6  */
/*                              address in an in6_addr struct.                */
/* operator==                 : Check equality of an IPAddress class.  Can be */
/*                              compared against another IPAddress class, a   */
/*                              string host name, or addresses stored in an   */
/*                              in_addr or in6_addr structs.                  */
/* SetHostName()              : Sets an IPAddress by providing a host name    */
/*                              and a boolean variable to indicate whether we */
/*                              want an IPv6 or IPv4 address.                 */
/* SetAddress()               : Sets an IPAddress by providing a pointer to   */
/*                              an address structure of the form in_addr or   */
/*                              in6_addr.  This pointer is cast to (char *).  */
/*                              A boolean value is used to indicate if this   */
/*                              points to an IPv6 or IPv4 address.            */
/* GetAddressFamily()         : Returns a constant integer of the address     */
/*                              family represented by the IPAddress.          */
/* GetHostName()              : Returns a constant string pointer to the      */
/*                              hostname of the represented IP Address.       */
/* Get_in_addr()              : Returns a (void *) to the address represented */
/*                              by IPAddress. This must be cast to (in_addr *)*/
/*                              or to (in6_addr *) for use.                   */
/* GetAddressString()         : Returns a constant string pointer to the      */
/*                              string representation of the IP Address       */
/*                              suitable for visual presentation.             */
/* ConvertToAddressFamily()   : Converts the represented IP Address to the    */
/*                              specified type.  An IPv4 Address can be mapped*/
/*                              to an IPv6 Address.  An IPv6 Address can only */
/*                              be converted if it is an IPv6 Mapped IPv4     */
/*                              Address.                                      */
/* GetUnmappedAddressString() : Returns a constant string pointer to the      */
/*                              string representation of the IP Address       */
/*                              suitable for visual representation.  If the   */
/*                              address is an IPv6 mapped IPv4 Address, the   */
/*                              IPv4 string is returned.                      */
/******************************************************************************/
class IPAddress
{
    public:
                            IPAddress();
                            ~IPAddress() {}

        IPAddress           & operator=(const IPAddress &cOrigAddr);
        IPAddress           & operator=(const char *pcNewHostName);
        IPAddress           & operator=(const in_addr sIP4Addr);
        IPAddress           & operator=(const in6_addr sIP6Addr);

        bool                operator==(const IPAddress &cOtherAddr) const;
        bool                operator==(const char *pcNewHostName) const;
        bool                operator==(const in_addr sIP4Addr) const;
        bool                operator==(const in6_addr sIP6Addr) const;

        operator            const char*() { return GetHostName(); }

        void                SetHostName(const char *pcNewHostName, bool bIPv6);
        void                SetAddress(const char *pcNewAddress, bool bIPv6);

        const int           GetAddressFamily()   { return iAddressType;       }
        const char *        GetHostName()        { privResolveHostName(); return strHostName.c_str(); }
        const void *        Get_in_addr()        { return (void *) pcAddress; }
        const int           Get_in_addr_length() { return iAddressLength;     }
        const char *        GetAddressString()   { privResolveStrAddress(); return pcStrAddress; }

        void                ConvertToAddressFamily(int iNewAddressFamily);
        const char *        GetUnmappedAddressString();

    private:
        int                 iAddressType;
        int                 iAddressLength;
        std::string         strHostName;
        char                pcAddress[16];
        char                pcStrAddress[INET6_ADDRSTRLEN];
        bool                bHostNameUnresolved, bStrAddressUnresolved;

        static MutualExclusion  mutexKernelLock;

        void                privSetAddress(const char *pcNewAddress,
                                           const int iNewAddressLength);
        void                privResolveHostName();
        void                privResolveStrAddress();
};

/******************************************************************************/
/* class SocketBase.                                                          */
/*                                                                            */
/* This class is used to provide basic IP socket functionality within a C++   */
/* class environment.  The socket descriptor is stored as a member variable   */
/* within the socket class.  The member methods are simple wrappers to the    */
/* stadard socket library function calls except they use IPAddress instances  */
/* and port numbers rather than sockaddr structures.  Also, the methods throw */
/* exceptions when errors occur allowing for affirmative programming within a */
/* try block.  The provided methods are as follows:                           */
/*                                                                            */
/* Constructor   : Create either an IPv4 or IPv6 socket.  The default socket  */
/*                 type is SOCK_STREAM or TCP.  A second constructor allows a */
/*                 class instance to be created to represent a socket where   */
/*                 a socket descriptor already exists.                        */
/*                 A second constructor allows creation of a SocketBase class */
/*                 given a socket descriptor.  This is primarily used to      */
/*                 implement a cleaner interface for accepting connections on */
/*                 a TCPServerSocket, but can also be used when migrating     */
/*                 existing programs as it allows keeping existing code that  */
/*                 creates the actual socket descriptor and then creating a   */
/*                 socket class from that.  The overall effect is that these  */
/*                 programs can be converted partially, one bit at a time.    */
/* Destructor    : Destroy the created socket.                                */
/* Connect()     : Connect the socket to the specified IP Address and port    */
/*                 number.                                                    */
/* Bind()        : Bind the local socket to the provided details, there are   */
/*                 methods.  If the IP Address is specified then bind to that */
/*                 IP Address, otherwise bind to the wildcard IP Address.     */
/*                 The default port number of 0 specifies for the operating   */
/*                 system to choose the port number.                          */
/* Listen()      : The socket to begin listening for connections, specifying  */
/*                 the maximum number of pending connections.                 */
/* Accept()      : Accept a pending connection, returns an instance of a      */
/*                 socket class of the specified templated type.  This socket */
/*                 class must have a constructor that can be constructed from */
/*                 a socket descriptor.                                       */
/* Recv()        : Receive data into a buffer from the socket.                */
/* RecvFrom()    : Receive data into a buffer from the socket, the IP Address */
/*                 and port number of the sender are also returned.           */
/* Send()        : The socket transmits the data to the remote socket.        */
/* SendTo()      : The socket transmits the data payload to the specified IP  */
/*                 address and port number.                                   */
/* GetSockName() : Get details of the IP address and port number bound to the */
/*                 local socket.                                              */
/* GetPeerName() : Get details of the IP address and port number bound to the */
/*                 remote socket.                                             */
/* GetSockOpt()  : Get current option settings on the socket.  Needs the code */
/*                 level and option name (same parameters as for getsockopt() */
/*                 call).  Returns current setting of option and size of data.*/
/* SetSockOpt()  : Set current option settings on the socket.  Needs the code */
/*                 level and option name (same parameters as for setsockopt() */
/*                 call) as well as new value for the option.                 */
/* FCntl()       : Call any fcntl command on the socket (same parameters as   */
/*                 for fnctl()                                                */
/*                                                                            */
/* NOTE: SocketSet is made a friend of SocketBase so that it can access the   */
/*       protected member variables of SocketBase when maintaining its fd_set.*/
/******************************************************************************/
class SocketBase
{
    friend      class SocketSet;

    public:
                        SocketBase(bool bSocketIsIPv6 = false, int iSocketType = SOCK_STREAM);
                        SocketBase(int iNewSockDesc);
        virtual         ~SocketBase();

        void            Connect(IPAddress &cServAddr, int iServPortNo);
        void            Bind(IPAddress &cLocalAddr, int iLocalPort = 0);
        void            Bind(int iLocalPort = 0);
        void            Listen(int iBackLog);
        void            Accept(SocketBase *pcConnection, IPAddress &cRemoteAddr, int &iRemotePortNo);
        template<class ReturnType>
        ReturnType *    Accept(IPAddress &cRemoteAddr, int &iRemotePortNo)
        {
            return new ReturnType(protAccept(cRemoteAddr, iRemotePortNo));
        }

        int             Recv(void *pBuffer, int iBufLength, unsigned int uiFlags);
        int             RecvFrom(void *pBuffer, int iBufLength, unsigned int uiFlags,
                                 IPAddress &cSourceIP, int &iSourcePortNumber);

        int             Send(const void *pPayload, int iPayloadLength, unsigned int uiFlags);
        int             SendTo(const void *pPayload, int iPayloadLength, unsigned int uiFlags,
                               IPAddress &cDestinationIP, int iDestinationPortNumber);

        void            GetSockName(IPAddress &cLocalAddr, int &iLocalPort);
        void            GetPeerName(IPAddress &cRemoteAddr, int &iRemotePort);

        void            GetSockOpt(int iCodeLevel, int iOptionName, void *pOptionData, int &iDataLength);
        void            SetSockOpt(int iCodeLevel, int iOptionName, const void *pOptionData, int iDataLength);
        int             FCntl(int iCommand, long lArgument);

    protected:
        MutualExclusion mutexSocket;
        pid_t           pidMutExOwner;

        int             iSockDesc;
        bool            bIPv6Socket;

        static bool     bFirstInstance;
        static void     IOSignalHandler(int iSig) { return; }

        int             protAccept(IPAddress &cRemoteAddr, int &iRemotePortNo);

        void            WaitMutex() { mutexSocket.Lock(); pidMutExOwner = getpid(); }
        void            ClearMutex() { mutexSocket.Unlock(); }
};

/******************************************************************************/
/* TCP Socket Classes.                                                        */
/******************************************************************************/
/* class TCPSocket.                                                           */
/*                                                                            */
/* The TCPSocket class provides common TCP functionality of client and server */
/* side TCP sockets within a class and provides a class from which both       */
/* TCPClientSocker and TCPServerSocket can inherit - this class inherits from */
/* the SocketBase base class.  Provision is made to both send and receive     */
/* data, as well as obtain addressing information (IP Address and port        */
/* number), both of the local and remote side of the socket.  All of these    */
/* features/methods will fail if the socket is not connected and a Socket     */
/* Exception will be thrown.  The provided methods are as follows:            */
/*                                                                            */
/* Constructor        : Create either an IPv6 or IPv4 TCP Socket, forwards    */
/*                      the call to the base class constructor.  There also   */
/*                      exists a protected constructor that allows creation   */
/*                      of a socket using an existing socket descriptor.      */
/*                      A second constructor allows creation of a TCP Socket  */
/*                      class given a socket descriptor.                      */
/* Destructor         : Destroy the created Socket.                           */
/* SendData()         : Send data from this socket to the remote connection.  */
/*                      Will fail if not connected.                           */
/* SendASCII()        : Send a NULL terminated character string from this     */
/*                      socket to the remote connection.                      */
/* SendBinary16Bits() : Send a 16-bit value from this socket to the remote    */
/*                      connection.  Binary value will be converted from host */
/*                      to network byte order.                                */
/* SendBinary32Bits() : Send a 32-bit value from this socket to the remote    */
/*                      connection.  Binary value will be converted from host */
/*                      to network byte order.                                */
/* RecvData()         : Receive data sent to this socket from the remote      */
/*                      connection.  Will fail if not connected.              */
/* RecvBinary16Bits() : Receive a 16-bit value sent to this socket from the   */
/*                      remote connection.  Binary value will be converted    */
/*                      from network to host byte order.                      */
/* RecvBinary32Bits() : Receive a 32-bit value sent to this socket from the   */
/*                      remote connection.  Binary value will be converted    */
/*                      from network to host byte order.                      */
/* RecvASCII()        : Receive a NULL terminated ASCII string sent to this   */
/*                      socket from the remote connection.  A new (char *)    */
/*                      is allocated and must be free by the calling code.    */
/* LocalIPAddress()   : If set, returns the local IP address of the socket.   */
/* RemoteIPAddress()  : If connected, returns the IP address of the remote    */
/*                      socket connection.                                    */
/* LocalPortNumber()  : If set, returns the local port number of the socket.  */
/* RemotePortNumber() : If connected, returns the port number of the remote   */
/*                      socket connection.                                    */
/* GetSockOpt()       : Provision of call through to base class method.       */
/* SetSockOpt()       : Provision of call through to base class method.       */
/* FCntl()            : Provision of call through to base class method.       */
/*                                                                            */
/* NOTE: TCPServerSocket is made a friend of TCPSocket so that it can call    */
/*       the protected member methods of TCPSocket in the Accept() method.    */
/******************************************************************************/
class TCPSocket : protected SocketBase
{
    friend          class TCPServerSocket;

    public:
                    TCPSocket(bool bUseIPv6 = false);
                    TCPSocket(int iNewSockDesc);
        virtual     ~TCPSocket() {}

        int         SendData(const void *pData, int iDataLen, unsigned int uiFlags = 0);
        void        SendASCII(const char *pcData, char cTerminator = '\0', unsigned int uiFlags = 0);
        void        SendBinary16Bits(const int iData, unsigned int uiFlags = 0);
        void        SendBinary32Bits(const long lData, unsigned int uiFlags = 0);

        int         RecvData(void *pBuffer, int iBufferLen, unsigned int uiFlags = 0);
        char *      RecvASCII(char cTerminator = '\0', unsigned int uiFlags = 0);
        int         RecvBinary16Bits(unsigned int uiFlags = 0);
        long        RecvBinary32Bits(unsigned int uiFlags = 0);

        IPAddress   &LocalIPAddress();
        IPAddress   &RemoteIPAddress();
        int         LocalPortNumber();
        int         RemotePortNumber();

        void        GetSockOpt(int iCodeLevel, int iOptionName, void *pOptionData, int &iDataLength);
        void        SetSockOpt(int iCodeLevel, int iOptionName, const void *pOptionData, int iDataLength);
        int         FCntl(int iCommand, long lArgument);

    protected:
        bool        bLocalSet, bIsConnected;
        IPAddress   cLocalAddress, cRemoteAddress;
        int         iLocalPort, iRemotePort;

        void        SetLocal();
        void        SetConnected();
};

/******************************************************************************/
/* class TCPClientSocket.                                                     */
/*                                                                            */
/* The TCPClientSocket class provides client based TCP functionality within a */
/* class.  This class inherits from the TCPSocket base class and provides the */
/* base functionality of a TCP client socket.  All the basic TCP methods from */
/* TCPSocket are inherited, the constructor also performs connection to the   */
/* server, the destructor disconnects the socket.  If an error occurs at any  */
/* stage, a Socket Exception is thrown.  The provided methods are as follows: */
/*                                                                            */
/* Constructor : Create either an IPv6 or IPv4 TCP Client Socket, the socket  */
/*               is connected to the server as specified by the provided IP   */
/*               Address and port number.  The type (IPv4 or IPv6) of the     */
/*               socket is determined by the IP address of the server.        */
/* Destructor  : Destroy the created Socket.                                  */
/*                                                                            */
/* The following methods are inherited from TCPSocket:                        */
/*                                                                            */
/*    SendData()    LocalIPAddress()    RemoteIPAddress()    GetSockOpt()     */
/*    RecvData()    LocalPortNumber()   RemotePortNumber()   SetSockOpt()     */
/******************************************************************************/
class TCPClientSocket : public TCPSocket
{
    public:
                    TCPClientSocket(IPAddress &cServAddr, int iServPort);
        virtual     ~TCPClientSocket() {}
};

/******************************************************************************/
/* class TCPServerSocket.                                                     */
/*                                                                            */
/* The TCPServerSocket class provides server based TCP functionality within a */
/* class.  This class inherits from the TCPSocket base class and provides the */
/* base functionality of a TCP server socket.  All the basic TCP methods from */
/* TCPSocket are inherited, newly provided methods are Accept() and new       */
/* constructors which enable creation of a listening server socket where the  */
/* bound IP Address is either specified or a wildcard and the bound port      */
/* number is either specified or a wildcard.  If an error occurs at any       */
/* stage, a Socket Exception is thrown.  The provided methods are as follows: */
/* are as follows:                                                            */
/*                                                                            */
/* Constructor    : Create either an IPv6 or IPv4 TCP Server Socket, there    */
/*                  are two constructors.  One constructor offers the option  */
/*                  of specifying an IP Address to bind the listening socket  */
/*                  to, the other will bind to the wildcard IP Address.  The  */
/*                  other options are common to both constructors. iLocalPort */
/*                  specifies the port number bound to the listening socket,  */
/*                  a value of zero means the operating system will choose    */
/*                  this value, it can then be obtained by calling the base   */
/*                  class LocalPortNumber() method.  iBackLog indicates the   */
/*                  maximum number of pending connections.  If the IP Address */
/*                  is provided, it will be queried to determine whether to   */
/*                  create an IPv4 or IPv6 socket.  If we are binding to the  */
/*                  wildcard address, the bUseIPv6 flag determines what type  */
/*                  of socket to create (IPv4 or IPv6).                       */
/* Destructor     : Destroy the created Socket.                               */
/* AcceptClient() : Accepts a pending connection on the listening server      */
/*                  socket.  Creates a TCPSocket instance to refer to the     */
/*                  newly created socket for future communications to that    */
/*                  client.  A pointer to the TCPSocket instance is returned. */
/*                  This pointer must be deleted to close the socket with the */
/*                  client.                                                   */
/*                                                                            */
/* The following methods are inherited from TCPSocket:                        */
/*                                                                            */
/*    SendData()    LocalIPAddress()    RemoteIPAddress()    GetSockOpt()     */
/*    RecvData()    LocalPortNumber()   RemotePortNumber()   SetSockOpt()     */
/******************************************************************************/
class TCPServerSocket : public TCPSocket
{
    public:
                    TCPServerSocket(IPAddress &cLocalAddr, int iPortNo = 0,
                                    int iBackLog = 1);
                    TCPServerSocket(int iPortNo = 0, int iBackLog = 1,
                                    bool bUseIPv6 = false);
        virtual     ~TCPServerSocket() {}

        TCPSocket   *AcceptClient();
};

/******************************************************************************/
/* UDP Socket Classes.                                                        */
/******************************************************************************/
/* class UDPSocket.                                                           */
/*                                                                            */
/* The UDPSocket class provides client based UDP functionality within a       */
/* class.  This class inherits from the SocketBase base class and provides    */
/* only the base functionality of a UDP client socket.  If any error occurs,  */
/* a Socket Exception is thrown.  The provided methods are as follows:        */
/*                                                                            */
/* Constructor       : Create either an IPv6 or IPv4 Socket.                  */
/*                     A second constructor allows creation of a UDPSocket    */
/*                     class given a socket descriptor.  This can be used     */
/*                     when migrating existing programs as it allows keeping  */
/*                     existing code that creates the actual socket           */
/*                     descriptor and then creating a socket class from that. */
/*                     The overall effect is that these programs can be       */
/*                     converted partially, one bit at a time.                */
/* Destructor        : Destroy the created Socket.                            */
/* SendDatagram()    : Send a Datagram from this socket to the provided       */
/*                     destination IP Address and Port Number.                */
/* ReceiveDatagram() : Receive a datagram sent to this socket.  The source IP */
/*                     Address and Port Number are stored in the provided     */
/*                     variables.                                             */
/* GetSockOpt()      : Provision of call through to base class method.        */
/* SetSockOpt()      : Provision of call through to base class method.        */
/* FCntl()           : Provision of call through to base class method.        */
/******************************************************************************/
class UDPSocket : protected SocketBase
{
    public:
                UDPSocket(bool bUseIPv6 = false);
                UDPSocket(int iNewSockDesc) : SocketBase(iNewSockDesc) {}
        virtual ~UDPSocket() {}

        int     SendDatagram(const void *pBuffer, int iBufLength, IPAddress &cDestAddr,
                             int iDestPort, unsigned int uiFlags = 0);
        int     ReceiveDatagram(void *pBuffer, int iBufLength, IPAddress &cSourceAddr,
                                int &iSourcePort, unsigned int uiFlags = 0);

        void    GetSockOpt(int iCodeLevel, int iOptionName, void *pOptionData, int &iDataLength);
        void    SetSockOpt(int iCodeLevel, int iOptionName, const void *pOptionData, int iDataLength);
        int     FCntl(int iCommand, long lArgument);
};

/******************************************************************************/
/* class UDPServerSocket.                                                     */
/*                                                                            */
/* The UDPServerSocket class provides server based UDP functionality within   */
/* a class.  This class inherits from the UDPSocket base class and provides   */
/* the base functionality of a Server side UDP Socket.  The Send and Receive  */
/* data methods are inherited from UDPSocket, the constructor requires the    */
/* provision of local port number and IP Address to bind to.  If an error     */
/* occurs at any stage, a Socket Exception is thrown.  The provided methods   */
/* are as follows:                                                            */
/*                                                                            */
/* Constructor       : Create either an IPv6 or IPv4 Server Socket on the     */
/*                     specified port.  Two constructors are available, the   */
/*                     first binds the socket to the specified local IP       */
/*                     Address and port number, the second to all local IP    */
/*                     Addresses and the specified port number.  The default  */
/*                     port number of zero means that the system chooses the  */
/*                     the port number of the socket.                         */
/* Destructor        : Destroy the created Socket.                            */
/* SendDatagram()    : Inherited from UDPSocket with the same parameters.     */
/* ReceiveDatagram() : Inherited from UDPSocket with the same parameters.     */
/* LocalIPAddress()  : If connected, returns the local IP address of the      */
/*                     connected socket.                                      */
/* LocalPortNumber() : If connected, returns the local port number of the     */
/*                     connected socket.                                      */
/******************************************************************************/
class UDPServerSocket : public UDPSocket
{
    public:
                    UDPServerSocket(IPAddress &cLocalAddr, int iLocalPortNo = 0);
                    UDPServerSocket(int iLocalPortNo = 0, bool bUseIPv6 = false);
        virtual     ~UDPServerSocket() {}

        IPAddress   &LocalIPAddress()  { return cLocalAddress;  }
        int         LocalPortNumber()  { return iLocalPort;  }

    private:
        IPAddress   cLocalAddress;
        int         iLocalPort;
};

/******************************************************************************/
/* class UDPConnectedSocket.                                                  */
/*                                                                            */
/* The UDPConnectedSocket class provides client based UDP functionality       */
/* within a class.  This class inherits from the SocketBase base class and    */
/* provides the base functionality of a connected UDP Socket.  Data can only  */
/* be sent or received once the socket has been connected and can only be     */
/* sent to or received from the server the socket is connected to.  There is  */
/* provision to connect and disconnect the socket as well as obtaining        */
/* information about the local and remote IP addresses and port numbers.  If  */
/* an error occurs at any stage, a Socket Exception is thrown.  The provided  */
/* methods are as follows:                                                    */
/*                                                                            */
/* Constructor        : Create either an IPv6 or IPv4 Connected Socket, two   */
/*                      constructors are available, the first creates the     */
/*                      socket in a disconnected state, the second connects   */
/*                      to the specified server.                              */
/* Destructor         : Destroy the created Socket.                           */
/* Connect()          : Connect the socket to the specified server and port.  */
/* DisConnect()       : Disconnect the socket, can no longer send or receive  */
/*                      data.  This is not necessary when destroying the      */
/*                      socket, the destructor will handle any necessary      */
/*                      clean-up.                                             */
/* SendDatagram()     : Send a Datagram from this socket to the connected     */
/*                      server.  Will fail if not connected.                  */
/* ReceiveDatagram()  : Receive a datagram sent to this socket from the       */
/*                      connected server.  Will fail if not connected.        */
/* LocalIPAddress()   : If connected, returns the local IP address of the     */
/*                      connected socket.                                     */
/* RemoteIPAddress()  : If connected, returns the server IP address of the    */
/*                      connected socket.                                     */
/* LocalPortNumber()  : If connected, returns the local port number of the    */
/*                      connected socket.                                     */
/* RemotePortNumber() : If connected, returns the server port number of the   */
/*                      connected socket.                                     */
/* GetSockOpt()       : Provision of call through to base class method.       */
/* SetSockOpt()       : Provision of call through to base class method.       */
/* FCntl()            : Provision of call through to base class method.       */
/******************************************************************************/
class UDPConnectedSocket : protected SocketBase
{
    public:
                    UDPConnectedSocket(bool bUseIPv6 = false);
                    UDPConnectedSocket(IPAddress &cServAddr, int iServPortNo,
                                   bool bUseIPv6 = false);
        virtual     ~UDPConnectedSocket() {}

        void        Connect(IPAddress &cServAddr, int iServPortNo);
        void        DisConnect();

        int         SendDatagram(const void *pBuffer, int iBufLength, unsigned int uiFlags = 0);
        int         ReceiveDatagram(void *pBuffer, int iBufLength, unsigned int uiFlags = 0);

        IPAddress   &LocalIPAddress()  { return cLocalAddress;  }
        IPAddress   &RemoteIPAddress() { return cServerAddress; }
        int         LocalPortNumber()  { return iLocalPort;  }
        int         RemotePortNumber() { return iServerPort; }

        void        GetSockOpt(int iCodeLevel, int iOptionName, void *pOptionData, int &iDataLength);
        void        SetSockOpt(int iCodeLevel, int iOptionName, const void *pOptionData, int iDataLength);
        int         FCntl(int iCommand, long lArgument);

    private:
        bool        bIsConnected;
        IPAddress   cLocalAddress, cServerAddress;
        int         iLocalPort, iServerPort;
};

/******************************************************************************/
/* class SocketSet.                                                           */
/*                                                                            */
/* The SocketSet class provides support for programming using the select()    */
/* socket function calls.  This class provides support for the building and   */
/* and querying of fd_set type constructs used in by select().  Pointers to   */
/* SocketBase and derivered classes are supplied so that the socket descriptor*/
/* is fully hidden.  The basic implementation is fully contained within the   */
/* the header file.  Public methods are:                                      */
/*                                                                            */
/* Constructor     : Initialise the Socket Set to the empty set.              */
/* Destructor      : Free any allocated resources.                            */
/* Clear()         : Clear the Set to contain no socket descriptors.          */
/* operator +=     : Add the provided Socket to the represented Set.          */
/* operator -=     : Remove the provided Socket from the represented Set.     */
/* IsMember()      : Return true if the provided Socket class is a member of  */
/*                   the represented Set.                                     */
/* cast (int)      : Return the largest socket descriptor contained by Set.   */
/* cast (fd_set *) : Return a pointer to the contained fd_set object.         */
/******************************************************************************/
class SocketSet
{
    public:
                    SocketSet() { Clear(); }
        virtual     ~SocketSet() {}

        void        Clear() { FD_ZERO(&fsSet); iMaxSockDesc = 0; }

        void        operator+=(SocketBase *pcSocket)
                    {
                        FD_SET(pcSocket->iSockDesc, &fsSet);
                        if (pcSocket->iSockDesc > iMaxSockDesc)
                            iMaxSockDesc = pcSocket->iSockDesc;
                    }

        void        operator-=(SocketBase *pcSocket)
                    { FD_CLR(pcSocket->iSockDesc, &fsSet); }

        bool        IsMember(SocketBase *pcSocket) 
                    { return FD_ISSET(pcSocket->iSockDesc, &fsSet); }

        operator    int() { return iMaxSockDesc; }
        operator    fd_set *() { return &fsSet; }

    private:
        int         iMaxSockDesc;
        fd_set      fsSet;
};

#endif

/******************************************************************************/
/* End of File: socketcc.h                                                    */
/******************************************************************************/
