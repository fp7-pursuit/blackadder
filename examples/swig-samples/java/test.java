/*-
 * Copyright (C) 2011  Oy L M Ericsson Ab, NomadicLab
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * Alternatively, this software may be distributed under the terms of the
 * BSD license.
 *
 * See LICENSE and COPYING for more details.
 */

/* Simple Blackadder connection example. */

public class test {
    static {
        System.loadLibrary("blackadder_java");
    }
    static final blackadder_java.BA BA = null;

    public static void main(String args[]) {
        blackadder_java.Blackadder ba =
	    blackadder_java.Blackadder.Instance(true);
        ba.publish_scope("00001111".getBytes(), new byte[0],
                         BA.NODE_LOCAL, null);
        ba.publish_info("XXXXYYYY".getBytes(), "00001111".getBytes(),
                        BA.NODE_LOCAL, null);
        ba.disconnect();
    }
}
