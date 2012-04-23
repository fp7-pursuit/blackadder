/*-
 * Copyright (C) 2011-2012  Mobile Multimedia Laboratory, AUEB
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

import java.io.BufferedReader;
import java.io.DataInputStream;
import java.io.FileInputStream;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.util.HashMap;
import java.util.Properties;
import java.util.StringTokenizer;
import java.io.FileNotFoundException;
import java.io.File;

public class Configuration {

	Properties p = new Properties();
	
	private static Configuration instance = null;

	protected Configuration() {

	}

	public static Configuration getInstance() {
		if (instance == null) {
			instance = new Configuration();
		}
		return instance;
	}

	public void configure(String file) {

		try {
			InputStream resourceAsStream = this.getClass().getResourceAsStream(
					file);
			if (resourceAsStream==null)
			{
				File f =new File(file);
				if (!f.exists()){
					throw new FileNotFoundException("Cannot find config file "+file);
				}
				resourceAsStream=new FileInputStream(file);
			}
			p.load(resourceAsStream);

		} catch (Exception e) {
			System.err.println("Error: " + e.getMessage());
		}
	}

	public int getBufSize() {

		return Integer.parseInt(p.getProperty("BUFFER_SIZE"));
	}

	public String getJniPath() {

		return p.getProperty("JNI_PATH");
	}
}
