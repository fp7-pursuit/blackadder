/*-
 * Copyright (C) 2011-2012  Mobile Multimedia Laboratory, AUEB
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

package eu.pursuit.vopsi.gui;

import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.io.UnsupportedEncodingException;
import java.security.NoSuchAlgorithmException;

import javax.imageio.ImageIO;
import javax.swing.ImageIcon;
import javax.swing.JButton;
import javax.swing.JFrame;
import javax.swing.JLabel;
import javax.swing.JOptionPane;
import javax.swing.JScrollPane;
import javax.swing.JTextArea;
import javax.swing.JTextField;
import javax.swing.SwingUtilities;
import javax.swing.UIManager;

import org.dyno.visual.swing.layouts.Bilateral;
import org.dyno.visual.swing.layouts.Constraints;
import org.dyno.visual.swing.layouts.GroupLayout;
import org.dyno.visual.swing.layouts.Leading;
import org.dyno.visual.swing.parser.listener.ThisClassModel;

import eu.pursuit.vopsi.Configuration;
import eu.pursuit.vopsi.Node;

public class VoPSI_App extends JFrame {

	private static final long serialVersionUID = 1L;
	private JLabel lblCallee;
	private static JButton btnClose;
	private static JTextField txtCallee;
	private static JTextField txtCaller;
	private static JLabel lblCaller;
	private TxtLog txtLog = new TxtLog();
	private JScrollPane jScrollPane0;
	private static JButton btnCall;
	private static final String PREFERRED_LOOK_AND_FEEL = "javax.swing.plaf.metal.MetalLookAndFeel";
	private static boolean isCaller, callStarted = false;
	private AppThreadCallee threadCallee;
	private AppThreadCaller threadCaller;

	public VoPSI_App() {
		initComponents();
		initHandlers();
	}

	private void initHandlers() {
		btnClose.addActionListener(new ActionListener() {

			// Send notification
			@Override
			public void actionPerformed(ActionEvent arg0) {
				
				if (callStarted){
					Logger.log("closing conversation...");
					btnClose.setText("Close");

			        if (isCaller)
			        {
			        	threadCaller.stopNode();
			        }
			        else{
			        	threadCallee.stopNode();
			        }
			        
			        callStarted = false;
					
				}
				else{
					Logger.log("closing");
					System.exit(0);
				}
				
			}
		});

		btnCall.addActionListener(new ActionListener() {

			@Override
			public void actionPerformed(ActionEvent e) {
				startCall();
			}
		});
	}

	private void initComponents() {
		setTitle("VoPSI - Voice Application over PSI Network");
		setResizable(false);
		setLayout(new GroupLayout());
		add(getLblCallee(), new Constraints(new Leading(12, 12, 12),
				new Leading(54, 12, 12)));
		add(getLblCaller(), new Constraints(new Leading(12, 12, 12),
				new Leading(16, 12, 12)));
		add(getJTextField1(), new Constraints(new Leading(125, 186, 12, 12),
				new Leading(50, 26, 12, 12)));
		add(getJTextField0(), new Constraints(new Leading(125, 186, 10, 10),
				new Leading(12, 26, 12, 12)));
		add(getJScrollPane0(), new Constraints(new Bilateral(12, 12, 22),
				new Bilateral(94, 12, 22)));
		add(getJButton1(), new Constraints(new Leading(370, 102, 12, 12),
				new Leading(15, 40, 37)));
		add(getJButton0(), new Constraints(new Leading(370, 102, 12, 12),
				new Leading(50, 12, 12)));
		setSize(500, 356);
		setDefaultCloseOperation(VoPSI_App.EXIT_ON_CLOSE);

		try {
			this.setIconImage(ImageIO.read(this.getClass().getResourceAsStream(
					"logo.png")));
		} catch (Exception e) {
			this.txtLog.log("Icon not loaded..");
		}

		Logger.logger = txtLog;

	}

	private JButton getJButton1() {
		if (btnCall == null) {
			btnCall = new JButton();
			if (isCaller) {
				btnCall.setText("Call");
			} else {
				btnCall.setText("Receive");
			}
		}
		return btnCall;
	}

	private JScrollPane getJScrollPane0() {
		if (jScrollPane0 == null) {
			jScrollPane0 = new JScrollPane();
			jScrollPane0.setViewportView(getJTextArea0());
		}
		return jScrollPane0;
	}

	private JTextArea getJTextArea0() {
		return txtLog;
	}

	private JLabel getLblCallee() {
		if (lblCallee == null) {
			lblCallee = new JLabel();
			if (isCaller) {
				lblCallee.setText("Callee's name");
			} else {
				lblCallee.setText("Your name");
			}
		}
		return lblCallee;
	}

	private JTextField getJTextField0() {
		if (txtCaller == null) {
			txtCaller = new JTextField();
		}
		return txtCaller;
	}

	private JTextField getJTextField1() {
		if (txtCallee == null) {
			txtCallee = new JTextField();
		}
		return txtCallee;
	}

	private JButton getJButton0() {
		if (btnClose == null) {
			btnClose = new JButton();
			btnClose.setText("Close");
		}
		return btnClose;
	}

	private JLabel getLblCaller() {
		if (lblCaller == null) {
			lblCaller = new JLabel();
			lblCaller.setText("Your name");
		}
		return lblCaller;
	}

	private static void installLnF() {
		try {
			String lnfClassname = PREFERRED_LOOK_AND_FEEL;
			if (lnfClassname == null)
				lnfClassname = UIManager.getCrossPlatformLookAndFeelClassName();
			UIManager.setLookAndFeel(lnfClassname);
		} catch (Exception e) {
			System.err.println("Cannot install " + PREFERRED_LOOK_AND_FEEL
					+ " on this platform:" + e.getMessage());
		}
	}

	private void startCall() {

		String callee = this.txtCallee.getText();

		if (isCaller) {
			String caller = this.txtCaller.getText();

			if (caller.length() < 1) {
				JOptionPane.showMessageDialog(this, "Please enter your name",
						"Missing caller name", JOptionPane.INFORMATION_MESSAGE);
				txtCaller.requestFocus();
				return;
			}

			if (callee.length() < 1) {
				JOptionPane.showMessageDialog(this, "Please enter callee name",
						"Missing callee name", JOptionPane.INFORMATION_MESSAGE);
				txtCallee.requestFocus();
				return;
			}

			threadCaller = new AppThreadCaller(caller, callee);
			threadCaller.start();
			this.btnCall.setEnabled(true);
		}

		else {

			if (callee.length() < 1) {
				JOptionPane.showMessageDialog(this, "Please enter your name",
						"Missing callee name", JOptionPane.INFORMATION_MESSAGE);
				txtCallee.requestFocus();
				return;
			}

			threadCallee = new AppThreadCallee(callee);
			threadCallee.start();
			this.btnCall.setEnabled(true);
		}
		
		this.btnClose.setText("Stop");
		callStarted = true;

	}

	public static void main(String[] args) {

		installLnF();
		int optionType = JOptionPane.DEFAULT_OPTION;
		int messageType = JOptionPane.PLAIN_MESSAGE; // no standard icon
		ImageIcon icon = new ImageIcon("pursuit.gif", "blob");
		Object[] selValues = { "Make a call", "Receive a call" };

		int res = JOptionPane.showOptionDialog(null, "What do you want to do?",
				"Call or receive?", optionType, messageType, icon, selValues,
				selValues[0]);

		if (res == 0) {
			isCaller = true;
		} else {
			isCaller = false;
		}
		
		if (args.length == 2)
		{
			if (args[0].equals("-f"))
			{
				Configuration.getInstance().configure(args[1]);
			}
			else{
				System.out.println("Bad argument. Expecting -f. Will continue with default file.");				
				Configuration.getInstance().configure("config");
			}
		}
		else{
			Configuration.getInstance().configure("config");
		}

		SwingUtilities.invokeLater(new Runnable() {
			@Override
			public void run() {
				VoPSI_App frame = new VoPSI_App();
				frame.setDefaultCloseOperation(VoPSI_App.EXIT_ON_CLOSE);
				frame.setTitle("VoPSI - Voice Application over PSI");
				frame.getContentPane().setPreferredSize(frame.getSize());
				frame.pack();
				frame.setLocationRelativeTo(null);
				frame.setVisible(true);
				if (!isCaller) {
					txtCaller.setVisible(false);
					lblCaller.setVisible(false);
					txtCallee.requestFocus();
				} else {
					txtCaller.requestFocus();
				}
			}
		});
	}

	private class AppThreadCaller extends Thread {

		private final String caller;
		private final String callee;
		private Node node;

		public AppThreadCaller(String caller, String callee) {
			this.caller = caller;
			this.callee = callee;
			this.node = new Node();
		}

		public void stopNode() {
			node.recorder.finish();			
			node.senderThread.finish();
			node.player.finish();			
			node.receiverThread.finish();
			
		}

		@Override
		public void run() {
			try {
				node.makeCall(caller, callee);
			} catch (NoSuchAlgorithmException e) {
				e.printStackTrace();
			} catch (UnsupportedEncodingException e) {
				e.printStackTrace();
			}
		}
	}

	private class AppThreadCallee extends Thread {

		private final String callee;
		private Node node;

		public AppThreadCallee(String callee) {
			this.callee = callee;
			this.node = new Node();
		}
		
		public void stopNode() {
			node.recorder.finish();			
			node.senderThread.finish();
			node.player.finish();			
			node.receiverThread.finish();
			
		}

		@Override
		public void run() {
			try {
				node.receiveCall(callee);
			} catch (NoSuchAlgorithmException e) {
				e.printStackTrace();
			} catch (UnsupportedEncodingException e) {
				e.printStackTrace();
			}
		}
	}
	
}
