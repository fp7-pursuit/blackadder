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

package model;

import java.sql.Timestamp;
import java.text.DateFormat;

/**
 * A class to represent a single video stream.
 * Covers the class, StreamingInfo, in the C implementation (I think...)
 * @author Ben Tagger
 * @version Started 14/10/2011
 */

public class VideoStream {

	private String absolutePath, channelID, timestamp, rid;
	private boolean stream;


	public VideoStream(String absolutePath, String channelID, String rID) {
		super();
		this.absolutePath = absolutePath;
		this.channelID = channelID;
		this.rid = rID;
		java.util.Date date= new java.util.Date();
		timestamp = DateFormat.getInstance().format(date);
	}
	
	public String getAbsolutePath() {
		return absolutePath;
	}

	public void setAbsolutePath(String absolutePath) {
		this.absolutePath = absolutePath;
	}

	public String getChannelID() {
		return channelID;
	}

	public void setChannelID(String channelID) {
		this.channelID = channelID;
	}

	public String getTimestamp() {
		return timestamp;
	}

	public void setTimestamp(String timestamp) {
		this.timestamp = timestamp;
	}

	public String getrID() {
		return rid;
	}

	public void setrID(String rID) {
		this.rid = rID;
	}
	
	public String getRid() {
		return rid;
	}

	public void setRid(String rid) {
		this.rid = rid;
	}

	public boolean isStream() {
		return stream;
	}

	public void setStream(boolean stream) {
		this.stream = stream;
	}

	// For testing purposes.
	public static void main(String [] args){
		java.util.Date date= new java.util.Date();
		System.out.println(new Timestamp(date.getTime()));
	}
	
}
