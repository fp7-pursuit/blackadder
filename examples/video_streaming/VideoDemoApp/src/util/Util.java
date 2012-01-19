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
import java.security.MessageDigest;
import java.security.NoSuchAlgorithmException;

/**
 * A utilities class for the Pursuit Video GUI
 * @author Ben Tagger
 * @version Started 13/10/2011
 */

public class Util {

	/**
	 * Converts a hexidecimal number to a string
	 * @param hex - the hexidecimal number.
	 * @return - the string representation
	 */
	public String hexToCharArray(String hex){	  
		StringBuilder sb = new StringBuilder();
		StringBuilder temp = new StringBuilder();
		// cuts the hex into blocks of 2.
		for( int i=0; i<hex.length()-1; i+=2 ){	 
			// grab the hex in pairs
			String output = hex.substring(i, (i + 2));
			// convert hex to decimal
			int decimal = Integer.parseInt(output, 16);
			// convert the decimal to character
			sb.append((char)decimal);
			temp.append(decimal);
		}
		return sb.toString();
	}

	/**
	 * Converts a string to a hexidecimal number
	 * @param charString - the String
	 * @return - the hexidecimal number.
	 */
	public static String charArrayToHex(String charString){
		StringBuffer hex = new StringBuffer();
		// For each character (converted to number 0-255)
		for (int i = 0; i < charString.length(); i++){
			// TODO check if there's a length problem here....
			hex.append(Integer.toHexString((int)charString.charAt(i)).toUpperCase());
		}
		return hex.toString();
	}
	
	/**
	 * Converts a string to a hexidecimal number
	 * @param byteArray - the String
	 * @return - the hexidecimal number.
	 */
	public static String byteArrayToHex(byte [] byteArray){
		StringBuffer hex = new StringBuffer();
		// For each character (converted to number 0-255)
		for (int i = 0; i < byteArray.length; i++){
			// TODO check if there's a length problem here....
			String thing = Integer.toHexString((int)byteArray[i]);
			if (thing.length() == 1){
				thing = "0" + thing;
			}else{
				int len = thing.length();
				thing = thing.substring(len-2, len);
			}
			hex.append(thing);
		}
		return hex.toString();
	}

	/**
	 * 
	 * @param data
	 */
	public static void printBytes(byte[] data) {
		for (byte b : data) {
			System.err.print(b+" ");
		}
		System.err.println();	
	}

	public static String getString(byte [] data){
		return new String(data); 
	}

	private static String convertToHex(byte[] data) { 
		StringBuffer buf = new StringBuffer();
		for (int i = 0; i < data.length; i++) { 
			int halfbyte = (data[i] >>> 4) & 0x0F;
			int two_halfs = 0;
			do { 
				if ((0 <= halfbyte) && (halfbyte <= 9)) 
					buf.append((char) ('0' + halfbyte));
				else 
					buf.append((char) ('a' + (halfbyte - 10)));
				halfbyte = data[i] & 0x0F;
			} while(two_halfs++ < 1);
		} 
		return buf.toString();
	} 

	public static String SHA1(String text) throws NoSuchAlgorithmException, UnsupportedEncodingException  { 
		MessageDigest md;
		md = MessageDigest.getInstance("SHA-1");
		byte[] sha1hash = new byte[16];
		md.update(text.getBytes(), 0, text.length());
		sha1hash = md.digest();
		return convertToHex(sha1hash);
	} 

}
