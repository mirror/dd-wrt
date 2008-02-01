{router_time::<% localtime(); %>}
{mem_info::<% dumpmeminfo(); %>}
{uptime::<% get_uptime(); %>}
{ip_conntrack::<% dumpip_conntrack(); %>}
{cpu_temp::<% get_cputemp(); %>}
{voltage::<% get_voltage(); %>}
{ipinfo::<% show_wanipinfo(); %>}