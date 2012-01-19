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


package eu.pursuit.vopsi.gui;

import javax.swing.JTextArea;

public class TxtLog extends JTextArea {

	private static final long serialVersionUID = 1851181954398500326L;

	public void log(String msg) {
		setText(getText() + msg + "\n");
		setCaretPosition(getText().length());
		repaint();
	}
}
