Feature: See if hostip can resolve names given a specific resolver

  Resolve names with the hostip utility using Google DNS.
  
  Scenario: resolve test-ff.dnscrypt.org
  
    When I run `hostip -r 8.8.8.8 test-ff.dnscrypt.org`
    Then the output should contain exactly:
    """
    255.255.255.255

    """
    And the exit status should be 0

  Scenario: resolve IPv6 address for test-ipv6.dnscrypt.org
  
    When I run `hostip -r 8.8.8.8 -6 test-ipv6.dnscrypt.org`
    Then the output should contain exactly:
    """
    fe80::123:4567:89ab:cdef

    """
    And the exit status should be 0

  Scenario: resolve a nonexistent name
  
    When I run `hostip -r 8.8.8.8 nonexistent.local`
    Then the output should contain exactly:
    """
    [name does not exist]

    """
    And the exit status should be 1

  Scenario: resolve a nonexistent IPv6 name
  
    When I run `hostip -r 8.8.8.8 -6 nonexistent.local`
    Then the output should contain exactly:
    """
    [name does not exist]

    """
    And the exit status should be 1
