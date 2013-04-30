#  
#  Add your example to the EXEC list
#  

CXX?=g++
LIBDIR:=../../lib/qos
CXXFLAGS?=-I$(LIBDIR) -Wall
LIBS:= -lblackadder -lpthread -lssl -lcrypto

EXEC=	broadcast_linkstate_monitor \
	qos_publisher \
	qos_subscriber \
	metadata_provider

.PHONY: all clean

all: $(EXEC)

LIBOBJS:= bytearray.o metadatapacket.o
$(LIBOBJS): %.o: $(LIBDIR)/%.cpp $(LIBDIR)/%.hpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

%.o: %.cpp %.hpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Generic target compile. Override if you want different behavior.
%: %.cpp $(LIBOBJS)
	$(CXX) $(CXXFLAGS) $^ -o $@ $(LDFLAGS) $(LIBS)

qos_%: qos_%.cpp $(LIBOBJS)
	$(CXX) $(CXXFLAGS) $^ -o $@ $(LDFLAGS) $(LIBS)

clean:
	-rm -f $(EXEC)
	-rm -f *.o
