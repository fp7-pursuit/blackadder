all:
	$(CXX) $(CXXFLAGS) channel_publisher.cpp -o channel_publisher $(LDFLAGS) -lblackadder -lpthread
	$(CXX) $(CXXFLAGS) channel_subscriber.cpp -o channel_subscriber $(LDFLAGS) -lblackadder -lpthread
	$(CXX) $(CXXFLAGS) publisher.cpp  -o publisher $(LDFLAGS) -lblackadder -lpthread
	$(CXX) $(CXXFLAGS) subscriber.cpp -o subscriber $(LDFLAGS) -lblackadder -lpthread
	$(CXX) $(CXXFLAGS) nb_publisher.cpp  -o nb_publisher $(LDFLAGS) -lblackadder -lpthread
	$(CXX) $(CXXFLAGS) nb_subscriber.cpp -o nb_subscriber $(LDFLAGS) -lblackadder -lpthread
	$(CXX) $(CXXFLAGS) broadcast_subscriber.cpp -o broadcast_subscriber $(LDFLAGS) -lblackadder -lpthread
	$(CXX) $(CXXFLAGS) broadcast_publisher.cpp -o broadcast_publisher $(LDFLAGS) -lblackadder -lpthread

clean:
	rm -f channel_publisher channel_subscriber publisher subscriber nb_publisher nb_subscriber broadcast_publisher broadcast_subscriber
