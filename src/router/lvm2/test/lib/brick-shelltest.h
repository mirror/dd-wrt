// -*- mode: C++; indent-tabs-mode: nil; c-basic-offset: 4 -*-

/*
 * This brick allows you to build a test runner for shell-based functional
 * tests. It comes with fairly elaborate features (although most are only
 * available on posix systems), geared toward difficult-to-test software.
 *
 * It provides a full-featured "main" function (brick::shelltest::run) that you
 * can use as a drop-in shell test runner.
 *
 * Features include:
 * - interactive and batch-mode execution
 * - collects test results and test logs in a simple text-based format
 * - measures resource use of individual tests
 * - rugged: suited for running in monitored virtual machines
 * - supports test flavouring
 */

/*
 * (c) 2014 Petr Rockai <me@mornfall.net>
 * (c) 2014 Red Hat, Inc.
 */

/* Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE. */

#include <fcntl.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <errno.h>

#include <vector>
#include <map>
#include <deque>
#include <string>
#include <iostream>
#include <iomanip>
#include <fstream>
#include <sstream>
#include <cassert>
#include <iterator>
#include <algorithm>
#include <stdexcept>

#include <dirent.h>

#ifdef __unix
#include <sys/stat.h>
#include <sys/resource.h> /* rusage */
#include <sys/select.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/klog.h>
#include <sys/utsname.h>
#include <time.h>
#include <unistd.h>
#endif

#include "configure.h"

/*  Timeout for the whole test suite in hours */
static const time_t TEST_SUITE_TIMEOUT = 4;

#ifndef BRICK_SHELLTEST_H
#define BRICK_SHELLTEST_H

namespace brick {
namespace shelltest {

/* TODO: remove this section in favour of brick-filesystem.h */

inline std::runtime_error syserr( std::string msg, std::string ctx = "" ) {
    return std::runtime_error( std::string( strerror( errno ) ) + " " + msg + " " + ctx );
}

struct dir {
    DIR *d;
    dir( std::string p ) {
        d = opendir( p.c_str() );
        if ( !d )
            throw syserr( "error opening directory", p );
    }
    ~dir() { closedir( d ); }
};

typedef std::vector< std::string > Listing;

inline void fsync_name( std::string n )
{
    int fd = open( n.c_str(), O_WRONLY );
    if ( fd >= 0 ) {
        fsync( fd );
        close( fd );
    }
}

inline Listing listdir( std::string p, bool recurse = false, std::string prefix = "" )
{
    Listing r;

    dir d( p );
#if !defined(__GLIBC__) || (__GLIBC__ < 2) || ((__GLIBC__ == 2) && (__GLIBC_MINOR__ < 23))
    /* readdir_r is deprecated with newer GLIBC */
    struct dirent entry, *iter = 0;
    while ( (errno = readdir_r( d.d, &entry, &iter )) == 0 && iter ) {
        std::string ename( entry.d_name );
#else
    struct dirent *entry;
    errno = 0;
    while ( (entry = readdir( d.d )) ) {
        std::string ename( entry->d_name );
#endif

        if ( ename == "." || ename == ".." )
            continue;

        if ( recurse ) {
            struct stat64 stat;
            std::string s = p + "/" + ename;
            if ( ::stat64( s.c_str(), &stat ) == -1 ) {
                errno = 0;
                continue;
            }
            if ( S_ISDIR(stat.st_mode) ) {
                Listing sl = listdir( s, true, prefix + ename + "/" );
                for ( Listing::iterator i = sl.begin(); i != sl.end(); ++i )
                    r.push_back( prefix + *i );
            } else
                r.push_back( prefix + ename );
        } else
            r.push_back( ename );
    };

    if ( errno != 0 )
        throw syserr( "error reading directory", p );

    return r;
}

/* END remove this section */

struct Journal {
    enum R {
        STARTED,
        RETRIED,
        UNKNOWN,
        FAILED,
        INTERRUPTED,
        KNOWNFAIL,
        PASSED,
        SKIPPED,
        TIMEOUT,
        WARNED,
    };

    friend std::ostream &operator<<( std::ostream &o, R r ) {
        switch ( r ) {
            case STARTED: return o << "started";
            case RETRIED: return o << "retried";
            case FAILED: return o << "failed";
            case INTERRUPTED: return o << "interrupted";
            case PASSED: return o << "passed";
            case SKIPPED: return o << "skipped";
            case TIMEOUT: return o << "timeout";
            case WARNED: return o << "warnings";
            default: return o << "unknown";
        }
    }

    friend std::istream &operator>>( std::istream &i, R &r ) {
        std::string x;
        i >> x;

        r = UNKNOWN;
        if ( x == "started" ) r = STARTED;
        if ( x == "retried" ) r = RETRIED;
        if ( x == "failed" ) r = FAILED;
        if ( x == "interrupted" ) r = INTERRUPTED;
        if ( x == "passed" ) r = PASSED;
        if ( x == "skipped" ) r = SKIPPED;
        if ( x == "timeout" ) r = TIMEOUT;
        if ( x == "warnings" ) r = WARNED;
        return i;
    }

    template< typename S, typename T >
    friend std::istream &operator>>( std::istream &i, std::pair< S, T > &r ) {
        return i >> r.first >> r.second;
    }

    typedef std::map< std::string, R > Status;
    Status status, written;

    std::string location, list;
    int timeouts;

    void append( std::string path ) {
        std::ofstream of( path.c_str(), std::fstream::app );
        Status::iterator writ;
        for ( Status::iterator i = status.begin(); i != status.end(); ++i ) {
            writ = written.find( i->first );
            if ( writ == written.end() || writ->second != i->second )
                of << i->first << " " << i->second << std::endl;
        }
        written = status;
        of.close();
    }

    void write( std::string path ) {
        std::ofstream of( path.c_str() );
        for ( Status::iterator i = status.begin(); i != status.end(); ++i )
            of << i->first << " " << i->second << std::endl;
        of.close();
    }

    void sync() {
        append( location );
        fsync_name( location );
        write ( list );
        fsync_name( list );
    }

    void started( std::string n ) {
        if ( status.count( n ) && status[ n ] == STARTED )
            status[ n ] = RETRIED;
        else
            status[ n ] = STARTED;
        sync();
    }

    void done( std::string n, R r ) {
        status[ n ] = r;
        if ( r == TIMEOUT )
            ++ timeouts;
        else
            timeouts = 0;
        sync();
    }

    bool done( std::string n ) {
        if ( !status.count( n ) )
            return false;
        return status[ n ] != STARTED && status[ n ] != INTERRUPTED;
    }

    int count( R r ) {
        int c = 0;
        for ( Status::iterator i = status.begin(); i != status.end(); ++i )
            if ( i->second == r )
                ++ c;
        return c;
    }

    void banner() {
        std::cout << std::endl << "### " << status.size() << " tests: "
                  << count( PASSED ) << " passed, "
                  << count( SKIPPED ) << " skipped, "
                  << count( TIMEOUT ) << " timed out, " << count( WARNED ) << " warned, "
                  << count( FAILED ) << " failed" << std::endl;
    }

    void details() {
        for ( Status::iterator i = status.begin(); i != status.end(); ++i )
            if ( i->second != PASSED )
                std::cout << i->second << ": " << i->first << std::endl;
    }

    void read( std::string n ) {
        std::ifstream ifs( n.c_str() );
        typedef std::istream_iterator< std::pair< std::string, R > > It;
        for ( It i( ifs ); i != It(); ++i )
            status[ i->first ] = i->second;
    }

    void read() { read( location ); }

    Journal( std::string dir )
        : location( dir + "/journal" ),
          list( dir + "/list" ),
          timeouts( 0 )
    {}
};

struct TimedBuffer {
    typedef std::pair< time_t, std::string > Line;

    std::deque< Line > data;
    Line incomplete;
    bool stamp;

    Line shift( bool force = false ) {
        Line result = std::make_pair( 0, "" );
        if ( force && data.empty() )
            std::swap( result, incomplete );
        else {
            result = data.front();
            data.pop_front();
        }
        return result;
    }

    void push( std::string buf ) {
        time_t now = stamp ? time( 0 ) : 0;
        std::string::iterator b = buf.begin(), e = buf.begin();

        while ( e != buf.end() )
        {
            e = std::find( b, buf.end(), '\n' );
            incomplete.second += std::string( b, e );

            if ( !incomplete.first )
                incomplete.first = now;

            if ( e != buf.end() ) {
                incomplete.second += "\n";
                data.push_back( incomplete );
                if (incomplete.second[0] == '#') {
                    /* Disable timing between '## 0 STACKTRACE' & '## teardown' keywords */
                    if (incomplete.second.find("# 0 STACKTRACE", 1) != std::string::npos ||
                        incomplete.second.find("# timing off", 1) != std::string::npos) {
                        stamp = false;
                        now = 0;
                    } else if (incomplete.second.find("# teardown", 1) != std::string::npos ||
                               incomplete.second.find("# timing on", 1) != std::string::npos) {
                        stamp = true;
                        now = time( 0 );
                    }
                }
                incomplete = std::make_pair( now, "" );
            }
            b = (e == buf.end() ? e : e + 1);
        }
    }

    bool empty( bool force = false ) {
        if ( force && !incomplete.second.empty() )
            return false;
        return data.empty();
    }

    TimedBuffer() : stamp(true) {}
};

struct Sink {
    virtual void outline( bool ) {}
    virtual void push( std::string x ) = 0;
    virtual void sync( bool ) {}
    virtual ~Sink() {}
};

struct Substitute {
    typedef std::map< std::string, std::string > Map;
    std::string testdir; // replace testdir first
    std::string prefix;

    std::string map( std::string line ) {
        if ( std::string( line, 0, 9 ) == "@TESTDIR=" )
            testdir = std::string( line, 9, line.length() - 10 ); // skip \n
        else if ( std::string( line, 0, 8 ) == "@PREFIX=" )
            prefix = std::string( line, 8, line.length() - 9 );  // skip \n
        else {
            size_t off;
            if (!testdir.empty())
                while ( (off = line.find( testdir )) != std::string::npos )
                    line.replace( off, testdir.length(), "@TESTDIR@" );

            if (!prefix.empty())
                while ( (off = line.find( prefix )) != std::string::npos )
                    line.replace( off, prefix.length(), "@PREFIX@" );
        }
        return line;
    }
};

struct Format {
    time_t start;
    Substitute subst;

    std::string format( TimedBuffer::Line l ) {
        std::stringstream result;
        if ( l.first >= start ) {
            time_t rel = l.first - start;
            result << "[" << std::setw( 2 ) << std::setfill( ' ' ) << rel / 60
                   << ":" << std::setw( 2 ) << std::setfill( '0' ) << rel % 60 << "] ";
        }
        result << subst.map( l.second );
        return result.str();
    }

    Format() : start( time( 0 ) ) {}
};

struct BufSink : Sink {
    TimedBuffer data;
    Format fmt;

    virtual void push( std::string x ) {
        data.push( x );
    }

    void dump( std::ostream &o ) {
        o << std::endl;
        while ( !data.empty( true ) )
            o << "| " << fmt.format( data.shift( true ) );
    }
};

struct FdSink : Sink {
    int fd;

    TimedBuffer stream;
    Format fmt;
    bool killed;

    virtual void outline( bool force )
    {
        TimedBuffer::Line line = stream.shift( force );
        std::string out = fmt.format( line );
        write( fd, out.c_str(), out.length() );
    }

    virtual void sync( bool force ) {
        if ( killed )
            return;
        while ( !stream.empty( force ) )
            outline( force );
    }

    virtual void push( std::string x ) {
        if ( !killed )
            stream.push( x );
    }

    FdSink( int _fd ) : fd( _fd ), killed( false ) {}
};

struct FileSink : FdSink {
    std::string file;
    FileSink( std::string n ) : FdSink( -1 ), file( n ) {}

    void sync( bool force ) {
        if ( fd < 0 && !killed ) {
#ifdef O_CLOEXEC
            fd = open( file.c_str(), O_WRONLY | O_CREAT | O_TRUNC | O_CLOEXEC, 0644 );
#else
            fd = open( file.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0644 );
            if ( fcntl( fd, F_SETFD, FD_CLOEXEC ) < 0 )
                perror("failed to set FD_CLOEXEC on file");
#endif
            if ( fd < 0 )
                killed = true;
        }
        FdSink::sync( force );
    }

    ~FileSink() {
        if ( fd >= 0 ) {
            fsync( fd );
            close( fd );
        }
    }
};

#define BRICK_SYSLOG_ACTION_READ           2
#define BRICK_SYSLOG_ACTION_READ_ALL       3
#define BRICK_SYSLOG_ACTION_READ_CLEAR     4
#define BRICK_SYSLOG_ACTION_CLEAR          5
#define BRICK_SYSLOG_ACTION_SIZE_UNREAD    9
#define BRICK_SYSLOG_ACTION_SIZE_BUFFER   10

struct Source {
    int fd;

    virtual void sync( Sink *sink ) {
        ssize_t sz;
        char buf[ 128 * 1024 ];
        if ( (sz = read(fd, buf, sizeof(buf) - 1)) > 0 )
            sink->push( std::string( buf, sz ) );

        /*
         * On RHEL5 box this code busy-loops here, while
         * parent process no longer writes anything.
         *
         * Unclear why 'select()' is anouncing available
         * data, while we read 0 bytes with errno == 0.
         *
         * Temporarily resolved with usleep() instead of loop.
         */
        if (!sz && (!errno || errno == EINTR))
            usleep(50000);

        if ( sz < 0 && errno != EAGAIN )
            throw syserr( "reading pipe" );
    }

    virtual void reset() {}

    virtual int fd_set_( fd_set *set ) {
        if ( fd >= 0 ) {
            FD_SET( fd, set );
            return fd;
        } else
            return -1;
    }

    Source( int _fd = -1 ) : fd( _fd ) {}
    virtual ~Source() {
        if ( fd >= 0 )
            ::close( fd );
    }
};

struct FileSource : Source {
    std::string file;
    FileSource( std::string n ) : Source( -1 ), file( n ) {}

    int fd_set_( ::fd_set * ) { return -1; } /* reading a file is always non-blocking */
    void sync( Sink *s ) {
        if ( fd < 0 ) {
#ifdef O_CLOEXEC
            fd = open( file.c_str(), O_RDONLY | O_CLOEXEC | O_NONBLOCK );
#else
            fd = open( file.c_str(), O_RDONLY | O_NONBLOCK );
            if ( fcntl( fd, F_SETFD, FD_CLOEXEC ) < 0 )
                perror("failed to set FD_CLOEXEC on file");
#endif
            if ( fd >= 0 )
                lseek( fd, 0, SEEK_END );
        }
        if ( fd >= 0 )
            Source::sync( s );
    }
};

struct KMsg : Source {
    bool can_clear;
    ssize_t buffer_size;

    KMsg() : can_clear( strcmp(getenv("LVM_TEST_CAN_CLOBBER_DMESG") ? : "0", "0") ),
        buffer_size(128 * 1024)
    {
#ifdef __unix
        struct utsname uts;
        unsigned kmaj, kmin, krel;
        const char *read_msg = "/dev/kmsg";

        // Can't use kmsg on kernels pre 3.5, read /var/log/messages
        if ( ( ::uname(&uts) == 0 ) &&
            ( ::sscanf( uts.release, "%u.%u.%u", &kmaj, &kmin, &krel ) == 3 ) &&
            ( ( kmaj < 3 ) || ( ( kmaj == 3 ) && ( kmin < 5 ) ) ) )
            can_clear = false, read_msg = "/var/log/messages";

        if ( ( fd = open(read_msg, O_RDONLY | O_NONBLOCK)) < 0 ) {
            if ( errno != ENOENT ) /* Older kernels (<3.5) do not support /dev/kmsg */
                fprintf( stderr, "open log %s %s\n", read_msg, strerror( errno ) );
            if ( can_clear && ( klogctl( BRICK_SYSLOG_ACTION_CLEAR, 0, 0 ) < 0 ) )
                can_clear = false;
        } else if ( lseek( fd, 0L, SEEK_END ) == (off_t) -1 ) {
            fprintf( stderr, "lseek log %s %s\n", read_msg, strerror( errno ) );
            close(fd);
            fd = -1;
        }
#endif
    }

    bool dev_kmsg() {
        return fd >= 0;
    }

    void sync( Sink *s ) {
#ifdef __unix
        ssize_t sz;
        char buf[ buffer_size ];

        if ( dev_kmsg() ) {
            while ( (sz = ::read(fd, buf, buffer_size)) > 0 )
                s->push( std::string( buf, sz ) );
        } else if ( can_clear ) {
            while ( ( sz = klogctl( BRICK_SYSLOG_ACTION_READ_CLEAR, buf,
                                   ( int) buffer_size ) ) > 0 )
                s->push( std::string( buf, sz ) );
            if ( sz < 0 && errno == EPERM )
                can_clear = false;
        }
#endif
    }
};

struct Observer : Sink {
    TimedBuffer stream;

    bool warnings;
    Observer() : warnings( false ) {}

    void push( std::string s ) {
        stream.push( s );
    }

    void sync( bool force ) {
        while ( !stream.empty( force ) ) {
            TimedBuffer::Line line = stream.shift( force );
            if ( line.second.find( "TEST WARNING" ) != std::string::npos )
                warnings = true;
        }
    }
};

struct IO : Sink {
    typedef std::vector< Sink* > Sinks;
    typedef std::vector< Source* > Sources;

    mutable Sinks sinks;
    mutable Sources sources;

    Observer *_observer;

    virtual void push( std::string x ) {
        for ( Sinks::iterator i = sinks.begin(); i != sinks.end(); ++i )
            (*i)->push( x );
    }

    void sync( bool force ) {
        for ( Sources::iterator i = sources.begin(); i != sources.end(); ++i )
            (*i)->sync( this );

        for ( Sinks::iterator i = sinks.begin(); i != sinks.end(); ++i )
            (*i)->sync( force );
    }

    void close() {
        for ( Sources::iterator i = sources.begin(); i != sources.end(); ++i )
            delete *i;
        sources.clear();
    }

    int fd_set_( fd_set *set ) {
        int max = -1;

        for ( Sources::iterator i = sources.begin(); i != sources.end(); ++i )
            max = std::max( (*i)->fd_set_( set ), max );
        return max + 1;
    }

    Observer &observer() { return *_observer; }

    IO() {
        clear();
    }

    /* a stealing copy constructor */
    IO( const IO &io ) : sinks( io.sinks ), sources( io.sources ), _observer( io._observer )
    {
        io.sinks.clear();
        io.sources.clear();
    }

    IO &operator= ( const IO &io ) {
        this->~IO();
        return *new (this) IO( io );
    }

    void clear( int to_push = 1 ) {
        for ( Sinks::iterator i = sinks.begin(); i != sinks.end(); ++i )
            delete *i;
        sinks.clear();
        if ( to_push )
            sinks.push_back( _observer = new Observer );
    }

    ~IO() { close(); clear(0); }

};

namespace {
pid_t kill_pid = 0;
bool fatal_signal = false;
bool interrupt = false;
}

struct Options {
    bool verbose, batch, interactive, cont, fatal_timeouts, kmsg;
    std::string testdir, outdir, workdir, heartbeat;
    std::vector< std::string > flavours, filter, skip, watch;
    std::string flavour_envvar;
    int timeout;
    Options() : verbose( false ), batch( false ), interactive( false ),
                cont( false ), fatal_timeouts( false ), kmsg( true ),
                timeout( 180 ) {}
};

struct TestProcess
{
    std::string filename;
    bool interactive;
    int fd;

    void exec() __attribute__ ((noreturn)) {
        assert( fd >= 0 );
        if ( !interactive ) {
            int devnull = ::open( "/dev/null", O_RDONLY );
            if ( devnull >= 0 ) { /* gcc really doesn't like to not have stdin */
                dup2( devnull, STDIN_FILENO );
                close( devnull );
            } else
                close( STDIN_FILENO );
            dup2( fd, STDOUT_FILENO );
            dup2( fd, STDERR_FILENO );
            close( fd );
        }

        setpgid( 0, 0 );

        execlp( "bash", "bash", "-noprofile", "-norc", filename.c_str(), NULL );
        perror( "execlp" );
        _exit( 202 );
    }

    TestProcess( std::string file )
        : filename( file ), interactive( false ), fd( -1 )
    {}
};

struct TestCase {
    TestProcess child;
    std::string name, flavour;
    IO io;
    BufSink *iobuf;

    struct rusage usage;
    int status;
    bool timeout;
    pid_t pid;

    time_t start, end, silent_start, last_update, last_heartbeat;
    Options options;

    Journal *journal;

    std::string pretty() {
        if ( options.batch )
            return flavour + ": " + name;
        return "[" + flavour + "] " + name;
    }

    std::string id() {
        return flavour + ":" + name;
    }

    void pipe() {
        int fds[2];

        if (socketpair( PF_UNIX, SOCK_STREAM, 0, fds )) {
            perror("socketpair");
            exit(201);
        }

#if 0
        if (fcntl( fds[0], F_SETFL, O_NONBLOCK ) == -1) {
            perror("fcntl on socket");
            exit(202);
        }
#endif

        io.sources.push_back( new Source( fds[0] ) );
        child.fd = fds[1];
        child.interactive = options.interactive;
    }

    bool monitor() {
        end = time( 0 );

        /* heartbeat */
        if ( end - last_heartbeat >= 20 && !options.heartbeat.empty() ) {
            std::ofstream hb( options.heartbeat.c_str(), std::fstream::app );
            hb << ".";
            hb.close();
            fsync_name( options.heartbeat );
            last_heartbeat = end;
        }

        if ( wait4(pid, &status, WNOHANG, &usage) != 0 ) {
            io.sync( true );
            return false;
        }

        /* kill off tests after a timeout silence */
        if ( !options.interactive )
            if ( end - silent_start > options.timeout ) {
                kill( pid, SIGINT );
                sleep( 5 ); /* wait a bit for a reaction */
                if ( waitpid( pid, &status, WNOHANG ) == 0 ) {
                    system( "echo t > /proc/sysrq-trigger 2> /dev/null" );
                    kill( -pid, SIGKILL );
                    waitpid( pid, &status, 0 );
                }
                timeout = true;
                io.sync( true );
                return false;
            }

        struct timeval wait;
        fd_set set;

        FD_ZERO( &set );
        int nfds = io.fd_set_( &set );
        wait.tv_sec = 0;
        wait.tv_usec = 500000; /* timeout 0.5s */

        if ( !options.verbose && !options.interactive && !options.batch ) {
            if ( end - last_update >= 1 ) {
                progress( Update ) << tag( "running" ) << pretty() << " "
                                   << end - start << std::flush;
                last_update = end;
            }
        }
        if ( select( nfds, &set, NULL, NULL, &wait ) > 0 ) {
            silent_start = end; /* something happened */
            io.sync( false );
        }

        return true;
    }

    std::string timefmt( time_t t ) {
        std::stringstream ss;
        ss << t / 60 << ":" << std::setw( 2 ) << std::setfill( '0' ) << t % 60;
        return ss.str();
    }

    std::string rusage()
    {
        std::stringstream ss;
        time_t wall = end - start, user = usage.ru_utime.tv_sec,
             system = usage.ru_stime.tv_sec;
        size_t rss = usage.ru_maxrss / 1024,
               inb = usage.ru_inblock / 100,
              outb = usage.ru_oublock / 100;

        size_t inb_10 = inb % 10, outb_10 = outb % 10;
        inb /= 10; outb /= 10;

        ss << timefmt( wall ) << " wall " << timefmt( user ) << " user "
           << timefmt( system ) << " sys   " << std::setw( 3 ) << rss << "M RSS | "
           << "IOPS: " << std::setw( 5 ) << inb << "." << inb_10 << "K in "
           << std::setw( 5 ) << outb << "." << outb_10 << "K out";
        return ss.str();
    }

    std::string tag( std::string n ) {
        if ( options.batch )
            return "## ";
        size_t pad = n.length();
        pad = (pad < 12) ? 12 - pad : 0;
        return "### " + std::string( pad, ' ' ) + n + ": ";
    }

    std::string tag( Journal::R r ) {
        std::stringstream s;
        s << r;
        return tag( s.str() );
    }

    enum P { First, Update, Last };

    std::ostream &progress( P p = Last )
    {
        static struct : std::streambuf {} buf;
        static std::ostream null(&buf);

        if ( options.batch && p == First )
            return std::cout;

        if ( isatty( STDOUT_FILENO ) && !options.batch ) {
            if ( p != First )
                return std::cout << "\r";
            return std::cout;
        }

        if ( p == Last )
            return std::cout;

        return null;
    }

    void parent()
    {
        ::close( child.fd );
        setupIO();

        journal->started( id() );
        silent_start = start = time( 0 );

        progress( First ) << tag( "running" ) << pretty() << std::flush;
        if ( options.verbose || options.interactive )
            progress() << std::endl;

        while ( monitor() )
            /* empty */ ;

        Journal::R r = Journal::UNKNOWN;

        if ( timeout ) {
            r = Journal::TIMEOUT;
        } else if ( WIFEXITED( status ) ) {
            if ( WEXITSTATUS( status ) == 0 )
                r = Journal::PASSED;
            else if ( WEXITSTATUS( status ) == 200 )
                r = Journal::SKIPPED;
            else
                r = Journal::FAILED;
        } else if ( interrupt && WIFSIGNALED( status ) && WTERMSIG( status ) == SIGINT )
            r = Journal::INTERRUPTED;
        else
            r = Journal::FAILED;

        if ( r == Journal::PASSED && io.observer().warnings )
            r = Journal::WARNED;

        io.close();

        if ( iobuf && ( r == Journal::FAILED || r == Journal::TIMEOUT ) )
            iobuf->dump( std::cout );

        journal->done( id(), r );

        if ( options.batch ) {
            int spaces = std::max( 64 - int(pretty().length()), 0 );
            progress( Last ) << " " << std::string( spaces, '.' ) << " "
                << std::left << std::setw( 9 ) << std::setfill( ' ' ) << r;
            if ( r != Journal::SKIPPED )
                progress( First ) << "   " << rusage();
            progress( Last ) << std::endl;
        } else
            progress( Last ) << tag( r ) << pretty() << std::endl;

        io.clear();
    }

    void run() {
        pipe();
        pid = kill_pid = fork();
        if (pid < 0) {
            perror("Fork failed.");
            exit(201);
        } else if (pid == 0) {
            io.close();
            chdir( options.workdir.c_str() );
            if ( !options.flavour_envvar.empty() )
                setenv( options.flavour_envvar.c_str(), flavour.c_str(), 1 );
            child.exec();
        } else {
            parent();
        }
    }

    void setupIO() {
        iobuf = 0;
        if ( options.verbose || options.interactive )
            io.sinks.push_back( new FdSink( 1 ) );
        else if ( !options.batch )
            io.sinks.push_back( iobuf = new BufSink() );

        std::string n = id();
        std::replace( n.begin(), n.end(), '/', '_' );
        std::string fn = options.outdir + "/" + n + ".txt";
        io.sinks.push_back( new FileSink( fn ) );

        for ( std::vector< std::string >::iterator i = options.watch.begin();
              i != options.watch.end(); ++i )
            io.sources.push_back( new FileSource( *i ) );
        if ( options.kmsg )
            io.sources.push_back( new KMsg );
    }

    TestCase( Journal &j, Options opt, std::string path, std::string _name, std::string _flavour )
        : child( path ), name( _name ), flavour( _flavour ), timeout( false ),
          last_update( 0 ), last_heartbeat( 0 ), options( opt ), journal( &j )
    {
    }
};

struct Main {
    bool die;
    time_t start;

    typedef std::vector< TestCase > Cases;
    typedef std::vector< std::string > Flavours;

    Journal journal;
    Options options;
    Cases cases;

    void setup() {
        bool filter;
        Listing l = listdir( options.testdir, true );
        std::sort( l.begin(), l.end() );

        for ( Flavours::iterator flav = options.flavours.begin();
              flav != options.flavours.end(); ++flav ) {

            for ( Listing::iterator i = l.begin(); i != l.end(); ++i ) {
                if ( ( i->length() < 3 ) || ( i->substr( i->length() - 3, i->length() ) != ".sh" ) )
                    continue;
                if ( i->substr( 0, 4 ) == "lib/" )
                    continue;

                if (!options.filter.empty()) {
                    filter = true;
                    for ( std::vector< std::string >::iterator filt = options.filter.begin();
                          filt != options.filter.end(); ++filt ) {
                        if ( i->find( *filt ) != std::string::npos ) {
                            filter = false;
                            break;
                        }
                    }
                    if ( filter )
                        continue;
                }

                if (!options.skip.empty()) {
                    filter = false;
                    for ( std::vector< std::string >::iterator filt = options.skip.begin();
                          filt != options.skip.end(); ++filt ) {
                        if ( i->find( *filt ) != std::string::npos ) {
                            filter = true;
                            break;
                        }
                    }
                    if ( filter )
                        continue;
                }

                cases.push_back( TestCase( journal, options, options.testdir + *i, *i, *flav ) );
                cases.back().options = options;
            }
        }

        if ( options.cont )
            journal.read();
        else
            ::unlink( journal.location.c_str() );
    }

    int run() {
        setup();
        start = time( 0 );
        std::cerr << "running " << cases.size() << " tests" << std::endl;

        for ( Cases::iterator i = cases.begin(); i != cases.end(); ++i ) {

            if ( options.cont && journal.done( i->id() ) )
                continue;

            i->run();

            if ( options.fatal_timeouts && journal.timeouts >= 2 ) {
                journal.started( i->id() ); // retry the test on --continue
                std::cerr << "E: Hit 2 timeouts in a row with --fatal-timeouts" << std::endl;
                std::cerr << "Suspending (please restart the VM)." << std::endl;
                sleep( 3600 );
                die = 1;
            }

            if ( time(0) - start > (TEST_SUITE_TIMEOUT * 3600) ) {
                std::cerr << TEST_SUITE_TIMEOUT << " hours passed, giving up..." << std::endl;
                die = 1;
            }

            if ( die || fatal_signal )
                break;
        }

        journal.banner();
        if ( die || fatal_signal )
            return 1;

        return journal.count( Journal::FAILED ) || journal.count( Journal::TIMEOUT ) ? 1 : 0;
    }

    Main( Options o ) : die( false ), journal( o.outdir ), options( o ) {}
};

namespace {

void handler( int sig ) {
    signal( sig, SIG_DFL ); /* die right away next time */
    if ( kill_pid > 0 )
        kill( -kill_pid, sig );
    fatal_signal = true;
    if ( sig == SIGINT )
        interrupt = true;
}

void setup_handlers() {
    /* set up signal handlers */
    for ( int i = 0; i <= 32; ++i )
        switch (i) {
            case SIGCHLD: case SIGWINCH: case SIGURG:
            case SIGKILL: case SIGSTOP: break;
            default: signal(i, handler);
        }
}

}

/* TODO remove in favour of brick-commandline.h */
struct Args {
    typedef std::vector< std::string > V;
    V args;

    Args( int argc, const char **argv ) {
        for ( int i = 1; i < argc; ++ i )
            args.push_back( argv[ i ] );
    }

    bool has( std::string fl ) {
        return std::find( args.begin(), args.end(), fl ) != args.end();
    }

    // TODO: This does not handle `--option=VALUE`:
    std::string opt( std::string fl ) {
        V::iterator i = std::find( args.begin(), args.end(), fl );
        if ( i == args.end() || i + 1 == args.end() )
            return "";
        return *(i + 1);
    }
};

namespace {

bool hasenv( const char *name ) {
    const char *v = getenv( name );
    if ( !v )
        return false;
    if ( strlen( v ) == 0 || !strcmp( v, "0" ) )
        return false;
    return true;
}

template< typename C >
void split( std::string s, C &c ) {
    std::stringstream ss( s );
    std::string item;
    while ( std::getline( ss, item, ',' ) )
        c.push_back( item );
}

}

const char *DEF_FLAVOURS="ndev-vanilla";

std::string resolve_path(std::string a_path, const char *default_path=".")
{
    char temp[PATH_MAX];
    const char *p;
    p = a_path.empty() ? default_path : a_path.c_str();
    if ( !realpath( p, temp ) )
        throw syserr( "Failed to resolve path", p );
    return temp;
}

static int run( int argc, const char **argv, std::string fl_envvar = "TEST_FLAVOUR" )
{
    Args args( argc, argv );
    Options opt;

    if ( args.has( "--help" ) ) {
        std::cout <<
            "  lvm2-testsuite - Run a lvm2 testsuite.\n\n"
            "lvm2-testsuite"
            "\n\t"
            " [--flavours FLAVOURS]"
            " [--only TESTS]"
            "\n\t"
            " [--outdir OUTDIR]"
            " [--testdir TESTDIR]"
            " [--workdir WORKDIR]"
            "\n\t"
            " [--batch|--verbose|--interactive]"
            "\n\t"
            " [--fatal-timeouts]"
            " [--continue]"
            " [--heartbeat]"
            " [--watch WATCH]"
            " [--timeout TIMEOUT]"
            " [--nokmsg]\n\n"
            /* TODO: list of flavours:
            "lvm2-testsuite"
            "\n\t"
            " --list-flavours [--testdir TESTDIR]"
            */
            "\n\n"
            "OPTIONS:\n\n"
            // TODO: looks like this could be worth a man page...
            "Filters:\n"
            "  --flavours FLAVOURS\n\t\t- comma separated list of flavours to run.\n\t\t  For the list of flavours see `$TESTDIR/lib/flavour-*`.\n\t\t  Default: \"" << DEF_FLAVOURS << "\".\n"
            "  --only TESTS\t- comma separated list of tests to run. Default: All tests.\n"
            "\n"
            "Directories:\n"
            "  --testdir TESTDIR\n\t\t- directory where tests reside. Default: \"" TESTSUITE_DATA "\".\n"
            "  --workdir WORKDIR\n\t\t- directory to change to when running tests.\n\t\t  This is directory containing testing libs. Default: TESTDIR.\n"
            "  --outdir OUTDIR\n\t\t- directory where all the output files should go. Default: \".\".\n"
            "\n"
            "Formatting:\n"
            "  --batch\t- Brief format for automated runs.\n"
            "  --verbose\t- More verbose format for automated runs displaying progress on stdout.\n"
            "  --interactive\t- Verbose format for interactive runs.\n"
            "\n"
            "Other:\n"
            "  --fatal-timeouts\n\t\t- exit after encountering 2 timeouts in a row.\n"
            "  --continue\t- If set append to journal. Otherwise it will be overwritten.\n"
            "  --heartbeat HEARTBEAT\n\t\t- Name of file to update periodicaly while running.\n"
            "  --watch WATCH\t- Comma separated list of files to watch and print.\n"
            "  --timeout TIMEOUT\n\t\t- Period of silence in seconds considered a timeout. Default: 180.\n"
            "  --nokmsg\t- Do not try to read kernel messages.\n"
            "\n\n"
            "ENV.VARIABLES:\n\n"
            "  T\t\t- see --only\n"
            "  INTERACTIVE\t- see --interactive\n"
            "  VERBOSE\t- see --verbose\n"
            "  BATCH\t\t- see --batch\n"
            "  LVM_TEST_CAN_CLOBBER_DMESG\n\t\t- when set and non-empty tests are allowed to flush\n\t\t  kmsg in an attempt to read it."
            "\n\n"
            "FORMATS:\n\n"
            "When multiple formats are specified interactive overrides verbose\n"
            "which overrides batch. Command line options override environment\n"
            "variables.\n\n"
            ;
        return 0;
    }

    opt.flavour_envvar = fl_envvar;

    if ( args.has( "--continue" ) )
        opt.cont = true;

    if ( args.has( "--only" ) )
        split( args.opt( "--only" ), opt.filter );
    else if ( hasenv( "T" ) )
        split( getenv( "T" ), opt.filter );

    if ( args.has( "--skip" ) )
        split( args.opt( "--skip" ), opt.skip );
    else if ( hasenv( "S" ) )
        split( getenv( "S" ), opt.skip );

    if ( args.has( "--fatal-timeouts" ) )
        opt.fatal_timeouts = true;

    if ( args.has( "--heartbeat" ) )
        opt.heartbeat = args.opt( "--heartbeat" );

    if ( args.has( "--batch" ) || args.has( "--verbose" ) || args.has( "--interactive" ) ) {
        if ( args.has( "--batch" ) ) {
            opt.verbose = false;
            opt.batch = true;
        }

        if ( args.has( "--verbose" ) ) {
            opt.batch = false;
            opt.verbose = true;
        }

        if ( args.has( "--interactive" ) ) {
            opt.verbose = false;
            opt.batch = false;
            opt.interactive = true;
        }
    } else {
        if ( hasenv( "BATCH" ) ) {
            opt.verbose = false;
            opt.batch = true;
        }

        if ( hasenv( "VERBOSE" ) ) {
            opt.batch = false;
            opt.verbose = true;
        }

        if ( hasenv( "INTERACTIVE" ) ) {
            opt.verbose = false;
            opt.batch = false;
            opt.interactive = true;
        }
    }

    if ( args.has( "--flavours" ) )
        split( args.opt( "--flavours" ), opt.flavours );
    else
        split( DEF_FLAVOURS, opt.flavours );

    if ( args.has( "--watch" ) )
        split( args.opt( "--watch" ), opt.watch );

    if ( args.has( "--timeout" ) )
        opt.timeout = atoi( args.opt( "--timeout" ).c_str() );

    if ( args.has( "--nokmsg" ) )
        opt.kmsg = false;

    opt.testdir = resolve_path( args.opt( "--testdir" ), TESTSUITE_DATA ) + "/";
    opt.workdir = resolve_path( args.opt( "--workdir" ), opt.testdir.c_str() );
    opt.outdir = resolve_path( args.opt( "--outdir" ), "." );

    setup_handlers();

    Main main( opt );
    return main.run();
}

}
}

#endif

#ifdef BRICK_DEMO

int main( int argc, const char **argv ) {
    return brick::shelltest::run( argc, argv );
}

#endif

// vim: syntax=cpp tabstop=4 shiftwidth=4 expandtab
