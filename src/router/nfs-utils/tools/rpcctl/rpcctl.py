#!/usr/bin/python3
import argparse
import collections
import errno
import os
import pathlib
import socket
import sys

with open("/proc/mounts", 'r') as f:
    mount = [ line.split()[1] for line in f if "sysfs" in line ]
    if len(mount) == 0:
        print("ERROR: sysfs is not mounted")
        sys.exit(1)

sunrpc = pathlib.Path(mount[0]) / "kernel" / "sunrpc"
if not sunrpc.is_dir():
    print("ERROR: sysfs does not have sunrpc directory")
    sys.exit(1)

def read_addr_file(path):
    try:
        with open(path, 'r') as f:
            return f.readline().strip()
    except:
        return "(enoent)"

def write_addr_file(path, newaddr):
    with open(path, 'w') as f:
        f.write(newaddr)
    return read_addr_file(path)

def read_info_file(path):
    res = collections.defaultdict(int)
    try:
        with open(path) as info:
            lines = [ l.split("=", 1) for l in info if "=" in l ]
            res.update({ key:int(val.strip()) for (key, val) in lines })
    finally:
        return res


class Xprt:
    def __init__(self, path):
        self.path = path
        self.name = path.stem.rsplit("-", 1)[0]
        self.type = path.stem.split("-")[2]
        self.info = read_info_file(path / "xprt_info")
        self.dstaddr = read_addr_file(path / "dstaddr")
        self.srcaddr = read_addr_file(path / "srcaddr")
        self.read_state()

    def __lt__(self, rhs):
        return self.name < rhs.name

    def _xprt(self):
        main = ", main" if self.info.get("main_xprt") else ""
        return f"{self.name}: {self.type}, {self.dstaddr}, " \
               f"port {self.info['dst_port']}, state <{self.state}>{main}"

    def _src_reqs(self):
        return f"	Source: {self.srcaddr}, port {self.info['src_port']}, " \
               f"Requests: {self.info['num_reqs']}"

    def _cong_slots(self):
        return f"	Congestion: cur {self.info['cur_cong']}, win {self.info['cong_win']}, " \
               f"Slots: min {self.info['min_num_slots']}, max {self.info['max_num_slots']}"

    def _queues(self):
        return f"	Queues: binding {self.info['binding_q_len']}, " \
               f"sending {self.info['sending_q_len']}, pending {self.info['pending_q_len']}, " \
               f"backlog {self.info['backlog_q_len']}, tasks {self.info['tasks_queuelen']}"

    def __str__(self):
        if not self.path.exists():
            return f"{self.name}: has been removed"
        return "\n".join([self._xprt(), self._src_reqs(),
                          self._cong_slots(), self._queues() ])

    def read_state(self):
        if self.path.exists():
            with open(self.path / "xprt_state") as f:
                self.state = ','.join(f.readline().split()[1:])

    def small_str(self):
        main = " [main]" if self.info.get("main_xprt") else ""
        return f"{self.name}: {self.type}, {self.dstaddr}{main}"

    def set_dstaddr(self, newaddr):
        self.dstaddr = write_addr_file(self.path / "dstaddr", newaddr)

    def set_state(self, state):
        if self.info.get("main_xprt"):
            raise Exception(f"Main xprts cannot be set {state}")
        with open(self.path / "xprt_state", 'w') as f:
            f.write(state)
        self.read_state()

    def remove(self):
        if self.info.get("main_xprt"):
            raise Exception("Main xprts cannot be removed")
        self.set_state("offline")
        self.set_state("remove")

    def add_command(subparser):
        parser = subparser.add_parser("xprt", help="Commands for individual xprts")
        parser.set_defaults(func=Xprt.show, xprt=None)
        subparser = parser.add_subparsers()

        remove = subparser.add_parser("remove", help="Remove an xprt")
        remove.add_argument("xprt", metavar="XPRT", nargs=1,
                            help="Name of the xprt to remove")
        remove.set_defaults(func=Xprt.set_property, property="remove")

        show = subparser.add_parser("show", help="Show xprts")
        show.add_argument("xprt", metavar="XPRT", nargs='?',
                          help="Name of a specific xprt to show")
        show.set_defaults(func=Xprt.show)

        set = subparser.add_parser("set", help="Change an xprt property")
        set.add_argument("xprt", metavar="XPRT", nargs=1,
                         help="Name of a specific xprt to modify")
        subparser = set.add_subparsers(required=True)
        online = subparser.add_parser("online", help="Set an xprt online")
        online.set_defaults(func=Xprt.set_property, property="online")
        offline = subparser.add_parser("offline", help="Set an xprt offline")
        offline.set_defaults(func=Xprt.set_property, property="offline")
        dstaddr = subparser.add_parser("dstaddr", help="Change an xprt's dstaddr")
        dstaddr.add_argument("newaddr", metavar="NEWADDR", nargs=1,
                             help="The new address for the xprt")
        dstaddr.set_defaults(func=Xprt.set_property, property="dstaddr")

    def get_by_name(name):
        glob = f"**/{name}-*" if name else "**/xprt-*"
        res = [ Xprt(x) for x in (sunrpc / "xprt-switches").glob(glob) ]
        if name and len(res) == 0:
            raise FileNotFoundError(errno.ENOENT, os.strerror(errno.ENOENT),
                                    f"{sunrpc / 'xprt-switches' / glob}")
        return sorted(res)

    def show(args):
        for xprt in Xprt.get_by_name(args.xprt):
            print(xprt)

    def set_property(args):
        for xprt in Xprt.get_by_name(args.xprt[0]):
            if args.property == "dstaddr":
                xprt.set_dstaddr(socket.gethostbyname(args.newaddr[0]))
            elif args.property == "remove":
                xprt.remove()
            else:
                xprt.set_state(args.property)
        print(xprt)


class XprtSwitch:
    def __init__(self, path, sep=":"):
        self.path = path
        self.name = path.stem
        self.info = read_info_file(path / "xprt_switch_info")
        self.xprts = sorted([ Xprt(p) for p in self.path.iterdir() if p.is_dir() ])
        self.sep = sep

    def __lt__(self, rhs):
        return self.name < rhs.name

    def __str__(self):
        switch =  f"{self.name}{self.sep} " \
                  f"xprts {self.info['num_xprts']}, " \
                  f"active {self.info['num_active']}, " \
                  f"queue {self.info['queue_len']}"
        xprts = [ f"	{x.small_str()}" for x in self.xprts ]
        return "\n".join([ switch ] + xprts)

    def add_command(subparser):
        parser = subparser.add_parser("switch", help="Commands for xprt switches")
        parser.set_defaults(func=XprtSwitch.show, switch=None)
        subparser = parser.add_subparsers()

        show = subparser.add_parser("show", help="Show xprt switches")
        show.add_argument("switch", metavar="SWITCH", nargs='?',
                          help="Name of a specific switch to show")
        show.set_defaults(func=XprtSwitch.show)

        set = subparser.add_parser("set", help="Change an xprt switch property")
        set.add_argument("switch", metavar="SWITCH", nargs=1,
                         help="Name of a specific xprt switch to modify")
        subparser = set.add_subparsers(required=True)
        dstaddr = subparser.add_parser("dstaddr", help="Change an xprt switch's dstaddr")
        dstaddr.add_argument("newaddr", metavar="NEWADDR", nargs=1,
                             help="The new address for the xprt switch")
        dstaddr.set_defaults(func=XprtSwitch.set_property, property="dstaddr")

    def get_by_name(name):
        xprt_switches = sunrpc / "xprt-switches"
        if name:
            return [ XprtSwitch(xprt_switches / name) ]
        return [ XprtSwitch(f) for f in sorted(xprt_switches.iterdir()) ]

    def show(args):
        for switch in XprtSwitch.get_by_name(args.switch):
            print(switch)

    def set_property(args):
        for switch in XprtSwitch.get_by_name(args.switch[0]):
            resolved = socket.gethostbyname(args.newaddr[0])
            for xprt in switch.xprts:
                xprt.set_dstaddr(resolved)
        print(switch)


class RpcClient:
    def __init__(self, path):
        self.path = path
        self.name = path.stem
        self.switch = XprtSwitch(path / (path / "switch").readlink(), sep=",")

    def __lt__(self, rhs):
        return self.name < rhs.name

    def __str__(self):
        return f"{self.name}: {self.switch}"

    def add_command(subparser):
        parser = subparser.add_parser("client", help="Commands for rpc clients")
        parser.set_defaults(func=RpcClient.show, client=None)
        subparser = parser.add_subparsers()

        show = subparser.add_parser("show", help="Show rpc clients")
        show.add_argument("client", metavar="CLIENT", nargs='?',
                          help="Name of a specific rpc client to show")
        parser.set_defaults(func=RpcClient.show)

    def get_by_name(name):
        rpc_clients = sunrpc / "rpc-clients"
        if name:
            return [ RpcClient(rpc_clients / name) ]
        return [ RpcClient(f) for f in sorted(rpc_clients.iterdir()) ]

    def show(args):
        for client in RpcClient.get_by_name(args.client):
            print(client)


parser = argparse.ArgumentParser()

def show_small_help(args):
    parser.print_usage()
    print("sunrpc dir:", sunrpc)
parser.set_defaults(func=show_small_help)

subparser = parser.add_subparsers(title="commands")
RpcClient.add_command(subparser)
XprtSwitch.add_command(subparser)
Xprt.add_command(subparser)

args = parser.parse_args()
try:
    args.func(args)
except Exception as e:
    print(str(e))
    sys.exit(1)
