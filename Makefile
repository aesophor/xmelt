CXX=g++
CXXFLAGS=-g -std=c++11 -flto -Os -Wall -pthread
SRC=$(wildcard src/*.cc)
BIN=meltdown

all:
	$(CXX) -o $(BIN) $(SRC) $(CXXFLAGS)

clean:
	rm $(BIN)

run:
	./$(BIN)
