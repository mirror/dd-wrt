# Integration Tests

This directory contains a suite of tests that exercises the rtpproxy command channel, and various
aspects of the rtpproxy operation. 

Running `make check` in the root rtpproxy directory will run these tests. The rtpproxy repository is hooked up to a continuous integration services (travis or drone.io) that will automatically run all tests.


## rtpproxy payload conventions

The tests make use of prerecorded files that are encoded in various formats (payloads). Each file has a numeric suffix that represents the payload type as defined in (rfc3551 Section 6. Payload Type Definitions)[http://tools.ietf.org/html/rfc3551#section-6].



## Adding new tests

To add additional tests, create a bourne shell script, and turn on the executable file bit. If your test requires supporting files, such files should use the same name as the test script, and use a suffix that reasonably describes its purpose.

