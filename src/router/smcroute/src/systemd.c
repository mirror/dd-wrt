/* systemd service monitor backend */
#include <unistd.h>
#include <systemd/sd-daemon.h>

void systemd_notify_ready(const char *status)
{
	sd_notifyf(0, "READY=1\nSTATUS=%s\nMAINPID=%lu\n", status, (unsigned long)getpid());
}

void systemd_notify_reload(const char *status)
{
	sd_notifyf(0, "RELOADING=1\nSTATUS=%s\n", status);
}

