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

import javax.swing.JFileChooser;
import javax.swing.JFrame;
import javax.swing.JOptionPane;
import javax.swing.JPanel;
import java.awt.BorderLayout;
import java.awt.FlowLayout;
import java.awt.List;

import javax.swing.JButton;
import java.awt.event.ActionListener;
import java.awt.event.ActionEvent;
import java.security.NoSuchAlgorithmException;
import java.util.HashMap;
import java.util.Map;

import javax.swing.filechooser.FileNameExtensionFilter;

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
import java.awt.event.WindowAdapter;
import java.awt.event.WindowEvent;
import java.io.UnsupportedEncodingException;

/**
 * The main class for the publisher interface
 * @author Ben Tagger
 * @version Started - 13/11/2011
 */
public class VideoPublisherGUI implements PublisherView{

	private JFrame frmBlackvidPubsubber;
	
	private ByteIdentifier rootScopeId;
	private BlackAdderClient client;
	private IDGenerator rootGenerator;
	private ScopeID rootScope;
	private Strategy strategy = Strategy.DOMAIN_LOCAL;
	private VideoPublisher videoPublisher;

	private List list;
	
	private Map<String, String> ridMappings;

	

	/**
	 * Launch the application.
	 */
	public static void main(String[] args) {
		EventQueue.invokeLater(new Runnable() {
			public void run() {
				try {
					VideoPublisherGUI window = new VideoPublisherGUI();
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
	 */
	public VideoPublisherGUI() throws DecoderException {
		initialize();		
		
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

		// Start the event handler
		PublisherEventHandler handler = new PublisherEventHandler(this, strategy);
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
				// unpublish everything
				videoPublisher.cleanup();
				// then disconnect...
				client.disconnect();
			}
		});
		frmBlackvidPubsubber.setTitle("BlackVid Publisher");
		frmBlackvidPubsubber.setBounds(100, 100, 450, 300);
		frmBlackvidPubsubber.setDefaultCloseOperation(JFrame.EXIT_ON_CLOSE);
		frmBlackvidPubsubber.getContentPane().setLayout(new BorderLayout(0, 0));
		
		final JPanel panel = new JPanel();
		frmBlackvidPubsubber.getContentPane().add(panel, BorderLayout.SOUTH);
		panel.setLayout(new FlowLayout(FlowLayout.CENTER, 5, 5));
		
		JButton publishButton = new JButton("publish video");
		publishButton.addActionListener(new ActionListener() {
			public void actionPerformed(ActionEvent arg0) {
				// Open file dialog
				JFileChooser chooser = new JFileChooser();
			    FileNameExtensionFilter filter = new FileNameExtensionFilter(
			        "Video Files", "mov", "mpg", "mkv", "mp4", "avi", "mpeg");
			    chooser.setFileFilter(filter);
			    int returnVal = chooser.showOpenDialog(frmBlackvidPubsubber);
			    if(returnVal == JFileChooser.APPROVE_OPTION) {
			       System.out.println("You chose to open this file: " +
			            chooser.getSelectedFile().getAbsolutePath());
			    }
				
				try {
					// publish the event. Under root for now...
				    String newPubIDString = rootGenerator.getNextID(chooser.getSelectedFile().getAbsolutePath(), IDStrategy.RANDOM);
				    videoPublisher.publishVideo(newPubIDString, chooser.getSelectedFile().getAbsolutePath());
					populatePublishList();
					
				} catch (DecoderException e) {
					// Report the failed event.
					JOptionPane.showConfirmDialog(panel, "Could Not Publish the Video...");
					e.printStackTrace();
				} catch (NoSuchAlgorithmException e) {
					// TODO Auto-generated catch block
					e.printStackTrace();
				} catch (UnsupportedEncodingException e) {
					// TODO Auto-generated catch block
					e.printStackTrace();
				}
		    }
		});
		panel.add(publishButton);
		
		JButton unpublishButton = new JButton("unpublish");
		unpublishButton.addActionListener(new ActionListener() {
			public void actionPerformed(ActionEvent e) {
				// unpublish a video.
				// check something has been selected
				String selected = getList().getSelectedItem();
				if (selected != null){
					// get the rid
					String rid = ridMappings.get(selected);
					//unpublish by rid
					ByteIdentifier vidID;
					try {
						videoPublisher.unpublishVideo(rid);
					} catch (DecoderException e1) {
						// TODO Auto-generated catch block
						e1.printStackTrace();
					}					
					
					populatePublishList();
				}
			}
		});
		
		JButton btnPublishStream = new JButton("publish stream");
		btnPublishStream.addActionListener(new ActionListener() {
			public void actionPerformed(ActionEvent e) {
				String httpStr = JOptionPane.showInputDialog("Please enter URL of media stream.");
				System.out.println(httpStr);
				try {
					// publish the event. Under root for now...
				    String newPubIDString = rootGenerator.getNextID(httpStr, IDStrategy.RANDOM);
				    videoPublisher.publishVideo(newPubIDString, httpStr);
					populatePublishList();
					
				} catch (DecoderException e2) {
					// Report the failed event.
					JOptionPane.showConfirmDialog(panel, "Could Not Publish the Media...");
					e2.printStackTrace();
				} catch (NoSuchAlgorithmException e3) {
					// TODO Auto-generated catch block
					e3.printStackTrace();
				} catch (UnsupportedEncodingException e4) {
					// TODO Auto-generated catch block
					e4.printStackTrace();
				}
			}
		});
		panel.add(btnPublishStream);
		panel.add(unpublishButton);
		
		list = new List();
		frmBlackvidPubsubber.getContentPane().add(list, BorderLayout.CENTER);
		
		JPanel panel_1 = new JPanel();
		frmBlackvidPubsubber.getContentPane().add(panel_1, BorderLayout.NORTH);
		panel_1.setLayout(new FlowLayout(FlowLayout.CENTER, 5, 5));
		
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
