{uptime:Time : <% get_uptime(); %>}
{wan:WAN <% nvram_match("wan_proto","disabled","disabled <!--"); %>IP : <% nvram_status_get("wan_ipaddr"); %><% nvram_match("wan_proto","disabled","-->"); %>}
