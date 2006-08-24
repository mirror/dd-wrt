{wl_mac::<% show_wl_mac(); %>}
{wl_ssid::<% nvram_get("wl_ssid"); %>}
{wl_channel::<% get_curchannel(); %>}
{wl_radio::<% get_radio_state(); %>}
{wl_xmit::<% nvram_get("txpwr"); %> mW}
{wl_rate::<% get_currate(); %>}
{active_wireless::<% active_wireless(0); %>}
{active_wds::<% active_wds(0); %>}
{packet_info::<% wl_packet_get(); %>}