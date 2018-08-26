\function{socket}
\synopsis{Create a communications socket}
\usage{FD_Type socket (domain, type, protocol)}
\description
  The \ifun{socket} function creates a communications socket of the
  specified domain, type, and protocol.  Currently supported domains
  include \ivar{PF_UNIX} and \ivar{PF_INET}.  The various socket types
  may be specified by the symbolic constants
#v+
    SOCK_STREAM
    SOCK_DGRAM
    SOCK_SEQPACKET
    SOCK_RAW
    SOCK_RDM
#v-
  The \exmp{protocol} parameter specifies the protocol to be used.
  Normally only one protocol is support for a particular domain.  For
  such cases, 0 should be passed for the \exmp{protocol} parameter.

  If successful, the \ifun{socket} function will return a
  file-descriptor that may be used with the \ifun{read} and
  \ifun{write} function, or passed to other socket related functions.
  Upon error, a \ivar{SocketError} exception will be thrown and
  \ivar{errno} set accordingly.

  When finished with the socket, it should be passed to the
  \ifun{close} function.
\example
  The following example illustrates the creation of a socket for use
  in the internet domain:
#v+
     try {
        s = socket (PF_INET, SOCK_STREAM, 0);
     }
     catch SocketError: {
        () = fprintf (stderr, "socket failed: %s", errno_string(errno));
        exit (1);
     }
#v-
\seealso{accept, bind, connect, listen, setsockopt, getsockopt}
\done

\function{connect}
\synopsis{Make a connection to a socket}
\usage{connect (FD_Type s, address-args)}
\description
  The \ifun{connect} function may be used to connect a socket \exmp{s}
  to the address specified by the address-arguments.  The type and
  number of the address arguments must be consistent with the domain
  of the socket.  For example, if the socket is in the Unix domain
  (PF_UNIX), then a single string giving a filename must be passed as
  the address-argument.  Sockets in the internet domain (PF_INET) take two
  address arguments: a hostname and a port.

  Upon failure, the function may throw a \ivar{SocketError} exception and set
  \ivar{errno}, or throw a \ivar{SocketHError} and set \ivar{h_error}.
  It should be noted that \ivar{SocketHError} is a subclass of
  \ivar{SocketError}.
\example
  The following example creates an internet domain socket and connects
  it to port 32100 of a specified host:
#v+
     try {
        s = socket (PF_INET, SOCK_STREAM, 0);
        connect (s, "some.host.com", 32100);
     }
     catch SocketHError: {...}
     catch SocketError: {...}
#v-
\seealso{accept, bind, listen, socket, setsockopt, getsockopt}
\done

\function{bind}
\synopsis{Bind a local address-name to a socket}
\usage{bind (FD_Type s, address-args)}
\description
  The \ivar{bind} function may be used to assign a name to a specified
  socket.  The address-args parameters specify the name in a
  domain-specific manner.  For Unix domain sockets, the address is the
  name of a file.  For sockets in the internet domain, the address is
  given by a hostname and port number.

  Upon failure, the function will throw a \ivar{SocketError} or
  \ivar{SocketHError} exception.
\example
  The following example creates a \ivar{PF_UNIX} domain socket and
  binds it to \exmp{"/tmp/mysock"}:
#v+
    s = socket (PF_UNIX, SOCK_STREAM, 0);
    bind (s, "/tmp/mysock");
#v-

  The next example creates a \ivar{PF_INET} domain socket and binds it
  to port 32000 of the local host \exmp{my.host.com}:
#v+
    s = socket (PF_INET, SOCK_STREAM, 0);
    bind (s, "my.host.com", 32000);
#v-
\seealso{accept, connect, listen, socket, setsockopt, getsockopt}
\done

\function{accept}
\synopsis{Accept a connection on a socket}
\usage{FD_Type accept (FD_Type s [,&address-args...]}
\description
  The \ifun{accept} function accepts a connection on the specified
  socket and returns a new socket that may be used to communicate with
  the remote end.  It can optionally return the address of the remote
  end through reference arguments.

  Upon error, \ifun{accept} will throw a \ivar{SocketError} exception.
\example
  The following example accepts a remote connection on \ivar{PF_INET}
  socket and returns the hostname and port used by the connected party:
#v+
    s1 = accept (s, &hostip, &port);
    vmessage ("Accepted connection from %s on port %d", hostip, port);
#v-
\seealso{bind, connect, listen, socket, setsockopt, getsockopt}
\done

\function{listen}
\synopsis{Listen for connections on a socket}
\usage{listen (FD_Type s, Int_Type max_pending)}
\description
  The \ifun{listen} function may be used to wait for a connection to a
  socket \exmp{s}.  The second argument specified the maximum number
  of pending connections to allow before refusing more.  Upon error, a
  \ivar{SocketError} exception will be thrown.
\seealso{accept, bind, connect, socket, setsockopt, getsockopt}
\seealso{}
\done

\function{getsockopt}
\synopsis{Get a socket option}
\usage{option = getsockopt (FD_Type s, Int_Type level, Int_Type optname)}
\description
  The \ifun{getsockopt} function returns the value of the option
  specified by the integer \exmp{optname} residing at the specified
  level.  The actual object returned depends upon the option
  requested.  Upon error, a \ivar{SocketError} exception will be
  generated.
\example
  The following example returns the SO_LINGER option of a socket.  In
  this case, the value returned will be a structure:
#v+
     s = socket (PF_INET, SOCK_STREAM, 0);
     linger = getsockopt (s, SOL_SOCKET, SO_LINGER);
#v-
\seealso{accept, bind, connect, listen, socket, getsockopt}
\done

\function{setsockopt}
\synopsis{Set an option on a socket}
\usage{setsockopt (FD_Type s, Int_Type level, Int_Type optname, value)}
\description
  The \ifun{setsockopt} function sets the value of the option
  specified by the integer \exmp{optname} residing at the specified
  level.  The value of the last parameter will vary with the option to
  be set.  Upon error, a \ivar{SocketError} exception will be
  generated.
\example
  The following example sets the SO_LINGER option of a socket.  In
  this case, the value is a structure:
#v+
     s = socket (PF_INET, SOCK_STREAM, 0);
     linger = struct { l_onoff, l_linger };
     linger.l_onoff = 1; linger.l_linger = 10;
     setsockopt (s, SOL_SOCKET, SO_LINGER, linger);
#v-
\seealso{accept, bind, connect, listen, socket, getsockopt}
\done
