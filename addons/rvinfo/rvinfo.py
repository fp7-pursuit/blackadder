#!/usr/bin/env python

#-
# Copyright (C) 2013  Oy L M Ericsson Ab, NomadicLab
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License version 2 as
# published by the Free Software Foundation.
#
# Alternatively, this software may be distributed under the terms of the
# BSD license.
#
# See LICENSE and COPYING for more details.
#

"""
Simple tool for visualizing information structures.

Reads RV information from Blackadder and outputs a DOT file.
Modify the source code according to your needs.
"""

from socket import *
#import traceback
import json
import igraph
import binascii
import time

PURSUIT_ID_LEN = 8

ELEMENT = "localRV"
HANDLER = "dump"
read_u = "READ %s.%s\n" % (ELEMENT, HANDLER) # Userlevel READ
file_k = "/click/%s/%s" % (ELEMENT, HANDLER) # Kernel file

SRC_HOST = "localhost"
DST_HOST = "localhost"
DST_PORT = 55500

DUMP_KERN = False # Set this to True if the RV element runs in the kernel

# Table for mapping (hex) IDs to human-readable names
id_table = {
'FFFFFFFFFFFFFFFB': 'TM',
'00000001': 'NODE1',
}

def dump(kernel, *args):
    if kernel:
        return dump_k(*args)
    else:
        return dump_u(*args)

def dump_u(dst_host=DST_HOST, dst_port=DST_PORT):
    """Dump function for userlevel control socket."""
    
    rsp = None
    src_addr = (SRC_HOST, 0)
    dst_addr = (dst_host, dst_port)
    s = socket(AF_INET, SOCK_STREAM)
    try:
        s.bind(src_addr)
        print "Connect to:", dst_addr
        s.connect(dst_addr)
        rsp = s.recv(4096) # Click
        print rsp
        s.send(read_u)
        rsp = s.recv(8192) # DATA
        print repr(rsp)
        #print rsp
        s.send("QUIT\n")
        if not rsp or not rsp.startswith("200"):
            print "Expected 200, got %r", rsp
    finally:
        s.close()
    return rsp

def dump_k():
    """Dump function for kernel-level clickfs."""
    
    rsp = None
    try:
        f = open(file_k, "r")
        rsp = f.read()
    finally:
        f.close()
    return rsp

def parse(rsp):
    """Parse response data."""
    
    i_data = rsp.find("{")
    data = rsp[i_data:]
    
    d = json.loads(data)
    scopes = d["scopes"]
    iitems = d["iitems"]
    all_index = []
    i = 0
    
    # Create indexes for scopes and items.
    for item in (scopes, iitems):
        label = "scope" if item is scopes else "iitem"
        index = []
        for ids in item:
            for path in ids:
                full_id = str(path["id"])
                split_path = [full_id[j:j+2*PURSUIT_ID_LEN]
                              for j in xrange(0, len(full_id),
                                              2*PURSUIT_ID_LEN)]
                entry = [split_path, [i, label, ids]]
                index.append(entry)
            i += 1
        # Make sure paths are listed shortest first so that we can
        # detect scopeless scopes and iitems correctly.
        index.sort(lambda x, y: len(x[0]) - len(y[0]))
        all_index.append(index)
    
    return (i, all_index)

def make_path_dict(all_index):
    path_dict = { "": [-1, "scope", [""]] } # Init dict with dummy parent
    for index in all_index:
        for entry in index:
            key = "".join(entry[0])
            p_key = "".join(entry[0][:-1])
            if p_key not in path_dict or path_dict[p_key][1] != "scope":
                print "Warning: %r: Parent %r not found" % (key, p_key)
                continue
            if "key" in path_dict and path_dict["key"] != entry[1]:
                # Not expected to occur
                old, new = path_dict["key"], entry[1]
                print "Warning: %r: Replacing %r with %r" % (key, old, new)
            path_dict[key] = entry[1]
    return path_dict

def make_graph(all_index, path_dict, n):
    g = igraph.Graph(directed=True)
    g.add_vertices(n)
    g.vs["label"] = None
    for index in all_index:
        for entry in index:
            path, subentry = entry
            name = path[-1]
            p_key = "".join(path[:-1]) # parent
            i, label, items = subentry
            if p_key not in path_dict:
                print "Warning: %r: Parent %r not found" % (name, p_key)
                continue
            # Note that scopes with both the dummy parent and another
            # scope as a parent (if Blackadder would allow that) are
            # probably shown under the other scope, not at the top
            # level as root scopes. Also note that we don't check
            # whether items are on the top level and not in any scope.
            p_subentry = path_dict[p_key]
            p_i, p_label, p_items = p_subentry
            if g.vs[i]["label"] is None:
                if name in id_table:
                    name_repr = id_table[name]
                else:
                    name_repr = repr(binascii.unhexlify(name))[1:-1]
                    name_repr = name_repr.replace("\\x", "")
                pub, sub = set(), set()
                for item in items:
                    for node in item["pub"]:
                        nd = str(node)
                        pub.add(id_table[nd] if nd in id_table else nd)
                    for node in item["sub"]:
                        nd = str(node)
                        sub.add(id_table[nd] if nd in id_table else nd)
                if pub:
                    name_repr += "\n"
                    for p in pub:
                        name_repr += "P/%s" % repr(p)[1:-1]
                if sub:
                    name_repr += "\n"
                    for s in sub:
                        name_repr += "S/%s " % repr(s)[1:-1]
                #g.vs[i]["label"] = name
                g.vs[i]["label"] = name_repr # +"\n"+
                g.vs[i]["shape"] = ("oval" if label == "scope" else "box")
                g.vs[i]["tooltip"] = repr(items)
                g.vs[i]["fontname"] = "sans-serif"
            else:
                print "%r already exists in graph at %d as %r" \
                    % (name, i, g.vs[i]["label"])
            if p_i >= 0: # Top-level scopes (or iitems) don't have a parent
                try:
                    # Perhaps we should check this in another way?
                    eid = g.get_eid(p_i, i)
                except igraph.InternalError:
                    g.add_edges((p_i, i))
                    eid = g.get_eid(p_i, i)
                    g.es[eid]["arrowhead"] = "none"
    return g

def print_index(all_index):
    for index in all_index:
        for entry in index:
            print entry

def _main(argv):
    try:
        t1 = time.time()
        rsp = dump(DUMP_KERN, *argv[1:])
        n, all_index = parse(rsp)
        #print_index(all_index)
        path_dict = make_path_dict(all_index)
        g = make_graph(all_index, path_dict, n)
        g.write_dot("rvinfo.dot")
        # "dot -Tsvg rvinfo.dot -o rvinfo.svg"
        t2 = time.time()
        print "%.3fus" % ((t2-t1)*10**6)
    except:
        #traceback.print_exc()
        raise

if __name__ == "__main__":
    import sys
    _main(sys.argv)
