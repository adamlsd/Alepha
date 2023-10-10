CXXFLAGS+= -std=c++20 -I .

all: example

example: ProgramOptions.o string_algorithms.o Console.o word_wrap.o
