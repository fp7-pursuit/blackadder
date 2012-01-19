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

public enum Strategy {
	NODE((byte) 0), 
	LINK_LOCAL((byte) 1), 
	DOMAIN_LOCAL((byte) 2),
	IMPLICIT_RENDEZVOUS((byte)3),
	BROADCAST_IF((byte)4);
	
	private final byte byteValue;
	
	private Strategy(byte val){
		this.byteValue = val;
	}
	
	public byte byteValue(){
		return byteValue;
	}

	public static Strategy byValue(byte b) {
		Strategy strategy = null;
		for (Strategy strat: values()) {
			if(strat.byteValue == b){
				strategy = strat;
				break;
			}
		}
		
		return strategy;
	}

}
