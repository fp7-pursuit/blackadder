BA_LDFLAGS=-lblackadder -lpthread

all: channel_publisher channel_subscriber publisher subscriber nb_publisher nb_subscriber broadcast_subscriber broadcast_publisher algid_publisher algid_subscriber nb_channel_publisher nb_channel_subscriber

channel_publisher: channel_publisher.cpp
	$(CXX) $(CXXFLAGS) -O3 $^ -o $@ $(LDFLAGS) $(BA_LDFLAGS)
channel_subscriber: channel_subscriber.cpp
	$(CXX) $(CXXFLAGS) -O3 $^ -o $@ $(LDFLAGS) $(BA_LDFLAGS)
publisher: publisher.cpp
	$(CXX) $(CXXFLAGS) $^ -o $@ $(LDFLAGS) $(BA_LDFLAGS)
subscriber: subscriber.cpp
	$(CXX) $(CXXFLAGS) $^ -o $@ $(LDFLAGS) $(BA_LDFLAGS)
nb_publisher: nb_publisher.cpp
	$(CXX) $(CXXFLAGS) $^ -o $@ $(LDFLAGS) $(BA_LDFLAGS)
nb_subscriber: nb_subscriber.cpp
	$(CXX) $(CXXFLAGS) $^ -o $@ $(LDFLAGS) $(BA_LDFLAGS)
broadcast_subscriber: broadcast_subscriber.cpp
	$(CXX) $(CXXFLAGS) $^ -o $@ $(LDFLAGS) $(BA_LDFLAGS)
broadcast_publisher: broadcast_publisher.cpp
	$(CXX) $(CXXFLAGS) $^ -o $@ $(LDFLAGS) $(BA_LDFLAGS)
algid_publisher: algid_publisher.cpp
	$(CXX) $(CXXFLAGS) $^ -o $@ $(LDFLAGS) $(BA_LDFLAGS) -lcrypto
algid_subscriber: algid_subscriber.cpp
	$(CXX) $(CXXFLAGS) $^ -o $@ $(LDFLAGS) $(BA_LDFLAGS) -lcrypto
nb_channel_publisher: nb_channel_publisher.cpp
	$(CXX) $(CXXFLAGS) $^ -o $@ $(LDFLAGS) $(BA_LDFLAGS)
nb_channel_subscriber: nb_channel_subscriber.cpp
	$(CXX) $(CXXFLAGS) $^ -o $@ $(LDFLAGS) $(BA_LDFLAGS)

clean:
	rm -f channel_publisher channel_subscriber publisher subscriber nb_publisher nb_subscriber broadcast_publisher broadcast_subscriber algid_subscriber algid_publisher nb_channel_publisher nb_channel_subscriber
