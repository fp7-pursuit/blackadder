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
import java.net.SocketException;

import model.VideoStream;

import eu.pursuit.core.ItemName;
import eu.pursuit.core.Publication;
import eu.pursuit.core.ScopeID;
import eu.pursuit.core.Strategy;
import util.Util;
import view.PublisherView;
import view.VideoPublisherGUI;

/**
 * A class to set up the video publication
 * @author Ben Tagger
 * @version 29/11/2011
 */
public class VideoEventHandler extends Thread{

	private PublisherView publisher;
	private byte[] rID;
	private DatagramSocket ds;
	private VideoRunner videoRunner;
	private Strategy strategy;
	private int subscribers;

	public VideoEventHandler(PublisherView gui, byte[] rID, Strategy strategy){
		this.publisher = gui;
		this.rID = rID;
		this.strategy = strategy;
		try {
			ds = new DatagramSocket();
		} catch (SocketException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		}
	}

	public void run() {
		// start sending the video.
		// get the video that's been requested
		String r = Util.byteArrayToHex(rID);
		int len = r.length();
		String request = r.substring(len-16, len);
		VideoStream vs = publisher.getVideoPublisher().getVideoStreamByRID(request);
		// Get a handle on the video
		System.err.println("Requested " + vs.getAbsolutePath());
		// Implement the video strategy.
		videoRunner = new VideoRunner(vs.getAbsolutePath(), ds.getLocalPort());
		videoRunner.start();
		ItemName newPubName = ItemName.parseSerializedName(rID, ScopeID.SEGMENT_SIZE);
		try {
			DatagramPacket p = null;
			do{
				int buffer_size = 1316;
				byte[] buffer = new byte[buffer_size];
				p = new DatagramPacket(buffer, buffer.length);
				ds.receive(p);
				// publication					    	
				Publication pub = new Publication(newPubName, p.getData());
//				System.err.println(".");
				publisher.getClient().publishData(pub, strategy);
//				System.err.println("|");
				
			}while(p.getData() != null);
		} catch (SocketException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		} catch (IOException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		}
	}

	private void stopVideoRunner(){
		videoRunner.stop();
		videoRunner = null;
		this.interrupt();
	}
	
	public VideoRunner getVideoRunner() {
		return videoRunner;
	}

	public void setVideoRunner(VideoRunner videoRunner) {
		this.videoRunner = videoRunner;
	}
	
	public void addSubscriber(){
		subscribers++;
	}
	
	public void removeSubscriber(){
		if (subscribers > 0)
			subscribers --;
		if(subscribers == 0)
			stopVideoRunner();
	}

	public int getSubscribers() {
		return subscribers;
	}
}
