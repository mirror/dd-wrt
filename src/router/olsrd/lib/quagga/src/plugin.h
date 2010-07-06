/*
 * OLSRd Quagga plugin
 *
 * Copyright (C) 2006-2008 Immo 'FaUl' Wehrenberg <immo@chaostreff-dortmund.de>
 * Copyright (C) 2007-2010 Vasilis Tsiligiannis <acinonyxs@yahoo.gr>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation or - at your option - under
 * the terms of the GNU General Public Licence version 2 but can be
 * linked to any BSD-Licenced Software with public available sourcecode
 *
 */

/* -------------------------------------------------------------------------
 * File               : plugin.h
 * Description        : header file for plugin.c
 * ------------------------------------------------------------------------- */

int zplugin_redistribute(const char*, void*, set_plugin_parameter_addon);
int zplugin_exportroutes(const char*, void*, set_plugin_parameter_addon);
int zplugin_distance(const char*, void*, set_plugin_parameter_addon);
int zplugin_localpref(const char*, void*, set_plugin_parameter_addon);
int zplugin_sockpath(const char*, void*, set_plugin_parameter_addon);
int zplugin_port(const char*, void*, set_plugin_parameter_addon);
int zplugin_version(const char*, void*, set_plugin_parameter_addon);

/*
 * Local Variables:
 * c-basic-offset: 2
 * indent-tabs-mode: nil
 * End:
 */
