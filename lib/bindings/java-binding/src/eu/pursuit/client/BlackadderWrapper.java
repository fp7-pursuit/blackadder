/*
 * Copyright (C) 2011  Christos Tsilopoulos, Mobile Multimedia Laboratory, 
 * Athens University of Economics and Business 
 * All rights reserved.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License version
 * 2 as published by the Free Software Foundation.
 *
 * Alternatively, this software may be distributed under the terms of
 * the BSD license.
 *
 * See LICENSE and COPYING for more details.
 */

package eu.pursuit.client;

import java.nio.ByteBuffer;

import eu.pursuit.core.Event;
import eu.pursuit.core.Event.EventType;

public class BlackadderWrapper {	
	
	private static boolean USERSPACE = true;
	private final long baPtr;	
	private boolean closed = false;
	
	private static boolean configured = false;
	private static BlackadderWrapper instance = null;
	
	public static void configureObjectFile(String path_to_so){
		System.load(path_to_so);
		configured = true;
	}
	
	public static void setUserSpace(boolean userspace){
		USERSPACE = userspace;
	}
	
	public static BlackadderWrapper getWrapper() {
		if(instance == null){
			instance  = new BlackadderWrapper(USERSPACE);			
		}
		
		return instance;
	}
	
	private BlackadderWrapper(boolean userspace){
		if(!configured){
			throw new IllegalStateException("Shared library not set. Call static method BlackadderWrapper.configure() first");
		}
		
		int user = userspace? 1 : 0;		
		baPtr = create_new_ba(user);
	}

	public void publishScope(byte[] scope, byte [] prefixScope, byte strat, byte[] strategyOptions) {		
		c_publish_scope(baPtr, scope, prefixScope, strat, strategyOptions);
	}	

	public void publishItem(byte[] scope, byte[] prefixScope, byte strat, byte[] strategyOptions) {
		c_publish_item(baPtr, scope, prefixScope, strat, strategyOptions);		
	}

	public void unpublishScope(byte[] scope, byte[] prefixScope, byte strat, byte[] strategyOptions) {
		c_unpublish_scope(baPtr, scope, prefixScope, strat, strategyOptions);
	}

	public void unpublishItem(byte[] scope, byte[] prefixScope, byte strat, byte[] strategyOptions) {
		c_unpublish_item(baPtr, scope, prefixScope, strat, strategyOptions);
	}

	public void subscribeScope(byte[] scope, byte[] prefixScope, byte strat, byte[] strategyOptions) {
		c_subscribe_scope(baPtr, scope, prefixScope, strat, strategyOptions);
	}

	public void subscribeItem(byte[] scope, byte[] prefixScope, byte strat, byte[] strategyOptions) {
		c_subscribe_item(baPtr, scope, prefixScope, strat, strategyOptions);
	}

	public void unsubscribeScope(byte[] scope, byte[] prefixScope,	byte strat, byte[] strategyOptions) {
		c_unsubscribe_scope(baPtr, scope, prefixScope, strat, strategyOptions);
	}

	public void unsubscribeItem(byte[] scope, byte[] prefixScope, byte strat, byte[] strategyOptions) {
		c_unsubscribe_item(baPtr, scope, prefixScope, strat, strategyOptions);
	}	

	public void publishData(byte[] scope, byte strat, byte[] strategyOptions, byte[] jData) {			
		c_publish_data(baPtr, scope, strat, strategyOptions, jData, jData.length);		
	}
	
	public void publishData(byte[] scope, byte strat, byte[] strategyOptions, ByteBuffer buffer) {			
		c_publish_data_direct(baPtr, scope, strat, strategyOptions, buffer, buffer.capacity());		
	}
	
	public Event getNextEventDirect() {
		EventInternal e = new EventInternal();
		long event_ptr = c_nextEvent_direct(baPtr, e);		
		
		EventType type = EventType.getById(e.getType());
		Event retval = null;
		if(e.getData()!=null){
			retval = new Event(type, e.getId(), e.getData(), e.getData().capacity()); 
		}else{
			retval = new Event(type, e.getId());
		}
		
		retval.setNativeMemoryMappings(this, event_ptr);		
		return retval;
	}
	
	public void deleteEvent(long event_ptr) {
		c_delete_event(baPtr, event_ptr);
		
	}

	/*
	 * package protected for a reason
	 * */
	void close() {
		if(!closed){
			closed = true;			
			delete_ba(baPtr);
		}		
	}
	
	@Override
	protected void finalize() throws Throwable {
		close();
		super.finalize();
	}

	/* native methods */
	private native long create_new_ba(int userspace);
	private native void delete_ba(long ba_ptr);
	private native void c_publish_scope(long ba_ptr, byte[] scope, byte[] prefixScope, byte strat, byte[] strategyOptions);
	private native void c_publish_item(long ba_ptr, byte[] scope, byte[] prefixScope, byte strat, byte[] strategyOptions);
	private native void c_unpublish_scope(long ba_ptr, byte[] scope, byte[] prefixScope, byte strat, byte[] strategyOptions);
	private native void c_unpublish_item(long ba_ptr, byte[] scope, byte[] prefixScope, byte strat, byte[] strategyOptions);
	private native void c_subscribe_scope(long ba_ptr, byte[] scope, byte[] prefixScope, byte strat, byte[] strategyOptions);
	private native void c_subscribe_item(long ba_ptr, byte[] scope, byte[] prefixScope, byte strat, byte[] strategyOptions);
	private native void c_unsubscribe_scope(long ba_ptr, byte[] scope, byte[] prefixScope, byte strat, byte[] strategyOptions);
	private native void c_unsubscribe_item(long ba_ptr, byte[] scope, byte[] prefixScope, byte strat, byte[] strategyOptions);
	
	
	private native void c_publish_data(long ba_ptr, byte[] scope, byte strat, byte[] strategyOptions, byte [] dataBuffer, int length);
	private native void c_publish_data_direct(long ba_ptr, byte[] scope, byte strat, byte[] strategyOptions, ByteBuffer buffer, int length);
	
	private native long c_nextEvent_direct(long baPtr, EventInternal e);
	
	private native void c_delete_event(long baPtr, long event_ptr);		
}
