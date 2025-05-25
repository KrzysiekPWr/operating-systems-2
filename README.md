# operating-systems-2
# Dining Philosophers Problem and Resource Hierarchy Solution

## Introduction

The **Dining Philosophers Problem** is a classic synchronization problem in computer science, illustrating challenges in resource sharing and deadlock prevention. The scenario involves multiple philosophers sitting around a table, each requiring two chopsticks or forks (shared resources) to eat. Since each philosopher shares a chopstick with their neighbor, improper resource allocation can lead to **deadlock** (where no one can proceed) or **starvation** (where some philosophers never get to eat).

## Solution
This C++ implementation of the **Dining Philosophers Problem** ensures safe resource sharing using **mutexes** and **deadlock prevention** via a **resource hierarchy** (lower-numbered fork first). Each philosopher runs in a separate thread, alternating between **thinking** and **eating**, with `std::lock()` preventing circular wait conditions. A **console or table view** provides real-time status updates. The simulation runs for **x seconds**, then stops and displays eating statistics.

## Compiling
```
g++ -o output_file .\hungry_plilosophers.cpp --std=c++17
```

## Output
Console Table View:
```
Philosopher ID | State     | Eat Count
---------------|-----------|----------
Philosopher 0 |      EATING | 1
Philosopher 1 |    THINKING | 1
Philosopher 2 |      EATING | 2
Philosopher 3 |    THINKING | 2
Philosopher 4 |    THINKING | 1
Philosopher ID | State     | Eat Count
---------------|-----------|----------
```
or Console View:
```
Philosopher 2 is thinking.
Philosopher 3 is thinking.
Philosopher 1 is thinking.
Philosopher 4 is thinking.
Philosopher 0 is eating.
Philosopher 3 is eating.
Philosopher 0 has finished eating.
Philosopher 1 is eating.
Philosopher 0 is thinking.
Philosopher 1 has finished eating.
Philosopher 1 is thinking.
Philosopher 3 has finished eating.
Philosopher 3 is thinking.
Philosopher 2 is eating.
Philosopher 4 is eating.
Philosopher 4 has finished eating.
Philosopher 4 is thinking.
```

---
---

# Simple Chat Server

A basic console-based chat server implemented in C++ for Operating Systems class. This simplified version provides the core functionality with a single chat room.

## Shared Resources & Synchronization

### Server-Side (server.cpp, server.h)

#### Shared Resources
- `ChatServer::clients`: Map of connected clients (accessed by accept thread & client threads).
- `ChatServer::chat_rooms`: Map of chat rooms (modifiable by client threads).
- `ChatServer::clients_chat_rooms_numbers`: Maps client to room number.
- `ChatRoom::message_history`: Vector of messages in a room (accessed concurrently).
- `Client::socket_fd`: Write operations are protected (to ensure atomic sends).

#### Synchronization Tools
- `std::mutex`:
  - `clients_mutex`: Guards `clients`, `chat_rooms`, and `clients_chat_rooms_numbers`.
  - `ChatRoom::history_mutex`: Guards message history.
  - `Client::write_mutex`: Protects client socket sending.

#### Threads
- `accept_thread` (in `start()`):
  - Accepts new client connections.
  - Spawns `Client::client_thread` for each client.
- `Client::client_thread` (per client):
  - Handles communication from that specific client.
  - Invokes message handler for chat/broadcasting.

---

### Client-Side (client.cpp)
#### Threads
- Main Thread:
  - Reads user input and sends it to server.
  - Handles local commands (e.g. /exit).
- `receive_thread`:
  - Listens for and displays server messages (uses `print_mutex`).

---

### Summary
- Shared resources are protected using mutexes and atomic flags.
- Thread-per-client model allows concurrent message handling.
- Console and socket output is synchronized to avoid race conditions.

## Features

- Multithreaded server (one thread per client)
- Message synchronization (new clients see message history)
- Various room with default being chat room number 0. Room can be changed or/and created by client.

## Building

To build the server and client with g++ use:
```
make all
```

To build the server and client with mingw use:
```
make server_mingw
```
and
```
make client_mingw
```


This will create two executables: `server.exe` and `client.exe`.

## Running the Server

To start the server:

```
server.exe [port]
```

If no port is specified, the server will use the default port 8080.

## Running the Client

To start the client:

```
client.exe [server_ip] [port]
```

If no server IP is specified, it will connect to localhost (127.0.0.1).
If no port is specified, it will use the default port 8080.

## Client Commands

- `/help` - Display help information
- `/exit` - Exit the client
- `/chrm <n>` - /chrm <n> - Changes chat room to room with number <n>




Any other text is sent to all connected users in the same room.

## How It Works

1. The server creates a new thread for each client connection
2. Messages are synchronized between clients
3. All messages are stored in history and sent to new clients


## Demo
https://github.com/user-attachments/assets/7416562f-572c-4dfe-b1ec-0959859dde91


## Next Steps

You can expand this basic chat server by:

1. Adding private messaging between clients
2. Adding user authentication
3. Implementing file transfer capability
4. Adding a graphical user interface 
