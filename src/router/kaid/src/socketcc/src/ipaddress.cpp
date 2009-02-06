/******************************************************************************/
/* File: ipaddress.cpp                                                        */
/******************************************************************************/
/* This source code file is the source file for the IPAddress class - this    */
/* class is used to provide transparent IPv4 and IPv6 address support for the */
/* Socket classes within the library.                                         */
/******************************************************************************/
/* Written By: Jason But                                                      */
/* Copyright:  CTIE - Monash University 2001-2003                             */
/*                                                                            */
/* Contributions:                                                             */
/* Andreas Almroth  - Support for Solaris OS.                                 */
/* Desmond Schmidt  - Support for MacOS X 1.                                  */
/* Daniel Grimm     - Testing for MacOS X Jaguar.                             */
/* Clayborne Taylor - Support for FreeBSD.                                    */
/*                                                                            */
/* Notes:                                                                     */
/*    Version 1.00 - Original Version of module.                              */
/*                                                                            */
/*    Version 1.10 - Now uses pthreadcc to implement the IPAddress Mutual     */
/*                   Exclusion.                                               */
/*                 - Lazy evaluation of pcHostName via reverse DNS lookup and */
/*                   pcStrAddress.  Evaluation performed only if the string   */
/*                   values are requested - major performance increase.       */
/*                 - New private member methods to perform lazy evaluation of */
/*                   pcHostName and pcStrAddress.                             */
/*                 - Added new member method to convert represented address   */
/*                   into either an IPv4 or IPv6 address.                     */
/*                                                                            */
/*    Version 1.11 - Support for Solaris, many thanks to Andreas Almroth for  */
/*                   his effort.                                              */
/*                 - Inclusion of macros for multi-platform support.          */
/*                 - Updated comments to reflect changes.                     */
/*                                                                            */
/*    Version 1.20 - Fixed lazy evaluation bug, bStrAddressUnresolved and     */
/*                   pcStrAddress incorrectly set when copying from an        */
/*                   IPAddress instance where this string has yet to be       */
/*                   evaluated.                                               */
/*                 - All errUnknown exceptions now include value of errno.    */
/*                 - pcHostName replaced with strHostName of type std::string */
/*                   This should fix a range of minor memory leakage problems */
/*                   as well as simplify the code somewhat.                   */
/*                                                                            */
/*    Version 1.30 - Support for MacOS, many thanks to Desmond Schmidt and    */
/*                   Daniel Grimm.                                            */
/*                 - Now freeing memory allocated to pheDetails if required.  */
/*                 - Added new method to IPAddress to return an unmapped      */
/*                   (not IPv6 Mapped IPv4) string representation of the IP   */
/*                   Address.                                                 */
/*                                                                            */
/*    Version 1.38 - Support for FreeBSD, many thanks to Clayborne Taylor.    */
/******************************************************************************/

/******************************************************************************/
/* Include C Socket Libraries.                                                */
/******************************************************************************/
#include <errno.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <string.h>

/******************************************************************************/
/* Include class header file.                                                 */
/******************************************************************************/
#include "socketcc.h"

/******************************************************************************/
/* Platform specific code.                                                    */
/*                                                                            */
/* These macros allow different implementations for functionality on the      */
/* supported platforms.  The macros define the following tasks, details on    */
/* specific implementations are detailed in the specific platform support     */
/* sections.                                                                  */
/*                                                                            */
/* CLASS_MUTEX_LOCK                                                           */
/*    Locks the class MutEx, evaluates to NULL if not required.               */
/* CLASS_MUTEX_UNLOCK                                                         */
/*    Unlocks the class MutEx, evaluates to NULL if not required.             */
/* IPADDR_TO_HOST(pheDetails, pcAddress, iAddressLength, iAddressType)        */
/*    Resolves the IP Address details of the provided binary IP address.  The */
/*    macro variables are:                                                    */
/*      pheDetails type (struct hostent *) - Host details returned in this    */
/*      structure.  If the name lookup is unresolved, must be set to NULL.    */
/*      pcAddress type (pointer) - Pointer to binary IP Address.              */
/*      iAddressLength type (int) - Size of binary IP Address.                */
/*      iAddressType type (int) - Request an AF_INET or AF_INET6 lookup.      */
/* IPNAME_TO_HOST(pheDetails, pcHostName, iAddressType, iError)               */
/*    Resolves the IP Address details of the provided host name.  The macro   */
/*    variables are:                                                          */
/*      pheDetails type (struct hostent *) - Host details returned in this    */
/*      structure.  If the address lookup is unresolved, must be set to NULL. */
/*      pcHostName type (char *) - String representation of the host name.    */
/*      iAddressType type (int) - Request an AF_INET or AF_INET6 lookup.      */
/*      iError type (int) - Error details in the operation are returned in    */
/*      this variable.                                                        */
/* FREE_HOSTENT(pheDetails0                                                   */
/*    Frees memory allocated to pheDetails if required.  Necessary if thread  */
/*    safe functions like getipnodebyaddr() and getipnodebyname() are used.   */
/******************************************************************************/
/* Linux                                                                      */
/* - The MutEx is required.                                                   */
/* - IPADDR_TO_HOST is implemented using the gethostbyaddr() function.        */
/* - IPNAME_TO_HOST is implemented using the gethostbyname2() function.       */
/* - FREE_HOSTENT is not required.                                            */
/******************************************************************************/
#if defined(PLATFORM_linux)

#define CLASS_MUTEX_LOCK        mutexKernelLock.Lock();
#define CLASS_MUTEX_UNLOCK      mutexKernelLock.Unlock();

#define IPADDR_TO_HOST(pheDetails, pcAddress, iAddressLength, iAddressType)   \
    {                                                                         \
        pheDetails = gethostbyaddr(pcAddress, iAddressLength, iAddressType);  \
    }

#define IPNAME_TO_HOST(pheDetails, pcHostName, iAddressType, iError)          \
    {                                                                         \
        pheDetails = gethostbyname2(pcHostName, iAddressType);                \
        iError = h_errno;                                                     \
    }

#define FREE_HOSTENT(pheDetails)

/******************************************************************************/
/* Solaris, FreeBSD                                                           */
/* - The MutEx is not required.                                               */
/* - IPADDR_TO_HOST is implemented using the getipnodebyaddr() function.      */
/* - IPNAME_TO_HOST is implemented using the getipnodebyname() function.      */
/* - FREE_HOSTENT is required.                                                */
/******************************************************************************/
#elif defined(PLATFORM_solaris) || defined(PLATFORM_freebsd)

#define CLASS_MUTEX_LOCK
#define CLASS_MUTEX_UNLOCK

#define IPADDR_TO_HOST(pheDetails, pcAddress, iAddressLength, iAddressType)   \
    {                                                                         \
        int rc = 0;                                                           \
        pheDetails = getipnodebyaddr(pcAddress, iAddressLength,               \
                                     iAddressType, &rc);                      \
    }

#define IPNAME_TO_HOST(pheDetails, pcHostName, iAddressType, iError)          \
    {                                                                         \
        int rc = 0;                                                           \
        pheDetails = getipnodebyname(pcHostName, iAddressType, 0, &rc);       \
        iError = rc;                                                          \
    }

#define FREE_HOSTENT(pheDetails)                                              \
    freehostent(pheDetails);

/******************************************************************************/
/* MacOS X Jaguar                                                             */
/* - The MutEx is not required.                                               */
/* - IPADDR_TO_HOST is implemented using the getipnodebyaddr() function.      */
/* - IPNAME_TO_HOST is implemented using the getipnodebyname() function.      */
/* - FREE_HOSTENT is required.                                                */
/******************************************************************************/
#elif defined(PLATFORM_macosx_jaguar)

#define CLASS_MUTEX_LOCK
#define CLASS_MUTEX_UNLOCK

#define IPADDR_TO_HOST(pheDetails, pcAddress, iAddressLength, iAddressType)   \
    {                                                                         \
        int rc = 0;                                                           \
        pheDetails = getipnodebyaddr(pcAddress, iAddressLength,               \
                                     iAddressType, &rc);                      \
    }

#define IPNAME_TO_HOST(pheDetails, pcHostName, iAddressType, iError)          \
    {                                                                         \
        int rc = 0;                                                           \
        pheDetails = getipnodebyname(pcHostName, iAddressType, 0, &rc);       \
        iError = rc;                                                          \
    }

#define FREE_HOSTENT(pheDetails)                                              \
    freehostent(pheDetails);

/******************************************************************************/
/* MacOS X.1.?                                                                */
/* - The MutEx is required.                                                   */
/* - IPADDR_TO_HOST is implemented using the gethostbyaddr() function.        */
/* - IPNAME_TO_HOST is implemented using the gethostbyname2() function.       */
/* - FREE_HOSTENT is not required.                                            */
/* - Function inet_ntop() is not defined in the header files and must be      */
/*   explicitly defined.  We also create sub-functions inet_ntop4() and       */
/*   inet_ntop6().  We do not need to check for buffer overflow as the calling*/
/*   code in IPAddress guarantees the string length is long enough to contain */
/*   the output string.                                                       */
/******************************************************************************/
#elif defined(PLATFORM_macosx_1)

#define CLASS_MUTEX_LOCK        mutexKernelLock.Lock();
#define CLASS_MUTEX_UNLOCK      mutexKernelLock.Unlock();

#define IPADDR_TO_HOST(pheDetails, pcAddress, iAddressLength, iAddressType)   \
    {                                                                         \
        pheDetails = gethostbyaddr(pcAddress, iAddressLength, iAddressType);  \
    }

#define IPNAME_TO_HOST(pheDetails, pcHostName, iAddressType, iError)          \
    {                                                                         \
        pheDetails = gethostbyname2(pcHostName, iAddressType);                \
        iError = h_errno;                                                     \
    }

#define FREE_HOSTENT(pheDetails)

const char *
inet_ntop6(const struct in6_addr *addr, char *dst, size_t size)
{
    const u_int16_t *puiAddress = addr->__u6_addr.__u6_addr16;
    int             iColon = 0;
    char            *pcBuffer = dst;
    bool            bProcessNumber;

    for (int iCount = 0; iCount < 8; iCount++, puiAddress++)
    {
        bProcessNumber = true;

        if (*puiAddress == 0)
        {
            switch (iColon)
            {
                case 0:     *pcBuffer++= ':';
                            iColon++;
                case 1:     bProcessNumber = false;
            }
        }
        if (bProcessNumber)
        {
            if (iCount) *pcBuffer++= ':';
            iColon++;
            sprintf(pcBuffer, "%x", *puiAddress);
            pcBuffer+= strlen(pcBuffer);
        }
    }

    *pcBuffer = 0;
    return pcBuffer;
}

const char *
inet_ntop4(const struct in_addr *addr, char *dst, size_t size)
{
    const u_int8_t  *pucAddress = (u_int8_t *) &(addr->s_addr);
    char            *pcBuffer = dst;

    for (int iCount = 0; iCount < 4; iCount++, pucAddress++)
    {
        if (iCount) *pcBuffer++= '.';
        sprintf(pcBuffer, "%d", *pucAddress);
        pcBuffer+= strlen(pcBuffer);
    }
    *pcBuffer = 0;
    return pcBuffer;
}

const char *
inet_ntop(int af, const void *addr, char *dst, size_t size)
{
    switch (af)
    {
        case AF_INET6:  return inet_ntop6((in6_addr *)addr, dst, size);
        case AF_INET:   return inet_ntop4((in_addr *)addr, dst, size);
    }
    return NULL;
}

#endif

/******************************************************************************/
/* class IPAddress.                                                           */
/******************************************************************************/
/* IPAddress Static Member Variables.                                         */
/*                                                                            */
/* mutexKernelLock : Locks access to the gethostbyname2() and gethostbyaddr() */
/*                   function calls.  These functions and the results that    */
/*                   they return are non-reentrant.  Code that calls these    */
/*                   functions and then processes their return values forms a */
/*                   critical section.  This MutualExclusion ensures that     */
/*                   only one thread can run this code at any time.  This     */
/*                   member variable is always defined but not used on some   */
/*                   platforms.                                               */
/******************************************************************************/
MutualExclusion     IPAddress::mutexKernelLock;

/******************************************************************************/
/* IPAddress Constructor.                                                     */
/*                                                                            */
/* The default constructor assigns the localhost IPv4 address to the class.   */
/* The Address type and length are set, memory is allocated for the host name */
/* string.  Finally the address itself is filled with the correct values and  */
/* the string representation of the address is filled.  The 2 lazy evaluation */
/* flags are reset as both pcHostName and pcStrAddress are valid.             */
/******************************************************************************/
IPAddress::IPAddress()
{
    try
    {
        iAddressType = AF_INET;

        strHostName = "localhost";

        iAddressLength = 4;
        pcAddress[0] = 127;
        pcAddress[1] = pcAddress[2] = 0;
        pcAddress[3] = 1;

        strcpy(pcStrAddress, "127.0.0.1");

        bHostNameUnresolved = bStrAddressUnresolved = false;
    }
    catch (...)
    {
        throw SocketException(SocketException::errMemory);
    }
}

/******************************************************************************/
/* void privSetAddress(const char *pcNewAddress, const int iNewAddressLength) */
/*                                                                            */
/* This private method is used to set the contents of the iAddressLength and  */
/* pcAddress member variables.  Address Length bytes are copied from the      */
/* source address to the pcAddress array.  This array is 16 bytes long, long  */
/* enough to hold both IPv4 and IPv6 addresses.                               */
/******************************************************************************/
void
IPAddress::privSetAddress(const char *pcNewAddress, const int iNewAddressLength)
{
    iAddressLength = iNewAddressLength;
    memcpy(pcAddress, pcNewAddress, iAddressLength);
}

/******************************************************************************/
/* void privResolveHostName()                                                 */
/*                                                                            */
/* This private method is used to resolve the hostname using a reverse DNS    */
/* lookup, this need be performed only if the lazy evaluation flag is set.    */
/* The flag is reset at completion of the method to indicate that the host    */
/* name has been resolved.  The reverse DNS lookup is performed via the       */
/* IPADDR_TO_HOST() macro.  The MutEx LOCK and UNLOCK macros protect a        */
/* potential critical section if required, platforms that do not require the  */
/* protection will evaluate these macros to NULL.  If the IP details lookup   */
/* fails, pheDetails is set to NULL and we cannot obtain the host name, we    */
/* instead use the string representation of the ip address as a hostname -    */
/* calling privResolveStrAddress() first to force the evaluation of this      */
/* string.  Otherwise we set the hostname from the values returned by the     */
/* function call.                                                             */
/******************************************************************************/
void
IPAddress::privResolveHostName()
{
    if (bHostNameUnresolved)
    {
        struct hostent  *pheDetails;

        CLASS_MUTEX_LOCK

        IPADDR_TO_HOST(pheDetails, pcAddress, iAddressLength, iAddressType)
        if (pheDetails == NULL)
        {
            privResolveStrAddress();
            strHostName = pcStrAddress;
        } else
        {
            strHostName = pheDetails->h_name;
        }
        FREE_HOSTENT(pheDetails)

        bHostNameUnresolved = false;

        CLASS_MUTEX_UNLOCK
    }
}

/******************************************************************************/
/* void privResolveStrAddress()                                               */
/*                                                                            */
/* This private method is used to resolve the string representation of the ip */
/* address stored by the class,  this need be performed only if the lazy      */
/* evaluation flag is set.  The flag is reset at completion of the method to  */
/* indicate that the address string has been resolved - the string is         */
/* generated via a call to inet_ntop().                                       */
/******************************************************************************/
void
IPAddress::privResolveStrAddress()
{
    if (bStrAddressUnresolved)
    {
        inet_ntop(iAddressType, pcAddress, pcStrAddress, INET6_ADDRSTRLEN);
        bStrAddressUnresolved = false;
    }
}

/******************************************************************************/
/* Assignment Operator Overloads.                                             */
/******************************************************************************/
/* Assign equal to existing IPAddress class.                                  */
/*                                                                            */
/* Allows us to assign equality to an existing IPAddress class.  We copy over */
/* values of the variables iAddressType, strHostName and pcStrAddress stored  */
/* within the original class.  The private method privSetAddress() is used to */
/* set the other internal member variables.  The bStrAddressUnresolved flag   */
/* and bHostNameUnresolved flag must be directly copied from the original     */
/* class.  If pcStrAddress is resolved, we also copy that string.             */
/******************************************************************************/
IPAddress &
IPAddress::operator=(const IPAddress &cOrigAddr)
{
    iAddressType = cOrigAddr.iAddressType;
    strHostName = cOrigAddr.strHostName;
    privSetAddress(cOrigAddr.pcAddress, cOrigAddr.iAddressLength);
    bHostNameUnresolved = cOrigAddr.bHostNameUnresolved;
    if (!(bStrAddressUnresolved = cOrigAddr.bStrAddressUnresolved))
    {
        strcpy(pcStrAddress, cOrigAddr.pcStrAddress);
    }
    return *this;
}

/******************************************************************************/
/* Assign equal to host name.                                                 */
/*                                                                            */
/* Allows us to assign equality to an existing string host name.  The         */
/* SetHostName() method is called to set the IPv4 address for the provided    */
/* Host Name.  The previous value is unchanged if SetHostName() fails.        */
/******************************************************************************/
IPAddress &
IPAddress::operator=(const char *pcNewHostName)
{
    SetHostName(pcNewHostName, false);
    return *this;
}

/******************************************************************************/
/* Assign equal to IPv4 Address Structure.                                    */
/*                                                                            */
/* Allows as to assign equality to an existing IPv4 in_addr structure.  The   */
/* SetAddress() method is called with a pointer to the structure cast as a    */
/* char pointer to complete the conversion.                                   */
/******************************************************************************/
IPAddress &
IPAddress::operator=(const in_addr sIP4Addr)
{
    SetAddress((char *)&sIP4Addr, false);
    return *this;
}

/******************************************************************************/
/* Assign equal to IPv6 Address Structure.                                    */
/*                                                                            */
/* Allows as to assign equality to an existing IPv6 in6_addr structure.  The  */
/* SetAddress() method is called with a pointer to the structure cast as a    */
/* char pointer to complete the conversion.                                   */
/******************************************************************************/
IPAddress &
IPAddress::operator=(const in6_addr sIP6Addr)
{
    SetAddress((char *)&sIP6Addr, true);
    return *this;
}

/******************************************************************************/
/* Equality Operator Overloads.                                               */
/******************************************************************************/
/* Check equality with an existing IPAddress class.                           */
/*                                                                            */
/* Allows us to check equality with another IPAddress class.  Equality exists */
/* if the both address types and lengths are equal and the contents of the    */
/* pcAddress arrays are equal up to the correct length.  We return any        */
/* positive results on all three of these tests.                              */
/******************************************************************************/
bool
IPAddress::operator==(const IPAddress &cOtherAddr) const
{
    return ((iAddressType == cOtherAddr.iAddressType) &&
            (iAddressLength == cOtherAddr.iAddressLength) &&
            (bcmp(pcAddress, cOtherAddr.pcAddress, iAddressLength) == 0));
}

/******************************************************************************/
/* Check equality with a host name.                                           */
/*                                                                            */
/* Allows us to check equality with an existing string hostname.  A temporary */
/* IPAddress class is created with the given hostname and the same type as    */
/* the existing IPAddress class.  The two class instances are then compared   */
/* and the result is returned.                                                */
/******************************************************************************/
bool
IPAddress::operator==(const char *pcNewHostName) const
{
    IPAddress   cCheckAddr;

    cCheckAddr.SetHostName(pcNewHostName, (iAddressType == AF_INET6));
    return ((*this) == cCheckAddr);
}

/******************************************************************************/
/* Check equality with an IPv4 Address Structure.                             */
/*                                                                            */
/* Allows as to check equality to an existing IPv4 in_addr structure.  A      */
/* temporary IPAddress class is created with the given IPv4 address.  This is */
/* then compared with the existing IPAddress class and the result is returned.*/
/******************************************************************************/
bool
IPAddress::operator==(const in_addr sIP4Addr) const
{
    IPAddress   cCheckAddr;

    cCheckAddr = sIP4Addr;
    return ((*this) == cCheckAddr);
}

/******************************************************************************/
/* Check equality with an IPv6 Address Structure.                             */
/*                                                                            */
/* Allows as to check equality to an existing IPv6 in6_addr structure.  A     */
/* temporary IPAddress class is created with the given IPv6 address.  This is */
/* then compared with the existing IPAddress class and the result is returned.*/
/******************************************************************************/
bool
IPAddress::operator==(const in6_addr sIP6Addr) const
{
    IPAddress   cCheckAddr;

    cCheckAddr = sIP6Addr;
    return ((*this) == cCheckAddr);
}

/******************************************************************************/
/* bool SetHostName(const char *pcNewHostName, bool bIPv6)                    */
/*                                                                            */
/* This method is called to set the IPAddress class contents to the address   */
/* specified by the provided hostname.  An extra parameter is used to allow   */
/* specification of an IPv4 or IPv6 address.  This is performed using the     */
/* IPNAME_TO_HOST macro along with the MutEx LOCK and UNLOCK macros which     */
/* protect the potential critical section as in privResolveHostName().  There */
/* two calls to the CLASS_MUTEX_UNLOCK macro, at both points where the method */
/* can terminate.  If the lookup fails, then we cannot obtain the IP address  */
/* of the host name, no internal variables are set - (the class contains the  */
/* old address still), and an exception is thrown.  If the call succeeds,     */
/* internal variables are set based on the results.                           */
/******************************************************************************/
void
IPAddress::SetHostName(const char *pcNewHostName, bool bIPv6)
{
    struct hostent  *pheDetails;
    int             iError;

    CLASS_MUTEX_LOCK

    IPNAME_TO_HOST(pheDetails, pcNewHostName, ((bIPv6)?AF_INET6:AF_INET), iError)
    if (pheDetails == NULL)
    {
        CLASS_MUTEX_UNLOCK
        switch (iError)
        {
            case HOST_NOT_FOUND: throw SocketException(SocketException::errBadHostName);
            case NO_ADDRESS:     throw SocketException(SocketException::errNoIPAddress);
            case NO_RECOVERY:
            case TRY_AGAIN:      throw SocketException(SocketException::errDNSError);
            default:             throw SocketException(SocketException::errUnknown, iError);
        }
    }

    iAddressType = pheDetails->h_addrtype;

    strHostName = pheDetails->h_name;
    privSetAddress(pheDetails->h_addr_list[0], pheDetails->h_length);

    bHostNameUnresolved = false;
    bStrAddressUnresolved = true;

    FREE_HOSTENT(pheDetails)
    
    CLASS_MUTEX_UNLOCK
}

/******************************************************************************/
/* void SetAddress(const char *pcNewAddress, bool bIPv6)                      */
/*                                                                            */
/* This method is called to set the IPAddress class contents to the address   */
/* specified, an extra parameter is used to allow specification of an IPv4 or */
/* IPv6 address.  The internal variables of the class are set as a copy of    */
/* the provided information, however to increase the speed of execution, the  */
/* value of pcHostName and pcStrAddress are set via lazy evaluation - this is */
/* done be setting the two lazy evaluation flags.                             */
/******************************************************************************/
void
IPAddress::SetAddress(const char *pcNewAddress, bool bIPv6)
{
    iAddressType = (bIPv6)?AF_INET6:AF_INET;
    privSetAddress(pcNewAddress, (bIPv6)?sizeof(in6_addr):sizeof(in_addr));

    bHostNameUnresolved = bStrAddressUnresolved = true;
}

/******************************************************************************/
/* void ConvertToAddressFamily(int iNewAddressFamily)                         */
/*                                                                            */
/* This method converts the IPAddress instance to the specified type - either */
/* AF_INET or AF_INET6.  If the address family is already of the specified    */
/* type, then no changes are made.  The following steps are for converting to:*/
/*                                                                            */
/* IPv4: If the existing IPv6 address is not an IPv4 Mapped IPv6 address the  */
/*       conversion cannot take place and an exception is thrown.  Otherwise, */
/*       the last 32 bits of the IPv6 address form the IPv4 address and we    */
/*       call privSetAddress() to set the address to these four bytes.        */
/* IPv6: The 32 bits of the IPv4 address are copied to the last 32 bits of    */
/*       the 128-bit IPv address.  This is then prepended with 80 zero bits   */
/*       and 16 one bits to form an IPv4 Mapped IPv6 address.                 */
/*                                                                            */
/* An exception is thrown for an unknown family type being specified.         */
/* Finally, the new address family is set along with both lazy evaluation     */
/* flags.                                                                     */
/******************************************************************************/
void
IPAddress::ConvertToAddressFamily(int iNewAddressFamily)
{
    if (iAddressType == iNewAddressFamily) return;

    switch (iNewAddressFamily)
    {
        case AF_INET:   if (!(IN6_IS_ADDR_V4MAPPED((struct in6_addr *)pcAddress)))
                        {
                            throw SocketException(SocketException::errCannotConvertToIPv4);
                        }
                        privSetAddress(pcAddress + 12, sizeof(in_addr));
                        break;
        case AF_INET6:  iAddressLength = sizeof(in6_addr);
                        memcpy(pcAddress + 12, pcAddress, 4);
                        bzero(pcAddress, 10);
                        memset(pcAddress + 10, 0xff, 2);
                        break;
        default:        throw SocketException(SocketException::errNoProtocolSupport);
    }

    iAddressType = iNewAddressFamily;
    bHostNameUnresolved = bStrAddressUnresolved = true;
}

/******************************************************************************/
/* const char * GetUnmappedAddressString()                                    */
/*                                                                            */
/* this method returns a visual representation of the contained IP Address.   */
/* It is similar to the GetAddressString() method with one minor difference,  */
/* if the address is an IPv6 Mapped IPv4 Address, the IPv4 representation     */
/* string only is returned.  This string begins at the seventh character in   */
/* in the Address String.  First we ensure that the string has been resolved  */
/* before returning either the base pcStrAddress pointer or an offset based   */
/* on the type of the contained address.                                      */
/******************************************************************************/
const char *
IPAddress::GetUnmappedAddressString()
{
    privResolveStrAddress();

    return ((iAddressType == AF_INET6) && (IN6_IS_ADDR_V4MAPPED((struct in6_addr *)pcAddress)))?(pcStrAddress + 7):(pcStrAddress);
}

/******************************************************************************/
/* End of File: ipaddress.cpp                                                 */
/******************************************************************************/
