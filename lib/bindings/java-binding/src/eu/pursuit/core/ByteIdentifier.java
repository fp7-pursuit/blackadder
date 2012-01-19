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
import java.util.Arrays;

public class ByteIdentifier {
	private final byte[] id;	

	public ByteIdentifier(byte[] id) {
		this.id = id;		
	}
	
	public ByteIdentifier() {
		this.id = new byte[0];		
	}

	public ByteIdentifier(byte value, int length) {
		id = new byte[length];
		Arrays.fill(id, value);
	}

	public ByteIdentifier(byte[] bytes, int length) {
		ByteBuffer buffer = ByteBuffer.allocate(length);
		if (bytes.length >= length){
			buffer.put(bytes, 0, length);
		}else{
			byte [] leftover = new byte[length-bytes.length];
			Arrays.fill(leftover, (byte)0);
			buffer.put(leftover);
			buffer.put(bytes);
		}
		this.id = buffer.array();
	}

	public byte[] getId() {
		return id;
	}
	
	public int length(){
		return this.id.length;
	}

	@Override
	public int hashCode() {
		final int prime = 31;
		int result = 1;
		result = prime * result + Arrays.hashCode(id);
		return result;
	}

	@Override
	public boolean equals(Object obj) {
		if (this == obj)
			return true;
		if (obj == null)
			return false;
		if (getClass() != obj.getClass())
			return false;
		ByteIdentifier other = (ByteIdentifier) obj;
		if (!Arrays.equals(id, other.id))
			return false;
		return true;
	}
	
	public static ByteIdentifier createEmpty() {
		return new ByteIdentifier(new byte[0]);
	}	
}
