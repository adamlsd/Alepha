CPPFLAGS+= -I ../../../
CXXFLAGS+= -std=c++1z
CXXFLAGS+= -g -O0
#CXXFLAGS+= -O3
CXX=g++
LDLIBS+= -lboost_thread -lboost_system
LDLIBS+= -lpthread
CC=gcc

TESTS=$(shell ls test* | sed -e "s/\.cc//g" -e 's/\.o//g' | sort | uniq)
TEST_OBJS=`ls test* | sed -e 's/.cc/.o/g' | grep '\.o\>' | sort | uniq`

all: $(TESTS)


HEADERS= ../MockMutex.h

test0.o: $(HEADERS)
test1.o: $(HEADERS)

clean:
	rm -f *.o $(TESTS)
