#!/usr/bin/perl -w

#
#  bluez-tools - a set of tools to manage bluetooth devices for linux
#
#  Copyright (C) 2010  Alexander Orlenko <zxteam@gmail.com>
#
#
#  This program is free software; you can redistribute it and/or modify
#  it under the terms of the GNU General Public License as published by
#  the Free Software Foundation; either version 2 of the License, or
#  (at your option) any later version.
#
#  This program is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#  GNU General Public License for more details.
#
#  You should have received a copy of the GNU General Public License
#  along with this program; if not, write to the Free Software
#  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
#

use strict;

die "usage: -header|source FILE <hid>\n" unless $ARGV[0] && $ARGV[1] && $ARGV[0] =~ /^-(header|source)$/;

sub parse_doc_api {
    my ($doc_api_file, $hierarchy_id) = @_;

    $hierarchy_id = 1 unless defined $hierarchy_id && $hierarchy_id > 0;

    my %data;

    open INPUT, "<$doc_api_file" or die "Can't open '$doc_api_file': $!\n";
    my $n = 0;
    my $hierarchy_n = 0;
    my $section = '';
    while(<INPUT>) {
        $n++;
        s/\s+$//;

        if ($n == 1) {
            if (/^BlueZ D-Bus (\S+\s{1})+API description$/) {
            	$data{'api'} = 'BlueZ';
                $data{'dbus_conn'} = 'system_conn';
            } elsif (/^obex.*?API description$/i) {
            	$data{'api'} = 'Obex';
                $data{'dbus_conn'} = 'session_conn';
            } elsif (/^Cycling Speed and Cadence API description$/) {
            	$data{'api'} = 'Cycling Speed';
                $data{'dbus_conn'} = 'system_conn';
            } elsif (/^Heart Rate API description$/) {
            	$data{'api'} = 'Heart Rate';
                $data{'dbus_conn'} = 'system_conn';
            } else {
                die "invalid file format (1)\n";
            }

            next;
        }

        /^\s*$/ && next;

        if (/^(.+) hierarchy$/) {
            my $hierarchy = $1;
            $section = 'hierarchy';
            $hierarchy_n++;
            my $dbus_conn = $data{'dbus_conn'};
            undef %data if $hierarchy_n == $hierarchy_id;
            last if $hierarchy_n == $hierarchy_id+1;
            $data{'hierarchy'} = $hierarchy;
            $data{'dbus_conn'} = $dbus_conn;
        } elsif (/^Service\s*(.+)$/) {
            my $service = $1;
            
            # Remove strings surrounded by brackets and parentheses
            $service =~ s/\s*((\(|\[|\{).*(\)|\]|\}))\s*//;
            
            die "invalid file format (2)\n" unless $section eq 'hierarchy';
            
            if($service eq 'org.bluez' || $service eq 'org.bluez.obex') {
            	$data{'serviceName'} = $service;
            } elsif ($service eq 'unique name') {
            	# $data{'serviceName'} = undef;
            	die "invalid service: User defined DBus object. Please create manually.\n";
            } else {
            	die "invalid service: $service\n";
            }
        } elsif (/^Interface\s*(.+)$/) {
            my $intf = $1;
            
            # Remove everything after the whitespace
            $intf =~ s/\s+.*//;
            
            die "invalid file format (3)\n" unless $section eq 'hierarchy';
            die "invalid interface: $intf\n" unless $intf =~ /^org\.bluez\./;
            $data{'intf'} = $intf;

            $data{$intf} = undef;
            $data{$intf}{'methods'} = undef;
            $data{$intf}{'signals'} = undef;
            $data{$intf}{'properties'} = undef;
        } elsif (/^Object path\s*(.+)/) {
            my $obj_path = $1;
            
            die "invalid file format (4)\n" unless $section eq 'hierarchy';

            if($obj_path =~ /^freely definable/ || $obj_path =~ /^\[.*\]/ || $obj_path =~ /\{.*\}/ || $obj_path =~ /dev_XX_XX_XX_XX_XX_XX/) {
            	$data{'objectPath'} = undef;
            } else {
            	$data{'objectPath'} = $obj_path if $obj_path =~ /^[A-Za-z0-9\/]+$/;
            }
        } elsif (/^Object name\s*(.+)/) {
            my $obj_name = $1;
            die "invalid file format (4)\n" unless $section eq 'hierarchy';
            $data{'objectName'} = $obj_name if $obj_name =~ /^[A-Za-z]+$/;
        } elsif (/^Methods/) {
            die "invalid file format (5)\n" unless $section eq 'hierarchy';
            $section = 'methods';
            s/Methods/       /;
        } elsif (/^Signals/) {
        	# Disabled for now. Some APIs do not have methods.
            # die "invalid file format (6)\n" unless $section eq 'methods';
            $section = 'signals';
            s/Signals/       /;
        } elsif(/^Properties/) {
        	# Disabled for now. Some APIs do not have methods or signals.
            # die "invalid file format (7)\n" unless $section eq 'signals' || $section eq 'methods';
            $section = 'properties';
            s/Properties/          /;
        } elsif(/^Filter/) {
            $section = 'filter';
            s/Filter/          /;
        }

        if (defined $section && $section eq 'methods' && /^\s+((\S+) (\w+)\((.*)\)( \[(\w+)\])?)$/) {
            my $decl = $1;
            my $ret = $2;
            my $name = $3;
            my $args = $4;
            my $flag = $6;
            
            # Check for void parameters
            if(lc($args) eq 'void') {
            	$args = '';
            }

            $data{$data{'intf'}}{'methods'}{$name}{'decl'} = $decl;
            $data{$data{'intf'}}{'methods'}{$name}{'ret'} = $ret;
            $data{$data{'intf'}}{'methods'}{$name}{'flag'} = $flag;
            @{$data{$data{'intf'}}{'methods'}{$name}{'args'}} = map {type => (split / /, $_)[0], name => (split / /, $_)[1]}, (split /, /, $args);
        } elsif (defined $section && $section eq 'signals' && /^\s+((\w+)\((.*)\))$/) {
            my $decl = $1;
            my $name = $2;
            my $args = $3;

            $data{$data{'intf'}}{'signals'}{$name}{'decl'} = $decl;
            @{$data{$data{'intf'}}{'signals'}{$name}{'args'}} = map {type => (split / /, $_)[0], name => (split / /, $_)[1]}, (split /, /, $args);
        } elsif (defined $section && $section eq 'properties' && /^\s+((\S+) (\w+) \[(readonly|writeonly|readwrite|read\/write).*\])$/) {
            my $decl = $1;
            my $type = $2;
            my $name = $3;
            my $mode = $4;
            my $optional = 0;
            if ($decl =~ /\(optional\)/i or $decl =~ /\[.*optional\]/i) {
            	$optional = 1;
            }
            
            $data{$data{'intf'}}{'properties'}{$name}{'decl'} = $decl;
            $data{$data{'intf'}}{'properties'}{$name}{'type'} = $type;
            $data{$data{'intf'}}{'properties'}{$name}{'mode'} = $mode;
            $data{$data{'intf'}}{'properties'}{$name}{'optional'} = $optional;
        } elsif (defined $section && $section eq 'filter' && /^\s+((\S+) (\w+)):$/) {
            my $decl = $1;
            my $type = $2;
            my $name = $3;

            $data{$data{'intf'}}{'filter'}{$name}{'decl'} = $decl;
            $data{$data{'intf'}}{'filter'}{$name}{'type'} = $type;
        }
    }

    return \%data;
}

my $HEADER = <<EOH;
/*
 *
 *  bluez-tools - a set of tools to manage bluetooth devices for linux
 *
 *  Copyright (C) 2010  Alexander Orlenko <zxteam\@gmail.com>
 *
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */
EOH

sub get_g_type {
    my $obj_type = shift;
    my $g_type;

    $g_type = 'void ' if $obj_type eq 'void';
    $g_type = 'gchar *' if $obj_type eq 'object' || $obj_type eq 'string';
    # $g_type = 'GHashTable *' if $obj_type =~ /^dict/; # May be used later...
    $g_type = 'GVariant *' if $obj_type =~ /^dict/; # GVariant dictionary
    $g_type = 'GVariant *' if $obj_type eq 'variant';
    $g_type = 'guint8 ' if $obj_type eq 'uint8' || $obj_type eq 'byte';
    $g_type = 'gboolean ' if $obj_type eq 'boolean';
    $g_type = 'gint16 ' if $obj_type eq 'int16';
    $g_type = 'guint16 ' if $obj_type eq 'uint16';
    $g_type = 'gint32 ' if $obj_type eq 'int32';
    $g_type = 'guint32 ' if $obj_type eq 'uint32' || $obj_type eq 'fd';
    $g_type = 'guint64 ' if $obj_type eq 'uint64';
    # $g_type = 'GPtrArray *' if $obj_type eq 'array{object}' || $obj_type eq 'array{dict}';
    $g_type = 'gchar **' if $obj_type eq 'array{object}';
    $g_type = 'GVariant *' if $obj_type eq 'array{dict}';
    $g_type = 'gchar **' if $obj_type eq 'array{string}';
    $g_type = 'guint8 *' if $obj_type eq 'array{byte}';

    die "unknown object type (1): $obj_type\n" unless defined $g_type;

    return $g_type;
}

sub get_default_value {
    my $c_type = shift;
    my $default_value;

	$default_value = '0x0';
    $default_value = 'NULL' if $c_type =~ /\*$/;
    $default_value = 'FALSE' if $c_type =~ /boolean/;
    $default_value = '0' if $c_type =~ /int/;

    die "unknown C type (3): $c_type\n" unless defined $default_value;

    return $default_value;
}

sub is_const_type {
	my $obj_type = shift;

	if($obj_type eq 'object' || $obj_type eq 'string' || $obj_type eq 'array{object}' || $obj_type eq 'array{string}') {
		return 1;
	} else {
		return 0;
	}
}

sub get_g_variant_format_char {
	my $obj_type = shift;
    my $g_variant_type;

    $g_variant_type = 'o' if $obj_type eq 'object';
    $g_variant_type = 's' if $obj_type eq 'string';
    $g_variant_type = '@a{sv}' if $obj_type =~ /^dict/;
    $g_variant_type = 'v' if $obj_type eq 'variant';
    $g_variant_type = 'y' if $obj_type eq 'uint8' || $obj_type eq 'byte';
    $g_variant_type = 'b' if $obj_type eq 'boolean';
    $g_variant_type = 'n' if $obj_type eq 'int16';
    $g_variant_type = 'q' if $obj_type eq 'uint16';
    $g_variant_type = 'h' if $obj_type eq 'int32' || $obj_type eq 'fd';
    $g_variant_type = 'u' if $obj_type eq 'uint32';
    $g_variant_type = 't' if $obj_type eq 'uint64';
    $g_variant_type = '@ao' if $obj_type eq 'array{object}';
    # $g_variant_type = 'a&v' if $obj_type eq 'array{dict}';
    $g_variant_type = '@as' if $obj_type eq 'array{string}';
    $g_variant_type = '@ay' if $obj_type eq 'array{byte}';

    die "unknown object type (1): $obj_type\n" unless defined $g_variant_type;

    return $g_variant_type;
}

sub generate_g_variant_params {
	my @params = @{$_[0]};
	my $variant;
	
	# Size greater than 0?
	die "Too few arguments\n" unless @params > 0;
	
	$variant = 'g_variant_new (';
	$variant .= '"(';
	foreach(@params) {
		$variant .= get_g_variant_format_char($_->{'type'});
	}
	$variant .= ')", ';
	
	for (my $i=0; $i < @params; $i++) {
		$variant .= '(guint64) ' if $params[$i]{'type'} eq 'uint64';
		$variant .= '(gint64) ' if $params[$i]{'type'} eq 'int64';
		$variant .= $params[$i]{'name'};
		$variant .= ', ' unless $i == (@params - 1);
	}
	$variant .= ')';
	
	return $variant;
}

sub generate_header {
    my $node = shift;

    my $HEADER_TEMPLATE = <<EOT;
#ifndef __{\$OBJECT}_H
#define __{\$OBJECT}_H

#ifdef	__cplusplus
extern "C" {
#endif

#include <glib-object.h>

{DBUS_OBJECT_DEFS}
/*
 * Type macros
 */
#define {\$OBJECT}_TYPE				({\$object}_get_type())
#define {\$OBJECT}(obj)				(G_TYPE_CHECK_INSTANCE_CAST((obj), {\$OBJECT}_TYPE, {\$Object}))
#define {\$OBJECT}_IS(obj)				(G_TYPE_CHECK_INSTANCE_TYPE((obj), {\$OBJECT}_TYPE))
#define {\$OBJECT}_CLASS(klass)		(G_TYPE_CHECK_CLASS_CAST((klass), {\$OBJECT}_TYPE, {\$Object}Class))
#define {\$OBJECT}_IS_CLASS(klass)		(G_TYPE_CHECK_CLASS_TYPE((klass), {\$OBJECT}_TYPE))
#define {\$OBJECT}_GET_CLASS(obj)		(G_TYPE_INSTANCE_GET_CLASS((obj), {\$OBJECT}_TYPE, {\$Object}Class))

typedef struct _{\$Object} {\$Object};
typedef struct _{\$Object}Class {\$Object}Class;
typedef struct _{\$Object}Private {\$Object}Private;

struct _{\$Object} {
	GObject parent_instance;

	/*< private >*/
	{\$Object}Private *priv;
};

struct _{\$Object}Class {
	GObjectClass parent_class;
};

/* used by {\$OBJECT}_TYPE */
GType {\$object}_get_type(void) G_GNUC_CONST;

/*
 * Constructor
 */
{CONSTRUCTOR_DEFS}
{IF_METHODS}
/*
 * Method definitions
 */
{FI_METHODS}
{METHOD_DEFS}

#ifdef	__cplusplus
}
#endif

#endif /* __{\$OBJECT}_H */
EOT

    my $intf = $node->{'intf'};
    my $obj = exists $node->{'objectName'} ? $node->{'objectName'} : (split /\./, $intf)[-1];
    
    # Because the BlueZ 5 API uses version numbers at the end of the interfaces, trim off the version number 
    $obj =~ s/\d+$//;
    # TODO: In the future, when BlueZ 5 includes mutiple versions of the same interface, we should distinguish them.
    
    # Prefix the Obex API calls to avoid conflicts with the BlueZ API
	$obj = "Obex".$obj if $intf =~ /obex/i;
    
    # TODO: In the future, when BlueZ 5 includes mutiple versions of the same interface, we should distinguish them.
    my $obj_lc = lc join('_', $obj =~ /([A-Z]+[a-z]*)/g);
    my $obj_uc = uc join('_', $obj =~ /([A-Z]+[a-z]*)/g);
    
    # Prefix the Obex API calls to avoid conflicts with the BlueZ API
     

    my $dbus_object_defs = "";
    $dbus_object_defs .= "#define {\$OBJECT}_DBUS_SERVICE \"$node->{'serviceName'}\"\n"  if defined $node->{'serviceName'};
    $dbus_object_defs .= "#define {\$OBJECT}_DBUS_INTERFACE \"$node->{'intf'}\"\n";
    $dbus_object_defs .= "#define {\$OBJECT}_DBUS_PATH \"$node->{'objectPath'}\"\n"  if defined $node->{'objectPath'};

	my $constructor_defs = "{\$Object} *{\$object}_new(";
	my $constructor_args = "";
	
	if(not exists $node->{'objectPath'} or not defined $node->{'objectPath'}) {
		$constructor_args .= ", " if length($constructor_args) > 0;
		$constructor_args .= "const gchar *dbus_object_path"
	}
	
	$constructor_defs .= $constructor_args;
	$constructor_defs .= ");\n";

    my $method_defs = "";

    $method_defs .= "const gchar *{\$object}_get_dbus_object_path({\$Object} *self);\n" unless defined $node->{'objectPath'};
    $method_defs .= "\n" if length($method_defs) > 0;
    
    for my $method (sort keys %{$node->{$intf}{'methods'}}) {
        my @a = $method =~ /([A-Z]+[a-z]*)/g;
        my %m = %{$node->{$intf}{'methods'}{$method}};

        my $in_args = join ', ', (map "const ".get_g_type($_->{'type'}).$_->{'name'}, @{$m{'args'}});
        
		$method_defs .=
		(is_const_type($m{'ret'}) eq 1 ? "const " : "").get_g_type($m{'ret'})."{\$object}_".(join '_', (map lc $_, @a))."({\$Object} *self, ".
		($in_args eq '' ? "" : "$in_args, ")."GError **error);\n";
    }
    
    $method_defs .= "\n" if %{$node->{$intf}{'methods'}};
    
    # Add Properties interface definitions
    if(keys(%{$node->{$intf}{'properties'}}) > 0) {
	    $method_defs .= "GVariant *{\$object}_get_properties({\$Object} *self, GError **error);\n";
	    $method_defs .= "void {\$object}_set_property({\$Object} *self, const gchar *name, const GVariant *value, GError **error);\n";
		$method_defs .= "\n";
	}
    
    for my $property (sort keys %{$node->{$intf}{'properties'}}) {
        my @a = $property =~ /([A-Z]+[a-z]*)/g;
        my %p = %{$node->{$intf}{'properties'}{$property}};

		# If the property is named 'Type', rename it to something else as it will conflict with GLib's 'Type' property
		if($property =~ /^type$/i) {
			my @i_name = $obj =~ /([A-Z]+[a-z]*)/g;
			my @new_name = ($i_name[-1], @a);
			$method_defs .= "// This has been renamed because '{\$object}_get_type' is already used by GLib\n";
			if ($p{'mode'} eq 'readwrite' or $p{'mode'} eq 'readonly' or $p{'mode'} eq 'read/write') {
				$method_defs .= (is_const_type($p{'type'}) eq 1 ? "const " : "").get_g_type($p{'type'})."{\$object}_get_".(join '_', (map lc $_, @new_name))."({\$Object} *self, GError **error);\n";
			}
			if ($p{'mode'} eq 'readwrite' or $p{'mode'} eq 'writeonly' or $p{'mode'} eq 'read/write') {
				$method_defs .= "void {\$object}_set_".(join '_', (map lc $_, @new_name))."({\$Object} *self, const ".get_g_type($p{'type'})."value, GError **error);\n";
			}
		} else {
			if ($p{'mode'} eq 'readwrite' or $p{'mode'} eq 'readonly' or $p{'mode'} eq 'read/write') {
				$method_defs .= (is_const_type($p{'type'}) eq 1 ? "const " : "").get_g_type($p{'type'})."{\$object}_get_".(join '_', (map lc $_, @a))."({\$Object} *self, GError **error);\n";
			}
			if ($p{'mode'} eq 'readwrite' or $p{'mode'} eq 'writeonly' or $p{'mode'} eq 'read/write') {
				$method_defs .= "void {\$object}_set_".(join '_', (map lc $_, @a))."({\$Object} *self, const ".get_g_type($p{'type'})."value, GError **error);\n";
			}
		}
    }

    $method_defs =~ s/\s+$//s;

    my $output = "$HEADER\n$HEADER_TEMPLATE\n";
    $output =~ s/{DBUS_OBJECT_DEFS}/$dbus_object_defs/;
    $output =~ s/{CONSTRUCTOR_DEFS}/$constructor_defs/;
    if (scalar keys %{$node->{$intf}{'methods'}} > 0 or scalar keys %{$node->{$intf}{'properties'}} > 0) {
        $output =~ s/\{IF_METHODS\}\s+(.+?)\s+\{FI_METHODS\}/$1/gs;
    } else {
        $output =~ s/\s+\{IF_METHODS\}.+?\{FI_METHODS\}//gs;
    }
    $output =~ s/{METHOD_DEFS}/$method_defs/;
    $output =~ s/{\$OBJECT}/$obj_uc/g;
    $output =~ s/{\$Object}/$obj/g;
    $output =~ s/{\$object}/$obj_lc/g;

    return $output;
}

sub generate_source {
    my $node = shift;

    my $SOURCE_TEMPLATE = <<EOT;
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif
#include <gio/gio.h>
#include <glib.h>
#include <string.h>

#include "../dbus-common.h"
#include "../properties.h"

#include "{\$object}.h"

#define {\$OBJECT}_GET_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE((obj), {\$OBJECT}_TYPE, {\$Object}Private))

struct _{\$Object}Private {
	GDBusProxy *proxy;
	{IF_PROPERTIES}
	Properties *properties;
	{FI_PROPERTIES}
	{IF_NO_OBJECT_PATH}
	gchar *object_path;
	{FI_NO_OBJECT_PATH}
};

G_DEFINE_TYPE_WITH_PRIVATE({\$Object}, {\$object}, G_TYPE_OBJECT);

enum {
	PROP_0,
{CONSTRUCTOR_PROPERTIES}
};

static void _{\$object}_get_property(GObject *object, guint property_id, GValue *value, GParamSpec *pspec);
static void _{\$object}_set_property(GObject *object, guint property_id, const GValue *value, GParamSpec *pspec);

static void _{\$object}_create_gdbus_proxy({\$Object} *self, const gchar *dbus_service_name, const gchar *dbus_object_path, GError **error);

static void {\$object}_dispose(GObject *gobject)
{
	{\$Object} *self = {\$OBJECT}(gobject);

	/* Proxy free */
	g_clear_object (&self->priv->proxy);
	{IF_PROPERTIES_EXT}
	/* Properties free */
	g_clear_object(&self->priv->properties);
	{FI_PROPERTIES_EXT}
	{IF_NO_OBJECT_PATH}
	/* Object path free */
	g_free(self->priv->object_path);
	{FI_NO_OBJECT_PATH}
	/* Chain up to the parent class */
	G_OBJECT_CLASS({\$object}_parent_class)->dispose(gobject);
}

static void {\$object}_finalize (GObject *gobject)
{
	{\$Object} *self = {\$OBJECT}(gobject);
	G_OBJECT_CLASS({\$object}_parent_class)->finalize(gobject);
}

static void {\$object}_class_init({\$Object}Class *klass)
{
	GObjectClass *gobject_class = G_OBJECT_CLASS(klass);

	gobject_class->dispose = {\$object}_dispose;

	/* Properties registration */
	GParamSpec *pspec = NULL;

	gobject_class->get_property = _{\$object}_get_property;
	gobject_class->set_property = _{\$object}_set_property;
	
	{IF_NO_OBJECT_PATH}
	/* object DBusObjectPath [readwrite, construct only] */
	pspec = g_param_spec_string("DBusObjectPath", "dbus_object_path", "{\$Object} D-Bus object path", NULL, G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY);
	g_object_class_install_property(gobject_class, PROP_DBUS_OBJECT_PATH, pspec);
	{FI_NO_OBJECT_PATH}
	if (pspec)
		g_param_spec_unref(pspec);
}

static void {\$object}_init({\$Object} *self)
{
	self->priv = {\$object}_get_instance_private (self);
	self->priv->proxy = NULL;
	{IF_PROPERTIES}
	self->priv->properties = NULL;
	{FI_PROPERTIES}
	{IF_NO_OBJECT_PATH}
	self->priv->object_path = NULL;
	{FI_NO_OBJECT_PATH}
	g_assert({\$conn} != NULL);
{CONSTRUCTOR_CALL}
}

static void _{\$object}_get_property(GObject *object, guint property_id, GValue *value, GParamSpec *pspec)
{
	{\$Object} *self = {\$OBJECT}(object);

	switch (property_id) {
{CONSTRUCTOR_GETTERS}
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
		break;
	}
}

static void _{\$object}_set_property(GObject *object, guint property_id, const GValue *value, GParamSpec *pspec)
{
	{\$Object} *self = {\$OBJECT}(object);
	GError *error = NULL;

	switch (property_id) {
{CONSTRUCTOR_SETTERS}
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
		break;
	}

	if (error != NULL)
		g_critical("%s", error->message);

	g_assert(error == NULL);
}

/* Constructor */
{CONSTRUCTOR}
/* Private DBus proxy creation */
static void _{\$object}_create_gdbus_proxy({\$Object} *self, const gchar *dbus_service_name, const gchar *dbus_object_path, GError **error)
{
	g_assert({\$OBJECT}_IS(self));
	self->priv->proxy = g_dbus_proxy_new_sync({\$conn}, G_DBUS_PROXY_FLAGS_NONE, NULL, dbus_service_name, dbus_object_path, {\$OBJECT}_DBUS_INTERFACE, NULL, error);

	if(self->priv->proxy == NULL)
		return;

	{IF_PROPERTIES}
	self->priv->properties = g_object_new(PROPERTIES_TYPE, "DBusType", {\$dbus_type}, "DBusServiceName", dbus_service_name, "DBusObjectPath", dbus_object_path, NULL);
	g_assert(self->priv->properties != NULL);
	{FI_PROPERTIES}
}

{IF_METHODS}
/* Methods */
{FI_METHODS}

{IF_NO_OBJECT_PATH}
/* Get DBus object path */
const gchar *{\$object}_get_dbus_object_path({\$Object} *self)
{
	g_assert({\$OBJECT}_IS(self));
	g_assert(self->priv->proxy != NULL);
	return g_dbus_proxy_get_object_path(self->priv->proxy);
}
{FI_NO_OBJECT_PATH}

{METHODS}

{IF_PROPERTIES}
/* Properties access methods */
{PROPERTIES_ACCESS_METHODS}
{FI_PROPERTIES}
EOT

    my $intf = $node->{'intf'};
    my $obj = exists $node->{'objectName'} ? $node->{'objectName'} : (split /\./, $intf)[-1];
    
    # Prefix the Obex API calls to avoid conflicts with the BlueZ API
	$obj = "Obex".$obj if $intf =~ /obex/i;
    
    # Because the BlueZ 5 API uses version numbers at the end of the interfaces, trim off the version number 
    $obj =~ s/\d+$//;
    # TODO: In the future, when BlueZ 5 includes mutiple versions of the same interface, we should distinguish them.
    
    my $obj_lc = lc join('_', $obj =~ /([A-Z]+[a-z]*)/g);
    my $obj_uc = uc join('_', $obj =~ /([A-Z]+[a-z]*)/g);
    
    my $constructor_properties = "";
    if(not exists $node->{'objectPath'} or not defined $node->{'objectPath'}) {
    	$constructor_properties .= "\tPROP_DBUS_OBJECT_PATH";
    	$constructor_properties .= " /* readwrite, construct only */\n";
    }
    
    my $constructor_call = "";
    if(exists $node->{'serviceName'} and defined $node->{'serviceName'} and exists $node->{'objectPath'} and defined $node->{'objectPath'}) {
    	$constructor_call .=
    	"\tGError *error = NULL;\n".
    	"\t_{\$object}_create_gdbus_proxy(self, {\$OBJECT}_DBUS_SERVICE, {\$OBJECT}_DBUS_PATH, &error);\n".
    	"\tg_assert(error == NULL);\n";
    }
    
    my $constructor_getters = "";
	if(not exists $node->{'objectPath'} or not defined $node->{'objectPath'}) {
	    $constructor_getters .= "\tcase PROP_DBUS_OBJECT_PATH:\n";
		$constructor_getters .= "\t\tg_value_set_string(value, {\$object}_get_dbus_object_path(self));\n";
		$constructor_getters .= "\t\tbreak;\n";
	}
	
	my $constructor_setters = "";
	if(not exists $node->{'objectPath'} or not defined $node->{'objectPath'}) {
	    $constructor_setters .= 
	    "\tcase PROP_DBUS_OBJECT_PATH:\n".
		"\t\tself->priv->object_path = g_value_dup_string(value);\n";
		$constructor_setters .=
		"\t\t_{\$object}_create_gdbus_proxy(self, {\$OBJECT}_DBUS_SERVICE, self->priv->object_path, &error);\n";
		$constructor_setters .= "\t\tbreak;\n";
	}

	my $constructor_def = "{\$Object} *{\$object}_new(";
	my $constructor_args = "";
	
	if(not exists $node->{'objectPath'} or not defined $node->{'objectPath'}) {
		$constructor_args .= ", " if length($constructor_args) > 0;
		$constructor_args .= "const gchar *dbus_object_path"
	}
	
	$constructor_def .= $constructor_args;
	$constructor_def .=
	")\n".
	"{\n".
	"\treturn g_object_new({\$OBJECT}_TYPE, ";
	if(not exists $node->{'objectPath'} or not defined $node->{'objectPath'}) {
		$constructor_def .= "\"DBusObjectPath\", dbus_object_path, ";
	}
	$constructor_def .=
	"NULL);\n".
	"}\n";
	
	my $dbus_type = "NULL";
	
	if(exists $node->{'dbus_conn'} and defined $node->{'dbus_conn'}) {
		$dbus_type = "\"system\"" if $node->{'dbus_conn'} eq 'system_conn';
		$dbus_type = "\"session\"" if $node->{'dbus_conn'} eq 'session_conn';
	} else {
		die '$node->{\'dbus_conn\'} is undefined!';
	}

    my $methods = "";

    for my $method (sort keys %{$node->{$intf}{'methods'}}) {
        my @a = $method =~ /([A-Z]+[a-z]*)/g;
        my %m = %{$node->{$intf}{'methods'}{$method}};

        my $in_args = join ', ', (map "const ".get_g_type($_->{'type'}).$_->{'name'}, @{$m{'args'}});

		my $method_def =
		(is_const_type($m{'ret'}) eq 1 ? "const " : "").get_g_type($m{'ret'})."{\$object}_".(join '_', (map lc $_, @a))."({\$Object} *self, ".
		($in_args eq '' ? "" : "$in_args, ")."GError **error)";

		$methods .=
			"/* $m{'decl'} */\n".
            "$method_def\n".
            "{\n".
            "\tg_assert({\$OBJECT}_IS(self));\n";
            
		if($m{'ret'} eq 'void') {
			$methods .= "\tg_dbus_proxy_call_sync(self->priv->proxy, \"$method\", ".($in_args eq '' ? "NULL" : generate_g_variant_params($m{'args'})).", G_DBUS_CALL_FLAGS_NONE, -1, NULL, error);\n";
		} else {				
			$methods .= "\t".(is_const_type($m{'ret'}) eq 1 ? "const " : "").get_g_type($m{'ret'})."ret = ".get_default_value(get_g_type($m{'ret'})).";\n".
				"\tGVariant *proxy_ret = g_dbus_proxy_call_sync(self->priv->proxy, \"$method\", ".($in_args eq '' ? "NULL" : generate_g_variant_params($m{'args'})).", G_DBUS_CALL_FLAGS_NONE, -1, NULL, error);\n".
				"\tif (proxy_ret != NULL)\n".
				"\t\treturn ".get_default_value(get_g_type($m{'ret'})).";\n".
				"\tproxy_ret = g_variant_get_child_value(proxy_ret, 0);\n";
				
			if($m{'ret'} eq 'boolean') {
				$methods .= "\tret = g_variant_get_boolean(proxy_ret);\n";
			} elsif($m{'ret'} eq 'byte') {
				$methods .= "\tret = g_variant_get_byte(proxy_ret);\n";
			} elsif($m{'ret'} eq 'int8') {
				$methods .= "\tret = (gint8) g_variant_get_byte(proxy_ret);\n";
			} elsif($m{'ret'} eq 'uint8') {
				$methods .= "\tret = (guint8) g_variant_get_byte(proxy_ret);\n";
			} elsif($m{'ret'} eq 'int16') {
				$methods .= "\tret = g_variant_get_int16(proxy_ret);\n";
			} elsif($m{'ret'} eq 'uint16') {
				$methods .= "\tret = g_variant_get_uint16(proxy_ret);\n";
			} elsif($m{'ret'} eq 'int32') {
				$methods .= "\tret = g_variant_get_int32(proxy_ret);\n";
			} elsif($m{'ret'} eq 'uint32') {
				$methods .= "\tret = g_variant_get_uint32(proxy_ret);\n";
			} elsif($m{'ret'} eq 'fd') {
				$methods .= "\tret = g_variant_get_uint32(proxy_ret);\n";
			} elsif($m{'ret'} eq 'int64') {
				$methods .= "\tret = g_variant_get_int64(proxy_ret);\n";
			} elsif($m{'ret'} eq 'uint64') {
				$methods .= "\tret = g_variant_get_uint64(proxy_ret);\n";
			} elsif($m{'ret'} eq 'float' || $m{'ret'} eq 'double') {
				$methods .= "\tret = g_variant_get_double(proxy_ret);\n";
			} elsif($m{'ret'} eq 'object' || $m{'ret'} eq 'string') {
				$methods .= "\tret = g_variant_get_string(proxy_ret, NULL);\n";
			} elsif($m{'ret'} eq 'variant') {
				$methods .= "\tret = g_variant_get_variant(proxy_ret);\n";
			} elsif($m{'ret'} eq 'array{string}') {
				$methods .= "\tret = g_variant_get_strv(proxy_ret, NULL);\n";
			} elsif($m{'ret'} eq 'array{object}') {
				$methods .= "\tret = g_variant_get_objv(proxy_ret, NULL);\n";
			} elsif($m{'ret'} eq 'array{byte}') {
				$methods .= "\tret = (guint8 *) g_variant_get_fixed_array(proxy_ret, NULL, sizeof(guint8));\n";
			} else {
				# die "Unknown method return type: $m{'ret'}\n";
				
				# Assume this is an array of variants or dictionaries or something else
				$methods .= "\tret = g_variant_ref_sink(proxy_ret);\n";
			}
				
			$methods .= "\tg_variant_unref(proxy_ret);\n".
			"\treturn ret;\n";
		}
		
		$methods .= "}\n\n";
    }

    $methods =~ s/\s+$//s;

    my $properties_access_methods = "";
    
    if(keys(%{$node->{$intf}{'properties'}}) > 0) {
    	$properties_access_methods .=
    	"GVariant *{\$object}_get_properties({\$Object} *self, GError **error)\n".
    	"{\n".
    	"\tg_assert({\$OBJECT}_IS(self));\n".
    	"\tg_assert(self->priv->properties != NULL);\n".
    	"\treturn properties_get_all(self->priv->properties, {\$OBJECT}_DBUS_INTERFACE, error);\n".
    	"}\n".
    	"\n".
    	"void {\$object}_set_property({\$Object} *self, const gchar *name, const GVariant *value, GError **error)\n".
    	"{\n".
    	"\tg_assert({\$OBJECT}_IS(self));\n".
    	"\tg_assert(self->priv->properties != NULL);\n".
    	"\tproperties_set(self->priv->properties, {\$OBJECT}_DBUS_INTERFACE, name, value, error);\n".
    	"}\n".
    	"\n";
    }
    
    for my $property (sort keys %{$node->{$intf}{'properties'}}) {
        my @a = $property =~ /([A-Z]+[a-z]*)/g;
        my %p = %{$node->{$intf}{'properties'}{$property}};

        # my $property_var = join '_', (map lc $_, @a);
        my $property_var = '';
        
        # If the property is named 'Type', rename it to something else as it will conflict with GLib's 'Type' property
        if($property =~ /^type$/i) {
        	my @i_name = $obj =~ /([A-Z]+[a-z]*)/g;
			my @new_name = ($i_name[-1], @a);
			$property_var = join '_', (map lc $_, @new_name);
        } else {
        	$property_var = join '_', (map lc $_, @a);
        }
        
        my $enum = "PROP_".(join '_', (map uc $_, @a));
        my $property_get_method = "{\$object}_get_$property_var";
        my $property_set_method = "{\$object}_set_$property_var";

		if ($p{'mode'} eq 'readwrite' or $p{'mode'} eq 'readonly' or $p{'mode'} eq 'read/write') {
	        $properties_access_methods .=
	        (is_const_type($p{'type'}) eq 1 ? "const " : "").get_g_type($p{'type'})."$property_get_method({\$Object} *self, GError **error)\n".
	        "{\n".
	        "\tg_assert({\$OBJECT}_IS(self));\n".
	        "\tg_assert(self->priv->properties != NULL);\n".
	        "\tGVariant *prop = properties_get(self->priv->properties, {\$OBJECT}_DBUS_INTERFACE, \"$property\", error);\n".
	        "\tif(prop == NULL)\n".
	        "\t\treturn ".get_default_value(get_g_type($p{'type'})).";\n";
	        
	        if($p{'type'} eq 'boolean') {
				$properties_access_methods .= "\tgboolean ret = g_variant_get_boolean(prop);\n";
			} elsif($p{'type'} eq 'byte') {
				$properties_access_methods .= "\tguchar ret = g_variant_get_byte(prop);\n";
			} elsif($p{'type'} eq 'int8') {
				$properties_access_methods .= "\tgint8 ret = g_variant_get_byte(prop);\n";
			} elsif($p{'type'} eq 'uint8') {
				$properties_access_methods .= "\tguint8 ret = g_variant_get_byte(prop);\n";
			} elsif($p{'type'} eq 'int16') {
				$properties_access_methods .= "\tgint16 ret = g_variant_get_int16(prop);\n";
			} elsif($p{'type'} eq 'uint16') {
				$properties_access_methods .= "\tguint16 ret = g_variant_get_uint16(prop);\n";
			} elsif($p{'type'} eq 'int32') {
				$properties_access_methods .= "\tgint32 ret = g_variant_get_int32(prop);\n";
			} elsif($p{'type'} eq 'uint32') {
				$properties_access_methods .= "\tguint32 ret = g_variant_get_uint32(prop);\n";
			} elsif($p{'type'} eq 'fd') {
				$properties_access_methods .= "\tguint32 ret = g_variant_get_uint32(prop);\n";
			} elsif($p{'type'} eq 'int64') {
				$properties_access_methods .= "\tgint64 ret = g_variant_get_int64(prop);\n";
			} elsif($p{'type'} eq 'uint64') {
				$properties_access_methods .= "\tguint64 ret = g_variant_get_uint64(prop);\n";
			} elsif($p{'type'} eq 'float' || $p{'type'} eq 'double') {
				$properties_access_methods .= "\tgdouble ret = g_variant_get_double(prop);\n";
			} elsif($p{'type'} eq 'object' || $p{'type'} eq 'string') {
				$properties_access_methods .= "\tconst gchar *ret = g_variant_get_string(prop, NULL);\n";
			} elsif($p{'type'} eq 'variant') {
				$properties_access_methods .= "\tGVariant *ret = g_variant_get_variant(prop);\n";
			} elsif($p{'type'} eq 'array{string}') {
				$properties_access_methods .= "\tconst gchar **ret = g_variant_get_strv(prop, NULL);\n";
			} elsif($p{'type'} eq 'array{object}') {
				$properties_access_methods .= "\tconst gchar **ret = g_variant_get_objv(prop, NULL);\n";
			} elsif($p{'type'} eq 'array{byte}') {
				$properties_access_methods .= "\tguint8 *ret = (guint8 *) g_variant_get_fixed_array(prop, NULL, sizeof(guint8));\n";
			} else {
				# die "Unknown property return type: $p{'type'}\n";
				
				# Assume this is an array of variants or dictionaries or something else
				$properties_access_methods .= "\tGVariant *ret = g_variant_ref_sink(prop);\n";
			}
	        
	        $properties_access_methods .=
	        "\tg_variant_unref(prop);\n".
	        "\treturn ret;\n".
	        "}\n\n";
		}

        if ($p{'mode'} eq 'readwrite' or $p{'mode'} eq 'writeonly' or $p{'mode'} eq 'read/write') {
            $properties_access_methods .=
            "void $property_set_method({\$Object} *self, const ".get_g_type($p{'type'})."value, GError **error)\n".
            "{\n".
            "\tg_assert({\$OBJECT}_IS(self));\n".
            "\tg_assert(self->priv->properties != NULL);\n".
	        "\tproperties_set(self->priv->properties, {\$OBJECT}_DBUS_INTERFACE, \"$property\", ";
	        
	        if($p{'type'} eq 'boolean') {
				$properties_access_methods .= "g_variant_new_boolean(value)";
			} elsif($p{'type'} eq 'byte') {
				$properties_access_methods .= "g_variant_new_byte(value)";
			} elsif($p{'type'} eq 'uint8') {
				$properties_access_methods .= "g_variant_new_byte(value)";
			} elsif($p{'type'} eq 'int16') {
				$properties_access_methods .= "g_variant_new_int16(value)";
			} elsif($p{'type'} eq 'uint16') {
				$properties_access_methods .= "g_variant_new_uint16(value)";
			} elsif($p{'type'} eq 'int32') {
				$properties_access_methods .= "g_variant_new_int32(value)";
			} elsif($p{'type'} eq 'uint32') {
				$properties_access_methods .= "g_variant_new_uint32(value)";
			} elsif($p{'type'} eq 'int64') {
				$properties_access_methods .= "g_variant_new_int64(value)";
			} elsif($p{'type'} eq 'uint64') {
				$properties_access_methods .= "g_variant_new_uint64(value)";
			} elsif($p{'type'} eq 'double') {
				$properties_access_methods .= "g_variant_new_double(value)";
			} elsif($p{'type'} eq 'object') {
				$properties_access_methods .= "g_variant_new_object_path(value)";
			} elsif($p{'type'} eq 'string') {
				$properties_access_methods .= "g_variant_new_string(value)";
			} elsif($p{'type'} eq 'variant') {
				$properties_access_methods .= "g_variant_new_variant(value)";
			} elsif($p{'type'} eq 'array{string}') {
				$properties_access_methods .= "g_variant_new_strv(value, -1)";
			} elsif($p{'type'} eq 'array{object}') {
				$properties_access_methods .= "g_variant_new_objv(value, -1)";
			} elsif($p{'type'} =~ /^dict/) {
				$properties_access_methods .= "value";
			} else {
				die "Unknown object type for access property: $p{'type'}\n";
			}
			
	        $properties_access_methods .= 
	        ", error);\n".
            "}\n\n";
        }
    }

    $properties_access_methods =~ s/\s+$//s;

    my $output = "$HEADER\n$SOURCE_TEMPLATE";
    if (defined $node->{'objectPath'}) {
        $output =~ s/\{IF_INIT\}\s+(.+?)\s+\{FI_INIT\}/$1/gs;
        $output =~ s/\s+\{IF_POST_INIT\}.+?\{FI_POST_INIT\}\s+/\n\n/gs;
        $output =~ s/\s+\{IF_NO_OBJECT_PATH\}.+?\{FI_NO_OBJECT_PATH\}//gs;
    } else {
        $output =~ s/\{IF_POST_INIT\}\s+(.+?)\s+\{FI_POST_INIT\}/$1/gs;
        $output =~ s/\s+\{IF_INIT\}.+?\{FI_INIT\}\s+/\n/gs;
        $output =~ s/\{IF_NO_OBJECT_PATH\}\s+(.+?)\s+\{FI_NO_OBJECT_PATH\}/$1/gs;
    }
    if (scalar keys %{$node->{$intf}{'signals'}} > 0) {
        $output =~ s/\{IF_SIGNALS\}\s+(.+?)\s+\{FI_SIGNALS\}/$1/gs;
    } else {
        $output =~ s/\s+\{IF_SIGNALS\}.+?\{FI_SIGNALS\}//gs;
    }
    if (scalar keys %{$node->{$intf}{'properties'}} > 0) {
        $output =~ s/\{IF_PROPERTIES\}\s+(.+?)\s+\{FI_PROPERTIES\}/$1/gs;
    } else {
        $output =~ s/\s+\{IF_PROPERTIES\}.+?\{FI_PROPERTIES\}//gs;
    }
    if (scalar keys %{$node->{$intf}{'properties'}} > 0) {
        $output =~ s/\{IF_PROPERTIES_EXT\}\s+(.+?)\s+\{FI_PROPERTIES_EXT\}/$1/gs;
    } else {
        $output =~ s/\s+\{IF_PROPERTIES_EXT\}.+?\{FI_PROPERTIES_EXT\}//gs;
    }
    if (scalar keys %{$node->{$intf}{'methods'}} > 0 or not defined $node->{'serviceName'} or not defined $node->{'objectPath'}) {
        $output =~ s/\{IF_METHODS\}\s+(.+?)\s+\{FI_METHODS\}/$1/gs;
    } else {
        $output =~ s/\s+\{IF_METHODS\}.+?\{FI_METHODS\}\n//gs;
    }
    $output =~ s/{\$conn}/$node->{'dbus_conn'}/g;
    $output =~ s/{CONSTRUCTOR_PROPERTIES}/$constructor_properties/;
    $output =~ s/{CONSTRUCTOR_CALL}/$constructor_call/;
    $output =~ s/{CONSTRUCTOR_GETTERS}/$constructor_getters/;
    $output =~ s/{CONSTRUCTOR_SETTERS}/$constructor_setters/;
    $output =~ s/{CONSTRUCTOR}/$constructor_def/;
    $output =~ s/{\$dbus_type}/$dbus_type/;
    $output =~ s/{PROPERTIES_ACCESS_METHODS}/$properties_access_methods/;
    $output =~ s/{METHODS}/$methods/;
    $output =~ s/{\$OBJECT}/$obj_uc/g;
    $output =~ s/{\$Object}/$obj/g;
    $output =~ s/{\$object}/$obj_lc/g;

    # Some formatting fixes
    $output =~ s/\s+?(\t*\})/\n$1/g;
    $output =~ s/(switch \(\w+\) \{\n)\s+?(\t+default:)/$1$2/s;
    $output =~ s/\s+$/\n\n/s;

    return $output;
}

my $data = parse_doc_api($ARGV[1], $ARGV[2]);

print generate_header($data) if $ARGV[0] eq '-header';
print generate_source($data) if $ARGV[0] eq '-source';