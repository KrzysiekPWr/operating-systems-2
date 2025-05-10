CXX = g++
CXXFLAGS = -std=c++11 -Wall -pthread
LDFLAGS = -lws2_32
DEPS = server.h

all: server client

server: main.cpp server.cpp $(DEPS)
	$(CXX) $(CXXFLAGS) -o server main.cpp server.cpp $(LDFLAGS)

client: client.cpp
	$(CXX) $(CXXFLAGS) -o client client.cpp $(LDFLAGS)

server_mingw: 
	i686-w64-mingw32-c++  -I/usr/i686-w64-mingw32/include  server.cpp main.cpp -o server -lws2_32 -static

client_mingw:

	i686-w64-mingw32-c++  -I/usr/i686-w64-mingw32/include  client.cpp -o client -lws2_32 -static

clean:
	del server.exe client.exe

.PHONY: all clean 