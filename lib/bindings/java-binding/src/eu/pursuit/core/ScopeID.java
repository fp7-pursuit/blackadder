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

public class ScopeID {

	public static int SEGMENT_SIZE = 8;

	private final List<ByteIdentifier> list = new ArrayList<ByteIdentifier>();;	
	private String str = "";
	
	public ScopeID() {		
	}
	
	public ScopeID(List<ByteIdentifier> ids) {
		Util.checkNull(ids);
		for (ByteIdentifier byteIdentifier : ids) {
			addSegment(byteIdentifier);
		}		
	}	
	
	public ScopeID(ByteIdentifier id) {
		addSegment(id);
	}

	public ScopeID addSegment(ByteIdentifier identifier) {
		Util.checkNull(identifier);	
		if(identifier.length() != SEGMENT_SIZE){
			throw new IllegalArgumentException("invalid segment size: "+identifier.length());
		}		
		
		if(!this.list.add(identifier)){
			throw new RuntimeException("failed to add to list");
		}			
		
		String hex = Hex.encodeHexString(identifier.getId());
		str += "/" + hex;		
		return this;
	}
	
	public ByteIdentifier get(int index){
		return this.list.get(index);
	}
	
	public int getLength() {
		int totalLength = 0;
		for (ByteIdentifier id : list) {
			totalLength += id.getId().length;
		}
		return totalLength;
	}
	
	public byte [] toByteArray(){				
		ByteBuffer buffer = ByteBuffer.allocate(getLength());
		fill(buffer);		
		return buffer.array();
	}
	
	public void fill(ByteBuffer buffer) {
		for (ByteIdentifier id : list) {
			buffer.put(id.getId());
		}
	}		

	@Override
	public int hashCode() {
		return list.hashCode();
	}

	@Override
	public boolean equals(Object obj) {
		if (this == obj)
			return true;
		if (obj == null)
			return false;
		if (getClass() != obj.getClass())
			return false;
		ScopeID otherScope = (ScopeID)obj;
		return list.equals(otherScope.list);
	}
	
	@Override
	public String toString(){
		return str;
	}
}
