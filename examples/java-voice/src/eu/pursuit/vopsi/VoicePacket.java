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

public class VoicePacket {

	private byte[] buffer = null;

	public VoicePacket() {
		isData();
	}

	public VoicePacket(byte[] buffer) {
		this.buffer = buffer;
	}

	public byte[] getBuffer() {
		return buffer;
	}

	private void isData() {

		buffer[0] = 1;
		buffer[1] = 0;
		buffer[2] = 0;
	}

}
