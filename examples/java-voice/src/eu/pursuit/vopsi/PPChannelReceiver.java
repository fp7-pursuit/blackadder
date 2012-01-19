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
import java.security.NoSuchAlgorithmException;

import eu.pursuit.client.BlackAdderClient;
import eu.pursuit.core.ByteIdentifier;
import eu.pursuit.core.Event;
import eu.pursuit.core.Event.EventType;
import eu.pursuit.core.ItemName;
import eu.pursuit.core.ScopeID;

public class PPChannelReceiver {
	private ScopeID scope;
	private ByteIdentifier rid;
	private BlackAdderClient ba;
	private Event event;
	private ItemName receiveData;
	private byte[] msg;

	public PPChannelReceiver(String seed, ByteIdentifier rid,
			BlackAdderClient ba) throws UnsupportedEncodingException,
			NoSuchAlgorithmException {
		this.rid = new ByteIdentifier(Utility.sha1(seed));
		this.scope = new ScopeID();
		scope.addSegment(new ByteIdentifier(Utility.sha1(rid.getId())));
		this.ba = ba;
	}

	public PPChannelReceiver(ScopeID scope, ByteIdentifier rid,
			BlackAdderClient ba) {
		this.scope = scope;
		this.rid = rid;
		this.ba = ba;
		this.receiveData = new ItemName(scope, rid);
	}

	public byte[] receive() {

		event = ba.getNextEvent();

		if (event.getType() == EventType.PUBLISHED_DATA) {
			msg = event.getDataCopy();
		}
		event.freeNativeBuffer();
		return msg;
	}
}
