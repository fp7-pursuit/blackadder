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

package pubsub;

import java.io.IOException;
import java.net.DatagramPacket;
import java.net.DatagramSocket;
import java.net.InetAddress;
import java.net.SocketException;
import java.util.Arrays;

import eu.pursuit.core.Event;
import uk.co.caprica.vlcj.player.MediaPlayer;
import uk.co.caprica.vlcj.player.MediaPlayerFactory;
import util.Util;
import view.VideoSubscriberGUI;

/**
 * A class to handle the subscription events
 * @author Ben Tagger
 * @version 29/11/2011
 */
public class SubscriberEventHandler extends Thread{

	private VideoSubscriberGUI gui;
	private DatagramSocket ds;
	private MediaPlayer mediaPlayer;
	
	public SubscriberEventHandler(VideoSubscriberGUI gui) throws SocketException{
		this.gui = gui;
		ds= new DatagramSocket();
		MediaPlayerFactory mediaPlayerFactory = new MediaPlayerFactory();
		mediaPlayer = mediaPlayerFactory.newMediaPlayer();	
	}

	public void run() {
		System.err.println("Processing the Events from the Thread");
		// Process the events...
		mediaPlayer.playMedia("udp://@:6666");
		while(true){
			Event event = gui.getClient().getNextEvent();
//			System.err.println("Got " + event.getType());
			switch(event.getType()){
			case PUBLISHED_DATA:
				// Subscriber receives event.
				// is it the catalog?
				if (Arrays.equals(event.getId(), gui.getVideoSubscriber().getCatalog())){
//					Util.printBytes(event.getDataCopy());	
					// populate the catalog list.
					gui.populateCatalogList(Util.getString(event.getDataCopy()));
					event.freeNativeBuffer();
					// finally unsubscribe to the catalog
					gui.getVideoSubscriber().unsubscribeCatalog();
				}else{
					// Is a video
					// get the packet and UDP it.
				    try {
						byte [] buffer = event.getDataCopy();
				    	DatagramPacket p = new DatagramPacket(buffer, buffer.length, InetAddress.getLocalHost(), 6666);
				    	ds.send(p);
					} catch (SocketException e) {
						// TODO Auto-generated catch block
						e.printStackTrace();
					} catch (IOException e) {
						// TODO Auto-generated catch block
						e.printStackTrace();
					}
					event.freeNativeBuffer();
				}
				break;
			}
		}
	}

	public static void main(String args[]) throws SocketException {
		(new SubscriberEventHandler(null)).start();
	}
}
