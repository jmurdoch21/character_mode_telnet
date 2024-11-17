CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -pthread
LDFLAGS = -lsqlite3 -lcrypto

SRCS = server.cpp game.cpp room.cpp client.cpp client_terminal.cpp database.cpp ansi_commands.cpp bounding_box.cpp set_piece.cpp
OBJS = $(SRCS:.cpp=.o)
TARGET = server

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS)

#client.o: client.cpp client.h
#	$(CXX) $(CXXFLAGS) -c client.cpp
    
%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET)

.PHONY: all clean

