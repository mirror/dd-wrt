/******************************************************************************/
/* File: myecho.cpp                                                           */
/******************************************************************************/
/* This source code file contains sample code for using the SocketCC network  */
/* class library.  The sample code implements an Echo Server and client,      */
/* using both TCP and UDP sockets over IPv4 and IPv6 networks.  Command line  */
/* parameters indicate the functionality at run time.  This code is mainly    */
/* for show and an example in using SocketCC.                                 */
/******************************************************************************/
/* Written By: Jason But                                                      */
/* Copyright:  CTIE - Monash University 2001                                  */
/*                                                                            */
/* Notes:                                                                     */
/*    Version 1.00 - Original version of sample program.                      */
/*                                                                            */
/*    Version 1.10 - Modified to use version 1.10 of SocketCC.                */
/******************************************************************************/

/******************************************************************************/
/* Include standard C Libraries.                                              */
/******************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/******************************************************************************/
/* Include SocketCC Socket Class Library.                                     */
/******************************************************************************/
#include <socketcc.h>

/******************************************************************************/
/* void PrintIPAddress(char *pcInformationString, IPAddress &cAddr)           */
/*                                                                            */
/* This function will take an information string and IPAddress and output the */
/* the information string and details of the IPAddress to stdout.             */
/******************************************************************************/
void
PrintIPAddress(char *pcInformationString, IPAddress &cAddr)
{
    printf("%s\n", pcInformationString);
    printf(" - Name    : %s\n", (const char *) cAddr);
    printf(" - Family  : %s\n", (cAddr.GetAddressFamily() == AF_INET6)?"IPv6":
                    ((cAddr.GetAddressFamily() == AF_INET)?"IPv4":"Unknown"));
    printf(" - Address : %s\n", cAddr.GetAddressString());
}

/******************************************************************************/
/* void TCPEchoClient(IPAddress &cEchoServer, int iPortNumber,                */
/*                    char *pcEchoString, bool bIPv6)                         */
/*                                                                            */
/* This function performs the task of the echo client operating using a TCP   */
/* socket.  Parameters include the IP Address and Port Number of the Echo     */
/* Server to use, the data string to send to the Echo Server and a flag that  */
/* indicates whether to use IPv6 or IPv4 sockets.  The actual functional code */
/* consists of creating a TCPClientSocket instance, calling the ConnectServer */
/* method to connect to the Echo Server, then calling SendData followed by    */
/* RecvData to perform the transaction.  The function then ends, closing the  */
/* TCP Socket when cClientSocket goes out of scope and returning to the       */
/* calling function.  If an error occurs, function execution stops and an     */
/* exception is thrown, this is not caught at this level.  Other code simply  */
/* prints details for debugging purposes.                                     */
/******************************************************************************/
void
TCPEchoClient(IPAddress &cEchoServer, int iPortNumber, char *pcEchoString)
{
    TCPClientSocket     cClientSocket(cEchoServer, iPortNumber);
    char                pcRecvBuffer[65535];
    int                 iBytesTransferred;

    pcRecvBuffer[0] = 0;

    printf("TCP %s Echo Client\n\n", (cEchoServer.GetAddressFamily() == AF_INET6)?"IPv6":"IPv4");

    PrintIPAddress("Using Echo Server", cEchoServer);
    printf(" - Port    : %d\n\n", iPortNumber);

    PrintIPAddress("Socket Details - Local Information", cClientSocket.LocalIPAddress());
    printf(" - Port    : %d\n\n", cClientSocket.LocalPortNumber());

    PrintIPAddress("Socket Details - Server Information", cClientSocket.RemoteIPAddress());
    printf(" - Port    : %d\n\n", cClientSocket.RemotePortNumber());

    printf("Send Buffer    : [%s]\n", pcEchoString);
    printf("Receive Buffer : [%s]\n\n", pcRecvBuffer);

    printf("Sending Echo Request...\n");
    iBytesTransferred = cClientSocket.SendData(pcEchoString, strlen(pcEchoString));
    printf("Sent %d bytes of data\n\n", iBytesTransferred);

    printf("Receiving Echo Response...\n");
    iBytesTransferred = cClientSocket.RecvData(pcRecvBuffer, 65535);
    printf("Received %d bytes of data\n", iBytesTransferred);
    pcRecvBuffer[iBytesTransferred] = 0;

    printf("Send Buffer    : [%s]\n", pcEchoString);
    printf("Receive Buffer : [%s]\n", pcRecvBuffer);

}

/******************************************************************************/
/* void TCPEchoServer(int iPortNumber, bool bIPv6)                            */
/*                                                                            */
/* This function performs the task of the echo server operating using a TCP   */
/* socket.  Parameters include Port Number the server will set up for the     */
/* listening socket to use and a flag that indicates whether to use IPv6 or   */
/* IPv4 sockets.  The actual functional code consists of creating a           */
/* TCPServerSocket instance bound to the provided Port Number.  We then go    */
/* into an infinite loop where we call AcceptClient, waiting for a connection */
/* from an Echo client.  The connection is assigned to pcClientSocket.  We    */
/* then call RecvData followed by SendData to perform the Echo transaction.   */
/* Finally, pcClientSocket is deleted to close the TCP Connection, we then    */
/* jump to the top of the loop to wait for the next connection.  If an error  */
/* occurs, function execution stops and an exception is thrown, this is not   */
/* caught at this level.  Other code prints details for debugging purposes.   */
/******************************************************************************/
void
TCPEchoServer(int iPortNumber, bool bIPv6)
{
    TCPServerSocket     cServerSocket(iPortNumber, 1, bIPv6);
    TCPSocket           *pcClientSocket;
    int                 iBytesTransferred;
    char                pcBuffer[65535];

    pcBuffer[0] = 0;

    printf("TCP %s Echo Server\n\n", (bIPv6)?"IPv6":"IPv4");

    PrintIPAddress("Created Server Socket - Socket Details", cServerSocket.LocalIPAddress());
    printf(" - Port    : %d\n\n", cServerSocket.LocalPortNumber());

    for (;;)
    {
        printf("Waiting for Connection...\n");

        pcClientSocket = cServerSocket.AcceptClient();

        printf("Connection Made...\n\n");

        PrintIPAddress("Listening Server Socket - Socket Details", cServerSocket.LocalIPAddress());
        printf(" - Port    : %d\n\n", cServerSocket.LocalPortNumber());

        PrintIPAddress("Client Communication Socket on", pcClientSocket->LocalIPAddress());
        printf(" - Port    : %d\n\n", pcClientSocket->LocalPortNumber());
        PrintIPAddress("Is Connected to", pcClientSocket->RemoteIPAddress());
        printf(" - Port    : %d\n\n", pcClientSocket->RemotePortNumber());

        iBytesTransferred = pcClientSocket->RecvData(pcBuffer, 65535);
        printf("Received %d bytes of data\n", iBytesTransferred);
        printf("Buffer : [");
        for (int i = 0; i < iBytesTransferred; i++)
        {
            printf("%c", pcBuffer[i]);
        }
        printf("]\n\n");

        printf("Echoing data to client...\n\n");

        iBytesTransferred = pcClientSocket->SendData(pcBuffer, iBytesTransferred);
        printf("Sent %d bytes of data\n\n", iBytesTransferred);

        printf("Closing Client Socket...\n\n");
        delete pcClientSocket;
    }
}

/******************************************************************************/
/* void UDPEchoClient(IPAddress &cEchoServer, int iPortNumber,                */
/*                    char *pcEchoString, bool bIPv6)                         */
/*                                                                            */
/* This function performs the task of the echo client operating using a UDP   */
/* socket.  Parameters include the IP Address and Port Number of the Echo     */
/* Server to use, the data string to send to the Echo Server and a flag that  */
/* indicates whether to use IPv6 or IPv4 sockets.  The actual functional code */
/* consists of creating a UDPConnectedSocket instance, calling the Connect    */
/* method to connect (using the concept of a connected UDP socket where data  */
/* can only be sent to or received from the connected remote socket) to the   */
/* Echo Server, then calling SendDatagram followed by ReceiveDatagram to      */
/* perform the transaction.  DisConnect is called to close the connected UDP  */
/* socket and we return to the calling function.  If there is an error,       */
/* function execution stops and an exception is thrown, this is not caught at */
/* this level.  Other code simply prints details for debugging purposes.      */
/******************************************************************************/
void
UDPEchoClient(IPAddress &cEchoServer, int iPortNumber, char *pcEchoString, bool bIPv6)
{
    UDPConnectedSocket  cClientSocket(bIPv6);
    char                pcRecvBuffer[65535];
    int                 iBytesTransferred;

    pcRecvBuffer[0] = 0;

    printf("UDP %s Echo Client\n\n", (bIPv6)?"IPv6":"IPv4");

    PrintIPAddress("Using Echo Server", cEchoServer);
    printf(" - Port    : %d\n\n", iPortNumber);

    cClientSocket.Connect(cEchoServer, iPortNumber);

    printf("Socket Connected (UDP connected socket)...\n");

    PrintIPAddress("Socket Details - Local Information", cClientSocket.LocalIPAddress());
    printf(" - Port    : %d\n\n", cClientSocket.LocalPortNumber());

    PrintIPAddress("Socket Details - Server Information", cClientSocket.RemoteIPAddress());
    printf(" - Port    : %d\n\n", cClientSocket.RemotePortNumber());

    printf("Send Buffer    : [%s]\n", pcEchoString);
    printf("Receive Buffer : [%s]\n\n", pcRecvBuffer);

    printf("Sending Echo Request...\n");
    iBytesTransferred = cClientSocket.SendDatagram(pcEchoString, strlen(pcEchoString));
    printf("Sent %d bytes of data\n\n", iBytesTransferred);

    printf("Receiving Echo Response...\n");
    iBytesTransferred = cClientSocket.ReceiveDatagram(pcRecvBuffer, 65535);
    printf("Received %d bytes of data\n", iBytesTransferred);
    pcRecvBuffer[iBytesTransferred] = 0;

    printf("Send Buffer    : [%s]\n", pcEchoString);
    printf("Receive Buffer : [%s]\n", pcRecvBuffer);

    cClientSocket.DisConnect();
}

/******************************************************************************/
/* void UDPEchoServer(int iPortNumber, bool bIPv6)                            */
/*                                                                            */
/* This function performs the task of the echo server operating using a UDP   */
/* socket.  Parameters include Port Number the server will set up to listen   */
/* for datagrams and a flag that indicates whether to use IPv6 or IPv4        */
/* sockets.  The actual functional code consists of creating a UDPServerSocket*/
/* instance bound to the provided Port Number.  We then go into an infinite   */
/* waiting for a datagram to arrive by calling ReceiveDatagram, this returns  */
/* the IP Address and Port Number of the sender.  We call the SendDatagram    */
/* method to echo the datagram back to the sender.  Finally, we jump to the   */
/* top of the loop to wait for the next datagram.  If an error occurs,        */
/* function execution stops and an exception is thrown, this is not caught at */
/* this level.  Other code prints details for debugging purposes.             */
/******************************************************************************/
void
UDPEchoServer(int iPortNumber, bool bIPv6)
{
    UDPServerSocket     cServerSocket(iPortNumber, bIPv6);
    IPAddress           cIPSource;
    int                 iBytesTransferred, iPortSource;
    char                pcBuffer[65535];

    pcBuffer[0] = 0;

    printf("UDP %s Echo Server\n\n", (bIPv6)?"IPv6":"IPv4");

    PrintIPAddress("Created Server Socket - Socket Details", cServerSocket.LocalIPAddress());
    printf(" - Port    : %d\n\n", cServerSocket.LocalPortNumber());

    for (;;)
    {
        PrintIPAddress("Waiting for data on :", cServerSocket.LocalIPAddress());
        printf(" - Port    : %d\n\n", cServerSocket.LocalPortNumber());

        iBytesTransferred = cServerSocket.ReceiveDatagram(pcBuffer, 65535, cIPSource, iPortSource);
        PrintIPAddress("Received datagram from :", cIPSource);
        printf(" - Port    : %d\n", iPortSource);
        printf(" - Size    : %d bytes\n", iBytesTransferred);
        printf(" - Buffer  : [");
        for (int i = 0; i < iBytesTransferred; i++)
        {
            printf("%c", pcBuffer[i]);
        }
        printf("]\n\n");

        printf("Echoing datagram to client...\n\n");

        iBytesTransferred = cServerSocket.SendDatagram(pcBuffer, iBytesTransferred, cIPSource, iPortSource);
        printf("Sent %d bytes of data\n\n", iBytesTransferred);
    }
}

/******************************************************************************/
/* bool SetFlag(char *pcArg, char *pcTrueString, char *pcFalseString,         */
/*              bool &bFlag, bool bDefault)                                   */
/*                                                                            */
/* This function will take an argument presented as a string, as well as two  */
/* strings to compare the argument to, the first of these two strings will    */
/* represent setting the flag to true, the second to false.  The fourth       */
/* parameter is the flag that will be set based on the value of the argument  */
/* while the final parameter signifies the default value for the flag if      */
/* neither of the two values match.  The function returns true if one of the  */
/* arguments matched and the flag was set.  The function returns false if     */
/* neither argument matched and the flag was set to the default value.  This  */
/* function is used to help parse the command line looking for flags.  The    */
/* return value will say if the parameter was parsed or a default value was   */
/* applied.                                                                   */
/******************************************************************************/
bool
SetFlag(char *pcArg, char *pcTrueString, char *pcFalseString, bool &bFlag, bool bDefault)
{
    bFlag = false;
    if (strcasecmp(pcArg, pcTrueString))
    {
        if (strcasecmp(pcArg, pcFalseString))
        {
            bFlag = bDefault;
            return false;
        }
    } else
    {
        bFlag = true;
    }
    return true;
}

/******************************************************************************/
/* int main(int iArgC, char **ppcArgV)                                        */
/*                                                                            */
/* Program code, first some long winded code to parse all of the command line */
/* parameters, set variables based on these paramters and print a command     */
/* usage description if the parameters are incorrectly formatted.  The        */
/* command line format is:                                                    */
/*                                                                            */
/* myecho serv [ip4 | ip6] [tcp | udp] portnum                                */
/* myecho [client] [ip4 | ip6] [tcp | udp] servname portnum echostring        */
/*                                                                            */
/* For the echo server, the second two parameters are optional, for the echo  */
/* client, the first three parameters are optional.  The default values       */
/* signify a IPv4 TCP Echo Client.  A port number is always specified, the    */
/* echo client also requires the name of the Echo Server and the string to    */
/* send to the server for processing.                                         */
/*                                                                            */
/* Following this, we call one of the four echo functions based on the values */
/* of the variables.  This is done in a try block, a socket exception is then */
/* reported as an error.                                                      */
/******************************************************************************/
int
main(int iArgC, char **ppcArgV)
{
    bool        bUseIPv6, bUseTCP, bServer;
    IPAddress   cEchoServerAddress;
    int         iServerPortNumber, iCurrentArg;
    char        *pcEchoString;

    try
    {
        iCurrentArg = 1;
        if (iCurrentArg == iArgC) throw iCurrentArg;

        if (SetFlag(ppcArgV[iCurrentArg], "serv", "client", bServer, false))
        {
            iCurrentArg++;
            if (iCurrentArg == iArgC) throw iCurrentArg;
        }
        if (SetFlag(ppcArgV[iCurrentArg], "ip6", "ip4", bUseIPv6, false))
        {
            iCurrentArg++;
            if (iCurrentArg == iArgC) throw iCurrentArg;
        }
        if (SetFlag(ppcArgV[iCurrentArg], "tcp", "udp", bUseTCP, true))
        {
            iCurrentArg++;
            if (iCurrentArg == iArgC) throw iCurrentArg;
        }

        if (!bServer)
        {
            try
            {
                cEchoServerAddress.SetHostName(ppcArgV[iCurrentArg], bUseIPv6);
                iCurrentArg++;
                if (iCurrentArg == iArgC) throw iCurrentArg;
            }
            catch (SocketException &excep)
            {
                printf("Socket Exception : %s\n\n", (const char *) excep);
                throw iCurrentArg;
            }
        }

        char *pcEndNumber;
        iServerPortNumber = strtol((const char *)ppcArgV[iCurrentArg], &pcEndNumber, 10);
        if (strlen(pcEndNumber)) throw iCurrentArg;
        iCurrentArg++;

        if (!bServer)
        {
            if (iCurrentArg == iArgC) throw iCurrentArg;
            pcEchoString = ppcArgV[iCurrentArg];
            iCurrentArg++;
        }

        if (iCurrentArg != iArgC) throw iCurrentArg;
    }
    catch (int &iBadArg)
    {
        printf("Error in Argument %d, signified by (...)\n\n", iBadArg);
        printf("./%s ", ppcArgV[0]);
        for (iCurrentArg = 1; iCurrentArg < iArgC; iCurrentArg++)
        {
            printf((iCurrentArg == iBadArg)?"(%s) ":"%s ", ppcArgV[iCurrentArg]);
        }
        printf("\n\n");
        printf("Usage: ./%s serv [ip4 | ip6] [tcp | udp] portnum\n", ppcArgV[0]);
        printf("Usage: ./%s [client] [ip4 | ip6] [tcp | udp] servname portnum echostring\n", ppcArgV[0]);
        printf("Default Values signify a IPv4 TCP Echo Client\n");

        return 1;
    }

    try
    {
        if (bServer)
        {
            if (bUseTCP)
            {
                TCPEchoServer(iServerPortNumber, bUseIPv6);
            } else
            {
                UDPEchoServer(iServerPortNumber, bUseIPv6);
            }
        } else
        {
            if (bUseTCP)
            {
                TCPEchoClient(cEchoServerAddress, iServerPortNumber, pcEchoString);
            } else
            {
                UDPEchoClient(cEchoServerAddress, iServerPortNumber, pcEchoString, bUseIPv6);
            }
        }
    }
    catch (SocketException &excep)
    {
        printf("Socket Exception : %s\n\n", (const char *) excep);
        return 1;
    }
    catch (...)
    {
        printf("Other Error\n\n");
        return 1;
    }
    return 0;
}


/******************************************************************************/
/* End of File: myecho.cpp                                                    */
/******************************************************************************/
