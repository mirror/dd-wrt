{wl_mac::<% nvram_get("wl0_hwaddr"); %>}
{wl_ssid::<% nvram_get("wl_ssid"); %>}
{wl_channel::<% get_curchannel(); %>}
{wl_xmit::<% nvram_get("txpwr"); %> mW}
{wl_rate::<% get_currate(); %> Mbps}
{active_wireless::<% active_wireless(0); %>}
{active_wds::<% active_wds(0); %>}
{packet_info::<% wl_packet_get(); %>}