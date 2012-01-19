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

package view;

import java.awt.EventQueue;

import javax.swing.JFrame;
import javax.swing.JOptionPane;
import javax.swing.JPanel;
import java.awt.BorderLayout;
import java.awt.FlowLayout;
import java.awt.List;

import javax.swing.JButton;

import org.apache.commons.codec.DecoderException;
import org.apache.commons.codec.binary.Hex;

import pubsub.SubscriberEventHandler;
import pubsub.VideoSubscriber;

import util.ProjectPropertiesSingleton;

import eu.pursuit.client.BlackAdderClient;
import eu.pursuit.client.BlackadderWrapper;
import eu.pursuit.core.ByteIdentifier;
import eu.pursuit.core.ScopeID;
import eu.pursuit.core.Strategy;

import java.awt.event.ActionListener;
import java.awt.event.ActionEvent;
import java.net.SocketException;
import java.util.HashMap;
import java.util.Map;
import java.awt.event.WindowAdapter;
import java.awt.event.WindowEvent;

/**
 * The main class for the subscriber interface
 * @author Ben Tagger
 * @version Started - 13/11/2011
 */
public class VideoSubscriberGUI{

	private JFrame frmBlackvidPubsubber;
	
	private ByteIdentifier rootScopeId;
	private BlackAdderClient client;
	private ScopeID rootScope;
	private VideoSubscriber videoSubscriber;
	private long channelID;
	private List list;
	private Map<String, String> ridMappings;
	private Strategy strategy = Strategy.DOMAIN_LOCAL;
	

	/**
	 * Launch the application.
	 */
	public static void main(String[] args) {
		EventQueue.invokeLater(new Runnable() {
			public void run() {
				try {
					VideoSubscriberGUI window = new VideoSubscriberGUI();
					window.frmBlackvidPubsubber.setVisible(true);
				} catch (Exception e) {
					e.printStackTrace();
				}
			}
		});
	}

	/**
	 * Create the application.
	 * @throws DecoderException 
	 * @throws SocketException 
	 */
	public VideoSubscriberGUI() throws DecoderException, SocketException {
		initialize();		
		
		// Setup the Blackadder environment.
		String sharedObjPath = ProjectPropertiesSingleton.getInstance().getProperty("BAWrapperPath");		
		BlackadderWrapper.configureObjectFile(sharedObjPath);		
		client = BlackAdderClient.getInstance();
		channelID = 1;
		
		// Initialise the rootscope
		// publish the root scope where all videos will be published
		String rootScopeStr = "1111111111111111";
        ByteIdentifier rootId = new ByteIdentifier(Hex.decodeHex(rootScopeStr.toCharArray()));
        rootScope = new ScopeID(rootId);
		
		// immediately subscribe to the catalog
		videoSubscriber = new VideoSubscriber(client, rootScope, strategy);
		
		// initialise ridMappings
		ridMappings = new HashMap<String, String>();
		
		// Start the event handler
		SubscriberEventHandler handler = new SubscriberEventHandler(this);
		handler.start();
		
	}

	/**
	 * Initialize the contents of the frame.
	 */
	private void initialize() {
		frmBlackvidPubsubber = new JFrame();
		frmBlackvidPubsubber.addWindowListener(new WindowAdapter() {
			@Override
			public void windowClosing(WindowEvent e) {
				// unsubscribe from catalog - just in case
				videoSubscriber.cleanup();
				// and then disconnect.
				client.disconnect();
			}
		});
		frmBlackvidPubsubber.setTitle("BlackVid Subscriber");
		frmBlackvidPubsubber.setBounds(100, 100, 450, 300);
		frmBlackvidPubsubber.setDefaultCloseOperation(JFrame.EXIT_ON_CLOSE);
		frmBlackvidPubsubber.getContentPane().setLayout(new BorderLayout(0, 0));
		
		final JPanel panel = new JPanel();
		frmBlackvidPubsubber.getContentPane().add(panel, BorderLayout.SOUTH);
		panel.setLayout(new FlowLayout(FlowLayout.CENTER, 5, 5));
		
		JButton subscribeButton = new JButton("subscribe");
		subscribeButton.addActionListener(new ActionListener() {
			public void actionPerformed(ActionEvent arg0) {				
				// check something was selected.
				String selected = list.getSelectedItem();
				if (selected != null){
					// get the rid of the video clicked.
					String rid = ridMappings.get(selected);
					if(!videoSubscriber.subscribeVideo(rid))
						JOptionPane.showMessageDialog(frmBlackvidPubsubber, "Something went wrong with the subscription.");
				}else{
					// error
					JOptionPane.showMessageDialog(frmBlackvidPubsubber, "You must select a video from the list.");
				}
			}
		});
		panel.add(subscribeButton);
		
		JButton unsubscribeButton = new JButton("unsubscribe");
		unsubscribeButton.addActionListener(new ActionListener() {
			public void actionPerformed(ActionEvent e) {
				// check something was selected.
				String selected = list.getSelectedItem();
				if (selected != null){
					// get the rid of the video clicked.
					String rid = ridMappings.get(selected);
					if(!videoSubscriber.unsubscribeVideo(rid))
						JOptionPane.showMessageDialog(frmBlackvidPubsubber, "Something went wrong with the subscription.\nPerhaps you had not subscribed to that video?");
				}else{
					// error
					JOptionPane.showMessageDialog(frmBlackvidPubsubber, "You must select a video from the list.");
				}
			}
		});
		panel.add(unsubscribeButton);
		
		list = new List();
		frmBlackvidPubsubber.getContentPane().add(list, BorderLayout.CENTER);
		
		JPanel panel_1 = new JPanel();
		frmBlackvidPubsubber.getContentPane().add(panel_1, BorderLayout.NORTH);
		panel_1.setLayout(new FlowLayout(FlowLayout.CENTER, 5, 5));
		
		JButton refreshButton = new JButton("Refresh Catalogue");
		refreshButton.addActionListener(new ActionListener() {
			public void actionPerformed(ActionEvent arg0) {
				// unsubscribe and then subscribe again.
//				videoSubscriber.unsubscribeCatalog();
				videoSubscriber.subscribeCatalog();
			}
		});
		panel_1.add(refreshButton);
	}
	
	private String getChannelID(){
		return "" + channelID++;
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

	public ScopeID getRootScope() {
		return rootScope;
	}

	public void setRootScope(ScopeID rootScope) {
		this.rootScope = rootScope;
	}

	public void setChannelID(long channelID) {
		this.channelID = channelID;
	}
	
	public VideoSubscriber getVideoSubscriber() {
		return videoSubscriber;
	}

	public void setVideoSubscriber(VideoSubscriber videoSubscriber) {
		this.videoSubscriber = videoSubscriber;
	}
	
	public void populateCatalogList(String content){
		// Get data in rows
		String [] rows = content.split("--");
		getList().removeAll();
		for (String item: rows){
			if (!item.equals("")) {
				// Get the RID
				String[] pre = item.split("@");
				String rid = pre[1];
				getList().add(pre[2]);
				// retain the rid mapping.
				ridMappings.put(pre[2], pre[1]);
			}
		}
	}
	
	public List getList() {
		return list;
	}
}
