Feature: Plugins

  Start the proxy with plugins

  Scenario: start the proxy with the no-op plugin

    When I run `dnscrypt-proxy --test=0 -R random --plugin=libdcplugin_example.la`
    Then the output should contain:
    """
    A sample (useless) plugin for dnscrypt-proxy
    """
    And the exit status should be 0

  Scenario: start the proxy with the ldns_aaaa_blocking plugin

    When I run `dnscrypt-proxy --test=0 -R random --plugin=libdcplugin_example_ldns_aaaa_blocking.la`
    Then the output should contain:
    """
    Directly return an empty response to AAAA queries
    """
    And the exit status should be 0
