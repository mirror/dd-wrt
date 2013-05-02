static char *UPTIME(int uptime)
{
	int days, minutes;
	static char str[64] = { 0 };
	memset(str, 0, 64);
	days = uptime / (60 * 60 * 24);
	if (days)
		sprintf(str, "%d day%s, ", days, (days == 1 ? "" : "s"));
	minutes = uptime / 60;
	if (strlen(str) > 0)
		sprintf(str, "%s %d:%02d:%02d", str, (minutes / 60) % 24, minutes % 60, uptime % 60);
	else
		sprintf(str, "%d:%02d:%02d", (minutes / 60) % 24, minutes % 60, uptime % 60);
	return str;
}
