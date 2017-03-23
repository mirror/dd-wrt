Feature: Show help and version for dnscrypt-proxy

  Display the help when the -h switch is given.
  
  Scenario: start with the -h switch.
  
    When I run `dnscrypt-proxy -h`
    Then the output should contain:
    """
    Options:
    """
    And the exit status should be 0
    
  Scenario: start with the --help switch

    When I run `dnscrypt-proxy --help`
    Then the output should contain:
    """
    Options:
    """
    And the exit status should be 0

  Scenario: start with a nonexistent switch

    When I run `dnscrypt-proxy -%`
    Then the output should contain:
    """
    Options:
    """
    And the exit status should be 1

  Scenario: start the -V switch

    When I run `dnscrypt-proxy -V`
    Then the output should contain:
    """
    dnscrypt-proxy
    """
    And the output should not contain:
    """
    Options
    """
    And the exit status should be 0
