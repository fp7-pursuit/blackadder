/*-
 * Copyright (C) 2011-2012  Oy L M Ericsson Ab, NomadicLab
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

/* Simple non-blocking text publisher example. */

class NBPublisher {
    static {
        System.loadLibrary("blackadder_java");
    }
    static final blackadder_java.BA BA = null;

    int strategy = BA.DOMAIN_LOCAL;
    //int strategy = BA.NODE_LOCAL;

    blackadder_java.NB_Blackadder nb_ba;
    byte[] sidrid;

    static NBEventHandler nb_evh = null;
    static void _event_handler(long evptr) {
        nb_evh.event_handler(new blackadder_java.EventProxy(evptr));
    }

    class NBEventHandler {
        void event_handler(blackadder_java.EventProxy ev) {
            int type = ev.getType();
            System.out.println(type);
            if (type == 0)
                return;
            if (type != BA.START_PUBLISH) {
                return;
            }

            nb_ba.publish_data(sidrid, strategy, null, "Hello1".getBytes());
            nb_ba.publish_data(sidrid, strategy, null, "Hello2".getBytes());
            nb_ba.publish_data(sidrid, strategy, null, "Hello3".getBytes());
            try {
                Thread.sleep(1);
            }
            catch (java.lang.InterruptedException ie) {
            }
        }
    }

    void publish(byte[] sid, byte[] rid) {
        nb_ba = blackadder_java.NB_Blackadder.Instance(true);

        try {
            sidrid = new byte[sid.length + rid.length];
            System.arraycopy(sid, 0, sidrid, 0, sid.length);
            System.arraycopy(rid, 0, sidrid, sid.length, rid.length);

            nb_evh = new NBEventHandler();
            nb_ba.setJavaCallback(this, "_event_handler");

            nb_ba.publish_scope(sid, new byte[0], strategy, null);
            nb_ba.publish_info(rid, sid, strategy, null);

            nb_ba.join();
        }
        finally {
            nb_ba.disconnect();
        }
    }
}

public class test_nb_pub {
    public static void main(String args[]) throws Exception {
        byte[] sid = new byte[8]; sid[0] = 0x0a; sid[7] = 0x0b;
        byte[] rid = { 0x0c, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0d };

        NBPublisher nb_pub = new NBPublisher();
        nb_pub.publish(sid, rid);
    }
}
