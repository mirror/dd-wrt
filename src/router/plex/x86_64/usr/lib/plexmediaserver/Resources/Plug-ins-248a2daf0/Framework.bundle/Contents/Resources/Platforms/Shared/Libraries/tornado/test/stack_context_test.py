#!/usr/bin/env python
from __future__ import with_statement

from tornado.stack_context import StackContext, wrap
from tornado.testing import AsyncHTTPTestCase, AsyncTestCase, LogTrapTestCase
from tornado.web import asynchronous, Application, RequestHandler
import contextlib
import functools
import logging
import unittest

class TestRequestHandler(RequestHandler):
    def __init__(self, app, request, io_loop):
        super(TestRequestHandler, self).__init__(app, request)
        self.io_loop = io_loop

    @asynchronous
    def get(self):
        logging.info('in get()')
        # call self.part2 without a self.async_callback wrapper.  Its
        # exception should still get thrown
        self.io_loop.add_callback(self.part2)

    def part2(self):
        logging.info('in part2()')
        # Go through a third layer to make sure that contexts once restored
        # are again passed on to future callbacks
        self.io_loop.add_callback(self.part3)

    def part3(self):
        logging.info('in part3()')
        raise Exception('test exception')

    def get_error_html(self, status_code, **kwargs):
        if 'exception' in kwargs and str(kwargs['exception']) == 'test exception':
            return 'got expected exception'
        else:
            return 'unexpected failure'

class HTTPStackContextTest(AsyncHTTPTestCase, LogTrapTestCase):
    def get_app(self):
        return Application([('/', TestRequestHandler,
                             dict(io_loop=self.io_loop))])

    def test_stack_context(self):
        self.http_client.fetch(self.get_url('/'), self.handle_response)
        self.wait()
        self.assertEquals(self.response.code, 500)
        self.assertTrue('got expected exception' in self.response.body)

    def handle_response(self, response):
        self.response = response
        self.stop()

class StackContextTest(AsyncTestCase, LogTrapTestCase):
    def setUp(self):
        super(StackContextTest, self).setUp()
        self.active_contexts = set()

    @contextlib.contextmanager
    def context(self, name):
        assert name not in self.active_contexts
        self.active_contexts.add(name)
        yield
        assert name in self.active_contexts
        self.active_contexts.remove(name)

    # Simulates the effect of an asynchronous library that uses its own
    # StackContext internally and then returns control to the application.
    def test_exit_library_context(self):
        def library_function(callback):
            # capture the caller's context before introducing our own
            callback = wrap(callback)
            with StackContext(functools.partial(self.context, 'library')):
                self.io_loop.add_callback(
                  functools.partial(library_inner_callback, callback))
        def library_inner_callback(callback):
            assert 'application' in self.active_contexts
            assert 'library' in self.active_contexts
            # pass the callback out to the IOLoop to get out of the library
            # context (could also use a NullContext here, but that would result
            # in multiple instantiations of the application context)
            self.io_loop.add_callback(callback)
        def final_callback():
            assert 'application' in self.active_contexts
            assert 'library' not in self.active_contexts
            self.stop()
        with StackContext(functools.partial(self.context, 'application')):
            library_function(final_callback)
        self.wait()

if __name__ == '__main__':
    unittest.main()
