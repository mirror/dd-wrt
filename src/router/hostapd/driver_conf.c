/* THIS FILE AUTOMATICALLY GENERATED, DO NOT EDIT! */
#include "includes.h"
#include "hostapd.h"
#include "driver.h"
void hostap_driver_register(void);
void madwifi_driver_register(void);
void prism54_driver_register(void);
void register_drivers(void) {
hostap_driver_register();
madwifi_driver_register();
prism54_driver_register();
}
