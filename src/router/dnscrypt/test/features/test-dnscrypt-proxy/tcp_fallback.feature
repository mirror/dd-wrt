Feature: fallback to TCP

  A query that doesn't fit in a small UDP packet should make the proxy
send a truncated reply, then the stub resolver should retry with TCP
and the proxy should handle TCP just fine.
  
  Scenario: query an existing name over UDP, expect fallback to TCP.
  
    Given a working server proxy on 212.47.228.136
    And a running dnscrypt proxy with options "--edns-payload-size=0 -R random"
    When a client asks dnscrypt-proxy for "test-tcp.dnscrypt.org"
    Then dnscrypt-proxy returns "127.0.0.1"
