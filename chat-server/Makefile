CXX = g++
CXXFLAGS = -std=c++11 -Wall -pthread
LDFLAGS = -lws2_32
DEPS = server.h

all: server client

server: main.cpp server.cpp $(DEPS)
	$(CXX) $(CXXFLAGS) -o server main.cpp server.cpp $(LDFLAGS)

client: client.cpp
	$(CXX) $(CXXFLAGS) -o client client.cpp $(LDFLAGS)

clean:
	del server.exe client.exe

.PHONY: all clean 