{wl_mac::<% nvram_get("wl0_hwaddr"); %>}
{wl_mode::<% nvram_match("wl_mode", "wet", "Client Bridge"); %><% nvram_match("wl_mode", "ap", "AP"); %><% nvram_match("wl_mode", "sta", "Client"); %><% nvram_match("wl_mode", "infra", "Adhoc"); %><% nvram_match("wl_mode", "apsta", "Repeater"); %>}
{wl_net_mode::<% nvram_match("wl_net_mode", "mixed", "Mixed"); %><% nvram_match("wl_net_mode", "g-only", "G-Only"); %><% nvram_match("wl_net_mode", "disabled","Disabled"); %><% nvram_match("wl_net_mode", "b-only", "B-Only"); %>}
{wl_ssid::<% nvram_get("wl_ssid"); %>}
{wl_channel::<% get_curchannel(); %>}
{wl_xmit::<% nvram_get("txpwr"); %> mW}
{wl_rate::<% get_currate(); %> Mbps}
{wl_security::<% nvram_match("security_mode", "disabled", "Disabled"); %><% nvram_invmatch("security_mode", "disabled", "Enabled,&nbsp;"); %><% nvram_match("security_mode", "psk", "WPA Pre-shared Key"); %><% nvram_match("security_mode", "wpa", "WPA RADIUS"); %><% nvram_match("security_mode", "psk2", "WPA2 Pre-Shared Key Only"); %><% nvram_match("security_mode", "wpa2", "WPA2 RADIUS Only"); %><% nvram_match("security_mode", "psk psk2", "WPA2 Pre-Shared Key Mixed"); %><% nvram_match("security_mode", "wpa wpa2", "WPA2 RADIUS Mixed"); %><% nvram_match("security_mode", "radius", "RADIUS"); %><% nvram_match("security_mode", "wep", "WEP"); %>}
{pptp::<% nvram_else_match("pptpd_connected", "1", "Connected", "Disconnected"); %>}
{active_wireless::<% active_wireless(0); %>}
{active_wds::<% active_wds(0); %>}