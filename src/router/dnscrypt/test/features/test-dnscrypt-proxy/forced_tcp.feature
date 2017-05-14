Feature: check that --tcp-only works as advertised.

  A query sent using UDP should, when the proxy is running with
--tcp-only, return a truncated reply so that the client retries using
TCP.
  
  Scenario: query an existing name over UDP, and even though this name
would fit in a 512-bytes UDP packet, expect a forced fallback to TCP.
  
    Given a working server proxy on 212.47.228.136
    And a running dnscrypt proxy with options "--edns-payload-size=4096 --tcp-only -R random"
    When a client asks dnscrypt-proxy for "test-ff.dnscrypt.org"
    Then dnscrypt-proxy returns "255.255.255.255"
