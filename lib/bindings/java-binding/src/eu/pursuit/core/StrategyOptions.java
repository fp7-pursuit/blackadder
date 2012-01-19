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

public interface StrategyOptions {
	
	public int getSerializedSize();
	
	public byte [] toByteArray();
	public void fromBytes(byte [] array);
	
	public ByteBuffer toByteBuffer();
	public void fromBuffer(ByteBuffer buffer);

}
