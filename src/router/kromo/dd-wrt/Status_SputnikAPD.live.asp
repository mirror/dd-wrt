{sputnik_status::<% sputnik_apd_status("scc_server"); %>}
{sputnik_state::<% sputnik_apd_status("phase"); %>}
{sputnik_serial::<% sputnik_apd_status("lsk_serial"); %>}
{uptime::<% get_uptime(); %>}
{ipinfo::<% show_wanipinfo(); %>}