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

import java.util.*;
import java.io.*;
import java.net.URL;

import java.net.URLClassLoader;

/**
 * A class to load project properties
 * Uses the Singleton pattern
 * To retrieve property, use: ProjectPropertiesSingleton.getInstance().getProperty(propKey)
 * @author Ben Tagger
 * @version 13/11/2011
 */

public class ProjectPropertiesSingleton {

	// Variable holding the singleton
	private static ProjectPropertiesSingleton singleton = null;
	private static boolean printPath = false;

	// The properties object
	private Properties props = null;
	

	// Private constructor - handle to object is made through getInstance()
	private ProjectPropertiesSingleton() {
		props = new Properties();

		// Load the properties file from project.properties
		//Get the System Classloader
		ClassLoader sysClassLoader = ClassLoader.getSystemClassLoader();

		//Get the URLs from the classpath
		URL[] urls = ((URLClassLoader)sysClassLoader).getURLs();

		File f = null;
		// Search for property url.
		for(int i=0; i< urls.length; i++)
		{
			if (urls[i].getPath().contains("project.properties")){
				f = new File(urls[i].getPath());
				break;
			}
			if (printPath) System.out.println(urls[i].getPath());
		} 
		// If not specified in classpath, use the default location
		if (f == null){
			// use local version
			f = new File("project.properties");
		}
		
		System.out.println("------PROPERTY FILENAME-------\n" + f.getAbsolutePath());

		try {
			FileInputStream fis = new FileInputStream(f);
			props.load(fis);
			System.out.println("props="+props);
		}catch (Exception e) {
			System.out.println("Could not load properties file: " + f.getAbsolutePath());
			e.printStackTrace();
		}
	}

	// Checks whether object has been instantiated
	public synchronized static ProjectPropertiesSingleton getInstance() {
		if (singleton == null){
			singleton = new ProjectPropertiesSingleton();
		}
		return singleton;
	}

	public String getProperty(String key) {
        String value = null;
        if (props.containsKey(key))
            value = (String) props.get(key);
        else {
        	System.out.println("Could not retrieve property.");	
        }
        return value;
    }
	
	public Properties getProperties(){
		return props;
	}

}
