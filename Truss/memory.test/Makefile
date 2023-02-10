CPPFLAGS+= -I ../../../
CXXFLAGS+= -std=c++1z
CXXFLAGS+= -g -O0
#CXXFLAGS+= -O3
CXX=clang++
#LDLIBS+= -lboost_thread -lboost_system
CC=clang++

TESTS=test0 test1

all: $(TESTS)


HEADERS= ../memory.h ../../Mockination/MockFunction.h Makefile

test0.o: $(HEADERS)
test1.o: $(HEADERS)

clean:
	rm -f *.o $(TESTS)
