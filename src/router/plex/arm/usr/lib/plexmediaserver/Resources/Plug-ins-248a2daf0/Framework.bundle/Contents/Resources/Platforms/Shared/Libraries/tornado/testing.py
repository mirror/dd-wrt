#!/usr/bin/env python
"""Support classes for automated testing.

This module contains three parts:
* AsyncTestCase/AsyncHTTPTestCase:  Subclasses of unittest.TestCase
  with additional support for testing asynchronous (IOLoop-based) code.
* LogTrapTestCase:  Subclass of unittest.TestCase that discards log output
  from tests that pass and only produces output for failing tests.
* main(): A simple test runner (wrapper around unittest.main()) with support
  for the tornado.autoreload module to rerun the tests when code changes.

These components may be used together or independently.  In particular,
it is safe to combine AsyncTestCase and LogTrapTestCase via multiple
inheritance.  See the docstrings for each class/function below for more
information.
"""

from __future__ import with_statement

from cStringIO import StringIO
from tornado.httpclient import AsyncHTTPClient
from tornado.httpserver import HTTPServer
from tornado.stack_context import StackContext, NullContext
import contextlib
import functools
import logging
import os
import sys
import time
import tornado.ioloop
import traceback
import unittest

class AsyncTestCase(unittest.TestCase):
    """TestCase subclass for testing IOLoop-based asynchronous code.

    The unittest framework is synchronous, so the test must be complete
    by the time the test method returns.  This method provides the stop()
    and wait() methods for this purpose.  The test method itself must call
    self.wait(), and asynchronous callbacks should call self.stop() to signal
    completion.

    By default, a new IOLoop is constructed for each test and is available
    as self.io_loop.  This IOLoop should be used in the construction of
    HTTP clients/servers, etc.  If the code being tested requires a
    global IOLoop, subclasses should override get_new_ioloop to return it.

    The IOLoop's start and stop methods should not be called directly.
    Instead, use self.stop self.wait.  Arguments passed to self.stop are
    returned from self.wait.  It is possible to have multiple
    wait/stop cycles in the same test.

    Example:
        # This test uses an asynchronous style similar to most async
        # application code.
        class MyTestCase(AsyncTestCase):
            def test_http_fetch(self):
                client = AsyncHTTPClient(self.io_loop)
                client.fetch("http://www.tornadoweb.org/", self.handle_fetch)
                self.wait()

            def handle_fetch(self, response)
                # Test contents of response (failures and exceptions here
                # will cause self.wait() to throw an exception and end the
                # test).
                self.stop()

        # This test uses the argument passing between self.stop and self.wait
        # for a simpler, more synchronous style
        class MyTestCase2(AsyncTestCase):
            def test_http_fetch(self):
                client = AsyncHTTPClient(self.io_loop)
                client.fetch("http://www.tornadoweb.org/", self.stop)
                response = self.wait()
                # Test contents of response
    """
    def __init__(self, *args, **kwargs):
        super(AsyncTestCase, self).__init__(*args, **kwargs)
        self.__stopped = False
        self.__running = False
        self.__failure = None
        self.__stop_args = None

    def setUp(self):
        super(AsyncTestCase, self).setUp()
        self.io_loop = self.get_new_ioloop()

    def tearDown(self):
        if self.io_loop is not tornado.ioloop.IOLoop.instance():
            # Try to clean up any file descriptors left open in the ioloop.
            # This avoids leaks, especially when tests are run repeatedly
            # in the same process with autoreload (because curl does not
            # set FD_CLOEXEC on its file descriptors)
            for fd in self.io_loop._handlers.keys()[:]:
                if (fd == self.io_loop._waker_reader.fileno() or
                    fd == self.io_loop._waker_writer.fileno()):
                    # Close these through the file objects that wrap
                    # them, or else the destructor will try to close
                    # them later and log a warning
                    continue
                try:
                    os.close(fd)
                except:
                    logging.debug("error closing fd %d", fd, exc_info=True)
            self.io_loop._waker_reader.close()
            self.io_loop._waker_writer.close()

    def get_new_ioloop(self):
        '''Creates a new IOLoop for this test.  May be overridden in
        subclasses for tests that require a specific IOLoop (usually
        the singleton).
        '''
        return tornado.ioloop.IOLoop()

    @contextlib.contextmanager
    def _stack_context(self):
        try:
            yield
        except:
            self.__failure = sys.exc_info()
            self.stop()

    def run(self, result=None):
        with StackContext(self._stack_context):
            super(AsyncTestCase, self).run(result)

    def stop(self, _arg=None, **kwargs):
        '''Stops the ioloop, causing one pending (or future) call to wait()
        to return.

        Keyword arguments or a single positional argument passed to stop() are
        saved and will be returned by wait().
        '''
        self.__stop_args = _arg or kwargs
        if self.__running:
            self.io_loop.stop()
            self.__running = False
        self.__stopped = True

    def wait(self, condition=None, timeout=5):
        """Runs the IOLoop until stop is called or timeout has passed.

        In the event of a timeout, an exception will be thrown.

        If condition is not None, the IOLoop will be restarted after stop()
        until condition() returns true.
        """
        if not self.__stopped:
            if timeout:
                def timeout_func():
                    try:
                        raise self.failureException(
                          'Async operation timed out after %d seconds' %
                          timeout)
                    except:
                        self.__failure = sys.exc_info()
                    self.stop()
                self.io_loop.add_timeout(time.time() + timeout, timeout_func)
            while True:
                self.__running = True
                with NullContext():
                    # Wipe out the StackContext that was established in
                    # self.run() so that all callbacks executed inside the
                    # IOLoop will re-run it.
                    self.io_loop.start()
                if (self.__failure is not None or
                    condition is None or condition()):
                    break
        assert self.__stopped
        self.__stopped = False
        if self.__failure is not None:
            raise self.__failure[0], self.__failure[1], self.__failure[2]
        result = self.__stop_args
        self.__stop_args = None
        return result


class AsyncHTTPTestCase(AsyncTestCase):
    '''A test case that starts up an HTTP server.

    Subclasses must override get_app(), which returns the
    tornado.web.Application (or other HTTPServer callback) to be tested.
    Tests will typically use the provided self.http_client to fetch
    URLs from this server.

    Example:
        class MyHTTPTest(AsyncHTTPTestCase):
            def get_app(self):
                return Application([('/', MyHandler)...])

            def test_homepage(self):
                self.http_client.fetch(self.get_url('/'), self.stop)
                response = self.wait()
                # test contents of response
    '''
    __next_port = 10000

    def setUp(self):
        super(AsyncHTTPTestCase, self).setUp()
        self.__port = None

        self.http_client = AsyncHTTPClient(io_loop=self.io_loop)
        self._app = self.get_app()
        self.http_server = HTTPServer(self._app, io_loop=self.io_loop)
        self.http_server.listen(self.get_http_port())

    def get_app(self):
        """Should be overridden by subclasses to return a
        tornado.web.Application or other HTTPServer callback.
        """
        raise NotImplementedError()

    def get_http_port(self):
        """Returns the port used by the HTTPServer.

        A new port is chosen for each test.
        """
        if self.__port is not None:
            return self.__port
        self.__port = AsyncHTTPTestCase.__next_port
        AsyncHTTPTestCase.__next_port = self.__port + 1
        return self.__port

    def get_url(self, path):
        """Returns an absolute url for the given path on the test server."""
        return 'http://localhost:%s%s' % (self.get_http_port(), path)

    def tearDown(self):
        self.http_server.stop()
        self.http_client.close()
        super(AsyncHTTPTestCase, self).tearDown()

class LogTrapTestCase(unittest.TestCase):
    """A test case that captures and discards all logging output
    if the test passes.

    Some libraries can produce a lot of logging output even when
    the test succeeds, so this class can be useful to minimize the noise.
    Simply use it as a base class for your test case.  It is safe to combine
    with AsyncTestCase via multiple inheritance
    ("class MyTestCase(AsyncHTTPTestCase, LogTrapTestCase):")

    This class assumes that only one log handler is configured and that
    it is a StreamHandler.  This is true for both logging.basicConfig
    and the "pretty logging" configured by tornado.options.
    """
    def run(self, result=None):
        logger = logging.getLogger()
        if len(logger.handlers) > 1:
            # Multiple handlers have been defined.  It gets messy to handle
            # this, especially since the handlers may have different
            # formatters.  Just leave the logging alone in this case.
            super(LogTrapTestCase, self).run(result)
            return
        if not logger.handlers:
            logging.basicConfig()
        self.assertEqual(len(logger.handlers), 1)
        handler = logger.handlers[0]
        assert isinstance(handler, logging.StreamHandler)
        old_stream = handler.stream
        try:
            handler.stream = StringIO()
            logging.info("RUNNING TEST: " + str(self))
            old_error_count = len(result.failures) + len(result.errors)
            super(LogTrapTestCase, self).run(result)
            new_error_count = len(result.failures) + len(result.errors)
            if new_error_count != old_error_count:
                old_stream.write(handler.stream.getvalue())
        finally:
            handler.stream = old_stream

def main():
    """A simple test runner with autoreload support.

    The easiest way to run a test is via the command line:
        python -m tornado.testing --autoreload tornado.test.stack_context_test
    See the standard library unittest module for ways in which tests can
    be specified.

    Projects with many tests may wish to define a test script like
    tornado/test/runtests.py.  This script should define a method all()
    which returns a test suite and then call tornado.testing.main().
    Note that even when a test script is used, the all() test suite may
    be overridden by naming a single test on the command line.
        # Runs all tests
        tornado/test/runtests.py --autoreload
        # Runs one test
        tornado/test/runtests.py --autoreload tornado.test.stack_context_test

    If --autoreload is specified, the process will continue running
    after the tests finish, and when any source file changes the tests
    will be rerun.  Without --autoreload, the process will exit
    once the tests finish (with an exit status of 0 for success and
    non-zero for failures).
    """
    from tornado.options import define, options, parse_command_line

    define('autoreload', type=bool, default=False)
    argv = [sys.argv[0]] + parse_command_line(sys.argv)
    if __name__ == '__main__' and len(argv) == 1:
        print >> sys.stderr, "No tests specified"
        sys.exit(1)
    try:
        # In order to be able to run tests by their fully-qualified name
        # on the command line without importing all tests here,
        # module must be set to None (in which case the defaultTest must
        # also be fully-qualified
        unittest.main(module=None, defaultTest='__main__.all', argv=argv)
    except SystemExit, e:
        if e.code == 0:
            logging.info('PASS')
        else:
            logging.error('FAIL')
        if not options.autoreload:
            raise
    if options.autoreload:
        import tornado.autoreload
        import tornado.ioloop
        ioloop = tornado.ioloop.IOLoop()
        tornado.autoreload.start(ioloop)
        ioloop.start()

if __name__ == '__main__':
    main()
