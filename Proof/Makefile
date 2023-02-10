#CXX=clang++
CXX=g++
CXXFLAGS+= -std=c++1z
#CXXFLAGS+= -stdlib=libstdc++
CXXFLAGS+= -O3
all: simple test

test.cc: Attestation.h
simple.cc: Attestation.h

clean:
	$(RM) *.o test
