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

import java.util.Arrays;
import java.util.concurrent.BlockingQueue;

import javax.sound.sampled.AudioFormat;
import javax.sound.sampled.AudioSystem;
import javax.sound.sampled.DataLine;
import javax.sound.sampled.LineUnavailableException;
import javax.sound.sampled.TargetDataLine;

public class JavaSoundRecorder implements Runnable {
	private int BUFF_SIZE;
	volatile boolean finished = false;

	AudioFormat audioFormat = new AudioFormat(AudioFormat.Encoding.PCM_SIGNED,
			8000, 16, 2, 4, 8000, false);

	DataLine.Info dataLineInfo = new DataLine.Info(TargetDataLine.class,
			audioFormat);

	TargetDataLine target = null;

	private BlockingQueue<VoicePacket> queue;

	public JavaSoundRecorder(BlockingQueue<VoicePacket> queue) {
		this.queue = queue;
		this.BUFF_SIZE = Configuration.getInstance().getBufSize();
	}

	public void run() {

		try {
			target = (TargetDataLine) AudioSystem
					.getTargetDataLine(audioFormat);
			target.open(audioFormat, BUFF_SIZE);
			target.start();
		} catch (LineUnavailableException e) {
			e.printStackTrace();
		}

		byte[] abBuffer = new byte[BUFF_SIZE];
		
		while (!finished) {

			int nBytesRead = target.read(abBuffer, 0, abBuffer.length);
			byte[] data = Arrays.copyOf(abBuffer, nBytesRead);
			VoicePacket packet = new VoicePacket(data);

			boolean ok = this.queue.offer(packet);

		}
	}

	public synchronized void finish() {
		finished = true;
		
	}
}
