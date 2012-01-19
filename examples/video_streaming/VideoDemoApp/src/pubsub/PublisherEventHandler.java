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

import java.nio.ByteBuffer;
import java.nio.CharBuffer;
import java.util.Arrays;
import java.util.HashMap;
import java.util.Map;

import eu.pursuit.core.Event;
import eu.pursuit.core.Publication;
import eu.pursuit.core.Strategy;
import util.Util;
import view.PublisherView;

/**
 * A class to handle the publication events
 * @author Ben Tagger
 * @version 29/11/2011
 */
public class PublisherEventHandler extends Thread{

	private PublisherView publisher;
	private Map<String, VideoEventHandler> runningThreads;
	private Strategy strategy;

	public PublisherEventHandler(PublisherView publisher, Strategy strategy){
		this.publisher = publisher;
		runningThreads = new HashMap<String, VideoEventHandler>();
		this.strategy = strategy;
	}

	public void run() {
		System.err.println("Processing the Events from the Thread");
		// Process the events...
		while(true){
			System.err.println("Publisher: Getting event...");
			Event event = publisher.getClient().getNextEvent();
			System.err.println("Publisher: Got event...");
			System.err.println(event.getType());
			byte[] id = event.getId();
			byte [] cat = publisher.getVideoPublisher().getCatalog();
			switch(event.getType()){
			case START_PUBLISH:
				// Is the event the catalog? Problem here!...
				if (Arrays.equals(id, cat)){
					// Get the catalog data
					String catData = publisher.getVideoPublisher().getCatalogNames();
					Publication pub = new Publication(publisher.getVideoPublisher().getCatName(), catData.getBytes());
					// publish catalog data
					publisher.getClient().publishData(pub, strategy);
				}else{
					// start the video event handler
					// Check to see if the video is already started.
					VideoEventHandler veh = new VideoEventHandler(publisher, id, strategy);
					veh.start();
					String idStr = Util.byteArrayToHex(id);// this.getStringFromBytes(id);
					runningThreads.put(idStr, veh);
				}
				break;
			case STOP_PUBLISH:
				// Stop publishing material
				if (Arrays.equals(id, cat)){
					// ignore sending of catalog
				}else{
					// if the video is still publishing, then stop it.
					String idStr = Util.byteArrayToHex(id);
					if (runningThreads.containsKey(idStr)){
						runningThreads.get(idStr).removeSubscriber();
						if (runningThreads.get(idStr).getSubscribers() ==0)
							runningThreads.remove(idStr);
					}
				else{
						// The video is not running
					}
				}
				break;
			
			}
		}
	}
	
}
