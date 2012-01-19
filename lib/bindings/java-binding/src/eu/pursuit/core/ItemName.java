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

package eu.pursuit.core;

import java.nio.ByteBuffer;
import java.util.ArrayList;
import java.util.List;

import org.apache.commons.codec.binary.Hex;

public class ItemName {
	private final ScopeID scopeId;
	private final ByteIdentifier rendezvousId;
	
	public ItemName(ScopeID scopeId, ByteIdentifier rendezvousId) {
		super();
		this.scopeId = scopeId;
		this.rendezvousId = rendezvousId;
	}

	public ScopeID getScopeId() {
		return scopeId;
	}

	public ByteIdentifier getRendezvousId() {
		return rendezvousId;
	}
	
	@Override
	public String toString(){
		return scopeId.toString() + "/" +Hex.encodeHexString(rendezvousId.getId());
	}

	public byte[] toByteArray() {
		int length = 0;
		if(scopeId != null){
			length+=scopeId.getLength();
		}
		
		if(rendezvousId!=null){
			length += rendezvousId.getId().length;			
		}
		
		ByteBuffer buffer = ByteBuffer.allocate(length);
		if(scopeId != null){
			scopeId.fill(buffer);
		}
		
		if(rendezvousId!=null){
			buffer.put(rendezvousId.getId());			
		}
		return buffer.array();
	}

	public static ItemName parseSerializedName(byte[] array, int segmentSize) {
		if(array.length % segmentSize != 0){			
			throw new IllegalArgumentException("array length ("+array.length+") not a multiple of segmentSize ("+segmentSize+")");
		}
		
		ByteBuffer bbuffer = ByteBuffer.wrap(array);
		List<ByteIdentifier> list = new ArrayList<ByteIdentifier>();
		int howmany = array.length / segmentSize;
		for (int i = 0; i < howmany-1; i++) {
			byte [] arr = new byte[segmentSize];
			bbuffer.get(arr);
			list.add(new ByteIdentifier(arr));			
		}
		ScopeID scope = new ScopeID(list);
		
		byte [] arr = new byte[segmentSize];
		bbuffer.get(arr);
		ByteIdentifier rid = new ByteIdentifier(arr);
		
		return new ItemName(scope, rid);		
	}
}
