{af_serviceid::<% nvram_get("af_serviceid"); %>}
{af_servicestatus::<% nvram_get("af_servicestatus"); %>}
{uptime::<% get_uptime(); %>}
{ipinfo::<% show_wanipinfo(); %>}