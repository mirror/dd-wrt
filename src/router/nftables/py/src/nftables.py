#!/usr/bin/python
# Copyright(C) 2018 Phil Sutter <phil@nwl.cc>

# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, version 2 of the License.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.

import json
from ctypes import *
import sys
import os

NFTABLES_VERSION = "0.1"

class SchemaValidator:
    """Libnftables JSON validator using jsonschema"""

    def __init__(self):
        schema_path = os.path.join(os.path.dirname(__file__), "schema.json")
        with open(schema_path, 'r') as schema_file:
            self.schema = json.load(schema_file)
        import jsonschema
        self.jsonschema = jsonschema

    def validate(self, json):
        self.jsonschema.validate(instance=json, schema=self.schema)

class Nftables:
    """A class representing libnftables interface"""

    input_flags = {
        "no-dns": 0x1,
        "json": 0x2,
    }

    debug_flags = {
        "scanner":   0x1,
        "parser":    0x2,
        "eval":      0x4,
        "netlink":   0x8,
        "mnl":       0x10,
        "proto-ctx": 0x20,
        "segtree":   0x40,
    }

    output_flags = {
        "reversedns":     (1 << 0),
        "service":        (1 << 1),
        "stateless":      (1 << 2),
        "handle":         (1 << 3),
        "json":           (1 << 4),
        "echo":           (1 << 5),
        "guid":           (1 << 6),
        "numeric_proto":  (1 << 7),
        "numeric_prio":   (1 << 8),
        "numeric_symbol": (1 << 9),
        "numeric_time":   (1 << 10),
        "terse":          (1 << 11),
    }

    validator = None

    def __init__(self, sofile="libnftables.so.1"):
        """Instantiate a new Nftables class object.

        Accepts a shared object file to open, by default standard search path
        is searched for a file named 'libnftables.so'.

        After loading the library using ctypes module, a new nftables context
        is requested from the library and buffering of output and error streams
        is turned on.
        """
        self.__ctx = None

        lib = cdll.LoadLibrary(sofile)

        ### API function definitions

        self.nft_ctx_new = lib.nft_ctx_new
        self.nft_ctx_new.restype = c_void_p
        self.nft_ctx_new.argtypes = [c_int]

        self.nft_ctx_input_get_flags = lib.nft_ctx_input_get_flags
        self.nft_ctx_input_get_flags.restype = c_uint
        self.nft_ctx_input_get_flags.argtypes = [c_void_p]

        self.nft_ctx_input_set_flags = lib.nft_ctx_input_set_flags
        self.nft_ctx_input_set_flags.restype = c_uint
        self.nft_ctx_input_set_flags.argtypes = [c_void_p, c_uint]

        self.nft_ctx_output_get_flags = lib.nft_ctx_output_get_flags
        self.nft_ctx_output_get_flags.restype = c_uint
        self.nft_ctx_output_get_flags.argtypes = [c_void_p]

        self.nft_ctx_output_set_flags = lib.nft_ctx_output_set_flags
        self.nft_ctx_output_set_flags.argtypes = [c_void_p, c_uint]

        self.nft_ctx_output_get_debug = lib.nft_ctx_output_get_debug
        self.nft_ctx_output_get_debug.restype = c_int
        self.nft_ctx_output_get_debug.argtypes = [c_void_p]

        self.nft_ctx_output_set_debug = lib.nft_ctx_output_set_debug
        self.nft_ctx_output_set_debug.argtypes = [c_void_p, c_int]

        self.nft_ctx_buffer_output = lib.nft_ctx_buffer_output
        self.nft_ctx_buffer_output.restype = c_int
        self.nft_ctx_buffer_output.argtypes = [c_void_p]

        self.nft_ctx_get_output_buffer = lib.nft_ctx_get_output_buffer
        self.nft_ctx_get_output_buffer.restype = c_char_p
        self.nft_ctx_get_output_buffer.argtypes = [c_void_p]

        self.nft_ctx_buffer_error = lib.nft_ctx_buffer_error
        self.nft_ctx_buffer_error.restype = c_int
        self.nft_ctx_buffer_error.argtypes = [c_void_p]

        self.nft_ctx_get_error_buffer = lib.nft_ctx_get_error_buffer
        self.nft_ctx_get_error_buffer.restype = c_char_p
        self.nft_ctx_get_error_buffer.argtypes = [c_void_p]

        self.nft_run_cmd_from_buffer = lib.nft_run_cmd_from_buffer
        self.nft_run_cmd_from_buffer.restype = c_int
        self.nft_run_cmd_from_buffer.argtypes = [c_void_p, c_char_p]

        self.nft_run_cmd_from_filename = lib.nft_run_cmd_from_filename
        self.nft_run_cmd_from_filename.restype = c_int
        self.nft_run_cmd_from_filename.argtypes = [c_void_p, c_char_p]

        self.nft_ctx_add_include_path = lib.nft_ctx_add_include_path
        self.nft_ctx_add_include_path.restype = c_int
        self.nft_ctx_add_include_path.argtypes = [c_void_p, c_char_p]

        self.nft_ctx_clear_include_paths = lib.nft_ctx_clear_include_paths
        self.nft_ctx_clear_include_paths.argtypes = [c_void_p]

        self.nft_ctx_get_dry_run = lib.nft_ctx_get_dry_run
        self.nft_ctx_get_dry_run.restype = c_bool
        self.nft_ctx_get_dry_run.argtypes = [c_void_p]

        self.nft_ctx_set_dry_run = lib.nft_ctx_set_dry_run
        self.nft_ctx_set_dry_run.argtypes = [c_void_p, c_bool]

        self.nft_ctx_add_var = lib.nft_ctx_add_var
        self.nft_ctx_add_var.restype = c_int
        self.nft_ctx_add_var.argtypes = [c_void_p, c_char_p]

        self.nft_ctx_clear_vars = lib.nft_ctx_clear_vars
        self.nft_ctx_clear_vars.argtypes = [c_void_p]

        self.nft_ctx_free = lib.nft_ctx_free
        lib.nft_ctx_free.argtypes = [c_void_p]

        # initialize libnftables context
        self.__ctx = self.nft_ctx_new(0)
        self.nft_ctx_buffer_output(self.__ctx)
        self.nft_ctx_buffer_error(self.__ctx)

    def __del__(self):
        if self.__ctx is not None:
            self.nft_ctx_free(self.__ctx)
            self.__ctx = None

    def _flags_from_numeric(self, flags_dict, val):
        names = []
        for n, v in flags_dict.items():
            if val & v:
                names.append(n)
                val &= ~v
        if val:
            names.append(val)
        return names

    def _flags_to_numeric(self, flags_dict, values):
        if isinstance(values, (str, int)):
            values = (values,)

        val = 0
        for v in values:
            if isinstance(v, str):
                v = flags_dict.get(v)
                if v is None:
                    raise ValueError("Invalid argument")
            elif isinstance(v, int):
                if v < 0 or v > 0xFFFFFFFF:
                    raise ValueError("Invalid argument")
            else:
                raise TypeError("Not a valid flag")
            val |= v

        return val

    def get_input_flags(self):
        """Get currently active input flags.

        Returns a set of flag names. See set_input_flags() for details.
        """
        val = self.nft_ctx_input_get_flags(self.__ctx)
        return self._flags_from_numeric(self.input_flags, val)

    def set_input_flags(self, values):
        """Set input flags.

        Resets all input flags to values. Accepts either a single flag or a list
        of flags. Each flag might be given either as string or integer value as
        shown in the following table:

        Name      | Value (hex)
        -----------------------
        "no-dns"  | 0x1
        "json"    | 0x2

        "no-dns" disables blocking address lookup.
        "json" enables JSON mode for input.

        Returns a set of previously active input flags, as returned by
        get_input_flags() method.
        """
        val = self._flags_to_numeric(self.input_flags, values)
        old = self.nft_ctx_input_set_flags(self.__ctx, val)
        return self._flags_from_numeric(self.input_flags, old)

    def __get_output_flag(self, name):
        flag = self.output_flags[name]
        return (self.nft_ctx_output_get_flags(self.__ctx) & flag) != 0

    def __set_output_flag(self, name, val):
        flag = self.output_flags[name]
        flags = self.nft_ctx_output_get_flags(self.__ctx)
        if val:
            new_flags = flags | flag
        else:
            new_flags = flags & ~flag
        self.nft_ctx_output_set_flags(self.__ctx, new_flags)
        return (flags & flag) != 0

    def get_reversedns_output(self):
        """Get the current state of reverse DNS output.

        Returns a boolean indicating whether reverse DNS lookups are performed
        for IP addresses in output.
        """
        return self.__get_output_flag("reversedns")

    def set_reversedns_output(self, val):
        """Enable or disable reverse DNS output.

        Accepts a boolean turning reverse DNS lookups in output on or off.

        Returns the previous value.
        """
        return self.__set_output_flag("reversedns", val)

    def get_service_output(self):
        """Get the current state of service name output.

        Returns a boolean indicating whether service names are used for port
        numbers in output or not.
        """
        return self.__get_output_flag("service")

    def set_service_output(self, val):
        """Enable or disable service name output.

        Accepts a boolean turning service names for port numbers in output on
        or off.

        Returns the previous value.
        """
        return self.__set_output_flag("service", val)

    def get_stateless_output(self):
        """Get the current state of stateless output.

        Returns a boolean indicating whether stateless output is active or not.
        """
        return self.__get_output_flag("stateless")

    def set_stateless_output(self, val):
        """Enable or disable stateless output.

        Accepts a boolean turning stateless output either on or off.

        Returns the previous value.
        """
        return self.__set_output_flag("stateless", val)

    def get_handle_output(self):
        """Get the current state of handle output.

        Returns a boolean indicating whether handle output is active or not.
        """
        return self.__get_output_flag("handle")

    def set_handle_output(self, val):
        """Enable or disable handle output.

        Accepts a boolean turning handle output on or off.

        Returns the previous value.
        """
        return self.__set_output_flag("handle", val)

    def get_json_output(self):
        """Get the current state of JSON output.

        Returns a boolean indicating whether JSON output is active or not.
        """
        return self.__get_output_flag("json")

    def set_json_output(self, val):
        """Enable or disable JSON output.

        Accepts a boolean turning JSON output either on or off.

        Returns the previous value.
        """
        return self.__set_output_flag("json", val)

    def get_echo_output(self):
        """Get the current state of echo output.

        Returns a boolean indicating whether echo output is active or not.
        """
        return self.__get_output_flag("echo")

    def set_echo_output(self, val):
        """Enable or disable echo output.

        Accepts a boolean turning echo output on or off.

        Returns the previous value.
        """
        return self.__set_output_flag("echo", val)

    def get_guid_output(self):
        """Get the current state of GID/UID output.

        Returns a boolean indicating whether names for group/user IDs are used
        in output or not.
        """
        return self.__get_output_flag("guid")

    def set_guid_output(self, val):
        """Enable or disable GID/UID output.

        Accepts a boolean turning names for group/user IDs on or off.

        Returns the previous value.
        """
        return self.__set_output_flag("guid", val)

    def get_numeric_proto_output(self):
        """Get current status of numeric protocol output flag.

        Returns a boolean value indicating the status.
        """
        return self.__get_output_flag("numeric_proto")

    def set_numeric_proto_output(self, val):
        """Set numeric protocol output flag.

        Accepts a boolean turning numeric protocol output either on or off.

        Returns the previous value.
        """
        return self.__set_output_flag("numeric_proto", val)

    def get_numeric_prio_output(self):
        """Get current status of numeric chain priority output flag.

        Returns a boolean value indicating the status.
        """
        return self.__get_output_flag("numeric_prio")

    def set_numeric_prio_output(self, val):
        """Set numeric chain priority output flag.

        Accepts a boolean turning numeric chain priority output either on or
        off.

        Returns the previous value.
        """
        return self.__set_output_flag("numeric_prio", val)

    def get_numeric_symbol_output(self):
        """Get current status of numeric symbols output flag.

        Returns a boolean value indicating the status.
        """
        return self.__get_output_flag("numeric_symbol")

    def set_numeric_symbol_output(self, val):
        """Set numeric symbols output flag.

        Accepts a boolean turning numeric representation of symbolic constants
        in output either on or off.

        Returns the previous value.
        """
        return self.__set_output_flag("numeric_symbol", val)

    def get_numeric_time_output(self):
        """Get current status of numeric times output flag.

        Returns a boolean value indicating the status.
        """
        return self.__get_output_flag("numeric_time")

    def set_numeric_time_output(self, val):
        """Set numeric times output flag.

        Accepts a boolean turning numeric representation of time values
        in output either on or off.

        Returns the previous value.
        """
        return self.__set_output_flag("numeric_time", val)

    def get_terse_output(self):
        """Get the current state of terse output.

        Returns a boolean indicating whether terse output is active or not.
        """
        return self.__get_output_flag("terse")

    def set_terse_output(self, val):
        """Enable or disable terse output.

        Accepts a boolean turning terse output either on or off.

        Returns the previous value.
        """
        return self.__set_output_flag("terse", val)

    def get_debug(self):
        """Get currently active debug flags.

        Returns a set of flag names. See set_debug() for details.
        """
        val = self.nft_ctx_output_get_debug(self.__ctx)
        return self._flags_from_numeric(self.debug_flags, val)

    def set_debug(self, values):
        """Set debug output flags.

        Accepts either a single flag or a set of flags. Each flag might be
        given either as string or integer value as shown in the following
        table:

        Name      | Value (hex)
        -----------------------
        scanner   | 0x1
        parser    | 0x2
        eval      | 0x4
        netlink   | 0x8
        mnl       | 0x10
        proto-ctx | 0x20
        segtree   | 0x40

        Returns a set of previously active debug flags, as returned by
        get_debug() method.
        """
        val = self._flags_to_numeric(self.debug_flags, values)
        old = self.get_debug()
        self.nft_ctx_output_set_debug(self.__ctx, val)
        return old

    def cmd(self, cmdline):
        """Run a simple nftables command via libnftables.

        Accepts a string containing an nftables command just like what one
        would enter into an interactive nftables (nft -i) session.

        Returns a tuple (rc, output, error):
        rc     -- return code as returned by nft_run_cmd_from_buffer() fuction
        output -- a string containing output written to stdout
        error  -- a string containing output written to stderr
        """
        cmdline_is_unicode = False
        if not isinstance(cmdline, bytes):
            cmdline_is_unicode = True
            cmdline = cmdline.encode("utf-8")
        rc = self.nft_run_cmd_from_buffer(self.__ctx, cmdline)
        output = self.nft_ctx_get_output_buffer(self.__ctx)
        error = self.nft_ctx_get_error_buffer(self.__ctx)
        if cmdline_is_unicode:
            output = output.decode("utf-8")
            error = error.decode("utf-8")

        return (rc, output, error)

    def json_cmd(self, json_root):
        """Run an nftables command in JSON syntax via libnftables.

        Accepts a hash object as input.

        Returns a tuple (rc, output, error):
        rc     -- return code as returned by nft_run_cmd_from_buffer() function
        output -- a hash object containing library standard output
        error  -- a string containing output written to stderr
        """
        json_out_old = self.set_json_output(True)
        rc, output, error = self.cmd(json.dumps(json_root))
        if not json_out_old:
            self.set_json_output(json_out_old)
        if len(output):
            output = json.loads(output)
        return (rc, output, error)

    def json_validate(self, json_root):
        """Validate JSON object against libnftables schema.

        Accepts a hash object as input.

        Returns True if JSON is valid, raises an exception otherwise.
        """
        if not self.validator:
            self.validator = SchemaValidator()

        self.validator.validate(json_root)
        return True

    def cmd_from_file(self, filename):
        """Run a nftables command set from a file

        filename can be a str or a Path

        Returns a tuple (rc, output, error):
        rc     -- return code as returned by nft_run_cmd_from_filename() function
        output -- a string containing output written to stdout
        error  -- a string containing output written to stderr
        """
        filename_is_unicode = False
        if not isinstance(filename, bytes):
            filename_is_unicode = True
            filename = str(filename)
            filename= filename.encode("utf-8")
        rc = self.nft_run_cmd_from_filename(self.__ctx, filename)
        output = self.nft_ctx_get_output_buffer(self.__ctx)
        error = self.nft_ctx_get_error_buffer(self.__ctx)
        if filename_is_unicode:
            output = output.decode("utf-8")
            error = error.decode("utf-8")
        return (rc, output, error)

    def add_include_path(self, filename):
        """Add a path to the include file list
        The default list includes the built-in default one

        Returns True on success, False if memory allocation fails
        """
        if not isinstance(filename, bytes):
            filename = str(filename)
            filename= filename.encode("utf-8")
        rc = self.nft_ctx_add_include_path(self.__ctx, filename)
        return rc == 0

    def clear_include_paths(self):
        """Clear include path list

        Will also remove the built-in default one
        """
        self.nft_ctx_clear_include_paths(self.__ctx)

    def get_dry_run(self):
        """Get dry run state

        Returns True if set, False otherwise
        """
        return self.nft_ctx_get_dry_run(self.__ctx)

    def set_dry_run(self, onoff):
        """ Set dry run state

        Returns the previous dry run state
        """
        old = self.get_dry_run()
        self.nft_ctx_set_dry_run(self.__ctx, onoff)

        return old

    def add_var(self, var):
        """Add a variable to the variable list

        Returns True if added, False otherwise
        """
        if not isinstance(var, bytes):
            var = var.encode("utf-8")
        rc = self.nft_ctx_add_var(self.__ctx, var)
        return rc == 0

    def clear_vars(self):
        """Clear variable list
        """
        self.nft_ctx_clear_vars(self.__ctx)
