.. _kvno(1):

kvno
====

SYNOPSIS
--------

**kvno**
[**-c** *ccache*]
[**-e** *etype*]
[**-q**]
[**-h**]
[**-P**]
[**-S** *sname*]
[**-U** *for_user*]
[**--u2u** *ccache*]
*service1 service2* ...


DESCRIPTION
-----------

kvno acquires a service ticket for the specified Kerberos principals
and prints out the key version numbers of each.


OPTIONS
-------

**-c** *ccache*
    Specifies the name of a credentials cache to use (if not the
    default)

**-e** *etype*
    Specifies the enctype which will be requested for the session key
    of all the services named on the command line.  This is useful in
    certain backward compatibility situations.

**-q**
    Suppress printing output when successful.  If a service ticket
    cannot be obtained, an error message will still be printed and
    kvno will exit with nonzero status.

**-h**
    Prints a usage statement and exits.

**-P**
    Specifies that the *service1 service2* ...  arguments are to be
    treated as services for which credentials should be acquired using
    constrained delegation.  This option is only valid when used in
    conjunction with protocol transition.

**-S** *sname*
    Specifies that the *service1 service2* ... arguments are
    interpreted as hostnames, and the service principals are to be
    constructed from those hostnames and the service name *sname*.
    The service hostnames will be canonicalized according to the usual
    rules for constructing service principals.

**-U** *for_user*
    Specifies that protocol transition (S4U2Self) is to be used to
    acquire a ticket on behalf of *for_user*.  If constrained
    delegation is not requested, the service name must match the
    credentials cache client principal.

**--u2u** *ccache*
    Requests a user-to-user ticket.  *ccache* must contain a local
    krbtgt ticket for the server principal.  The reported version
    number will typically be 0, as the resulting ticket is not
    encrypted in the server's long-term key.

ENVIRONMENT
-----------

See :ref:`kerberos(7)` for a description of Kerberos environment
variables.


FILES
-----

|ccache|
    Default location of the credentials cache


SEE ALSO
--------

:ref:`kinit(1)`, :ref:`kdestroy(1)`, :ref:`kerberos(7)`
