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

package util;

import java.io.UnsupportedEncodingException;
import java.security.NoSuchAlgorithmException;

import org.apache.commons.codec.DecoderException;
import org.apache.commons.codec.binary.Hex;

/**
 * A Class that builds the ID for a publication
 * @author Ben Tagger
 * @version 13/11/2011
 */
public class IDGenerator {

	private long currentID;
	public enum IDStrategy {SEQUENTIAL, RANDOM};
	private int PURSUIT_ID_LEN = 16;
	public IDGenerator(){
		// 0 is reserved.
		currentID = 1;
	}

	public String getNextID(String text, IDStrategy strategy) throws DecoderException, NoSuchAlgorithmException, UnsupportedEncodingException{
		String total = null;
		switch (strategy){
		case SEQUENTIAL:
			String id = "" + currentID++;
			String prefix = "";
			for (int i = id.length(); i < 16; i++){
				prefix += "0";
			}
			total = prefix + id;
			break;
		case RANDOM:
			total = Util.SHA1(text);
			break;
		}
		if (total.length() > PURSUIT_ID_LEN){
			return total.substring(0, PURSUIT_ID_LEN);
		}else{
			return total;
		}
	}
}
