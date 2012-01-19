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

import java.util.Arrays;
import java.util.Vector;

import org.apache.commons.codec.binary.Hex;

import eu.pursuit.client.BlackAdderClient;
import eu.pursuit.core.ByteIdentifier;
import eu.pursuit.core.ItemName;
import eu.pursuit.core.ScopeID;
import eu.pursuit.core.Strategy;

/**
 * A class that handles the publication of videos through the blackadder network.
 * Handles publications.
 * @author Ben Tagger
 * @version - started 13/10/2011
 */
public class VideoSubscriber{
	
	private BlackAdderClient client;
	private ScopeID superScope;
	private ItemName catName;
	private Strategy strategy;
	
	private Vector<ItemName> subscribed;
	
	
	public VideoSubscriber(BlackAdderClient client, ScopeID superScope, Strategy strategy){
		this.client = client;
		this.superScope = superScope;
		this.strategy = strategy;
		subscribed = new Vector<ItemName>(0,1);
	}
	
	public boolean subscribeCatalog(){	
		try {
			// Subscribe to the catalog
			String catString = "0000000000000000";
			ByteIdentifier catID = new ByteIdentifier(Hex.decodeHex(catString.toCharArray()));
			catName = new ItemName(superScope, catID);
			client.subscribeItem(catName, strategy);
			return true;
		} catch (Exception e) {
			e.printStackTrace();
			return false;
		}
	}
	
	public boolean unsubscribeCatalog(){
		try {
			// Subscribe to the catalog
			String catString = "0000000000000000";
			ByteIdentifier catID = new ByteIdentifier(Hex.decodeHex(catString.toCharArray()));
			catName = new ItemName(superScope, catID);
			client.unsubscribeItem(catName, strategy);
			return true;
		} catch (Exception e) {
			e.printStackTrace();
			return false;
		}
	}
	
	public boolean subscribeVideo(String rid){
		try {
			// Subscribe to the video
			ByteIdentifier vidID = new ByteIdentifier(Hex.decodeHex(rid.toCharArray()));
			ItemName vidName = new ItemName(superScope, vidID);
			client.subscribeItem(vidName, strategy);
			subscribed.add(vidName);
			return true;
		} catch (Exception e) {
			e.printStackTrace();
			return false;
		}
	}
	
	public boolean unsubscribeVideo(String rid){
		try {
			// Subscribe to the video
			ByteIdentifier vidID = new ByteIdentifier(Hex.decodeHex(rid.toCharArray()));
			ItemName vidName = new ItemName(superScope, vidID);
			client.unsubscribeItem(vidName, strategy);
			// remove from list
			ItemName marked = null;
			for (ItemName in: subscribed){
				if (Arrays.equals(in.getRendezvousId().getId(), vidName.getRendezvousId().getId())){
					marked = in;
				}
			}
			if (marked!=null){
				subscribed.remove(marked);
			}
			return true;
		} catch (Exception e) {
			e.printStackTrace();
			return false;
		}
	}
	
	public byte[] getCatalog(){
		return catName.toByteArray();
	}
	
	/**
	 * Removes all subscriptions
	 * @return a boolean indicating success
	 */
	public boolean cleanup(){
		try {
			// unsubscribe from catalog
			this.unsubscribeCatalog();
			// unsubscribe all active subscriptions.
			for (ItemName in: subscribed){
				client.unsubscribeItem(in, strategy);
			}
			return true;
		} catch (Exception e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
			return false;
		}
	}
	
}
