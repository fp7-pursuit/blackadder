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

import util.IDGenerator;
import util.IDGenerator.IDStrategy;

public class IDGeneratorTest {

	public static void Test1() throws DecoderException, NoSuchAlgorithmException, UnsupportedEncodingException{
		IDGenerator generator = new IDGenerator();
		System.out.println(generator.getNextID(null, IDStrategy.SEQUENTIAL));
		System.out.println(generator.getNextID(null, IDStrategy.SEQUENTIAL));
		System.out.println(generator.getNextID(null, IDStrategy.SEQUENTIAL));
		System.out.println(generator.getNextID(null, IDStrategy.SEQUENTIAL));
		System.out.println(generator.getNextID(null, IDStrategy.SEQUENTIAL));
		System.out.println(generator.getNextID(null, IDStrategy.SEQUENTIAL));
	}
	
	
	public static void Test2() throws DecoderException, NoSuchAlgorithmException, UnsupportedEncodingException{
		IDGenerator generator = new IDGenerator();
		System.out.println(generator.getNextID("ben", IDStrategy.RANDOM));
		System.out.println(generator.getNextID("alice", IDStrategy.RANDOM));
		System.out.println(generator.getNextID("bob", IDStrategy.RANDOM));
		System.out.println(generator.getNextID("dirk", IDStrategy.RANDOM));
		System.out.println(generator.getNextID("george", IDStrategy.RANDOM));
		System.out.println(generator.getNextID("jimmy", IDStrategy.RANDOM));
	}
	
	public static void main(String [] args) throws DecoderException, NoSuchAlgorithmException, UnsupportedEncodingException{
//		Test1();
		Test2();
	}
}
