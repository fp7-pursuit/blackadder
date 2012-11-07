CXX_CFLAGS = -I../core
LIBRARIES = -lblackadder -lpthread
CORE = ../core/Transport.cpp ../core/PubSubWPayloadAPI.cpp ../core/BaseRV.cpp ../core/BaseEP.cpp
SOURCES= Publisher.cpp Subscriber.cpp ACRV.cpp

all: publisher subscriber rendezvous

publisher:Publisher.cpp $(CORE) 
	g++ Publisher.cpp  $(CORE) $(CXX_CFLAGS) $(LIBRARIES) -g -o  Publisher
	
subscriber:Subscriber.cpp  $(CORE)
	g++ Subscriber.cpp $(CORE) $(CXX_CFLAGS) $(LIBRARIES)  -g -o  Subscriber
	
rendezvous:ACRV.cpp $(CORE)
	g++ ACRV.cpp $(CORE) $(CXX_CFLAGS) $(LIBRARIES)  -g -o  Rendezvous
