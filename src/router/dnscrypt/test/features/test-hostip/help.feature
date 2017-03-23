Feature: Show help and version for the hostip utility

  Display the help when the -h switch is given.
  
  Scenario: start with the -h switch.
  
    When I run `hostip -h`
    Then the output should contain:
    """
    Usage:
    """
    And the exit status should be 0
    
  Scenario: start with the --help switch

    When I run `hostip --help`
    Then the output should contain:
    """
    Usage:
    """
    And the exit status should be 0

  Scenario: start with a nonexistent switch

    When I run `hostip -%`
    Then the output should contain:
    """
    Usage:
    """
    And the exit status should be 1

  Scenario: start the -V switch

    When I run `hostip -V`
    Then the output should contain:
    """
    hostip
    """
    And the output should not contain:
    """
    Usage
    """
    And the exit status should be 0
