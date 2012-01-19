package view;

import pubsub.VideoPublisher;
import eu.pursuit.client.BlackAdderClient;

public interface PublisherView {

	public BlackAdderClient getClient();
	public VideoPublisher getVideoPublisher();
}
