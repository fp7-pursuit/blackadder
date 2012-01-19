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

package pubsub;

import uk.co.caprica.vlcj.player.MediaPlayerFactory;
import uk.co.caprica.vlcj.player.headless.HeadlessMediaPlayer;
import uk.co.caprica.vlcj.player.list.MediaList;
import uk.co.caprica.vlcj.player.list.MediaListPlayer;
import uk.co.caprica.vlcj.player.list.MediaListPlayerMode;

/**
 * The class responsible for starting/stopping the video
 * @author Ben Tagger
 * @version - started 13/10/2011
 */
public class VideoRunner{

	private String filePath;
	private int port;
    private HeadlessMediaPlayer mediaPlayer;
    private enum MEDIAPLAYING{FILE, HTTP};
    private MEDIAPLAYING currentMedia;
    private MediaListPlayer mlp;
    private MediaPlayerFactory factory;
    
	public VideoRunner(String filePath, int port){
		this.filePath = filePath;
		this.port = port;
		factory = new MediaPlayerFactory();
		mediaPlayer = factory.newMediaPlayer();			
		mlp = factory.newMediaListPlayer();
	}

	public void start(){
		// Original Strategy
		if (filePath.startsWith("http")){
			mediaPlayer.setPlaySubItems(true); // if it's a stream.
			mediaPlayer.playMedia(filePath, ":sout=#udp{dst=127.0.0.1:"+ port +"}");	
			currentMedia = MEDIAPLAYING.HTTP;
		}else{			
			MediaList playlist = factory.newMediaList();
			playlist.addMedia(filePath, ":sout=#udp{dst=127.0.0.1:"+ port +"}");		
			mlp.setMediaList(playlist);
			mlp.setMode(MediaListPlayerMode.LOOP);
			mlp.play();		
			currentMedia = MEDIAPLAYING.FILE;
		}		
	}
	
	public void stop(){
		switch(currentMedia){
		case HTTP:
			mediaPlayer.stop();
			mediaPlayer = factory.newMediaPlayer();	
			break;
		case FILE:
			mlp.stop();
			mlp = factory.newMediaListPlayer();
			break;
		}
	}
}
