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

import java.util.concurrent.BlockingQueue;

import javax.sound.sampled.AudioFormat;
import javax.sound.sampled.AudioSystem;
import javax.sound.sampled.DataLine;
import javax.sound.sampled.LineUnavailableException;
import javax.sound.sampled.SourceDataLine;

import eu.pursuit.vopsi.gui.Logger;

public class JavaSoundPlayer implements Runnable {

	int pointer = 0;
	private int BUFF_SIZE;

	AudioFormat audioFormat = new AudioFormat(AudioFormat.Encoding.PCM_SIGNED,
			8000, 16, 2, 4, 8000, false);

	private byte[] abBuffer;

	DataLine.Info dataLineInfo = new DataLine.Info(SourceDataLine.class,
			audioFormat);

	SourceDataLine source = null;

	private BlockingQueue<VoicePacket> queue;
	private boolean finished = false;

	public JavaSoundPlayer(BlockingQueue<VoicePacket> queue) {
		this.queue = queue;
		this.BUFF_SIZE = Configuration.getInstance().getBufSize();
		this.abBuffer = new byte[BUFF_SIZE];
	}

	public void run() {

		try {
			source = (SourceDataLine) AudioSystem
					.getSourceDataLine(audioFormat);
			source.open(audioFormat, abBuffer.length);
			source.start();
		} catch (LineUnavailableException e) {
			e.printStackTrace();
		}
		Logger.log("Audio receiver started...");

		while (!finished) {
			try {
				VoicePacket data = queue.take();
				source.write(data.getBuffer(), 0, data.getBuffer().length);

			} catch (InterruptedException e) {
				e.printStackTrace();
			}
		}
	}
	
	public synchronized void finish() {
		finished = true;
		
	}
}
