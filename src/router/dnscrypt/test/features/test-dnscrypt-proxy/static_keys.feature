Feature: Static client keys

  Use static client keys

  Scenario: start the daemon with both ephemeral and static keys

    When I run `dnscrypt-proxy -R random --client-key=test-client.key --ephemeral-keys`
    Then the output should contain:
    """
    ERROR
    """
    And the exit status should be 1

  Scenario: start the daemon with a nonexistent static key kfile

    When I run `dnscrypt-proxy -R random --client-key=/nonexistent`
    Then the output should contain:
    """
    ERROR
    """
    And the exit status should be 1

  Scenario: query an existing name, with a static client key

    Given a working server proxy on 212.47.228.136
    And a running dnscrypt proxy with options "--edns-payload-size=0 -R random --client-key=test-client.key"
    When a client asks dnscrypt-proxy for "test-ff.dnscrypt.org"
    Then dnscrypt-proxy returns "255.255.255.255"

  Scenario: query a nonexistent name, with a static client key

    Given a working server proxy on 212.47.228.136
    And a running dnscrypt proxy with options "--edns-payload-size=0 -R random --client-key=test-client.key"
    When a client asks dnscrypt-proxy for "test-nonexistent.dnscrypt.org"
    Then dnscrypt-proxy returns a NXDOMAIN answer
