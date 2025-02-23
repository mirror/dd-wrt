/*
 * Copyright (c) 2023 SUSE LLC
 * SPDX-License-Identifier: MIT
 */

#include <assert.h>
#include <dbus/dbus.h>

int main(void)
{
  DBusMessage *m = NULL;

  m = dbus_message_new_signal ("/", "com.example.Interface", "Signal");
  assert (m != NULL);
  dbus_message_unref (m);
  return 0;
}
