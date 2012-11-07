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

/* Simple text publisher example. */

import java.io.*;
import sun.misc.*;

class Publisher {
    static {
        System.loadLibrary("blackadder_java");
    }
    static final blackadder_java.BA BA = null;

    int strategy = BA.DOMAIN_LOCAL;
    //int strategy = BA.NODE_LOCAL;

    blackadder_java.Blackadder ba = null;
    BufferedReader in = null;
    boolean pub = true;
    SignalHandler handler = null;

    Publisher() {
        this.ba = blackadder_java.Blackadder.Instance(true);
        this.in = new BufferedReader(new InputStreamReader(System.in));

        this.handler = new SignalHandler() {
                public void handle(Signal sig) {
                    try {
                        pub = false;
                        //ba.disconnect();
                        in.close();
                    } catch (IOException ioe) {
                        ioe.printStackTrace();
                    }
                }
            };
        Signal.handle(new Signal("INT"), this.handler);
    }

    void publish(byte[] sid, byte[] rid) throws IOException {
        try {
            byte[] sidrid = new byte[sid.length + rid.length];
            System.arraycopy(sid, 0, sidrid, 0, sid.length);
            System.arraycopy(rid, 0, sidrid, sid.length, rid.length);

            ba.publish_scope(sid, new byte[0], strategy, null);
            ba.publish_info(rid, sid, strategy, null);

            while (pub) {
                blackadder_java.Event ev = new blackadder_java.Event();
                ba.getEvent(ev);
                int type = ev.getType();
                System.out.println(type);
                if (type == 0)
                    return;
                if (type == BA.START_PUBLISH)
                    break;
            }
            if (!pub)
                return;

            while (pub) {
                System.out.print("Publish? ");
                String data = this.in.readLine();
                if (!pub)
                    break;
                ba.publish_data(sidrid, strategy, null, data.getBytes());
            }
        }
        finally {
            ba.disconnect();
        }
    }
}

public class test_pub {
    public static void main(String args[]) throws Exception {
        byte[] sid = new byte[8]; sid[0] = 0x0a; sid[7] = 0x0b;
        byte[] rid = { 0x0c, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0d };

        Publisher pub = new Publisher();
        pub.publish(sid, rid);
    }
}
