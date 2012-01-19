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

public class Util {

	/*
	 * Throw a NullpointerException if o is null
	 */
	public static void checkNull(Object o) {
		if (o == null)
			throw new NullPointerException();
	}

	public static void checkNull(Object o, String mesg) {
		if (o == null)
			throw new NullPointerException(mesg);
	}

	public static void checkIllegal(Object o, String fieldName) {
		if (o == null) {
			throw new IllegalArgumentException("field " + fieldName
					+ " not initialized");
		}

	}

	public static <T> void printArray(T[] array) {
		for (T t : array) {
			System.out.print(t + " ");
		}
		System.out.println();
	}

	public static void printArray(byte[] array) {
		for (byte b : array) {
			System.out.print(b + " ");
		}
		System.out.println();
	}
}
