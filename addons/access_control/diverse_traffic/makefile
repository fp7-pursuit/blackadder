CXX_CFLAGS = -I../core
LIBRARIES = -lblackadder -lpthread
CORE = ../core/Transport.cpp ../core/PubSubWPayloadAPI.cpp ../core/BaseRV.cpp ../core/BaseEP.cpp
SOURCES= Publisher.cpp Subscriber.cpp ACRV.cpp

all: publisher subscriber rendezvous

publisher:Publisher_Channel.cpp Publisher_Document.cpp $(CORE) 
	g++ Publisher_Channel.cpp  $(CORE) $(CXX_CFLAGS) $(LIBRARIES) -g -o  Publisher_Channel
	g++ Publisher_Document.cpp  $(CORE) $(CXX_CFLAGS) $(LIBRARIES) -g -o  Publisher_Document
	
subscriber:Subscriber.cpp  $(CORE)
	g++ Subscriber.cpp $(CORE) $(CXX_CFLAGS) $(LIBRARIES)  -g -o  Subscriber
	
rendezvous:Diverse.cpp $(CORE)
	g++ Diverse.cpp $(CORE) $(CXX_CFLAGS) $(LIBRARIES)  -g -o  Rendezvous
