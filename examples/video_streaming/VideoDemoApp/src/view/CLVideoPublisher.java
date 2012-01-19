/************************************************************************
 * Copyright (C) 2010-2012  Ben Tagger                                  *
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

package view;

import java.security.NoSuchAlgorithmException;
import java.util.HashMap;
import java.util.Map;
import java.util.Scanner;


import org.apache.commons.codec.DecoderException;
import org.apache.commons.codec.binary.Hex;

import pubsub.PublisherEventHandler;
import pubsub.VideoPublisher;

import util.IDGenerator;
import util.ProjectPropertiesSingleton;
import util.IDGenerator.IDStrategy;

import eu.pursuit.client.BlackAdderClient;
import eu.pursuit.client.BlackadderWrapper;
import eu.pursuit.core.ByteIdentifier;
import eu.pursuit.core.ScopeID;
import eu.pursuit.core.Strategy;

import java.io.File;
import java.io.UnsupportedEncodingException;

/**
 * The main class for the publisher interface
 * Uses a Command-line Interface
 * @author Ben Tagger
 * @version Started - 12/01/2012
 */
public class CLVideoPublisher implements PublisherView{

	private ByteIdentifier rootScopeId;
	private BlackAdderClient client;
	private IDGenerator rootGenerator;
	private ScopeID rootScope;
	private Strategy strategy = Strategy.DOMAIN_LOCAL;
	private VideoPublisher videoPublisher;

	private String list;	
	private Map<String, String> ridMappings;
	private Map<Integer, String> listMappings;


	/**
	 * Launch the application.
	 * @throws DecoderException 
	 * @throws UnsupportedEncodingException 
	 * @throws NoSuchAlgorithmException 
	 */
	public static void main(String[] args) throws DecoderException, NoSuchAlgorithmException, UnsupportedEncodingException {
		CLVideoPublisher pub = new CLVideoPublisher();

		//		Implement the command line interface.
		while (true){
			// print out commands
			System.err.println(pub.commandList());
			// wait for input
			Scanner sc = new Scanner(System.in);
			int i = sc.nextInt();
			// deal with input
			switch (i){
			case 1: // Print the list
				// refresh the list.
				pub.populatePublishList();
				System.err.println("\n\n----------Publications----------");
				System.err.println(pub.getList());
				break;
			case 2: // Add a publication
				System.err.println("\nEnter location of publication in folder ("+ ProjectPropertiesSingleton.getInstance().getProperty("DefaultMovieFolder") +")");
				String pubPath = ProjectPropertiesSingleton.getInstance().getProperty("DefaultMovieFolder") + sc.next();
				// Check the file exists.
				File file = new File(pubPath);
				if (!file.exists()){
					System.err.println("File does not exist. Returning...");
					break;
				}
				// publish the event. Under root for now...
				String newPubIDString = pub.rootGenerator.getNextID(pubPath, IDStrategy.RANDOM);
				pub.videoPublisher.publishVideo(newPubIDString, pubPath);
				pub.populatePublishList();
				System.err.println("done...");
				break;
			case 3: // Remove a publication
				// refresh the list.
				pub.populatePublishList();
				System.err.println("\n\n----------Publications----------");
				System.err.println(pub.getList());
				System.err.println("Enter the number of the publication to remove...");
				// check something has been selected
				int selected = sc.nextInt();
				String selectedS = pub.listMappings.get(selected);
				if (selectedS == null){
					System.err.println("Not a valid selection. Returning...");
					break;
				}
				System.err.println("You have selected "+ selectedS +" for removal. Continue... (Y or N)");
				String response = sc.next();
				if (response.equalsIgnoreCase("n")){
					System.err.println("Returning...");
					break;
				}else if (response.equalsIgnoreCase("y")){
					// unpublish the item.
					String rid = pub.ridMappings.get(selectedS);
					try {
						pub.videoPublisher.unpublishVideo(rid);
					} catch (DecoderException e1) {
						// TODO Auto-generated catch block
						e1.printStackTrace();
					}
				}else{
					System.err.println("Did not understand. Returning...");
					break;
				}
				break;
			case 4: // Exit
				System.err.println("Exiting...");
				pub.getClient().disconnect();
				System.exit(0);
				break;
			}
		}
	}

	/**
	 * Create the application.
	 * @throws DecoderException 
	 */
	public CLVideoPublisher() throws DecoderException {		

		// Setup the Blackadder environment.
		String sharedObjPath = ProjectPropertiesSingleton.getInstance().getProperty("BAWrapperPath");		
		BlackadderWrapper.configureObjectFile(sharedObjPath);		
		client = BlackAdderClient.getInstance();


		// publish the root scope where all videos will be published
		String rootScopeStr = "1111111111111111";
		ByteIdentifier rootId = new ByteIdentifier(Hex.decodeHex(rootScopeStr.toCharArray()));
		rootScope = new ScopeID(rootId);
		client.publishRootScope(rootId, strategy, null);
		rootGenerator = new IDGenerator();
		videoPublisher = new VideoPublisher(client, rootScope, strategy);
		// publish the catalog.
		videoPublisher.publishCatalog();
		ridMappings = new HashMap<String, String>();
		listMappings = new HashMap<Integer, String>();

		// Start the event handler
		PublisherEventHandler handler = new PublisherEventHandler(this, strategy);
		handler.start();
	}

	public ByteIdentifier getRootScopeId() {
		return rootScopeId;
	}

	public void setRootScopeId(ByteIdentifier rootScopeId) {
		this.rootScopeId = rootScopeId;
	}

	public BlackAdderClient getClient() {
		return client;
	}

	public void setClient(BlackAdderClient client) {
		this.client = client;
	}

	public IDGenerator getRootGenerator() {
		return rootGenerator;
	}

	public void setRootGenerator(IDGenerator rootGenerator) {
		this.rootGenerator = rootGenerator;
	}

	public ScopeID getRootScope() {
		return rootScope;
	}

	public void setRootScope(ScopeID rootScope) {
		this.rootScope = rootScope;
	}

	public VideoPublisher getVideoPublisher() {
		return videoPublisher;
	}

	public void setVideoPublisher(VideoPublisher videoPublisher) {
		this.videoPublisher = videoPublisher;
	}

	public void populatePublishList(){
		String catData = getVideoPublisher().getCatalogNames();

		// Get data in rows
		String [] rows = catData.split("--");
		list = "";
		int counter = 0;
		for (String item: rows){
			if (!item.equals("")) {
				// Get the RID
				String[] pre = item.split("@");
				String rid = pre[1];
				list += "\n" + pre[2];
				// retain the rid mapping.
				ridMappings.put(pre[2], rid);
				listMappings.put(++counter, pre[2]);
			}
		}
	}

	public String getList() {
		return list;
	}

	private String commandList(){
		String temp = "\n------------Commands------------";
		temp += "\n 1) Display publications...";
		temp += "\n 2) Publish a new publication...";
		temp += "\n 3) Unpublish a publication...";
		temp += "\n 4) Exit...";
		temp += "\n 0) Show options again...";
		temp += "\n Enter one number and press [enter]...\n";
		return temp;
	}
}
