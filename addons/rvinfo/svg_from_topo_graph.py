#!/usr/bin/env python

#-
# Copyright (C) 2012-2013  Oy L M Ericsson Ab, NomadicLab
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
Simple tool for generating pictures of topologies.

Takes a topology file as input in GraphML format and outputs an SVG file.
You can also modify the code accoring to your needs.
"""

import igraph

def svg_from_graph(topo_file="topology.graphml",
                   gfx_file="graph.svg",
                   layout="lgl",
                   width=800,
                   height=800,
                   node_size=14,
                   node_color="lightblue",
                   edge_color="grey",
                   label_key="id"):
    g = igraph.read(topo_file)
#    g.to_undirected() # Use this if you want the graph to be undirected
    
    # Example of alternative layout: g.layout_reingold_tilford(root=0)
    
    width = int(width)
    height = int (height)
    node_size = int(node_size)
    node_colors = [node_color]*len(g.vs)
    edge_colors = [edge_color]*len(g.es)
    node_labels = [v[label_key][:] for v in g.vs]
    
    g.write_svg(gfx_file, layout=layout, width=width, height=height,
                vertex_size=node_size, font_size=node_size,
                labels=node_labels, colors=node_colors,
                edge_colors=edge_colors)
#    g.write_dot(gfx_file) # You can use this to produce a DOT file instead

def _main(argv):
    svg_from_graph(*argv[1:])

if __name__ == "__main__":
    import sys
    _main(sys.argv)
