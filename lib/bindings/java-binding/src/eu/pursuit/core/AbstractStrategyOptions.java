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

public abstract class AbstractStrategyOptions implements StrategyOptions{

	public byte [] toByteArray(){
		ByteBuffer buffer = toByteBuffer();
		return buffer.array();
	}
	
	public void fromBytes(byte [] array){
		ByteBuffer buffer = ByteBuffer.wrap(array);
		fromBuffer(buffer);
	}
}
