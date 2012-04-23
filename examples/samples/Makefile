all:
	$(CXX) $(CXXFLAGS) channel_publisher.cpp -o channel_publisher $(LDFLAGS) -lblackadder -lpthread
	$(CXX) $(CXXFLAGS) channel_subscriber.cpp -o channel_subscriber $(LDFLAGS) -lblackadder -lpthread
	$(CXX) $(CXXFLAGS) publisher.cpp  -o publisher $(LDFLAGS) -lblackadder -lpthread
	$(CXX) $(CXXFLAGS) subscriber.cpp -o subscriber $(LDFLAGS) -lblackadder -lpthread
	$(CXX) $(CXXFLAGS) nb_publisher.cpp  -o nb_publisher $(LDFLAGS) -lblackadder -lpthread
	$(CXX) $(CXXFLAGS) nb_subscriber.cpp -o nb_subscriber $(LDFLAGS) -lblackadder -lpthread
	$(CXX) $(CXXFLAGS) broadcast_subscriber.cpp -o broadcast_subscriber $(LDFLAGS) -lblackadder -lpthread
	$(CXX) $(CXXFLAGS) broadcast_publisher.cpp -o broadcast_publisher $(LDFLAGS) -lblackadder -lpthread
	$(CXX) $(CXXFLAGS) algid_publisher.cpp -o algid_publisher $(LDFLAGS) -lblackadder -lpthread -lssl
	$(CXX) $(CXXFLAGS) algid_subscriber.cpp -o  $(LDFLAGS) algid_subscriber -lblackadder -lpthread -lssl
	$(CXX) $(CXXFLAGS) nb_channel_publisher.cpp -o nb_channel_publisher $(LDFLAGS) -lblackadder -lpthread
	$(CXX) $(CXXFLAGS) nb_channel_subscriber.cpp -o nb_channel_subscriber $(LDFLAGS) -lblackadder -lpthread

clean:
	rm -f channel_publisher channel_subscriber publisher subscriber nb_publisher nb_subscriber broadcast_publisher broadcast_subscriber algid_subscriber algid_publisher nb_channel_publisher nb_channel_subscriber
