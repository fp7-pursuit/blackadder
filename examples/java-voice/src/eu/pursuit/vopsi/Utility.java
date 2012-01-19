/*-
 * Copyright (C) 2011  Mobile Multimedia Laboratory, AUEB
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * Alternatively, this software may be distributed under the terms of the
 * BSD license.
 *
 * See LICENSE and COPYING for more details.
 */


package eu.pursuit.vopsi;

import java.io.UnsupportedEncodingException;
import java.security.MessageDigest;
import java.security.NoSuchAlgorithmException;

public class Utility {

	public static byte[] sha1(byte[] data) throws NoSuchAlgorithmException,
			UnsupportedEncodingException {

		MessageDigest md = MessageDigest.getInstance("SHA-1");
		byte[] sha1Hash = new byte[20];
		byte[] id = new byte[8]; /* CHANGED array length, original data.length */

		sha1Hash = md.digest(data);

		// Id generation
		System.arraycopy(sha1Hash, 0, id, 0, 8);
		for (int i = 20; i < id.length; i++) {
			id[i] = 0;
		}

		return id;
	}

	public static byte[] sha1(String name) throws NoSuchAlgorithmException,
			UnsupportedEncodingException {

		byte[] buffer = name.getBytes("iso-8859-1");
		return sha1(buffer);
	}

	public static boolean compare(byte[] id1, byte[] id2) {
		for (int i = 0; i < 32; i++) {
			if (id1[i] == id2[i]) {
				continue;
			}

			else {
				return false;
			}
		}

		return true;
	}

}
