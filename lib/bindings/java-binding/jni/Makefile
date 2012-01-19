CXX=g++
JNI_LOC = /usr/lib/jvm/java-6-openjdk/include/
#JNI_LOC = /usr/lib/jvm/java-6-sun/include
JNI_INC+=-I$(JNI_LOC) -I$(JNI_LOC)/linux

NAME=eu_pursuit_client_BlackadderWrapper

.PHONY: all clean

all: $(NAME).so

$(NAME).o: $(NAME).cc
	$(CXX) -c -g -fPIC $(CXXFLAGS) $(JNI_INC) -o $(NAME).o $(NAME).cc

$(NAME).so: $(NAME).o
	$(CXX) -shared -fPIC -o $(NAME).so $(NAME).o $(LDFLAGS) -lblackadder

clean:
	rm -f $(NAME).o $(NAME).so
