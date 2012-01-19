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
import java.util.Arrays;
import java.util.Vector;
import java.util.concurrent.BlockingQueue;
import java.util.concurrent.LinkedBlockingQueue;

import eu.pursuit.vopsi.PPChannelReceiver;
import eu.pursuit.vopsi.PPChannelSender;

import eu.pursuit.client.BlackAdderClient;
import eu.pursuit.client.BlackadderWrapper;
import eu.pursuit.core.ByteIdentifier;
import eu.pursuit.core.Event;
import eu.pursuit.core.Event.EventType;
import eu.pursuit.core.ItemName;
import eu.pursuit.core.ScopeID;
import eu.pursuit.core.Strategy;
import eu.pursuit.vopsi.gui.Logger;

public class Node {

	private byte msg[] = new byte[8];
	private byte sendMsg[] = null;
	private int msgSeqNum = 0;
	private BlockingQueue<VoicePacket> recQueue;
	private BlockingQueue<VoicePacket> dataQueue;
	PPChannelReceiver channelRecv;
	PPChannelSender channelSend;
	private BlackAdderClient client;
	private Event event;
	private ScopeID sId;
	private ByteIdentifier myRid, otherEndId;
	private Vector<ByteIdentifier> participants;

	Strategy strategy = Strategy.DOMAIN_LOCAL;

	String sharedObjPath = null;
	public JavaSoundRecorder recorder;
	public SenderThread senderThread;
	public JavaSoundPlayer player;
	public ReceiverThread receiverThread;

	private enum MsgType {
		DATA, NEWPART, BYE
	}

	public Node() {
		sharedObjPath = Configuration.getInstance().getJniPath();
		BlackadderWrapper.configureObjectFile(sharedObjPath);
		client = BlackAdderClient.getInstance();
		participants = new Vector<ByteIdentifier>();
	}

	public void receiveCall(String callee) throws NoSuchAlgorithmException,
			UnsupportedEncodingException {

		myRid = new ByteIdentifier(Utility.sha1(callee));
		sId = new ScopeID();
		ByteIdentifier b = new ByteIdentifier(Utility.sha1(myRid.getId()));
		sId.addSegment(b);
		ItemName receiveData = new ItemName(sId, myRid);

		Logger.log("waiting for call...");

		client.subscribeItem(receiveData, strategy, null);
		event = client.getNextEvent();

		Logger.log("incoming call...");

		if (event.getType() == EventType.PUBLISHED_DATA) {
			msg = event.getDataCopy();
		}

		otherEndId = new ByteIdentifier(msg);

		msgSeqNum++;

		client.publishRootScope(b, strategy, null);

		ItemName sendData = new ItemName(sId, otherEndId);

		Logger.log("Sending response to caller...");
		client.publishItem(sendData, strategy, null);
		event = client.getNextEvent();

		if (event.getType() == EventType.START_PUBLISH) {

			client.publishData(event.getId(), msg, strategy, null);
			Logger.log("Conversation started...");
		}
		/*
		 * The callee responses to the invitation message verifying the pRId
		 * derived from the other's party name.
		 */

		msgSeqNum++;

		participants.add(otherEndId);

		this.channelSend = new PPChannelSender(sId, otherEndId, client);

		this.channelRecv = new PPChannelReceiver(sId, myRid, client);

		startVoice();
	}

	/*
	 * The caller sends the callee an invitation message that contains his RId
	 * derived from his name.
	 */
	public void makeCall(String caller, String callee)
			throws NoSuchAlgorithmException, UnsupportedEncodingException {

		myRid = new ByteIdentifier(Utility.sha1(caller));
		otherEndId = new ByteIdentifier(Utility.sha1(callee));
		sId = new ScopeID();
		ByteIdentifier b = new ByteIdentifier(Utility.sha1(otherEndId.getId()));
		sId.addSegment(b);
		ItemName sendData = new ItemName(sId, otherEndId);

		client.publishRootScope(b, strategy, null);
		Logger.log("Calling " + callee);

		client.publishItem(sendData, strategy, null);
		event = client.getNextEvent();

		System.arraycopy(myRid.getId(), 0, msg, 0, myRid.length());

		if (event.getType() == EventType.START_PUBLISH) {

			client.publishData(event.getId(), msg, strategy, null);
		}

		msgSeqNum++;

		ItemName receiveData = new ItemName(sId, myRid);

		client.subscribeItem(receiveData, strategy, null);
		event = client.getNextEvent();

		if (event.getType() == EventType.PUBLISHED_DATA) {
			msg = event.getDataCopy();
			System.out.println("Type :" + event.getType());
		}

		if (Arrays.equals(msg, myRid.getId())) {

			participants.add(otherEndId);
			Logger.log("Answer received...");
			Logger.log("Session established...");

		} else {
			Logger.log("Mismatched RId received as response...");
			Logger.log("Program will exit now.");
			System.exit(0);
		}

		this.channelSend = new PPChannelSender(sId, otherEndId, client);

		this.channelRecv = new PPChannelReceiver(sId, myRid, client);

		startVoice();
	}

	public void addParticipant(String participantName)
			throws NoSuchAlgorithmException, UnsupportedEncodingException {

		ByteIdentifier participantId = new ByteIdentifier(
				Utility.sha1(participantName));
		participants.add(participantId);

		client.publishRootScope(sId.get(0), strategy, null);

		ItemName updateData;

		for (int i = 0; i < participants.size(); i++) {

			updateData = new ItemName(sId, participants.get(i));

			System.out.println("Sending new participant to all...");
			Logger.log("Adding " + participantName + " to the conversation...");
			client.publishItem(updateData, strategy, null);
			event = client.getNextEvent();
			System.out.println("got a notification");

			prepareHeader(MsgType.NEWPART);

			System.arraycopy(participantId, 0, sendMsg, 1,
					participantId.length());

			if (event.getType() == EventType.START_PUBLISH) {

				client.publishData(event.getId(), sendMsg, strategy, null);
				System.out.println("New participant sent...");
			}
		}
	}

	public void leaveConversation() {

		client.publishRootScope(sId.get(0), strategy, null);

		ItemName leaveData;

		for (int i = 0; i < participants.size(); i++) {

			leaveData = new ItemName(sId, participants.get(i));

			System.out.println("Sending bye msg to all...");
			Logger.log("Sending Bye message to all...");
			client.publishItem(leaveData, strategy, null);
			event = client.getNextEvent();
			System.out.println("got a notification");

			prepareHeader(MsgType.BYE);

			System.arraycopy(myRid, 0, sendMsg, 5, myRid.length());

			if (event.getType() == EventType.START_PUBLISH) {

				client.publishData(event.getId(), sendMsg, strategy, null);
				System.out.println("New participant sent...");
			}
		}
	}

	public void startVoice() {
		this.dataQueue = new LinkedBlockingQueue<VoicePacket>();
		this.recQueue = new LinkedBlockingQueue<VoicePacket>();

		recorder = new JavaSoundRecorder(recQueue);
		Thread recorderThread = new Thread(recorder, "recorder");

		senderThread = new SenderThread(this.channelSend, this.recQueue);
		senderThread.start();
		recorderThread.start();

		try {
			Thread.sleep(100);
		} catch (InterruptedException e) {
			e.printStackTrace();
		}

		player = new JavaSoundPlayer(this.dataQueue);
		Thread playerThread = new Thread(player, "player");

		receiverThread = new ReceiverThread(this.channelRecv, this.dataQueue);
		receiverThread.start();
		playerThread.start();

	}

	public class SenderThread extends Thread {

		private BlockingQueue<VoicePacket> queue;
		private PPChannelSender channel;
		final VoicePacket STOP = new VoicePacket(null);

		public SenderThread(PPChannelSender channelSend,
				BlockingQueue<VoicePacket> recQueue) {
			this.channel = channelSend;
			this.queue = recQueue;
		}

		@Override
		public void run() {

			while (true) {

				try {
					VoicePacket p = this.queue.take();
					
					if(p == STOP){
						break;
					}
					
					this.channel.send(p.getBuffer());
				} catch (InterruptedException e) {
					e.printStackTrace();
				}
			}
		}

		public void finish() {
			this.queue.clear();
			this.queue.add(STOP);
		}

	}

	public class ReceiverThread extends Thread {

		private PPChannelReceiver channel;
		private BlockingQueue<VoicePacket> queue;
		private boolean finished = false;

		public ReceiverThread(PPChannelReceiver channelRecv,
				BlockingQueue<VoicePacket> dataQueue) {
			this.queue = dataQueue;
			this.channel = channelRecv;
		}

		@Override
		public void run() {

			while (!finished) {

				try {
					byte[] msg = channel.receive();
					if (msg.length > 0) {
						this.queue.put(new VoicePacket(msg));
					}
				} catch (InterruptedException e) {
					e.printStackTrace();
				}
			}
		}
		
		public void finish() {

			finished = true;
		}

	}

	public void prepareHeader(MsgType type) {

		/*
		 * First bit: Stream or document (1 for stream) Sec bit: Cacheable or
		 * not (0 for no) Third and fourth bit for three types of messages
		 * during conversation
		 */

		if (type.equals(MsgType.DATA)) {

			sendMsg[0] = 1;
			sendMsg[1] = 0;
			sendMsg[2] = 0;
		}

		else if (type.equals(MsgType.NEWPART)) {

			sendMsg[0] = 1;
			sendMsg[1] = 0;
			sendMsg[2] = 1;
			sendMsg[3] = 0;
		}

		// Bye msg
		else {
			sendMsg[0] = 1;
			sendMsg[1] = 0;
			sendMsg[2] = 1;
			sendMsg[3] = 1;
		}
	}
}
