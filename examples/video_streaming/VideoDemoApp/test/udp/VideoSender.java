/************************************************************************
 * Copyright (C) 2010-2011  Ben Tagger                                  *
 * All rights reserved.                                                 *
 *                                                                      *
 * This program is free software; you can redistribute it and/or        *
 * modify it under the terms of the GNU General Public License version  *
 * 2 as published by the Free Software Foundation.                      *
 *                                                                      *
 * Alternatively, this software may be distributed under the terms of   *
 * the BSD license.                                                     *
 *                                                                      *
 * See LICENSE and COPYING for more details.                            *
 ************************************************************************/

package udp;

import java.net.*;

class VideoSender {
	public static int serverPort = 6666;
	public static int clientPort = 5555;
	public static int buffer_size = 1024;
	public static DatagramSocket ds;
	public static byte buffer[] = new byte[buffer_size];
	
	public static void TheServer() throws Exception {
		int pos=0;
		// send video to the udp port
	}
	public static void TheClient() throws Exception {
		while(true) {
			DatagramPacket p = new DatagramPacket(buffer,
					buffer.length);
			ds.receive(p);
			System.out.println(new String(p.getData(), 0,
					p.getLength()));
		}
	}
	public static void main(String args[]) throws Exception {
		if(args.length == 1) {
			ds = new DatagramSocket(serverPort);
			TheServer();
		} else {
			ds = new DatagramSocket(clientPort);
			TheClient();
		}
	}
}