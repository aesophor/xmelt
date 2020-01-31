CXX=gcc
CXXFLAGS=-flto -Os -Wall -lX11
SRC=$(wildcard *.c)
BIN=xmelt

all:
	$(CXX) -o $(BIN) $(SRC) $(CXXFLAGS)
	strip $(BIN) --strip-unneeded -s -R .comment -R .gnu.version

clean:
	rm $(BIN)

run:
	./$(BIN)
