CXX?=g++
LIBDIR:=../lib/qos
CXXFLAGS?=-I$(LIBDIR) -Wall
LIBS:=-lblackadder -lpthread -ligraph -lcrypto

.PHONY: all clean

all: tm	tm_rv_brk

LIBOBJS:=metadatapacket.o lsmpacket.o bytearray.o
$(LIBOBJS): %.o: $(LIBDIR)/%.cpp $(LIBDIR)/%.hpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

%.o: %.cpp %.hpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

tm: tm_graph.o tm_igraph.o tm_qos.o tm_max_flow.o te_graph_mf.o \
	$(LIBOBJS) tm.o
	$(CXX) $(CXXFLAGS) $^ -o $@ $(LDFLAGS) $(LIBS)

tm_rv_brk: tm_graph.o tm_igraph.o tm_max_flow.o te_graph_mf.o tm_rv_brk.o
	$(CXX) $(CXXFLAGS) $^ -o $@ $(LDFLAGS) $(LIBS)

clean:
	-rm -f tm tm_rv_brk *.o 
