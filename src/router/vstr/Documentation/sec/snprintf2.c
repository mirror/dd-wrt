char buf[4];
int ret = snprintf(buf, sizeof(buf), "%s", "\x80\x80\x80\x80 123456789");
